//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
// For more information see http://www.openwalnut.org/copying
//
// This file is part of OpenWalnut.
//
// OpenWalnut is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWalnut is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with OpenWalnut. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <omp.h>

#include "core/common/datastructures/WDendrogram.h"
#include "core/common/datastructures/WUnionFind.h"
#include "core/common/WLimits.h"
#include "core/kernel/WKernel.h"
#include "WMDetTractClusteringGP.h"

WMDetTractClusteringGP::WMDetTractClusteringGP():
    WModule()
{
}

WMDetTractClusteringGP::~WMDetTractClusteringGP()
{
}

boost::shared_ptr< WModule > WMDetTractClusteringGP::factory() const
{
    return boost::shared_ptr< WModule >( new WMDetTractClusteringGP() );
}

const std::string WMDetTractClusteringGP::getName() const
{
    return "Deterministic Tract Clustering using Gaussian Proccesses";
}

const std::string WMDetTractClusteringGP::getDescription() const
{
    return "Clusters Gaussian Processes which represents deterministic tracts with the Wassermann "
           "approach described in the paper: http://dx.doi.org/10.1016/j.neuroimage.2010.01.004";
}

void WMDetTractClusteringGP::connectors()
{
    m_gpIC = WModuleInputData< WDataSetGP >::createAndAdd( shared_from_this(), "gpInput", "WDataSetGP providing the Gaussian processes" );
    m_dendOC = WModuleOutputData< WDendrogram >::createAndAdd( shared_from_this(), "dendrogramOutput", "WDendrogram as a result of this clustering" );
    m_matrixOC = WModuleOutputData< WDataSetMatrixSymFLT >::createAndAdd( shared_from_this(), "similaritiesOutput",
        "WDataSetMatrixSym containing the similarities" );

    WModule::connectors();
}

void WMDetTractClusteringGP::properties()
{
    WModule::properties();
}

void WMDetTractClusteringGP::moduleMain()
{
    m_moduleState.setResetable( true, true ); // remember actions when actually not waiting for actions
    m_moduleState.add( m_gpIC->getDataChangedCondition() );

    ready();

    while( !m_shutdownFlag() ) // loop until the module container requests the module to quit
    {
        debugLog() << "Waiting..";
        m_moduleState.wait();
        if( !m_gpIC->getData().get() ) // ok, the output has not yet sent data
        {
            continue;
        }

        boost::shared_ptr< WDataSetGP > dataSet = m_gpIC->getData();
        if( !dataSet || dataSet->size() == 0 )
        {
            debugLog() << "Invalid data--> continue";
            continue;
        }
        if( m_gpIC->handledUpdate() )
        {
            debugLog() << "Input has been updated...";
        }

        infoLog() << "Generating similarity matrix...";

        computeDistanceMatrix( dataSet );
        if( m_shutdownFlag() ) // since computeDistanceMatrix may quit earlier due to the m_shutdownFlag()
        {
            debugLog() << "Abort all computations since shutdown requested";
            break;
        }

        infoLog() << "Building dendrogram...";
        boost::shared_ptr< WDendrogram > result( computeDendrogram( dataSet->size() ) );
        if( m_shutdownFlag() ) // since computeDendrogram may quit earlier due to the m_shutdownFlag()
        {
            debugLog() << "Abort all computations since shutdown requested";
            break;
        }
        m_dendOC->updateData( result );

        infoLog() << "Done.";
    }
}

