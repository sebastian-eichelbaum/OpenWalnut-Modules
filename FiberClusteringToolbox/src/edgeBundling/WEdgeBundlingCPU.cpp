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

#include "WEdgeBundlingCPU.h"

WEdgeBundlingCPU::WEdgeBundlingCPU()
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
}

WEdgeBundlingCPU::~WEdgeBundlingCPU()
{
}

WDataSetFibers::SPtr WEdgeBundlingCPU::operator()( WProgress::SPtr progress, WBoolFlag const & shutdown, WDataSetFibers::SPtr fibers, WDataSetScalar::SPtr mask )
{
    WDataSetFibers::SPtr result( new WDataSetFibers() );
    return result;
}
