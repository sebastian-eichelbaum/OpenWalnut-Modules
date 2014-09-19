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
#include "../../common/math/leastSquares/WLeastSquares.h"
#include "../../common/math/vectors/WVectorMaths.h"
#include "WParameterSpaceSearcher.h"
#include "WLariBruteforceClustering.h"


WLariBruteforceClustering::WLariBruteforceClustering( WLariPointClassifier* classifier )
{
    m_parameterDomain = new WKdTreeND( 3 );
    m_parameterDomain->add( classifier->getParameterDomain()->getAllPoints() );

    m_segmentationMaxAngleDegrees = 10.0;
    m_segmentationMaxPlaneDistance = 1.0;
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
            {
                parameterNodes->push_back( oldParameterNodes->at( index ) );
            }
            else
            {
                m_parameterDomain->removePoint( oldParameterNodes->at( index ) );
            }
            cout << "Current parameter space Size: " << m_parameterDomain->getAllPoints()->size() << "    ";
    }
}

void WLariBruteforceClustering::setSegmentationSettings( double maxAngleDegrees, double planeDistance )
{
    m_segmentationMaxAngleDegrees = maxAngleDegrees;
    m_segmentationMaxPlaneDistance = planeDistance;
}

void WLariBruteforceClustering::setCpuThreadCount( size_t cpuThreadCount )
{
    m_cpuThreadCount = cpuThreadCount;
    m_cpuThreads.reserve( m_cpuThreadCount );
    m_cpuThreads.resize( m_cpuThreadCount );
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
            WParameterSpaceSearcher parameterSearcher;
            parameterSearcher.setExaminedKdTree( m_parameterDomain );
            parameterSearcher.setSegmentationSettings( m_segmentationMaxAngleDegrees, m_segmentationMaxPlaneDistance );
            parameterSearcher.setSearchedPeakCenter( refreshable->getCoordinate() );
            size_t count = parameterSearcher.getNearestNeighborCount();

            if( count > 10000 )
                cout << "!WARNING! - Extent " << index << "/" << pointsToProcess->size() << " with size " << count << "!" << endl;
            refreshable->setExtentPointCount( count );
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
    WParameterSpaceSearcher taggerToRefresh;
    taggerToRefresh.setExaminedKdTree( m_parameterDomain );
    taggerToRefresh.setSegmentationSettings( m_segmentationMaxAngleDegrees, m_segmentationMaxPlaneDistance );
    for(size_t index = threadIndex; index < extentPoints->size(); index += m_cpuThreadCount )
    {
        WParameterDomainKdPoint* extentPoint = static_cast<WParameterDomainKdPoint*>( extentPoints->at( index ) );
        if( !extentPoint->isAddedToPlane() )
        {
            taggerToRefresh.setSearchedPeakCenter( extentPoint->getCoordinate() );
            taggerToRefresh.tagExtentToRefresh();

            WSpatialDomainKdPoint* assignedSpatialNode = extentPoint->getSpatialPoint();
            assignedSpatialNode->setClusterID( clusterID );

            extentPoint->setIsAddedToPlane( true );
        }
    }
}

vector<WParameterDomainKdPoint*>* WLariBruteforceClustering::getParametersOfExtent( const vector<double>& parametersXYZ0 )
{
    WParameterSpaceSearcher parameterSearcher;
    parameterSearcher.setExaminedKdTree( m_parameterDomain );
    parameterSearcher.setSegmentationSettings( m_segmentationMaxAngleDegrees, m_segmentationMaxPlaneDistance );
    parameterSearcher.setSearchedPeakCenter( parametersXYZ0 );
    vector<WPointDistance>* nearestPoints = parameterSearcher.getNearestPoints();
    vector<WParameterDomainKdPoint*>* neighborParameters = new vector<WParameterDomainKdPoint*>();
    for( size_t index = 0; index < nearestPoints->size(); index++ )
        neighborParameters->push_back( static_cast<WParameterDomainKdPoint*>
                ( nearestPoints->at( index ).getComparedPoint() ) );
    delete nearestPoints;
    return neighborParameters;
}
