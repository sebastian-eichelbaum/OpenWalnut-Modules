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

// C headers
// ...

// C++ headers
#include <string>

// External lib headers
#include <boost/shared_ptr.hpp>

// OW core headers
#include <core/common/WAssert.h>
#include <core/common/WPropertyHelper.h>

// own, local headers
#include "WMAnotherModule.h"
#include "WMAnotherModule.xpm"

WMAnotherModule::WMAnotherModule():
    WModule()
{
}

WMAnotherModule::~WMAnotherModule()
{
    // cleanup
}

boost::shared_ptr< WModule > WMAnotherModule::factory() const
{
    // See "src/modules/template/" for an extensively documented example.
    return boost::shared_ptr< WModule >( new WMAnotherModule() );
}

const char** WMAnotherModule::getXPMIcon() const
{
    return WMAnotherModule_xpm;
}

const std::string WMAnotherModule::getName() const
{
    return "My Other Module";
}

const std::string WMAnotherModule::getDescription() const
{
    // Specify your module description here. Be detailed. This text is read by the user.
    return "TODO: write a module description here";
}

void WMAnotherModule::connectors()
{
    // create the inputs
    // ...

    // and the outputs
    // ...

    // call WModule's initialization
    WModule::connectors();
}

void WMAnotherModule::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition() );

    // Add your properties here

    // Call parent method
    WModule::properties();
}

void WMAnotherModule::moduleMain()
{
    m_moduleState.add( m_propCondition );
    ready();

    // You do not see the debug messages on console output? Set log-level in Settings menu to debug.
    debugLog() << "Hey! Here I am (debug).";
    infoLog() <<  "Hey! Here I am (info).";
    warnLog() << "Hey! Here I am (warning).";
    errorLog() << "Hey! Here I am (error).";

    // lets go
    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        if( m_shutdownFlag() )
        {
            break;
        }

        // do your stuff here
    }
}

