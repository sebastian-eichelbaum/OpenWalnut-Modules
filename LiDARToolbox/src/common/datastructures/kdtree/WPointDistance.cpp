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
#include "../../math/vectors/WVectorMaths.h"

WPointDistance::WPointDistance()
{
    m_pointDistance = 0;
    m_comparedPoint = 0;
}

WPointDistance::WPointDistance( vector<double> sourcePoint, WKdPointND* comparedPoint )
{
    m_comparedPoint = comparedPoint;
    m_pointDistance = WVectorMaths::getEuclidianDistance( sourcePoint, getComparedPoint()->getCoordinate() );
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

vector<WPosition>* WPointDistance::convertToPointSet( vector<WPointDistance>* pointDistances )
{
    vector<WPosition>* pointSet = new vector<WPosition>();
    for( size_t index = 0; index < pointDistances->size(); index++ )
    {
        vector<double> coordinate = pointDistances->at( index ).getComparedCoordinate();
        if( coordinate.size() == 3 )
            pointSet->push_back( WPosition( coordinate[0], coordinate[1], coordinate[2] ) );
    }
    return pointSet;
}

bool WPointDistance::operator<( WPointDistance const& right ) const
{
    return m_pointDistance < right.m_pointDistance;
}
