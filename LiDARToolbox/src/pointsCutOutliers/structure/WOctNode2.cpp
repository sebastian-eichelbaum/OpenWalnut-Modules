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

#include "WOctree2.h"
#include "WOctNode2.h"

const size_t WOctNode2::vX[] = {0, 1, 0, 1, 0, 1, 0, 1};
const size_t WOctNode2::vY[] = {0, 0, 1, 1, 0, 0, 1, 1};
const size_t WOctNode2::vZ[] = {0, 0, 0, 0, 1, 1, 1, 1};

WOctNode2::WOctNode2( double centerX, double centerY, double centerZ, double radius )
{
    m_groupNr = 0;
    m_center[0] = centerX;
    m_center[1] = centerY;
    m_center[2] = centerZ;
    this->m_radius = radius;
    for  ( size_t index = 0; index < 8; index++ )
        m_child[index] = 0;
}

WOctNode2::~WOctNode2()
{
}

WOctNode2* WOctNode2::getChild( size_t drawer )
{
    return m_child[ drawer ];
}

bool WOctNode2::fitsIn( double x, double y, double z )
{
    if  ( z < m_center[2]-m_radius || z >= m_center[2]+m_radius )
        return false;
    if  ( x < m_center[0]-m_radius || x >= m_center[0]+m_radius )
        return false;
    if  ( y < m_center[1]-m_radius || y >= m_center[1]+m_radius )
        return false;
    return true;
}

size_t WOctNode2::getFittingCase( double x, double y, double z )
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

void WOctNode2::expand()
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
            WOctNode2* newChild = new WOctNode2( centerX, centerY, centerZ, m_radius );
            newChild->setChild( m_child[oct], newCase );
            setChild( newChild, oct );
        }
    }
    m_radius *= 2.0;
}

/**
 * Creates a new octree node within a particular drawer if doesn't exist;
 */
void WOctNode2::touchNode( size_t drawer )
{
    if  ( m_child[drawer] == 0 )
    {
        double centerX = m_center[0] - m_radius*0.5 + static_cast<double>( vX[drawer] )*m_radius;
        double centerY = m_center[1] - m_radius*0.5 + static_cast<double>( vY[drawer] )*m_radius;
        double centerZ = m_center[2] - m_radius*0.5 + static_cast<double>( vZ[drawer] )*m_radius;
        m_child[drawer] = new WOctNode2( centerX, centerY, centerZ, m_radius * 0.5 );
    }
}

double WOctNode2::getRadius()
{
    return m_radius;
}
double WOctNode2::getCenter( size_t dimension )
{
    return m_center[dimension];
}

size_t WOctNode2::getGroupNr()
{
    return m_groupNr;
}
void WOctNode2::setGroupNr( size_t groupNr )
{
    m_groupNr = groupNr;
}


void WOctNode2::setChild( WOctNode2* child, size_t drawer )
{
    m_child[drawer] = child;
}
