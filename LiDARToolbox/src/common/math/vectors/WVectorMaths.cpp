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

#include <vector>
#include "WVectorMaths.h"

WVectorMaths::WVectorMaths()
{
}
WVectorMaths::~WVectorMaths()
{
}
const double WVectorMaths::ANGLE_90_DEGREES = asin( 1.0 );

double WVectorMaths::getAngleOfVectors( vector<double> vector1, vector<double> vector2 )
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
    double angle = abs( sum ) < 1.0 //double has ohly 1,0000000000000002 and no 1.0 what makes problems for acos( sum )
            ?acos( sum ) * 90.0 / ANGLE_90_DEGREES
            :90.0 - 90.0 * ( sum / abs( sum ) );
    return angle;
}
double WVectorMaths::getAngleOfPlanes( vector<double> vector1, vector<double> vector2 )
{
    double angle = getAngleOfVectors( vector1, vector2 );
    return angle > 90.0 ?180.0 - angle :angle;
}
double WVectorMaths::getAngleOfPlanes( WVector3d vector1, WVector3d vector2 )
{
    vector<double> unidimensionalVector1( vector1.size(), 0.0 );
    vector<double> unidimensionalVector2( vector2.size(), 0.0 );
    for( size_t dimension = 0; dimension < vector1.size(); dimension++ )
    {
        unidimensionalVector1[dimension] = vector1[dimension];
        unidimensionalVector2[dimension] = vector2[dimension];
    }
    return getAngleOfPlanes( unidimensionalVector1, unidimensionalVector2 );
}
