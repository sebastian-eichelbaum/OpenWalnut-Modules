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
#include <algorithm>
#include <vector>
#include "WKdPointND.h"

using std::cout;
using std::endl;
WKdPointND::WKdPointND( vector<double> coordinate )
{
    m_coordinate = coordinate;
}

WKdPointND::WKdPointND( double x, double y )
{
    m_coordinate = vector<double>( 2, 0 );
    m_coordinate[0] = x;
    m_coordinate[1] = y;
}

WKdPointND::WKdPointND( double x, double y, double z )
{
    m_coordinate = vector<double>( 3, 0 );
    m_coordinate[0] = x;
    m_coordinate[1] = y;
    m_coordinate[2] = z;
}

WKdPointND::~WKdPointND()
{
}

size_t WKdPointND::getDimensionCount()
{
    return m_coordinate.size();
}

vector<double> WKdPointND::getCoordinate()
{
    return m_coordinate;
}

void WKdPointND::setCoordinate( vector<double> coordinate )
{
    m_coordinate = coordinate;
}
