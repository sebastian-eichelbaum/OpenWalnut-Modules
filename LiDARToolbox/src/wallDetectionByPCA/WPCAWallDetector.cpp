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

#include <iostream>
#include <vector>
#include "WPCAWallDetector.h"

WPCAWallDetector::WPCAWallDetector( WWallDetectOctree* analyzableOctree, boost::shared_ptr< WProgress > progressStatus )
{
    m_analyzableOctree = analyzableOctree;
    m_progressStatus = progressStatus;
}

WPCAWallDetector::~WPCAWallDetector()
{
    m_minimalGroupSize = 1;
    m_maximalGroupSize = 100000000;
    m_minimalPointsPerVoxel = 1;
}

void WPCAWallDetector::analyze()
{
    analyzeNode( static_cast<WWallDetectOctNode*>( m_analyzableOctree->getRootNode() ) );
}

void WPCAWallDetector::analyzeNode( WWallDetectOctNode* node )
{
    m_progressStatus->increment( 1 );
    if  ( node->getRadius() <= m_analyzableOctree->getDetailLevel() )
    {
        if(node->getPointCount() >= 3 )
        {
            WPrincipalComponentAnalysis pca;
            pca.analyzeData( *( node->getInputPoints() ) );
            node->clearInputData();
            node->setMean( pca.getMean() );
            node->setEigenVectors( pca.getEigenVectors() );
            node->setEigenValues( pca.getEigenValues() );
        }
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                analyzeNode( static_cast<WWallDetectOctNode*>( node->getChild( child ) ) );
    }
}

boost::shared_ptr< WDataSetPoints > WPCAWallDetector::getOutlinePoints( WDataSetPoints::VertexArray inputVertices )
{
    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );

    size_t count = inputVertices->size() / 3;
    for( size_t index = 0; index < count; index++)
    {
        double x = inputVertices->at( index * 3 );
        double y = inputVertices->at( index * 3 + 1 );
        double z = inputVertices->at( index * 3 + 2 );
        WOctNode* octNode = m_analyzableOctree->getLeafNode( x, y, z );
        if( octNode != 0 )
        {
            WWallDetectOctNode* node = static_cast<WWallDetectOctNode*>( octNode );
            size_t groupSize = m_analyzableOctree->getNodeCountOfGroup( node->getGroupNr() );
            bool groupMatch = groupSize >= m_minimalGroupSize && groupSize <= m_maximalGroupSize;
            bool pointCountMatch = node->getPointCount() >= m_minimalPointsPerVoxel;
            if( groupMatch && pointCountMatch )
            {
                outVertices->push_back( x );
                outVertices->push_back( y );
                outVertices->push_back( z );
                size_t group = node->getGroupNr();
                outColors->push_back( WOctree::calcColor( group, 0 ) ),
                outColors->push_back( WOctree::calcColor( group, 1 ) );
                outColors->push_back( WOctree::calcColor( group, 2 ) );
            }
        }
    }
    if( outVertices->size() == 0 )
    {
        for( size_t item = 0; item < 3; item++ )
        {
            outVertices->push_back( 0 );
            outColors->push_back( 0 );
        }
    }
    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
}

boost::shared_ptr< WTriangleMesh > WPCAWallDetector::getOutline()
{
    boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
    drawNode( static_cast<WWallDetectOctNode*>( m_analyzableOctree->getRootNode() ), tmpMesh );
    return tmpMesh;
}

void WPCAWallDetector::drawNode( WWallDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    if( node->getRadius() <= m_analyzableOctree->getDetailLevel() )
    {
        size_t groupSize = m_analyzableOctree->getNodeCountOfGroup( node->getGroupNr() );
        if( groupSize < m_minimalGroupSize || groupSize > m_maximalGroupSize )
            return;
        if( node->getPointCount() < m_minimalPointsPerVoxel )
            return;

        if( m_voxelOutlineMode == 0 )
            drawLeafNodeCube( node, outputMesh );
        if( m_voxelOutlineMode == 1 )
            drawLeafNodeNormalVector( node, outputMesh );
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                drawNode( static_cast<WWallDetectOctNode*>( node->getChild( child ) ), outputMesh );
    }
}

