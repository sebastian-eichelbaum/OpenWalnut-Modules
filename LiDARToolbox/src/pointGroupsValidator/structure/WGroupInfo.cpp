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
#include "WGroupInfo.h"

WGroupInfo::WGroupInfo()
{
    m_referenceGroupPointCount = 0;
    m_correctlyDetectedPointCount = 0;
    m_uncorrectlyDetectedPointCount = 0;
}

WGroupInfo::~WGroupInfo()
{
}


size_t WGroupInfo::getAreaMissingPointCount()
{
    return m_pointCountOfMissingAreas;
}

double WGroupInfo::getAreaCompleteness()
{
    double falseNegatives = static_cast<double>( getAreaMissingPointCount() );
    double truePositives = static_cast<double>( getReferenceGroupPointCount() ) - falseNegatives;
    return truePositives / ( truePositives + falseNegatives );
}

double WGroupInfo::getCompletess()
{
    double truePositives = static_cast<double>( getCorrectlyDetectedPointCount() );
    double falseNegatives = static_cast<double>( getReferenceGroupPointCount()
            - getCorrectlyDetectedPointCount() );
    return truePositives / ( truePositives + falseNegatives );
}

size_t WGroupInfo::getCorrectlyDetectedPointCount()
{
    return m_correctlyDetectedPointCount;
}

double WGroupInfo::getCorrectness()
{
    double truePositives = static_cast<double>( getCorrectlyDetectedPointCount() );
    double falsePositives = static_cast<double>( getUncorrectlyDetectedPointCount() );
    return truePositives / ( truePositives + falsePositives );
}

size_t WGroupInfo::getReferenceGroupID()
{
    return m_referenceGroupID;
}

size_t WGroupInfo::getReferenceGroupPointCount()
{
    return m_referenceGroupPointCount;
}

size_t WGroupInfo::getValidatedGroupID()
{
    return m_validatedGroupID;
}

size_t WGroupInfo::getUncorrectlyDetectedPointCount()
{
    return m_uncorrectlyDetectedPointCount;
}

bool WGroupInfo::isCertainlyDetected()
{
    return isCertainlyDetectedByCompleteness() && isCertainlyDetectedByPointAreaCompleteness()
            && isCertainlyDetectedByCorectness();
}

bool WGroupInfo::isCertainlyDetectedByCompleteness()
{
    return getCompletess() >= m_minimalPointCompleteness;
}

bool WGroupInfo::isCertainlyDetectedByPointAreaCompleteness()
{
    return getAreaCompleteness() >= m_minimalpointAreaCompleteness;
}

bool WGroupInfo::isCertainlyDetectedByCorectness()
{
    return getCorrectness() >= m_minimalPointCorrectness;
}






void WGroupInfo::setReferenceGroupID( size_t groupID )
{
    m_referenceGroupID = groupID;
}

void WGroupInfo::setValidatedGroupID( size_t groupID )
{
    m_validatedGroupID = groupID;
}

void WGroupInfo::setReferenceGroupPointCount( size_t referenceGroupPointCount )
{
    m_referenceGroupPointCount = referenceGroupPointCount;
}

void WGroupInfo::setCorrectlyDetectedPointCount( size_t correctlyDetectedPointCount )
{
    m_correctlyDetectedPointCount = correctlyDetectedPointCount;
}

void WGroupInfo::setUncorrectlyDetectedPointCount( size_t uncorrectlyDetectedPointCount )
{
    m_uncorrectlyDetectedPointCount = uncorrectlyDetectedPointCount;
}

void WGroupInfo::setPointCountOfMissingAreas( size_t pointCountOfMissingAreas )
{
    m_pointCountOfMissingAreas = pointCountOfMissingAreas;
}

void WGroupInfo::setMinimalPointCompleteness( double minPointCompleteness )
{
    m_minimalPointCompleteness = minPointCompleteness;
}

void WGroupInfo::setMinimalPointAreaCompleteness( double minPointAreaCompleteness )
{
    m_minimalpointAreaCompleteness = minPointAreaCompleteness;
}

void WGroupInfo::setMinimalPointCorrectness( double minPointCorrectness )
{
    m_minimalPointCorrectness = minPointCorrectness;
}
