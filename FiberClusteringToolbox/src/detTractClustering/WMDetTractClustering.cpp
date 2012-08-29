//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include <osg/Geode>
#include <osg/Geometry>

#include "core/common/datastructures/WFiber.h"
#include "core/common/math/WMatrixSym.h"
#include "core/common/WAssert.h"
#include "core/common/WColor.h"
#include "core/common/WIOTools.h"
#include "core/common/WLogger.h"
#include "core/common/WProgress.h"
#include "core/common/WStringUtils.h"
#include "core/common/WThreadedFunction.h"
#include "core/dataHandler/datastructures/WFiberCluster.h"
#include "core/dataHandler/exceptions/WDHIOFailure.h"
#include "core/dataHandler/io/WReaderMatrixSymVTK.h"
#include "core/dataHandler/io/WWriterMatrixSymVTK.h"
#include "core/dataHandler/WDataSetFiberVector.h"
#include "core/dataHandler/WSubject.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/kernel/WKernel.h"
#include "WMDetTractClustering.h"
#include "WMDetTractClustering.xpm"

#ifdef CUDA_FOUND
#include "WMDetTractClusteringCudaInterface.h"
#endif

// This line is needed by the module loader to actually find your module.
W_LOADABLE_MODULE( WMDetTractClustering )

WMDetTractClustering::WMDetTractClustering()
    : WModule(),
      m_lastTractsSize( 0 ),
      m_dLtTableExists( false ),
      m_update( new WCondition() )
{
    m_osgNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
}

WMDetTractClustering::~WMDetTractClustering()
{
}

boost::shared_ptr< WModule > WMDetTractClustering::factory() const
{
    return boost::shared_ptr< WModule >( new WMDetTractClustering() );
}

const char** WMDetTractClustering::getXPMIcon() const
{
    return detTractClustering_xpm;
}

void WMDetTractClustering::moduleMain()
{
    m_moduleState.setResetable( true, true ); // modules state remembers fired events while not waiting
    m_moduleState.add( m_tractIC->getDataChangedCondition() );
    m_moduleState.add( m_update );

    ready();

    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        if( m_shutdownFlag() ) // in case of shutdown => abort
        {
            break;
        }

        if( !m_tractIC->getData().get() ) // ok, the output has not yet sent data
        {
            continue;
        }

        if( m_rawTracts != m_tractIC->getData() ) // in case data has changed
        {
            m_rawTracts = m_tractIC->getData();
            boost::shared_ptr< WProgress > convertProgress( new WProgress( "Converting tracts", 1 ) );
            m_progress->addSubProgress( convertProgress );
            m_tracts = boost::shared_ptr< WDataSetFiberVector >( new WDataSetFiberVector( m_rawTracts ) );
            m_numTracts->set( static_cast< int32_t >( m_tracts->size() ) );
            convertProgress->finish();
        }

        if( m_run->get( true ) == WPVBaseTypes::PV_TRIGGER_TRIGGERED )
        {
            infoLog() << "Start processing tracts";
            update();
            infoLog() << "Processing finished";
            m_run->set( WPVBaseTypes::PV_TRIGGER_READY, false );
        }

        if( m_clusterOutputID->changed() )
        {
            m_clusterOutputID->get( true );
            updateOutput();
        }
    }
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_osgNode );
}

void WMDetTractClustering::properties()
{
    m_maxDistance_t   = m_properties->addProperty( "Max cluster distance", "Maximum distance of two tracts in one cluster.", 6.5 );
    m_proximity_t     = m_properties->addProperty( "Min point distance", "Min distance of points of two tracts which should be considered", 0.0 );
    m_minClusterSize  = m_properties->addProperty( "Min cluster size", "Minium of tracts per cluster", 10 );
    m_clusterOutputID = m_properties->addProperty( "Output cluster ID", "This cluster ID will be connected to the output.", 0, m_update );
    m_run             = m_properties->addProperty( "Start clustering", "Start", WPVBaseTypes::PV_TRIGGER_READY, m_update );

    // information properties
    m_numTracts = m_infoProperties->addProperty( "#Tracts:", "Number of tracts beeing processed", 0 );
    m_numTracts->setMin( 0 );
    m_numTracts->setMax( wlimits::MAX_INT32_T );
    m_numUsedTracts = m_infoProperties->addProperty( "#Tracts used:", "Number of tracts beeing finally used in the clustering", 0 );
    m_numUsedTracts->setMin( 0 );
    m_numUsedTracts->setMax( wlimits::MAX_INT32_T );
    m_numClusters = m_infoProperties->addProperty( "#Clusters:", "Number of clusters beeing computed", 0 );
    m_numClusters->setMin( 0 );
    m_numClusters->setMax( wlimits::MAX_INT32_T );
    m_numValidClusters = m_infoProperties->addProperty( "#Clusters used:", "Number of clusters beeing bigger than the given threshold", 0 );
    m_numValidClusters->setMin( 0 );
    m_numValidClusters->setMax( wlimits::MAX_INT32_T );
    m_clusterSizes = m_infoProperties->addProperty( "Cluster sizes:", "Size of each valid cluster", std::string() );
#ifdef CUDA_FOUND
    m_useCuda         = m_properties->addProperty( "Use CUDA", "Prefer CUDA algorithm over CPU algorithm", true );
#endif

    WModule::properties();
}

