//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#include "WCutOutliersDeamon.h"
#include "structure/WOctNode2.h"
#include "structure/WOctree2.h"

WCutOutliersDeamon::WCutOutliersDeamon()
{
    m_detailDepth = 1;
}

WCutOutliersDeamon::~WCutOutliersDeamon()
{
}

boost::shared_ptr< WDataSetPoints > WCutOutliersDeamon::cutOutliers( boost::shared_ptr< WDataSetPoints > points )
{
    WDataSetPoints::VertexArray verts = points->getVertices();
    WDataSetPoints::ColorArray colors = points->getColors();
    size_t count = verts->size()/3;
    WOctree2* octree = new WOctree2( m_detailDepth );

    for  ( size_t vertex = 0; vertex < count; vertex++)
    {
        double x = verts->at( vertex*3 );
        double y = verts->at( vertex*3+1 );
        double z = verts->at( vertex*3+2 );
        octree->registerPoint( x, y, z );
    }
    octree->groupNeighbourLeafs();

    resizeItemCountGroupList( 0 );
    countGroups( octree->getRootNode() );
    size_t largestGroup = 0;
    size_t largestGroupNodeCount = 0;
    for( size_t index = 0; index < m_pointCounts.size(); index++ )
        if( m_pointCounts[index] > largestGroupNodeCount )
        {
            largestGroup = index;
            largestGroupNodeCount = m_pointCounts[index];
        }

    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );

    for( size_t vertex = 0; vertex < count; vertex++ )
    {
        double x = verts->at( vertex*3 );
        double y = verts->at( vertex*3+1 );
        double z = verts->at( vertex*3+2 );
        WOctNode2* node = octree->getLeafNode( x, y, z );
        if(node != 0 && node->getGroupNr() == largestGroup )
        {
            float r = colors->at( vertex*3 );
            float g = colors->at( vertex*3+1 );
            float b = colors->at( vertex*3+2 );
            outVertices->push_back( x );
            outVertices->push_back( y );
            outVertices->push_back( z );
            outColors->push_back( r );
            outColors->push_back( g );
            outColors->push_back( b );
        }
    }

    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
}

void WCutOutliersDeamon::countGroups( WOctNode2* node )
{
    if  ( node->getRadius() <= m_detailDepth )
    {
        size_t groupNr = node->getGroupNr();
        if( groupNr >= m_pointCounts.size() )
            resizeItemCountGroupList( groupNr + 1 );
        m_pointCounts[groupNr] = m_pointCounts[groupNr] + 1;
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                countGroups( node->getChild( child ) );
    }
}


void WCutOutliersDeamon::resizeItemCountGroupList( size_t newSize )
{
    size_t oldSize = m_pointCounts.size();
    m_pointCounts.reserve( newSize );
    m_pointCounts.resize( newSize );
    if( newSize > oldSize )
        for( size_t index = oldSize; index < newSize; index++)
            m_pointCounts[index] = 0;
}

void WCutOutliersDeamon::setDetailDepth( double detailDepth )
{
    m_detailDepth = detailDepth;
}
