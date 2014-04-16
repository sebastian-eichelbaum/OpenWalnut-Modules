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

#ifndef WSAMPLER2D_H
#define WSAMPLER2D_H

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/vector.hpp>

#include <core/common/math/linearAlgebra/WVectorFixed.h> // for WVector2d
#include <core/common/WMixinVector.h>
#include <core/common/WProgress.h>

/**
 * Generates random points on a two dimensional disk (aka plane, aka slice).
 */
class WSampler2D : public WMixinVector< WVector2d >
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2D > SPtr;

    WSampler2D() {}

    explicit WSampler2D( std::vector< WVector2d > v ) {
        for( size_t i = 0; i < v.size(); ++i )
        {
            push_back( v[i] );
        }
    }

};

/**
 * I hate bool flags, (as the calls yields in: foo( true, true, false )). To manage if we need to
 * call std::srand or not, this enum is used.
 */
enum RandomInit
{
    CALL_SRAND,
    DONT_CALL_SRAND
};

class WSampler2DBase : public WSampler2D
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2DBase > SPtr;

    WSampler2DBase( size_t numSamples, double width, double height, RandomInit init = DONT_CALL_SRAND );

protected:
    /**
     * X dimension.
     */
    double m_width;

    /**
     * Y dimension.
     */
    double m_height;
};

/**
 * Generates uniform distributed random positions within the given a rectangular domain.
 */
class WSampler2DUniform : public WSampler2DBase
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2DUniform > SPtr;

    /**
     * Generates random points distributed uniformly within the given rectangular 2D slice.
     *
     * \param numSamples How many samples.
     * \param width X dimension
     * \param height Y dimension
     * \param init Flag for calling srand or not
     */
    explicit WSampler2DUniform( size_t numSamples, double width = 1.0, double height = 1.0, RandomInit init = CALL_SRAND );
};

class WSampler2DRegular : public WSampler2DBase
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2DRegular > SPtr;

    /**
     * Generates stratified sampled points distributed uniformly within each cell.
     *
     * \param numSamples How many samples.
     * \param width X dimension
     * \param height Y dimension
     * \param init Flag for calling srand or not
     */
    explicit WSampler2DRegular( size_t numSamples, double width = 1.0, double height = 1.0 );
};

class WSampler2DStratified : public WSampler2DBase
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2DStratified > SPtr;

    /**
     * Generates stratified sampled points distributed uniformly within each cell.
     *
     * \param numSamples How many samples.
     * \param width X dimension
     * \param height Y dimension
     * \param init Flag for calling srand or not
     */
    explicit WSampler2DStratified( size_t numSamples, double width = 1.0, double height = 1.0, RandomInit init = CALL_SRAND );
};


/**
 * Creates a Poisson Disk sampling in the [0,1]^2 domain.
 * \note This sampling can be used as tile.
 * \note This is very fast (~200.000pts in less than a second).
 */
class WSampler2DPoisson : public WSampler2D
{
public:
    explicit WSampler2DPoisson( float radius );
//    void scale( double width, double height );
private:
    float m_radius;
};

class Hierarchy {
private:
    friend class boost::serialization::access;
    struct Point2d {
        Point2d() {}
        Point2d( WVector2d v ) {
            x = v[0];
            y = v[1];
        }
        double x;
        double y;
        WVector2d get() {
            return WVector2d( x, y );
        }
        template<class Archive> void serialize(Archive & ar, const unsigned int /* version */)
        {
            ar & x;
            ar & y;
        }
    };
    std::vector< std::vector< Point2d > > m_h;
    template<class Archive> void serialize(Archive & ar, const unsigned int /* version */)
    {
        ar & m_h;
    }
public:
    Hierarchy() {
    }

    explicit Hierarchy( std::vector< WSampler2D >& h ) {
        set( h );
    }

    void set( std::vector< WSampler2D >& h ) {
        for( size_t i = 0; i < h.size(); ++i ) {
            std::vector< Point2d > l;
            for( size_t j = 0; j < h[i].size(); ++j ) {
                l.push_back( Point2d( h[i][j] ) );
            }
            m_h.push_back( l );
        }
    }

    std::vector< WSampler2D > get() {
        std::vector< WSampler2D > result;
        for( size_t i = 0; i < m_h.size(); ++i ) {
            std::vector< WVector2d > l;
            for( size_t j = 0; j < m_h[i].size(); ++j ) {
                l.push_back( WVector2d( m_h[i][j].x, m_h[i][j].y ) );
            }
            WSampler2D s( l );
            result.push_back( s );
        }
        return result;
    }
};

std::vector< WSampler2D > splitSampling( const WSampler2D& sampler, size_t numComponents );
std::vector< WSampler2D > splitSamplingPoisson( const WSampler2D& sampler, size_t numComponents, boost::shared_ptr< WProgress > splitProgress );
#endif // WSAMPLER2D_H