void WMDetTractClustering::updateOutput()
{
    if( m_clusters.empty() )
    {
        warnLog() << "There are no clusters for the output, leave output connecter untouched";
        return;
    }

    if( m_clusterOutputID->get() >= static_cast< int >( m_clusters.size() ) || m_clusterOutputID->get() < 0 )
    {
        warnLog() << "Invalid cluster ID for output selected: " << m_clusterOutputID->get() << " using default ID 0";
        m_clusterOutputID->set( 0, true );
    }
    m_cluserOC->updateData( boost::shared_ptr< WFiberCluster >( new WFiberCluster( m_clusters[ m_clusterOutputID->get() ] ) ) );
}

void WMDetTractClustering::update()
{
    if( !( m_dLtTableExists = dLtTableExists() ) )
    {
        debugLog() << "Consider old table as invalid.";
        m_dLtTable.reset( new WMatrixSymDBL( m_tracts->size() ) );
    }

    cluster();

    boost::shared_ptr< WProgress > saveProgress( new WProgress( "Saving tracts", 1 ) );
    m_progress->addSubProgress( saveProgress );
    if( !fileExists( lookUpTableFileName() ) )
    {
        WWriterMatrixSymVTK w( lookUpTableFileName(), true );
        try
        {
            w.writeTable( m_dLtTable->getData(), m_lastTractsSize );
        }
        catch( const WDHIOFailure& e )
        {
            errorLog() << "Could not write dlt file: " << e.what();
        }
    }
    saveProgress->finish();

    // reset min max of the slider for the cluster ID
    m_clusterOutputID->setMin( 0 );
    m_clusterOutputID->setMax( m_clusters.size() - 1 );

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_osgNode );
    m_osgNode = paint();
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_osgNode );

    // TODO(math): For reasons of simplicity (no multiple input connectors possible) just forward one cluster to the voxelizer
    for( size_t i = 0; i < m_clusters.size(); ++i )
    {
        m_clusters[ i ].setDataSetReference( m_tracts );
        m_clusters[ i ].generateCenterLine();
    }

    updateOutput();
}

bool WMDetTractClustering::dLtTableExists()
{
    boost::shared_ptr< WProgress > readProgress( new WProgress( "Try to read dLt table", 1 ) );
    m_progress->addSubProgress( readProgress );

    std::string dLtFileName = lookUpTableFileName();

    if( fileExists( dLtFileName ) )
    {
        try
        {
            debugLog() << "trying to read table from: " << dLtFileName;
            WReaderMatrixSymVTK r( dLtFileName );
            boost::shared_ptr< std::vector< double > > data( new std::vector< double >() );
            r.readTable( data );
            m_dLtTable.reset( new WMatrixSymDBL( static_cast< size_t >( data->back() ) ) );
            m_lastTractsSize = static_cast< size_t >( data->back() );

            // remove the dimension from data array since it's not representing any distance
            data->pop_back();

            m_dLtTable->setData( *data );
            readProgress->finish();
            return true;
        }
        catch( const WDHException& e )
        {
            debugLog() << e.what() << std::endl;
        }
    }
    readProgress->finish();
    return false;
}

