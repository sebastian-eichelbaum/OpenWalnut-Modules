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

#ifndef WMSURFACEDETECTIONBYLARI_H
#define WMSURFACEDETECTIONBYLARI_H


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
#include "../datastructures/quadtree/WQuadTree.h"

#include "../datastructures/WDataSetPointsGrouped.h"



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
 * Detects building structures within a WDataSetPoints. The recognition algorithm works by the 
 * principle of relative height minimum height thresholding.
 * \ingroup modules
 */
using std::cout;
using std::endl;

/**
 * OpenWalnut module that should correspond to the approach fo the paper of Lari/Habib 
 * (2014). It's a new method for the surface detection. This plug in is currently in a 
 * very early stage and can not be used productively at all.
 * 
 * Currently it analyzes each node if it's either planar or linear/cylindrical. The 
 * first one are marked red and the other two blue. Points that meet both criterias are 
 * magenta. Those of none of them are grey.
 */
class WMSurfaceDetectionByLari: public WModule
{
public:
    /**
     * Creates the module for the Building detection.
     */
    WMSurfaceDetectionByLari();

    /**
     * Destroys this module.
     */
    virtual ~WMSurfaceDetectionByLari();

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
    WPropDouble m_xMin;
    /**
     * Info tab property: Maximal x value of input x coordunates.
     */
    WPropDouble m_xMax;
    /**
     * Info tab property: Minimal y value of input x coordunates.
     */
    WPropDouble m_yMin;
    /**
     * Info tab property: Maximal y value of input x coordunates.
     */
    WPropDouble m_yMax;
    /**
     * Info tab property: Minimal z value of input x coordunates.
     */
    WPropDouble m_zMin;
    /**
     * Info tab property: Maximal z value of input x coordunates.
     */
    WPropDouble m_zMax;



    /**
     * The maximal count of analyzed neighbors of an 
     * examined input point.
     */
    WPropInt m_numberPointsK;
    /**
     * Maximal radius within which the nearest neighbors are examined.
     */
    WPropDouble m_maxPointDistanceR;
    /**
     * The lower limit of the first normalized lambda (eigen value) to detect point's 
     * feature as planar. The eigen values are sorted descending.
     */
    WPropDouble m_surfaceNLambda1Min;
    /**
     * The higher limit of the first normalized lambda (eigen value) to detect point's 
     * feature as planar. The eigen values are sorted descending.
     */
    WPropDouble m_surfaceNLambda1Max;
    /**
     * The lower limit of the second normalized lambda (eigen value) to detect point's 
     * feature as planar. The eigen values are sorted descending.
     */
    WPropDouble m_surfaceNLambda2Min;
    /**
     * The higher limit of the second normalized lambda (eigen value) to detect point's 
     * feature as planar. The eigen values are sorted descending.
     */
    WPropDouble m_surfaceNLambda2Max;
    /**
     * The lower limit of the third normalized lambda (eigen value) to detect point's 
     * feature as planar. The eigen values are sorted descending.
     */
    WPropDouble m_surfaceNLambda3Min;
    /**
     * The higher limit of the third normalized lambda (eigen value) to detect point's 
     * feature as planar. The eigen values are sorted descending.
     */
    WPropDouble m_surfaceNLambda3Max;
    /**
     * The lower limit of the first normalized lambda (eigen value) to detect point's 
     * feature as linear/cylindrical. The eigen values are sorted descending.
     */
    WPropDouble m_cylNLambda1Min;
    /**
     * The higher limit of the first normalized lambda (eigen value) to detect point's 
     * feature as linear/cylindrical. The eigen values are sorted descending.
     */
    WPropDouble m_cylNLambda1Max;
    /**
     * The lower limit of the second normalized lambda (eigen value) to detect point's 
     * feature as linear/cylindrical. The eigen values are sorted descending.
     */
    WPropDouble m_cylNLambda2Min;
    /**
     * The higher limit of the second normalized lambda (eigen value) to detect point's 
     * feature as linear/cylindrical. The eigen values are sorted descending.
     */
    WPropDouble m_cylNLambda2Max;
    /**
     * The lower limit of the third normalized lambda (eigen value) to detect point's 
     * feature as linear/cylindrical. The eigen values are sorted descending.
     */
    WPropDouble m_cylNLambda3Min;
    /**
     * The higher limit of the third normalized lambda (eigen value) to detect point's 
     * feature as linear/cylindrical. The eigen values are sorted descending.
     */
    WPropDouble m_cylNLambda3Max;


    WPropTrigger  m_reloadData; //!< This property triggers the actual reading,

    /**
     * Plugin progress status.
     */
    boost::shared_ptr< WProgress > m_progressStatus;
};

#endif  // WMSURFACEDETECTIONBYLARI_H
