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
     * Sets the maximal allowed surface normal vector angle to the neighbor voxel.
     * Nodes with a difference above that angle won't be grouped.
     * \param angleDegrees Maximal angle between two nodes for merge.
     */
    void setWallMaxAngleToNeighborVoxel( double angleDegrees );
    /**
     * Sets the minimal point per voxel count. Voxels below that point count aren't
     * processed.
     * \param minimalPointsPerVoxel Voxel's point count to be processed.
     */
    void setMinimalPointsPerVoxel( size_t minimalPointsPerVoxel );
    /**
     * Sets the maximal quotient of the second Eigen Value over the biggest one. 
     * Voxels below that value are treated as linear and processed another way.
     * \param linearThreshold The linear level limit.
     */
    void setEigenValueQuotientLinear( double linearThreshold );
    /**
     * Sets largest allowed value: The smallest point distribution vector strenth
     * divided by the strongest. Nodes above that value aren't connected to any node.
     * \param isotropicThreshold Maximal isotropic threshold for merge.
     */
    void setMaxIsotropicThresholdForVoxelMerge( double isotropicThreshold );
    /**
     * Calculates angle between two vectors
     * \param vector1 First vector.
     * \param vector2 Second vector.
     * \return Angle of the two vectors using the decree scale.
     */
    static double getAngleOfVectors( WVector3d vector1, WVector3d vector2 );
    /**
     * Calculates the angle of two vectors. It has the range of 180°. Vectors
     * showing exactly the same direction have 0°. Lying in the same line but
     * pointing the opposite direction have the difference of 180°.
     * \param vector1 First vector to calculate an angle between.
     * \param vector2 Second vector to calculate an angle between.
     * \return The angle between the two vectors.
     */
    static double getAngleOfNormals( WVector3d vector1, WVector3d vector2 );
    /**
     * Returns how many nodes a group ID has.
     * \param groupNr The group ID.
     * \return Leaf node count of that croup.
     */
    size_t getNodeCountOfGroup( size_t groupNr );
    /**
     * Counts The number of leaf nodes for each group.
     */
    void generateNodeCountsOfGroups();
    /**
     * Shows whether a node has linear properties.
     * \param node Node to examine.
     * \return The node is linear or not.
     */
    bool isLinearNode( WWallDetectOctNode* node );
    /**
     * Shows whether a node has isotropic properties.
     * \param node Node to examine.
     * \return The node is isotropic or not.
     */
    bool isIsotropicNode( WWallDetectOctNode* node );
    /**
     * Radial amount of 90 degrees.
     */
    static const double ANGLE_90_DEGREES;

protected:
    /**
     * Returns possible neighbors of a node. Nodes that are in no case traversed 
     * before for a comparison aren't added.
     * \param node Node to get neighbors of.
     * \return Neighbors of the node that are probably traversed.
     */
    virtual vector<WOctNode*> getNeighborsOfNode( WOctNode* node );

private:
    /**
     * Adds a node and its children to the node counts for groups.
     * \param node Node to analyze.
     */
    void addGroupCountsFromNode( WWallDetectOctNode* node );

    /**
     * The maximal allowed angle between two node surface normal vectors. Nodes above 
     * that angle difference aren't grouped.
     */
    double m_wallMaxAngleToNeighborVoxel;
    /**
     * The maximal quotient of the second Eigen Value over the biggest one. 
     * Voxels below that value are treated as linear and processed another way.
     */
    double m_eigenValueQuotientLinear;
    /**
     * The biggest allowed value consisting of that: Weakest point distribution 
     * vector strength divided by the strongest. Nodes above that quotient aren't 
     * grouped.
     */
    double m_maxIsotropicThresholdForVoxelMerge;
    /**
     * The minimal point per voxel count. Voxels below that point count aren't
     * processed.
     */
    size_t m_minimalPointsPerVoxel;
    /**
     * Node counts for each connected group.
     */
    vector<size_t> m_nodeCountsOfGroups;
};

#endif  // WWALLDETECTOCTREE_H
