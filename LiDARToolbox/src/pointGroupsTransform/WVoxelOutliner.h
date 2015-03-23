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

#ifndef WVOXELOUTLINER_H
#define WVOXELOUTLINER_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/datastructures/octree/WOctNode.h"
#include "../common/datastructures/octree/WOctree.h"

/**
 * Tool to draw an octree to a WTriangle mesh in order to e. g. display it using the plugin 
 * triangle mesh renderer.
 * Voxels are organized using an unbounded regular grid.
 */
class WVoxelOutliner
{
public:
    /**
     * Constructor of the voxel outliner class. It's not necessary because getOutline() can be 
     * accessed statically.
     */
    explicit WVoxelOutliner();

    /**
     * Destructor of the voxel outliner.
     */
    virtual ~WVoxelOutliner();

    /**
     * Sets outline voxel widt.
     * \param voxelWidth Width of outlining voxels.
     */
    void setVoxelWidth( double voxelWidth );

    /**
     * Registers a point to be outlined as a voxel in a regular grid.
     * \param x X point coordinate.
     * \param y Y point coordinate.
     * \param z Z point coordinate.
     */
    void registerPoint( const double x, const double y, const double z );

    /**
     * returns an octree leaf node voxel of a corresponding coordinate.
     * This makes possible to further assign parameters as a group ID to outline colors.
     * \param x X point coordinate covered by the desiired voxel.
     * \param y Y point coordinate covered by the desiired voxel.
     * \param z Z point coordinate covered by the desiired voxel.
     * \return The desired voxel covered by a coordinate.
     */
    WOctNode* getOctreeLeafNode( double x, double y, double z );

    /**
     * Converts an octree to a triangle mesh. Only smallest possible octree nodes will be drawn.
     * \param highlightUsingColors Add color to voxels corresponding to their group IDs.
     * \return The drawn output triangle mesh.
     */
    boost::shared_ptr< WTriangleMesh > getOutline( bool highlightUsingColors );

private:
    /**
     * Outlines an octree node in the triangle mesh if it's a leaf noce. Parents are 
     * just traversed recursively.
     * \param node Octree node to outline. It doesn't outline itself but children if it 
     *             has some. All subnodes will be traversed.
     * \param outputMesh The target triangle mesh to draw octree leaf nodes.
     * \param octree The octree object of the node. It's required to poll some dimension 
     *               propertiies.
     * \param highlightUsingColors Add color to voxels corresponding to their group ID.
     */
    static void drawNode( WOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh, WOctree* octree,
                          bool highlightUsingColors );

    /**
     * Voxels organized using a regular grid. They are used to outline point areas. It 
     * has an unbounded octree structure.
     */
    WOctree* m_tree;
};

#endif  // WVOXELOUTLINER_H
