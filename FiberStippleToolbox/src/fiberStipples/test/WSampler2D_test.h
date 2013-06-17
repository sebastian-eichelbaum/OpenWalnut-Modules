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

#include "../WSampler2D.h"

/**
 * Testsuite for 2D sampler.
 */
class WPoissonDiskGaussProcessTest : public CxxTest::TestSuite
{
public:
    /**
     * Generate ten points inside the unit quad [0,1]^2.
     */
    void testRectangularUniformSampling( void )
    {
        std::srand( 0 );
        WSampler2DUniform sampler( 10, 1.0, 1.0, DONT_CALL_SRAND );
        std::srand( 0 );
        WSampler2DUniform expected( 10, 1.0, 1.0, DONT_CALL_SRAND );

        TS_ASSERT_EQUALS( expected.size(), 10 );
        TS_ASSERT_EQUALS( sampler.size(), expected.size() );

        size_t i = 0;
        BOOST_FOREACH( WVector2d point, sampler )
        {
            TS_ASSERT( point[0] >= 0.0 && point[0] <= 1.0 );
            TS_ASSERT( point[1] >= 0.0 && point[1] <= 1.0 );
            TS_ASSERT_EQUALS( point, expected.at( i ) );
            ++i;
        }
    }
};

#endif  // WPOISSONDISKGEN_TEST_H
