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
#include <vector>
#include "WBoundaryDetectPoint.h"

WBoundaryDetectPoint::WBoundaryDetectPoint( vector<double> coordinate ) : WKdPointND( coordinate[0], coordinate[1] )
{
    if( coordinate.size() >= 3 )
        m_zCoordinate = coordinate[2];
}

WBoundaryDetectPoint::WBoundaryDetectPoint( double x, double y, double z ) : WKdPointND( x, y )
{
    m_zCoordinate = z;
}

WBoundaryDetectPoint::~WBoundaryDetectPoint()
{
}


WSpatialDomainKdPoint* WBoundaryDetectPoint::getSpatialPoint()
{
    return m_assignedSpatialPoint;
}

bool WBoundaryDetectPoint::isAddedToPlane()
{
    return m_isAddedToPlane;
}

void WBoundaryDetectPoint::setIsAddedToPlane( bool isAddedToPlane )
{
    m_isAddedToPlane = isAddedToPlane;
}

void WBoundaryDetectPoint::setSpatialPoint( WSpatialDomainKdPoint* assignedSpatialPoint )
{
    m_assignedSpatialPoint = assignedSpatialPoint;
}
