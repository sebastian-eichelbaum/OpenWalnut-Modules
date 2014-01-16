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

#include <string>
#include <utility>
#include <vector>

#include "core/common/datastructures/WFiber.h"
#include "core/common/WProgress.h"
#include "core/dataHandler/WDataSetFibers.h"
#include "core/dataHandler/WDataSetFiberVector.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"
#include "WLengthCulling.h"
#include "WZhangThresholdCulling.h"
#include "WMDetTractCulling.h"

WMDetTractCulling::WMDetTractCulling()
    : WModule(),
      m_strategy( "Fiber culling strategies", "Select one of these strategies to cullout some unwanted fibers.", NULL,
                  "Culling Strategies", "A list of all known culling strategies." )
{
    m_strategy.addStrategy( WLengthCulling::SPtr( new WLengthCulling() ) );
    m_strategy.addStrategy( WZhangThresholdCulling::SPtr( new WZhangThresholdCulling() ) );
}

WMDetTractCulling::~WMDetTractCulling()
{
}

boost::shared_ptr< WModule > WMDetTractCulling::factory() const
{
    return boost::shared_ptr< WModule >( new WMDetTractCulling() );
}

void WMDetTractCulling::moduleMain()
{
    m_moduleState.setResetable( true, true ); // remember actions when actually not waiting for actions
    m_moduleState.add( m_tractIC->getDataChangedCondition() );
    m_moduleState.add( m_strategy.getProperties()->getUpdateCondition() );

    ready();

    while( !m_shutdownFlag() ) // loop until the module container requests the module to quit
    {
        debugLog() << "Waiting...";

        m_moduleState.wait();

        if( !m_tractIC->getData() ) // only continue if there is a valid dataset
        {
            continue;
        }

        m_dataset = m_tractIC->getData();

        debugLog() << "Start culling out fibers";
        WProgress::SPtr progress( new WProgress( "Culling out fibers", m_dataset->size() ) );
        m_progress->addSubProgress( progress );
        std::pair< WDataSetFibers::SPtr, WDataSetFibers::SPtr > result = m_strategy()->operator()( m_dataset, progress );
        m_remainingTractsOC->updateData( result.first );
        m_culledTractsOC->updateData( result.second );
    }
}

void WMDetTractCulling::connectors()
{
    m_tractIC = WModuleInputData< WDataSetFibers >::createAndAdd( shared_from_this(), "tractInput",
            "Some deterministic tracts" );
    m_remainingTractsOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(),
            "remainingTractsOutput", "Tracts that survied culling." );
    m_culledTractsOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(),  "culledTractsOutput",
            "Tracts that were culled." );

    WModule::connectors();  // call WModules initialization
}

void WMDetTractCulling::properties()
{
    m_properties->addProperty( m_strategy.getProperties() );

    WModule::properties();
}
