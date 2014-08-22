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

#include <vector>

#include "WPointDistance.h"

WPointDistance::WPointDistance()
{
    m_pointDistance = 0;
    m_comparedPoint = 0;
}
WPointDistance::WPointDistance( vector<double> sourcePoint, WKdPointND* comparedPoint )
{
    m_comparedPoint = comparedPoint;
    m_pointDistance = getPointDistance( sourcePoint, getComparedPoint()->getCoordinate() );
}

WPointDistance::~WPointDistance()
{
}
vector<double> WPointDistance::getComparedCoordinate()
{
    return m_comparedPoint->getCoordinate();
}
WKdPointND* WPointDistance::getComparedPoint()
{
    return m_comparedPoint;
}
double WPointDistance::getDistance()
{
    return m_pointDistance;
}
double WPointDistance::getPointDistance( const vector<double>& point1, const vector<double>& point2 )
{    //TODO(aschwarzkopf): Not verified that the euclidian distance is calculated right also for points above 3 dimensions.
    double distance = 0;
    for( size_t index = 0; index < point1.size() && index < point2.size(); index++ )
        distance += pow( point1[index] - point2[index], 2 );
    return pow( distance, 0.5 );
}
bool WPointDistance::operator<( WPointDistance const& right ) const
{
    return m_pointDistance < right.m_pointDistance;
}
