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

#ifndef WPCADETECTOR_H
#define WPCADETECTOR_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "../datastructures/octree/WOctNode.h"
#include "../datastructures/quadtree/WQuadNode.h"
#include "../datastructures/quadtree/WQuadTree.h"
#include "../datastructures/octree/WOctree.h"
#include "structure/WPcaDetectOctNode.h"
#include "core/common/math/PrincipalComponentAnalsis/WPrincipalComponentAnalysis.h"
#include "core/common/WProgress.h"

/**
 * Deamon that does a Principal Component analysis on a point data set. Firstly it 
 * calculates an isotropic threshold for each node's point set and giives it a corresponding 
 * color.
 */
class WPCADetector
{
public:
    /**
     * Instantiates the isotropic analysis based on the Principal Component Analysis.
     * \param octree The input data that is organized by voxels.
     * \param progressStatus The associated progress status.
     */
    explicit WPCADetector( WOctree* octree, boost::shared_ptr< WProgress > progressStatus );
    /**
     * Destroys the isotropic analysis instance.
     */
    virtual ~WPCADetector();
    /**
     * Starts the isotropic analysis for all point data voxels.
     */
    void analyze();
    /**
     * Starts the isotropic analysis for all children pf a node.
     * \param node Node and all its subchildren to to analyze.
     */
    void analyzeNode( WPcaDetectOctNode* node );
    /**
     * Sets the displayed isotropic threshold range.
     * \param showedIsotropicThresholdMin Minimal showed threshold.
     * \param showedIsotropicThresholdMax Maximal showed threshold.
     */
    void setDisplayedVarianceQuotientRange( double showedIsotropicThresholdMin, double showedIsotropicThresholdMax );
    /**
     * Sets the maximal isotropic level for drawing voxels. More exactly said: Smallest 
     * point distribution direction strength divided by the biggest one (Smallest Eigen 
     * Value divided by the biggest one). Voxels above that level aren't drawn.
     * \param maximalEigenValueQuotientToDraw Maximal isotropicity level to draw voxels.
     */
    void setMaximalEigenValueQuotientToDraw( double maximalEigenValueQuotientToDraw );
    /**
     * Minimal point amount per voxel to draw. Voxels below that amount aren't drawn.
     * \param minPointsPerVoxelToDraw Minimap point count to draw voxels.
     */
    void setMinPointsPerVoxelToDraw( size_t minPointsPerVoxelToDraw );
    /**
     * Puts the voxel's isotropic threshold display in a triangle mesh.
     * \return The ouput triangle mesh that depicts the data.
     */
    boost::shared_ptr< WTriangleMesh > getOutline();

private:
    /**
     * Draws a colored voxel into the output triangle mesh. It draws nodes that can have
     * leaf nodes.
     * \param node Node to export to the triangle mesh.
     * \param outputMesh Triangle mesh where the isotropic threshold of voxels should be 
     * depicted.
     */
    void drawNode( WPcaDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh );
    /**
     * Draws a colored voxel into the output triangle mesh. It draws only leaf nodes.
     * \param node Node to export to the triangle mesh.
     * \param outputMesh Triangle mesh where the isotropic threshold of voxels should be 
     * depicted.
     */
    void drawLeafNode( WPcaDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh );
    /**
     * Calculates a color for a drawable leaf node. The color depends on the quotient of
     * the smallest Eigen Value over the biggest. That's how the isotropic level is 
     * described here. Planar or linear nodes are blue and isotropic ones are red.
     * \param node Node to calculate the color for.
     * \return The color calculated for the node.
     */
    osg::Vec4 calculateColorForNode( WPcaDetectOctNode* node );
    /**
     * The point data set grouped by voxels to be analyzed.
     */
    WOctree* m_analyzableOctree;
    /**
     * Progress status.
     */
    boost::shared_ptr< WProgress > m_progressStatus;
    /**
     * Minimal showed isotropic threshold.
     */
    double m_showedisotropicThresholdMin;
    /**
     * maximal showed isotropic threshold.
     */
    double m_showedisotropicThresholdMax;
    /**
     * Maximal isotropic level for drawing voxels. More exactly said: Smallest point 
     * distribution direction strength divided by the biggest one (Smallest Eigen Value 
     * divided by the biggest one). Voxels above that level aren't drawn.
     */
    double m_maximalEigenValueQuotientToDraw;
    /**
     * Minimal point amount per voxel to draw. Voxels below that amount aren't drawn.
     */
    size_t m_minPointsPerVoxelToDraw;
};

#endif  // WPCADETECTOR_H
