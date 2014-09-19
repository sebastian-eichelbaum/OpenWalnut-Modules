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

#include "WGroupEdit.h"

WGroupEdit::WGroupEdit()
{
    m_mergeGroups = false;
}

WGroupEdit::~WGroupEdit()
{
}

void WGroupEdit::initProocessBegin()
{
    m_groupSizes.resize( 0 );
    m_groupSizes.reserve( 0 );
    m_lastGroupID = 0;

    WDataSetPointsGrouped::VertexArray newVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    m_vertices = newVertices;
    WDataSetPointsGrouped::ColorArray newColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    m_colors = newColors;
    WDataSetPointsGrouped::GroupArray newGroups(
            new WDataSetPointsGrouped::GroupArray::element_type() );
    m_groups = newGroups;
}

void WGroupEdit::setMergeGroups( bool mergeGroups )
{
    m_mergeGroups = mergeGroups;
}

void WGroupEdit::setGroupSizeThreshold( size_t groupSizeThreshold )
{
    m_groupSizeThreshold = groupSizeThreshold;
}

void WGroupEdit::mergeGroupSet( boost::shared_ptr< WDataSetPointsGrouped > points )
{
    size_t groupIDOffset = m_lastGroupID == 0 ?0 :m_lastGroupID + 1;
    if( !points )
        return;
    WDataSetPointsGrouped::VertexArray vertices = points->getVertices();
    WDataSetPointsGrouped::ColorArray colors = points->getColors();
    WDataSetPointsGrouped::GroupArray groups = points->getGroups();

    for( size_t index = 0; index < vertices->size(); index++ )
        m_vertices->push_back( vertices->at( index ) );
    for( size_t index = 0; index < colors->size(); index++ )
        m_colors->push_back( colors->at( index ) );
    for( size_t index = 0; index < groups->size(); index++ )
    {
        size_t groupID = groups->at( index );
        if( !m_mergeGroups )
            groupID += groupIDOffset;
        m_groups->push_back( groupID );
        if( groupID > m_lastGroupID )
            m_lastGroupID = groupID;

        if( isValidGroupID( groupID ) )
        {
            double currentSize = m_groupSizes.size();
            if( groupID >= currentSize )
            {
                m_groupSizes.reserve( groupID + 1 );
                m_groupSizes.resize( groupID + 1 );
                for( size_t newGroup = currentSize; newGroup <= groupID; newGroup++ )
                    m_groupSizes[newGroup] = 0;
            }
            m_groupSizes[groupID] = m_groupSizes[groupID] + 1;
        }
    }
}

double WGroupEdit::getVertex( size_t pointIndex, size_t dimension )
{
    return m_vertices->at( pointIndex * 3 + dimension );
}

double WGroupEdit::getColor( size_t pointIndex, size_t colorChannel )
{
    return m_colors->at( pointIndex * 3 + colorChannel );
}

size_t WGroupEdit::getOldGroupID( size_t pointIndex )
{
    return m_groups->at( pointIndex );
}

size_t WGroupEdit::getNewGroupID( size_t pointIndex )
{
    size_t originalGroupID = getOldGroupID( pointIndex );
    if( !isValidGroupID( originalGroupID ) || !isPointCollected( pointIndex ) )
        return originalGroupID;
    return m_groupIDMap[originalGroupID];
}

bool WGroupEdit::isPointCollected( size_t pointIndex )
{
    size_t originalGroupID = getOldGroupID( pointIndex );
    if( !isValidGroupID( originalGroupID ) )
        return false;
    return m_groupSizes[originalGroupID] >= m_groupSizeThreshold;
}

size_t WGroupEdit::getInputPointCount()
{
    return m_vertices->size() / 3;
}



size_t WGroupEdit::getLastGroupID()
{
    return m_lastGroupID;
}

size_t WGroupEdit::getGroupSize( size_t origGroupID )
{
    return m_groupSizes[origGroupID];
}

void WGroupEdit::modifyGroupIDs()
{
    m_groupIDMap.reserve( m_groupSizes.size() );
    m_groupIDMap.resize( m_groupSizes.size() );

    size_t currentID = 0;
    for( size_t index = 0; index < m_groupSizes.size(); index++ )
        if( m_groupSizes[index] >= m_groupSizeThreshold )
            m_groupIDMap[index] = currentID++;
    m_lastGroupID = currentID - 1;
}

bool WGroupEdit::isValidGroupID( size_t groupID )
{
    return groupID < 1000 * 1000 * 100;
}
