#ifndef WMMYNEWMODULE_H
#define WMMYNEWMODULE_H

#include <string>

#include <core/kernel/WModule.h>
#include <core/kernel/WModuleOutputData.h>
#include <core/kernel/WModuleInputData.h>
#include <core/common/WProperties.h>

/**
 * TODO write some documentation here
 *
 * \ingroup modules
 */
class WMMyNewModule: public WModule
{
public:
    /**
     * Constructor.
     */
    WMMyNewModule();

    /**
     * Destructor
     */
    virtual ~WMMyNewModule();

    /**
     * Gives back the name of this module.
     * \return the module's name.
     */
    virtual const std::string getName() const;

    /**
     * Gives back a description of this module.
     * \return description to module.
     */
    virtual const std::string getDescription() const;

    /**
     * Due to the prototype design pattern used to build modules, this method returns a new instance of this method. NOTE: it
     * should never be initialized or modified in some other way. A simple new instance is required.
     *
     * \return the prototype used to create every module in OpenWalnut.
     */
    virtual boost::shared_ptr< WModule > factory() const;

    /**
     * Get the icon for this module in XPM format.
     * \return The icon.
     */
    virtual const char** getXPMIcon() const;

protected:
    /**
     * Entry point after loading the module. Runs in separate thread.
     */
    virtual void moduleMain();

    /**
     * Initialize the connectors this module is using.
     */
    virtual void connectors();

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

private:
    /**
     * Condition used throughout the module to notify the thread if some changes happened (like properties have changed and similar).
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Connectors
    //  * provide all the input and output connections here
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ...

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Properties
    //  * provide all your module properties here
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ...

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Other members
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ...
};

#endif  // WMMYNEWMODULE_H
