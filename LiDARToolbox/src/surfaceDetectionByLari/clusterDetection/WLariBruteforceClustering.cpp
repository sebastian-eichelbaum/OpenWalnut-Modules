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
#include "WLariBruteforceClustering.h"
#include "../../common/math/leastSquares/WLeastSquares.h"


WLariBruteforceClustering::WLariBruteforceClustering( WLariPointClassifier* classifier )
{
    m_parameterDomain = classifier->getParameterDomain();

    m_segmentationMaxAngleDegrees = 10.0;
    m_segmentationPlaneDistance = 1.0;
    setCpuThreadCount( 8 );
}

WLariBruteforceClustering::~WLariBruteforceClustering()
{
}

void WLariBruteforceClustering::detectClustersByBruteForce()
{
    vector<WKdPointND*>* parameterNodes = m_parameterDomain->getAllPoints();
    size_t currentClusterID = 0;
    while( parameterNodes->size() > 0 )
    {
        initExtentSizes( parameterNodes );
        WParameterDomainKdPoint* biggestExtentPoint = 0;
        int mostNeighborsCount = 0;
        for( size_t index = 0; index < parameterNodes->size(); index++ )
        {
            WParameterDomainKdPoint* parameterPoint = static_cast<WParameterDomainKdPoint*>( parameterNodes->at( index ) );
            if( parameterPoint->getExtentPointCount() > mostNeighborsCount )
            {
                biggestExtentPoint = parameterPoint;
                mostNeighborsCount = parameterPoint->getExtentPointCount();
            }
        }
        addExtentCluster( biggestExtentPoint, currentClusterID++ );

        vector<WKdPointND*>* oldParameterNodes = parameterNodes;
        parameterNodes = new vector<WKdPointND*>();
        for(size_t index = 0; index < oldParameterNodes->size(); index++ )
            if( !( static_cast<WParameterDomainKdPoint*>( oldParameterNodes->at(index) ) )->isAddedToPlane() )
                parameterNodes->push_back( oldParameterNodes->at( index ) );
    }
}
void WLariBruteforceClustering::setSegmentationSettings( double maxAngleDegrees, double planeDistance )
{
    m_segmentationMaxAngleDegrees = maxAngleDegrees;
    m_segmentationPlaneDistance = planeDistance;
}
void WLariBruteforceClustering::setCpuThreadCount( size_t cpuThreadCount )
{
    m_cpuThreadCount = cpuThreadCount;
    m_cpuThreads.reserve( m_cpuThreadCount );
    m_cpuThreads.resize( m_cpuThreadCount );
}


