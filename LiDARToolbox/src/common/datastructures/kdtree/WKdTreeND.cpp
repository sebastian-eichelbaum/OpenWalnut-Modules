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
#include <algorithm>
#include <vector>
#include "WKdTreeND.h"

using std::cout;
using std::endl;
WKdTreeND::WKdTreeND()
{
    m_dimensions = 3;
    m_splittingDimension = 0;
    m_splittingPosition = 0.0;
    m_allowDoubles = true;
    m_points = new vector<WKdPointND* >();
    m_parentSplittingDimension = 3;
    m_hierarchyLevel = 0;
    m_higherChild = 0;
    m_lowerChild = 0;
}

WKdTreeND::WKdTreeND( size_t dimensions )
{
    m_dimensions = dimensions;
    m_splittingDimension = 0;
    m_splittingPosition = 0.0;
    m_allowDoubles = true;
    m_points = new vector<WKdPointND* >();
    m_parentSplittingDimension = dimensions;
    m_hierarchyLevel = 0;
    m_higherChild = 0;
    m_lowerChild = 0;
}

WKdTreeND::~WKdTreeND()
{
    for( size_t index = 0; index < m_points->size(); index++ )
        delete m_points->at( index );
    delete m_points;
    if( m_lowerChild != 0 )
    {
        delete m_lowerChild;
    }
    if( m_higherChild != 0 )
    {
        delete m_higherChild;
    }
}

void WKdTreeND::add( vector<WKdPointND*>* addables )
{
    if( addables->size() == 0 )
        return;
    for( size_t index = 0; index < addables->size(); index++ )
        m_points->push_back( addables->at( index ) );

    if( m_lowerChild == 0 && m_lowerChild == 0 )
    {
        determineNewSplittingDimension( m_points );
        if( canSplit() )
        {
            if( m_points->size() == 2 )
            {
                double point1Scalar = m_points->at( 0 )->getCoordinate()[m_splittingDimension];
                double point2Scalar = m_points->at( 1 )->getCoordinate()[m_splittingDimension];
                m_splittingPosition = ( point1Scalar + point2Scalar ) / 2.0;
                if( m_splittingPosition == point2Scalar && point1Scalar > point2Scalar )
                    m_splittingPosition = point1Scalar;
                if( m_splittingPosition == point1Scalar && point2Scalar > point1Scalar )
                    m_splittingPosition = point2Scalar;
            }
            else
            {
                calculateSplittingPosition( m_points );
            }
            initSubNodes();
            addPointsToChildren( m_points );
            m_points->resize( 0 );
            m_points->reserve( 0 );
        }
    }
    else
    {
        if( m_lowerChild != 0 && m_lowerChild != 0 )
        {
            addPointsToChildren( m_points );
            m_points->resize( 0 );
            m_points->reserve( 0 );
        }
        else
        {
            cout << "!!!UNKNOWN EXCEPTION!!! - adding items" << endl;
        }
    }
}

bool WKdTreeND::canSplit()
{
    return m_splittingDimension < m_dimensions;
}

void WKdTreeND::assignZeroToPointers()
{
    m_lowerChild = 0;
    m_higherChild = 0;
    m_points->resize( 0 );
    m_points->reserve( 0 );
}

void WKdTreeND::copyPointersFrom( WKdTreeND* copiedNode )
{
    m_parentSplittingDimension = copiedNode->m_parentSplittingDimension;
    m_lowerChild = copiedNode->m_lowerChild;
    m_higherChild = copiedNode->m_higherChild;
    m_dimensions = copiedNode->m_dimensions;
    m_splittingDimension = copiedNode->m_splittingDimension;
    m_splittingPosition = copiedNode->m_splittingPosition;
    m_allowDoubles = copiedNode->m_allowDoubles;
    m_points->resize( 0 );
    m_points->reserve( 0 );
    for( size_t index = 0; index < copiedNode->m_points->size(); index++ )
        m_points->push_back( copiedNode->m_points->at( index ) );
}

void WKdTreeND::fetchPoints( vector<WKdPointND* >* targetPointSet )
{
    if( m_points->size() > 0 && m_lowerChild == 0 && m_lowerChild == 0 )
    {
        for(size_t index = 0; index < m_points->size(); index++)
            targetPointSet->push_back( m_points->at( index ) );
    }
    else
    {
        if( m_points->size() == 0 && m_lowerChild != 0 && m_lowerChild != 0 )
        {
            m_lowerChild->fetchPoints( targetPointSet );
            m_higherChild->fetchPoints( targetPointSet );
        }
        else
        {
            if( m_points->size() != 0 || m_lowerChild != 0 || m_lowerChild != 0 )
                cout << "!!!UNKNOWN EXCEPTION!!!" << endl;
        }
    }
}

