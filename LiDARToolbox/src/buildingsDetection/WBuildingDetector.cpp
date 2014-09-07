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

#include "WBuildingDetector.h"

WBuildingDetector::WBuildingDetector()
{
    m_detailDepth = 0.5;
    m_minSearchDetailDepth = 8.0;
    m_minSearchCutUntilAbove = 4.0;

    m_detailDepthBigHeights = 32.0;
    m_minSearchCutUntilAboveBigHeights = 20.0;
}

WBuildingDetector::~WBuildingDetector()
{
}

void WBuildingDetector::detectBuildings( boost::shared_ptr< WDataSetPoints > points )
{
    WDataSetPoints::VertexArray verts = points->getVertices();
    size_t count = verts->size()/3;
    WOctree* zones3d = new WOctree( m_detailDepth );
    WQuadTree* zones2d = new WQuadTree( m_detailDepth );
    WQuadTree* minimalMaxima = new WQuadTree( m_minSearchDetailDepth );
    WQuadTree* targetShowables = new WQuadTree( m_detailDepth );
    m_targetGrouped3d = new WOctree( m_detailDepth );
    for( size_t vertex = 0; vertex < count; vertex++ )
    {
        float x = verts->at( vertex*3 );
        float y = verts->at( vertex*3+1 );
        float z = verts->at( vertex*3+2 );
        zones3d->registerPoint( x, y, z );
        zones2d->registerPoint( x, y, z );
    }
    initMinimalMaxima( zones2d->getRootNode(), minimalMaxima );
    projectDrawableAreas( zones2d->getRootNode(), minimalMaxima, targetShowables );

    fetchBuildingVoxels( zones3d->getRootNode(), targetShowables, m_targetGrouped3d );
    m_targetGrouped3d->groupNeighbourLeafsFromRoot();

    //targetShowables->setExportElevationImageSettings( -16.0, 8.0 );
    //targetShowables->exportElevationImage( "/home/renegade/Dokumente/Projekte/OpenWalnut@Eclipse/elevation images/targetShowables.bmp", 1 );
}
void WBuildingDetector::setDetectionParams( int detailDepth, int minSearchDetailDepth,
        double minSearchCutUntilAbove )
{
    m_detailDepth = pow( 2.0, detailDepth );
    m_minSearchDetailDepth = pow( 2.0, minSearchDetailDepth );
    m_minSearchCutUntilAbove = minSearchCutUntilAbove;
}
WOctree* WBuildingDetector::getBuildingGroups()
{
    return m_targetGrouped3d;
}
void WBuildingDetector::initMinimalMaxima( WQuadNode* sourceNode, WQuadTree* targetTree )
{
    if  ( sourceNode->getRadius() <= m_detailDepth )
    {
        double coordX = sourceNode->getCenter( 0 );
        double coordY = sourceNode->getCenter( 1 );
        double d = m_minSearchDetailDepth / 2.0;
        double height = sourceNode->getValueMax();
        targetTree->registerPoint( coordX - d, coordY - d, height );
        targetTree->registerPoint( coordX - d, coordY + d, height );
        targetTree->registerPoint( coordX + d, coordY - d, height );
        targetTree->registerPoint( coordX + d, coordY + d, height );
    }
    else
    {
        for( int child = 0; child < 4; child++ )
            if( sourceNode->getChild( child ) != 0 )
                initMinimalMaxima( sourceNode->getChild( child ), targetTree );
    }
}
void WBuildingDetector::projectDrawableAreas( WQuadNode* sourceNode,
        WQuadTree* minimalMaxima, WQuadTree* targetTree )
{
    if( sourceNode->getRadius() <= m_detailDepth )
    {
        double coordX = sourceNode->getCenter( 0 );
        double coordY = sourceNode->getCenter( 1 );
        WQuadNode* minimalNode = minimalMaxima->getLeafNode( coordX, coordY );
        WQuadNode* minimalNodeBigHeight = minimalMaxima->getLeafNode( coordX, coordY,
                m_minSearchCutUntilAboveBigHeights );
        if( minimalNode == 0 || minimalNodeBigHeight == 0 ) return;
        double minimalHeight = m_minSearchCutUntilAbove + minimalNode->getValueMin();
        double minimalHeightBigHeights = m_minSearchCutUntilAbove + minimalNodeBigHeight->getValueMin();
        if( sourceNode->getValueMax() < minimalHeight
            && sourceNode->getValueMax() < minimalHeightBigHeights ) return;
        targetTree->registerPoint( coordX, coordY, sourceNode->getValueMax() );
    }
    else
    {
        for( int child = 0; child < 4; child++ )
            if( sourceNode->getChild( child ) != 0 )
                    projectDrawableAreas( sourceNode->getChild( child ), minimalMaxima, targetTree );
    }
}

void WBuildingDetector::fetchBuildingVoxels( WOctNode* sourceNode, WQuadTree* buildingPixels,
        WOctree* targetTree )
{
    if( sourceNode->getRadius() <= m_detailDepth )
    {
        double coordX = sourceNode->getCenter( 0 );
        double coordY = sourceNode->getCenter( 1 );
        double coordZ = sourceNode->getCenter( 2 );
        WQuadNode* buildingArea = buildingPixels->getLeafNode( coordX, coordY );
        if( buildingArea != 0 )
            targetTree->registerPoint( coordX, coordY, coordZ );
    }
    else
    {
        for( int child = 0; child < 8; child++ )
            if( sourceNode->getChild( child ) != 0 )
                    fetchBuildingVoxels( sourceNode->getChild( child ), buildingPixels, targetTree );
    }
}
