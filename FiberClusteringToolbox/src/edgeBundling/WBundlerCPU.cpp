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

#include <vector>

#include "WBundlerCPU.h"

WBundlerCPU::WBundlerCPU()
    : WObjectNDIP< WEdgeBundlingInterface >( "Edge Bundler CPU", "Bundles edges of fibers (CPU implementation)" )
{
    m_stiffness      = m_properties->addProperty( "Stiffness", "Variability of fiber sample points", 0.5 );
    m_stiffness->setMin( 0.0 );
    m_stiffness->setMax( 1.0 );
    m_fixedEndings   = m_properties->addProperty( "Fixed Ending", "If true, endings of fibers will not change positions", true );
    m_maxCurvature   = m_properties->addProperty( "Max curvature", "Curvature Threshold as constraint for moving positions", 1.0 );
    m_maxCurvature->setMin( 0.0 );
    m_maxCurvature->setMax( 1.0 );
    m_curveSegments  = m_properties->addProperty( "Curvature Segments", "How many segments used for Max Curvature threshold", 1 );
    m_curveSegments->setMin( 0 );
    m_minDistance    = m_properties->addProperty( "Min Point Distance", "How close may edges packed", 0.0 );
    m_minDistance->setMin( 0.0 );
    m_minDistance->setMax( 1.0 );
    m_maxExtension   = m_properties->addProperty( "Max Extension", "Percentage how much fibers may be elongated", 100.0 );
    m_maxExtension->setMin( 0.0 );
    m_maxContraction = m_properties->addProperty( "Max Contraction", "Percentage how much fibers may be shortened", 50.0 );
    m_maxContraction->setMin( 0.0 );
    m_maxContraction->setMax( 100.0 );
    m_maxRadius      = m_properties->addProperty( "Attraction Radius", "Only segments within this radius will contribute to attraction", 0.0 );
    m_maxRadius->setMin( 0.0 );
    m_maxIter = m_properties->addProperty( "Max Iterations", "Only this number of iterations are performed", 0 );
    m_maxIter->setMin( 0 );
}

WBundlerCPU::~WBundlerCPU()
{
}

WDataSetFibers::SPtr WBundlerCPU::operator()( WProgress::SPtr progress, WBoolFlag const & shutdown, WDataSetFibers::SPtr fibers,
                                              WDataSetScalar::SPtr mask )
{
    if( !fibers )
    {
        wlog::debug( "WBundlerCPU" ) << "Got an invalid fiber dataset, no bundling performed";
        return fibers;
    }

    // Assume that there is a valid dataset
    WDataSetFibers::IndexArray   fibStart = fibers->getLineStartIndexes();
    WDataSetFibers::LengthArray  fibLen   = fibers->getLineLengths();
    WDataSetFibers::VertexArray  fibVerts = fibers->getVertices();

    WDataSetFibers::IndexArray   newStarts( new std::vector< size_t > );
    WDataSetFibers::LengthArray  newLen( new std::vector< size_t > );
    WDataSetFibers::VertexArray  newVerts( new std::vector< float > );
    WDataSetFibers::IndexArray   newReverseMap( new std::vector< size_t > );

    for( size_t fidx = 0; fidx < fibStart->size(); ++fidx ) // iterate over fibers
    {
        if( shutdown )
        {
            wlog::info( "WBundlerCPU" ) << "Abort bundling due to shutdown request.";
            break;
        }
        ++*progress;
        size_t firstPointIdx = fibStart->at( fidx );
        size_t lastPointIdx = fibStart->at( fidx ) + 3 * ( fibLen->at( fidx ) - 1 );

        newStarts->push_back( newVerts->size() / 3 );
        newVerts->push_back( fibVerts->at( firstPointIdx ) );
        newVerts->push_back( fibVerts->at( firstPointIdx + 1 ) );
        newVerts->push_back( fibVerts->at( firstPointIdx + 2 ) );
        newVerts->push_back( fibVerts->at( lastPointIdx ) );
        newVerts->push_back( fibVerts->at( lastPointIdx  + 1 ) );
        newVerts->push_back( fibVerts->at( lastPointIdx  + 2 ) );
        newLen->push_back( 2 );
        newReverseMap->push_back( fidx );
        newReverseMap->push_back( fidx );
    }

    return WDataSetFibers::SPtr( new WDataSetFibers( newVerts, newStarts, newLen, newReverseMap ) );
}
