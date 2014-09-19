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

#ifndef WGROUPVALIDATOR_H
#define WGROUPVALIDATOR_H

#include <vector>
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/algorithms/groupEdit/WGroupEdit.h"
#include "structure/WGroupInfo.h"
#include "../common/datastructures/WDataSetPointsGrouped.h"
#include "../common/algorithms/pointSubtractionHelper/WPointSubtactionHelper.h"


using std::vector;


/**
 * Validates segmented groups using a reference point group set. Refrence groups may be 
 * created by hand.
 */
class WGroupValidator
{
public:
    /**
     * Creates the point group validator instance.
     */
    explicit WGroupValidator();

    /**
     * Destroys the point group validator instance.
     */
    virtual ~WGroupValidator();



    /**
     * Returns the outline of reference points that were not detected by groups to be 
     * validated at all.
     * \return Outline of not segmented points to be validated:
     */
    boost::shared_ptr< WDataSetPoints > getOutlineNotSegmentedPoints();

    /**
     * Returns the outline of points of groups to be validated that were assigned to a 
     * wrong group to be validated.
     * \return Points segmented out of the correct reference group.
     */
    boost::shared_ptr< WDataSetPoints > getOutlinePointsBelongingForeignGroups();

    /**
     * Returns points that show area that is not segmented by the group to be validated.
     * Missing areas are similarly calculated like reference points not segmented by 
     * groups to be validated. But as difference a radius was applied on points of 
     * groups to be validated to unmark segmented area points.
     * \return Points that depict not segmented areas.
     */
    boost::shared_ptr< WDataSetPoints > getOutlineNotSegmentedGroupAreas();

    /**
     * Returns the generated group information that tells about quality of groups to be 
     * validated.
     * \return Information of groups to be validated.
     */
    vector<WGroupInfo*>* getGroupInfo();

    /**
     * Sets  the coordinate accuracy. Coordinate differences between reference and 
     * validated points by that euclidean distance are regarded.
     * \param maxEuclideanDeviance Tolerated coordinate euclidean deviance.
     */
    void setCoordinateAccuracy( double maxEuclideanDeviance );

    /**
     * Radius contraint for point area testing. E. g. correctly segmented area reference 
     * points are segmented by searching them using validated group point coordinates by 
     * the set radius. So it can be distinguished between sporadeously not segmented 
     * points and missing areas.
     * \param areaTestingPointRadius Area point search radius along from group points to 
     *                               be validated.
     */
    void setAreaTestingPointRadius( double areaTestingPointRadius );

    /**
     * Starts to validate groups using a reference point group set.
     * \param referenceGroups REference group set that is probably a hand segmented.
     * \param validatedGroups Groups that are segmented by an algorithm to be evaluated.
     */
    void validateGroups( boost::shared_ptr< WDataSetPointsGrouped > referenceGroups,
            boost::shared_ptr< WDataSetPointsGrouped > validatedGroups );




private:
    /**
     * Validates a single group using a reference point group set.
     * \param referenceGroupID Reference group to be verefied for completeness.
     * \param validatedGroupID Best matching group of the reference group to be 
     *                         evaluated.
     */
    void validateGroup( size_t referenceGroupID, size_t validatedGroupID );

    /**
     * Identifies points of reference groups that were not detected by groups to be 
     * validated. Validated groups are left out if they were not fit to a reference 
     * group.
     */
    void identifyNotSegmentedGroupPoints();

    /**
     * Returns an ID of a group to be evaluated that matches at best to a reference 
     * group.
     * \param referenceGroup Reference group ID to find a group to be validated for.
     * \return ID of the corresponding group to be evaluated.
     */
    size_t getBestMatchingGroupID( boost::shared_ptr< WDataSetPoints > referenceGroup );

    /**
     * Returns all points of any entire group.
     * \param groupedPoints Point group set to where to look for a group
     * \param groupID Desired point group ID.
     * \return desired group points.
     */
    static boost::shared_ptr< WDataSetPoints > getPointsOfGroup(
            boost::shared_ptr< WDataSetPointsGrouped > groupedPoints, size_t groupID );

