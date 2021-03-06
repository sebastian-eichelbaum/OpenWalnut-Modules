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

#ifndef WPCAWALLDETECTOR_H
#define WPCAWALLDETECTOR_H

#include <math.h>
#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/datastructures/quadtree/WQuadNode.h"
#include "../common/datastructures/quadtree/WQuadTree.h"
#include "structure/WWallDetectOctree.h"
#include "structure/WWallDetectOctNode.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include "core/common/WProgress.h"

/**
 * Class that does a Principal Component Analysis on the point data set voxels.
 * It also draws surface group nodes into a triangle mesh.
 */
class WPCAWallDetector
{
public:
    /**
     * Instantiates the Principal Component Analysis analysis class.
     * \param octree Octree to analyze
     * \param progressStatus Assigned progress status.
     */
    explicit WPCAWallDetector( WWallDetectOctree* octree, boost::shared_ptr< WProgress > progressStatus );

    /**
     * Destroys the PCA analysis class.
     */
    virtual ~WPCAWallDetector();

    /**
     * Fires a Principal Component Analysis on all leaf nodes.
     */
    void analyze();

    /**
     * Fires a Principal Component Analysis on all node leafs.
     * \param node Node and all its subchildren to to analyze.
     */
    void analyzeNode( WWallDetectOctNode* node );

    /**
     * Returns the voxel outline in a triangle mesh. Depending on m_voxelOutlineMode
     * it's either depicted as voxels of its group color (=0) or rhombs displaying
     * the node group, Eigen Vectors, Eigen Values and the input point's mean (=1).
     * \param inputVertices Points to be added a group color.
     * \return Input points with an added group color.
     */
    boost::shared_ptr< WDataSetPoints > getOutlinePoints( WDataSetPoints::VertexArray inputVertices );

    /**
     * Draws the nodes into a triangle mesh. Groups of each surface are displayed 
     * using colors.
     * \return The voxels grouped by surfaces.
     */
    boost::shared_ptr< WTriangleMesh > getOutline();

    /**
     * Draws an octree noe containing grouped surface data into a triangle mesh.
     * \param node Octree node to draw.
     * \param outputMesh Target triangle mesh.
     */
    void drawNode( WWallDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh );

    /**
     * Sets the minimal group size of nodes or its surface parts that makes voxels
     * be drawn.Especially parts of buildings have bigger connected parts than e. g. 
     * trees.
     * \param minimalGroupSize The minimal connected node count to put out.
     */
    void setMinimalGroupSize( double minimalGroupSize );

    /**
     * Sets the maximal node count of groups. Groups above that voxel count aren't
     * displayed
     * \param maximalGroupSize Maximal node count for groups to set.
     */
    void setMaximalGroupSize( double maximalGroupSize );

    /**
     * Sets the minimal allowed point count per voxel. Very small pixel counts don't
     * make sense for the Principal Component Analysis.
     * \param minimalPointsPerVoxel The minimal point count per voxel to put them 
     *                              out.
     */
    void setMinimalPointsPerVoxel( double minimalPointsPerVoxel );

    /**
     * Sets the mode how the output triangle mesh is organized.
     * 0: Voxels with a group color
     * 1: Rhombs displaying the node group, the three Eigen Vectors, relative Eigen
     *    Values and the mean coordinate of input points.
     * \param voxelOutlineMode The kind of the voxel outline.
     */
    void setVoxelOutlineMode( size_t voxelOutlineMode );

private:
    /**
     * Draws a leaf node as a voxel on the output triangle mesh.
     * \param node Leaf node to draw.
     * \param outputMesh The output triangle mesh to draw a voxel.
     */
    void drawLeafNodeCube( WWallDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh );

    /**
     * Draws a voxel using a rhomb. It Displays the node group, Eigen Vectors, Eigen
     * Values and the mean of input coordinates.
     * \param node Leaf node to draw.
     * \param outputMesh The output triangle mesh to draw a voxel.
     */
    void drawLeafNodeNormalVector( WWallDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh );

    /**
     * Input point data to be analyzed organized by voxels.
     */
    WWallDetectOctree* m_analyzableOctree;

    /**
     * The assigned progress status.
     */
    boost::shared_ptr< WProgress > m_progressStatus;

    /**
     * The minimal group size of nodes or its surface parts that makes voxels be 
     * drawn. Especially parts of buildings have bigger connected parts than e. g. 
     * trees.
     */
    double m_minimalGroupSize;

    /**
     * Maximal node count of groups to display. Groups above that voxel count aren't 
     * put out.
     */
    double m_maximalGroupSize;

    /**
     * Minimal allowed point count per voxel. Very small pixel counts don't make 
     * sense for the Principal Component Analysis.
     */
    double m_minimalPointsPerVoxel;

    /**
     * The mode how the output triangle mesh is organized.
     * 0: Voxels with a group color
     * 1: Rhombs displaying the node group, the three Eigen Vectors, relative Eigen
     *    Values and the mean coordinate of input points.
     */
    size_t m_voxelOutlineMode;
};

#endif  // WPCAWALLDETECTOR_H
