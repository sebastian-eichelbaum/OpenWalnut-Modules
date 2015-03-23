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


#include "../common/algorithms/pointSaver/WPointSaver.h"


//!.Unnecessary imports
#include "core/common/WItemSelection.h"
#include "core/common/WItemSelector.h"

#include "core/kernel/WModuleOutputData.h"

#include <osg/Group>
#include <osg/Material>
#include <osg/StateAttribute>
#include "../common/algorithms/pointSubtractionHelper/WPointSubtactionHelper.h"

#include "core/kernel/WKernel.h"
#include "core/common/exceptions/WFileNotFound.h"
#include "core/common/WColor.h"
#include "core/common/WPathHelper.h"
#include "core/common/WPropertyHelper.h"
#include "core/common/WItemSelectionItem.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WGERequirement.h"
#include "../common/math/vectors/WVectorMaths.h"

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;


using std::numeric_limits;

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

    /**
     * Color adjustment reference constant - Automatical color adjustment.
     */
    static const size_t M_COLOR_AUTO;

    /**
     * Color adjustment reference constant - Automatical color adjustment with the offset
     * of 0.
     */
    static const size_t M_COLOR_AUTO_ONLY_CONTRAST;

    /**
     * Color adjustment reference constant - Manual color adjustment.
     */
    static const size_t M_COLOR_MANUAL;

    /**
     * Color adjustment reference constant - Editing colors manually with the same
     * aspect ratio between all colors.
     */
    static const size_t M_COLOR_MANUAL_JOINED;

    /**
     * Color adjustment reference constant - manual color adjustment with all enterable
     * numbers (without useing sliders).
     */
    static const size_t M_COLOR_MANUAL_UNBOUNDED;

    /**
     * Color adjustment reference constant - manual color adjustment with all enterable
     * numbers (without useing sliders). Aspect ratio between colors is kept.
     */
    static const size_t M_COLOR_MANUAL_UNBOUNDED_JOINED;

    /**
     * Color display mode - Colored.
     */
    static const size_t M_COLOR_MODE_COLORED;

    /**
     * Color display mode - Perceptional proportions of Red=30%, Green=59% and Blue=11%.
     */
    static const size_t M_COLOR_MODE_GREYSCALE_PERCEPTIONAL;

    /**
     * Color display mode - Proportional proportions of Red=33%, Green=33% and Blue=33%.
     */
    static const size_t M_COLOR_MODE_GREYSCALE_PROPORTIONAL;

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
     * Method that is executing for loading files. File is loaded every time when the 
     * path is correct.
     * \return Returns true if at least one point could be loaded.
     */
    bool onFileLoad();

    /**
     * Method that is executed to save a file if the save button is pressed.
     */
    void onFileSave();

    /**
     * Method that handles color intensity correction.
     */
    void onColorIntensityCorrect();

    /**
     * WDataSetPoints data input (proposed for LiDAR data).
     */
    vector<boost::shared_ptr< WModuleInputData< WDataSetPoints > > > m_input;

    /**
     * WDataSetPoints data that is subtracted from the input points.
     */
    boost::shared_ptr< WModuleInputData< WDataSetPoints > > m_inputSubtraction;

    /**
     * Processed point data with cut off points after cropping and subtracting points.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_output;

    /**
     * Processed point data with cut off points after cropping and subtracting points.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > > m_outputPointsGrouped;

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
     * Groups of the output second point data set.
     */
    WDataSetPointsGrouped::GroupArray m_outGroups;


    /**
     * Wall time of the point transformation routine.
     */
    WPropDouble m_infoRenderTimeSeconds;

    /**
     * Point count before transforming:
     */
    WPropInt m_infoInputPointCount;

    /**
     * Point count after transforming:
     */
    WPropInt m_infoOutputPointCount;

    /**
     * Information about minimal X/Y/Z values.
     */
    vector<WPropDouble> m_infoBoundingBoxMin;

    /**
     * Information about minimal X/Y/Z values.
     */
    vector<WPropDouble> m_infoBoundingBoxMax;

    /**
     * Information about minimal color channel intensity values.
     */
    vector<WPropDouble> m_infoColorMin;

    /**
     * Information about maximal color channel intensity values.
     */
    vector<WPropDouble> m_infoColorMax;

    /**
     * Options for surface features.
     */
    WPropGroup m_pointsCropGroup;

    /**
     * Minimal Coordinate value of the selection.
     */
    vector<WPropDouble> m_fromCoord;

    /**
     * Maximal Coordinate value of the selection.
     */
    vector<WPropDouble> m_toCoord;

    /**
     * Switch to cut away the selection instead of to crop the area.
     */
    WPropBool m_invertCropping;

    /**
     * The most right input connector is a point set which represents coordinates which 
     * are subtracted from all input points. This switch inverts the selection that is 
     * done by the subtraction.
     */
    WPropBool m_invertSubtraction;

    /**
     * Switch to temporarily turn of point set cropping.
     */
    WPropBool m_disablePointCrop;

    /**
     * Radius of points that are subtracted from the point set. The subtracted points 
     * are connected by the most right input.
     */
    WPropDouble m_pointSubtractionRadius;

    /**
     * Options for point coordinate translation using an offset vector.
     */
    WPropGroup m_translatePointsGroup;

    /**
     * Coordinate translation offset.
     */
    vector<WPropDouble> m_translationOffset;

    /**
     * Group that multiplies each coordinate by a factor.
     */
    WPropGroup m_groupMultiplyPoints;

    /**
     * Each coordinate is multiplied by this factor.
     */
    vector<WPropDouble> m_coordFactor;

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
     * Rotation anchor coordinate.
     */
    vector<WPropDouble> m_rotationAnchor;

    /**
     * Color equalizer settings group.
     */
    WPropGroup m_groupColorEqualizer;

    /**
     * Color contrast - Factor that is applied before adding the green offset.
     * Colors are sorted in order: Red, green, blue
     */
    vector<WPropDouble> m_contrast;

    /**
     * Color offset.- Offset that is added after applying the red factor.
     * Colors are sorted in order: Red, green, blue
     */
    vector<WPropDouble> m_colorOffset;

    /**
     * Type of the color adjustment:
     *     M_COLOR_AUTO = 0;
     *     M_COLOR_AUTO_ONLY_CONTRAST = 1;
     *     M_COLOR_MANUAL = 2;
     *     M_COLOR_MANUAL_JOINED = 3;
     *     M_COLOR_MANUAL_UNBOUNDED = 4;
     *     M_COLOR_MANUAL_UNBOUNDED_JOINED = 5;
     */
    WPropSelection m_colorAdjustmentType;

    /**
     * Color modes:
     *     M_COLOR_MODE_COLORED = 0 (Usual separated color channels).
     *     M_COLOR_MODE_GREYSCALE_PERCEPTIONAL = 1 (Red=30%, Green=59% and Blue=11%).
     *     M_COLOR_MODE_GREYSCALE_PROPORTIONAL = 2 (Red=33%, Green=33% and Blue=33%).
     */
    WPropSelection m_colorModeType;

    /**
     * Color equalizer settings group.
     */
    WPropGroup m_groupFileOperations;

    /**
     * Output file path where processed points are exported.
     */
    WPropFilename m_outputFile;

    /**
     * Button that triggers the point saving process.
     */
    WPropTrigger m_savePointsTrigger;

    /**
     * Input file path where processed points are loaded from.
     */
    WPropFilename m_inputFile;

    /**
     * Button that triggers the point reload process.
     */
    WPropTrigger m_reloadPointsTrigger;


    /**
     * Operation for WDataSetPointsGrouped points for conversion of input data to a 
     * point set with a group ID.
     */
    WPropGroup m_pointGroupOptionsGroup;

    /**
     * Minimal X value of the selection.
     */
    WPropInt m_assignedGroupID;

    /**
     * Point data set that is subtracted from input points. There is a radius araund 
     * points are removed from coordinates of this dataset.
     */
    WPointSubtactionHelper m_pointSubtraction;

    /**
     * Minimal coordinate of input points.
     */
    vector<double> m_minCoord;

    /**
     * Maximal coordinate of input points.
     */
    vector<double> m_maxCoord;

    /**
     * Minimal Color channel intensity.
     */
    vector<double> m_minColorIntensity;

    /**
     * Maximal Color channel intensity.
     */
    vector<double> m_maxColorIntensity;

    /**
     * Sets how many points should be skipped after adding a single point to the output.
     */
    WPropInt m_skipRatio;

    /**
     * Input file points that are merged into the input point sets.
     */
    WPointSaver m_pointInputFile;

    /**
     * File that can be saved after altering points.
     */
    WPointSaver m_pointOutputFile;
};

#endif  // WMPOINTSTRANSFORM_H
