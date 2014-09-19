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

#ifndef WPOINTDISTANCE_H
#define WPOINTDISTANCE_H

#include <math.h>

#include <iostream>
#include <vector>
#include "WKdPointND.h"
#include "core/common/math/linearAlgebra/WPosition.h"

using std::vector;

/**
 * Object that pairs a coordinate with a distance information. It has also an operator 
 * that allows to use std::sort to sort coordinates by the ascending distance. The 
 * coordinates have the vector type because the class is used paired with the kd tree 
 * which is unidimensional.
 */
class WPointDistance
{
public:
    /**
     * Sets up the object without filling parameters.
     */
    WPointDistance();

    /**
     * Instantiates the coordinate distance pair filling all necessary parameters.
     * \param sourcePoint Reference point used to calculate the distance. Its 
     *                    coordinate isn't stored.
     * \param comparedPoint The second point that is used to calculate the distance 
     *                      between. The object stores its coordinates by that.
     */
    WPointDistance( vector<double> sourcePoint, WKdPointND* comparedPoint );

    /**
     * Object destructor
     */
    virtual ~WPointDistance();

    /**
     * Returns the coordinate of the point compared to the reference.
     * \return The compared point coordinate.
     */
    vector<double> getComparedCoordinate();

    /**
     * Returns the point that is considered within the current distance calculation 
     * instance.
     * \return Point that is considered within the current distance calculation instance.
     */
    WKdPointND* getComparedPoint();

    /**
     * Returns the distance between the reference point and the compared one.
     * \return The distance between the two instantiated points using the constructor.
     */
    double getDistance();

    /**
     * Static method that fetches coordinates of WPointDistance sets into a three 
     * dimensional WPosition point list.
     * \param pointDistances Point distance data sets to fetch positions from.
     * \return A WPosition list. That type is very commonly used in OpenWalnut.
     */
    static vector<WPosition>* convertToPointSet( vector<WPointDistance>* pointDistances );

    /**
     * Operator for sorting a vector<WPointDistance> using std::sort.
     * \param right The right compared object to this one.
     * \return The distance of this object is smaller than the right one or not.
     */
    bool operator<( WPointDistance const& right ) const;

private:
    /**
     * The euclidian distance between the two points instantiated using the constructor.
     */
    double m_pointDistance;

    /**
     * The unidimensional coordinate of the compared point instantiated using the 
     * constructor.
     */
    WKdPointND* m_comparedPoint;
};

#endif  // WPOINTDISTANCE_H
