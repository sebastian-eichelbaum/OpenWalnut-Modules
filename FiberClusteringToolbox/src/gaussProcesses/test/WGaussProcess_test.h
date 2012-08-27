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

#ifndef WGAUSSPROCESS_TEST_H
#define WGAUSSPROCESS_TEST_H

#include <vector>

#include <cxxtest/TestSuite.h>

#include "core/common/datastructures/WFiber.h"
#include "core/common/WLogger.h"
#include "core/dataHandler/WDataSetDTI.h"
#include "core/dataHandler/WDataSetFiberVector.h"
#include "../WGaussProcess.h"

/**
 * Testsuite for the Gaussian process class.
 */
class WGaussProcessTest : public CxxTest::TestSuite
{
public:
    /**
     * If the point for the mean function is outside of the environment with distance R the mean
     * should equals to zero.
     */
    void testMeanFunctionOutsideOf_R_Environment( void )
    {
        WGaussProcess p( m_tractID, m_tracts, m_emptyDTIDataSet );
        TS_ASSERT_DELTA( p.mean( WPosition( -( p.m_R + wlimits::DBL_EPS ), 0.0, 0.0 ) ), 0.0, wlimits::DBL_EPS );
    }

    /**
     * Inside of the R environment there shall be values unequal to zero. (-p.m_R + EPS would be not change the mean value
     * significantly, we have to go a little more inside the kernel!)
     */
    void testMeanFunctionInsideOf_R_Environment( void )
    {
        WGaussProcess p( m_tractID, m_tracts, m_emptyDTIDataSet );
        TS_ASSERT( std::abs( p.mean( WPosition( -p.m_R + 1.0e-7, 0.0, 0.0 ) ) ) > wlimits::DBL_EPS );
    }

    /**
     * Inside of the R environment the values should be monoton increasing in direction to the tract
     * segements.
     */
    void testMeanFunctionMonotonyIn_R_Environment( void )
    {
        WGaussProcess p( m_tractID, m_tracts, m_emptyDTIDataSet );
        TS_ASSERT( std::abs( p.mean( WPosition( -p.m_R + 1.0e-8, 0.0, 0.0 ) ) ) >
                             p.mean( WPosition( -p.m_R + 0.5e-8, 0.0, 0.0 ) ) );
    }

    /**
     * The mean value on the sample point is the maximum level set.
     */
    void testMeanFunctionOnSamplePoint( void )
    {
        WGaussProcess p( m_tractID, m_tracts, m_emptyDTIDataSet );
        TS_ASSERT_DELTA( p.mean( WPosition( 0.0, 0.0, 0.0 ) ), p.m_maxLevel, 2 * wlimits::DBL_EPS );
    }

//    void testMeanFunctionOnSegmentButNotOnSamplePoint( void )
//    {
//        WGaussProcess p( m_tract, m_emptyDTIDataSet );
//        TS_ASSERT_DELTA( p.mean( WPosition( 0.4, 0.4, 0.0 ) ), p.m_maxLevel, wlimits::DBL_EPS );
//    }

    /**
     * Non overlapping processes should return 0.0 as inner product.
     */
    void testInnerProductOnNonOverlappingIndicatorFunctions( void )
    {
        WGaussProcess p1( 0, m_tracts, m_emptyDTIDataSet );
        WGaussProcess p2( 1, m_tracts, m_emptyDTIDataSet );
        TS_ASSERT_DELTA( gauss::innerProduct( p1, p2 ), 0.0, wlimits::DBL_EPS );
    }

    /**
     * Its hard to find an example where the integral over the spheres is exactly determined, incase
     * of an overlap and no full overlap. Hence we take a tract with many points where almost 50
     * percent will overlap. When increasing the number of points the innerproduct should converge.
     */
    void testPartialOverlapWith10Points( void )
    {
        boost::shared_ptr< std::vector< WFiber > > tracts( new std::vector< WFiber > );
        WFiber tract;
        for( size_t i = 0; i < 10; ++i )
        {
            tract.push_back( WPosition( static_cast< double >( i ), 0.0, 0.0 ) );
        }
        tracts->push_back( tract );
        tract.clear();
        for( size_t i = 0; i < 5; ++i )
        {
            tract.push_back( WPosition( static_cast< double >( i ), 0.0, 0.0 ) );
        }
        for( size_t i = 1; i < 6; ++i )
        {
            tract.push_back( WPosition( 4.0, static_cast< double >( i ), 0.0 ) );
        }
        tracts->push_back( tract );
        boost::shared_ptr< WDataSetFiberVector > fvDS( new WDataSetFiberVector( tracts ) );

        WGaussProcess p1( 0, fvDS->toWDataSetFibers(), m_emptyDTIDataSet );
        WGaussProcess p2( 1, fvDS->toWDataSetFibers(), m_emptyDTIDataSet );
        double overlap = gauss::innerProduct( p1, p2 ) / ( std::sqrt( gauss::innerProduct( p1, p1 ) ) * std::sqrt( gauss::innerProduct( p2, p2 ) ) );
        TS_ASSERT_DELTA( overlap, 0.5687, 0.003 );
    }

