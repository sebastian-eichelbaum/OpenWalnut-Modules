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

#include <iostream>

#include "WQuadTree.h"
#include "WQuadNode.h"

const size_t WQuadNode::vX[] = {0, 1, 0, 1};
const size_t WQuadNode::vY[] = {0, 0, 1, 1};

WQuadNode::WQuadNode( double centerX, double centerY, double radius )
{
    m_pointCount = 0;
    m_xMin = m_xMax = m_yMin = m_yMax = m_zMin = m_zMax = 0;
    m_center[0] = centerX;
    m_center[1] = centerY;
    this->m_radius = radius;
    for  ( size_t index = 0; index < 4; index++ )
        m_child[index] = 0;
}

WQuadNode::~WQuadNode()
{
    m_pointCount = 0;
}

WQuadNode* WQuadNode::getChild( size_t drawer )
{
    return m_child[ drawer ];
}

bool WQuadNode::fitsIn( double x, double y )
{
    if  ( x < m_center[0]-m_radius || x >= m_center[0]+m_radius )
        return false;
    if  ( y < m_center[1]-m_radius || y >= m_center[1]+m_radius )
        return false;
    return true;
}

size_t WQuadNode::getFittingCase( double x, double y )
{
    if  ( !fitsIn( x, y ) )
        return false;
    size_t caseX = x < m_center[0] ?0 :1;
    size_t caseY = y < m_center[1] ?0 :1;
    size_t quad = 0;
    while  ( vX[quad] != caseX || vY[quad] != caseY )
        quad++;
    return quad;
}

void WQuadNode::expand()
{
    for  ( size_t quad = 0; quad < 4; quad++ )
    {
        if  ( m_child[quad] != 0 )
        {
            size_t newCase = 0;
            while  ( vX[newCase] == vX[quad] || vY[newCase] == vY[quad] )
                newCase++;

            double centerX = ( static_cast<double>( vX[quad] )*2.0 - 1.0 ) * m_radius + m_center[0];
            double centerY = ( static_cast<double>( vY[quad] )*2.0 - 1.0 ) * m_radius + m_center[1];
            WQuadNode* newChild = new WQuadNode( centerX, centerY, m_radius );
            newChild->setChild( m_child[quad], newCase );
            setChild( newChild, quad );
        }
    }
    m_radius *= 2.0;
}

void WQuadNode::touchNode( size_t drawer )
{
    if  ( m_child[drawer] == 0 )
    {
        double centerX = m_center[0] - m_radius*0.5 + static_cast<double>( vX[drawer] )*m_radius;
        double centerY = m_center[1] - m_radius*0.5 + static_cast<double>( vY[drawer] )*m_radius;
        m_child[drawer] = new WQuadNode( centerX, centerY, m_radius * 0.5 );
    }
}

double WQuadNode::getRadius()
{
    return m_radius;
}
double WQuadNode::getCenter( size_t dimension )
{
    return m_center[dimension];
}
void WQuadNode::registerPoint( double x, double y, double elevation )
{
    if( !m_pointCount > 0 )
    {
        m_xMin = m_xMax = x;
        m_yMin = m_yMax = y;
        m_zMin = m_zMax = elevation;
    }
    if( x < m_xMin ) m_xMin = x;
    if( x > m_xMax ) m_xMax = x;
    if( y < m_yMin ) m_yMin = y;
    if( y > m_yMax ) m_yMax = y;
    if( elevation < m_zMin ) m_zMin = elevation;
    if( elevation > m_zMax ) m_zMax = elevation;
    m_pointCount++;
}




void WQuadNode::setChild( WQuadNode* child, size_t drawer )
{
    m_child[drawer] = child;
}

size_t WQuadNode::getPointCount()
{
    return m_pointCount;
}
double WQuadNode::getXMin()
{
    return m_xMin;
}
double WQuadNode::getXMax()
{
    return m_xMax;
}
double WQuadNode::getYMin()
{
    return m_yMin;
}
double WQuadNode::getYMax()
{
    return m_yMax;
}
double WQuadNode::getElevationMin()
{
    return m_zMin;
}
double WQuadNode::getElevationMax()
{
    return m_zMax;
}
