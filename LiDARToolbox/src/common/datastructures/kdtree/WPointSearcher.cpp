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

#include "WPointSearcher.h"

WPointSearcher::WPointSearcher( WKdTreeND* kdTree )
{
    m_distanceSteps = 4;
    m_examinedKdTree = 0;
    m_maxResultPointCount = 50;
    m_maxSearchDistance = 0.5;
    m_examinedKdTree = kdTree;
}
WPointSearcher::~WPointSearcher()
{
    m_searchedPoint.resize( 0 );
    m_searchedPoint.reserve( 0 );
}
vector<WPosition>* WPointSearcher::convertToPointSet( vector<WPointDistance>* pointDistances )
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
vector<WPointDistance>* WPointSearcher::getNearestPoints()
{
    vector<WPointDistance>* nearestPoints = new vector<WPointDistance>();
    double index = 1;
    for( index = 1; index <= m_distanceSteps
            && nearestPoints->size() < m_maxResultPointCount; index++ )
    {
        nearestPoints->resize( 0 );
        nearestPoints->reserve( 0 );
        double maxDistance = m_maxSearchDistance * pow( 2.0, - ( double )m_distanceSteps + ( double )index );
        fetchNearestPoints( m_examinedKdTree, nearestPoints, maxDistance );
        //cout << "Attempt at max distance: " << maxDistance << "    size = " << nearestPoints->size() << endl;
    }
    std::sort( nearestPoints->begin(), nearestPoints->end() );
    //cout << "Found " << nearestPoints->size() << " Neighbors on index " << (index - 1)
    //      << " with maximal disstance " << nearestPoints->at(nearestPoints->size() - 1).getDistance() << endl;
    if( nearestPoints->size() > m_maxResultPointCount )
    {
        nearestPoints->resize( m_maxResultPointCount );
        nearestPoints->reserve( m_maxResultPointCount );
    }
    return nearestPoints;
}

void WPointSearcher::setExaminedKdTree( WKdTreeND* kdTree )
{
    m_examinedKdTree = kdTree;
}
void WPointSearcher::setSearchedPoint( vector<double> searchedPoint )
{
    m_searchedPoint = searchedPoint;
}
void WPointSearcher::setMaxSearchDistance( double distance )
{
    m_maxSearchDistance = distance;
}
void WPointSearcher::setMaxResultPointCount( size_t maxPointCount )
{
    m_maxResultPointCount = maxPointCount;
}
void WPointSearcher::fetchNearestPoints( WKdTreeND* currentNode, vector<WPointDistance>* targetPoints, double maxDistance )
{
    vector<vector<double> >* nodePoints = currentNode->getNodePoints();
    WKdTreeND* lowerChild = currentNode->getLowerChild();
    WKdTreeND* higherChild = currentNode->getHigherChild();
    if( nodePoints->size() > 0 && lowerChild == 0 && higherChild == 0)
    {
        for( size_t index = 0; index < nodePoints->size(); index++ )
        {
            vector<double> point = nodePoints->at( index );
            if( WPointDistance::getPointDistance( m_searchedPoint, point ) <= maxDistance )
                targetPoints->push_back( WPointDistance( m_searchedPoint, point ) );
        }
    }
    else
    {
        if( nodePoints->size() == 0 && lowerChild != 0 && higherChild != 0 )
        {
            double splittingPosition = currentNode->getSplittingPosition();
            double pointCoord = m_searchedPoint.at( currentNode->getSplittingDimension() );
            if( pointCoord < splittingPosition + maxDistance )
                fetchNearestPoints( currentNode->getLowerChild(), targetPoints, maxDistance );
            if( pointCoord > splittingPosition - maxDistance )
                fetchNearestPoints( currentNode->getHigherChild(), targetPoints, maxDistance );
        }
        else
        {
            cout << "!!!UNKNOWN EXCEPTION!!! - getting nearest points" << endl;
        }
    }
}
