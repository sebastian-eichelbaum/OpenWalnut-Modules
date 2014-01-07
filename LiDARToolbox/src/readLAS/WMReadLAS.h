//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#ifndef WMREADLAS_H
#define WMREADLAS_H


#include <liblas/liblas.hpp>
#include <string>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include "core/kernel/WModule.h"

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/shaders/WGEShader.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>





//!.Unnecessary imports
#include "core/common/WItemSelection.h"
#include "core/common/WItemSelector.h"

#include "core/kernel/WModuleOutputData.h"

#include <osg/Group>
#include <osg/Material>
#include <osg/StateAttribute>

#include "core/kernel/WKernel.h"
#include "core/common/exceptions/WFileNotFound.h"
#include "core/common/WColor.h"
#include "core/common/WPathHelper.h"
#include "core/common/WPropertyHelper.h"
#include "core/common/WItemSelectionItem.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WGERequirement.h"

#include "WLasReader.h"

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;

/**Draws cubes where a value is at least as big as the preset ISO value
 * \ingroup modules*/
class WMReadLAS: public WModule
{
public:
    /**Creates the module for drawing contour lines.*/
    WMReadLAS();

    /**Destroys this module.*/
    virtual ~WMReadLAS();

    /**Gives back the name of this module.
     * \return the module's name.*/
    virtual const std::string getName() const;

    /**Gives back a description of this module.
     * \return description to module.*/
    virtual const std::string getDescription() const;

    /**Due to the prototype design pattern used to build modules, this method returns a new instance of this method. NOTE: it
     * should never be initialized or modified in some other way. A simple new instance is required.
     * \return the prototype used to create every module in OpenWalnut.*/
    virtual boost::shared_ptr< WModule > factory() const;

    /**Get the icon for this module in XPM format.
     * \return The icon.*/
    virtual const char** getXPMIcon() const;

protected:
    /**Entry point after loading the module. Runs in separate thread.*/
    virtual void moduleMain();

    /**Initialize the connectors this module is using.*/
    virtual void connectors();

    /**Initialize the properties for this module.*/
    virtual void properties();

    /**Initialize requirements for this module.*/
    virtual void requirements();

private:
    /**
     * Refreshs the minimal and maximal values of the scrollbars
     */
    void refreshScrollBars();

    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_output;  //!< Output connector provided by this module.

    /**The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.*/
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**Needed for recreating the geometry, incase when resolution changes.*/
    boost::shared_ptr< WCondition > m_propCondition;

    /**Shader unit for drawing. */
    WGEShader::RefPtr m_shader;

    /**Instance for applying drawable geoms. */
    osg::ref_ptr< osg::Geode > m_geode;

    /**
     * Path of the LiDAR input file (www.liblas.org)
     */
    WPropFilename m_lasFile; //!< The mesh will be read from this file.
    /**
     * The maximal width of the output data.
     */
    WPropInt m_outputDataWidth;
    /**
     * Scrollbar that changes the minimal output X value
     */
    WPropInt m_scrollBarX;
    /**
     * Scrollbar that changes the minimal output Y value
     */
    WPropInt m_scrollBarY;
    /**
     * Enables to put the output data to the coordinate system center
     */
    WPropBool m_translateDataToCenter;

    WPropInt m_nbVertices; //!< Info-property showing the number of vertices in the mesh.


    /**
     * Instance that puts out a WDataSetPoints of a LiDAR file (see www.liblas.org).
     */
    laslibb::WLasReader reader;
};

#endif  // WMREADLAS_H
