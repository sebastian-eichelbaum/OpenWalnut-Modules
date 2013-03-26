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

#include <core/common/WStrategyHelper.h>
#include <core/dataHandler/WDataSetFibers.h>
#include <core/dataHandler/WDataSetScalar.h>
#include <core/kernel/WKernel.h>
#include <core/kernel/WModuleInputData.h>

// For now we will implement a CPU version first and later do OpenCL implementation
// #include "WOpenCLCode.h"
#include "WMEdgeBundling.h"
#include "WEdgeBundlingCPU.h"

WMEdgeBundling::WMEdgeBundling():
    WModule(),
    m_strategy( "Edge Bundling Algorithms", "Select one algorithm to bundle fibers via edge bundling", NULL,
                "Bundler", "A list of all known bundling strategies" )
{
    m_strategy.addStrategy( WEdgeBundlingCPU::SPtr( new WEdgeBundlingCPU() ) );
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
    m_properties->addProperty( m_strategy.getProperties() );

    WModule::properties();
}

void WMEdgeBundling::requirements()
{
}

void WMEdgeBundling::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_fibersIC->getDataChangedCondition() );
    m_moduleState.add( m_maskIC->getDataChangedCondition() );
    m_moduleState.add( m_strategy.getProperties()->getUpdateCondition() );

    ready();

    // main loop
    while( !m_shutdownFlag() )
    {
        infoLog() << "Waiting ...";
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

        WProgress::SPtr progress( new WProgress( "Bundling Edges", fibers->size() ) );
        m_progress->addSubProgress( progress );
        debugLog() << "Starting bundling edges";
        m_fibersOC->updateData( m_strategy()->operator()( progress, m_shutdownFlag, fibers, mask ) );
        progress->finish();
        m_progress->removeSubProgress( progress );
        debugLog() << "Finished bundling edges";
    }
}
