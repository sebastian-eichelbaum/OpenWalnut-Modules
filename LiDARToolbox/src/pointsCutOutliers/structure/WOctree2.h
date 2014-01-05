//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#ifndef WOCTREE2_H
#define WOCTREE2_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "WOctNode2.h"

/**
 * Octree structure for analyzing buildings point data
 */
class WOctree2
{
public:
    /**
     * Octree constructor
     * \param detailDepth Supported octree node resolution resolution. Currently only
     *                    numbers covering 2^n results including negative n values.
     */
    explicit WOctree2( double detailDepth );
    /**
     * Octree destructor.
     */
    virtual ~WOctree2();
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
    WOctNode2* getLeafNode( double x, double y, double z );
    /**
     * Adjusts group numbers of all leaf nodes so that nodes have the same ID that 
     * represent altogether a single block.
     */
    void groupNeighbourLeafs();
    /**
     * Refreshs group IDs using the temporary id mapping vector m_groupEquivs.
     * \param node Node to refresh.
     */
    void refreshNodeGroup( WOctNode2* node );
    /**
     * Returns a WTriangleMesh which outlines the octree.
     * \return Triangle mesh that represents the outline.
     */
    boost::shared_ptr< WTriangleMesh > getOutline();
    /**
     * Returns the root node of the octree.
     * \return The octree root noce.
     */
    WOctNode2* getRootNode();

private:
    /**
     * Draws an octree node. All subchildren will also be drawn.
     * \param node Octree node to draw
     * \param outputMesh Output triangle mesh where the outline will be drawn.
     */
    void drawNode( WOctNode2* node, boost::shared_ptr< WTriangleMesh > outputMesh );
    /**
     * method that traverses a node in order to group all neighbor cubes into mutual 
     * group numbers.
     * \param node Subnodes to traverse recursively.
     */
    void groupNeighbourLeafs( WOctNode2* node );
    /**
     * Resizes the temporary voxel group id mapping array.
     * \param listLength Target list size to apply.
     */
    void resizeGroupList( size_t listLength );
    /**
     * The root octree node of the whole tree.
     */
    WOctNode2* m_root;
    /**
     * Detail level of the octree. Currently only numbers covering 2^n results 
     * including negative n values.
     */
    double m_detailLevel;
    /**
     * Variable used during voxel grouping.
     * It contains Voxel group number to building ID mapping.
     * After the process the size corresponds to the recognized building count.
     */
    std::vector<size_t> m_groupEquivs;
};

#endif  // WOCTREE2_H
