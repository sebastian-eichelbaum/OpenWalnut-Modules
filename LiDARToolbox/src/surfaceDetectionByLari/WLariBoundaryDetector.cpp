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
#include <limits>
#include "WLariBoundaryDetector.h"
#include "../common/math/leastSquares/WLeastSquares.h"


WLariBoundaryDetector::WLariBoundaryDetector()
{
    m_spatialDomain = new WKdTreeND();
    m_spatialInputClusters = new vector<vector<WSpatialDomainKdPoint*>*>();
    m_currentClusterID = 0;
    m_transformAngle1zx = 0.0;
    m_transformAngle2zy = 0.0;
    m_currentBoundary = new vector<WBoundaryDetectPoint*>();
    m_clusterSearcher.setMaxResultPointCount( numeric_limits< size_t >::max() );
}

WLariBoundaryDetector::~WLariBoundaryDetector()
{
}

void WLariBoundaryDetector::detectBoundaries( WKdTreeND* parameterDomain )
{
    cout << "detectBoundaries()" << endl;
    groupPointsByGroupID( parameterDomain );

    for( size_t cluster = 0; cluster < m_spatialInputClusters->size(); cluster++ )
    {
        cout << "Modified convex hull on cluster " << m_spatialInputClusters->at( cluster )->size()
                << " points. [" << cluster << "]" << endl;
        detectInputCluster( m_spatialInputClusters->at( cluster ) );
    }
}

void WLariBoundaryDetector::setMaxPointDistanceR( double maxPointDistanceR )
{
    m_maxPointDistanceR = maxPointDistanceR;
    m_clusterSearcher.setMaxSearchDistance( m_maxPointDistanceR );
}


void WLariBoundaryDetector::groupPointsByGroupID( WKdTreeND* parameterDomain )
{
    cout << "groupPointsByGroupID()" << endl;
    vector<WKdPointND*>* parameterPoints = parameterDomain->getAllPoints();
    m_spatialInputClusters = new vector<vector<WSpatialDomainKdPoint*>*>();

    for( size_t index = 0; index < parameterPoints->size(); index++ )
    {
        WSpatialDomainKdPoint* spatialPoint = static_cast<WParameterDomainKdPoint*>(
                parameterPoints->at( index ) )->getSpatialPoint();
        size_t currentInputClusterID = spatialPoint->getClusterID();
        if( currentInputClusterID >= parameterPoints->size() )
        {
            cout << "!!!UNKNOWN EXCEPTION!!! groupPointsByGroupID() - clusterID " << currentInputClusterID
                    << " above point count " << parameterPoints->size() << " (" << index << "/" << parameterPoints->size()
                    << ")" << "   Input array index = " << spatialPoint->getIndexInInputArray() << endl;
        }
        else
        {
            while( m_spatialInputClusters->size() <= currentInputClusterID )
                m_spatialInputClusters->push_back( new vector<WSpatialDomainKdPoint*>() );
            m_spatialInputClusters->at( currentInputClusterID )->push_back( spatialPoint );
        }
    }
}

void WLariBoundaryDetector::transformPoint( vector<double>* transformable )
{
    WVectorMaths::rotateVector( transformable, 2, 0, m_transformAngle1zx );
    WVectorMaths::rotateVector( transformable, 2, 1, m_transformAngle2zy );
}

