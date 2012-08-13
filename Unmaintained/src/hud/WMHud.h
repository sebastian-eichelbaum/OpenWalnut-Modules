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

#ifndef WMHUD_H
#define WMHUD_H

#include <string>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Node>
#include <osgText/Text>

#include "core/kernel/WModule.h"
#include "core/graphicsEngine/WGEGroupNode.h"
#include "core/graphicsEngine/WPickInfo.h"

/**
 * This module implements several onscreen status displays. At the moment the main purpose
 * is the display of information from picking, i.e. what is picked.
 * \ingroup modules
 */
class WMHud : public WModule
{
public:
    /**
     * standard constructor
     */
    WMHud();

    /**
     * destructor
     */
    virtual ~WMHud();

    /**
     * Returns the name of this module.
     * \return the module's name.
     */
    virtual const std::string getName() const;

    /**
     * Returns a description of this module.
     * \return description of module.
     */
    virtual const std::string getDescription() const;

    /**
     * Sets pick text member variable
     *
     * \param pickInfo information about the pick
     */
    void updatePickText( WPickInfo pickInfo );

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

private:
    boost::shared_mutex m_updateLock; //!< Lock to prevent concurrent threads trying to update the osg node

    /**
     * Projection node for defining view frustrum for HUD
     */
    osg::ref_ptr<osg::Projection> m_rootNode;

    /**
     * Geometry group for all HUD related things
     */
    osg::ref_ptr< WGEGroupNode > m_HUDs;

    /**
     * Text instance that will show up in the HUD
     */
    osg::ref_ptr< osgText::Text > m_osgPickText;

    bool m_updatedPickText; //!< Tells us whether the picktext has been updated an has to be rendered.

    /**
     * string to store the pick result from the picking method
     */
    std::string m_pickText;

    /**
     * Set up of the HUD
     */
    void init();

    /**
     * Updating HUD text
     */
    void update();

    /**
     * Gets signaled from the properties object when something was changed. Now, only m_active is used. This method therefore simply
     * activates/deactivates the HUD.
     */
    void activate();

    /**
     * The update callback that is called for the osg node of this module.
     */
    void updateCallback();
};

#endif  // WMHUD_H
