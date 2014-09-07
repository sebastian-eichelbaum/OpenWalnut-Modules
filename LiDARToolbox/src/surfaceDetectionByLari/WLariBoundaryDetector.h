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

#ifndef WLARIBOUNDARYDETECTOR_H
#define WLARIBOUNDARYDETECTOR_H

#include <iostream>
#include <vector>

#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WParameterDomainKdPoint.h"
#include "structure/WSpatialDomainKdPoint.h"
#include "structure/WBoundaryDetectPoint.h"
#include "../common/datastructures/kdtree/WKdTreeND.h"
#include "../common/datastructures/kdtree/WKdPointND.h"
#include "../common/datastructures/kdtree/WPointSearcher.h"
#include "../common/math/vectors/WVectorMaths.h"


using std::cout;
using std::endl;
using std::numeric_limits;
using std::vector;

/**
 * Class postprocesses the surface detection that grouped points that belonged to the 
 * same planar formula. But these can be spatially disconnected. This class resolves 
 * that ambiguity.
 * 
 * How it works:
 *  A) Isolating points in sets to treat point sets only of a similar planar formula of 
 *     points in relation to their neighbors.
 *  B) Treating each cluster
 *      1) Rotating that way so that this planar normal vector is approximately parallel 
 *         to the Z axis to enable the set to be analyzed using a two dimensional 
 *         coordinate system. That point set is added to the wait list.
 *      2) While still points remain in the wait list.
 *          a) marking the most left point.
 *          b) Detecting the bound from that point counterclockwise.
 *          c) Remaining points are detected whether they are inside that bounds.
 *               - Conting how many times the bound is hit going from an outer point to 
 *                 the target point that is examined.
 *               - It follows the theory that a point enters or leaves the area after 
 *                 every bound hit. It supposes that this condition can't remain after a 
 *                 bound hit.
 *          d) Adding the bound and points inside to a new cluster.
 *          e) Removing it from the waiting list.
 */
class WLariBoundaryDetector
{
public:
    /**
     * Creates the instance to separate spatially disconnected points that are 
     * assigned to the same planar formula group.
     */
    explicit WLariBoundaryDetector();
    /**
     * Destroys the instance.
     */
    virtual ~WLariBoundaryDetector();
    /**
     * Rotates a point so that the normal vector if itsleast squares adjustment point 
     * group is approximately perpendicular to the z axis. It makes the boundary 
     * analysis on the two dimensional space possible.
     * \param transformable Point to be transformed.
     */
    void transformPoint( vector<double>* transformable );
    /**
     * Applies the algorithm on the input point data set where points have been 
     * clustered to groups that have a similar planar furmula. The algorithm splits 
     * grops that are spatially disconnected.
     * \param parameterDomain Parameter domain containing planar input points.
     */
    void detectBoundaries( WKdTreeND* parameterDomain );
    /**
     * Sets the neighbor point search distance limit:
     * \param maxPointDistanceR Neighbor search distance limit.
     */
    void setMaxPointDistanceR( double maxPointDistanceR );

private:
    /**
     * Initializes the spatial domain points. Points are grouped by the grop ID, 
     * originally groping by the same planar formula in relation to other point 
     * neighbors.
     * \param parameterDomain Parameter domain to fetch planar points..
     */
    void initSpatialDomain( WKdTreeND* parameterDomain );
    /**
     * Splits a single point group that can be spatially disconnected.
     * \param inputPointCluster Input point group to be further splitted.
     */
    void detectInputCluster( vector<WSpatialDomainKdPoint*>* inputPointCluster );
    /**
     * Inits the points rotation angles to use boundary detection on two dimensional 
     * basis. The averate normal vector of point's planar formulas in relation to its 
     * neighbors should be approximately parallel to the Z axis.
     * \param extentPointCluster Point group of which point's planar formula in relation 
     *                           to its neighbors are similar.
     */
    void initTransformationCoordinateSystem( vector<WSpatialDomainKdPoint*>* extentPointCluster );
    /**
     * Tells whether a point can reach the last added boundary point. Usually every
     * point has information about the distance to the n nearest neighbour. If one side
     * includes the other side point in respect to this distance, then these two points
     * are able to reach each other.
     * \param nextPoint Point to be checked whether it can reach the last added boundary
     *                  point.
     * \return last b oundary point can reach the point or not.
     */
    bool lastBoundaryPointCanReachPoint( WPointDistance nextPoint );
    /**
     * Calculates and returns the next point that belongs to the current bound point chain.
     * \return The next bound chain point.
     */
    WBoundaryDetectPoint* getNextBoundPoint();
    /**
     * Returns the outer counterclockwise angle of a line of three points.
     * \param previousPoint First point of the counterclockwise chain;
     * \param currentPoint Second point of the counterclockwise chain;
     * \param nextPoint Third point of the counterclockwise chain;
     * \return The outer edge angle of the current point belonging to the 
     *         counterclockwise running bound.
     */
    double getAngleToNextPoint( WBoundaryDetectPoint* previousPoint, WBoundaryDetectPoint* currentPoint, WBoundaryDetectPoint* nextPoint );
    /**
     * The algorithm follows the bound counterclockwise. There may be some difficult 
     * cases. Usually the next boundary point is taken one resulting the smallest angle 
     * (directed to the outer side) between the previous and the next point. Sometimes 
     * previous edges can have angles below 180 degrees. That results that next points 
     * can hit points that lie on the previously detected area. It results bound 
     * intersections with a completely invalit bounds. This method detects whether a 
     * potential next point could result bound intersections.
     * \param nextPoint Next point that could continue the bound chain.
     * \return Continuing the bound chain using that point can cause a bound line 
     *         intersection or not.
     */
    bool isResultingBoundIntersection( WBoundaryDetectPoint* nextPoint );
    /**
     * Calculates a bounding box out of the calculated bound chain points into member 
     * field variables.
     */
    void initAABoundingBoxFromBoundary();
    /**
     * Initializes an outer point that is a help to detect whether some points are 
     * within a bounded area or not. The principle is based on the theory that every 
     * bound hit changes the state whether a point is inside or outside the area.
     */
    void initOneOutsidePoint();
    /**
     * Calculates whether a point belongs to the bounding box of the lastly calculated 
     * cluster's bound chain. It simply speeds up when skipping the slower algorithm 
     * during a potential point is definitely too far away.
     * \param point Point to be examined whether it is in the bounding box.
     * \return The point is in the bounding box or not.
     */
    bool pointBelongsToBoundingBox( const vector<double>& point );
    /**
     * Method that accurately but more slowly determines whether a point is spatially inside the bounds 
     * of the current cluster.
     * \param point Examined point coordinate of being inside the cluster bound chain.
     * \return The point is in the cluster's bounds or not.
     */
    bool pointIsInBounds( const vector<double>& point );
    /**
     * Tells whether a point lies exactly on a bound or not.
     * \param point Point to be examined.
     * \param boundNr Bound part index to be examined.
     * \return point lies exactly on a bound line of an index or not.
     */
    bool pointLiesOnBound( const vector<double>& point, size_t boundNr );
    /**
     * Tells whether a point hits a bound of an index.
     * \param point Point to be examined.
     * \param boundNr Bound part index to be examined.
     * \return an arbitrary point hits a bound of an index or not.
     */
    bool pointHitsBound( const vector<double>& point, size_t boundNr );
    /**
     * Tells whether a bound is still valid. The validation begins after the point count 
     * of 10. It works by the principle that exact sequence of same two consecutive 
     * points means that the border runs somewhere in circle and doesn't come to the 
     * start.
     * \return Boundary chain is valid or not.
     */
    bool boundChainStillValid();