    /**
     * This is to test the converge nearly to 50 percent and should always be better than with just
     * 100 points.
     */
    void testPartialOverlapWith100Points( void )
    {
        boost::shared_ptr< std::vector< WFiber > > tracts( new std::vector< WFiber > );
        WFiber tract;
        for( size_t i = 0; i < 100; ++i )
        {
            tract.push_back( WPosition( static_cast< double >( i ), 0.0, 0.0 ) );
        }
        tracts->push_back( tract );
        tract.clear();
        for( size_t i = 0; i < 50; ++i )
        {
            tract.push_back( WPosition( static_cast< double >( i ), 0.0, 0.0 ) );
        }
        for( size_t i = 1; i < 51; ++i )
        {
            tract.push_back( WPosition( 49.0, static_cast< double >( i ), 0.0 ) );
        }
        tracts->push_back( tract );
        boost::shared_ptr< WDataSetFiberVector > fvDS( new WDataSetFiberVector( tracts ) );

        WGaussProcess p1( 0, fvDS->toWDataSetFibers(), m_emptyDTIDataSet );
        WGaussProcess p2( 1, fvDS->toWDataSetFibers(), m_emptyDTIDataSet );
        double overlap = gauss::innerProduct( p1, p2 ) / ( std::sqrt( gauss::innerProduct( p1, p1 ) ) * std::sqrt( gauss::innerProduct( p2, p2 ) ) );
        TS_ASSERT_DELTA( overlap, 0.5065, 0.003 );
    }

// TODO(math): This is just about to understand what the outcome of small tracts along long tracts in terms of Gaussian process similarity is, and
// can be removed when I understood this to full extent.
//
//    void testSmallTractAlongLongTract( void )
//    {
//        boost::shared_ptr< std::vector< WFiber > > tracts( new std::vector< WFiber > );
//        WFiber tract;
//        for( size_t i = 0; i < 1000; ++i )
//        {
//            tract.push_back( WPosition( static_cast< double >( i ), 0.0, 0.0 ) );
//        }
//        tracts->push_back( tract );
//        tract.clear();
//        for( size_t i = 20; i < 25; ++i )
//        {
//            tract.push_back( WPosition( static_cast< double >( i ), 0.0, 0.0 ) );
//        }
//        tracts->push_back( tract );
//        boost::shared_ptr< WDataSetFiberVector > fvDS( new WDataSetFiberVector( tracts ) );
//
//        WGaussProcess p1( 0, fvDS->toWDataSetFibers(), m_emptyDTIDataSet );
//        WGaussProcess p2( 1, fvDS->toWDataSetFibers(), m_emptyDTIDataSet );
//        std::cout <<  gauss::innerProduct( p1, p2 ) << std::endl;
//        std::cout << std::sqrt( gauss::innerProduct( p1, p1 ) ) << "    " << std::sqrt( gauss::innerProduct( p2, p2 ) ) << std::endl;
//       double overlap = gauss::innerProduct( p1, p2 ) / ( std::sqrt( gauss::innerProduct( p1, p1 ) ) * std::sqrt( gauss::innerProduct( p2, p2 ) ) );
//        TS_ASSERT_DELTA( overlap, 0.501, 0.003 );
//    }

protected:
    /**
     * SetUp test environment.
     */
    void setUp( void )
    {
        WLogger::startup();
        float dataArray[6] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 }; // NOLINT array init list
        boost::shared_ptr< std::vector< float > > data( new std::vector< float >( &dataArray[0],
                    &dataArray[0] + sizeof( dataArray ) / sizeof( float ) ) );
        boost::shared_ptr< WValueSetBase > newValueSet( new WValueSet< float >( 1, 6, data, W_DT_FLOAT ) );
        boost::shared_ptr< WGrid > newGrid( new WGridRegular3D( 1, 1, 1 ) );
        m_emptyDTIDataSet = boost::shared_ptr< WDataSetDTI >(  new WDataSetDTI( newValueSet, newGrid ) );

        boost::shared_ptr< std::vector< WFiber > > tracts( new std::vector< WFiber > );
        WFiber tract0;
        tract0.push_back( WPosition( 0.0, 0.0, 0.0 ) );
        tract0.push_back( WPosition( 1.0, 1.0, 0.0 ) );
        tract0.push_back( WPosition( 1.0, 2.0, 0.0 ) );
        tract0.push_back( WPosition( 2.0, 2.0, 0.0 ) );
        tracts->push_back( tract0 );
        WFiber tract1;
        tract1.push_back( WPosition( 2.0 + 2.0 * sqrt( 8.0 ), 2.0, 0.0 ) );
        tract1.push_back( WPosition( 2.0 + 2.0 * sqrt( 8.0 ) + 1.0, 2.0, 0.0 ) );
        tracts->push_back( tract1 );
        boost::shared_ptr< WDataSetFiberVector > fvDS( new WDataSetFiberVector( tracts ) );
        m_tracts = fvDS->toWDataSetFibers();

        m_tractID = 0;
    }

    /**
     * Clean up everything.
     */
    void tearDown( void )
    {
        m_tracts.reset();
        m_emptyDTIDataSet.reset();
    }

private:
    /**
     * Dummy DTI dataset.
     */
    boost::shared_ptr< WDataSetDTI > m_emptyDTIDataSet;

    /**
     * Dummy tract dataset for Gaussian process generation.
     */
    boost::shared_ptr< WDataSetFibers > m_tracts;

    /**
     * A single tract id;
     */
    size_t m_tractID;
};

#endif  // WGAUSSPROCESS_TEST_H
