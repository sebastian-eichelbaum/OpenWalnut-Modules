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


#ifndef WBMPIMAGE_H
#define WBMPIMAGE_H

#include <vector>

#include "../../datastructures/quadtree/WQuadNode.h"
#include "../../datastructures/quadtree/WQuadTree.h"
#include "../../datastructures/octree/WOctree.h"
#include "../../datastructures/WDataSetPointsGrouped.h"

/**
 * Image object. Currently it's used for saving bmp files.
 */
class WBmpImage
{
public:
    /**
     * Image constructor that also defines the main parameters.
     * \param sizeX Image width.
     * \param sizeY Image height.
     */
    WBmpImage( size_t sizeX, size_t sizeY );

    /**
     * Image container destructor.
     */
    virtual ~WBmpImage();

    /**
     * Returns the image width.
     * \return The image width.
     */
    size_t getSizeX();

    /**
     * Returns the image height.
     * \return The image height.
     */
    size_t getSizeY();

    /**
     * Resizes and clears the image.
     * \param sizeX Image width.
     * \param sizeY Image height.
     */
    void resizeImage( size_t sizeX, size_t sizeY );

    /**
     * Returns the red color value of a pixel.
     * \param x Pixel index on X axis (0 to width-1).
     * \param y Pixel index on Y axis (0 to height-1).
     * \return The red color value (0 to 255).
     */
    size_t getR( size_t x, size_t y );

    /**
     * Returns the green color value of a pixel.
     * \param x Pixel index on X axis (0 to width-1).
     * \param y Pixel index on Y axis (0 to height-1).
     * \return The green color value (0 to 255).
     */
    size_t getG( size_t x, size_t y );

    /**
     * Returns the blue color value of a pixel.
     * \param x Pixel index on X axis (0 to width-1).
     * \param y Pixel index on Y axis (0 to height-1).
     * \return The blue color value (0 to 255).
     */
    size_t getB( size_t x, size_t y );

    /**
     * Returns the opacity value of a pixel.
     * \param x Pixel index on X axis (0 to width-1).
     * \param y Pixel index on Y axis (0 to height-1).
     * \return The opacity value (0 to 255).
     */
    size_t getA( size_t x, size_t y );

    /**
     * Sets a color within a pixel using just an intensity.
     * \param x Pixel index on X axis (0 to width-1).
     * \param y Pixel index on Y axis (0 to height-1).
     * \param intensity Value that will be put to all color values equally (0 to 255).
     */
    void setPixel( size_t x, size_t y, size_t intensity );

    /**
     * Sets a color within a pixel.
     * \param x Pixel index on X axis (0 to width-1).
     * \param y Pixel index on Y axis (0 to height-1).
     * \param r The red color intensity (0 to 255).
     * \param g The green color intensity (0 to 255).
     * \param b The blue color intensity (0 to 255).
     */
    void setPixel( size_t x, size_t y, size_t r, size_t g, size_t b );


    /**
     * Imports Quadtree data to the bitmap image. The Quadtree is aligned automatically by 
     * the data voxels (currently only smallest possible leafs).
     * \param quadTree Elevation image that is imported to the bitmap image.
     * \param elevImageMode Mode oft the elevation image.
     *                      0: Minimal Z values each X/Y bin coordinate.
     *                      1: Maximal Z values each X/Y bin coordinate.
     *                      2: Point count each X/Y bin coordinate.
     */
    void importElevationImage( WQuadTree* quadTree, size_t elevImageMode );

    /**
     * Sets the elevation image export settings.
     * \param minElevImageZ The elevation height that is mapped to the black color.
     * \param intensityIncreasesPerMeter Intensity increases count per meter.
     */
    void setExportElevationImageSettings( double minElevImageZ, double intensityIncreasesPerMeter );

    /**
     * Highlights point groups in the image using the grouped data set points. Colors vary 
     * by the group ID.
     * \param groupedPoints The data set points with the 3D cooordinate and point group parameter.
     * \param quadTree The quadtree which was used to import elevation image from. The method knows 
     *                 where to place points using this param's coordinate min/max values.
     */
    void highlightBuildingGroups( boost::shared_ptr< WDataSetPointsGrouped >  groupedPoints, WQuadTree* quadTree );

private:
    /**
     * Returns the color data vector index using X and Y coordinates.
     * \param x X coordinate of the image.
     * \param y Y coordinate of the image.
     * \return The suuitable color data vector index (e. g. for m_dataG).
     */
    size_t getIndex( size_t x, size_t y );

    /**
     * Image width.
     */
    size_t m_sizeX;

    /**
     * Image height.
     */
    size_t m_sizeY;

    /**
     * Image red color intensity data. The order corresponds to the following pixel traversing.
     * It traverses linewise each starting at Y=0 from first to last X value.
     */
    std::vector<size_t> m_dataR;

    /**
     * Image green color intensity data. The order corresponds to the following pixel traversing.
     * It traverses linewise each starting at Y=0 from first to last X value.
     */
    std::vector<size_t> m_dataG;

    /**
     * Image blue color intensity data. The order corresponds to the following pixel traversing.
     * It traverses linewise each starting at Y=0 from first to last X value.
     */
    std::vector<size_t> m_dataB;

    /**
     * Elevation image export setting.
     * The elevation height that is mapped to the black color.
     */
    double m_minElevImageZ;

    /**
     * Elevation image export setting.
     * Intensity increase count per meter.
     */
    double m_intensityIncreasesPerMeter;

    /**
     * Draws a quadtree leaf node to the bmp file. All subchildren will also be drawn.
     * \param node Quadtree node to draw
     * \param quadTree Quadtree that is drawn. Method knows where to draw points on the image 
     *                 using mandatory parameters (puxel size, x/y min/max etc.).
     * \param elevImageMode Mode of the elevation image.
     *                      0: Minimal Z value each X/Y bin coordinate.
     *                      1: Maximal Z value each X/Y bin coordinate.
     *                      2: Point count each X/Y bin coordinate.
     */
    void importElevationImageDrawNode( WQuadNode* node, WQuadTree* quadTree, size_t elevImageMode );
};

#endif  // WBMPIMAGE_H
