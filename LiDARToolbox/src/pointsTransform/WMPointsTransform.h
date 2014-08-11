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

#ifndef WMPOINTSTRANSFORM_H
#define WMPOINTSTRANSFORM_H


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
#include "../common/datastructures/octree/WOctree.h"





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
 * Transforms a point set: Available options: Cropping, stretching, translation and 
 * rotation.
 * \ingroup modules
 */
class WMPointsTransform: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMPointsTransform();

    /**
     * Destroys this module.
     */
    virtual ~WMPointsTransform();

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
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;
    /**
     * Calculates the bounding box of the input point data set. Minimals and
     * maximals of X/Y/Z are set.
     * \param isFirstPointSet Tells whether the point set is the first one. So the first 
     *                        coordinate will be overridden as min/max value for each 
     *                        coordinate in the first check.
     */
    void initBoundingBox( bool isFirstPointSet );
    /**
     * Assigns the bounding box value to the minimal and maximal values of the cropping 
     * settings.
     */
    void setMinMax();
    /**
     * Returns a cropped data set corresponding to the selection. The selection is
     * set by m_<from/to>_<X/Y/Z>. m_cutInsteadOfCrop determines whether to crop to
     * a selection or to cut away a cube area.
     * \return The cropped or cut point data set.
     */
    void addTransformedPoints();

    /**
     * WDataSetPoints data input (proposed for LiDAR data).
     */
    vector<boost::shared_ptr< WModuleInputData< WDataSetPoints > > > m_input;
    /**
     * Processed point data with cut off outliers.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_output;

    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * Plugin progress status that is shared with the reader.
     */
    boost::shared_ptr< WProgress > m_progressStatus;

    /**
     * Input point coordinates to crop.
     */
    WDataSetPoints::VertexArray m_inVerts;
    /**
     * Colors of the input point data set that are also passed through.
     */
    WDataSetPoints::ColorArray m_inColors;
    /**
     * Vertices of the output point data set.
     */
    WDataSetPoints::VertexArray m_outVerts;
    /**
     * Colors of the output point data set.
     */
    WDataSetPoints::ColorArray m_outColors;

    /**
     * Options for surface features.
     */
    WPropGroup m_pointsCropGroup;
    /**
     * Minimal X value of the selection.
     */
    WPropDouble m_fromX;
    /**
     * Maximal X value of the selection.
     */
    WPropDouble m_toX;
    /**
     * Maximal Y value of the selection.
     */
    WPropDouble m_fromY;
    /**
     * Minimal Y value of the selection.
     */
    WPropDouble m_toY;
    /**
     * Minimal Z value of the selection.
     */
    WPropDouble m_fromZ;
    /**
     * Maximal Z value of the selection.
     */
    WPropDouble m_toZ;
    /**
     * Switch to cut away the selection instead of to crop the area.
     */
    WPropBool m_cutInsteadOfCrop;

    /**
     * Options for surface features.
     */
    WPropGroup m_translatePointsGroup;
    /**
     * X coordinate translation offset.
     */
    WPropDouble m_translateX;
    /**
     * Y coordinate translation offset.
     */
    WPropDouble m_translateY;
    /**
     * Z coordinate translation offset.
     */
    WPropDouble m_translateZ;

    /**
     * Group that multiplies each coordinate by a factor.
     */
    WPropGroup m_groupMultiplyPoints;
    /**
     * Each X coordinate is multiplied by this value
     */
    WPropDouble m_factorX;
    /**
     * Each Y coordinate is multiplied by this value
     */
    WPropDouble m_factorY;
    /**
     * Each Z coordinate is multiplied by this value
     */
    WPropDouble m_factorZ;

    /**
     * Rotation options.
     */
    WPropGroup m_groupRotation;
    /**
     * 1st applied rotation: Along the plane XY
     */
    WPropDouble m_rotation1AngleXY;
    /**
     * 2nd applied rotation: Along the plane XY
     */
    WPropDouble m_rotation2AngleYZ;
    /**
     * 3rd applied rotation: Along the plane XY
     */
    WPropDouble m_rotation3AngleXZ;
    /**
     * Rotation anchor on the X coordinate.
     */
    WPropDouble m_rotationAnchorX;
    /**
     * Rotation anchor on the Y coordinate.
     */
    WPropDouble m_rotationAnchorY;
    /**
     * Rotation anchor on the Z coordinate.
     */
    WPropDouble m_rotationAnchorZ;

    /**
     * Minimal X coordinate of input points.
     */
    double m_minX;
    /**
     * Maximal X coordinate of input points.
     */
    double m_maxX;
    /**
     * Minimal Y coordinate of input points.
     */
    double m_minY;
    /**
     * Maximal Y coordinate of input points.
     */
    double m_maxY;
    /**
     * Minimal Z coordinate of input points.
     */
    double m_minZ;
    /**
     * Maximal Z coordinate of input points.
     */
    double m_maxZ;
    /**
     * Sets how many points should be skipped after adding a single point to the output.
     */
    WPropInt m_skipRatio;
};

#endif  // WMPOINTSTRANSFORM_H
