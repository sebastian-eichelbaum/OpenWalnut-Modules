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

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "WBmpImage.h"


WBmpImage::WBmpImage( size_t sizeX, size_t sizeY )
{
    resizeImage( sizeX, sizeY );
}

WBmpImage::~WBmpImage()
{
    resizeImage( 0, 0 );
}
size_t WBmpImage::getSizeX()
{
    return m_sizeX;
}
size_t WBmpImage::getSizeY()
{
    return m_sizeY;
}
void WBmpImage::resizeImage( size_t sizeX, size_t sizeY )
{
    m_sizeX = sizeX;
    m_sizeY = sizeY;
    size_t size = sizeX * sizeY;
    m_dataR.reserve( size );
    m_dataG.reserve( size );
    m_dataB.reserve( size );
    m_dataR.resize( size );
    m_dataG.resize( size );
    m_dataB.resize( size );
    for( size_t index = 0; index < size; index++ )
    {
        m_dataB[index] = 0;
        m_dataG[index] = 0;
        m_dataB[index] = 0;
    }
}
size_t WBmpImage::getR( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return m_dataR[getIndex( x, y )];
}
size_t WBmpImage::getG( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return m_dataG[getIndex( x, y )];
}
size_t WBmpImage::getB( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return m_dataB[getIndex( x, y )];
}
size_t WBmpImage::getA( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return 255;
}
void WBmpImage::setPixel( size_t x, size_t y, size_t intensity )
{
    setPixel( x, y, intensity, intensity, intensity );
}
void WBmpImage::setPixel( size_t x, size_t y, size_t r, size_t g, size_t b )
{
    if( x >= m_sizeX || y >= m_sizeY ) return;
    size_t index = getIndex( x, y );
    m_dataR[index] = r < 256 ?r :255;
    m_dataG[index] = g < 256 ?g :255;
    m_dataB[index] = b < 256 ?b :255;
}
size_t WBmpImage::getIndex( size_t x, size_t y )
{
    return x + m_sizeX * y;
}



void WBmpImage::importElevationImage( WQuadTree* quadTree, size_t elevImageMode )
{
    WQuadNode* rootNode = quadTree->getRootNode();
    double width = quadTree->getBin( rootNode->getXMax() ) - quadTree->getBin( rootNode->getXMin() ) + 1.0;
    double height = quadTree->getBin( rootNode->getYMax() ) - quadTree->getBin( rootNode->getYMin() ) + 1.0;
    resizeImage( width, height );
    for( size_t y = 0; y < getSizeY(); y++ )
        for( size_t x = 0 ; x < getSizeX(); x++ )
            setPixel( x, y, 255, 0, 0 );

    importElevationImageDrawNode( rootNode, quadTree, elevImageMode );
}

void WBmpImage::importElevationImageDrawNode( WQuadNode* node, WQuadTree* quadTree, size_t elevImageMode )
{
    if( node->getRadius() <= quadTree->getDetailLevel() )
    {
        int intensity = ( ( elevImageMode != 0
                ?node->getValueMax() :node->getValueMin() )
                - m_minElevImageZ ) * m_intensityIncreasesPerMeter;
        if(elevImageMode == 2) intensity = node->getPointCount();
        if( intensity < 0 ) intensity = 0;
        if( intensity > 255 ) intensity = 255;
        double x = quadTree->getBin( node->getCenter( 0 ) ) - quadTree->getBin( quadTree->getRootNode()->getXMin() );
        double y = quadTree->getBin( node->getCenter( 1 ) ) - quadTree->getBin( quadTree->getRootNode()->getYMin() );
        setPixel( x, y, intensity );
    }
    else
    {
        for  ( size_t child = 0; child < 4; child++ )
            if  ( node->getChild( child ) != 0 )
                importElevationImageDrawNode( node->getChild( child ), quadTree, elevImageMode );
    }
}
void WBmpImage::setExportElevationImageSettings( double minElevImageZ, double intensityIncreasesPerMeter )
{
    m_minElevImageZ = minElevImageZ;
    m_intensityIncreasesPerMeter = intensityIncreasesPerMeter;
}

void WBmpImage::highlightBuildingGroups( boost::shared_ptr< WDataSetPointsGrouped >  groupedPoints, WQuadTree* quadTree )
{
    if( !groupedPoints ) return;
    WDataSetPointsGrouped::VertexArray verts = groupedPoints->getVertices();
    WDataSetPointsGrouped::GroupArray groups = groupedPoints->getGroups();
    size_t count = verts->size()/3;
    for( size_t vertex = 0; vertex < count; vertex++)
    {
        float xTotal = verts->at( vertex*3 );
        float yTotal = verts->at( vertex*3+1 );
        size_t group = groups->at( vertex );
        double x = quadTree->getBin( xTotal ) - quadTree->getBin( quadTree->getRootNode()->getXMin() );
        double y = quadTree->getBin( yTotal ) - quadTree->getBin( quadTree->getRootNode()->getYMin() );
        setPixel( x, y, WOctree::calcColor( group, 0 )*255.0f,
                WOctree::calcColor( group, 1 )*255.0f, WOctree::calcColor( group, 2 )*255.0f );
    }
}
