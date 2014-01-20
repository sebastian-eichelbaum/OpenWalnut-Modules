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
#include "WQuadNode.h"
#include "../bitmapImage/WBmpImage.h"
#include "../bitmapImage/WBmpSaver.h"

#ifndef WQUADTREE_H
#define WQUADTREE_H

/**
 * Octree structure for analyzing buildings point data
 */
class WQuadTree
{
public:
    /**
     * Octree constructor
     * \param detailDepth Supported quadtree node resolution resolution. Currently only
     *                    numbers covering 2^n results including negative n values.
     */
    explicit WQuadTree( double detailDepth );
    /**
     * Octree destructor.
     */
    virtual ~WQuadTree();
    /**
     * Registers a new quadtree node corresponding to the entered resolution.
     * Sub quadtree nodes will be created until the level of the smallest set up 
     * node detail level.
     * \param x X coordinate of the registerable point
     * \param y Y coordinate of the registerable point
     * \param elevation Elevation to register. A x/Y square stores the min. and
     *                  max. elevation.
     */
    void registerPoint( double x, double y, double elevation );
    /**
     * Returns a leaf node of the maximum detail depth covering X/Y coordinates.
     * \param x X coordinate of the quadtree node.
     * \param y Y coordinate of the quadtree node.
     * \return The leaf quadtree node that corresponds to a X/Y coordinate. the 
     *         return is 0 if no node of the finest detail level is found.
     */
    WQuadNode* getLeafNode( double x, double y );
    /**
     * Returns a leaf node of a particular detail depth covering X/Y coordinates.
     * \param x X coordinate of the quadtree node.
     * \param y Y coordinate of the quadtree node.
     * \param detailDepth The detail level that the returned node must have.
     * \return The leaf quadtree node that corresponds to a X/Y coordinate. the 
     *         return is 0 if no node of the finest detail level is found.
     */
    WQuadNode* getLeafNode( double x, double y, double detailDepth );
    /**
     * Returns a WTriangleMesh which outlines the quadtree.
     * \return Triangle mesh that represents the outline.
     */
    WQuadNode* getRootNode();

    /**
     * Exports the quadtree to an elevation image of the bmp format.
     * \param path Target image file path.
     * \param elevImageMode Mode oft the elevation image.
     *                      0: Minimal values each X/Y bin coordinate.
     *                      1: Maximal values each X/Y bin coordinate.
     *                      2: Point count each X/Y bin coordinate.
     */
    void exportElevationImage( const char* path, size_t elevImageMode );
    /**
     * Sets the elevation image export settings.
     * \param minElevImageZ The minimal elevation where the image intensity starts to rise.
     * \param intensityIncreasesPerMeter Intensity increase count per meter.
     */
    void setExportElevationImageSettings( double minElevImageZ, double intensityIncreasesPerMeter );

private:
    /**
     * Draws an quadtree node to the bmp file. All subchildren will also be drawn.
     * \param node Quadtree node to draw
     * \param rootNode Root node that holds some important parameters for each calculation.
     * \param image Image to set pixels into.
     * \param elevImageMode Mode oft the elevation image.
     *                      0: Minimal values each X/Y bin coordinate.
     *                      1: Maximal values each X/Y bin coordinate.
     *                      2: Point count each X/Y bin coordinate.
     */
    void drawNode( WQuadNode* node, WQuadNode* rootNode, WBmpImage* image, size_t elevImageMode );

    /**
     * Calculates relative coordinates which correspond to quadtree bin scheme.
     * \param x Coordinate of any dimension.
     * \return Linear bin coordinate index of a coordinate.
     */
    size_t getBin( double x );

    /**
     * The root quadtree node of the whole tree.
     */
    WQuadNode* m_root;
    /**
     * Detail level of the quadtree. Currently only numbers covering 2^n results 
     * including negative n values.
     */
    double m_detailLevel;
    /**
     * Elevation image export setting.
     * The minimal elevation where the image intensity starts to rise.
     */
    double m_minElevImageZ;
    /**
     * Elevation image export setting.
     * Intensity increase count per meter
     */
    double m_intensityIncreasesPerMeter;
};

#endif  // WQUADTREE_H