    /**
     * Spatial domain point set kd tree to be analyzed and its point group IDs modified.
     */
    WKdTreeND* m_spatialDomain;
    /**
     * Spatial domain points grouped by their group ID. Previously points are detected 
     * using the peak detection approach oof Lari/Habib. So previously points were 
     * merged with the most widespread point's planar formula in relation to it's 
     * neighbors. And that routine was executed until segmenting every last point.
     */
    vector<vector<WSpatialDomainKdPoint*>*>* m_spatialInputClusters;
    /**
     * Cluster ID counter. This field is incremented every time after detecting a new 
     * spatially connected cluster.
     */
    size_t m_currentClusterID;
    /**
     * First transformation rotation that is done so that points lie in the most optimal 
     * way to analyze them using a two dimensional coordinate system. This first 
     * rotation rotates between Z and X axis.
     */
    double m_transformAngle1zx;
    /**
     * First transformation rotation that is done so that points lie in the most optimal 
     * way to analyze them using a two dimensional coordinate system. This first 
     * rotation rotates between Z and Y axis.
     */
    double m_transformAngle2zy;
    /**
     * Boundary point chain of the current spatially connected cluster.
     */
    vector<WBoundaryDetectPoint*>* m_currentBoundary;
    /**
     * Point search instance to find points near an arbitrary coordinate.
     */
    WPointSearcher m_clusterSearcher;
    /**
     * Points above that distance aren't searched.
     */
    double m_maxPointDistanceR;
    /**
     * Lower axis aligned bounding box border of the current spatially connected bound 
     * point chain. So the border bounding box is defined by two coordinates.
     */
    vector<double> m_boundaryAABoundingBoxMin;
    /**
     * Upper axis aligned bounding box border of the current spatially connected bound 
     * point chain. So the border bounding box is defined by two coordinates.
     */
    vector<double> m_boundaryAABoundingBoxMax;
    /**
     * One outer point that helps to detect whether a point is inside a bound or not. 
     * The detection follows the theory that goint across the line the condition changes 
     * every time (whether it is inside or outside the bounds) when hitting a bound 
     * piece.
     */
    vector<double> m_oneOutsidePoint;
};

#endif  // WLARIBOUNDARYDETECTOR_H
