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

#include "WOctree.h"
#include "WOctNode.h"

const size_t WOctNode::vX[] = {0, 1, 0, 1, 0, 1, 0, 1};
const size_t WOctNode::vY[] = {0, 0, 1, 1, 0, 0, 1, 1};
const size_t WOctNode::vZ[] = {0, 0, 0, 0, 1, 1, 1, 1};

WOctNode::WOctNode( double centerX, double centerY, double centerZ, double radius )
{
    m_groupNr = 0;
    m_center[0] = centerX;
    m_center[1] = centerY;
    m_center[2] = centerZ;
    this->m_radius = radius;
    for  ( size_t index = 0; index < 8; index++ )
        m_child[index] = 0;
}

WOctNode::~WOctNode()
{
}

WOctNode* WOctNode::getChild( size_t drawer )
{
    return m_child[ drawer ];
}

bool WOctNode::fitsIn( double x, double y, double z )
{
    if  ( z < m_center[2]-m_radius || z >= m_center[2]+m_radius )
        return false;
    if  ( x < m_center[0]-m_radius || x >= m_center[0]+m_radius )
        return false;
    if  ( y < m_center[1]-m_radius || y >= m_center[1]+m_radius )
        return false;
    return true;
}

size_t WOctNode::getFittingCase( double x, double y, double z )
{
    if  ( !fitsIn( x, y, z ) )
        return false;
    size_t caseX = x < m_center[0] ?0 :1;
    size_t caseY = y < m_center[1] ?0 :1;
    size_t caseZ = z < m_center[2] ?0 :1;
    size_t oct = 0;
    while  ( vX[oct] != caseX || vY[oct] != caseY || vZ[oct] != caseZ )
        oct++;
    return oct;
}

void WOctNode::expand()
{
    for  ( size_t oct = 0; oct < 8; oct++ )
    {
        if  ( m_child[oct] != 0 )
        {
            size_t newCase = 0;
            while  ( vX[newCase] == vX[oct] || vY[newCase] == vY[oct] || vZ[newCase] == vZ[oct] )
                newCase++;

            double centerX = ( static_cast<double>( vX[oct] )*2.0 - 1.0 ) * m_radius + m_center[0];
            double centerY = ( static_cast<double>( vY[oct] )*2.0 - 1.0 ) * m_radius + m_center[1];
            double centerZ = ( static_cast<double>( vZ[oct] )*2.0 - 1.0 ) * m_radius + m_center[2];
            WOctNode* newChild = new WOctNode( centerX, centerY, centerZ, m_radius );
            newChild->setChild( m_child[oct], newCase );
            setChild( newChild, oct );
        }
    }
    m_radius *= 2.0;
}

/**
 * Creates a new octree node within a particular drawer if doesn't exist;
 */
void WOctNode::touchNode( size_t drawer )
{
    if  ( m_child[drawer] == 0 )
    {
        double centerX = m_center[0] - m_radius*0.5 + static_cast<double>( vX[drawer] )*m_radius;
        double centerY = m_center[1] - m_radius*0.5 + static_cast<double>( vY[drawer] )*m_radius;
        double centerZ = m_center[2] - m_radius*0.5 + static_cast<double>( vZ[drawer] )*m_radius;
        m_child[drawer] = new WOctNode( centerX, centerY, centerZ, m_radius * 0.5 );
    }
}

double WOctNode::getRadius()
{
    return m_radius;
}
double WOctNode::getCenter( size_t dimension )
{
    return m_center[dimension];
}

size_t WOctNode::getGroupNr()
{
    return m_groupNr;
}
void WOctNode::setGroupNr( size_t groupNr )
{
    m_groupNr = groupNr;
}


void WOctNode::setChild( WOctNode* child, size_t drawer )
{
    m_child[drawer] = child;
}
