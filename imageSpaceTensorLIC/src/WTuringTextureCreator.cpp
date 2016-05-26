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

#include <iostream>
#include <vector>

#include <boost/random.hpp>

#include <core/common/exceptions/WPreconditionNotMet.h>

#include "WTuringTextureCreator.h"

WTuringTextureCreator::WTuringTextureCreator( std::size_t numThreads )
    : m_numIterations( 100 ),
      m_spotIrregularity( 0.1 ),
      m_spotSize( 0.1 )
{
    WPrecond( numThreads > 0, "" );

    for( std::size_t k = 0; k < numThreads; ++k )
    {
        m_threads.push_back( boost::shared_ptr< TextureThread >( new TextureThread( k, numThreads ) ) );
    }
}

WTuringTextureCreator::~WTuringTextureCreator()
{
}

void WTuringTextureCreator::setNumIterations( std::size_t iter )
{
    WPrecond( iter > 0, "Invalid number of iterations!" );

    m_numIterations = iter;
}

void WTuringTextureCreator::setSpotIrregularity( float irr )
{
    WPrecond( irr >= 0.0f && irr <= 1.0f, "Spot irregularity must be in [0,1]!" );

    m_spotIrregularity = irr;
}

void WTuringTextureCreator::setSpotSize( float size )
{
    WPrecond( size >= 0.0f && size <= 1.0f, "Spot size must be in [0,1]!" );

    m_spotSize = size;
}

osg::ref_ptr< WGETexture3D > WTuringTextureCreator::create( std::size_t sizeX, std::size_t sizeY, std::size_t sizeZ )
{
    // some constants, maybe change to parameters
    float const spotFactor = ( 0.02f + 0.58f * ( 1.0f - m_spotSize ) ) / 15.0f;
    float const d1 = 0.125f;
    float const d2 = 0.03125f;
    float const speed = 1.0f;

    std::vector< float > concentration1( sizeX * sizeY * sizeZ, 4.0f );
    std::vector< float > concentration2( sizeX * sizeY * sizeZ, 4.0f );
    std::vector< float > delta1( sizeX * sizeY * sizeZ, 0.0f );
    std::vector< float > delta2( sizeX * sizeY * sizeZ, 0.0f );
    std::vector< float > noise( sizeX * sizeY * sizeZ );

    boost::mt19937 generator( std::time( 0 ) );

    float noiseRange = 0.1f + 4.9f * m_spotIrregularity;

    boost::uniform_real< float > dist( 12.0f - noiseRange, 12.0f + noiseRange );
    boost::variate_generator< boost::mt19937&, boost::uniform_real< float > > rand( generator, dist );

    // initialize noise
    for( std::size_t k = 0; k < sizeX * sizeY * sizeZ; ++k )
    {
        noise[ k ] = rand();
    }

    // iterate
    for( std::size_t iter = 0; iter < m_numIterations; ++iter )
    {
        std::cout << "iteration: " << iter << std::endl;

        // step one: calculate change in concentration across the volume
        for( std::size_t k = 0; k < m_threads.size(); ++k )
        {
            m_threads[ k ]->setTextureSize( sizeX, sizeY, sizeZ );
            m_threads[ k ]->setSpotFactor( spotFactor );
            m_threads[ k ]->setDiffusionConstants( d1, d2 );
            m_threads[ k ]->setBufferPointers( &concentration1, &concentration2, &noise, &delta1, &delta2 );
            m_threads[ k ]->run();
        }

        for( std::size_t k = 0; k < m_threads.size(); ++k )
        {
            m_threads[ k ]->wait();
        }

        // applying the change in concentration is not too costly
        for( std::size_t k = 0; k < sizeX * sizeY * sizeZ; ++k )
        {
            concentration1[ k ] += speed * delta1[ k ];
            concentration2[ k ] += speed * delta2[ k ];
        }
    }

    // allocate new osg::Image for the texture data
    osg::ref_ptr< osg::Image > img = new osg::Image;
    img->allocateImage( sizeX, sizeY, sizeZ, GL_LUMINANCE, GL_UNSIGNED_BYTE );

    // find min and max
    float c1min = *std::min_element( concentration1.begin(), concentration1.end() );
    float c1max = *std::max_element( concentration1.begin(), concentration1.end() );

    // copy to image
    for( std::size_t k = 0; k < sizeX * sizeY * sizeZ; ++k )
    {
        img->data()[ k ] = 255.0f * ( concentration1[ k ] - c1min ) / ( c1max - c1min );
    }

    return osg::ref_ptr< WGETexture< osg::Texture3D > >( new WGETexture< osg::Texture3D >( img ) );
}

WTuringTextureCreator::TextureThread::TextureThread( std::size_t id, std::size_t max )
    : WThreadedRunner(),
      m_id( id ),
      m_maxThreads( max ),
      m_sizeX( 2 ),
      m_sizeY( 2 ),
      m_sizeZ( 2 ),
      m_spotFactor( 0.5 ),
      m_diffusionConstant1( 0.5 ),
      m_diffusionConstant2( 0.5 )
{
}

WTuringTextureCreator::TextureThread::~TextureThread()
{
}

void WTuringTextureCreator::TextureThread::setDiffusionConstants( float d1, float d2 )
{
    WPrecond( d1 >= 0.0 && d1 <= 1.0, "" );
    WPrecond( d2 >= 0.0 && d2 <= 1.0, "" );

    m_diffusionConstant1 = d1;
    m_diffusionConstant2 = d2;
}

