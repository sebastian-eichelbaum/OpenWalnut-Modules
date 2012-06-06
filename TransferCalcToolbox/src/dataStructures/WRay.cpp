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

#include "WRay.h"

WRay::WRay()
{
    // constructor
}

WRay::WRay( WVector4d st, WVector4d dir ) :
    m_start( st ),
    m_direction( dir )
{
    double n = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );
    m_direction *= ( 1/n );
}

WRay::~WRay()
{
    // destructor
}

WVector4d WRay::getStart( )
{
    return m_start;
}

WVector4d WRay::getDirection( )
{
    return m_direction;
}

WVector4d WRay::getSpot( double t )
{
    return m_start + m_direction * t;
}
