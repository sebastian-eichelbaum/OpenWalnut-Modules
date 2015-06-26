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

#ifndef WMATLASSURFACES_H
#define WMATLASSURFACES_H

#include <string>
#include <vector>
#include <utility>
#include <map>

#include <osg/Geode>
#include <osg/Uniform>

#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"

#include "core/graphicsEngine/WGEGroupNode.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/graphicsEngine/shaders/WGEShader.h"

class WDataSetScalar;



/**
 * Create surface representation of atlas from volumetric atlas file
 * and text file with labels for atlas regions.
 *
 * \ingroup modules
 */
class WMAtlasSurfaces: public WModule
{
public:
    /**
     *
     */
    WMAtlasSurfaces();

    /**
     *
     */
    virtual ~WMAtlasSurfaces();

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
     * function that updates the currently shown gfx according to the current selection
     */
    void updateGraphics();

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

    /**
     * Creates the osg geometry nodes from the previously created triangle meshes
     */
    void createOSGNode();

private:
    /**
     * Creates a triangle mesh for each region
     */
    void createSurfaces();

    /**
     * Callback to listen for property changes
     */
    void propertyChanged();

    /**
     * Helper function to read in a text file
     *
     * \param fileName
     * \return the text file as a vector of strings for each line
     */
    std::vector< std::string > readFile( const std::string fileName );

    /**
     * Helper function to read, parse and store the labels
     *
     * \param fileName
     */
    void loadLabels( std::string fileName );

    /**
     * function creates arbitrary rois from selected regions and adds them to the roi manager
     */
    void createRoi();

    /**
     * extracts an area from the dataset
     * \param index index of the region
     */
    void cutArea( int index );


    boost::shared_ptr< WModuleInputData< WDataSetScalar > > m_input;  //!< Input connector required by this module.

    boost::shared_ptr< const WDataSetScalar > m_dataSet; //!< pointer to dataSet to be able to access it throughout the whole module.

    boost::shared_ptr< std::vector< boost::shared_ptr< WTriangleMesh > > >m_regionMeshes2; //!< stores pointers to all triangle meshes

    std::map< size_t, std::pair< std::string, std::string > >m_labels; //!< the labels with their id

    osg::ref_ptr< WGEGroupNode > m_moduleNode; //!< Pointer to the modules group node. We need it to be able to update it when callback is invoked.

    osg::ref_ptr< osg::Geode > m_outputGeode; //!< Pointer to geode containing the glyphs

    bool m_dirty; //!< flag true if something happened that requires redrawing of gfx

    bool m_labelsLoaded; //!< true when a label file is loaded

    /**
     * A condition used to notify about changes in several properties.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * A property allowing the user to select multiple elements of a list.
     */
    WPropSelection m_aMultiSelection;
    WPropFilename m_labelFile; //!< The labels will be read from this file.

    /**
     * A list of items that can be selected using m_aSingleSelection or m_aMultiSelection.
     */
    boost::shared_ptr< WItemSelection > m_possibleSelections;

    WPropTrigger  m_propCreateRoiTrigger; //!< This property triggers the actual reading,
    WPropInt m_opacityProp; //!< Property holding the opacity valueassigned to the surface
    /**
     * The shader
     */
    osg::ref_ptr< WGEShader > m_shader;
};

#endif  // WMATLASSURFACES_H