void WMDetTractClusteringGP::computeDistanceMatrix( boost::shared_ptr< const WDataSetGP > dataSet )
{
    const size_t steps = dataSet->size() * ( dataSet->size() - 2 ) / 2; // n(n-2)/2
    boost::shared_ptr< WProgress > progress( new WProgress( "Similarity matrix computation", steps ) );
    m_progress->addSubProgress( progress );

    m_similarities = WMatrixSymFLT::SPtr( new WMatrixSymFLT( dataSet->size() ) );

    #pragma omp parallel for
    for( int i = 0; i < static_cast< int >( dataSet->size() ); ++i )
    {
        for( size_t j = i + 1; j < dataSet->size() && !m_shutdownFlag(); ++j )
        {
            const WGaussProcess& p1 = ( *dataSet )[i];
            const WGaussProcess& p2 = ( *dataSet )[j];
            bool computeInnerProduct = p1.getBB().minDistance( p2.getBB() ) < ( p1.getRadius() + p2.getRadius() );
            float innerProduct = 0.0;
            if( computeInnerProduct )
            {
                innerProduct = gauss::innerProduct( p1, p2 ); // As written in the paper, we don't use the normalized inner product
            }
            #pragma omp critical
            {
                (*m_similarities)( i, j ) = innerProduct;
            }
        }
        #pragma omp critical
        {
            *progress = *progress + ( dataSet->size() - i - 1 );
        }
    }

    m_matrixOC->updateData( WDataSetMatrixSymFLT::SPtr( new WDataSetMatrixSymFLT( m_similarities ) ) );
    progress->finish();
    m_progress->removeSubProgress( progress );
}

boost::shared_ptr< WDendrogram > WMDetTractClusteringGP::computeDendrogram( size_t n )
{
    boost::shared_ptr< WDendrogram > dend( new WDendrogram( n ) );
    boost::shared_ptr< WProgress > progress( new WProgress( "Matrix => Dendrogram", n - 1 ) );
    m_progress->addSubProgress( progress );

    WUnionFind uf( n );
    std::vector< size_t > innerNode( n ); // The refernces from the canonical Elements (cE) to the inner nodes.
    std::set< size_t > idx; // valid indexes, to keep trac of already erased columns
    std::vector< size_t > clusterSize( n, 1 ); // to keep trac how many elements a cluster has.

    for( size_t i = 0; i < n; ++i )
    {
        innerNode[i] = i; // initialize them with their corresponding leafs.
        idx.insert( i );
    }

    for( size_t i = 0; i < n - 1 && !m_shutdownFlag(); ++i )
    {
        // Nearest Neighbour find: update p, q, and sim, so iterate over all valid matrix entries
        // NOTE, WARNING, ATTENTION: This is brute force NN finding strategy and requires O(n^2) time
        float maxSim = -wlimits::MAX_FLOAT; // This is not 0.0, since the similarity maybe very near to 0.0, and thus no new pair would be found!
        size_t p = 0;
        size_t q = 0;
        for( std::set< size_t >::const_iterator it = idx.begin(); it != idx.end() && !m_shutdownFlag(); ++it )
        {
            for( std::set< size_t >::const_iterator jt = boost::next( it ); jt != idx.end() && !m_shutdownFlag(); ++jt )
            {
                if( (*m_similarities)( *it, *jt ) > maxSim )
                {
                    maxSim = (*m_similarities)( *it, *jt );
                    p = *it;
                    q = *jt;
                }
            }
        }
        if( m_shutdownFlag() )
        {
            break;
        }

        uf.merge( p, q );
        size_t newCE = uf.find( p );
        innerNode[ newCE ] = dend->merge( innerNode[ p ], innerNode[ q ], maxSim );

        // erase one of the columns
        size_t col_to_delete = p;
        if( newCE == p )
        {
            col_to_delete = q;
        }
        idx.erase( col_to_delete );

        // update the column where now the new cluster pq resides
        for( std::set< size_t >::const_iterator it = idx.begin(); it != idx.end(); ++it )
        {
            if( *it != newCE )
            {
                // we have two Gauss processes p and q. We have merged p and q into pq. Hence for all valid indexes we must
                // recompute < pq, k > where k is a GP identified through an valid index, where:
                // < pq, k > = |p| / ( |p| + |q| ) < p, k > + |q| / (|p| + |q|) < q, k >
                double firstFactor = static_cast< double >( clusterSize[ p ] ) / ( clusterSize[ p ] + clusterSize[ q ] );
                double secondFactor = static_cast< double >( clusterSize[ q ] ) / ( clusterSize[ p ] + clusterSize[ q ] );
                (*m_similarities)( newCE, *it ) = firstFactor * (*m_similarities)( p, *it ) + secondFactor * (*m_similarities)( q, *it );
            }
        }
        clusterSize[ newCE ] = clusterSize[ p ] + clusterSize[ q ];
        ++*progress;
    }

    progress->finish();
    m_progress->removeSubProgress( progress );

    return dend;
}
