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

#include "../../math/vectors/WVectorMaths.h"
#include "WPointSearcher.h"

WPointSearcher::WPointSearcher()
{
    m_distanceSteps = 4;
    m_examinedKdTree = 0;
    m_maxResultPointCount = 50;
    m_maxSearchDistance = 0.5;
    m_foundPoints = 0;
}

WPointSearcher::WPointSearcher( WKdTreeND* kdTree )
{
    m_distanceSteps = 4;
    m_examinedKdTree = 0;
    m_maxResultPointCount = 50;
    m_maxSearchDistance = 0.5;
    m_examinedKdTree = kdTree;
    m_foundPoints = 0;
}

WPointSearcher::~WPointSearcher()
{
    m_searchedCoordinate.resize( 0 );
    m_searchedCoordinate.reserve( 0 );
}

vector<WPointDistance>* WPointSearcher::getNearestPoints()
{
    m_foundPoints = new vector<WPointDistance>(); //TODO(aschwarzkopf): Only debugging step over crashes on that line.
    double index = 1;
    size_t distanceSteps = m_maxResultPointCount == numeric_limits< size_t >::max() ?1 :m_distanceSteps;
    for( index = 1; index <= distanceSteps
            && m_foundPoints->size() < m_maxResultPointCount; index++ )
    {
        delete m_foundPoints;
        m_foundPoints = new vector<WPointDistance>();
        double maxDistance = m_maxSearchDistance * pow( 2.0, - ( double )distanceSteps + ( double )index );
        traverseNodePoints( m_examinedKdTree, maxDistance );
        //cout << "Attempt at max distance: " << maxDistance << "    size = " << m_foundPoints->size() << endl;
    }
    std::sort( m_foundPoints->begin(), m_foundPoints->end() );
    //cout << "Found " << m_foundPoints->size() << " Neighbors on index " << (index - 1)
    //      << " with maximal disstance " << m_foundPoints->at(m_foundPoints->size() - 1).getDistance() << endl;
    if( m_foundPoints->size() > m_maxResultPointCount )
    {
        m_foundPoints->resize( m_maxResultPointCount );
        m_foundPoints->reserve( m_maxResultPointCount );
    }
    return m_foundPoints;
}

size_t WPointSearcher::getNearestNeighborCount()
{
    if( m_maxResultPointCount == numeric_limits< size_t >::max() )
    {
        return getNearestNeighborCountInfiniteMaxCount( m_examinedKdTree );
    }
    else
    {
        vector<WPointDistance>* nearestPoints = getNearestPoints();
        size_t neighbourCount = nearestPoints->size();
        delete nearestPoints;
        return neighbourCount;
    }
}

size_t WPointSearcher::getNearestNeighborCountInfiniteMaxCount( WKdTreeND* currentNode )
{
    size_t pointCount = 0;
    vector<WKdPointND* >* nodePoints = currentNode->getNodePoints();
    WKdTreeND* lowerChild = currentNode->getLowerChild();
    WKdTreeND* higherChild = currentNode->getHigherChild();
    if( nodePoints->size() > 0 && lowerChild == 0 && higherChild == 0)
    {
        for( size_t index = 0; index < nodePoints->size(); index++ )
        {
            WKdPointND* point = nodePoints->at( index );
            if( pointCanBelongToPointSet( point->getCoordinate(), m_maxSearchDistance ) )
                pointCount++;
        }
    }
    else
    {
        if( nodePoints->size() == 0 && lowerChild != 0 && higherChild != 0 )
        {
            double splittingDimension = currentNode->getSplittingDimension();
            double pointCoord = m_searchedCoordinate.at( splittingDimension );
            if( currentNode->isLowerKdNodeCase( pointCoord - m_maxSearchDistance ) )
                pointCount += getNearestNeighborCountInfiniteMaxCount( currentNode->getLowerChild() );
            if( !currentNode->isLowerKdNodeCase( pointCoord + m_maxSearchDistance ) )
                pointCount += getNearestNeighborCountInfiniteMaxCount( currentNode->getHigherChild() );
        }
        else
        {
            cout << "!!!UNKNOWN EXCEPTION!!! - getting nearest points" << endl;
        }
    }
    return pointCount;
}

void WPointSearcher::setExaminedKdTree( WKdTreeND* kdTree )
{
    m_examinedKdTree = kdTree;
}

void WPointSearcher::setSearchedPoint( const vector<double>& searchedPoint )
{
    m_searchedCoordinate = searchedPoint;
}

void WPointSearcher::setMaxSearchDistance( double distance )
{
    m_maxSearchDistance = distance;
}

void WPointSearcher::setMaxResultPointCount( size_t maxPointCount )
{
    m_maxResultPointCount = maxPointCount;
}

void WPointSearcher::setMaxResultPointCountInfinite()
{
    m_maxResultPointCount = numeric_limits< size_t >::max();
}

void WPointSearcher::traverseNodePoints( WKdTreeND* currentNode, double maxDistance )
{
    vector<WKdPointND* >* nodePoints = currentNode->getNodePoints();
    WKdTreeND* lowerChild = currentNode->getLowerChild();
    WKdTreeND* higherChild = currentNode->getHigherChild();
    if( nodePoints->size() > 0 && lowerChild == 0 && higherChild == 0)
    {
        for( size_t index = 0; index < nodePoints->size(); index++ )
        {
            WKdPointND* point = nodePoints->at( index );
            if( pointCanBelongToPointSet( point->getCoordinate(), maxDistance ) )
                onPointFound( point );
        }
    }
    else
    {
        if( nodePoints->size() == 0 && lowerChild != 0 && higherChild != 0 )
        {
            size_t splittingDimension = currentNode->getSplittingDimension();
            double pointCoord = m_searchedCoordinate.at( splittingDimension );
            if( currentNode->isLowerKdNodeCase( pointCoord - maxDistance ) )
                traverseNodePoints( currentNode->getLowerChild(), maxDistance );
            if( !currentNode->isLowerKdNodeCase( pointCoord + maxDistance ) )
                traverseNodePoints( currentNode->getHigherChild(), maxDistance );
        }
        else
        {
            cout << "!!!UNKNOWN EXCEPTION!!! - getting nearest points" << endl;
        }
    }
}

void WPointSearcher::onPointFound( WKdPointND* point )
{
    m_foundPoints->push_back( WPointDistance( m_searchedCoordinate, point ) );
}

bool WPointSearcher::pointCanBelongToPointSet( const vector<double>& point, double maxDistance )
{
    return WVectorMaths::getEuclidianDistance( m_searchedCoordinate, point ) <= maxDistance;
}
