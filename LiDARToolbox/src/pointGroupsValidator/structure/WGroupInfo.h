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

#ifndef WGROUPINFO_H
#define WGROUPINFO_H

#include <vector>


using std::vector;

/**
 * Instance which contains information about quality of a point group.
 * 
 * What it mainly measures:
 *   - Amount of correctly segmented points.
 *   - Amount of segmented points that do not belong to the proposed group.
 *   - Area of a reference group that was not segmented by a group to be validated. Area 
 *     completeness is calculated just like completeness. But as difference a radius is 
 *     applied that is applied on correctly segmented points.
 */
class WGroupInfo
{
public:
    /**
     * Creates a point group quality info instance.
     */
    explicit WGroupInfo();

    /**
     * Destroys the group meta data set.
     */
    virtual ~WGroupInfo();

    /**
     * Returns the point count of not segmented point areas.
     * \return Point count of not segmented area points.
     */
    size_t getAreaMissingPointCount();

    /**
     * Returns the amount of correctly segmented point areas.
     * Completeness is calculated by means of Lari/Habib 2014 as folowing: 
     *     Completeness = true positives / ( true positives + false negatives )
     * \return The amount of correctly segmented point areas ( 0.0 ~ 1.0 ).
     */
    double getAreaCompleteness();

    /**
     * Returns the amount of correctly segmented points regarding the reference group 
     * points. Completeness is calculated by means of Lari/Habib 2014 as folowing: 
     *     Completeness = true positives / ( true positives + false negatives )
     * \return The amount of correctly segmented points ( 0.0 ~ 1.0 ).
     */
    double getCompletess();

    /**
     * Returns the point count of the group to be validated that was correctly segmented 
     * regarding a reference group.
     * \return Count of correctly segmented points.
     */
    size_t getCorrectlyDetectedPointCount();

    /**
     * Returns the correctness amount of group points to be validated.
     * Correctness is calculated by means of Lari/Habib 2014 as folowing: 
     *     Correctness = true positives / ( true positives + false positives ).
     * \return Correctness of the point group to be validated.
     */
    double getCorrectness();

    /**
     * Returns the reference group ID.
     * \return ID of the reference group.
     */
    size_t getReferenceGroupID();

    /**
     * Returns the group within the validated point group set that matches at most to a 
     * reference group.
     * \return Point group to be validated with the best matching point count to the 
     *         reference group.
     */
    size_t getValidatedGroupID();

    /**
     * Returns the point count of the reference group.
     * \return Point count of the reference group.
     */
    size_t getReferenceGroupPointCount();

    /**
     * Returns the point count of a group to be validated that does not belong to the 
     * reference group.
     * \return Not correctly segmented point count.
     */
    size_t getUncorrectlyDetectedPointCount();

    /**
     * Returns whether the group of this meta has been certainly detected corresponding 
     * to criterion thresholds as completeness, area completeness and correctness.
     * \return The group of the meta info has been certainly detected or not.
     */
    bool isCertainlyDetected();

    /**
     * The group has been certainly detected by the completeness of detected points or 
     * not. It is certainly detected if a sufficient amount of a reference group is 
     * covered by bhe group to be validated.
     * \return The group is certainly detected by the point completeness criterion or 
     *         not.
     */
    bool isCertainlyDetectedByCompleteness();

    /**
     * The group has been certainly detected by the point area completeness of detected 
     * points or not. It is certainly detected if not too many points of a reference 
     * group are farer apart from corectly detected points to be validated than by a 
     * threshold distance.
     * \return The group is certainly detected by the point area completeness criterion 
     *         or not.
     */
    bool isCertainlyDetectedByPointAreaCompleteness();

    /**
     * The group has been certainly detected by the correctness of detected points 
     * criterion or not. It is certainly detected if not too many points of a group to 
     * be validated exist which do not cover its reference group.
     * \return The group is certainly detected by the point correctness criterion or not.
     */
    bool isCertainlyDetectedByCorectness();


    /**
     * Sets the reference group ID.
     * \param groupID ID of the reference group.
     */
    void setReferenceGroupID( size_t groupID );

    /**
     * Sets the group within the validated point group set that matches at most to a 
     * reference group.
     * \param groupID Point group to be validated with the best matching point count to the 
     *                reference group.
     */
    void setValidatedGroupID( size_t groupID );

    /**
     * Sets the point count of the reference group.
     * \param referenceGroupPointCount Point count of the reference group.
     */
    void setReferenceGroupPointCount( size_t referenceGroupPointCount );