void WLariBoundaryDetector::detectInputCluster( vector<WSpatialDomainKdPoint*>* inputPointCluster )
{
    initTransformationCoordinateSystem( inputPointCluster );
    vector<WBoundaryDetectPoint*>* clusterPoints = new vector<WBoundaryDetectPoint*>();
    WBoundaryDetectPoint* mostLeftPoint = 0;

    for( size_t index = 0; index < inputPointCluster->size(); index++ )
    {
        WSpatialDomainKdPoint* currentPoint = inputPointCluster->at( index );
        vector<double>* coordinate = WVectorMaths::copyVectorForPointer( currentPoint->getCoordinate() );
        transformPoint( coordinate );
        WBoundaryDetectPoint* clusterPoint = new WBoundaryDetectPoint( *coordinate );
        clusterPoint->setSpatialPoint( currentPoint );
        clusterPoints->push_back( clusterPoint );
        currentPoint->setClusterID( 9 );
        delete coordinate;
    }

    while( clusterPoints->size() > 0 )
    {
        m_currentBoundary->resize( 0 );
        m_currentBoundary->reserve( 0 );

        for( size_t index = 0; index < clusterPoints->size(); index++ )
        {
            vector<double> coordinate = clusterPoints->at( index )->getCoordinate();
            if( index == 0 || coordinate[0] < mostLeftPoint->getCoordinate()[0] )
                mostLeftPoint = clusterPoints->at( index );
            if( !WVectorMaths::isValidVector( coordinate ) )
            {
                WBoundaryDetectPoint* clusterPoint = clusterPoints->at( index );
                cout << "!!! FOUND INVALID POINT !!!  P(" << index << "/" << clusterPoints->size() << ") with distance "
                        << "  P(" << coordinate[0] << ", " << coordinate[1] << ")" << "   Input array index = "
                        << clusterPoint->getSpatialPoint()->getIndexInInputArray() << endl;
                clusterPoint->getSpatialPoint()->setClusterID( m_currentClusterID++ );
                clusterPoint->setIsAddedToPlane( true );
            }
        }

        WKdTreeND* clusterKdTree = new WKdTreeND( 2 );
        clusterKdTree->add( reinterpret_cast< vector<WKdPointND*>*>( clusterPoints ) );
        m_clusterSearcher.setExaminedKdTree( clusterKdTree );

        m_currentBoundary->push_back( mostLeftPoint );
        mostLeftPoint->getSpatialPoint()->setClusterID( m_currentClusterID );
        mostLeftPoint->setIsAddedToPlane( true );
        WBoundaryDetectPoint* boundPoint = getNextBoundPoint();
        bool isChainOpen = true;
        bool isChainValid = true;
        while( boundPoint != 0 && isChainOpen && isChainValid )
        {
            m_currentBoundary->push_back( boundPoint );
            boundPoint->getSpatialPoint()->setClusterID( m_currentClusterID );
            boundPoint->setIsAddedToPlane( true );
            boundPoint = getNextBoundPoint();
            if( m_currentBoundary->at( 0 ) == m_currentBoundary->at( m_currentBoundary->size() - 1 )
                    && m_currentBoundary->at( 1 ) == boundPoint )
                isChainOpen = false;
            isChainValid = boundChainStillValid();
        }

        initAABoundingBoxFromBoundary();
        initOneOutsidePoint();
        for( size_t index = 0; isChainValid && index < clusterPoints->size(); index++ )
        {
            WBoundaryDetectPoint* currentPoint = clusterPoints->at( index );
            const vector<double>& coordinate = currentPoint->getCoordinate();
            if( pointBelongsToBoundingBox( coordinate ) && pointIsInBounds( coordinate ) )
            {
                currentPoint->getSpatialPoint()->setClusterID( m_currentClusterID );
                currentPoint->setIsAddedToPlane( true );
            }
        }

        m_currentClusterID++;

        vector<WBoundaryDetectPoint*>* oldPoints = clusterPoints;
        clusterPoints = new vector<WBoundaryDetectPoint*>();
        for( size_t index = 0; index < oldPoints->size(); index++ )
            if( !oldPoints->at( index )->isAddedToPlane() )
                clusterPoints->push_back( oldPoints->at( index ) );
        //TODO(aschwarzkopf): Merge invalid points group by distance.
    }
}

void WLariBoundaryDetector::initTransformationCoordinateSystem( vector<WSpatialDomainKdPoint*>* extentPointCluster )
{
    //TODO(aschwarzkopf): Still not the very original peak center but the mean of peak center of each parameter.
    vector<WSpatialDomainKdPoint*>* currentPoints = extentPointCluster;
    vector<double>* meanNormalVector = new vector<double>( 3, 0.0 );
    for( size_t index = 0; index < currentPoints->size(); index++ )
    {
        vector<double>* currentPoint = WVectorMaths::copyVectorForPointer( currentPoints->at( index )->getParametersXYZ0() );
        WVectorMaths::normalizeVector( currentPoint );
        if( WVectorMaths::isValidVector( *currentPoint ) )
            WVectorMaths::addVector( meanNormalVector, *currentPoint );
        delete currentPoint;
    }
    WVectorMaths::normalizeVector( meanNormalVector );
    m_transformAngle1zx = - WVectorMaths::getAngleToAxis( meanNormalVector->at( 2 ), meanNormalVector->at( 0 ) );
    WVectorMaths::rotateVector( meanNormalVector, 2, 0, m_transformAngle1zx );
    m_transformAngle2zy = - WVectorMaths::getAngleToAxis( meanNormalVector->at( 2 ), meanNormalVector->at( 1 ) );
    delete meanNormalVector;
}

