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

#include "WOctree.h"

WOctree::WOctree( double detailDepth )
{
    m_root = new WOctNode( 0.0, 0.0, 0.0, detailDepth );
    m_detailLevel = detailDepth;
}

WOctree::~WOctree()
{
}

void WOctree::registerPoint( double x, double y, double z )
{
//    std::cout << "Inflating point: " << x << ", " << y << ", " << z << std::endl;
    while  ( !m_root->fitsIn( x, y, z ) || m_root->getRadius() <= m_detailLevel )
        m_root->expand();

    WOctNode* node = m_root;
    while  ( node->getRadius() > m_detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y, z );
        node->touchNode( drawer );
        node = node->getChild( drawer );
    }
}
WOctNode* WOctree::getLeafNode( double x, double y, double z )
{
    if( !m_root->fitsIn( x, y, z ) )
        return 0;

    WOctNode* node = m_root;
    while  ( node->getRadius() > m_detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y, z );
        if( node->getChild( drawer ) == 0 )
            return 0;
        node = node->getChild( drawer );
    }
    return node;
}
WOctNode* WOctree::getRootNode()
{
    return m_root;
}
size_t WOctree::getGroupCount()
{
    return m_groupEquivs.size();
}
void WOctree::groupNeighbourLeafs()
{
    resizeGroupList( 0 );
    groupNeighbourLeafs( m_root );
    size_t currentFinalGroup = 0;
    for( size_t groupID = 0; groupID < m_groupEquivs.size(); groupID++ )
    {
        bool idUsed = false;
        for( size_t index = 0; index < m_groupEquivs.size(); index++ )
        {
            if( m_groupEquivs[index] == groupID )
            {
                m_groupEquivs[index] = currentFinalGroup;
                idUsed = true;
            }
        }
        if( idUsed ) currentFinalGroup++;
    }
    refreshNodeGroup( m_root );
    resizeGroupList( currentFinalGroup );
    for( size_t index = 0; index < currentFinalGroup; index++ )
        m_groupEquivs[index] = index;
    std::cout << "Found " << currentFinalGroup << " groups." << std::endl;
}
void WOctree::refreshNodeGroup( WOctNode* node )
{
    if  ( node->getRadius() <= m_detailLevel )
    {
        size_t oldGroupNr = node->getGroupNr();
        node->setGroupNr( m_groupEquivs[oldGroupNr] );
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                refreshNodeGroup( node->getChild( child ) );
    }
}
void WOctree::groupNeighbourLeafs( WOctNode* node )
{
    if  ( node->getRadius() <= m_detailLevel )
    {
        size_t group = m_groupEquivs.size();
        double x = node->getCenter( 0 );
        double y = node->getCenter( 1 );
        double z = node->getCenter( 2 );
        double d = node->getRadius() * 2.0;
        WOctNode* neighbors[3] = {
//            getLeafNode( x-d, y-d, z ),
            getLeafNode( x  , y-d, z ),
//            getLeafNode( x+d, y-d, z ),
            getLeafNode( x-d, y  , z ),
//            getLeafNode( x-d, y-d, z-d ),
//            getLeafNode( x  , y-d, z-d ),
//            getLeafNode( x+d, y-d, z-d ),
//            getLeafNode( x-d, y  , z-d ),
            getLeafNode( x  , y  , z-d ),
//            getLeafNode( x+d, y  , z-d ),
//            getLeafNode( x-d, y+d, z-d ),
//            getLeafNode( x  , y+d, z-d )
//            getLeafNode( x+d, y+d, z-d )
        };
        for( size_t index = 0; index < 3; index++ )
        {
            WOctNode* neighbor = neighbors[index];
            size_t neighborID = neighbor == 0 ?group :m_groupEquivs[neighbor->getGroupNr()];
            if( group > neighborID && m_detailLevel == neighbor->getRadius() )
                group = neighborID;
        }
        for( size_t index = 0; index < 3; index++ )
        {
            WOctNode* neighbor = neighbors[index];
            size_t neighborID = neighbor == 0 ?group :m_groupEquivs[neighbor->getGroupNr()];
            if( group < neighborID && m_detailLevel == neighbor->getRadius() )
            {
                for( size_t newIdx = 0; newIdx < m_groupEquivs.size(); newIdx++ )
                    if( m_groupEquivs[newIdx] == neighborID )
                        m_groupEquivs[newIdx] = group;
            }
        }
        if( group >= m_groupEquivs.size() )
        {
            resizeGroupList( group + 1 );
            m_groupEquivs[group] = group;
        }
        node->setGroupNr( group );
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                groupNeighbourLeafs( node->getChild( child ) );
    }
}
void WOctree::resizeGroupList( size_t listLength )
{
    m_groupEquivs.resize( listLength );
    m_groupEquivs.reserve( listLength );
}


boost::shared_ptr< WTriangleMesh > WOctree::getOutline()
{
    boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
    drawNode( m_root, tmpMesh );
    return tmpMesh;
}

void WOctree::drawNode( WOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    if  ( node->getRadius() <= m_detailLevel )
    {
        size_t index = outputMesh->vertSize();
        for( size_t vertex = 0; vertex <= 8; vertex++ )
        {
            double iX = vertex % 2;
            double iY = ( vertex / 2 ) % 2;
            double iZ = ( vertex / 4 ) % 2;
            double x = node->getCenter( 0 ) + node->getRadius() * ( iX * 2.0 - 1.0 );
            double y = node->getCenter( 1 ) + node->getRadius() * ( iY * 2.0 - 1.0 );
            double z = node->getCenter( 2 ) + node->getRadius() * ( iZ * 2.0 - 1.0 );
            outputMesh->addVertex( x, y, z );
        }
        // Z = 0
        outputMesh->addTriangle( index + 0, index + 2, index + 1 );
        outputMesh->addTriangle( index + 3, index + 1, index + 2 );
        // X = 0
        outputMesh->addTriangle( index + 0, index + 4, index + 2 );
        outputMesh->addTriangle( index + 4, index + 6, index + 2 );
        // Y = 0
        outputMesh->addTriangle( index + 0, index + 1, index + 4 );
        outputMesh->addTriangle( index + 1, index + 5, index + 4 );
        // Z = 1
        outputMesh->addTriangle( index + 4, index + 5, index + 6 );
        outputMesh->addTriangle( index + 5, index + 7, index + 6 );
        // X = 1
        outputMesh->addTriangle( index + 1, index + 3, index + 5 );
        outputMesh->addTriangle( index + 3, index + 7, index + 5 );
        // Y = 1
        outputMesh->addTriangle( index + 2, index + 6, index + 3 );
        outputMesh->addTriangle( index + 6, index + 7, index + 3 );
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                drawNode( node->getChild( child ), outputMesh );
    }
}