vector<WKdPointND*>* WKdTreeND::getAllPoints()
{
    vector<WKdPointND*>* outputPoints = new vector<WKdPointND*>();
    vector<WKdTreeND*>* kdNodes = getAllLeafNodes();
    for( size_t nodeIndex = 0; nodeIndex < kdNodes->size(); nodeIndex++ )
        for( size_t pointIndex = 0; pointIndex < kdNodes->at( nodeIndex )->getNodePoints()->size(); pointIndex++ )
            outputPoints->push_back( kdNodes->at( nodeIndex )->getNodePoints()->at( pointIndex ) );
    kdNodes->resize( 0 );
    kdNodes->reserve( 0 );
    return outputPoints;
}

vector<WKdTreeND*>* WKdTreeND::getAllLeafNodes()
{
    vector<WKdTreeND*>* outputLeafNodes = new vector<WKdTreeND*>();
    fetchAllLeafNodes( outputLeafNodes );
    return outputLeafNodes;
}

size_t WKdTreeND::getDimensions()
{
    return m_dimensions;
}

WKdTreeND* WKdTreeND::getHigherChild()
{
    return m_higherChild;
}

WKdTreeND* WKdTreeND::getLowerChild()
{
    return m_lowerChild;
}

vector<WKdPointND*>* WKdTreeND::getNodePoints()
{
    return m_points;
}

size_t WKdTreeND::getSplittingDimension()
{
    return m_splittingDimension;
}

bool WKdTreeND::isEmpty()
{
    return m_points->size() == 0 && m_lowerChild == 0 && m_lowerChild == 0;
}

bool WKdTreeND::isLowerKdNodeCase( double position )
{
    return position < m_splittingPosition;
}

bool WKdTreeND::removePoint( WKdPointND* removablePoint )
{
    if( m_points->size() > 0 && m_lowerChild == 0 && m_lowerChild == 0 )
    {
        size_t keptNodeCount = 0;
        size_t size = m_points->size();
        for(size_t index = 0; index < size; index++)
            if( m_points->at( index ) != removablePoint )
            {
                if( index > keptNodeCount )
                    m_points->at( keptNodeCount ) = m_points->at( index );
                keptNodeCount++;
            }
        m_points->resize( keptNodeCount );
        m_points->reserve( keptNodeCount );
        return keptNodeCount < size;
    }
    else
    {
        if( m_points->size() == 0 && m_lowerChild != 0 && m_lowerChild != 0 )
        {
            double position = removablePoint->getCoordinate()[getSplittingDimension()];
            WKdTreeND* deletedNode = isLowerKdNodeCase( position ) ?m_lowerChild :m_higherChild;
            WKdTreeND* copiedNode = !isLowerKdNodeCase( position ) ?m_lowerChild :m_higherChild;

            bool couldFind = deletedNode->removePoint( removablePoint );
            if( deletedNode->isEmpty() )
            {
                delete deletedNode;
                copyPointersFrom( copiedNode );
                copiedNode->assignZeroToPointers();
                delete copiedNode;
            }
            return couldFind;
        }
        else
        {
            if( m_points->size() != 0 || m_lowerChild != 0 || m_lowerChild != 0 )
                cout << "!!!UNKNOWN EXCEPTION!!!" << endl;
        }
    }
    return false;
}

WKdTreeND* WKdTreeND::getNewInstance( size_t dimensions )
{
    return new WKdTreeND( dimensions );
}

void WKdTreeND::addPointsToChildren( vector<WKdPointND* >* newPoints )
{
    vector<WKdPointND* >* lowerPoints = new vector<WKdPointND* >();
    vector<WKdPointND* >* higherPoints = new vector<WKdPointND* >();
    for( size_t index = 0; index < newPoints->size(); index++ )
    {
        WKdPointND* newPoint = newPoints->at( index );
        double position = newPoint->getCoordinate().at( m_splittingDimension );
        if( isLowerKdNodeCase( position ) )
        {
            lowerPoints->push_back( newPoint );
        }
        else
        {
            higherPoints->push_back( newPoint );
        }
    }

    size_t hierarchyLevel = m_hierarchyLevel;
    if( hierarchyLevel > 2 )
    {
        m_lowerChild->add( lowerPoints );
        m_higherChild->add( higherPoints );
    }
    else
    {
        boost::thread lowerChildThread( &WKdTreeND::add, m_lowerChild, lowerPoints );
        boost::thread higherChildThread( &WKdTreeND::add, m_higherChild, higherPoints );
        lowerChildThread.join();
        higherChildThread.join();
    }

    lowerPoints->reserve( 0 );
    lowerPoints->resize( 0 );
    higherPoints->reserve( 0 );
    higherPoints->resize( 0 );
    delete lowerPoints;
    delete higherPoints;
}

