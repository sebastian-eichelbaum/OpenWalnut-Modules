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

#ifndef WGROUPEDIT_H
#define WGROUPEDIT_H

#include <vector>

#include "../../datastructures/WDataSetPointsGrouped.h"

using std::cout;
using std::endl;
using std::vector;

/**
 * Class to edit groups of WDataSetPointsGrouped.
 * Its main purposes are:
 *   - Merging point sets with group IDs
 *   - Helps to remove groups below a group count threshold
 *   - When merging WDataSetPointsGrouped group IDs can eigher be kept by 
 *     setMergeGroups( true ) or continuously counted adding each dataset.using 
 *     setMergeGroups( false )
 * 
 * Workflow:
 *   - initProcessBegin()
 *   - setMergeGroups()
 *   - setGroupSizeThreshold()
 *   - Execute mergeGroupSet() for at least one grouped point dataset.
 *   - modifyGroupIDs()
 *   - traverse all getInputPointCount() points. Add new vertices, colors and groups 
 *     using your own class if isPointCollected() is true for the corresponding index.
 *   - New data is getVertex(), getColor() and getNewGroupID()
 */
class WGroupEdit
{
public:
    /**
     * Creates the group edit instance
     */
    explicit WGroupEdit();
    /**
     * Destroys the group edit instance
     */
    virtual ~WGroupEdit();

    /**
     * Initializes the group edit process.
     */
    void initProocessBegin();
    /**
     * Sets whether group IDs should be kept or continuously counted up when a new 
     * grouped point set is added.
     * \param mergeGroups Choose false in order to disconnect groups from those groups 
     *                    of previous datasets. Choosing true group IDs will remain the 
     *                    same if the group size threshold is 0.
     */
    void setMergeGroups( bool mergeGroups );
    /**
     * Sets the minimal group size. Groups below that point count are signaled not to be 
     * added. Further group IDs are changed changed that way so that no group until the 
     * last one remains without points.
     * \param groupSizeThreshold Minimal group size that is proposed to be added to the 
     *                           final dataset.
     */
    void setGroupSizeThreshold( size_t groupSizeThreshold );
    /**
     * Merges points to previously added points. This method is also used tu add grouped 
     * points for the first time.
     * \param points Points with group ID to be added.
     */
    void mergeGroupSet( boost::shared_ptr< WDataSetPointsGrouped > points );




    /**
     * returns the last group ID that is generated for the new merged dataset.
     * \return Last group ID of the new merged point dataset.
     */
    size_t getLastGroupID();
    /**
     * Returns the point count of a group.
     * \param origGroupID Point group ID of the original dataset to get the point count 
     *                    of. Warning - The original group ID changes after using 
     *                    setMergeGroups( false ).
     * \return Point count of a group.
     */
    size_t getGroupSize( size_t origGroupID );
    /**
     * Returns a vertex of a corresponding point within the merged dataset.
     * Points with a group sizw below a threshold still remain using that function. Use 
     * isPointCollected() to determine whether to add a point in your code.
     * \param pointIndex Point index of the merged dataset.
     * \param dimension Dimension (X/Y/Z or 0/1/2) or the corresponding coordinate.
     * \return Value that belongs to a coordinate value X, Y or Z of a point.
     */
    double getVertex( size_t pointIndex, size_t dimension );
    /**
     * Returns a color of a corresponding point within the merged dataset.
     * Points with a group sizw below a threshold still remain using that function. Use 
     * isPointCollected() to determine whether to add a point in your code.
     * \param pointIndex Point index of the merged dataset.
     * \param colorChannel Color channel (red/green/blue or 0/1/2) or the corresponding 
     *                  coordinate.
     * \return Value that belongs to color channel of a point.
     */
    double getColor( size_t pointIndex, size_t colorChannel );
    /**
     * Returns an original group ID of a point. Group IDs are changed after the first 
     * merged dataset if setMergeGroups( false ) was applied.
     * Points with a group sizw below a threshold still remain using that function. Use 
     * isPointCollected() to determine whether to add a point in your code.
     * \param pointIndex Point index of the merged dataset.
     * \return Group ID of the original dataset.
     */
    size_t getOldGroupID( size_t pointIndex );
    /**
     * Returns a new group ID. It is further changed if the group size threshold is 
     * above 0. Further no ID remains without points.
     * Points with a group sizw below a threshold still remain using that function. Use 
     * isPointCollected() to determine whether to add a point in your code.
     * \param pointIndex Point index of the merged dataset.
     * \return New group ID.
     */
    size_t getNewGroupID( size_t pointIndex );
    /**
     * Tells whether a point is collected by means if the point group size threshold.
     * \param pointIndex Point index of the merged dataset.
     * \return Point should be added to the final dataset by means of the group size 
     *         threshold or not.
     */
    bool isPointCollected( size_t pointIndex );

    /**
     * Gets point count of all points that were entered to be merged. Group size 
     * threshold does not play a role here.
     * \return Point count without means of group point count threshold.
     */
    size_t getInputPointCount();
    /**
     * Creates an array which shows a map from old to new group IDs. Array index is the 
     * old group ID. The map has finally no groups without point using a group size 
     * threshold above 0.
     */
    void modifyGroupIDs();

private:
    /**
     * This is a method that decides whether a group ID is valid or not. Points with a 
     * group ID above an index are not regarded. Very big IDs lead to very big arrays of 
     * group sizes and group maps.
     * //TODO(aschwarzkopf): Use a hashset for groups later or something else to solve that problem.
     * \param groupID Group ID to be validated.
     * \return Group ID is valid or not.
     */
    bool isValidGroupID( size_t groupID );

    /**
     * Array that depicts group sizes. The group ID is the array index. Values are point 
     * counts of each group.
     * //TODO(aschwarzkopf): Solve that problem that big IDs lead to big arrays with 
     *                       too many unused IDs.
     */
    vector<size_t> m_groupSizes;
    /**
     * Map that links an old group ID with a new one. Old group ID is the array index. 
     * The new group ID is its value.
     * //TODO(aschwarzkopf): Solve that problem that big IDs lead to big arrays with 
     *                       too many unused IDs.
     */
    vector<size_t> m_groupIDMap;
    /**
     * Last new group ID.
     */
    size_t m_lastGroupID;

    /**
     * Input point coordinates.
     */
    WDataSetPointsGrouped::VertexArray m_vertices;
    /**
     * Colors of the input point data set that are also passed through.
     */
    WDataSetPointsGrouped::ColorArray m_colors;
    /**
     * Input point groups.
     */
    WDataSetPointsGrouped::GroupArray m_groups;

    /**
     * Determines whether group IDs should be bigger than those of previous datasets or 
     * remain. before applying the group size threshold.
     */
    bool m_mergeGroups;
    /**
     * Group point count threshold that is applied. Groups with a point count below that 
     * value are proposed to be removed. Having a threshold above 0 group IDs are 
     * modified that way so that no ID remains with a zero point count.
     */
    size_t m_groupSizeThreshold;
};

#endif  // WGROUPEDIT_H
