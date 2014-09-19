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


#ifndef WWALLDETECTOCTNODE_H
#define WWALLDETECTOCTNODE_H
#include <vector>
#include "../../common/datastructures/octree/WOctNode.h"
#include "core/common/math/linearAlgebra/WPosition.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"

using std::vector;

/**
 * This Octree node voxels help to detect walls and other connected surfaces when using 
 * as leaf nodes. At first The PCA analysis is fired in order to calculate a normal 
 * vector of a surface. Then it groups all neighbor voxels that don't differ by the 
 * normal vector very much.
 */
class WWallDetectOctNode : public WOctNode
{
public:
    /**
     * Initialiizes the wall detection voxel.
     */
    WWallDetectOctNode();

    /**
     * Initializes the wall detection voxel also filling its space coverage 
     * parameters.
     * \param centerX X coordinate of the octree node center.
     * \param centerY Y coordinate of the octree node center.
     * \param centerZ Z coordinate of the octree node center.
     * \param radius Range from the center point that the node covers in each X/Y/Z 
     *               direction.
     */
    WWallDetectOctNode( double centerX, double centerY, double centerZ, double radius );

    /**
     * Destroys the wall detection voxel.
     */
    virtual ~WWallDetectOctNode();

    /**
     * The octree instance uses this method to instantiate a new node of the same 
     * class.
     * \param centerX X coordinate of the octree node center.
     * \param centerY Y coordinate of the octree node center.
     * \param centerZ Z coordinate of the octree node center.
     * \param radius Range from the center point that the node covers in each X/Y/Z 
     *               direction.
     * \return New node instance of the same class.
     */
    virtual WOctNode* newInstance( double centerX, double centerY, double centerZ, double radius );

    /**
     * Adds a coordinate to the node for the PCA analysis.
     * That method is executed when touchuing a coordinate.
     * \param x X coordinate of the new point.
     * \param y Y coordinate of the new point.
     * \param z Z coordinate of the new point.
     */
    virtual void onTouchPosition( double x, double y, double z );

    /**
     * Returns the mean coordinate of all input points.
     * \return The mean coordinate of all input points.
     */
    WPosition getMean();

    /**
     * Sets the mean coordinate of all input points.
     * \param mean The mean coordinate of all input points.
     */
    void setMean( WPosition mean );

    /**
     * Returns the input points covered by the node.
     * \return Input point data covered by the node.
     */
    vector<WPosition>* getInputPoints();

    /**
     * Returns the normal vector of the node. It points to the least point 
     * distributed direction.
     * \return The surface normal vector of the node.
     */
    WVector3d getNormalVector();

    /**
     * Returns the Eigen Vector of the biggest Eigen Value. It's usually the most 
     * spread direction of input points.
     * \return The Eigen Vector of the biggest Eigen Value.
     */
    WVector3d getStrongestEigenVector();

    /**
     * Returns a particular Eigen Vector. Its index corresponts to getEigenValues().
     * The items are sorted descending by the Eigen Value.
     * \param index Index of the Eigen Vector.
     * \return Eigen Vector of an index.
     */
    WVector3d getEigenVector( size_t index );

    /**
     * Sets the Eigen Vectors for the node. The index of the vectors must correspond
     * to setEigenValues().
     * \param eigenVectors Eigen Vectors to set.
     */
    void setEigenVectors( vector<WVector3d> eigenVectors );

    /**
     * Returns the linear level of an octree voxel. It's an quotient of the second 
     * Eigen Value over the biggest one. More linear point sets have a smaller 
     * quotient.
     * \return The linear level of this node.
     */
    double getLinearLevel();

    /**
     * Returns the smallest point distribution direction strength divided by the 
     * strongest. Very small values indicate a surface.
     * \return The isotropic threshold of the node.
     */
    double getIsotropicLevel();

    /**
     * Returns the Eigen Values of the input point set. They are sorting descending. 
     * Their index corresponds to getEigenVector(size_t index). The Eigen Value is 
     * the point distribution strength within a Eigen Vector's direction.
     * return Eigen Values of the input point set.
     * \return Eigen Values of the node.
     */
    vector<double> getEigenValues();

    /**
     * Sets the eigen values for the node. The Index of that values must correspond
     * to setEigenVectors().
     * \param eigenValues Eigen Values to set.
     */
    void setEigenValues( vector<double> eigenValues );

    /**
     * Tells whether a node has statistical information as Eigen Vectors and Eigen
     * Values Usually the Principal Component Analysis fails on point sets below a 
     * count of three.
     * \return The node has Eigen Values and Eigen Vectors or not.
     */
    bool hasEigenValuesAndVectors();

    /**
     * Clears the input point data. It mainly frees space after the Principal
     * Component Analysis.
     */
    void clearInputData();

private:
    /**
     * Input points covered by the space of the node.
     */
    vector<WPosition>* m_inputPoints;

    /**
     * The mean coordinate of all input points.
     */
    WPosition m_mean;

    /**
     * The smallest point distribution direction strength divided by the most 
     * strongest. Very small values indicate a surface.
     */
    vector<double> m_eigenValues;

    /**
     * The weakest point distribution direction. It's meant to show a surface normal 
     * vector.
     */
    vector<WVector3d> m_eigenVectors;
};
#endif  // WWALLDETECTOCTNODE_H