void WMDetTractClustering::cluster()
{
    bool useCuda = false;
#ifdef CUDA_FOUND
    useCuda = m_useCuda->get();
#endif
    double proximity_t = m_proximity_t->get();
    double maxDistance_t = m_maxDistance_t->get();
    size_t minClusterSize = m_minClusterSize->get();

    size_t numTracts = m_tracts->size();

    infoLog() << "Start clustering with " << numTracts << " tracts.";

    m_clusters.clear();  // remove evtl. old clustering

    m_clusterIDs = std::vector< size_t >( numTracts, 0 );

    for( size_t i = 0; i < numTracts; ++i )
    {
        m_clusters.push_back( WFiberCluster( i ) );
        m_clusterIDs[i] = i;
    }

    if( !m_dLtTableExists && useCuda )
    {
#ifdef CUDA_FOUND
        m_dLtTableExists = initDLtTableCuda( m_dLtTable, m_tracts, proximity_t, m_progress );
#endif
    }

    boost::shared_ptr< WProgress > progress( new WProgress( "Tract clustering", numTracts ) );
    m_progress->addSubProgress( progress );

    if( !m_dLtTableExists )
    {
        boost::shared_ptr< SimilarityMatrixComputation > threadInstance( new SimilarityMatrixComputation( m_dLtTable,
                                                                                                          m_tracts, proximity_t * proximity_t,
                                                                                                          m_shutdownFlag ) );
        WThreadedFunction< SimilarityMatrixComputation > threadPool( 2, threadInstance );
        threadPool.run();
        threadPool.wait();
    }

    for( size_t q = 0; q < numTracts; ++q )  // loop over all "symmetric" tract pairs
    {
        for( size_t r = q + 1;  r < numTracts; ++r )
        {
            if( m_clusterIDs[q] != m_clusterIDs[r] )  // both tracts are in different clusters
            {
                if( (*m_dLtTable)( q, r ) < maxDistance_t )  // q and r provide an inter-cluster-link
                {
                    meld( m_clusterIDs[q], m_clusterIDs[r] );
                }
            }
        }
        ++*progress;
    }

    progress->finish();
    m_dLtTableExists = true;

    boost::shared_ptr< WProgress > eraseProgress( new WProgress( "Erasing clusters", 1 ) );

    // remove empty clusters
    WFiberCluster emptyCluster;
    m_clusters.erase( std::remove( m_clusters.begin(), m_clusters.end(), emptyCluster ), m_clusters.end() );

    // determine #clusters and #small_clusters which are below a certain size
    size_t numSmallClusters = 0;
    for( size_t i = 0; i < m_clusters.size(); ++i )
    {
        m_clusters[i].sort();  // Refactor: why do we need sorting here?
        if( m_clusters[i].size() < minClusterSize )
        {
            m_clusters[i].clear();  // make small clusters empty to remove them easier
            ++numSmallClusters;
        }
    }
    m_numClusters->set( static_cast< int32_t >( m_clusters.size() ) );
    m_clusters.erase( std::remove( m_clusters.begin(), m_clusters.end(), emptyCluster ), m_clusters.end() );
    m_numValidClusters->set( static_cast< int32_t >( m_clusters.size() ) );

    m_lastTractsSize = m_tracts->size();
    eraseProgress->finish();
}

osg::ref_ptr< osg::Geode > WMDetTractClustering::genTractGeode( const WFiberCluster &cluster, const WColor& color ) const
{
    using osg::ref_ptr;
    ref_ptr< osg::Vec3Array > vertices = ref_ptr< osg::Vec3Array >( new osg::Vec3Array );
    ref_ptr< osg::Geometry > geometry = ref_ptr< osg::Geometry >( new osg::Geometry );

    std::list< size_t >::const_iterator cit = cluster.getIndices().begin();
    for( ; cit !=  cluster.getIndices().end(); ++cit )
    {
        const WFiber &fib = (*m_tracts)[ *cit ];
        for( size_t i = 0; i < fib.size(); ++i )
        {
            vertices->push_back( osg::Vec3( fib[i][0], fib[i][1], fib[i][2] ) );
        }
        geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINE_STRIP, vertices->size() - fib.size(), fib.size() ) );
    }

    geometry->setVertexArray( vertices );

    ref_ptr< osg::Vec4Array > colors = ref_ptr< osg::Vec4Array >( new osg::Vec4Array );
    colors->push_back( color );
    geometry->setColorArray( colors );
    geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
    osg::ref_ptr< osg::Geode > geode = osg::ref_ptr< osg::Geode >( new osg::Geode );
    geode->addDrawable( geometry.get() );

    return geode;
}


