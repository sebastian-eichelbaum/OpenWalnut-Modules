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

#include <algorithm>
#include <vector>
#include <limits>

#include "../../common/math/vectors/WVectorMaths.h"
#include "../structure/WParameterDomainKdPoint.h"
#include "WParameterSpaceSearcher.h"

WParameterSpaceSearcher::WParameterSpaceSearcher()
{
    m_tagToRefresh = false;
    m_segmentationMaxAngleDegrees = 15;
    m_segmentationMaxPlaneDistance = 0.7;
}
WParameterSpaceSearcher::~WParameterSpaceSearcher()
{
}

void WParameterSpaceSearcher::setSegmentationSettings( double maxAngleDegrees, double planeDistance )
{
    m_segmentationMaxAngleDegrees = maxAngleDegrees;
    m_segmentationMaxPlaneDistance = planeDistance;
}
void WParameterSpaceSearcher::setSearchedPeakCenter( const vector<double>& peakCenter )
{
    setSearchedPoint( peakCenter );
    setMaxSearchDistance( getMaxParameterDistance( peakCenter ) );
    setMaxResultPointCountInfinite();
    m_distanceSteps = 4;
    m_tagToRefresh = false;
}
void WParameterSpaceSearcher::tagExtentToRefresh()
{
    m_tagToRefresh = true;
    traverseNodePoints( m_examinedKdTree, m_maxSearchDistance );
    m_tagToRefresh = false;
}

void WParameterSpaceSearcher::onPointFound( WKdPointND* point )
{
    if( !m_tagToRefresh )
    {
        m_foundPoints->push_back( WPointDistance( m_searchedCoordinate, point ) );
    }
    else
    {
        static_cast<WParameterDomainKdPoint*>( point )->tagToRefresh( true );
    }
}
bool WParameterSpaceSearcher::pointCanBelongToPointSet( const vector<double>& point, double maxDistance )
{
    if( WVectorMaths::getEuclidianDistance( m_searchedCoordinate, point ) > maxDistance )
        return false;
    return ( isParameterOfSameExtent( m_searchedCoordinate, point ) );
}

double WParameterSpaceSearcher::getMaxParameterDistance( const vector<double>& parametersXYZ0 )
{   //TODO(aschwarzkopf): Implement a better bounding box concept later.
    vector<double> extent( 3, 0.0 );
    vector<double> origin( 3, 0.0 );
    extent[0] = WVectorMaths::getEuclidianDistance( origin, parametersXYZ0 );
    double angle = m_segmentationMaxAngleDegrees / 90.0 * asin( 1.0 );
    double distanceNear = extent[0] - m_segmentationMaxPlaneDistance;
    double distanceFar = extent[0] + m_segmentationMaxPlaneDistance;
    double rotationX = cos( angle ) - sin( angle );
    double rotationY = sin( angle ) + cos( angle );
    vector<double> parameterNear( 3, 0.0 );
    vector<double> parameterFar( 3, 0.0 );
    parameterNear[0] = rotationX * distanceNear;
    parameterNear[1] = rotationY * distanceNear;
    parameterFar[0] = rotationX * distanceFar;
    parameterFar[1] = rotationY * distanceFar;
    distanceNear = WVectorMaths::getEuclidianDistance( extent, parameterNear );
    distanceFar = WVectorMaths::getEuclidianDistance( extent, parameterFar );
    return distanceNear > distanceFar ?distanceNear :distanceFar;
}
bool WParameterSpaceSearcher::isParameterOfSameExtent( const vector<double>& parameters1, const vector<double>& parameters2 )
{
    vector<double> origin( 3, 0 );
    double distance1 = WVectorMaths::getEuclidianDistance( origin, parameters1 );
    double distance2 = WVectorMaths::getEuclidianDistance( origin, parameters2 );
    if( distance1 + distance2 == 0.0 )
        return true;
    if( abs( distance1 - distance2 ) > m_segmentationMaxPlaneDistance )
        return false;
    double angle = WVectorMaths::getAngleOfPlanes( parameters1, parameters2 );
    return angle <= m_segmentationMaxAngleDegrees;
}
