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

#ifndef WSPATIALDOMAINKDPOINT_H
#define WSPATIALDOMAINKDPOINT_H

#include <vector>
#include "../../common/datastructures/kdtree/WKdPointND.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"
#include "../../common/math/leastSquares/WLeastSquares.h"

/**
 * Point information container that is used for surface detection approach of Lari/Habib.
 */
class WSpatialDomainKdPoint : public WKdPointND
{
public:
    /**
     * Instantiates the point using an n dimensional coordinate.
     * \param coordinate An n dimensional coordinate.
     */
    explicit WSpatialDomainKdPoint( vector<double> coordinate );
    /**
     * Instantiates the point using a three dimensional coordinate.
     * \param x X coordinate.
     * \param y Y coordinate.
     * \param z Z coordinate.
     */
    explicit WSpatialDomainKdPoint( double x, double y, double z );
    /**
     * Destroys the point meta data set.
     */
    virtual ~WSpatialDomainKdPoint();
    /**
     * Returns the eigen value of the point in relation to its neighbor points.
     * \return Point's Eigen Values in relation to its neighbor points.
     */
    vector<double> getEigenValues();
    /**
     * Returns the plane cluster ID of the point.
     * \return Plane cluster ID of the point.
     */
    size_t getClusterID();
    /**
     * Returns the plane formula for the best fitted plane.
     * \return The hessian Normal Form of the best fitted plane. First n numbers (by 
     *         the dimensions coordinate count) represent the normal vector of the best 
     *         fitted plane. The last one is the perpendicular euclidian distance to the 
     *         coordinate system orign.
     */
    vector<double> getHessianNormalForm();
    /**
     * Returns the distance to the n-th nearest neighbor.
     * \return The distance to the n-th nearest neighbor.
     */
    double getDistanceToNthNearestNeighbor();
    /**
     * Gets the input point array index of this point's coordinate.
     * \return Index within the input point array of this point.
     */
    size_t getIndexInInputArray();
    /**
     * Returns the local point density index. It corresponds to the point count per 
     * square meter. It is calculated the k nearest point count and the n-th nearest 
     * point distance
     * \return Local point density index.
     */
    double getLocalPointDensity();
    /**
     * Returns the parameter domain coordinates of the point.
     * \return The parameter domain coordinates of the point.
     */
    vector<double> getParametersXYZ0();
    /**
     * Tells whether planar parameters are valid.
     * \return Has valid parameters or not.
     */
    bool hasValidParameters();
    /**
     * Sets the eigens values of the point in relation to its neighbors.
     * \param eigenValues The eigen values of a poinnt in relation to its neighbors.
     */
    void setEigenValues( vector<double> eigenValues );
    /**
     * Sets the eigens vectors of the point in relation to its neighbors.
     * \param eigenVectors The eigen vectors of a poinnt in relation to its neighbors.
     */
    void setEigenVectors( vector<WVector3d> eigenVectors );
    /**
     * Sets the Hessian normal form of the point's best fitted plane in relation to its 
     * neighbors.
     * \param hessianNormalForm The Hessian normal formula of the point's best 
     *                            fitted plane.
     */
    void setHessianNormalForm( vector<double> hessianNormalForm );
    /**
     * Sets the plane cluster ID to the point.
     * \param clusterID Plane cluster ID of the point.
     */
    void setClusterID( size_t clusterID );
    /**
     * Sets the count of k nearest point count.
     * \param kNearestPoints K nearest points.
     */
    void setKNearestPoints( size_t kNearestPoints );
    /**
     * Sets the distance to the n-th nearest point.
     * \param distanceToNthNearestNeighbor Distance to n-th nearest neighbor.
     */
    void setDistanceToNthNearestNeighbor( double distanceToNthNearestNeighbor );
    /**
     * Sets the input point array index of this point's coordinate.
     * \param indexInInputArray Index within the input point array of this point.
     */
    void setIndexInInputArray( size_t indexInInputArray );

private:
    /**
     * The eigen values of a poinnt in relation to its neighbors.
     */
    vector<double> m_eigenValues;
    /**
     * The eigen vectors of a poinnt in relation to its neighbors.
     */
    vector<WVector3d> m_eigenVectors;
    /**
     * Space for the calculated Hessian Normal Form.
     */
    vector<double> m_hessianNormalForm;
    /**
     * Plane cluster ID of the point.
     */
    size_t m_clusterID;
    /**
     * Count of k nearest neighbors.
     */
    size_t m_kNearestPoints;
    /**
     * Distance to the n-th nearest neighbor.
     */
    double m_distanceToNthNearestNeighbor;
    /**
     * Index within the input point array of this point.
     */
    size_t m_indexInInputArray;
};

#endif  // WSPATIALDOMAINKDPOINT_H