osg::ref_ptr< WGEManagedGroupNode > WMDetTractClustering::paint() const
{
    // get different colors via HSV color model for each cluster
    double hue = 0.0;
    double hue_increment = 1.0 / m_clusters.size();
    WColor color;

    osg::ref_ptr< WGEManagedGroupNode > result = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    size_t numUsedTracts = 0;
    std::stringstream clusterLog;
    for( size_t i = 0; i < m_clusters.size(); ++i, hue += hue_increment )
    {
        color = convertHSVtoRGBA( hue, 1.0, 0.75 );
        result->insert( genTractGeode( m_clusters[i], color ).get() );
        clusterLog << m_clusters[i].size() << " ";
        numUsedTracts += m_clusters[i].size();
    }
    m_clusterSizes->set( std::string( clusterLog.str() ) );
    m_numUsedTracts->set( static_cast< int32_t >( numUsedTracts ) );
    result->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    return result;
}

void WMDetTractClustering::meld( size_t qClusterID, size_t rClusterID )
{
    // first choose the cluster with the smaller ID
    if( qClusterID > rClusterID )  // merge always to the cluster with the smaller id
    {
        std::swap( qClusterID, rClusterID );
    }

    WFiberCluster& qCluster = m_clusters[ qClusterID ];
    WFiberCluster& rCluster = m_clusters[ rClusterID ];

    WAssert( !qCluster.empty() && !rCluster.empty(), "At least one cluster was empty while trying to merge!" );

    // second update m_clusterIDs array
    std::list< size_t >::const_iterator cit = rCluster.getIndices().begin();
    std::list< size_t >::const_iterator cit_end = rCluster.getIndices().end();
    for( ; cit != cit_end; ++cit )
    {
        m_clusterIDs[*cit] = qClusterID;
    }

    // and at last merge them
    qCluster.merge( rCluster );

    WAssert( rCluster.empty(), "The right cluster was no empty after melting!" );
}

void WMDetTractClustering::connectors()
{
    m_tractIC = WModuleInputData< WDataSetFibers >::createAndAdd( shared_from_this(), "tractInput", "A deterministic tract dataset." );
    m_cluserOC = WModuleOutputData< WFiberCluster >::createAndAdd( shared_from_this(), "clusterOutput", "A set of tract indices aka cluster" );

    WModule::connectors();  // call WModules initialization
}

std::string WMDetTractClustering::lookUpTableFileName() const
{
    std::stringstream newExtension;
    newExtension << std::fixed << std::setprecision( 2 );
    newExtension << ".pt-" << m_proximity_t->get() << ".dlt";
    boost::filesystem::path tractFileName( m_tracts->getFilename() );
    return tractFileName.replace_extension( newExtension.str() ).string();
}


WMDetTractClustering::OutputIDBound::OutputIDBound( const std::vector< WFiberCluster >& clusters )
    : m_clusters( clusters )
{
}

bool WMDetTractClustering::OutputIDBound::accept( boost::shared_ptr< WPropertyVariable< WPVBaseTypes::PV_INT > > /* property */,
                                               WPVBaseTypes::PV_INT value )
{
    return ( value >= 0 ) && ( value < static_cast< int >( m_clusters.size() ) );
}

WMDetTractClustering::SimilarityMatrixComputation::SimilarityMatrixComputation(
        boost::shared_ptr< WMatrixSymDBL > dLtTable,
        boost::shared_ptr< WDataSetFiberVector > tracts,
        double proxSquare,
        const WBoolFlag& shutdownFlag )
    : m_table( dLtTable ),
      m_tracts( tracts ),
      m_proxSquare( proxSquare ),
      m_shutdownFlag( shutdownFlag )
{
}

void WMDetTractClustering::SimilarityMatrixComputation::operator()( size_t id, size_t numThreads, WBoolFlag& b ) // NOLINT const ref
{
    wlog::debug( "WMDetTractClustering::SimilarityMatrixComputation" ) << "Thread: " << id << " starting its work";
    ( void ) b; // NOLINT for removing the warning about unused variables

    boost::function< double ( const WFiber& q, const WFiber& r ) > dLt; // NOLINT
    dLt = boost::bind( WFiber::distDLT, m_proxSquare, _1, _2 );

    size_t numTracts = m_tracts->size();
    size_t lines = 0;

    for( size_t q = 0; q < numTracts && !m_shutdownFlag() ; ++q )  // loop over all "symmetric" tract pairs
    {
        if( q % numThreads == id )
        {
            lines++;
            for( size_t r = q + 1;  r < numTracts; ++r )
            {
                (*m_table)( q, r ) = dLt( (*m_tracts)[q], (*m_tracts)[r] );
            }
        }
    }
    wlog::debug( "WMDetTractClustering::SimilarityMatrixComputation" ) << "Thread: " << id << " done processing " << lines << " lines.";
}


