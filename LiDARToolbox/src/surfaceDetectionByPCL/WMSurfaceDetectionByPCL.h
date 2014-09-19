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

#ifndef WMSURFACEDETECTIONBYPCL_H
#define WMSURFACEDETECTIONBYPCL_H


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
#include "core/common/WRealtimeTimer.h"

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;

/**
 * Detects surfaces using the Region Growing Segmentation of the Point Cloud Library 
 * http://pointclouds.org/
 * \ingroup modules
 */
class WMSurfaceDetectionByPCL: public WModule
{
public:
    /**
     * Creates the module for the Building detection.
     */
    WMSurfaceDetectionByPCL();

    /**
     * Destroys this module.
     */
    virtual ~WMSurfaceDetectionByPCL();

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
     * WDataSetPoints data input (proposed for LiDAR data).
     */
    boost::shared_ptr< WModuleInputData< WDataSetPoints > > m_input;

    /**
     * WDataSetPointsGrouped data output as point groups depicting each building
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > > m_outputPointsGrouped;

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
     * Info tab property: Input points count.
     */
    WPropInt m_infoNbPoints;

    /**
     * Info field - Wall time for the whole surface detection process.
     */
    WPropDouble m_infoRenderTimeSeconds;

    /**
     * Info field - The points per second detection rate of the segmentation process.
     */
    WPropDouble m_infoPointsPerSecond;

    /**
     * Info tab property: Minimal x value of input x coordunates.
     */
    WPropDouble m_infoXMin;

    /**
     * Info tab property: Maximal x value of input x coordunates.
     */
    WPropDouble m_infoXMax;

    /**
     * Info tab property: Minimal y value of input x coordunates.
     */
    WPropDouble m_infoYMin;

    /**
     * Info tab property: Maximal y value of input x coordunates.
     */
    WPropDouble m_infoYMax;

    /**
     * Info tab property: Minimal z value of input x coordunates.
     */
    WPropDouble m_infoZMin;

    /**
     * Info tab property: Maximal z value of input x coordunates.
     */
    WPropDouble m_infoZMax;

    WPropTrigger  m_reloadData; //!< This property triggers the actual reading,

    /**
     * Minimal cluster point count of detected surfaces.
     */
    WPropInt m_clusterSizeMin;

    /**
     * Maximal cluster point count of detected surfaces.
     */
    WPropInt m_clusterSizeMax;

    /**
     * The count of considered neighbors during considering the analysis point wise.
     */
    WPropInt m_numberOfNeighbours;

    /**
     * Allows to set smoothness threshold used for testing the points. The angle is 
     * scaled by degrees and it is a new threshold value for the angle between normals.
     */
    WPropDouble m_smoothnessThresholdDegrees;

    /**
     * Curvature threshold used for testing the points. It is a threshold value for 
     * curvature testing.
     */
    WPropDouble m_curvatureThreshold;


    /**
     * Plugin progress status.
     */
    boost::shared_ptr< WProgress > m_progressStatus;
};

#endif  // WMSURFACEDETECTIONBYPCL_H
