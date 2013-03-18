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

#include <string>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "core/dataHandler/WDataSetFibers.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"

#include "WMEdgeBundling.h"

WMEdgeBundling::WMEdgeBundling():
    WModule()
{
}

WMEdgeBundling::~WMEdgeBundling()
{
}

boost::shared_ptr< WModule > WMEdgeBundling::factory() const
{
    return boost::shared_ptr< WModule >( new WMEdgeBundling() );
}

const std::string WMEdgeBundling::getName() const
{
    return "Edge Bundling";
}

const std::string WMEdgeBundling::getDescription() const
{
    return "Performs edge bundling on line data.";
}

void WMEdgeBundling::connectors()
{
    m_fibersIC = WModuleInputData< WDataSetFibers >::createAndAdd( shared_from_this(), "fibers", "The line data to bundle." );
    m_maskIC   = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "mask", "Optional mask, specifying valid positions for segments" );
    m_fibersOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(), "bundledFibers", "The bundled line data." );

    WModule::connectors();
}

void WMEdgeBundling::properties()
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

    WModule::properties();
}

void WMEdgeBundling::requirements()
{
}

void WMEdgeBundling::initOpenCL()
{
    cl_int                      ret;
    std::string value;
    std::vector< cl::Platform > platforms;
    cl::Platform::get( &platforms );
    for( std::vector< cl::Platform >::const_iterator cit = platforms.begin(); cit != platforms.end(); ++cit )
    {
        cit->getInfo( CL_PLATFORM_PROFILE, &value );
        debugLog() << "CL_PLATFORM_PROFILE: " << value;
        cit->getInfo( CL_PLATFORM_VERSION, &value );
        debugLog() << "CL_PLATFORM_VERSION: " << value;
        cit->getInfo( CL_PLATFORM_NAME , &value );
        debugLog() << "CL_PLATFORM_NAME: " << value;
        cit->getInfo( CL_PLATFORM_VENDOR , &value );
        debugLog() << "CL_PLATFORM_VENDOR: " << value;
        cit->getInfo( CL_PLATFORM_EXTENSIONS, &value );
        debugLog() << "CL_PLATFORM_EXTENSIONS: " << value;
    }
}

void WMEdgeBundling::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_fibersIC->getDataChangedCondition() );
    m_moduleState.add( m_maskIC->getDataChangedCondition() );

    initOpenCL();

    ready();

    // main loop
    while( !m_shutdownFlag() )
    {
        infoLog() << "Waitings ...";
        m_moduleState.wait();

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        WDataSetScalar::SPtr mask = m_maskIC->getData();
        WDataSetFibers::SPtr fibers = m_fibersIC->getData();

        if( !fibers )
        {
            continue;
        }
    }
}
