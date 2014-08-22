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

#ifndef WPOINTSEARCHER_H
#define WPOINTSEARCHER_H

#include <iostream>
#include <vector>
#include "WPointDistance.h"
#include "WKdTreeND.h"
#include "WKdPointND.h"
#include "core/common/math/linearAlgebra/WPosition.h"

using std::cout;
using std::endl;
using std::vector;

/**
 * Instance that searchs for particular points within a kd tree. Currently it looks for 
 * ones within a radius and a maximal point count. This type is proposed to be suitable 
 * for unidimensional coordinate systems.
 */
class WPointSearcher
{
public:
    explicit WPointSearcher();
    /**
     * Instantiates the points searcher.
     * \param kdTree Assigned source kd tree to search points.
     */
    explicit WPointSearcher( WKdTreeND* kdTree );
    /**
     * Destroys the points searcher
     */
    virtual ~WPointSearcher();
    /**
     * Static method that fetches coordinates of WPointDistance sets into a three 
     * dimensional WPosition point list.
     * \param pointDistances Point distance data sets to fetch positions from.
     * \return A WPosition list. That type is very commonly used in OpenWalnut.
     */
    static vector<WPosition>* convertToPointSet( vector<WPointDistance>* pointDistances );
    /**
     * Returns nearest points of a particular coordinate. Execute methods for setting 
     * examined coordinate, maximal neighbor count and radius first. Consider that the 
     * nearest point will be the input point if you take a reference point of the kd 
     * tree.
     * \return Neighbor points of a coordinate. The list is sorted ascending by the 
     *         distance.
     */
    vector<WPointDistance>* getNearestPoints();
    /**
     * Links a kd tree to the search engine in order to find nearest points of a 
     * coordinate.
     * \param kdTree The kd tree where to look for neighbors.
     */
    void setExaminedKdTree( WKdTreeND* kdTree );
    /**
     * Sets the coordinate of the point to get its neighbors afterwarts.
     * \param searchedPoint Coordinate to search for neighbors.
     */
    void setSearchedPoint( const vector<double>& searchedPoint );
    /**
     * Sets the maximal distance of neighbors.
     * \param distance The maximal distance of neighbors.
     */
    void setMaxSearchDistance( double distance );
    /**
     * Sets the maximal point count.
     * \param maxPointCount The maximal neighbor count that is found for a coordinate.
     */
    void setMaxResultPointCount( size_t maxPointCount );

private:
    /**
     * Kd tree where nearest points are searched.
     */
    WKdTreeND* m_examinedKdTree;
    /**
     * The coordinate of which nearest points will be returned.
     */
    vector<double> m_searchedCoordinate;
    /**
     * maximal euclidian distance within which neighbors are searched.
     */
    double m_maxSearchDistance;
    /**
     * Maximal output neighbor count.
     */
    size_t m_maxResultPointCount;
    /**
     * A performance setting that has no effect on the result. Often the limited point
     * count is covered by a significantly smaller radius than the limit.
     * Searching through the nodes it firstly traverses nodes of a smaller distance. The
     * first radius is searched radius divided by pow(2, steps).
     */
    size_t m_distanceSteps;
    /**
     * Fetches the nearest points into a point list using a kd tree node. It doesn't 
     * consider the maximal point count.
     * \param currentNode The current node where neighbor points are searched for.
     * \param targetPoints The target list where points are put to.
     * \param maxDistance Maximal euclidian distance to a output point.
     */
    void fetchNearestPoints( WKdTreeND* currentNode, vector<WPointDistance>* targetPoints, double maxDistance );
};

#endif  // WPOINTSEARCHER_H
