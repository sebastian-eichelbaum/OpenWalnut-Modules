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

#ifndef WKDPOINTND_H
#define WKDPOINTND_H

#include <vector>
#include <algorithm>

using std::vector;
using std::size_t;
using std::cout;
using std::endl;

/**
 * A kd tree point that is added to n dimensional kd trees.
 */
class WKdPointND
{
public:
    /**
     * Instance to create an n dimensional kd tree point instance.
     * \param coordinate N dimensional coordinate of the new kd tree point.
     */
    explicit WKdPointND( vector<double> coordinate );

    /**
     * Instance to create a two dimensional kd tree point instance.
     * \param x Three dimensional X coordinate of the new kd tree point.
     * \param y Three dimensional Y coordinate of the new kd tree point.
     */
    WKdPointND( double x, double y );

    /**
     * Instance to create a three dimensional kd tree point instance.
     * \param x X coordinate of the new kd tree point.
     * \param y Y coordinate of the new kd tree point.
     * \param z Z coordinate of the new kd tree point.
     */
    WKdPointND( double x, double y, double z );

    /**
     * Destroys a kd tree point instance.
     */
    virtual ~WKdPointND();

    /**
     * Returns the dimension count of a kd tree point.
     * \return Dimension count of a kd tree point.
     */
    size_t getDimensionCount();

    /**
     * Returns the coordinate of a kd tree point.
     * \return Coordinate of a kd tree point.
     */
    vector<double> getCoordinate();

    /**
     * Sets coordinate of the point.
     * \param coordinate Point's coordinate.
     */
    void setCoordinate( vector<double> coordinate );

private:
    /**
     * N dimensional coordinate of a kd tree point.
     */
    vector<double> m_coordinate;
};

#endif  // WKDPOINTND_H