vector<WParameterDomainKdPoint*>* WLariBruteforceClustering::getParametersOfExtent( const vector<double>& parametersXYZ0 )
{
    double maxParameterDistance = getMaxParameterDistance( parametersXYZ0 );
    WPointSearcher parameterSearcher( m_parameterDomain );
    parameterSearcher.setMaxResultPointCount( std::numeric_limits< size_t >::max() );
    parameterSearcher.setMaxSearchDistance( maxParameterDistance );
    parameterSearcher.setSearchedPoint( parametersXYZ0 );
    vector<WPointDistance>* nearestPoints = parameterSearcher.getNearestPoints();
    vector<WParameterDomainKdPoint*>* neighborParameters = new vector<WParameterDomainKdPoint*>();
    for( size_t index = 0; index < nearestPoints->size(); index++ )
        if( isParameterOfSameExtent( parametersXYZ0, nearestPoints->at(index).getComparedCoordinate() ) )
            neighborParameters->push_back( static_cast<WParameterDomainKdPoint*>
                    ( nearestPoints->at( index ).getComparedPoint() ) );
    delete nearestPoints;
    return neighborParameters;
}
void WLariBruteforceClustering::initExtentSizes( vector<WKdPointND*>* pointsToProcess )
{
    size_t threads = m_cpuThreadCount < pointsToProcess->size() ?m_cpuThreadCount :pointsToProcess->size();
    for( size_t thread = 0; thread < threads; thread++ )
        m_cpuThreads[thread] = new boost::thread(
                &WLariBruteforceClustering::initExtentSizesAtThread, this, pointsToProcess, thread );
    for( size_t thread = 0; thread < threads; thread++ )
        m_cpuThreads[thread]->join();
}
void WLariBruteforceClustering::initExtentSizesAtThread( vector<WKdPointND*>* pointsToProcess, size_t threadIndex )
{
    for( size_t index = threadIndex; index < pointsToProcess->size(); index += m_cpuThreadCount )
    {
        WParameterDomainKdPoint* refreshable = static_cast<WParameterDomainKdPoint*>( pointsToProcess->at( index ) );
        if( refreshable->isTaggedToRefresh() )
        {
            vector<WParameterDomainKdPoint*>* coExtent =
                getParametersOfExtent( refreshable->getCoordinate() );
            size_t extentPointCount = 0;
            for( size_t coIndex = 0; coIndex < coExtent->size(); coIndex++ )
                if( coExtent->at(coIndex)->isAddedToPlane() == false )
                    extentPointCount++;
            refreshable->setExtentPointCount( extentPointCount );
            refreshable->tagToRefresh( false );
        }
    }
}
void WLariBruteforceClustering::addExtentCluster( WParameterDomainKdPoint* peakCenterPoint, size_t clusterID )
{
    vector<WParameterDomainKdPoint*>* extentPoints =
            getParametersOfExtent( peakCenterPoint->getCoordinate() );
    cout << "Updating Data for extent: " << extentPoints->size() << "/" << m_parameterDomain->getAllPoints()->size() << endl;
    size_t threads = m_cpuThreadCount < extentPoints->size() ?m_cpuThreadCount :extentPoints->size();
    for( size_t thread = 0; thread < threads; thread++ )
        m_cpuThreads[thread] = new boost::thread(
                &WLariBruteforceClustering::addExtentClusterAtThread, this, extentPoints, clusterID, thread );
    for( size_t thread = 0; thread < threads; thread++ )
        m_cpuThreads[thread]->join();
}
void WLariBruteforceClustering::addExtentClusterAtThread( vector<WParameterDomainKdPoint*>* extentPoints, size_t clusterID, size_t threadIndex )
{
    for(size_t index = threadIndex; index < extentPoints->size(); index += m_cpuThreadCount )
    {
        WParameterDomainKdPoint* extentPoint = static_cast<WParameterDomainKdPoint*>( extentPoints->at( index ) );
        if( !extentPoint->isAddedToPlane() )
        {
            vector<WParameterDomainKdPoint*>* coExtent =
                getParametersOfExtent( extentPoint->getCoordinate() );
            for(size_t paramIndex = 0; paramIndex < coExtent->size(); paramIndex++ )
                coExtent->at( paramIndex )->tagToRefresh( true );

            WSpatialDomainKdPoint* assignedSpatialNode = extentPoint->getSpatialPoint();
            assignedSpatialNode->setClusterID( clusterID );

            extentPoint->setIsAddedToPlane( true );
        }
    }
}

double WLariBruteforceClustering::getMaxParameterDistance( const vector<double>& parametersXYZ0 )
{
    vector<double> extent( 3, 0.0 );
    vector<double> origin( 3, 0.0 );
    extent[0] = WPointDistance::getPointDistance( origin, parametersXYZ0 );
    double angle = m_segmentationMaxAngleDegrees / 90.0 * asin( 1.0 );
    double distanceNear = extent[0] - m_segmentationPlaneDistance;
    double distanceFar = extent[0] + m_segmentationPlaneDistance;
    double rotationX = cos( angle ) - sin( angle );
    double rotationY = sin( angle ) + cos( angle );
    vector<double> parameterNear( 3, 0.0 );
    vector<double> parameterFar( 3, 0.0 );
    parameterNear[0] = rotationX * distanceNear;
    parameterNear[1] = rotationY * distanceNear;
    parameterFar[0] = rotationX * distanceFar;
    parameterFar[1] = rotationY * distanceFar;
    distanceNear = WPointDistance::getPointDistance( extent, parameterNear );
    distanceFar = WPointDistance::getPointDistance( extent, parameterFar );
    return distanceNear > distanceFar ?distanceNear :distanceFar;
}
bool WLariBruteforceClustering::isParameterOfSameExtent( const vector<double>& parameters1, const vector<double>& parameters2 )
{
    vector<double> origin( 3, 0 );
    double distance1 = WPointDistance::getPointDistance( origin, parameters1 );
    double distance2 = WPointDistance::getPointDistance( origin, parameters2 );
    if( distance1 + distance2 == 0.0 )
        return true;
    if( abs( distance1 - distance2 ) > m_segmentationPlaneDistance )
        return false;
    double angle = WVectorMaths::getAngleOfPlanes( parameters1, parameters2 );
    return angle <= m_segmentationMaxAngleDegrees;
}
