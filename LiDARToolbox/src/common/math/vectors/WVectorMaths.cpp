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
const double WVectorMaths::INFINITY_NEGATIVE = -1.0 / 0.0;
const double WVectorMaths::INFINITY_POSITIVE = 1.0 / 0.0;

void WVectorMaths::addVector( vector<double>* changedVector, const vector<double>& summand )
{
    size_t size = changedVector->size();
    for( size_t dimension = 0; dimension < size; dimension++ )
        changedVector->at( dimension ) += summand[dimension];
}
vector<double> WVectorMaths::copyVector( const vector<double>& copiedVector )
{
    vector<double> newVector( copiedVector.size(), 0 );
    for( size_t dimension = 0; dimension < copiedVector.size(); dimension++ )
        newVector[dimension] = copiedVector[dimension];
    return newVector;
}
vector<double>* WVectorMaths::copyVectorForPointer( const vector<double>& copiedVector )
{
    vector<double>* newVector = new vector<double>( copiedVector.size(), 0 );
    for( size_t dimension = 0; dimension < copiedVector.size(); dimension++ )
        newVector->at( dimension ) = copiedVector[dimension];
    return newVector;
}
double WVectorMaths::getAngleOfVectors( const vector<double>& vector1, const vector<double>& vector2 )
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
double WVectorMaths::getAngleOfPlanes( const vector<double>& vector1, const vector<double>& vector2 )
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
double WVectorMaths::getAngleToAxis( double x, double y )
{
    if( x == 0.0 && y != 0.0 )
        return 90.0;
    if( x == 0.0 )
        return 90.0;
    double angle = atan( y / x ) / ANGLE_90_DEGREES * 90.0;
    return angle;
}
double WVectorMaths::getAngleToAxisComplete( double x, double y )
{
    if( x == 0.0 && y == 0.0 )
        return atan( y / x ) / ANGLE_90_DEGREES * 90.0;
    double angle = getAngleToAxis( x, y );
    double angleRad = angle / 90.0 * ANGLE_90_DEGREES;
    vector<double>* orig = new2dVectorPointer( x, y );
    normalizeVector( orig );
    vector<double> rotated = new2dVector( cos( angleRad ), sin( angleRad ) );
    if( getEuclidianDistance( *orig, rotated ) > 1.0 )
        angle -= 180.0;
    while( angle < 0.0 )
        angle += 360.0;
    delete orig;
    return angle;
}
double WVectorMaths::getEuclidianDistance( const vector<double>& distanceVector )
{
    double sum = 0.0;
    for( size_t dimension = 0; dimension < distanceVector.size(); dimension++ )
        sum += pow( distanceVector[dimension], 2.0 );
    sum = pow( sum, 0.5 );
    return sum;
}
double WVectorMaths::getEuclidianDistance( const vector<double>& point1, const vector<double>& point2 )
{
    vector<double>* minuend = copyVectorForPointer( point1 );
    invertVector( minuend );
    addVector( minuend, point2 );
    double euclidianDistance = getEuclidianDistance( *minuend );
    delete minuend;
    return euclidianDistance;
}
vector<double> WVectorMaths::getIntersectionPoint( const vector<double>& line1P1,
        const vector<double>& line1P2, const vector<double>& line2P1, const vector<double>& line2P2 )
{
    double deltaX = line1P2[0] - line1P1[0];
    double deltaY = line1P2[1] - line1P1[1];
    vector<double> result = deltaX > deltaY
            ?getIntersectionPointAtY( line1P1[0], line1P1[1], line1P2[0], line1P2[1],
                    line2P1[0], line2P1[1], line2P2[0], line2P2[1] )
            :getIntersectionPointAtY( line1P1[1], line1P1[0], line1P2[1], line1P2[0],
                    line2P1[1], line2P1[0], line2P2[1], line2P2[0] );
    if( deltaX <= deltaY )
    {
        double tmp = result[0];
        result[0] = result[1];
        result[1] = tmp;
    }
    return result;
}
double WVectorMaths::getLawOfCosinesAlphaByPoints( const vector<double>& pointA,
        const vector<double>& pointB, const vector<double>& pointC )
{
    double lengthA = getEuclidianDistance( pointB, pointC );
    double lengthB = getEuclidianDistance( pointA, pointC );
    double lengthC = getEuclidianDistance( pointA, pointB );
    return getLawOfCosinesAlphaByLineLengths( lengthA, lengthB, lengthC );
}
double WVectorMaths::getLawOfCosinesAlphaByLineLengths( double lengthA, double lengthB, double lengthC )
{
    double quotient = ( lengthA*lengthA - lengthB*lengthB - lengthC*lengthC ) / ( - 2 * lengthB * lengthC );
    return acos( quotient ) / ANGLE_90_DEGREES * 90.0;
}
void WVectorMaths::invertVector( vector<double>* invertedVector )
{
    for( size_t dimension = 0; dimension < invertedVector->size(); dimension++ )
        invertedVector->at( dimension ) = - invertedVector->at( dimension );
}
bool WVectorMaths::isPointInRange( const vector<double>& point, const vector<double>& rangeFrom, const vector<double>& rangeTo )
{
    for( size_t index = 0; index < point.size(); index++ )
        if( ( point[index] < rangeFrom[index] && point[index] < rangeTo[index] ) ||
                ( point[index] > rangeFrom[index] && point[index] > rangeTo[index] ) )
            return false;
    return true;
}
bool WVectorMaths::isPointOnLine2d( const vector<double>& point,
        const vector<double>& lineP1, const vector<double>& lineP2  )
{
    double deltaX = lineP2[0] - lineP1[0];
    double deltaY = lineP2[1] - lineP1[1];
    return deltaX > deltaY
            ?isPointOnLineAtY( point[0], point[1],
                    lineP1[0], lineP1[1], lineP2[0], lineP2[1] )
            :isPointOnLineAtY( point[1], point[0],
                    lineP1[1], lineP1[0], lineP2[1], lineP2[0] );
}
bool WVectorMaths::isValidVector( const vector<double>& vector )
{
    for( size_t index = 0; index < vector.size(); index++ )
        if( vector[index] != vector[index] || vector[index] == INFINITY_POSITIVE || vector[index] == INFINITY_NEGATIVE )
            return false;
    return true;
}
bool WVectorMaths::linesCanIntersectBounded( const vector<double>& line1P1,
    const vector<double>& line1P2, const vector<double>& line2P1, const vector<double>& line2P2 )
{
    if( !WVectorMaths::linesCanIntersect( line1P1, line1P2, line2P1, line2P2 ) )
        return false;
    vector<double> intersection = getIntersectionPoint( line1P1, line1P2, line2P1, line2P2 );
    return WVectorMaths::isPointInRange( intersection, line1P1, line1P2 ) &&
            WVectorMaths::isPointInRange( intersection, line2P1, line2P2 );
}
bool WVectorMaths::linesCanIntersect( const vector<double>& line1P1,
    const vector<double>& line1P2, const vector<double>& line2P1, const vector<double>& line2P2 )
{
    double deltaX = line1P2[0] - line1P1[0];
    double deltaY = line1P2[1] - line1P1[1];
    return deltaX > deltaY
            ?linesCanIntersectAtY( line1P1[0], line1P1[1], line1P2[0], line1P2[1],
                    line2P1[0], line2P1[1], line2P2[0], line2P2[1] )
            :linesCanIntersectAtY( line1P1[1], line1P1[0], line1P2[1], line1P2[0],
                    line2P1[1], line2P1[0], line2P2[1], line2P2[0] );
}
void WVectorMaths::multiplyVector( vector<double>* changedVector, const vector<double>& factor )
{
    size_t size = changedVector->size();
    for( size_t dimension = 0; dimension < size; dimension++ )
        changedVector->at( dimension ) *= factor[dimension];
}
vector<double> WVectorMaths::new2dVector( double x, double y )
{
    vector<double> newVector( 2, 0 );
    newVector[0] = x;
    newVector[1] = y;
    return newVector;
}
vector<double>* WVectorMaths::new2dVectorPointer( double x, double y )
{
    vector<double>* newVector = new vector<double>( 2, 0 );
    newVector->at( 0 ) = x;
    newVector->at( 1 ) = y;
    return newVector;
}
vector<double> WVectorMaths::new3dVector( double x, double y, double z )
{
    vector<double> newVector( 3, 0 );
    newVector[0] = x;
    newVector[1] = y;
    newVector[2] = z;
    return newVector;
}
void WVectorMaths::normalizeVector( vector<double>* normalizableVector )
{
    double sum = getEuclidianDistance( *normalizableVector );
    if( sum == 0.0 )
        return;
    for( size_t dimension = 0; dimension < normalizableVector->size(); dimension++ )
        normalizableVector->at( dimension ) /= sum;
}
void WVectorMaths::rotateVector( vector<double>* rotatedVector, size_t firstAxis, size_t secondAxis, double angleDegrees )
{
    double firstValue = rotatedVector->at( firstAxis );
    double secondValue = rotatedVector->at( secondAxis );
    double angle = angleDegrees / 90.0 * ANGLE_90_DEGREES;
    rotatedVector->at( firstAxis ) = firstValue * cos( angle ) - secondValue * sin( angle );
    rotatedVector->at( secondAxis ) = firstValue * sin( angle ) + secondValue * cos( angle );
}






