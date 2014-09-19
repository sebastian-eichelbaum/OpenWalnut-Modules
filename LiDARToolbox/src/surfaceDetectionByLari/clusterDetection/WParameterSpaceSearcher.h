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

#ifndef WPARAMETERSPACESEARCHER_H
#define WPARAMETERSPACESEARCHER_H

#include <iostream>
#include <vector>
#include "../../common/datastructures/kdtree/WKdPointND.h"
#include "../../common/datastructures/kdtree/WPointSearcher.h"
#include "core/common/math/linearAlgebra/WPosition.h"

using std::cout;
using std::endl;
using std::vector;

/**
 * Instance that searchs for particular points within a kd tree. Currently it looks for 
 * ones within a radius and a maximal point count. This type is proposed to be suitable 
 * for unidimensional coordinate systems.
 */
class WParameterSpaceSearcher : public WPointSearcher
{
public:
    explicit WParameterSpaceSearcher();

    /**
     * Destroys the points searcher
     */
    virtual ~WParameterSpaceSearcher();

    /**
     * Tags kd tree points within the extent with searched point as peak center to be refreshed.
     */
    void tagExtentToRefresh();

    /**
     * Sets the main segmentation settings of the segmentation algorithm. They regard 
     * the planar formula of each spatial point in relation to its 
     * neighborship.
     * \param maxAngleDegrees Maximal angular deviation of plane formulas between two 
     *                        points.
     * \param planeDistance Maximal difference between two plane perpendicular distances 
     *                      to the origin.
     */
    void setSegmentationSettings( double maxAngleDegrees, double planeDistance );

    /**
     * Sets the peak center. It determines the region of searched coplanar parameters.
     * \param peakCenter Peak center to search coplana parameters which belong to it.
     */
    void setSearchedPeakCenter( const vector<double>& peakCenter );

protected:
    /**
     * Method that is executed during points are found. The method either adds points to 
     * the found point list or tags points to be refreshed that belong to an extent with 
     * the searched point as peak centre. Refreshed points are reassigned a new count of 
     * their extent point count if they would represent a peak centre.
     * \param point Peak centre whose extent points should be tagged to be refreshed.
     */
    virtual void onPointFound( WKdPointND* point );

    /**
     * Tells whether a point can belong to the extent of the searched coordinate as peak 
     * centre. This method overwrites the search method of the inherited class.
     * \param point Point to be tested.
     * \param maxDistance Distance limit during kd-tree search.
     * \return Point belongs to the extent with the current searched point as peak 
     *         centre or not.
     */
    virtual bool pointCanBelongToPointSet( const vector<double>& point, double maxDistance );


private:
    /**
     * Returns the masimal euclidian distance within an extent from the peak center in 
     * the parameter domain kd tree. It is a spherical bounding box concept.
     * \param parametersXYZ0 Parameter domain coordinate from which the maximal 
     *                       euclidian distance to the farest extent point is determined.
     * \return The maximal euclidian distance from the peak center to the farest 
     *         belonging parameter to the plane within the parameter domain.
     */
    double getMaxParameterDistance( const vector<double>& parametersXYZ0 );

    /**
     * Tells wehther two parameters can belong to the same extent or can belong to the 
     * same plane.
     * \param parameters1 First parameter to check.
     * \param parameters2 Second parameter to check.
     * \return parameters can belont to the same extent or not.
     */
    bool isParameterOfSameExtent( const vector<double>& parameters1, const vector<double>& parameters2 );

    /**
     * Setting that regards the planar formula of each spatial point in relation to its 
     * neighborship. This variable is the maximal angular deviation of plane formulas 
     * between two points.
     */
    double m_segmentationMaxAngleDegrees;

    /**
     * Setting that regards the planar formula of each spatial point in relation to its 
     * neighborship. This variable is the maximal difference between two plane normal 
     * distance to the origin.
     */
    double m_segmentationMaxPlaneDistance;

    /**
     * Tag points to be refreshed instead of searching them
     */
    bool m_tagToRefresh;
};

#endif  // WPARAMETERSPACESEARCHER_H
