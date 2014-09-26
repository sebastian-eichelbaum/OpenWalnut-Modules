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

#ifndef WMPOINTGROUPSVALIDATOR_H
#define WMPOINTGROUPSVALIDATOR_H


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
#include "../surfaceDetectionByLari/structure/WSpatialDomainKdPoint.h"
#include "../common/math/vectors/WVectorMaths.h"
#include "WGroupValidator.h"
#include "WExportCSV.h"


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



using std::vector;

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;

/**
 * Select a point group in order to display it as a triangle mesh outline or data set points. Each 
 * group can be displayed in a single color depicting all groups..
 * \ingroup modules
 */
class WMPointGroupsValidator: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMPointGroupsValidator();

    /**
     * Destroys this module.
     */
    virtual ~WMPointGroupsValidator();

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
     * Groups that depict an ideal segmentation result. This point set comes ofteh from 
     * manual editing or assigning.
     */
    boost::shared_ptr< WModuleInputData< WDataSetPointsGrouped > > m_inputReferenceGroupPoints;

    /**
     * Groups that should be validated. Usually this point sets arases from a 
     * segmentation process that comes from a module that should be validated.
     */
    boost::shared_ptr< WModuleInputData< WDataSetPointsGrouped > > m_inputValidatedGroupPoints;

    /**
     * Points from reference groups that are not segmented in a point set to be 
     * validated.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_outputNotSegmented;

    /**
     * Usually not segmented points maybe tell nothing about the area coverage itself. 
     * Points wihtin that are also points that were not detected in the group. But they 
     * are added here only if they are far enough apart from detected points to be 
     * validated (using a distance threshold).
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_outputPointsOfMissingAreas;

    /**
     * Points of reference groups that have a wrong group in the point group dataset to 
     * be validated.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_outputFalseSegmented;


    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;



    /**
     * Plugin progress status that is shared with the reader.
     */
    boost::shared_ptr< WProgress > m_progressStatus;

    /**
     * Settings group that deals with coordinate accuracy of points to be evaluated.
     */
    WPropGroup m_accuracyGroup;

    /**
     * Regarded euclidean deviance between reference points and points to be evaluated.
     */
    WPropDouble m_coordinateAccuracy;

    /**
     * Setting parameter group that is relevant to thresholds which determine which 
     * minimal point completeness, point area completeness and point correctness is 
     * necessary to classify a group as certainly detected.
     */
    WPropGroup m_detectionCertaintyGroup;

    /**
     * Setting that delas with finding not segmented area points. These points are found 
     * by taking points of a corresponding reference group that can not be reached by 
     * correctly segmented points regarding this euclideanradius around them.
     */
    WPropDouble m_pointAreaRadius;

    /**
     * Point completeness threshold that is related to detect whether a point group is 
     * certainly detected or not.
     * It relies on the following formula of Lari/Habib 2014.
     * Completeness = true positives / ( true positives + false negatives )
     * 
     * In the implementation:
     * True positives: Reference group count that matches to points of the group to ge 
     *                 validated.
     * False negatives: Point count of the reference group that are not covered by the 
     *                  group to be validated.
     */
    WPropDouble m_minimalPointCompleteness;

    /**
     * Point area completeness threshold that is related to detect whether a point group 
     * is certainly detected or not.
     * It relies on the following formula of Lari/Habib 2014.
     * Completeness = true positives / ( true positives + false negatives )
     * 
     * In the implementation:
     * True positives: Reference group count that matches to points of the group to ge 
     *                 validated.
     * False negatives: Count of points of the reference group that is farer away from 
     *                  all points of the group to be validated than by a threshold 
     *                  distance.
     */
    WPropDouble m_minimalAreaPointCompleteness;

    /**
     * Point segmentation correctness threshold that is related to detect whether a 
     * point group is certainly detected or not.
     * It relies on the following formula of Lari/Habib 2014.
     * Completeness = true positives / ( true positives + false negatives )
     * 
     * In the implementation:
     * True positives: Reference group count that matches to points of the group to ge 
     *                 validated.
     * False positives: Count of points of the group to be validated that arer not 
     *                  covered by the reference group..
     */
    WPropDouble m_minimalPointCorectness;


    /**
     * Settings that deal with point group evaluation itself.
     */
    WPropGroup m_evaluationGroup;


    /**
     * Cumulate groups with similar reference group point counts.
     */
    WPropBool m_cumulateGroups;

    /**
     * Result comma separated values table output file path.
     */
    WPropFilename m_outputFileCSV;

    /**
     * Result comma separated values table export launcher.
     */
    WPropTrigger m_saveCSVTrigger;

    /**
     * Group validator instance
     */
    WGroupValidator m_groupValidator;

    /**
     * Instance that exports results to a comma separated values table output file.
     */
    WExportCSV m_exportCSV;
};

#endif  // WMPOINTGROUPSVALIDATOR_H
