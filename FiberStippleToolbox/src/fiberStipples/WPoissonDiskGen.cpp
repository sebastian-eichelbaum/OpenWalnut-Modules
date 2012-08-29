//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#include <ctime>

#include <boost/foreach.hpp>

#include "core/common/math/WMath.h" // for piDouble constant
#include "WPoissonDiskGen.h"
#include "core/common/WStringUtils.h"

using namespace string_utils;

namespace
{
    const double rndMax = RAND_MAX;
    const WVector2d invalid( -1.0, -1.0 );
}

WPoissonDiskGen::WPoissonDiskGen( double radius, size_t maxNeighbourTests, double width, double height )
    : m_radius( radius ),
      m_maxNeighbourTests( maxNeighbourTests ),
      m_width( width ),
      m_height( height ),
      m_numCellsX( static_cast< size_t >( m_width / m_radius ) + 1 ),
      m_numCellsY( static_cast< size_t >( m_height / m_radius ) + 1 ),
      m_grid( m_numCellsY * m_numCellsX, invalid )
{
    // std::cout << "numCellsX: " << m_numCellsX << " numCellsY: " << m_numCellsY << std::endl;
    clock_t start = std::clock();
    std::srand( std::time( NULL ) );
    generatePoints();
    clock_t end = std::clock();
    m_time = (end - start) * 1000 / CLOCKS_PER_SEC;
}

WPoissonDiskGen::~WPoissonDiskGen()
{
}

std::string WPoissonDiskGen::time() const
{
    double hours = static_cast< int >( m_time / ( 1000.0 * 60.0 * 60.0 ) );
    double minutes = static_cast< int >( ( m_time % ( 1000 * 60 * 60 ) ) / ( 1000.0 * 60.0 ) );
    double seconds = static_cast< int >( ( ( m_time % ( 1000 * 60 * 60 ) ) % (1000 * 60 ) ) / 1000.0 );
    std::stringstream ss;
    ss << m_time << " XX " << hours << ":" << minutes << ":" << seconds;
    return ss.str();
}

void WPoissonDiskGen::generatePoints()
{
    // choose a random point
    WVector2d point( m_width * std::rand() / rndMax, m_height * std::rand() / rndMax );

    std::set< WVector2d > actives;
    actives.insert( point );
    m_grid.at( id( point ) ) = point;

    std::set< WVector2d >::const_iterator elem = actives.begin();
    while( !actives.empty() )
    {
        // choose a new random active point
        elem = actives.begin();
        std::advance( elem, std::rand() % actives.size() ); // this is slow!

        point = *elem;

        // try seeding more new points around that point
        if( !trySeedingNewPoints( point, actives ) )
        {
            actives.erase( actives.find( point ) );
            push_back( point );
        }
    }
}

int WPoissonDiskGen::id( const WVector2d& point ) const
{
    if( point[0] < 0 || point[1] < 0 )
    {
        return -1;
    }
    return static_cast< int >( point[0] / m_radius ) + static_cast< int >( point[1] / m_radius ) * m_numCellsX;
}

int WPoissonDiskGen::id( int x, int y ) const
{
    int result = y * m_numCellsX + x;
    if( x < 0 || y < 0 || result >= static_cast< int >( m_numCellsX * m_numCellsY ) )
    {
        return -1;
    }
    else
    {
        return result;
    }
}

bool WPoissonDiskGen::trySeedingNewPoints( const WVector2d& center, std::set< WVector2d >& actives )
{
    bool result = false;
    for( size_t i = 0; i < m_maxNeighbourTests; ++i )
    {
        WVector2d newPoint = nextRandomPoint( center );
        if( m_grid.at( id( newPoint ) ) == invalid && valid( newPoint ) )
        {
            actives.insert( newPoint );
            m_grid.at( id( newPoint ) ) = newPoint;
            result = true;
        }
    }
    return result;
}

bool WPoissonDiskGen::valid( const WVector2d& point ) const
{
    int x = static_cast< int >( id( point ) % m_numCellsX );
    int y = static_cast< int >( id( point ) / m_numCellsX );

    boost::array< int, 8 > neighbours;
    neighbours[0] = id( x - 1, y - 1);
    neighbours[1] = id( x - 0, y - 1);
    neighbours[2] = id( x + 1, y - 1);
    neighbours[3] = id( x - 1, y - 0);
    neighbours[4] = id( x + 1, y - 0);
    neighbours[5] = id( x - 1, y + 1);
    neighbours[6] = id( x - 0, y + 1);
    neighbours[7] = id( x + 1, y + 1);

    // std::cout << id(point) << ", ";
    // BOOST_FOREACH( int cell, neighbours )
    // {
    //     std::cout << cell << ", ";
    // }
    // std::cout << std::endl;

    size_t i = 0;
    BOOST_FOREACH( int cell, neighbours )
    {
        if( cell != -1 && m_grid.at( cell ) != invalid )
        {
            // std::cout << i << " id: " << cell << " point: " << point * 10.0 << " m_grid.at( cell ): " << m_grid.at( cell ) <<  std::endl;
            // std::cout << "klaus" << std::endl;
            if( length2( m_grid.at( cell ) - point ) <= m_radius * m_radius )
            {
                return false;
            }
        }
        ++i;
    }
    return true;
}

WVector2d WPoissonDiskGen::nextRandomPoint( const WVector2d& center ) const
{
    WVector2d point( -1.0, -1.0 );
    // select it from polar coordinates
    while( point[0] < 0.0 || point[0] >= m_width || point[1] < 0.0 || point[1] >= m_height )
    {
        double newRadius = m_radius + std::rand() / rndMax * m_radius;
        double phi = std::rand() / rndMax * 2.0 * piDouble;
        point[0] = center[0] + newRadius * std::cos( phi );
        point[1] = center[1] + newRadius * std::sin( phi );
    }

    return point;
}
