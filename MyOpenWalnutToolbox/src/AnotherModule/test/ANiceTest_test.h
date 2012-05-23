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


