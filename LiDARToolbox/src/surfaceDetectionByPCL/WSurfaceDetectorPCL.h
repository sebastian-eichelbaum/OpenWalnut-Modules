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

#ifndef WSURFACEDETECTORPCL_H
#define WSURFACEDETECTORPCL_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/datastructures/octree/WOctNode.h"
#include "../common/datastructures/quadtree/WQuadNode.h"
#include "../common/datastructures/quadtree/WQuadTree.h"
#include "../common/datastructures/octree/WOctree.h"
#include "../common/datastructures/WDataSetPointsGrouped.h"

/**
 * Class that detects surfaces using the Region Growing Segmentation of the Point Cloud
 * Library
 * More information: http://pointclouds.org/
 */
class WSurfaceDetectorPCL
{
public:
    explicit WSurfaceDetectorPCL();

    virtual ~WSurfaceDetectorPCL();

    /**
     * Detects surfaces within the point cloud usint the Region Growing Segmentation 
     * algorithm of the Point Cloud Library
     * \param inputPoints Input points to be processed.
     * \return The processed point cloud with the appended surface group ID.
     */
    boost::shared_ptr< WDataSetPointsGrouped > detectSurfaces( boost::shared_ptr< WDataSetPoints > inputPoints );

    /**
     * Sets the cluster size of detected surfaces.
     * \param sizeMin Minimal cluster point count of detected surfaces.
     * \param sizeMax Maximal cluster point count of detected surfaces.
     */
    void setClusterSizeRange( size_t sizeMin, size_t sizeMax );

    /**
     * Sets the count of considered neighbors during considering the analysis point wise.
     * \param count The count of considered neighbor points during the analysis.
     */
    void setNumberOfNeighbors( size_t count );

    /**
     * Allows to set smoothness threshold used for testing the points.
     * \param degrees New threshold value for the angle between normals.
     */
    void setSmoothnessThreshold( double degrees );

    /**
     * Allows to set curvature threshold used for testing the points.
     * \param threshold New threshold value for curvature testing.
     */
    void setCurvatureThreshold( double threshold );

private:
    /**
     * Minimal cluster point count of detected surfaces.
     */
    size_t m_clusterSizeMin;

    /**
     * Maximal cluster point count of detected surfaces.
     */
    size_t m_clusterSizeMax;

    /**
     * The count of considered neighbors during considering the analysis point wise.
     */
    size_t m_numberOfNeighbours;

    /**
     * Smoothness threshold used for testing the points. The angle is 
     * scaled by degrees and it is a threshold value for the angle between normals.
     */
    double m_smoothnessThresholdDegrees;

    /**
     * Curvature threshold used for testing the points. It is a threshold value for 
     * curvature testing.
     */
    double m_curvatureThreshold;
};

#endif  // WSURFACEDETECTORPCL_H
