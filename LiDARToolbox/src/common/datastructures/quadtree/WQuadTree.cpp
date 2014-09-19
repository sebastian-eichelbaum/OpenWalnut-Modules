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

#include "WQuadTree.h"


WQuadTree::WQuadTree( double detailLevel )
{
    m_root = new WQuadNode( 0.0, 0.0, detailLevel );
    m_detailLevel = detailLevel;
}

WQuadTree::~WQuadTree()
{
}

void WQuadTree::registerPoint( double x, double y, double value )
{
    while  ( !m_root->fitsIn( x, y ) )
        m_root->expand();

    WQuadNode* node = m_root;
    node->updateMinMax( x, y, value );
    while  ( node->getRadius() > m_detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y );
        node->touchNode( drawer );
        node = node->getChild( drawer );
        node->updateMinMax( x, y, value );
    }
}

WQuadNode* WQuadTree::getLeafNode( double x, double y )
{
    return getLeafNode( x, y, m_detailLevel);
}

WQuadNode* WQuadTree::getLeafNode( double x, double y, double detailLevel )
{
    if( !m_root->fitsIn( x, y ) )
        return 0;

    WQuadNode* node = m_root;
    while  ( node->getRadius() > detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y );
        if( node->getChild( drawer ) == 0 )
            return 0;
        node = node->getChild( drawer );
    }
    return node;
}

WQuadNode* WQuadTree::getRootNode()
{
    return m_root;
}

size_t WQuadTree::getBin( double x )
{
    double value = x / 2.0 / m_detailLevel;
    if( value >= 0.0 )
        return value;

    double goback = static_cast<int>( value - 1.0 );
    return static_cast<int>(value - goback) + goback;
}

double WQuadTree::getDetailLevel()
{
    return m_detailLevel;
}
