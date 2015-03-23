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


#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/search/search.h>
#include <pcl/search/kdtree.h>
#include <pcl/features/normal_3d.h>
//#include <pcl/visualization/cloud_viewer.h>
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/region_growing.h>

#include <iostream>
#include <vector>

#include "WSurfaceDetectorPCL.h"


WSurfaceDetectorPCL::WSurfaceDetectorPCL()
{
    m_clusterSizeMin = 100;
    m_clusterSizeMax = 1000 * 1000;
    m_numberOfNeighbours = 30;
    m_smoothnessThresholdDegrees = 7.0;
    m_curvatureThreshold = 1.0;
}

WSurfaceDetectorPCL::~WSurfaceDetectorPCL()
{
}

boost::shared_ptr< WDataSetPointsGrouped > WSurfaceDetectorPCL::detectSurfaces( boost::shared_ptr< WDataSetPoints > inputPoints )
{
    WDataSetPoints::VertexArray inputVerts = inputPoints->getVertices();
    WDataSetPoints::ColorArray inputColors = inputPoints->getColors();
    size_t count = inputVerts->size()/3;

    //pcl::PointCloud<pcl::PointXYZ> cloud;
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud( new pcl::PointCloud<pcl::PointXYZ> );

    // Fill in the cloud data
    cloud->width    = count;
    cloud->height   = 1;
    cloud->is_dense = false;
    cloud->points.resize( cloud->width * cloud->height );
    for  ( size_t vertex = 0; vertex < count; vertex++)
    {
        cloud->points[vertex].x = inputVerts->at( vertex*3 );
        cloud->points[vertex].y = inputVerts->at( vertex*3 + 1 );
        cloud->points[vertex].z = inputVerts->at( vertex*3 + 2 );
    }

    pcl::search::Search<pcl::PointXYZ>::Ptr tree = boost::shared_ptr<pcl::search::Search<pcl::PointXYZ> > ( new pcl::search::KdTree<pcl::PointXYZ> );
    pcl::PointCloud <pcl::Normal>::Ptr normals( new pcl::PointCloud <pcl::Normal> );
    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> normal_estimator;
    normal_estimator.setSearchMethod( tree );
    normal_estimator.setInputCloud( cloud );
    normal_estimator.setKSearch( 50 );
    normal_estimator.compute( *normals );

    pcl::IndicesPtr indices( new std::vector <int> );
    pcl::PassThrough<pcl::PointXYZ> pass;
    pass.setInputCloud( cloud );
    pass.setFilterFieldName( "z" );
    pass.setFilterLimits( 0.0, 1.0 );
    pass.filter( *indices );

    pcl::RegionGrowing<pcl::PointXYZ, pcl::Normal> reg;
    reg.setMinClusterSize( m_clusterSizeMin );
    reg.setMaxClusterSize( m_clusterSizeMax );
    reg.setSearchMethod( tree );
    reg.setNumberOfNeighbours( m_numberOfNeighbours );
    reg.setInputCloud( cloud );
    //reg.setIndices( indices );
    reg.setInputNormals( normals );
    reg.setSmoothnessThreshold( m_smoothnessThresholdDegrees / 180.0 * M_PI );
    reg.setCurvatureThreshold( m_curvatureThreshold );

    std::vector <pcl::PointIndices> clusters;
    reg.extract( clusters );



    WDataSetPointsGrouped::VertexArray outputVerts(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray outputColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    WDataSetPointsGrouped::GroupArray outputGroups(
            new WDataSetPointsGrouped::GroupArray::element_type() );


    for( size_t cluster = 0; cluster < clusters.size(); cluster++ )
    {
        for( size_t index = 0; index < clusters[cluster].indices.size(); index++ )
        {
            size_t point = clusters[cluster].indices.at( index );
            for( size_t channel = 0; channel < 3; channel++ )
            {
                outputVerts->push_back( inputVerts->at( point*3 + channel ) );
                outputColors->push_back( inputColors->at( point*3 + channel ) );
            }
            outputGroups->push_back( cluster );
        }
    }

    boost::shared_ptr< WDataSetPointsGrouped > output(
            new WDataSetPointsGrouped( outputVerts, outputColors, outputGroups ) );
    return output;
}

void WSurfaceDetectorPCL::setClusterSizeRange( size_t sizeMin, size_t sizeMax )
{
    m_clusterSizeMin = sizeMin;
    m_clusterSizeMax = sizeMax;
}

void WSurfaceDetectorPCL::setNumberOfNeighbors( size_t count )
{
    m_numberOfNeighbours = count;
}

void WSurfaceDetectorPCL::setSmoothnessThreshold( double degrees )
{
    m_smoothnessThresholdDegrees = degrees;
}

void WSurfaceDetectorPCL::setCurvatureThreshold( double threshold )
{
    m_curvatureThreshold = threshold;
}
