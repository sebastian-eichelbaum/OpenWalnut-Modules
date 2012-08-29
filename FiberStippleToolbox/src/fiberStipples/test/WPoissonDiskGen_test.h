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

#ifndef WPOISSONDISKGEN_TEST_H
#define WPOISSONDISKGEN_TEST_H

#include <cxxtest/TestSuite.h>

#include <boost/foreach.hpp>

#include "../WPoissonDiskGen.h"

/**
 * Testsuite for a Poisson Disk generator.
 */
class WPoissonDiskGaussProcessTest : public CxxTest::TestSuite
{
public:
    /**
     * For each point in Poisson-Disk distribution, there is no other point closer than the given radius.
     */
    void testEveryPointFullfillsMinimalSpaceRequirement( void )
    {
        double radius = 0.01;
        WPoissonDiskGen gen( radius ); // calculates points with r=0.1 in [-1,1]^2 where cell width = height = 0.1

        BOOST_FOREACH( WVector2d point, gen )
        {
            size_t numSamePoints = 0;
            // check that for every other point (different than currentPoint) the radius >= 0.1
            BOOST_FOREACH( WVector2d other, gen )
            {
                if( point != other )
                {
                    if( length2( point - other ) < radius * radius )
                    {
                        std::stringstream ss;
                        ss << "point: " << point << " other: " << other << " radius^2: " << radius * radius << " dist: " << length2( point - other );
                        int id_point = ( point[0] < 0 || point[1] < 0 ) ? -1 : static_cast< int >( point[0] / radius ) + static_cast< int >( point[1] / radius / radius );
                        int id_other = ( other[0] < 0 || other[1] < 0 ) ? -1 : static_cast< int >( other[0] / radius ) + static_cast< int >( point[1] / radius / radius );
                        ss << "ids: " << id_point << " " << id_other;

                        TS_FAIL( ss.str() );
                    }
                }
                else
                {
                    ++numSamePoints;
                }
            }
            TS_ASSERT_EQUALS( numSamePoints, 1 );
        }
        // std::cout << std::endl;
        // std::cout << "size: " << gen.size() << std::endl;
        // std::cout << "time: " << gen.time() << std::endl;
    }
};

#endif  // WPOISSONDISKGEN_TEST_H
