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

#include "core/dataHandler/WDataSetFibers.h"
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
    m_fibersOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(), "bundledFibers", "The bundled line data." );

    WModule::connectors();
}

void WMEdgeBundling::properties()
{
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
    }
}
