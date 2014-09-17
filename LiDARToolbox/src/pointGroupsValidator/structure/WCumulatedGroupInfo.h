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

#ifndef WCUMULATEDGROUPINFO_H
#define WCUMULATEDGROUPINFO_H

#include <vector>
#include "WGroupInfo.h"


using std::vector;

/**
 * Class that represents a cumulated point group info dataset. It is used to have a 
 * summerized overview over more than one group. These informations correspond to 
 * point group segmentation quality.
 * 
 * What it mainly measures:
 *   - Amount of correctly segmented points.
 *   - Amount of segmented points that do not belong to the proposed group.
 *   - Area of a reference group that was segmented by a group to be validated. Area 
 *     completeness is calculated just like completeness. But as difference a radius is 
 *     applied that is applied on correctly segmented points.
 */
class WCumulatedGroupInfo
{
public:
    /**
     * Creates an instance of a cumulated point dataset.
     */
    explicit WCumulatedGroupInfo();
    /**
     * Destroys an instance of a cumulated point dataset.
     */
    virtual ~WCumulatedGroupInfo();
    /**
     * Point group can be within ghe point group or not.
     * The cumulated group has the info about minimal and maximal point count. Groups 
     * are proposed to be cumulated here if their reference group point is within that 
     * boundries.
     * \param group Group info to be tested.
     * \return Group can be cumulated or not
     */
    bool canBeCumulated( WGroupInfo* group );
    /**
     * Cumulates a point group to this instance.
     * \param group Group results that should be cumulated.
     * \return Group could be cumulated or not.
     */
    bool cumulateGroup( WGroupInfo* group );
    /**
     * Returns the count of groups that were cumulated inside that instance.
     * \return Cumulated group count.
     */
    size_t getGroupCount();
    /**
     * Returns the point segmentation correctness by means of Lari/Habib 2014:
     *     Correctness = true positives / ( true positives + false positives ).
     * \return Average point segmentation correctness ( 0.0 ~ 1.0 ).
     */
    double getPointCorrectness();
    /**
     * Returns the point segmentation completeness by means of Lari/Habib 2014:
     *     Completeness = true positives / ( true positives + false negatives )
     * \return Average point segmentation completeness ( 0.0 ~ 1.0 ).
     */
    double getPointCompleteness();
    /**
     * Returns the point area segmentation completeness by means of Lari/Habib 2014:
     *     Completeness = true positives / ( true positives + false negatives )
     * 
     * Area of a reference group that was not segmented by a group to be validated. Area 
     * completeness is calculated just like completeness. But as difference a radius is 
     * applied that is applied on correctly segmented points.
     * \return Average point area segmentation completeness ( 0.0 ~ 1.0 ).
     */
    double getAreaPointCorrectness();
    /**
     * Returns the minimal reference group point count to cumulate groups here.
     * \return Minimal reference group point count to cumulate groups here.
     */
    size_t getRangeMinPointCount();
    /**
     * Returns the maximal reference group point count to cumulate groups here.
     * \return Maximal reference group point count to cumulate groups here.
     */
    size_t getRangeMaxPointCount();
    /**
     * Tells whether groups were cumulated inside that instance or not.
     * \return Groups were cumulated inside that instance or not.
     */
    bool hasGroups();
    /**
     * Sets the minimal and maximal reference group point count. Groups wiith a 
     * reference group point count are not cumulated inside the instance.
     * \param minPointCount Minimal reference group point count.
     * \param maxPointCount Maximal reference group point count.
     */
    void setPointCountRange( size_t minPointCount, size_t maxPointCount );

private:
    /**
     * Count of groups that were cumulated inside that instance.
     */
    size_t m_cumulatedGroupCount;
    /**
     * Point segmentation correctness by means of Lari/Habib 2014:
     *     Correctness = true positives / ( true positives + false positives ).
     * 
     * Divide that value by the group count to get the result.
     */
    double m_pointCorrectnessCumulated;
    /**
     * Point segmentation completeness by means of Lari/Habib 2014:
     *     Completeness = true positives / ( true positives + false negatives )
     * 
     * Divide that value by the group count to get the result.
     */
    double m_pointCompletenessCumulated;
    /**
     * Area of a reference group that was segmented by a group to be validated. Area 
     * completeness is calculated just like completeness. But as difference a radius is 
     * applied that is applied on correctly segmented points.
     */
    double m_areaPointCompletenessCumulated;
    /**
     * Minimal reference group point count to cumulate groups here.
     */
    size_t m_groupsRangeMinPointCount;
    /**
     * Maximal reference group point count to cumulate groups here.
     */
    size_t m_groupsRangeMaxPointCount;
};

#endif  // WCUMULATEDGROUPINFO_H
