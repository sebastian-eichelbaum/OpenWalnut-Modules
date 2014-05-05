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
#include "WOctree.h"

WOctree::WOctree( double detailDepth )
{
    m_root = new WOctNode( 0.0, 0.0, 0.0, detailDepth );
    m_detailLevel = detailDepth;
}
WOctree::WOctree( double detailDepth, WOctNode* nodeType )
{
    m_root = nodeType->newInstance( 0.0, 0.0, 0.0, detailDepth );
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
    node->touchPosition( x, y, z );
    while  ( node->getRadius() > m_detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y, z );
        node->touchNode( drawer );
        node = node->getChild( drawer );
        node->touchPosition( x, y, z );
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
void WOctree::groupNeighbourLeafsFromRoot()
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
//    std::cout << "Found " << currentFinalGroup << " groups." << std::endl;
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
        vector<WOctNode*> neighbors = getNeighborsOfNode( node );
        for( size_t index = 0; index < neighbors.size(); index++ )
        {
            WOctNode* neighbor = neighbors[index];
            size_t neighborID = neighbor == 0 ?group :m_groupEquivs[neighbor->getGroupNr()];
            if( group > neighborID && m_detailLevel == neighbor->getRadius() )
                if( neighbor->hasGroup() )
                    if( canGroupNodes( node, neighbor ) )
                        group = neighborID;
        }
        for( size_t index = 0; index < neighbors.size(); index++ )
        {
            WOctNode* neighbor = neighbors[index];
            size_t neighborID = neighbor == 0 ?group :m_groupEquivs[neighbor->getGroupNr()];
            if( group < neighborID && neighbor->hasGroup() && m_detailLevel == neighbor->getRadius() )
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
vector<WOctNode*> WOctree::getNeighborsOfNode( WOctNode* node )
{
    double x = node->getCenter( 0 );
    double y = node->getCenter( 1 );
    double z = node->getCenter( 2 );
    double d = node->getRadius() * 2.0;
    vector<WOctNode*> neighbors;
    neighbors.push_back( getLeafNode( x  , y-d, z ) );
    neighbors.push_back( getLeafNode( x-d, y  , z ) ),
    neighbors.push_back( getLeafNode( x  , y  , z-d ) );
    return neighbors;
}
bool WOctree::canGroupNodes( WOctNode* node1, WOctNode* node2 )
{
    node1 = node1;
    node2 = node2;
    return true;
}
void WOctree::resizeGroupList( size_t listLength )
{
    m_groupEquivs.resize( listLength );
    m_groupEquivs.reserve( listLength );
}
double WOctree::getDetailLevel()
{
    return m_detailLevel;
}
const size_t WOctree::colors[] = {
        0xff8080, 0xff0000, 0xa00000,   //red
        0xffb080, 0xff8000, 0xb25900,   //orange
        0xffff70, 0xffff00, 0xa0a000,   //yellow
        0x60ff60, 0x00ff00, 0x00b000,   //green
        0x70ffff, 0x00ffff, 0x00b2b2,   //cyan
        0x8080ff, 0x0000ff,             //blue
        0xff80cf, 0xff00a0, 0xb20070    //pink
};
const size_t WOctree::colorCount = sizeof( colors ) / sizeof( colors[0] );
float WOctree::calcColor( size_t groupNr, size_t colorChannel )
{
    groupNr = colors[groupNr%colorCount];
    groupNr = ( groupNr >> ( 16-8*colorChannel ) ) & 0xff;
    return static_cast<float>(groupNr)/255.0f;
}