void WPCAWallDetector::drawLeafNodeCube( WWallDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    size_t index = outputMesh->vertSize();
    osg::Vec4 color = osg::Vec4(
            WOctree::calcColor( node->getGroupNr(), 0 ),
            WOctree::calcColor( node->getGroupNr(), 1 ),
            WOctree::calcColor( node->getGroupNr(), 2 ), 1.0 );
    for( size_t vertex = 0; vertex <= 8; vertex++ )
    {
        double iX = vertex % 2;
        double iY = ( vertex / 2 ) % 2;
        double iZ = ( vertex / 4 ) % 2;
        double x = node->getCenter( 0 ) + node->getRadius() * ( iX * 2.0 - 1.0 );
        double y = node->getCenter( 1 ) + node->getRadius() * ( iY * 2.0 - 1.0 );
        double z = node->getCenter( 2 ) + node->getRadius() * ( iZ * 2.0 - 1.0 );
        outputMesh->addVertex( x, y, z );
        outputMesh->setVertexColor( index+vertex, color );
    }
    // Z = 0
    outputMesh->addTriangle( index + 0, index + 2, index + 1 );
    outputMesh->addTriangle( index + 3, index + 1, index + 2 );
    // X = 0
    outputMesh->addTriangle( index + 0, index + 4, index + 2 );
    outputMesh->addTriangle( index + 4, index + 6, index + 2 );
    // Y = 0
    outputMesh->addTriangle( index + 0, index + 1, index + 4 );
    outputMesh->addTriangle( index + 1, index + 5, index + 4 );
    // Z = 1
    outputMesh->addTriangle( index + 4, index + 5, index + 6 );
    outputMesh->addTriangle( index + 5, index + 7, index + 6 );
    // X = 1
    outputMesh->addTriangle( index + 1, index + 3, index + 5 );
    outputMesh->addTriangle( index + 3, index + 7, index + 5 );
    // Y = 1
    outputMesh->addTriangle( index + 2, index + 6, index + 3 );
    outputMesh->addTriangle( index + 6, index + 7, index + 3 );
}

void WPCAWallDetector::drawLeafNodeNormalVector( WWallDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    if( !node->hasEigenValuesAndVectors() )
    {
        drawLeafNodeCube( node, outputMesh );
        return;
    }

    size_t index = outputMesh->vertSize();
    osg::Vec4 color = osg::Vec4(
            WOctree::calcColor( node->getGroupNr(), 0 ),
            WOctree::calcColor( node->getGroupNr(), 1 ),
            WOctree::calcColor( node->getGroupNr(), 2 ), 1.0 );

    double meanX = node->getMean()[0];
    double meanY = node->getMean()[1];
    double meanZ = node->getMean()[2];
    double vec1_x = node->getEigenVector( 0 )[0];
    double vec1_y = node->getEigenVector( 0 )[1];
    double vec1_z = node->getEigenVector( 0 )[2];
    double vec2_x = node->getEigenVector( 1 )[0];
    double vec2_y = node->getEigenVector( 1 )[1];
    double vec2_z = node->getEigenVector( 1 )[2];
    double vec3_x = node->getEigenVector( 2 )[0];
    double vec3_y = node->getEigenVector( 2 )[1];
    double vec3_z = node->getEigenVector( 2 )[2];
    double eigen1 = pow( node->getEigenValues()[0], 0.5 ) * 2.0;
    double eigen1_s = eigen1 * 3.0 / 5.0;
    double eigen2 = pow( node->getEigenValues()[1], 0.5 ) * 2.0;
    double eigen3 = pow( node->getEigenValues()[2], 0.5 ) * 2.0;

    outputMesh->addVertex( meanX - vec2_x * eigen2, meanY - vec2_y * eigen2, meanZ - vec2_z * eigen2 );
    outputMesh->addVertex( meanX + vec1_x * eigen1, meanY + vec1_y * eigen1, meanZ + vec1_z * eigen1 );
    outputMesh->addVertex( meanX + vec2_x * eigen2, meanY + vec2_y * eigen2, meanZ + vec2_z * eigen2 );
    outputMesh->addVertex( meanX - vec1_x * eigen1, meanY - vec1_y * eigen1, meanZ - vec1_z * eigen1 );

    outputMesh->addVertex( meanX - vec3_x * eigen3, meanY - vec3_y * eigen2, meanZ - vec3_z * eigen3 );
    outputMesh->addVertex( meanX + vec1_x * eigen1_s, meanY + vec1_y * eigen1_s, meanZ + vec1_z * eigen1_s );
    outputMesh->addVertex( meanX + vec3_x * eigen3, meanY + vec3_y * eigen2, meanZ + vec3_z * eigen3 );
    outputMesh->addVertex( meanX - vec1_x * eigen1_s, meanY - vec1_y * eigen1_s, meanZ - vec1_z * eigen1_s );

    for( size_t vertex = 0; vertex < 4; vertex++ )
        outputMesh->setVertexColor( index+vertex, color );

    outputMesh->addTriangle( index + 0, index + 1, index + 2 );
    outputMesh->addTriangle( index + 0, index + 2, index + 3 );
    outputMesh->addTriangle( index + 4, index + 5, index + 6 );
    outputMesh->addTriangle( index + 4, index + 6, index + 7 );
}

void WPCAWallDetector::setMinimalGroupSize( double minimalGroupSize )
{
    m_minimalGroupSize = minimalGroupSize;
}

void WPCAWallDetector::setMaximalGroupSize( double maximalGroupSize )
{
    m_maximalGroupSize = maximalGroupSize;
}

void WPCAWallDetector::setMinimalPointsPerVoxel( double minimalPointsPerVoxel )
{
    m_minimalPointsPerVoxel = minimalPointsPerVoxel;
}

void WPCAWallDetector::setVoxelOutlineMode( size_t voxelOutlineMode )
{
    m_voxelOutlineMode = voxelOutlineMode;
}
