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

#ifndef WBUILDINGDETECTOR_H
#define WBUILDINGDETECTOR_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "../datastructures/octree/WOctNode.h"
#include "../datastructures/quadtree/WQuadNode.h"
#include "../datastructures/quadtree/WQuadTree.h"
#include "../datastructures/octree/WOctree.h"

/**
 * Class that detects buildings using the WDataSetPoints
 */
class WBuildingDetector
{
public:
    /**
     * Constructor of the building detector instance.
     */
    explicit WBuildingDetector();
    /**
     * Destructor of the building detector instance.
     */
    virtual ~WBuildingDetector();
    /**
     * Starts the routine detecting buildings using the point data.
     * After executing you can extract the building number from each node 
     * using the method getBuildingGroups().
     * \param points Point data to extract buildings from
     */
    void detectBuildings( boost::shared_ptr< WDataSetPoints > points );
    /**
     * Sets setting params for building recognition
     * \param m_detailDepth Data grid resolution of cube group data. The number must
     *                      be an element of 2^n. n can be smaller than 0 It means the 
     *                      area's radius by 2^n for recognition as building.
     * \param minSearchDetailDepth The area width to search a corresponding minimum.
     *                             This number must be an element of 2^n. n can be 
     *                             smaller than 0. Finally this parameter determines the 
     *                             search area of the relative minimum.
     * \param minSearchCutUntilAbove The minimal height above the relative minimal 
     *                               height in order to recognize as a building.
     */
    void setDetectionParams( int m_detailDepth, int minSearchDetailDepth,
            double minSearchCutUntilAbove );
    /**
     * Returns the 3D node tree. Each leaf node has a group ID number. Many nodes can pe 
     * pointed by an equal parameter to a single building area.
     * \return The octree of existing buildings. Buildings are grouped by an equal
     *         group number parameter of WOctNode leafs.
     */
    WOctree* getBuildingGroups();

private:
    /**
     * Inits the image of minimals which is used to determine a relative minimum of 
     * a X/Y coordinate.
     * \param sourceNode Input image with fine-grain maximal heights to calculate a 
     *                   better image of relative minimums. It removes most outliers 
     *                   below the ground.
     * \param targetTree The grifty image where minimums are takin in order to 
     *                   compare whether points are above threshold.
     */
    void initMinimalMaxima( WQuadNode* sourceNode, WQuadTree* targetTree );
    /**
     * Calculates 2D-areas which cover buildings. Building areas will be outlined in targetTree.
     * \param sourceImage Input image. Maximal point values are taken.
     * \param minimalMaxima Image of relative minimums calculated by initMinimalMaxima();
     * \param targetTree Output image containing elevation data. Areas covering no 
     *                   buildings won*t contain data.
     */
    void projectDrawableAreas( WQuadNode* sourceImage, WQuadTree* minimalMaxima,
            WQuadTree* targetTree );
    /**
     * It generates a voxel structure. Leaf nodes should appear where building points exist.
     * \param sourceNode Source octree contatining data that represents any 
     *                   point data altogether.
     * \param buildingPixels Image that depicts areas covered by buildings in order to map 
     *                       the source 3D cubes on them.
     * \param targetTree Not grouped output voxels that represent building point areas.
     */
    void fetchBuildingVoxels( WOctNode* sourceNode, WQuadTree* buildingPixels,
        WOctree* targetTree );

    /**
     * Resolution of input/output data in meters. Use only numbers depictable by 2^n 
     * where n can also be 0 or below.
     */
    double m_detailDepth;
    /**
     * Resolution of the relative minimum search image. Use only numbers depictable by 2^n 
     * where n can also be 0 or below. The bigger the pixels the greater are the areas 
     * searched from an examined X/Y area. Their radius equals that parameter.
     */
    double m_minSearchDetailDepth;
    /**
     * Height that must exceed above an relative minimum to recognize it as a building pixel.
     */
    double m_minSearchCutUntilAbove;
    /**
     * The same as m_minSearchDetailDepth. But it's used to still be able tu use smaller
     * m_minSearchDetailDepth values not having big proglems with larger buildings.
     */
    double m_detailDepthBigHeights;
    /**
     * The corresponding threshold height setting for m_detailDepthBigHeights.
     */
    double m_minSearchCutUntilAboveBigHeights;

    /**
     * The Octree that depicts the set of all buildings. Each leaf node represents a cube 
     * within X/Y/Z. Every cube has a group id which corresponds to a building number. 
     * This field is calculated by detectBuildings().
     */
    WOctree* m_targetGrouped3d;
};

#endif  // WBUILDINGDETECTOR_H