WBoundaryDetectPoint* WLariBoundaryDetector::getNextBoundPoint()
{
    WBoundaryDetectPoint* previousPoint = m_currentBoundary->size() >= 2
            ?m_currentBoundary->at( m_currentBoundary->size() - 2 )
            :new WBoundaryDetectPoint( m_currentBoundary->at( 0 )->getCoordinate()[0] - 1.0,
                                       m_currentBoundary->at( 0 )->getCoordinate()[1], 0 );
    WBoundaryDetectPoint* currentPoint = m_currentBoundary->at( m_currentBoundary->size() - 1 );

    m_clusterSearcher.setSearchedPoint( currentPoint->getCoordinate() );
    vector<WPointDistance>*  nearestPoints = m_clusterSearcher.getNearestPoints();
    WBoundaryDetectPoint* nextPoint = 0;
    double narrowestAngle = 361;
    for( size_t index = 0; index < nearestPoints->size(); index++ )
    {
        WBoundaryDetectPoint* currentNextPoint = static_cast<WBoundaryDetectPoint*>( nearestPoints->at(
                index ).getComparedPoint() ); //TODO(aschwarzkopf): Can't watch coordinates using expressions
        if( lastBoundaryPointCanReachPoint( nearestPoints->at(index) ) )
        {
            double nextAngle = getAngleToNextPoint( previousPoint, currentPoint, currentNextPoint );
            if( nextAngle < narrowestAngle && !isResultingBoundIntersection( currentNextPoint ) )
            {
                narrowestAngle = nextAngle;
                nextPoint = currentNextPoint;
            }
        }
    }

    if( m_currentBoundary->size() < 2 )
        delete previousPoint;
    return nextPoint;
}

bool WLariBoundaryDetector::lastBoundaryPointCanReachPoint( WPointDistance nextPointDistance )
{
    double distance = nextPointDistance.getDistance();
    if( distance == 0.0 )
        return false;
    WBoundaryDetectPoint* currentPoint = m_currentBoundary->at( m_currentBoundary->size() - 1 );
    WBoundaryDetectPoint* nextPoint = static_cast<WBoundaryDetectPoint*>( nextPointDistance.getComparedPoint() );
    WSpatialDomainKdPoint* nextPointSpatial = static_cast<WSpatialDomainKdPoint*>( nextPoint->getSpatialPoint() );
    double maxDistanceFromCurrent = currentPoint->getSpatialPoint()->getDistanceToNthNearestNeighbor();
    double maxDistanceFromNext = nextPointSpatial->getDistanceToNthNearestNeighbor();
    return distance <= maxDistanceFromCurrent || distance <= maxDistanceFromNext;
}

double WLariBoundaryDetector::getAngleToNextPoint( WBoundaryDetectPoint* previousPoint,
        WBoundaryDetectPoint* currentPoint, WBoundaryDetectPoint* nextPoint )
{
    vector<double>* vectorPoint = WVectorMaths::copyVectorForPointer( nextPoint->getCoordinate() );
    vector<double>* currentPointCoords = WVectorMaths::copyVectorForPointer( currentPoint->getCoordinate() );
    WVectorMaths::invertVector( currentPointCoords );
    WVectorMaths::addVector( vectorPoint, *currentPointCoords );
    double angleToNext = WVectorMaths::getAngleToAxisComplete( vectorPoint->at( 0 ), vectorPoint->at( 1 ) );
    vectorPoint = WVectorMaths::copyVectorForPointer( previousPoint->getCoordinate() );
    WVectorMaths::addVector( vectorPoint, *currentPointCoords );
    double angleToPrevious = WVectorMaths::getAngleToAxisComplete( vectorPoint->at( 0 ), vectorPoint->at( 1 ) );
    delete currentPointCoords;
    delete vectorPoint;
    while( angleToNext < 0.0 )
        angleToNext += 360.0;
    while( angleToPrevious <= angleToNext )
        angleToPrevious += 360.0;
    return angleToPrevious - angleToNext;
}

bool WLariBoundaryDetector::isResultingBoundIntersection( WBoundaryDetectPoint* nextPoint )
{
    WBoundaryDetectPoint* currentPoint = m_currentBoundary->at( m_currentBoundary->size() - 1 );
    for( size_t index = 1; index < m_currentBoundary->size() - 1; index++ )
    {
        const vector<double>& boundP1 = m_currentBoundary->at( index - 1 )->getCoordinate();
        const vector<double>& boundP2 = m_currentBoundary->at( index )->getCoordinate();
        const vector<double>& current = currentPoint->getCoordinate();
        const vector<double>& next = nextPoint->getCoordinate();
        if( WVectorMaths::linesCanIntersectBounded( boundP1, boundP2, current, next )
                && WVectorMaths::getEuclidianDistance( boundP1, current ) > 0.0
                && WVectorMaths::getEuclidianDistance( boundP2, current ) > 0.0
                && WVectorMaths::getEuclidianDistance( boundP1, next ) > 0.0
                && WVectorMaths::getEuclidianDistance( boundP2, next ) > 0.0
                && !WVectorMaths::isPointOnLine2d( current, boundP1, boundP2 )
                && !WVectorMaths::isPointOnLine2d( next, boundP1, boundP2 ) )
            return true;
    }
    return false;
}

