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
using std::numeric_limits;
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
     * Returns nearest points of a particular coordinate. Execute methods for setting 
     * examined coordinate, maximal neighbor count and radius first. Consider that the 
     * nearest point will be the input point if you take a reference point of the kd 
     * tree.
     * \return Neighbor points of a coordinate. The list is sorted ascending by the 
     *         distance.
     */
    vector<WPointDistance>* getNearestPoints();

    /**
     * Counts points within the region during regarding the maximal point count. Setting 
     * an infinite count speeds the process up. Points do would neither to be sorted nor 
     * there have to be an array to built up.
     * \return Point count within the region.
     */
    size_t getNearestNeighborCount();

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

    /**
     * Sets the maximal point count to infinite. Especially the point count within a 
     * fixed radius has a performance benefit. Points would have neither to be sorted 
     * nor there have to be an array to built up.
     */
    void setMaxResultPointCountInfinite();

protected:
    /**
     * Traverses kd-tree nodes to apply onPointFound() on points that were found using 
     * pointCanBelongToPointSet().
     * \param currentNode The current node where neighbor points are searched for.
     * \param maxDistance Maximal euclidian distance to a output point.
     */
    void traverseNodePoints( WKdTreeND* currentNode, double maxDistance );

    /**
     * Action which is executed when a point is found. This method adds points to the 
     * found points list. Overwrite this method in the inheriting class to define own 
     * functionality.
     * \param point Point that can be further processed during execution.
     */
    virtual void onPointFound( WKdPointND* point );

    /**
     * Determines whether a searched point can belong to the group of the coordinate to 
     * find neighbors of.
     * \param point Point to be tested.
     * \param maxDistance Distance limit during kd-tree search.
     * \return Point belongs to the searched point or not.
     */
    virtual bool pointCanBelongToPointSet( const vector<double>& point, double maxDistance );


    /**
     * Kd tree where nearest points are searched.
     */
    WKdTreeND* m_examinedKdTree;

    /**
     * maximal euclidian distance within which neighbors are searched.
     */
    double m_maxSearchDistance;

    /**
     * Maximal output neighbor count.
     */
    size_t m_maxResultPointCount;

    /**
     * The coordinate of which nearest points will be returned.
     */
    vector<double> m_searchedCoordinate;

    /**
     * A performance setting that has no effect on the result. Often the limited point
     * count is covered by a significantly smaller radius than the limit.
     * Searching through the nodes it firstly traverses nodes of a smaller distance. The
     * first radius is searched radius divided by pow(2, steps).
     */
    size_t m_distanceSteps;

    /**
     * Points that were found during the search.
     */
    vector<WPointDistance>* m_foundPoints;

private:
    /**
     * Returns the point count within a radius not regarding the maximal point count. It 
     * has an impact on performance because with that condition points do have neither 
     * to be sorted nor there have to be an array to built up.
     * \param currentNode Kd-tree to search.
     * \result Region point count not regarding the masimal point count.
     */
    size_t getNearestNeighborCountInfiniteMaxCount( WKdTreeND* currentNode );
};

#endif  // WPOINTSEARCHER_H