    /**
     * We still MUST create point sets wit at least one point. Complete OpenWalnut 
     * crashes creating a point set with no points.
     * 
     * There is a disability built in method void WDataSetPoints::init( bool calcBB )
     * 
     * Problematic line:
     * m_colorType = static_cast< ColorType >( m_colors->size() / ( m_vertices->size() / 3 ) );
     * \return Smallest creatable point set.
     */
    static boost::shared_ptr< WDataSetPoints > getEmptyShowablePointSet();


    /**
     * Coordinate accuracy. Coordinate differences between reference and validated 
     * points by that euclidean distance are regarded.
     */
    double m_coordinateAccuracy;

    /**
     * Radius contraint for point area testing. E. g. correctly segmented area reference 
     * points are segmented by searching them using validated group point coordinates by 
     * the set radius. So it can be distinguished between sporadeously not segmented 
     * points and missing areas.
     */
    double m_areaTestingPointRadius;

    /**
     * Information about groups to be evaluated that is generated in that class.
     */
    vector<WGroupInfo*>* m_groupInfo;

    /**
     * Group editor instance for reference points. It is used to evaluate point count 
     * information.
     */
    WGroupEdit m_referenceGroupEditor;

    /**
     * Grouped dataset points of the reference dataset.
     */
    boost::shared_ptr< WDataSetPointsGrouped > m_referencePoints;

    /**
     * Vertices of reference grouped points.
     */
    WDataSetPointsGrouped::VertexArray m_referenceVertices;

    /**
     * Colors of reference grouped points.
     */
    WDataSetPointsGrouped::ColorArray m_referenceColors;

    /**
     * Groups of reference grouped points.
     */
    WDataSetPointsGrouped::GroupArray m_referenceGroups;

    /**
     * Grouped dataset points of the group to be validated.
     */
    boost::shared_ptr< WDataSetPointsGrouped > m_validatedPoints;

    /**
     * Vertices of reference grouped points.
     */
    WDataSetPointsGrouped::VertexArray m_validatedVertices;

    /**
     * Colors of reference grouped points.
     */
    WDataSetPointsGrouped::ColorArray m_validatedColors;

    /**
     * Groups of reference grouped points.
     */
    WDataSetPointsGrouped::GroupArray m_validatedGroups;


    /**
     * Vertices of points to be validated that were segmented corresponding to reference 
     * groups.
     */
    WDataSetPoints::VertexArray m_correctlySegmentedVertices;

    /**
     * Colors of points to be validated that were segmented corresponding to reference 
     * groups.
     */
    WDataSetPoints::ColorArray m_correctlySegmentedColors;

    /**
     * Vertiices of points to be validated that were not segmented to the best matching 
     * reference group.
     */
    WDataSetPoints::VertexArray m_falseSegmentedVertices;

    /**
     * Colors of points to be validated that were not segmented to the best matching 
     * reference group.
     */
    WDataSetPoints::ColorArray m_falseSegmentedColors;

    /**
     * Points of reference groups that were not by correctly detected groups to be 
     * validated. Groups do not count here if they were not matched correctly to a 
     * reference group.
     */
    WDataSetPoints::VertexArray m_notSegmentedPointsVertices;

    /**
     * Colors of reference groups that were not by correctly detected groups to be 
     * validated. Groups do not count here if they were not matched correctly to a 
     * reference group.
     */
    WDataSetPoints::ColorArray m_notSegmentedPointsColors;

    /**
     * Points of areas not detected reference groups. They are found by finding 
     * reference group points that can not be reached by a radius around correctly 
     * segmented points.
     */
    WDataSetPoints::VertexArray m_pointsOfNotSegmentedAreasVertices;

    /**
     * Colors of Points of areas not detected reference groups. They are found by 
     * finding reference group points that can not be reached by a radius around 
     * correctly segmented points.
     */
    WDataSetPoints::ColorArray m_pointsOfNotSegmentedAreasColors;
};

#endif  // WGROUPVALIDATOR_H
