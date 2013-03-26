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

#ifndef WMLOADCLUSTERS_H
#define WMLOADCLUSTERS_H

#include <string>

#include <core/kernel/WModule.h>
#include <core/kernel/WModuleInputData.h>
#include <core/kernel/WModuleOutputData.h>

#include <core/dataHandler/WDataSetFiberClustering.h>

/**
 * A module for loading fiber clusterings.
 */
class WMLoadClusters : public WModule
{
public:
    /**
     * Constructor.
     */
    WMLoadClusters();

    /**
     * Destructor.
     */
    virtual ~WMLoadClusters();

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
     * \return the icon.
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
    //! A condition for property updates.
    boost::shared_ptr< WCondition > m_propCondition;

    //! The filename/path of the file to read from.
    WPropFilename m_propFilename;

    //! The output connector.
    boost::shared_ptr< WModuleOutputData< WDataSetFiberClustering > > m_output;
};

#endif  // WMLOADCLUSTERS_H
