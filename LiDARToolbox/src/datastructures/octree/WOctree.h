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

#ifndef WOCTREE_H
#define WOCTREE_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "WOctNode.h"

/**
 * Octree structure for analyzing the point data
 */
class WOctree
{
public:
    /**
     * Octree constructor.
     * \param detailDepth Supported octree node resolution resolution. It equals the 
     *                    node's radius. Currently only numbers covering 2^n results 
     *                    including negative n values.
     */
    explicit WOctree( double detailDepth );
    /**
     * Octree destructor.
     */
    virtual ~WOctree();
    /**
     * Registers a new octree node corresponding to the entered resolution.
     * Sub octree nodes will be created until the level of the smallest set up 
     * node detail level.
     * \param x X coordinate of the registerable point
     * \param y Y coordinate of the registerable point
     * \param z Z coordinate of the registerable point
     */
    void registerPoint( double x, double y, double z );
    /**
     * Returns the leaf octree node of the finest detail level that covers a X/Y/Z
     * coordinate
     * \param x X coordinate covering the voxel.
     * \param y Y coordinate covering the voxel.
     * \param z Z coordinate covering the voxel.
     * \return Octree node that corresponds to the X/Y/Z coordinate. If no node is found, 
     *         then 0 will be returnd.
     */
    WOctNode* getLeafNode( double x, double y, double z );
    /**
     * Returns the root node of the octree.
     * \return The octree root noce.
     */
    WOctNode* getRootNode();
    /**
     * Adjusts group numbers of all leaf nodes so that nodes have the same ID that 
     * represent altogether a single block.
     */
    void groupNeighbourLeafs();
    /**
     * Refreshs group IDs using the temporary id mapping vector m_groupEquivs.
     * \param node Node to refresh.
     */
    void refreshNodeGroup( WOctNode* node );
    /**
     * Returns the voxel neighbor group count. Execute groupNeighbourLeafs() before 
     * acquiring that parameter.
     * \return The voxel neighbor group count.
     */
    size_t getGroupCount();
    /**
     * Returns the detail level. It's the minimal allowed radius of any octnode.
     * \return The minimal radius of any octnode.
     */
    double getDetailLevel();
    //TODO(schwarzkopf): Function parameters that don't belong to the main function don't belong to it.
    /**
     * Returns a color channel value for a particular point group.
     * \param groupNr Group number to assign a corresponding color
     * \param colorChannel Channel of the whole color that is returned. 0=red, 1=green and 2=blue.
     * \return Group color of values between 0.0 and 1.0 corresponding to a particular colorChannel.
     */
    static float calcColor( size_t groupNr, size_t colorChannel ); //TODO(schwarzkopf): Implement the following parameter another way somewhere else.

private:
    /**
     * Method that traverses a node in order to group all neighbor cubes into mutual 
     * group numbers.
     * \param node Subnodes to traverse recursively.
     */
    void groupNeighbourLeafs( WOctNode* node );
    /**
     * Resizes the temporary voxel group id mapping array.
     * \param listLength Target list size to apply.
     */
    void resizeGroupList( size_t listLength );
    /**
     * The root octree node of the whole tree.
     */
    WOctNode* m_root;
    /**
     * The radius of smallest octree nodes. Currently only numbers covering 2^n (included negative n). 
     * are supported.
     */
    double m_detailLevel;
    /**
     * Variable used during voxel grouping for correcting group IDs.
     * It contains Voxel group number to building ID mapping.
     * After the process the size corresponds to the recognized building count.
     */
    std::vector<size_t> m_groupEquivs;
    /**
     * Colors that are used to highlight buildings using different colors.
     */
    static const size_t colors[];
    /**
     * Color count or size of the field colors[].
     */
    static const size_t colorCount;
};

#endif  // WOCTREE_H