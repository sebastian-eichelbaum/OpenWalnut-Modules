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
    m_eigenValues[3];
}

WWallDetectOctNode::WWallDetectOctNode( double centerX, double centerY, double centerZ, double radius ) :
        WOctNode( centerX, centerY, centerZ, radius )
{
    m_eigenValues[3];
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
WPosition WWallDetectOctNode::getMean()
{
    return m_mean;
}
void WWallDetectOctNode::setMean( WPosition mean )
{
    m_mean = mean;
}
double WWallDetectOctNode::getLinearLevel()
{
    return m_eigenValues.size() < 2 || m_eigenValues[0] == 0.0
            ?1.0 :m_eigenValues[1] / m_eigenValues[0];
}
double WWallDetectOctNode::getIsotropicLevel()
{
    return m_eigenValues.size() < 3 || m_eigenValues[0] == 0.0
            ?1.0 :m_eigenValues[2] / m_eigenValues[0];
}
vector<double> WWallDetectOctNode::getEigenValues()
{
    return m_eigenValues;
}
void WWallDetectOctNode::setEigenValues( vector<double> eigenValues )
{
    m_eigenValues = eigenValues;
}
WVector3d WWallDetectOctNode::getNormalVector()
{
    return m_eigenVectors[2];
}
WVector3d WWallDetectOctNode::getStrongestEigenVector()
{
    return m_eigenVectors[0];
}
void WWallDetectOctNode::setEigenVectors( vector<WVector3d> eigenVectors )
{
    m_eigenVectors = eigenVectors;
}
bool WWallDetectOctNode::hasEigenValuesAndVectors()
{
    return m_eigenValues.size() >= 3 && m_eigenVectors.size() >= 3;
}
WVector3d WWallDetectOctNode::getEigenVector( size_t index )
{
    return m_eigenVectors[index];
}
