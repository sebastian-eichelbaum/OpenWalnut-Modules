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

#include "WMFiberColoring.h"
#include "WSimilarityColoring.h"
#include "WBoyColoring.h"

WMFiberColoring::WMFiberColoring():
    WModule(),
    m_strategy( "Fiber Coloring Algorithms", "Select one algorithm to color fibers", NULL,
                "FiberColor", "A list of all known bundling strategies" )
{
    m_strategy.addStrategy( WBoyColoring::SPtr( new WBoyColoring() ) );
    m_strategy.addStrategy( WSimilarityColoring::SPtr( new WSimilarityColoring() ) );
}

WMFiberColoring::~WMFiberColoring()
{
}

boost::shared_ptr< WModule > WMFiberColoring::factory() const
{
    return boost::shared_ptr< WModule >( new WMFiberColoring() );
}

const std::string WMFiberColoring::getName() const
{
    return "Fiber Coloring";
}

const std::string WMFiberColoring::getDescription() const
{
    return "Performs coloring schemes on fibers.";
}

void WMFiberColoring::connectors()
{
    m_fibersIC = WModuleInputData< WDataSetFibers >::createAndAdd( shared_from_this(), "fibers", "The line data to bundle." );
    m_fibersOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(), "coloredFibers", "The colored line data." );

    WModule::connectors();
}

void WMFiberColoring::properties()
{
    m_properties->addProperty( m_strategy.getProperties() );

    WModule::properties();
}

void WMFiberColoring::requirements()
{
}

void WMFiberColoring::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_fibersIC->getDataChangedCondition() );
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

        WDataSetFibers::SPtr fibers = m_fibersIC->getData();

        if( !fibers )
        {
            continue;
        }

        WProgress::SPtr progress( new WProgress( "Coloring Fibers", fibers->size() ) );
        m_progress->addSubProgress( progress );
        debugLog() << "Start coloring";
        m_fibersOC->updateData( m_strategy()->operator()( progress, m_shutdownFlag, fibers ) );
        progress->finish();
        m_progress->removeSubProgress( progress );
        debugLog() << "Finished coloring";
    }
}
