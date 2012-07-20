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
#include "WMMyNewModule.h"
#include "WMMyNewModule.xpm"

WMMyNewModule::WMMyNewModule():
    WModule()
{
}

WMMyNewModule::~WMMyNewModule()
{
    // cleanup
}

boost::shared_ptr< WModule > WMMyNewModule::factory() const
{
    // See "src/modules/template/" for an extensively documented example.
    return boost::shared_ptr< WModule >( new WMMyNewModule() );
}

const char** WMMyNewModule::getXPMIcon() const
{
    return WMMyNewModule_xpm;
}

const std::string WMMyNewModule::getName() const
{
    return "My New Module";
}

const std::string WMMyNewModule::getDescription() const
{
    // Specify your module description here. Be detailed. This text is read by the user.
    return "TODO: write a module description here";
}

void WMMyNewModule::connectors()
{
    // create the inputs
    // ...

    // and the outputs
    // ...

    // call WModule's initialization
    WModule::connectors();
}

void WMMyNewModule::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition() );

    // Add your properties here

    // Call parent method
    WModule::properties();
}

void WMMyNewModule::moduleMain()
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
        debugLog() << "You will find the module resources here: " << ( m_localPath / "some_local_resource_file" ).string();
    }
}

