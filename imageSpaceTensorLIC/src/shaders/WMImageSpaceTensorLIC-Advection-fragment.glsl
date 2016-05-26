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

#version 120

#include "WGEUtils.glsl"

// This is updated by a callback:
uniform int u_animation;

/**
 * The texture Unit for the projected vectors
 */
uniform sampler2D u_texture0Sampler;

/**
 * The texture Unit for the edges and noise
 */
uniform sampler2D u_texture1Sampler;

/**
 * Size of advection texture in pixels
 */
uniform int u_texture0SizeX;

/**
 * Size of advection texture in pixels
 */
uniform int u_texture0SizeY;

/**
 * Size of advection texture in pixels
 */
uniform int u_texture0SizeZ;

/**
 * The blending ratio between noise and advected noise
 */
uniform float u_noiseRatio = 0.00;

/**
 * Number of iterations per frame.
 */
uniform int u_numIter = 30;

uniform float  u_stepSizeFactor = 1.0;

/**
 * Returns the vector at the given point.
 *
 * \param pos the position to retrieve the vector for
 *
 * \return the unscaled vector in [-1, 1]
 */
vec2 getVec1( in vec2 pos )
{
    return ( 2.0 * ( texture2D( u_texture0Sampler, pos ).rg - vec2( 0.5, 0.5 ) ) );
}

/**
 * Returns the vector at the given point.
 *
 * \param pos the position to retrieve the vector for
 *
 * \return the unscaled vector in [-1, 1]
 */
vec2 getVec2( in vec2 pos )
{
    return ( 2.0 * ( texture2D( u_texture0Sampler, pos ).ba - vec2( 0.5, 0.5 ) ) );
}

/**
 * Returns noise for the given position.
 *
 * \param pos the position
 *
 * \return noise
 */
float getNoise( in vec2 pos )
{
    return texture2D( u_texture1Sampler, pos ).b;
}

float advection1()
{
    vec2 texCoord = gl_TexCoord[0].st;


    vec2 vec    = getVec1( texCoord );

    // simply iterate along the line using the vector at each point
    vec2 lastVec1 = vec;
    vec2 lastPos1 = gl_TexCoord[0].st;
    vec2 lastVec2 = vec;
    vec2 lastPos2 = gl_TexCoord[0].st;
    float sum = 0.0;
    int m = 2 * u_numIter;
    for( int i = 0; i < u_numIter; ++i )
    {
        vec2 newPos1 = lastPos1 + u_stepSizeFactor * vec2( lastVec1.x / ( 2.0 * u_texture0SizeX ), lastVec1.y / ( 2.0 * u_texture0SizeY ) );
        vec2 newPos2 = lastPos2 - u_stepSizeFactor * vec2( lastVec2.x / ( 2.0 * u_texture0SizeX ), lastVec2.y / ( 2.0 * u_texture0SizeY ) );
        vec2 newVec1 = getVec1( newPos1 );
        vec2 newVec2 = getVec1( newPos2 );

        // if( ( length( newVec1 ) < 0.01 ) || ( length( newVec2 )  < 0.01 ) )
        // {
        //     m = 2 * i;
        //     break;
        // }

        // it is also possible to scale using a Geometric progression: float( u_numIter - i ) / u_numIter * texture2D
        sum += getNoise( newPos1 );
        sum += getNoise( newPos2 );

        lastPos1 = newPos1;
        lastVec1 = newVec1;
        lastPos2 = newPos2;
        lastVec2 = newVec2;
    }

    // the sum needs to be scaled to [0,1] again
    float n = sum / float( m );

    return n;
}


float advection2()
{
    vec2 texCoord = gl_TexCoord[0].st;


    vec2 vec    = getVec2( texCoord );

    // simply iterate along the line using the vector at each point
    vec2 lastVec1 = vec;
    vec2 lastPos1 = gl_TexCoord[0].st;
    vec2 lastVec2 = vec;
    vec2 lastPos2 = gl_TexCoord[0].st;
    float sum = 0.0;
    int m = 2 * u_numIter;
    for( int i = 0; i < u_numIter; ++i )
    {
        vec2 newPos1 = lastPos1 + u_stepSizeFactor * vec2( lastVec1.x / ( 2.0 * u_texture0SizeX ), lastVec1.y / ( 2.0 * u_texture0SizeY ) );
        vec2 newPos2 = lastPos2 - u_stepSizeFactor * vec2( lastVec2.x / ( 2.0 * u_texture0SizeX ), lastVec2.y / ( 2.0 * u_texture0SizeY ) );
        vec2 newVec1 = getVec2( newPos1 );
        vec2 newVec2 = getVec2( newPos2 );

        // if( ( length( newVec1 ) < 0.01 ) || ( length( newVec2 )  < 0.01 ) )
        // {
        //     m = 2 * i;
        //     break;
        // }

        // it is also possible to scale using a Geometric progression: float( u_numIter - i ) / u_numIter * texture2D
        sum += getNoise( newPos1 );
        sum += getNoise( newPos2 );

        lastPos1 = newPos1;
        lastVec1 = newVec1;
        lastPos2 = newPos2;
        lastVec2 = newVec2;
    }

    // the sum needs to be scaled to [0,1] again
    float n = sum / float( m );

    return n;
}


/**
 * Main. Calculates the Laplace Filter for each pixel.
 */
void main()
{
    vec2 texCoord = gl_TexCoord[0].st;

    // get some needed values
    float edge  = texture2D( u_texture1Sampler, texCoord ).r;
    float depth = texture2D( u_texture1Sampler, texCoord ).g;
    float noise = getNoise( texCoord );

    float n1 = advection1();
    float n2 = advection2();

    if( depth > 0.99 )
    {
        n1 = noise;
        n2 = noise;
    }

    // finally, blend noise and old noise
    gl_FragColor = vec4( n1, n2, 0.0, 1.0 );
}

