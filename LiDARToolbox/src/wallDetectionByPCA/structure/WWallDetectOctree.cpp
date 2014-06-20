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
#include "WWallDetectOctree.h"

WWallDetectOctree::WWallDetectOctree( double detailDepth ) : WOctree( detailDepth,
            new WWallDetectOctNode( 0.0, 0.0, 0.0, detailDepth ) )
{
}

WWallDetectOctree::~WWallDetectOctree()
{
    m_wallMaxAngleToNeighborVoxel = 20.0;
}
/*vector<WOctNode*> WWallDetectOctree::getNeighborsOfNode( WOctNode* node )
{
    double x = node->getCenter( 0 );
    double y = node->getCenter( 1 );
    double z = node->getCenter( 2 );
    double d = node->getRadius() * 2.0;
    vector<WOctNode*> neighbors;    //TODO(aschwarzkopf): Some elements are missing and
                    //check by additional bool variable whether has been checked instead.
    neighbors.push_back( getLeafNode( x-d, y-d, z ) );
    neighbors.push_back( getLeafNode( x  , y-d, z ) );
    neighbors.push_back( getLeafNode( x+d, y-d, z ) );
    neighbors.push_back( getLeafNode( x-d, y  , z ) );
    neighbors.push_back( getLeafNode( x-d, y-d, z-d ) );
    neighbors.push_back( getLeafNode( x  , y-d, z-d ) );
    neighbors.push_back( getLeafNode( x+d, y-d, z-d ) );
    neighbors.push_back( getLeafNode( x-d, y  , z-d ) );
    neighbors.push_back( getLeafNode( x  , y  , z-d ) );
    neighbors.push_back( getLeafNode( x+d, y  , z-d ) );
    neighbors.push_back( getLeafNode( x-d, y+d, z-d ) );
    neighbors.push_back( getLeafNode( x  , y+d, z-d ) );
    neighbors.push_back( getLeafNode( x+d, y+d, z-d ) );
    return neighbors;
}*/
bool WWallDetectOctree::canGroupNodes( WOctNode* octNode1, WOctNode* octNode2 )
{
    WWallDetectOctNode* node1 = ( WWallDetectOctNode* ) octNode1;
    WWallDetectOctNode* node2 = ( WWallDetectOctNode* ) octNode2;
    if( !( node1->hasEigenValuesAndVectors() ) || !( node2->hasEigenValuesAndVectors() )
            || node1->getPointCount() < m_minimalPointsPerVoxel
            || node2->getPointCount() < m_minimalPointsPerVoxel
            || isIsotropicNode( node1 ) || isIsotropicNode( node2 ) )
        return false;

    if( isLinearNode( node1 ) && isLinearNode( node2 ) )
        return getAngleOfNormals( node1->getStrongestEigenVector(), node2->getStrongestEigenVector() )
                <= m_wallMaxAngleToNeighborVoxel;
    if( isLinearNode( node1 ) && !isLinearNode( node2 ) )
        return getAngleOfNormals( node1->getStrongestEigenVector(), node2->getNormalVector() )
                > 90.0 - m_wallMaxAngleToNeighborVoxel;
    if( !isLinearNode( node1 ) && isLinearNode( node2 ) )
        return getAngleOfNormals( node1->getNormalVector(), node2->getStrongestEigenVector() )
                > 90.0 - m_wallMaxAngleToNeighborVoxel;

    return getAngleOfNormals( node1->getNormalVector(), node2->getNormalVector() )
            <= m_wallMaxAngleToNeighborVoxel;
}
void WWallDetectOctree::setWallMaxAngleToNeighborVoxel( double angleDegrees )
{
    m_wallMaxAngleToNeighborVoxel = angleDegrees;
}
void WWallDetectOctree::setMinimalPointsPerVoxel( size_t minimalPointsPerVoxel )
{
    m_minimalPointsPerVoxel = minimalPointsPerVoxel;
}
void WWallDetectOctree::setEigenValueQuotientLinear( double linearThreshold )
{
    m_eigenValueQuotientLinear = linearThreshold;
}
void WWallDetectOctree::setMaxIsotropicThresholdForVoxelMerge( double isotropicThreshold )
{
    m_maxIsotropicThresholdForVoxelMerge = isotropicThreshold;
}
double WWallDetectOctree::getAngleOfVectors( WVector3d vector1, WVector3d vector2 )
{
    double sum = 0;
    double range1 = 0;
    double range2 = 0;
    for( size_t dimension = 0; dimension < vector1.size() && dimension < vector2.size(); dimension++ )
    {
        sum += vector1[dimension] * vector2[dimension];
        range1 += pow( vector1[dimension], 2.0 );
        range2 += pow( vector2[dimension], 2.0 );
    }
    sum = sum / pow( range1, 0.5 ) / pow( range2, 0.5 );
    return acos( sum ) * 90.0 / ANGLE_90_DEGREES;
}
double WWallDetectOctree::getAngleOfNormals( WVector3d vector1, WVector3d vector2 )
{
    double angle = getAngleOfVectors( vector1, vector2 );
    return angle > 90.0 ?180.0 - angle :angle;
}
bool WWallDetectOctree::isLinearNode( WWallDetectOctNode* node )
{
    if( isIsotropicNode( node ) )
        return false;
    return node->getLinearLevel() <= m_eigenValueQuotientLinear;
}
bool WWallDetectOctree::isIsotropicNode( WWallDetectOctNode* node )
{
    return node->getIsotropicLevel() > m_maxIsotropicThresholdForVoxelMerge;
}
const double WWallDetectOctree::ANGLE_90_DEGREES = asin( 1.0 );

size_t WWallDetectOctree::getNodeCountOfGroup( size_t groupNr )
{
    if(groupNr >= m_nodeCountsOfGroups.size() )
        return 0;
    return m_nodeCountsOfGroups[groupNr];
}
void WWallDetectOctree::generateNodeCountsOfGroups()
{
    m_nodeCountsOfGroups.reserve( 0 );
    m_nodeCountsOfGroups.resize( 0 );
    addGroupCountsFromNode( ( WWallDetectOctNode* )getRootNode() );
}
void WWallDetectOctree::addGroupCountsFromNode( WWallDetectOctNode* node )
{
    if  ( node->getRadius() <= getDetailLevel() )
    {
        if( node->hasGroup() == false )
            return;
        size_t groupNr = node->getGroupNr();
        while( m_nodeCountsOfGroups.size() <= groupNr )
            m_nodeCountsOfGroups.push_back( 0 );
        m_nodeCountsOfGroups[groupNr]++;
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                addGroupCountsFromNode( ( WWallDetectOctNode* )( node->getChild( child ) ) );
    }
}
