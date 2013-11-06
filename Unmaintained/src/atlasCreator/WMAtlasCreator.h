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

#ifndef WMATLASCREATOR_H
#define WMATLASCREATOR_H

#include <string>
#include <vector>

#include <osg/Geode>

#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"

#include "core/dataHandler/WDataSetScalar.h"
#include "core/dataHandler/WValueSet.h"


/**
 * Someone should add some documentation here.
 * Probably the best person would be the module's
 * creator, i.e. "schurade".
 *
 * This is only an empty template for a new module. For
 * an example module containing many interesting concepts
 * and extensive documentation have a look at "src/modules/template"
 *
 * \ingroup modules
 */
class WMAtlasCreator: public WModule
{
public:
    /**
     *
     */
    WMAtlasCreator();

    /**
     *
     */
    virtual ~WMAtlasCreator();

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
     * loads and parses the meta file
     * \param path to the meta file
     * \return true if a meta file was successfully loaded, false otherwise
     */
    bool loadPngs( boost::filesystem::path path );

    /**
     * inserts a slice, given as a png image into the volume
     * \param image path to the image file
     */
    void addPngToVolume( boost::filesystem::path image );

    /**
     * updates the output connector
     */
    void updateOutDataset();

    WPropTrigger  m_propReadTrigger; //!< This property triggers the actual reading,
    WPropFilename m_propDirectory; //!< The png files will be loaded form this directory

    /**
     * A condition used to notify about changes in several properties.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    std::vector< std::string > m_regions; //!< store the region names extracted fromt he file name

    std::vector< uint8_t >m_volume; //!< volume data created from 2d images

    /**
     * An output connector for the output scalar dsataset.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetScalar > > m_output;

    int m_xDim; //!< x Dimension of the volume
    int m_yDim; //!< y Dimension of the volume
    int m_zDim; //!< z Dimension of the volume
};

#endif  // WMATLASCREATOR_H
