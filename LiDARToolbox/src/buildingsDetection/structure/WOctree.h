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

#include "core/graphicsEngine/WTriangleMesh.h"
#include "WOctNode.h"

#ifndef WOCTREE_H
#define WOCTREE_H

/**
 * Octree structure for analyzing buildings point data
 */
class WOctree
{
public:
    /**
     * Octree constructor
     * \param detailDepth Supported octree node resolution resolution. Currently only
     *                    numbers covering 2^n results including negative n values.
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
     * Returns a WTriangleMesh which outlines the octree.
     * \return Triangle mesh that represents the outline.
     */
    boost::shared_ptr< WTriangleMesh > getOutline();

private:
    /**
     * Draws an octree node. All subchildren will also be drawn.
     * \param node Octree node to draw
     * \param outputMesh Output triangle mesh where the outline will be drawn.
     */
    void drawNode( WOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh );
    /**
     * The root octree node of the whole tree.
     */
    WOctNode* m_root;
    /**
     * Detail level of the octree. Currently only numbers covering 2^n results 
     * including negative n values.
     */
    double m_detailLevel;
};

#endif  // WOCTREE_H