    /**
     * Sets the point count of the group to be validated that was correctly segmented 
     * regarding a reference group.
     * \param correctlyDetectedPointCount Count of correctly segmented points.
     */
    void setCorrectlyDetectedPointCount( size_t correctlyDetectedPointCount );

    /**
     * Sets the point count of a group to be validated that does not belong to the 
     * reference group.
     * \param uncorrectlyDetectedPointCount Not correctly segmented point count.
     */
    void setUncorrectlyDetectedPointCount( size_t uncorrectlyDetectedPointCount );

    /**
     * Sets the point count of not segmented point area points. Missing area points are 
     * marked by choosing reference points that can not be reached regarding a radius 
     * around all correctly segmented points of a validated group.
     * \param pointCountOfMissingAreas Point count of not segmented area points.
     */
    void setPointCountOfMissingAreas( size_t pointCountOfMissingAreas );

    /**
     * This setting is a threshold and not a property of the group.
     * It sets what is the minimal amount of reference group points that must be covered 
     * by points of the corresponding group to be validated.
     * \param minAmount 
     */
    void setMinimalPointCompleteness( double minAmount );

    /**
     * This setting is a threshold and not a property of the group.
     * Sets the minimal point area completeness threshold. It tells how big the amount 
     * of not caught reference group points (which are farer away from correctly 
     * detected points to be validated are farer away than by a threshold) may be in 
     * comparison to the count of points to be validated that match to reference group 
     * points. 0.0 = no match. 1.0 = all points should covered by the area.
     * \param minPoinrtAreaCompleteness Minimal amount of reference group points that 
     *                                  should be covered of correct points of the group 
     *                                  to be validated.
     */
    void setMinimalPointAreaCompleteness( double minPoinrtAreaCompleteness );

    /**
     * This setting is a threshold and not a property of the group.
     * Sets the amount how many points of the group to be validated do not need to match 
     * to their reference group. 0.0 = threshold is disabled. 1.0 = every existing point 
     * to be validated must be covered by a reference group point.
     * \param maxAmount Maximal amount of points that do not belong to the reference 
     *                  group.
     */
    void setMinimalPointCorrectness( double maxAmount );

private:
    /**
     * Reference point group ID.
     */
    size_t m_referenceGroupID;

    /**
     * ID of a group within the point dataset to be validated. It is usually the group 
     * with the best matches corresponding to the reference group.
     */
    size_t m_validatedGroupID;

    /**
     * Plane cluster ID of the point.
     */
    size_t m_referenceGroupPointCount;

    /**
     * Point count within the group to be validated that is covered by the reference 
     * group.
     */
    size_t m_correctlyDetectedPointCount;

    /**
     * Point count within the group to be validated that does not belong to the 
     * reference group.
     */
    size_t m_uncorrectlyDetectedPointCount;

    /**
     * Area of a reference group that was not segmented by a group to be validated. Area 
     * completeness is calculated just like completeness. But as difference a radius is 
     * applied that is applied on correctly segmented points.
     */
    size_t m_pointCountOfMissingAreas;

    /**
     * This setting is a threshold and not a property of the group.
     * It tells what is the minimal amount of reference group points that must be 
     * covered by points of the corresponding group to be validated.
     * 
     * According to Lari/Habib 2014:
     * Completeness = true positives / ( true positives + false negatives )
     */
    double m_minimalPointCompleteness;

    /**
     * This setting is a threshold and not a property of the group.
     * Minimal point area completeness threshold. It tells how big the amount of not 
     * caught reference group points (which are farer away from correctly detected 
     * points to be validated are farer away than by a threshold) may be in comparison 
     * to the count of points to be validated that match to reference group points. 
     * 0.0 = no match. 1.0 = all points should covered by the area.
     * 
     * According to Lari/Habib 2014:
     * Completeness = true positives / ( true positives + false negatives )
     */
    double m_minimalpointAreaCompleteness;

    /**
     * This setting is a threshold and not a property of the group.
     * Sets the amount how many points of the group to be validated do not need to match 
     * to their reference group. 0.0 = threshold is disabled. 1.0 = every existing point 
     * to be validated must be covered by a reference group point.
     * 
     * According to Lari/Habib 2014:
     * Completeness = true positives / ( true positives + false positives )
     */
    double m_minimalPointCorrectness;
};

#endif  // WGROUPINFO_H
