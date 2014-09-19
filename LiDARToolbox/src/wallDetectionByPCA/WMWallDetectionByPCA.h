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

#ifndef WMWALLDETECTIONBYPCA_H
#define WMWALLDETECTIONBYPCA_H


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
#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WWallDetectOctree.h"
#include "../common/datastructures/quadtree/WQuadTree.h"

#include "../common/datastructures/WDataSetPointsGrouped.h"



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
 * OpenWalnut plugin for detecting plain surfaces, mainly walls and roofs.
 */
class WMWallDetectionByPCA: public WModule
{
public:
    WMWallDetectionByPCA();

    /**
     * Destroys this module.
     */
    virtual ~WMWallDetectionByPCA();

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
     * Due to the prototype design pattern used to build modules, this method returns a 
     * new instance of this method. NOTE: it should never be initialized or modified in 
     * some other way. A simple new instance is required.
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
     * Determines the resolution of the smallest octree nodes in 2^n meters. The smallest 
     * radius equals to its result.
     */
    WPropInt m_detailDepth;

    /**
     * WDataSetPoints data input (proposed for LiDAR data).
     */
    boost::shared_ptr< WModuleInputData< WDataSetPoints > > m_input;

    /**
     * The output triangle mesh that is proposed to show surface node groups.
     */
    boost::shared_ptr< WModuleOutputData< WTriangleMesh > > m_outputTrimesh;

    /**
     * The input point data that is given the information of colored data according to a wall voxel group
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_outputPoints;

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached 
     * on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * Determines the resolution of smallest octree nodes. Their radius equal that value.
     */
    WPropDouble m_detailDepthLabel;

    WPropTrigger  m_reloadData; //!< This property triggers the actual reading,

    /**
     * The maximal allowed angle between two node surface normal vectors. Nodes above 
     * that angle difference aren't grouped.
     */
    WPropDouble m_wallMaxAngleToNeighborVoxel;

    /**
     * The quotient of the second Eigen Value over the biggest. Nodes with a value
     * below that are treated as linear.
     */
    WPropDouble m_eigenValueQuotientLinear;

    /**
     * The biggest allowed value consisting of that: Weakest point distribution vector 
     * strength divided by the strongest. Nodes above that quotient aren't grouped.
     */
    WPropDouble m_eigenValueQuotientIsotropic;

    /**
     * Neighborship detection mode. It's simply the allowed count of dimensions where 
     * planes stand next to instead overlap. Having a regular grid these settings mean 
     * following neighborship kinds:
     *  1: Neighborship of 6
     *  2: Neighborship of 18
     *  3: Neighborship of 27
     */
    WPropInt m_cornerNeighborClass;

    /**
     * The minimal group size of nodes or its surface parts that makes voxels be
     * drawn. Especially parts of buildings have bigger connected parts than e. g. 
     * trees.
     */
    WPropInt m_minimalGroupSize;

    /**
     * Maximal node count of groups to display. Groups above that voxel count aren't 
     * put out.
     */
    WPropInt m_maximalGroupSize;

    /**
     * Minimal allowed point count per voxel. Very small pixel counts don't make sense
     * for the Principal Component Analysis.
     */
    WPropInt m_minimalPointsPerVoxel;

    /**
     * The mode how the output triangle mesh is organized.
     * 0: Voxels with a group color
     * 1: Rhombs displaying the node group, the three Eigen Vectors, relative Eigen
     *    Values and the mean coordinate of input points.
     */
    WPropSelection m_voxelOutlineMode;

    /**
     * Plugin progress status.
     */
    boost::shared_ptr< WProgress > m_progressStatus;
};

#endif  // WMWALLDETECTIONBYPCA_H
