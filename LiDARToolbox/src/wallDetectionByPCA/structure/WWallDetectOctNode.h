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
#include "../../datastructures/octree/WOctNode.h"
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
     * Initializes the wall detection voxel also filling its space coverage parameters.
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
     * The octree instance uses this method to instantiate a new node of the same class.
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
     * Returns the input points covered by the node.
     * \return Input point data covered by the node.
     */
    vector<WPosition> getInputPoints();
    /**
     * Returns the normal vector of the node. It points to the least point distributed 
     * direction.
     * \return The surface normal vector of the node.
     */
    WVector3d getNormalVector();
    /**
     * Sets the normal vector of the normal vector of the node. It points to the least 
     * point distributed direction.
     * \param normalVector The surface normal vector of the node.
     */
    void setNormalVector( WVector3d normalVector );
    /**
     * Returns the smallest point distribution direction strength divided by the 
     * strongest. Very small values indicate a surface.
     * \return The isotropic threshold of the node.
     */
    double getIsotropicThreshold();
    /**
     * Sets the smallest point distribution direction strength divided by the most 
     * strongest. Very small values indicate a surface.
     * \param isotropicThreshold The node's isotropic threshold (0.0 to 1.0).
     */
    void setIsotropicThreshold( double isotropicThreshold );

private:
    /**
     * Input points covered by the space of the node.
     */
    vector<WPosition> m_inputPoints;
    /**
     * The smallest point distribution direction strength divided by the most strongest.
     * Very small values indicate a surface.
     */
    double m_isotropicThreshold;
    /**
     * The weakest point distribution direction. It's meant to show a surface normal 
     * vector.
     */
    WVector3d m_normalVector;
};
#endif  // WWALLDETECTOCTNODE_H
