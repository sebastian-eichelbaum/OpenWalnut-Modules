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
#include "WParameterDomainKdPoint.h"

WParameterDomainKdPoint::WParameterDomainKdPoint( vector<double> coordinate ) : WKdPointND( coordinate )
{
    m_markedToRefresh = true;
}

WParameterDomainKdPoint::WParameterDomainKdPoint( double x0, double y0, double z0 ) : WKdPointND( x0, y0, z0 )
{
    m_markedToRefresh = true;
}

WParameterDomainKdPoint::~WParameterDomainKdPoint()
{
}

void WParameterDomainKdPoint::setSpatialPoint( WSpatialDomainKdPoint* spatialPoint )
{
    m_assignedSpatialNode = spatialPoint;
}

int WParameterDomainKdPoint::getExtentPointCount()
{
    return m_extentPointCount;
}

WSpatialDomainKdPoint* WParameterDomainKdPoint::getSpatialPoint()
{
    return m_assignedSpatialNode;
}

bool WParameterDomainKdPoint::isAddedToPlane()
{
    return m_isAddedToPlane;
}

bool WParameterDomainKdPoint::isTaggedToRefresh()
{
    return m_markedToRefresh;
}

void WParameterDomainKdPoint::setExtentPointCount( int extentPointCount )
{
    m_extentPointCount = extentPointCount;
}

void WParameterDomainKdPoint::setIsAddedToPlane( bool isAddedToPlane )
{
    m_isAddedToPlane = isAddedToPlane;
}

void WParameterDomainKdPoint::tagToRefresh( bool setToRefreshe )
{
    m_markedToRefresh = setToRefreshe;
}