void WLariBoundaryDetector::initAABoundingBoxFromBoundary()
{
    if( m_currentBoundary->size() == 0 )
        return;
    m_boundaryAABoundingBoxMin = WVectorMaths::copyVector( m_currentBoundary->at( 0 )->getCoordinate() );
    m_boundaryAABoundingBoxMax = WVectorMaths::copyVector( m_boundaryAABoundingBoxMin );

    for( size_t index = 1; index < m_currentBoundary->size(); index++ )
    {
        const vector<double>& coordinate = m_currentBoundary->at( index )->getCoordinate();
        for( size_t dimension = 0; dimension < coordinate.size(); dimension++ )
        {
            if( coordinate[dimension] < m_boundaryAABoundingBoxMin[dimension] )
                m_boundaryAABoundingBoxMin[dimension] = coordinate[dimension];
            if( coordinate[dimension] > m_boundaryAABoundingBoxMax[dimension] )
                m_boundaryAABoundingBoxMax[dimension] = coordinate[dimension];
        }
    }
}

void WLariBoundaryDetector::initOneOutsidePoint()
{
    m_oneOutsidePoint.reserve( 2 );
    m_oneOutsidePoint.resize( 2 );
    m_oneOutsidePoint[0] = 1.5 * m_boundaryAABoundingBoxMin[0] - 0.5 * m_boundaryAABoundingBoxMax[0];
    m_oneOutsidePoint[1] = 0.5 * m_boundaryAABoundingBoxMin[1] - 0.5 * m_boundaryAABoundingBoxMax[1];
}

bool WLariBoundaryDetector::pointBelongsToBoundingBox( const vector<double>& point )
{
    for( size_t dimension = 0; dimension < m_boundaryAABoundingBoxMin.size(); dimension++ )
        if( point[dimension] < m_boundaryAABoundingBoxMin[dimension] ||
                point[dimension] > m_boundaryAABoundingBoxMax[dimension] )
            return false;
    return true;
}

bool WLariBoundaryDetector::pointIsInBounds( const vector<double>& point )
{
    bool isInBounds = false;
    for( size_t bound = 0; bound < m_currentBoundary->size() - 1; bound++ )
    {
        if( pointLiesOnBound( point, bound ) )
            return true;

        if( pointHitsBound( point, bound ) )
            isInBounds = !isInBounds;
            //TODO(aschwarzkopf): Implement an appropriate routine to handle when intersection hits exactly on points.
            //TODO(aschwarzkopf): Possible solution: Move m_oneOutsidePoint until bound points do not hit exactly.
    }
    return isInBounds;
}

bool WLariBoundaryDetector::pointLiesOnBound( const vector<double>& point, size_t boundNr )
{
    const vector<double> boundPoint1 = m_currentBoundary->at( boundNr )->getCoordinate();
    double secondIndex = boundNr + 1;
    while( boundNr >= m_currentBoundary->size() )
        boundNr -= m_currentBoundary->size();
    const vector<double> boundPoint2 = m_currentBoundary->at( secondIndex )->getCoordinate();
    if( !WVectorMaths::isPointOnLine2d( point, boundPoint1, boundPoint2 ) )
        return false;
    return WVectorMaths::isPointOnLine2d( point, boundPoint1, boundPoint2 );
}

bool WLariBoundaryDetector::pointHitsBound( const vector<double>& point, size_t boundNr )
{
    const vector<double> boundPoint1 = m_currentBoundary->at( boundNr )->getCoordinate();
    size_t secondIndex = boundNr + 1;
    while( secondIndex >= m_currentBoundary->size() )
        secondIndex -= m_currentBoundary->size();
    const vector<double> boundPoint2 = m_currentBoundary->at( secondIndex )->getCoordinate();
    return WVectorMaths::linesCanIntersectBounded( boundPoint1, boundPoint2, point, m_oneOutsidePoint );
}

bool WLariBoundaryDetector::boundChainStillValid()
{
    if( m_currentBoundary->size() < 10 )
        return true;
    size_t last = m_currentBoundary->size() - 1;
    size_t exponent = log( last - 2 ) / log( 2 );
    size_t middle = pow( 2.0, exponent );

    bool isBoundValid = m_currentBoundary->at( middle - 1 ) != m_currentBoundary->at( last - 1 )
            || m_currentBoundary->at( middle ) != m_currentBoundary->at( last );
    if( !isBoundValid )
        cout << "!!!BOUND NOT VALID!!! - on chain size of " << m_currentBoundary->size() << ". Sipping boundary points." << endl;
    return isBoundValid;
}
