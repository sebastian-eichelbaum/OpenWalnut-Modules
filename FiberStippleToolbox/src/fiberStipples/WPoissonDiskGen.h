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

#ifndef WPOISSONDISKGEN_H
#define WPOISSONDISKGEN_H

#include "core/common/math/linearAlgebra/WVectorFixed.h" // for WVector2d
#include "core/common/WMixinVector.h"

/**
 * Generates random points on a disk / plane / slice (whatever you may call
 * it) so that all points having at least a certain distance.
 */
class WPoissonDiskGen : public WMixinVector< WVector2d > { public: explicit
    WPoissonDiskGen( double radius, size_t maxNeighbourTests = 30, double width
            = 1.0, double height = 1.0 );

    virtual ~WPoissonDiskGen();

    std::string time() const;

protected:
    void generatePoints();

    bool trySeedingNewPoints( const WVector2d& center, std::set< WVector2d >& actives );

    bool valid( const WVector2d& point ) const;

    WVector2d nextRandomPoint( const WVector2d& center ) const;

    double m_radius;
    size_t m_maxNeighbourTests;
    double m_width;
    double m_height;
    size_t m_time;

    size_t m_numCellsX;
    size_t m_numCellsY;
private:
    int id( const WVector2d& point ) const;
    int id( int x, int y ) const;
    std::vector< WVector2d > m_grid;
};

#endif // WPOISSONDISKGEN_H

