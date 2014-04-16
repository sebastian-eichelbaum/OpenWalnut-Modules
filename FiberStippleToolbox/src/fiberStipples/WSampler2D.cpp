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

#include <fstream>

#include <boost/array.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <core/common/WProgress.h>
#include <core/common/WIOTools.h>

#include "ext/PDSampling.h"
#include "WSampler2D.h"

// Anonymous namespace for some constants only used in here
namespace
{
    const double rndMax = RAND_MAX;
}

WSampler2DBase::WSampler2DBase( size_t numSamples, double width, double height, RandomInit init )
    : WSampler2D(),
      m_width( width ),
      m_height( height )
{
    reserve( numSamples );
    if( init == CALL_SRAND )
    {
        srand( time( NULL ) );
    }

}


WSampler2DRegular::WSampler2DRegular( size_t numSamples, double width, double height )
    : WSampler2DBase( numSamples, width, height )
{
    double maxX = std::sqrt( static_cast< double >( numSamples ) );
    double maxY = maxX;
    for( double col = 0.0; col < width; col += width / maxX )
    {
        for( double row = 0.0; row < height; row += height / maxY )
        {
            push_back( WVector2d( row, col ) );
        }
    }
}

WSampler2DStratified::WSampler2DStratified( size_t numSamples, double width, double height, RandomInit init )
    : WSampler2DBase( numSamples, width, height, init )
{
    double maxX = std::sqrt( static_cast< double >( numSamples ) );
    double maxY = maxX;
    for( double col = 0.0; col < width; col += width / maxX )
    {
        for( double row = 0.0; row < height; row += height / maxY )
        {
            double rnd1 = std::rand() / rndMax;
            double rnd2 = std::rand() / rndMax;
            push_back( WVector2d( row, col ) + WVector2d( rnd1 * height / maxY, rnd2 * width / maxX ) );
        }
    }
}

WSampler2DUniform::WSampler2DUniform(  size_t numSamples, double width, double height, RandomInit init )
    : WSampler2DBase( numSamples, width, height, init )
{
    for( size_t i = 0; i < numSamples; ++i )
    {
        push_back( WVector2d( std::rand() / rndMax * m_width, std::rand() / rndMax * m_height ) );
    }
}

WSampler2DPoisson::WSampler2DPoisson( float radius )
    : WSampler2D(),
      m_radius( radius )
{
    boost::shared_ptr< PDSampler > sampler( new BoundarySampler( radius, true ) );
    sampler->complete(); // generates PDSampling in [-1,1]^2 domain.

    double x,y;

    for( size_t i = 0; i < sampler->points.size(); ++i )
    {
        // rescale to [0,1]^2 domain
        x = ( sampler->points[i].x + 1.0 ) / 2.0;
        y = ( sampler->points[i].y + 1.0 ) / 2.0;
        push_back( WVector2d( x, y ) );
    }
}

std::vector< WSampler2D > splitSampling( const WSampler2D& sampler, size_t numComponents )
{
    size_t numPointsEach = sampler.size() / numComponents;
    WSampler2D points( sampler );
    std::random_shuffle( points.begin(), points.end() );

    std::vector< WSampler2D > components;
    for( size_t i = 0; i < numComponents; ++i )
    {
        WSampler2D comp;
        for( size_t j = 0; j < numPointsEach; ++j )
        {

            comp.push_back( points[ i * numPointsEach + j ] );
        }
        components.push_back( comp );
    }
    // discard points
    return components;
}

namespace
{
    template <typename I>
    I random_element(I begin, I end)
    {
        const unsigned long n = std::distance( begin, end );
        if( n == 0 )
        {
            return begin;
        }
        const unsigned long divisor = ( RAND_MAX ) / n;
        unsigned long k;
        do
        {
            k = std::rand() / divisor;
        }
        while( k >= n );
        std::advance( begin, k );
        return begin;
    }

    template <typename I>
    bool valid( const WVector2d& p, I begin, I end, double radiusSquared )
    {
        for( I it = begin; it != end; ++it )
        {
            if( length2( p - *it ) < radiusSquared )
            {
                return false;
            }
        }
        return true;
    }

    bool valid( const WVector2d& p, const std::vector< WSampler2D >& c, double radiusSquared )
    {
        for( size_t i = 0; i < c.size(); ++i )
        {
            if( !valid( p, c[i].begin(), c[i].end(), radiusSquared ) )
            {
                return false;
            }
        }
        return true;
    }
}

namespace {

void saveHierarchy( std::vector< WSampler2D >& hier, std::string fname ) {
    Hierarchy h( hier );
    std::ofstream ofs( fname.c_str() );
    {
        boost::archive::text_oarchive oa(ofs);
        oa << h;
    }
}

std::vector< WSampler2D > loadHierarchy( std::string fname ) {
    Hierarchy h;
    std::ifstream ifs( fname.c_str() );
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> h;
    }
    return h.get();
}
}

// TODO(math): Remove this ugly hack as soon as possible
std::vector< WSampler2D > splitSamplingPoisson( const WSampler2D& sampler, size_t numComponents, boost::shared_ptr< WProgress > progress )
{
   std::vector< WSampler2D > components;

    std::string cache( "/tmp/klaus" );
    if( fileExists( cache ) ) {
        components = loadHierarchy( cache );
    }
    else {
        WSampler2D points( sampler );
        std::random_shuffle( points.begin(), points.end() );
        size_t numPoints = points.size(); // we need this as we want to delete in O(1)
        boost::array< double, 20 > coeff = {{ 7.0,
                                              5.0,
                                              4.0,
                                              3.5,
                                              3.0,
                                              2.6,
                                              2.3,
                                              2.1,
                                              2.02,
                                              2.00001,
                                              2.000005,
                                              2.000001,
                                              1.9999995,
                                              1.999999,
                                              1.999998,
                                              1.999997,
                                              1.9999955,
                                              1.999994,
                                              1.999992,
                                              1.99999}};

        for( size_t i = 0; i < numComponents; ++i )
        {
            double r2 = 0.001 * 0.001 * coeff[i] * coeff[i];
            WSampler2D comp;

            size_t failure = 0;
            while( numPoints > 0 && failure < 180 )
            {
                size_t n = std::rand() % numPoints;
                if( !valid( points[n], components, r2 ) || !valid( points[n], comp.begin(), comp.end(), r2 ) )
                {
                    failure++;
                }
                else
                {
                    comp.push_back( points[n] );
                    std::swap( points[n], points[ numPoints - 1 ] );
                    numPoints -= 1;
                    failure = 0;
                }
            }
            components.push_back( comp );
            ++*progress;
        }
    }

    saveHierarchy( components, "/tmp/klaus" );

    progress->finish();

    // discard points
    return components;
}



