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

#ifndef WWALLDETECTOCTREE_H
#define WWALLDETECTOCTREE_H

#include <vector>
#include "../../datastructures/octree/WOctree.h"
#include "WWallDetectOctNode.h"

using std::cout;
using std::endl;

/**
 * The Octree extension class that groups neighbor voxels which have a nearly equal 
 * surface normal vector.
 */
class WWallDetectOctree : public WOctree
{
public:
    /**
     * Creates the wall detection instance.
     * \param detailDepth Supported octree node resolution resolution. It equals the 
     *                    node's radius. Currently only numbers covering 2^n results 
     *                    including negative n values.
     */
    explicit WWallDetectOctree( double detailDepth );
    /**
     * Destroys the wall detection instance.
     */
    virtual ~WWallDetectOctree();
    /**
     * Describes the condition when neighbor nodes can be grouped.
     * \param node1 First node to verify.
     * \param node2 Second node to verify.
     * \return The two nodes can be merged or not.
     */
    virtual bool canGroupNodes( WOctNode* node1, WOctNode* node2 );
    /**
     * Sets the maximal allowed surface normal vector angle to the neighbor voxel. Nodes 
     * with a difference above that angle won't be grouped.
     * \param angleDegrees Maximal angle between two nodes for merge.
     */
    void setWallMaxAngleToNeighborVoxel( double angleDegrees );
    /**
     * Sets largest allowed value: The smallest point distribution vector strenth divided 
     * by the strongest. Nodes above that value aren't connected to any node.
     * \param isotropicThreshold Maximal isotropic threshold for merge.
     */
    void setMaxIsotropicThresholdForVoxelMerge( double isotropicThreshold );
    /**
     * Calculates angle between two vectors
     * \param vector1 First vector.
     * \param vector2 Second vector.
     * \return Angle of the two vectors using the decree scale.
     */
    static double getAngleOfTwoVectors( WVector3d vector1, WVector3d vector2 );
    /**
     * Radial amount of 90 degrees.
     */
    static const double ANGLE_90_DEGREES;

protected:
    /**
     * Returns possible neighbors of a node. Nodes that are in no case traversed before 
     * for a comparison aren't added.
     * \param node Node to get neighbors of.
     * \return Neighbors of the node that are probably traversed.
     */
    virtual vector<WOctNode*> getNeighborsOfNode( WOctNode* node );

private:
    /**
     * The maximal allowed angle between two node surface normal vectors. Nodes above 
     * that angle difference aren't grouped.
     */
    double m_wallMaxAngleToNeighborVoxel;
    /**
     * The biggest allowed value consisting of that: Weakest point distribution vector 
     * strength divided by the strongest. Nodes above that quotient aren't grouped.
     */
    double m_maxIsotropicThresholdForVoxelMerge;
};

#endif  // WWALLDETECTOCTREE_H
