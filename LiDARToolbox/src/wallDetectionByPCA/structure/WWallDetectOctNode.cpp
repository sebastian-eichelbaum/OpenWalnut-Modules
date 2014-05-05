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
#include "WWallDetectOctNode.h"

WWallDetectOctNode::WWallDetectOctNode()
{
}

WWallDetectOctNode::WWallDetectOctNode( double centerX, double centerY, double centerZ, double radius ) :
        WOctNode( centerX, centerY, centerZ, radius )
{
}

WWallDetectOctNode::~WWallDetectOctNode()
{
}

WOctNode* WWallDetectOctNode::newInstance( double centerX, double centerY, double centerZ, double radius )
{
    return new WWallDetectOctNode( centerX, centerY, centerZ, radius );
}
void WWallDetectOctNode::onTouchPosition( double x, double y, double z )
{
    m_inputPoints.push_back( WPosition( x, y, z ) );
}
vector<WPosition> WWallDetectOctNode::getInputPoints()
{
    return m_inputPoints;
}
double WWallDetectOctNode::getIsotropicThreshold()
{
    return m_isotropicThreshold;
}
void WWallDetectOctNode::setIsotropicThreshold( double isotropicThreshold )
{
    m_isotropicThreshold = isotropicThreshold;
}
WVector3d WWallDetectOctNode::getNormalVector()
{
    return m_normalVector;
}
void WWallDetectOctNode::setNormalVector( WVector3d normalVector )
{
    m_normalVector = normalVector;
}
