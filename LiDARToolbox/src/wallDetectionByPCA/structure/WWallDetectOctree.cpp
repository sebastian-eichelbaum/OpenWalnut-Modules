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
vector<WOctNode*> WWallDetectOctree::getNeighborsOfNode( WOctNode* node )
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
//     neighbors.push_back( getLeafNode( x-d, y-d, z-d ) );
    neighbors.push_back( getLeafNode( x  , y-d, z-d ) );
//     neighbors.push_back( getLeafNode( x+d, y-d, z-d ) );
    neighbors.push_back( getLeafNode( x-d, y  , z-d ) );
    neighbors.push_back( getLeafNode( x  , y  , z-d ) );
    neighbors.push_back( getLeafNode( x+d, y  , z-d ) );
//     neighbors.push_back( getLeafNode( x-d, y+d, z-d ) );
    neighbors.push_back( getLeafNode( x  , y+d, z-d ) );
//     neighbors.push_back( getLeafNode( x+d, y+d, z-d ) );
    return neighbors;
}
bool WWallDetectOctree::canGroupNodes( WOctNode* node1, WOctNode* node2 )
{
    WWallDetectOctNode* node1typed = ( WWallDetectOctNode* ) node1;
    WWallDetectOctNode* node2typed = ( WWallDetectOctNode* ) node2;
    if( node1typed->getIsotropicThreshold() > m_maxIsotropicThresholdForVoxelMerge
            || node2typed->getIsotropicThreshold() > m_maxIsotropicThresholdForVoxelMerge )
        return false;
    double angleDegrees = getAngleOfTwoVectors(
            node1typed->getNormalVector(), node2typed->getNormalVector() );
    if( angleDegrees > 90.0 ) angleDegrees = 180 - angleDegrees;
//     cout << angleDegrees << " <= " << m_wallMaxAngleToNeighborVoxel << endl;
    return angleDegrees <= m_wallMaxAngleToNeighborVoxel;
}
void WWallDetectOctree::setWallMaxAngleToNeighborVoxel( double angleDegrees )
{
    m_wallMaxAngleToNeighborVoxel = angleDegrees;
}
void WWallDetectOctree::setMaxIsotropicThresholdForVoxelMerge( double isotropicThreshold )
{
    m_maxIsotropicThresholdForVoxelMerge = isotropicThreshold;
}
double WWallDetectOctree::getAngleOfTwoVectors( WVector3d vector1, WVector3d vector2 )
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
const double WWallDetectOctree::ANGLE_90_DEGREES = asin( 1.0 );