void WKdTreeND::calculateSplittingPosition( vector<WKdPointND* >* inputPoints )
{
    if( !canSplit() )
        return;
    size_t count = inputPoints->size();
    vector<double>* line = new vector<double>( inputPoints->size(), 0 );
    for( size_t index = 0; index < inputPoints->size(); index++ )
        line->at( index ) = inputPoints->at( index )->getCoordinate()[m_splittingDimension];
    std::sort( line->begin(), line->end() );
    size_t medianIdx = count / 2;
    m_splittingPosition = line->at( medianIdx );
    size_t medianLeftIdx = medianIdx - 1;
    double medianLeft = line->at( medianLeftIdx );
    bool leftIdxValid = true;
    size_t medianRightIdx = medianIdx;
    double medianRight = line->at( medianRightIdx );
    bool rightIdxValid = true;
    while( rightIdxValid && medianRight == medianLeft )
    {
        if( medianRightIdx + 1 < inputPoints->size() )
        {
            medianRightIdx++;
            medianRight = line->at( medianRightIdx );
        }
        else
        {
            rightIdxValid = false;
        }
    }
    while( leftIdxValid && medianLeftIdx < inputPoints->size() && medianLeft == m_splittingPosition )
    {
        if( medianLeftIdx > 0 )
        {
            medianLeftIdx--;
            medianLeft = line->at( medianLeftIdx );
        }
        else
        {
            leftIdxValid = false;
        }
    }
    if( leftIdxValid && rightIdxValid )
    {
        if( medianRightIdx - medianIdx < medianIdx - medianLeftIdx - 1 )
        {
            medianIdx = medianRightIdx;
        }
        else
        {
            medianIdx = medianLeftIdx + 1;
        }
    }
    else
    {
        if( leftIdxValid )
        {
            medianIdx = medianLeftIdx + 1;
        }
        else
        {
            if( rightIdxValid )
            {
                medianIdx = medianRightIdx;
            }
            else
            {
                cout << "!!!UNKNOWN EXCEPTION!!!" << endl;
            }
        }
    }


    m_splittingPosition = ( line->at( medianIdx - 1 ) + line->at( medianIdx ) ) / 2.0;
    if( m_splittingPosition == line->at( medianIdx - 1 ) )
        m_splittingPosition = line->at( medianIdx );
    delete line;
}

bool WKdTreeND::determineNewSplittingDimension( vector<WKdPointND* >* inputPoints )
{
    m_splittingDimension = m_dimensions;
    if( inputPoints->size() < 2 )
        return false;
    vector<double>* boundingBoxMin = new vector<double>();
    vector<double>* boundingBoxMax = new vector<double>();
    boundingBoxMin->reserve( m_dimensions );
    boundingBoxMin->resize( m_dimensions );
    boundingBoxMax->reserve( m_dimensions );
    boundingBoxMax->resize( m_dimensions );
    for( size_t index = 0; index < inputPoints->size(); index++ )
        for( size_t dimension = 0; dimension < getDimensions(); dimension++ )
        {
            double position = inputPoints->at( index )->getCoordinate().at( dimension );
            if( index == 0 || position < boundingBoxMin->at( dimension ) )
                boundingBoxMin->at( dimension ) = position;
            if( index == 0 || position > boundingBoxMax->at( dimension ) )
                boundingBoxMax->at( dimension ) = position;
        }

    double spreadMax = 0.0;
    for( size_t dimension = 0; dimension < getDimensions(); dimension++ )
    {
        double currentSpread = boundingBoxMax->at( dimension ) - boundingBoxMin->at( dimension );
        if( dimension != m_parentSplittingDimension && currentSpread > spreadMax )
        {
            spreadMax = currentSpread;
            m_splittingDimension = dimension;
        }
    }
    if( m_splittingDimension >= m_dimensions && m_parentSplittingDimension < m_dimensions
            && boundingBoxMax->at( m_parentSplittingDimension ) - boundingBoxMin->at( m_parentSplittingDimension ) > 0.0 )
        m_splittingDimension = m_parentSplittingDimension;
    delete boundingBoxMin;
    delete boundingBoxMax;
    return m_splittingDimension < m_dimensions;
}

void WKdTreeND::fetchAllLeafNodes( vector<WKdTreeND*>* targetNodeList )
{
    if( m_points->size() > 0 && m_lowerChild == 0 && m_higherChild == 0 )
    {
        targetNodeList->push_back( this );
    }
    else
    {
        if( m_points->size() == 0 && m_lowerChild != 0 && m_higherChild != 0 )
        {
            m_lowerChild->fetchAllLeafNodes( targetNodeList );
            m_higherChild->fetchAllLeafNodes( targetNodeList );
        }
        else
        {
            cout << "!!!UNKNOWN EXCEPTION!!! - fetching leaf nodes" << endl;
        }
    }
}

bool WKdTreeND::hasParent()
{
    return m_parentSplittingDimension < m_dimensions;
}

void WKdTreeND::initSubNodes()
{
    m_lowerChild = getNewInstance( m_dimensions );
    m_higherChild = getNewInstance( m_dimensions );
    m_lowerChild->m_parentSplittingDimension = m_splittingDimension;
    m_higherChild->m_parentSplittingDimension = m_splittingDimension;
    m_lowerChild->m_hierarchyLevel = m_hierarchyLevel + 1;
    m_higherChild->m_hierarchyLevel = m_hierarchyLevel + 1;
}
