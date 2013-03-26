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

#include "WMDirectionHistogram.h"

WMDirectionHistogram::WMDirectionHistogram():
    WModule()
{
}

WMDirectionHistogram::~WMDirectionHistogram()
{
}

boost::shared_ptr< WModule > WMDirectionHistogram::factory() const
{
    return boost::shared_ptr< WModule >( new WMDirectionHistogram() );
}

const std::string WMDirectionHistogram::getName() const
{
    return "Direction Histogram";
}

const std::string WMDirectionHistogram::getDescription() const
{
    return "This module computes the histogram of direction on line data using start and end pints (global coloring scheme)";
}

void WMDirectionHistogram::connectors()
{
    m_fibersIC = WModuleInputData< WDataSetFibers >::createAndAdd( shared_from_this(), "fibers", "The line data to bundle." );

    WModule::connectors();
}

void WMDirectionHistogram::properties()
{
    WModule::properties();
}

void WMDirectionHistogram::requirements()
{
}

void WMDirectionHistogram::moduleMain()
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
