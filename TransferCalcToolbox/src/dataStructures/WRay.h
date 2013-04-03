//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#ifndef WRAY_H
#define WRAY_H

#include "core/common/math/WMatrix.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"

/**
 * class to implement a simple ray
 */
class WRay
{
public:
    /**
     * Standard Constructor
     */
    WRay();

    /**
     * constructor - 
     * normalizes direction
     *
     * \param st start vector
     * \param dir direction of the ray
     */
    WRay( WVector4d st, WVector4d dir );

    /**
     * destructor
     */
    virtual ~WRay();

    /**
     * Access the start of the ray.
     *
     * \return the start reference.
     */
    const WVector4d& start() const;

    /**
     * Access the start of the ray.
     *
     * \return the start reference.
     */
    WVector4d& start();

    /**
     * Access the direction of the ray.
     *
     * \return the direction reference.
     */
    const WVector4d& direction() const;

    /**
     * Access the direction of the ray.
     *
     * \return the direction reference.
     */
    WVector4d& direction();

    /**
     * Calculates a spot on the ray with given parameter t
     * ray = start + t * direction
     * 
     * \param t the distance to the start of the ray
     *
     * \return vector
     */
    virtual WVector4d getSpot( double t );

protected:
private:
    /**
     * origin of the ray
     */
    WVector4d m_start;

    /**
     * direction of the ray
     */
    WVector4d m_direction;
};

#endif  // WRAY_H
