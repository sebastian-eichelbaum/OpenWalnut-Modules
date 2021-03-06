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

#ifndef ANICETEST_TEST_H
#define ANICETEST_TEST_H

#include <boost/filesystem.hpp>

#include <cxxtest/TestSuite.h>

#include <core/common/WLogger.h>

/**
 * This is an example test. It has no further meaning.
 */
class ANiceTest : public CxxTest::TestSuite
{
public:
    /**
     * Setup logger and other stuff for each test.
     */
    void setUp()
    {
        // needed if some of your classes use the WLogger
        WLogger::startup();
    }

    /**
     * Test whether modules can be initialized without problems.
     */
    void testModuleInitialization( void )
    {
        // you can use test data you have put into the fixtures directory by using the path:
        // W_FIXTURE_PATH + "/testfile"
        TS_ASSERT( boost::filesystem::exists( W_FIXTURE_PATH + "/testfile" ) );
    }
};

#endif  // ANICETEST_TEST_H


