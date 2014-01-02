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

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "WQuadTree.h"


WQuadTree::WQuadTree( double detailDepth )
{
    m_root = new WQuadNode( 0.0, 0.0, 1.0 );
    m_detailLevel = detailDepth;
}

WQuadTree::~WQuadTree()
{
}

void WQuadTree::registerPoint( double x, double y, double elevation )
{
    while  ( !m_root->fitsIn( x, y ) )
        m_root->expand();

    WQuadNode* node = m_root;
    node->registerPoint( x, y, elevation );
    while  ( node->getRadius() > m_detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y );
        node->touchNode( drawer );
        node = node->getChild( drawer );
        node->registerPoint( x, y, elevation );
    }
}

WQuadNode* WQuadTree::getRootNode()
{
    return m_root;
}

void WQuadTree::exportElevationImage( const char* path, size_t elevImageMode )
{
    double width = getBin( m_root->getXMax() ) - getBin( m_root->getXMin() ) + 1.0;
    double height = getBin( m_root->getYMax() ) - getBin( m_root->getYMin() ) + 1.0;
    WBmpImage* output = new WBmpImage( width, height );
    for( size_t y = 0; y < output->getSizeY(); y++ )
        for( size_t x = 0 ; x < output->getSizeX(); x++ )
            output->setPixel( x, y, 255, 0, 0 );

    drawNode( m_root, m_root, output, elevImageMode );

    WBmpSaver::saveImage( output, path );
}

void WQuadTree::drawNode( WQuadNode* node, WQuadNode* rootNode, WBmpImage* image, size_t elevImageMode )
{
    if( node->getRadius() <= m_detailLevel )
    {
        int intensity = ( node->getElevationMax() + 16.0 ) * 8.0;
        if(elevImageMode == 0)
            intensity = ( node->getElevationMin() + 16.0 ) * 8.0;
        if(elevImageMode == 2)
            intensity = node->getPointCount();
        if( intensity < 0 )
            intensity = 0;
        if( intensity > 255 )
            intensity = 255;
        double x = getBin( node->getCenter( 0 ) ) - getBin( m_root->getXMin() );
        double y = getBin( node->getCenter( 1 ) ) - getBin( m_root->getYMin() );
        image->setPixel( x, y, intensity );
    }
    else
    {
        for  ( size_t child = 0; child < 4; child++ )
            if  ( node->getChild( child ) != 0 )
                drawNode( node->getChild( child ), rootNode, image, elevImageMode );
    }
}
size_t WQuadTree::getBin( double x )
{
    double value = x / 2.0 / m_detailLevel;
    if( value >= 0.0 )
        return value;

    double goback = static_cast<int>( value - 1.0 );
    return static_cast<int>(value - goback) + goback;
}
