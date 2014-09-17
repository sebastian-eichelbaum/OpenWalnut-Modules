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

#ifndef WELEVATIONIMAGEOUTLINER_H
#define WELEVATIONIMAGEOUTLINER_H


#include <liblas/liblas.hpp>
#include <string>
#include <vector>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include "core/kernel/WModule.h"

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WTriangleMesh.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include "core/dataHandler/WDataSetPoints.h"

#include "../common/datastructures/WDataSetPointsGrouped.h"



//!.Unnecessary imports
#include "core/common/WItemSelection.h"
#include "core/common/WItemSelector.h"

#include "core/kernel/WModuleOutputData.h"

#include <osg/Group>
#include <osg/Material>
#include <osg/StateAttribute>

#include "core/kernel/WKernel.h"
#include "core/common/exceptions/WFileNotFound.h"
#include "core/common/WColor.h"
#include "core/common/WPathHelper.h"
#include "core/common/WPropertyHelper.h"
#include "core/common/WItemSelectionItem.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WGERequirement.h"

#include "../common/algorithms/bitmapImage/WBmpImage.h"
#include "../common/algorithms/bitmapImage/WBmpSaver.h"
#include "../common/datastructures/quadtree/WQuadTree.h"
#include "../common/datastructures/quadtree/WQuadNode.h"
#include "../common/datastructures/octree/WOctree.h"
#include "../common/datastructures/octree/WOctNode.h"

/**
 * Tool that outlines an elevation imate to a triangle mesh.
 */
class WElevationImageOutliner
{
public:
    /**
     * The elevation image to triangle mesh outliner constructor.
     */
    WElevationImageOutliner();
    /**
     * The elevation image to triangle mesh outliner destructor.
     */
    virtual ~WElevationImageOutliner();
    /**
     * Sets the elevation image export settings.
     * \param minElevImageZ The elevation height that is mapped to the black color.
     * \param intensityIncreasesPerMeter Intensity increase count per meter.
     */
    void setExportElevationImageSettings( double minElevImageZ, double intensityIncreasesPerMeter );
    /**
     * Sets whether the elevation will be displayed in the triangle mesh color.
     * \param showElevationInMeshColor Show elevation in triangle mesh color.
     */
    void setShowElevationInMeshColor( bool showElevationInMeshColor );
    /**
     * Sets whether the elevation will be displayed in the triangle mesh height offset.
     * \param showElevationInMeshOffset Show elevation in triangle mesh offset.
     */
    void setShowElevationInMeshOffset( bool showElevationInMeshOffset );
    /**
     * Draws an elevation image to the m_outputMesh triangle mesh.
     * \param quadTree Input quadtree depicting an elevation image.
     * \param elevImageMode Input quadtree depicting an elevation image.
     *                      0: Minimal Z values each X/Y bin coordinate.
     *                      1: Maximal Z values each X/Y bin coordinate.
     *                      2: Corresponding to the Point count each X/Y bin coordinate.
     */
    void importElevationImage( WQuadTree* quadTree, size_t elevImageMode );
    /**
     * Highlights point groups within the elevation image (m_outputMesh) using a set of color.
     * \param groupedPoints The point data set with an additional group parameter.
     */
    void highlightBuildingGroups( boost::shared_ptr< WDataSetPointsGrouped >  groupedPoints );
    /**
     * Returns the triangle mesh depicting the elevation image calculated by importElevationImage().
     * \return Triangle mesh depicting the elevation image.
     */
    boost::shared_ptr< WTriangleMesh > getOutputMesh();

private:
    /**
     * Draws an elevation image to the m_outputMesh triangle mesh from an elevation image 
     * data node. All subnodes will be traversed.
     * \param node If it's a leaf node then physical neighbor nodes will be analyzed to 
     *             draw triangles to m_outputMesh. If it's a parent node then it'll 
     *             used to traverse children.
     * \param quadTree The quadtree object of the node. It's mainly used to be able to poll 
     *                 node's physical neighbors.
     * \param elevImageMode Elevation image kind to draw.
     *                      0: Depicts minimal Z values
     *                      1: Depicts maximal Z values
     *                      2: Depicts point counts within elevation image areas
     */
    void drawNode( WQuadNode* node, WQuadTree* quadTree, size_t elevImageMode );
    /**
     * Returns the vertex ID corresponding to a quadtree leaf node X/Y coordinate.
     * The corresponding output triangle mesh vertex will be added if it doesn't exist 
     * in m_outputMesh. All parameters including the color depicting the elevation height 
     * will be initialized.
     * \param node Elevation image area (only leaf nodes) to gather a corresponding 
     *             m_outputMesh vertex ID.
     * \param elevImageMode Elevation image type for initializing the vertex if doesn't 
     *                      exist before:
     *                      0: Minimal Z values each X/Y bin coordinate.
     *                      1: Maximal Z values each X/Y bin coordinate.
     *                      2: Corresponding to the Point count each X/Y bin coordinate.
     * \return The m_outputMesh vertex ID to the corresponding elevation image value node.
     */
    size_t getVertexID( WQuadNode* node, size_t elevImageMode );
    /**
     * Triangle mesh where the elevation image can be generated using importElevationImage().
     */
    boost::shared_ptr< WTriangleMesh > m_outputMesh;
    /**
     * This field is a map to real m_outputMesh vertex indices. Leaf node m_id params map to them.
     */
    WQuadTree* m_vertices;
    /**
     * The elevation image is a grid of vertices which are organized by quadrats.
     * during drawing triangles importElevationImage() needs to know on which quadrat 
     * areas triangle pairs have already been drawn in order to avoid duplicate triangles.
     */
    WQuadTree* m_printedQuadrats;
    /**
     * Elevation reference height which will be taken as the black color;
     */
    double m_minElevImageZ;
    /**
     * Intensity increases (of 8 bit) that are calculated each meter of the elevation image.
     */
    double m_intensityIncreasesPerMeter;
    /**
     * The elevation will be displayed in the triangle mesh color if the value is true.
     */
    bool m_showElevationInMeshColor;
    /**
     * The elevation will be displayed in the triangle mesh height offset if the value is true.
     */
    bool m_showElevationInMeshOffset;
};

#endif  // WELEVATIONIMAGEOUTLINER_H
