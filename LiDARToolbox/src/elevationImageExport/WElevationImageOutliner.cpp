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

#include <string>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include <vector>

#include <osg/Geometry>
#include "core/kernel/WModule.h"

#include "core/dataHandler/WDataSetScalar.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WElevationImageOutliner.h"

WElevationImageOutliner::WElevationImageOutliner()
{
    m_minElevImageZ = 0;
    m_intensityIncreasesPerMeter = 5;
    m_vertices = new WQuadTree( 1 );
    m_printedQuadrats = new WQuadTree( 1 );
}

WElevationImageOutliner::~WElevationImageOutliner()
{
}

void WElevationImageOutliner::importElevationImage( WQuadTree* quadTree, size_t elevImageMode )
{
    boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
    m_outputMesh = tmpMesh;
    m_vertices = new WQuadTree( quadTree->getDetailLevel() );
    m_printedQuadrats = new WQuadTree( quadTree->getDetailLevel() );
    drawNode( quadTree->getRootNode(), quadTree, elevImageMode );
}

void WElevationImageOutliner::drawNode(
        WQuadNode* node, WQuadTree* quadTree, size_t elevImageMode )
{
    if( node->getRadius() <= quadTree->getDetailLevel() )
    {
        double x = node->getCenter( 0 );
        double y = node->getCenter( 1 );
        double d = quadTree->getDetailLevel() * 2;

        WQuadNode* sourceNodes[][3] = { //NOLINT
            { quadTree->getLeafNode( x-d, y-d ), quadTree->getLeafNode( x, y-d ), quadTree->getLeafNode( x+d, y-d ) }, //NOLINT
            { quadTree->getLeafNode( x-d, y ), quadTree->getLeafNode( x, y ), quadTree->getLeafNode( x+d, y ) }, //NOLINT
            { quadTree->getLeafNode( x-d, y+d ), quadTree->getLeafNode( x, y+d ), quadTree->getLeafNode( x+d, y+d ) } }; //NOLINT
        for( size_t yIdx = 0; yIdx < 2; yIdx++)
            for( size_t xIdx = 0; xIdx < 2; xIdx++)
            {
                double xOff = static_cast<double>( xIdx )*d-d+x;
                double yOff = static_cast<double>( yIdx )*d-d+y;
                bool isDone = m_printedQuadrats->getLeafNode( xOff, yOff ) != 0;
                if( !isDone )
                {
                    WQuadNode* sourceQuadrat[] = {
                        sourceNodes[xIdx][yIdx], sourceNodes[xIdx][yIdx+1],
                        sourceNodes[xIdx+1][yIdx+1], sourceNodes[xIdx+1][yIdx] };
                    size_t current = 0;
                    WQuadNode* list[] = {0, 0, 0};
                    for(int index = 0; index < 4; index++ )
                    {
                        if( sourceQuadrat[index] != 0 ) list[current++] = sourceQuadrat[index];
                        if( current == 3 )
                        {
                            size_t vertID0 = getVertexID( list[0], elevImageMode );
                            size_t vertID1 = getVertexID( list[1], elevImageMode );
                            size_t vertID2 = getVertexID( list[2], elevImageMode );
                            m_outputMesh->addTriangle( vertID0, vertID1, vertID2 );
                            list[1] = list[2];
                            current--;
                        }
                    }
                    m_printedQuadrats->registerPoint( xOff, yOff, 0 );
                }
            }
    }
    else
    {
        for  ( size_t child = 0; child < 4; child++ )
            if  ( node->getChild( child ) != 0 )
                drawNode( node->getChild( child ), quadTree, elevImageMode );
    }
}

size_t WElevationImageOutliner::getVertexID(
        WQuadNode* node, size_t elevImageMode )
{
    double x = node->getCenter( 0 );
    double y = node->getCenter( 1 );
    WQuadNode* existingNode = m_vertices->getLeafNode( x, y );
    if( existingNode != 0 ) return existingNode->getID();
    double elevation = elevImageMode != 0 ?node->getValueMax() :node->getValueMin();
    if( elevImageMode == 2 ) elevation = node->getPointCount();
    m_vertices->registerPoint( x, y, elevation );
    existingNode = m_vertices->getLeafNode( x, y );
    size_t currentVertex = m_outputMesh->vertSize();
    existingNode->setID( currentVertex );
    m_outputMesh->addVertex( x, y, m_showElevationInMeshOffset ?elevation :0 );
    elevation = ( elevation - m_minElevImageZ ) * m_intensityIncreasesPerMeter;
    if( elevation < 0.0 ) elevation = 0.0;
    if( elevation > 255.0 ) elevation = 255.0;
    elevation /= 255.0;
    if( !m_showElevationInMeshColor ) elevation = 128;
    osg::Vec4 color = osg::Vec4( elevation, elevation, elevation, 1.0 );
    m_outputMesh->setVertexColor( currentVertex, color );
    return existingNode->getID();
}

void WElevationImageOutliner::setExportElevationImageSettings( double minElevImageZ, double intensityIncreasesPerMeter )
{
    m_minElevImageZ = minElevImageZ;
    m_intensityIncreasesPerMeter = intensityIncreasesPerMeter;
}

void WElevationImageOutliner::setShowElevationInMeshColor( bool showElevationInMeshColor )
{
    m_showElevationInMeshColor = showElevationInMeshColor;
}

void WElevationImageOutliner::setShowElevationInMeshOffset( bool showElevationInMeshOffset )
{
    m_showElevationInMeshOffset = showElevationInMeshOffset;
}

boost::shared_ptr< WTriangleMesh > WElevationImageOutliner::getOutputMesh()
{
    return m_outputMesh;
}

void WElevationImageOutliner::highlightBuildingGroups( boost::shared_ptr< WDataSetPointsGrouped >  groupedPoints )
{
    if( !groupedPoints ) return;
    WDataSetPointsGrouped::VertexArray verts = groupedPoints->getVertices();
    WDataSetPointsGrouped::GroupArray groups = groupedPoints->getGroups();
    size_t count = verts->size()/3;
    size_t nulls = 0;
    size_t outOfRanges = 0;
    for( size_t vertex = 0; vertex < count; vertex++)
    {
        float x = verts->at( vertex*3 );
        float y = verts->at( vertex*3+1 );
        WQuadNode* node = m_vertices->getLeafNode( x, y );
        if( node != 0 )
        {
            if( node->getID() < m_outputMesh->vertSize() )
            {
                size_t group = groups->at( vertex );
                osg::Vec4 color = osg::Vec4( WOctree::calcColor( group, 0 ),
                        WOctree::calcColor( group, 1 ), WOctree::calcColor( group, 2 ), 1.0f );
                m_outputMesh->setVertexColor( node->getID(), color );
            }
            else
            {
                outOfRanges++;
            }
        }
        else
        {
            nulls++;
        }
    }
    if( nulls > 0 )
        std::cout << "!!!WARNING!!! WElevationImageExporter: " << nulls << " invalid poinnt hits!" << std::endl;
    if( outOfRanges > 0 )
        std::cout << "!!!WARNING!!! WElevationImageExporter: " << outOfRanges << " out of ranges!" << std::endl;
}
