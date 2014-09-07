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
#include "WPcaDetectOctNode.h"

WPcaDetectOctNode::WPcaDetectOctNode()
{
    m_inputPoints = new vector<WPosition>();
    m_hasEigenValueQuotient = false;
}

WPcaDetectOctNode::WPcaDetectOctNode( double centerX, double centerY, double centerZ, double radius ) :
        WOctNode( centerX, centerY, centerZ, radius )
{
    m_inputPoints = new vector<WPosition>();
    m_hasEigenValueQuotient = false;
}

WPcaDetectOctNode::~WPcaDetectOctNode()
{
}

WOctNode* WPcaDetectOctNode::newInstance( double centerX, double centerY, double centerZ, double radius )
{
    return new WPcaDetectOctNode( centerX, centerY, centerZ, radius );
}
void WPcaDetectOctNode::onTouchPosition( double x, double y, double z )
{
    m_inputPoints->push_back( WPosition( x, y, z ) );
}
vector<WPosition>* WPcaDetectOctNode::getInputPoints()
{
    return m_inputPoints;
}
void WPcaDetectOctNode::setEigenValueQuotient( double eigenValueQuotient )
{
    m_eigenValueQuotient = eigenValueQuotient;
    m_hasEigenValueQuotient = true;
}

double WPcaDetectOctNode::getEigenValueQuotient()
{
    return m_eigenValueQuotient;
}
bool WPcaDetectOctNode::hasEigenValueQuotient()
{
    return m_hasEigenValueQuotient;
}
void WPcaDetectOctNode::clearInputData()
{
    m_inputPoints->resize( 0 );
    m_inputPoints->reserve( 0 );
}
