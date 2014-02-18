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
#include "../datastructures/octree/WOctNode.h"
#include "../datastructures/octree/WOctree.h"

/**
 * Tool to draw an octree to a WTriangle mesh in order to e. g. display it using the plugin 
 * triangle mesh renderer.
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
     * Converts an octree to a triangle mesh. Only smallest possible octree nodes will be drawn.
     * \param octree Octree to draw.
     * \return The drawn output triangle mesh.
     */
    static boost::shared_ptr< WTriangleMesh > getOutline( WOctree* octree );

private:
    /**
     * Outlines an octree node in the triangle mesh.
     * \param node Octree node to outline. It doesn't outline itself but children if it has some. All 
     *             subnodes will be traversed.
     * \param outputMesh The target triangle mesh to draw octree leaf nodes.
     * \param octree The octree object of the node. It's required to poll some dimension propertiies.
     */
    static void drawNode( WOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh, WOctree* octree );
};

#endif  // WVOXELOUTLINER_H