vector<double> WVectorMaths::getIntersectionPointAtY(
        const double& line1P1x, const double& line1P1y, const double& line1P2x, const double& line1P2y,
        const double& line2P1x, const double& line2P1y, const double& line2P2x, const double& line2P2y )
{
    double a1 = ( line1P2y - line1P1y ) / ( line1P2x - line1P1x );
    double t1 = line1P1y - a1 * line1P1x;

    double deltaX2 = line2P2x - line2P1x;
    double finalX = 0;
    if( deltaX2 == 0.0 )
    {
        finalX = line2P1x;
    }
    else
    {
        double a2 = ( line2P2y - line2P1y ) / deltaX2;
        double t2 = line2P1y - a2 * line2P1x;
        finalX = ( t1 - t2 ) / ( a2 - a1 );
    }
    vector<double> result = new2dVector( finalX, a1 * finalX + t1 );
    return result;
}
bool WVectorMaths::isPointOnLineAtY( const double& pointX, const double& pointY,
        const double& lineP1x, const double& lineP1y, const double& lineP2x, const double& lineP2y )
{
    double a = ( lineP2y - lineP1y ) / ( lineP2x - lineP1x );
    double t = lineP1y - a * lineP1x;
    return pointY == a * pointX + t;
}
bool WVectorMaths::linesCanIntersectAtY(
        const double& line1P1x, const double& line1P1y, const double& line1P2x, const double& line1P2y,
        const double& line2P1x, const double& line2P1y, const double& line2P2x, const double& line2P2y )
{
    double a1 = ( line1P2y - line1P1y ) / ( line1P2x - line1P1x );
    double deltaX2 = line2P2x - line2P1x;
    return deltaX2 == 0 ?true
            :a1 != ( line2P2y - line2P1y ) / deltaX2;
}
