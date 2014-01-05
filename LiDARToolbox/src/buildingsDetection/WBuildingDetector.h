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

#ifndef WBUILDINGDETECTOR_H
#define WBUILDINGDETECTOR_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WOctNode.h"
#include "structure/WQuadNode.h"
#include "structure/WQuadTree.h"
#include "structure/WOctree.h"

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
     * Don't forget to execute initMinimalMaxima() before using that method.
     * After executing you can extract the building number from each node 
     * using the method getBuildingGroups().
     * \param points Point data to extract buildings from
     */
    void detectBuildings( boost::shared_ptr< WDataSetPoints > points );
    /**
     * Sets setting params for building recognition
     * \param m_detailDepth data grid resolution of cube group data. The number must
     *                      be an element of 2^n. n can be smaller than 0.
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
     * Returns the 3D node tree. Each node has a group ID number. Many nodes can pe 
     * pointed by an equal parameter to a single building area.
     * \return The octree of existing buildings. Buildings are grouped by an equal
     *         group number parameter of WOctNode.
     */
    WOctree* getBuildingGroups();

private:
    /**
     * Inits the image of minimals which is used to determine a relative minimum of 
     * a X/Y coordinate.
     * \param sourceNode Input image with fine-grain maximal geights to calculate a 
     *                   better image of relative minimums. It removes outliers below 
     *                   the ground.
     * \param targetTree The grifty image to calculate relative minimums.
     */
    void initMinimalMaxima( WQuadNode* sourceNode, WQuadTree* targetTree );
    /**
     * Calculates 2D-areas which cover buildings.
     * \param sourceImage Input image, maximal points.
     * \param minimalMaxima Image of relative minimums calculated by initMinimalMaxima();
     * \param targetTree Output image containing elevation data. Areas covering no 
     *                   buildings should contain no data.
     */
    void projectDrawableAreas( WQuadNode* sourceImage, WQuadTree* minimalMaxima,
            WQuadTree* targetTree );
    /**
     * This is one of the last steps. It sorts out voxels that belong to buildings.
     * \param sourceNode Source octree contatining data that represent to contain any 
     *                   point data altogether.
     * \param buildingPixels Image that depicts areas covered by buildings in order to map 
     *                       the source 3D cubes on them.
     * \param targetTree Not grouped voxels that represent building point areas.
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
     * searched from an examined X/Y area.
     */
    double m_minSearchDetailDepth;
    /**
     * Height that must exceed above an relative minimum to recognize it as a building pixel.
     */
    double m_minSearchCutUntilAbove;
    /**
     * The same as m_minSearchDetailDepth. But it's used to still be able tu use smaller
     * areas not having big proglems with larger buildings.
     */
    double m_detailDepthBigHeights;
    /**
     * The corresponding height setting for m_detailDepthBigHeights.
     */
    double m_minSearchCutUntilAboveBigHeights;

    /**
     * The Octree that depicts the set of all buildings. Each node represents a cube 
     * within X/Y/Z. Every cube has a group id which corresponds to a building number. 
     * This field is calculated by detectBuildings().
     */
    WOctree* m_targetGrouped3d;
};

#endif  // WBUILDINGDETECTOR_H