void WTuringTextureCreator::TextureThread::setSpotFactor( float spotFactor )
{
    WPrecond( spotFactor > 0.0, "" );

    m_spotFactor = spotFactor;
}

void WTuringTextureCreator::TextureThread::setTextureSize( std::size_t sizeX, std::size_t sizeY, std::size_t sizeZ )
{
    WPrecond( sizeX > 0, "" );
    WPrecond( sizeY > 0, "" );
    WPrecond( sizeZ > 0, "" );

    m_sizeX = sizeX;
    m_sizeY = sizeY;
    m_sizeZ = sizeZ;
}

void WTuringTextureCreator::TextureThread::threadMain()
{
    WPrecond( m_concentration1 != 0, "Invalid pointer!" );
    WPrecond( m_concentration2 != 0, "Invalid pointer!" );
    WPrecond( m_noise != 0, "Invalid pointer!" );
    WPrecond( m_delta1 != 0, "Invalid pointer!" );
    WPrecond( m_delta2 != 0, "Invalid pointer!" );

    std::size_t const numVoxels = m_sizeX * m_sizeY * m_sizeZ;

    std::size_t start = m_id * ( numVoxels / m_maxThreads );
    std::size_t end = ( m_id + 1 ) * ( numVoxels / m_maxThreads );

    if( m_id == m_maxThreads - 1 )
    {
        end = numVoxels;
    }

    for( std::size_t idx = start; idx < end; ++idx )
    {
        std::size_t i = idx % m_sizeX;
        std::size_t j = ( idx / m_sizeX ) % m_sizeY;
        std::size_t k = ( idx / m_sizeX ) / m_sizeY;

        std::size_t iNext = ( i + 1 ) % m_sizeX;
        std::size_t iPrev = ( i + m_sizeX - 1 ) % m_sizeX;

        std::size_t jNext = ( j + 1 ) % m_sizeY;
        std::size_t jPrev = ( j + m_sizeY - 1 ) % m_sizeY;

        std::size_t kNext = ( k + 1 ) % m_sizeZ;
        std::size_t kPrev = ( k + m_sizeZ - 1 ) % m_sizeZ;

        // estimate change in concentrations using 3d laplace filter
        float dc1 = 0.0;
        dc1 += ( *m_concentration1 )[ iPrev + j * m_sizeX + k * m_sizeX * m_sizeY ];
        dc1 += ( *m_concentration1 )[ iNext + j * m_sizeX + k * m_sizeX * m_sizeY ];
        dc1 += ( *m_concentration1 )[ i + jPrev * m_sizeX + k * m_sizeX * m_sizeY ];
        dc1 += ( *m_concentration1 )[ i + jNext * m_sizeX + k * m_sizeX * m_sizeY ];
        dc1 += ( *m_concentration1 )[ i + j * m_sizeX + kPrev * m_sizeX * m_sizeY ];
        dc1 += ( *m_concentration1 )[ i + j * m_sizeX + kNext * m_sizeX * m_sizeY ];
        dc1 -= 6.0f * ( *m_concentration1 )[ idx ];

        float dc2 = 0.0;
        dc2 += ( *m_concentration2 )[ iPrev + j * m_sizeX + k * m_sizeX * m_sizeY ];
        dc2 += ( *m_concentration2 )[ iNext + j * m_sizeX + k * m_sizeX * m_sizeY ];
        dc2 += ( *m_concentration2 )[ i + jPrev * m_sizeX + k * m_sizeX * m_sizeY ];
        dc2 += ( *m_concentration2 )[ i + jNext * m_sizeX + k * m_sizeX * m_sizeY ];
        dc2 += ( *m_concentration2 )[ i + j * m_sizeX + kPrev * m_sizeX * m_sizeY ];
        dc2 += ( *m_concentration2 )[ i + j * m_sizeX + kNext * m_sizeX * m_sizeY ];
        dc2 -= 6.0f * ( *m_concentration2 )[ idx ];

        // diffusion
        ( *m_delta1 )[ idx ] = m_spotFactor * ( 16.0f - ( *m_concentration1 )[ idx ] * ( *m_concentration2 )[ idx ] ) + m_diffusionConstant1 * dc1;
        ( *m_delta2 )[ idx ] = m_spotFactor * ( ( *m_concentration1 )[ idx ] * ( *m_concentration2 )[ idx ] - ( *m_concentration2 )[ idx ] - ( *m_noise )[ idx ] ) + m_diffusionConstant2 * dc2;
    }
}

void WTuringTextureCreator::TextureThread::setBufferPointers( std::vector< float > const* concentration1, std::vector< float > const* concentration2,
                                                              std::vector< float > const* noise, std::vector< float >* delta1, std::vector< float >* delta2 )
{
    WPrecond( concentration1 != 0, "Invalid pointer!" );
    WPrecond( concentration2 != 0, "Invalid pointer!" );
    WPrecond( noise != 0, "Invalid pointer!" );
    WPrecond( delta1 != 0, "Invalid pointer!" );
    WPrecond( delta2 != 0, "Invalid pointer!" );

    m_concentration1 = concentration1;
    m_concentration2 = concentration2;
    m_noise = noise;
    m_delta1 = delta1;
    m_delta2 = delta2;
}
