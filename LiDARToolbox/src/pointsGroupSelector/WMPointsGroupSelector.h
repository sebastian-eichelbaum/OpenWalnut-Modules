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

#ifndef WMPOINTSGROUPSELECTOR_H
#define WMPOINTSGROUPSELECTOR_H


#include <liblas/liblas.hpp>
#include <string>
#include <vector>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include "core/kernel/WModule.h"

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WTriangleMesh.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include "../datastructures/octree/WOctree.h"

#include "../datastructures/WDataSetPointsGrouped.h"
#include "core/dataHandler/WDataSetPoints.h"



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

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;

/**
 * Select a point group in order to display it as a triangle mesh outline or data set points. Each 
 * group can be displayed in a single color depicting all groups..
 * \ingroup modules
 */
class WMPointsGroupSelector: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMPointsGroupSelector();

    /**
     * Destroys this module.
     */
    virtual ~WMPointsGroupSelector();

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
     *Initialize the connectors this module is using.
     */
    virtual void connectors();

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

    /**
     * Initialize requirements for this module.
     */
    virtual void requirements();

private:
    /**
     * Initializes progress bar settings.
     * \param steps Points count as reference to the progress bar.
     */
    void setProgressSettings( size_t steps );

    /**
     * WDataSetPointsGrouped data input (Grouped point data).
     */
    boost::shared_ptr< WModuleInputData< WDataSetPointsGrouped > > m_input;

    /**
     * Data output connector for triangle mesh output.
     */
    boost::shared_ptr< WModuleOutputData< WTriangleMesh > > m_outputTrimesh;
    /**
     * Data output connector for data set points output.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_outputPoints;

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * Info tab property: Input points count.
     */
    WPropInt m_nbPoints;
    /**
     * Info tab property: Minimal x value of output x coordunates.
     */
    WPropDouble m_xMin;
    /**
     * Info tab property: Maximal x value of output x coordunates.
     */
    WPropDouble m_xMax;
    /**
     * Info tab property: Minimal y value of output x coordunates.
     */
    WPropDouble m_yMin;
    /**
     * Info tab property: Maximal y value of output x coordunates.
     */
    WPropDouble m_yMax;
    /**
     * Info tab property: Minimal z value of output x coordunates.
     */
    WPropDouble m_zMin;
    /**
     * Info tab property: Maximal z value of output x coordunates.
     */
    WPropDouble m_zMax;
    /**
     * Voxel count that is cut off and kept regarding the ISO value.
     */

    WPropDouble m_stubSize;

    /**
     * Voxel count that is cut off and kept regarding the ISO value.
     */
    WPropDouble m_contrast;
    /**
     * Determines the resolution of the smallest octree nodes in 2^n meters
     */
    WPropInt m_detailDepth;
    /**
     * Determines the resolution of the smallest octree nodes in meters
     */
    WPropDouble m_detailDepthLabel;
    /**
     * Depicting the input data set points showing the point outline instead of regions
     * depicted as cubes that cover existing points.
     */
    WPropBool m_showTetraedersInsteadOfOctreeCubes;
    /**
     * Depicting the input data set points showing the point outline instead of regions
     * depicted as cubes that cover existing points.
     */
    WPropBool m_highlightUsingColors;

    /**
     * Property to choose an output building of a voxel group number. Currently 0 is 
     * cutting nothing and 1 is is showing all buildings altogether.
     */
    WPropInt m_selectedShowableBuilding;

    /**
     * Plugin progress status that is shared with the reader.
     */
    boost::shared_ptr< WProgress > m_progressStatus;
};

#endif  // WMPOINTSGROUPSELECTOR_H
