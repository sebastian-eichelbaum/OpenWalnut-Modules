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
#include "WCumulatedGroupInfo.h"

WCumulatedGroupInfo::WCumulatedGroupInfo()
{
    m_cumulatedGroupCount = 0;
    m_pointCorrectnessCumulated = 0.0;
    m_pointCompletenessCumulated = 0.0;
    m_areaPointCompletenessCumulated = 0.0;
    m_groupsRangeMinPointCount = 1;
    m_groupsRangeMaxPointCount = 0;
}

WCumulatedGroupInfo::~WCumulatedGroupInfo()
{
}


bool WCumulatedGroupInfo::canBeCumulated( WGroupInfo* group )
{
    return group->getReferenceGroupPointCount() >= m_groupsRangeMinPointCount &&
            group->getReferenceGroupPointCount() <= m_groupsRangeMaxPointCount;
}

bool WCumulatedGroupInfo::cumulateGroup( WGroupInfo* group )
{
    if( !canBeCumulated( group ) )
        return false;

    m_cumulatedGroupCount++;
    m_pointCorrectnessCumulated += group->getCorrectness();
    m_pointCompletenessCumulated += group->getCompletess();
    m_areaPointCompletenessCumulated += group->getAreaCompleteness();
    return true;
}

size_t WCumulatedGroupInfo::getGroupCount()
{
    return m_cumulatedGroupCount;
}

double WCumulatedGroupInfo::getPointCorrectness()
{
    return m_pointCorrectnessCumulated / static_cast<double>( getGroupCount() );
}

double WCumulatedGroupInfo::getPointCompleteness()
{
    return m_pointCompletenessCumulated / static_cast<double>( getGroupCount() );
}

double WCumulatedGroupInfo::getAreaPointCorrectness()
{
    return m_areaPointCompletenessCumulated / static_cast<double>( getGroupCount() );
}

size_t WCumulatedGroupInfo::getRangeMinPointCount()
{
    return m_groupsRangeMinPointCount;
}

size_t WCumulatedGroupInfo::getRangeMaxPointCount()
{
    return m_groupsRangeMaxPointCount;
}

bool WCumulatedGroupInfo::hasGroups()
{
    return m_cumulatedGroupCount > 0;
}

void WCumulatedGroupInfo::setPointCountRange( size_t minPointCount, size_t maxPointCount )
{
    m_groupsRangeMinPointCount = minPointCount;
    m_groupsRangeMaxPointCount = maxPointCount;
}
