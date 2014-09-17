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

#ifndef WMPOINTGROUPSTRANSFORM_H
#define WMPOINTGROUPSTRANSFORM_H


#include <liblas/liblas.hpp>
#include <string>
#include <vector>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include "core/kernel/WModule.h"
#include "../common/algorithms/groupEdit/WGroupEdit.h"

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WTriangleMesh.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/datastructures/octree/WOctree.h"
#include "../common/datastructures/WDataSetPointsGrouped.h"
#include "WVoxelOutliner.h"


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

/**
 * Class for latering WDataSetPointsGrouped point set. Contains method to put out in 
 * WDataSetPoints in order to watch it using e. g. Point Renderer. It has options like 
 * merging WDataSetPointsGrouped points, subtracting point sets and some other.
 * \ingroup modules
 */
class WMPointGroupsTransform: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMPointGroupsTransform();

    /**
     * Destroys this module.
     */
    virtual ~WMPointGroupsTransform();

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
    void onFileLoad();
    /**
     * Method that is executed to save a file if the save button is pressed.
     */
    void onFileSave();

    /**
     * WDataSetPointsGrouped data input (WDataSetPoints with group parameter).
     */
    vector<boost::shared_ptr< WModuleInputData< WDataSetPointsGrouped > > > m_input;
    /**
     * WDataSetPointsGrouped data that is subtracted from the input points.
     */
    boost::shared_ptr< WModuleInputData< WDataSetPoints > > m_inputSubtraction;
    /**
     * Processed point data with cut off subtracted points.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > > m_outputGroups;
    /**
     * Data output connector for data set points output.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_outputPoints;
    /**
     * Output for outlining using voxels. Their width can be specified in settings.
     */
    boost::shared_ptr< WModuleOutputData< WTriangleMesh > > m_outputVoxels;

    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * Plugin progress status that is shared with the reader.
     */
    boost::shared_ptr< WProgress > m_progressStatus;

    /**
     * Vertices of the output point data set.
     */
    WDataSetPointsGrouped::VertexArray m_outVerts;
    /**
     * Colors of the output point data set.
     */
    WDataSetPointsGrouped::ColorArray m_outColors;
    /**
     * Colors of the output point data set in order to outline point groups.
     * Parameter field for output without group ID.
     */
    WDataSetPointsGrouped::ColorArray m_outGroupColors;
    /**
     * Group of the output point data set.
     */
    WDataSetPointsGrouped::GroupArray m_outGroups;
    /**
     * Instance to edit group IDs. It can identify groups with point count below a 
     * desireable threshold in order to remove corresponding points.
     */
    WGroupEdit m_groupEditor;


    /**
     * Wall time of the point transformation routine.
     */
    WPropDouble m_infoRenderTimeSeconds;
    /**
     * Input point count.
     */
    WPropInt m_infoInputPointCount;
    /**
     * Information about the point group count. It is the last group ID.
     */
    WPropInt m_infoLastGroupID;
    /**
     * Information about minimal X/Y/Z values.
     */
    vector<WPropInt> m_infoBoundingBoxMin;
    /**
     * Information about minimal X/Y/Z values.
     */
    vector<WPropInt> m_infoBoundingBoxMax;

    /**
     * Options that are relevant for point removal
     */
    WPropGroup m_pointsCropGroup;
    /**
     * Radius of points that are subtracted from the point set. The subtracted points 
     * are connected by the most right input.
     */
    WPropDouble m_pointSubtractionRadius;



    /**
     * Options for point group outlining using group parameter less output.
     * It can e. g. apply color etc.
     */
    WPropGroup m_outlinerGroup;
    /**
     * Determines the resolution of the smallest octree node's radius in 2^n meters
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
    WPropBool m_highlightUsingColors;
    /**
     * Switch to remove color information:
     */
    WPropBool m_clearInputColor;
    /**
     * Put all point groups or a particular group to the output.
     */
    WPropBool m_loadAllGroups;
    /**
     * If the switch is put to show a single groupe, then a group will be put out by 
     * means of this group ID.
     */
    WPropInt m_selectedShowableGroup;
    /**
     * Group transformation settings have group processing options. This field displays 
     * the original ID though. The ID is changed if more than one input is connected.
     */
    WPropInt m_groupIDInOriginalPointSet;



    /**
     * Point group processing options
     */
    WPropGroup m_pointIDProcessingGroup;
    /**
     * Group ID treatment option using more than one input.
     * It is either not changed before appling the group size threshold or group ID 
     * continue the last group ID of the previous input dataset.
     */
    WPropBool m_mergeGroupIDsAllInputs;
    /**
     * Point count threshold for groups. Groups below that point count are removed. 
     * Afterwards IDs are changed that way so no ID stays without points.
     */
    WPropInt m_groupSizeThreshold;



    /**
     * Options for point coordinate translation using an offset vector.
     */
    WPropGroup m_translatePointsGroup;
    /**
     * Coordinate translation offset applied on input points.
     */
    vector<WPropDouble> m_offsetVector;

    /**
     * File processing options.
     */
    WPropGroup m_groupFileOperations;
    /**
     * Output file path for saving.
     */
    WPropFilename m_outputFile;
    /**
     * Saves grouped points to a file.
     */
    WPropTrigger m_savePointsTrigger;
    /**
     * Input file path for loading points.
     */
    WPropFilename m_inputFile;
    /**
     * Reloads points from the input file.
     */
    WPropTrigger m_reloadPointsTrigger;




    /**
     * Instance for subtracting a point dataset from input files.
     */
    WPointSubtactionHelper m_pointSubtraction;

    /**
     * Input file processing instance.
     */
    WPointSaver m_pointInputFile;
    /**
     * Output file processing instance.
     */
    WPointSaver m_pointOutputFile;
    /**
     * Insance to outline points voxel wise.
     */
    WVoxelOutliner m_voxelOutliner;
};

#endif  // WMPOINTGROUPSTRANSFORM_H
