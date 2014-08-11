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
     * Calculates angle between two vectors
     * \param vector1 First vector.
     * \param vector2 Second vector.
     * \return Angle of the two vectors using the decree scale.
     */
    static double getAngleOfVectors( vector<double> vector1, vector<double> vector2 );
    /**
     * Calculates the angle of two planes. It has the range of 180°. Vectors showing 
     * exactly the same direction have 0°. Lying in the same line but
     * pointing the opposite direction have the difference of 180°.
     * \param vector1 First plane normal vector to calculate an angle between.
     * \param vector2 Second plane normal vector to calculate an angle between.
     * \return The angle between the two planes.
     */
    static double getAngleOfPlanes( vector<double> vector1, vector<double> vector2 );
    /**
     * Returns the angle between two planes.
     * \param vector1 Normal vector of the first plane.
     * \param vector2 Normal vector of the second plane.
     * \return Angle between two planes.
     */
    static double getAngleOfPlanes( WVector3d vector1, WVector3d vector2 );

private:
};

#endif  // WVECTORMATHS_H
