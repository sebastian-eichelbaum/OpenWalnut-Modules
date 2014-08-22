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

#ifndef WVECTORMATHS_H
#define WVECTORMATHS_H

#include <vector>
#include "core/common/math/linearAlgebra/WVectorFixed.h"


using std::vector;
using std::allocator;

/**
 * A set of general vector calculation operations.
 */
class WVectorMaths
{
public:
    /**
     * Instantiates an instance for general vector calculation.
     */
    WVectorMaths();
    /**
     * Destroys an instance for general vector calculation.
     */
    virtual ~WVectorMaths();
    /**
     * Radial amount of 90 degrees.
     */
    static const double ANGLE_90_DEGREES;
    /**
     * Negative infinite value.
     */
    static const double INFINITY_NEGATIVE;
    /**
     * Positive infinite value.
     */
    static const double INFINITY_POSITIVE;

    /**
     * Adds a vector with another.
     * \param changedVector First summand. The result is written directly in that value.
     * \param summand Second summand.
     */
    static void addVector( vector<double>* changedVector, const vector<double>& summand );
    /**
     * Copies a vector
     * \param copiedVector Vector that should be copied.
     * \return The copied vector.
     */
    static vector<double> copyVector( const vector<double>& copiedVector );
    /**
     * Copies a vector returning a pointer of the new vector.
     * \param copiedVector Vector that should be copied.
     * \return The copied vector.
     */
    static vector<double>* copyVectorForPointer( const vector<double>& copiedVector );
    /**
     * Calculates angle between two vectors
     * \param vector1 First vector.
     * \param vector2 Second vector.
     * \return Angle of the two vectors using the decree scale.
     */
    static double getAngleOfVectors( const vector<double>& vector1, const vector<double>& vector2 );
    /**
     * Calculates the angle of two planes. It has the range of 180°. Vectors showing 
     * exactly the same direction have 0°. Lying in the same line but
     * pointing the opposite direction have the difference of 180°.
     * \param vector1 First plane normal vector to calculate an angle between.
     * \param vector2 Second plane normal vector to calculate an angle between.
     * \return The angle between the two planes.
     */
    static double getAngleOfPlanes( const vector<double>& vector1, const vector<double>& vector2 );
    /**
     * Returns the angle between two planes.
     * \param vector1 Normal vector of the first plane.
     * \param vector2 Normal vector of the second plane.
     * \return Angle between two planes.
     */
    static double getAngleOfPlanes( WVector3d vector1, WVector3d vector2 );
    /**
     * Returns a points angle from the coordinate system in relation to the coordinate 
     * system. The range is 180 degrees because it takes the same angle if the the angle 
     * is the same but the point goes other direction behind the coordinate system 
     * origin.
     * \param x X coordinate to calculate the angle of:
     * \param y Y coordinate to calculate the angle of:
     * \return The angle of the two dimensional coordinate in relation to the coordinate 
     *         system origin.
     */
    static double getAngleToAxis( double x, double y );
    /**
     * Calculates the angle of a point in relation to the coordinate system origin. It 
     * has the whole range of 360 degrees. It shows a different angle if a point would 
     * be behind the coordinate system origin.
     * \param x X coordinate to calculate the angle of:
     * \param y Y coordinate to calculate the angle of:
     * \return The angle of the two dimensional coordinate in relation to the coordinate 
     *         system origin.
     */
    static double getAngleToAxisComplete( double x, double y );
    /**
     * Returns the euclidian distance (shortest distance throgh the space) across a 
     * vector.
     * \param distanceVector Vector to calculate the euclidean distance of.
     * \return The euclidian length of a vector.
     */
    static double getEuclidianDistance( const vector<double>& distanceVector );
    /**
     * Returns the euclidian distance (shortest distance throgh the space) between two 
     * points.
     * \param point1 First vector to calculate the distance between.
     * \param point2 Second vector to calculate the distance between.
     * \return The shortest connection length between two points.
     */
    static double getEuclidianDistance( const vector<double>& point1, const vector<double>& point2 );
    /**
     * Returns the intersection point between two lines, each is described by two 
     * points. The method works only for two dimensional coordinate systems.
     * \param line1P1 First point of the first line.
     * \param line1P2 Second point of the first line.
     * \param line2P1 First point of the second line.
     * \param line2P2 Second point of the second line.
     * \return Intersection point between the two lines.
     */
    static vector<double> getIntersectionPoint( const vector<double>& line1P1,
            const vector<double>& line1P2, const vector<double>& line2P1, const vector<double>& line2P2 );
    /**
     * Law Of Cosines that returns an edge angle of a triangle knowing all point 
     * coordinates.
     * \param pointA Triangle point A.
     * \param pointB Triangle point B.
     * \param pointC Triangle point C.
     * \return Angle at the point A.
     */
    static double getLawOfCosinesAlphaByPoints( const vector<double>& pointA,
            const vector<double>& pointB, const vector<double>& pointC );
    /**
     * Law Of Cosines that returns an edge angle of a triangle knowing all line lengths.
     * \param lengthA Triangle line length A.
     * \param lengthB Triangle line length B.
     * \param lengthC Triangle line length C.
     * \return Angle of the edge at the opposite of the triangle line A.
     */
    static double getLawOfCosinesAlphaByLineLengths( double lengthA, double lengthB, double lengthC );
    /**
     * Tells whether a point lies within the rectangular quader described by two other 
     * points.
     * \param point Point that can be within the range.
     * \param rangeFrom First point describing the range quader.
     * \param rangeTo Second point describing the range quader.
     * \return Point is within that range or not.
     */
    static bool isPointInRange( const vector<double>& point,
            const vector<double>& rangeFrom, const vector<double>& rangeTo );
    /**
     * Tells whether a point is hit by a line exactly or not. The line is described by 
     * two two dimensional points. The line has no bounds.
     * \param point Point that can lie on the line.
     * \param lineP1 First point describing the not bounded line.
     * \param lineP2 Second point describing the not bounded line.
     * \return point lies on the line or not.
     */
    static bool isPointOnLine2d( const vector<double>& point,
            const vector<double>& lineP1, const vector<double>& lineP2 );
    /**
     * Inverts coordinates of a point of vector.
     * \param invertedVector Vector to be inverted.
     */
    static void invertVector( vector<double>* invertedVector );
    /**
     * Tells whether a vector is valid. No value of X/Y/Z etc. may be infinite or nan.
     * \param vector Vector to be checked whether valid or not.
     * \return The vector is valid or not.
     */
    static bool isValidVector( const vector<double>& vector );
    /**
     * Returns whether two lines without bounds can intersect within a two dimensional 
     * coordinate system or not.
     * \param line1P1 First point of the first line.
     * \param line1P2 Second point of the first line.
     * \param line2P1 First point of the second line.
     * \param line2P2 Second point of the second line.
     * \return Lines intersect or not.
     */
    static bool linesCanIntersect( const vector<double>& line1P1,
            const vector<double>& line1P2, const vector<double>& line2P1, const vector<double>& line2P2 );
    /**
     * Tells whether lines, each described by two points (also limited by them in 
     * length) can intersect or not.
     * \param line1P1 First point of the first line.
     * \param line1P2 Second point of the first line.
     * \param line2P1 First point of the second line.
     * \param line2P2 Second point of the second line.
     * \return Lines intersect or not.
     */
    static bool linesCanIntersectBounded( const vector<double>& line1P1,
            const vector<double>& line1P2, const vector<double>& line2P1, const vector<double>& line2P2 );
    /**
     * Multiplies a vector by another one.
     * \param changedVector First factor. The result is exported directly in that value.
     * \param factor Second factor.
     */
    static void multiplyVector( vector<double>* changedVector, const vector<double>& factor );
    /**
     * Creates a two dimensional vector using arbitrary values.
     * \param x X axis value;
     * \param y Y axis value;
     * \return The new vector.
     */
    static vector<double> new2dVector( double x, double y );
    /**
     * Returns a pointer of a new two dimensional vector using arbitrary values.
     * \param x X axis value;
     * \param y Y axis value;
     * \return Pointer of the new vector.
     */
    static vector<double>* new2dVectorPointer( double x, double y );
    /**
     * Creates a three dimensional vector using arbitrary values.
     * \param x X axis value;
     * \param y Y axis value;
     * \param z Z axis value;
     * \return The new vector.
     */
    static vector<double> new3dVector( double x, double y, double z );
    /**
     * Normalizes a vector. The euclidian distance from the coordinate system origin 
     * to this vector becomes 1.0 keeping the same direction.
     * \param normalizableVector Vector to be normalized.
     */
    static void normalizeVector( vector<double>* normalizableVector );
    /**
     * Rotates two dimensions of a vector. To rotate a vector with many directions the 
     * method has to be applied using many combinations pair wise.
     * \param rotatedVector Rotated vector.
     * \param firstAxis First rotated dimension.
     * \param secondAxis Second rotated dimension.
     * \param angleDegrees Rotation angle.
     */
    static void rotateVector( vector<double>* rotatedVector, size_t firstAxis, size_t secondAxis, double angleDegrees );


private:
    /**
     * Calculates the intersection point between two lines without bounds. The first 
     * line must not be perpendicular to the x axis.
     * \param line1P1x First point of the first line, X coordinate.
     * \param line1P1y First point of the first line, Y coordinate.
     * \param line1P2x Second point of the first line, X coordinate.
     * \param line1P2y Second point of the first line, Y coordinate.
     * \param line2P1x First point of the second line, X coordinate.
     * \param line2P1y First point of the second line, Y coordinate.
     * \param line2P2x Second point of the second line, X coordinate.
     * \param line2P2y Second point of the second line, Y coordinate.
     * \return The intersection point between the two lines.
     */
    static vector<double> getIntersectionPointAtY(
            const double& line1P1x, const double& line1P1y, const double& line1P2x, const double& line1P2y,
            const double& line2P1x, const double& line2P1y, const double& line2P2x, const double& line2P2y );
    /**
     * Tells whether a point hits a line without bounds exactly or not. The line must 
     * not be perpendicular to the x axis.
     * \param pointX X axis of the point:
     * \param pointY Y axis of the point:
     * \param lineP1x First point of the line, X coordinate;
     * \param lineP1y First point of the line, Y coordinate;
     * \param lineP2x Second point of the line, X coordinate;
     * \param lineP2y Second point of the line, Y coordinate;
     * \return Point hits the line without bounds exactly or not.
     */
    static bool isPointOnLineAtY( const double& pointX, const double& pointY,
            const double& lineP1x, const double& lineP1y, const double& lineP2x, const double& lineP2y );
    /**
     * Tells whether two lines without bounds can intersect within one single point or 
     * not. The first line must not be perpendicular to the x axis.
     * \param line1P1x First point of the first line X coordinate.
     * \param line1P1y First point of the first line Y coordinate.
     * \param line1P2x Second point of the first line X coordinate.
     * \param line1P2y Second point of the first line Y coordinate.
     * \param line2P1x First point of the second line X coordinate.
     * \param line2P1y First point of the second line Y coordinate.
     * \param line2P2x Second point of the second line X coordinate.
     * \param line2P2y Second point of the second line Y coordinate.
     * \return Lines can intersect within one single point or not.
     */
    static bool linesCanIntersectAtY(
            const double& line1P1x, const double& line1P1y, const double& line1P2x, const double& line1P2y,
            const double& line2P1x, const double& line2P1y, const double& line2P2x, const double& line2P2y );
};

#endif  // WVECTORMATHS_H
