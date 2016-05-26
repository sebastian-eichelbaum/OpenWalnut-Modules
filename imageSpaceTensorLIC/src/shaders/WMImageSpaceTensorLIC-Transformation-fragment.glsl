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

#include "WGEColormapping-fragment.glsl"

#include "WGEShadingTools.glsl"
#include "WGETextureTools.glsl"
#include "WGETransformationTools.glsl"
#include "WGEUtils.glsl"

#include "WMImageSpaceTensorLIC-Transformation-varyings.glsl"

/**
 * The texture unit sampler: evec1
 */
uniform sampler3D u_texture0Sampler;

/**
 * The texture unit sampler: evec2
 */
uniform sampler3D u_texture1Sampler;

/**
 * The texture unit sampler: evals
 */
uniform sampler3D u_texture2Sampler;

/**
 * The texture unit sampler for the noise texture
 */
uniform sampler2D u_texture3Sampler;

/**
 * Scaling factor to unscale the texture
 */
uniform float u_texture0Scale = 1.0;

/**
 * Smallest possible value in the texture
 */
uniform float u_texture0Min = 0.0;

/**
 * Scaling factor to unscale the texture
 */
uniform float u_texture1Scale = 1.0;

/**
 * Smallest possible value in the texture
 */
uniform float u_texture1Min = 0.0;
/**
 * Scaling factor to unscale the texture
 */
uniform float u_texture2Scale = 1.0;

/**
 * Smallest possible value in the texture
 */
uniform float u_texture2Min = 0.0;

/**
 * Size of input texture in pixels
 */
uniform int u_texture0SizeX = 255;

/**
 * Size of input texture in pixels
 */
uniform int u_texture0SizeY = 255;

/**
 * Size of input texture in pixels
 */
uniform int u_texture0SizeZ = 255;

/**
 * Size of input texture in pixels
 */
uniform int u_texture3SizeX = 255;

/**
 * Size of input texture in pixels
 */
uniform int u_texture3SizeY = 255;

/**
 * Projection angle cutoff
 */
uniform float u_projectionAngleThreshold;

/**
 * The "virtual" resolution of the noise texture in u_texture1Sampler
 */
uniform float u_noiseResoultuion = 3.0;

/**
 * Clip by FA
 */
uniform float u_clipFA = 0.01;

// FA as used everywhere (Kondratieva et al 2005)
// computes FA from eigenvalues
float getFA( vec3 evalues )
{
  float sqrt32 = sqrt(3./2.); // would make this const, if compiler let me
  float I1 = evalues.x+evalues.y+evalues.z;
  float mu = I1/3.;
  vec3 deriv;
  deriv.x = (evalues.x-mu);
  deriv.y = (evalues.y-mu);
  deriv.z = (evalues.z-mu);
  float FA = sqrt32 * length(deriv)/length(evalues);
  return FA;
}

/**
 * Transforms each vector on each pixel to image space.
 */
void main()
{
    vec2 noiseTexCoords = vec2( 0.0 );

#define CUBETEXTUREMAPPING_VARIANT2
//#define CUBETEXTUREMAPPING_VARIANT1_A
#define CUBETEXTUREMAPPING_VARIANT1_B

#ifdef CUBETEXTUREMAPPING_VARIANT1
    vec3 v = ( v_vertex.xyz ) - floor( v_vertex.xyz );
    vec3 normal = vec3( isZero( v_normalObject.x, 0.00001 ) ? 0.00001 : v_normalObject.x,
                        isZero( v_normalObject.y, 0.00001 ) ? 0.00001 : v_normalObject.y,
                        isZero( v_normalObject.z, 0.00001 ) ? 0.00001 : v_normalObject.z );

    // find nearest cube planes in direction of the normal
    int nearestPlane = 0;
    float smallestD = 1e10;
    vec3 cubeBase = vec3( 0.0 );//cubeBaseX, cubeBaseY, cubeBaseZ);

#ifdef CUBETEXTUREMAPPING_VARIANT1_A
    // try each of the 6 planes and find nearest (positive) hit
    for( int axis=0; axis<3; axis++ )
    {
        // search the n in :  vertex[axis]+n*_normal[axis] = 1.0 | 0.0
        float n1 = ( 1.0 - v[axis] ) / v_normalObject[axis];
        float n2 = (     - v[axis] ) / v_normalObject[axis];

        // NOTE this should also handle the case where the vertex is directly on the cube -> n1=n2=~0

        // smaller than currently found one?
        if( abs( n1 ) < abs( smallestD ) )
        {
            smallestD = n1;
            nearestPlane = axis;
        }
        else if( abs( n2 ) < abs( smallestD ) )
        {
            smallestD = n2;
            nearestPlane = axis;
        }
    }
#endif

#ifdef CUBETEXTUREMAPPING_VARIANT1_B
    // try each of the 6 planes and find nearest (positive) hit
    for( int axis=0; axis<3; axis++ )
    {
        float d1 = v[axis];
        float d2 = 1.0 - v[axis];

        // NOTE this should also handle the case where the vertex is directly on the cube -> d1=d2=~0

        // just take the smaller one
        float d = min( d1, d2 );

        // smaller than currently found one?
        if ( d < smallestD )
        {
            smallestD = d;
            nearestPlane = axis;
        }
    }
#endif

    // calculate hit point on cube
    // NOTE the hit will be in interval [0,1]
    vec3 hit = v.xyz + ( smallestD * normal.xyz )  - floor( v.xyz );

    /* Assume the following border configuration for our cube tile set (same letters - same border)
    FIXME rotate image 90 degree anti clockwise since the texture is stored this way
      _______________   _______________
     | BBBBBBBBBBBBB | | CCCCCCCCCCCCC |
     |A             A| |A             A|
     |A             A| |A             A|
     |A    Tile0    A| |A    Tile1    A|
     |A             A| |A             A|
     |A             A| |A             A|
     |A             A| |A             A|
     |_BBBBBBBBBBBBB_| |_CCCCCCCCCCCCC_|
      _______________
     | BBBBBBBBBBBBB |
     |C             C|
     |C             C|
     |C    Tile2    C|
     |C             C|
     |C             C|
     |C             C|
     |_BBBBBBBBBBBBB_|
    */
    if( nearestPlane == 0 )
    {
        noiseTexCoords = vec2( hit.y, hit.z );
    }
    else if( nearestPlane == 1 )
    {
        noiseTexCoords = vec2( hit.x, hit.z );
    }
    else
    {
        noiseTexCoords = vec2( hit.x, hit.y );
    }

#endif

#ifdef CUBETEXTUREMAPPING_VARIANT2
    vec3 v = v_vertex.xyz;// - floor( v_vertex.xyz );
    float scaler = ( u_noiseResoultuion / 100.0 );
    v*=scaler;
    if( abs( v_normalObject.x ) == max( abs( v_normalObject.x ), max( abs( v_normalObject.y ), abs( v_normalObject.z ) ) ) )
    {
        noiseTexCoords = vec2( v.y, v.z );
    }
    else if( abs( v_normalObject.y ) == max( abs( v_normalObject.x ), max( abs( v_normalObject.y ), abs( v_normalObject.z ) ) ) )
    {
        noiseTexCoords = vec2( v.x, v.z );
    }
    else if( abs( v_normalObject.z ) == max( abs( v_normalObject.x ), max( abs( v_normalObject.y ), abs ( v_normalObject.z ) ) ) )
    {
        noiseTexCoords = vec2( v.x, v.y );
    }
#endif

#ifdef CUBETEXTUREMAPPING_VARIANT3
    float scaler = 10.0 / u_noiseResoultuion ;
    noiseTexCoords = vec2( scaler * gl_FragCoord.x / u_texture3SizeX,
                           scaler * gl_FragCoord.y / u_texture3SizeY );
#endif

    // if we have a 3D noise texture, use it.
    float noise = texture2D( u_texture3Sampler, noiseTexCoords.xy ).r;

    // get the current vector at this position
    vec3 evec1 = normalize( texture3DUnscaled( u_texture0Sampler, gl_TexCoord[0].xyz, u_texture0Min, u_texture0Scale ).rgb );
    vec3 evec2 = normalize( texture3DUnscaled( u_texture1Sampler, gl_TexCoord[1].xyz, u_texture1Min, u_texture1Scale ).rgb );
    evec1 *= sign( evec1.x );
    evec2 *= sign( evec2.x );

    vec3 evals = 1000. * texture3DUnscaled( u_texture2Sampler, gl_TexCoord[2].xyz, u_texture2Min, u_texture2Scale ).rgb;
    //evec1 *= evals.r;
    //evec2 *= evals.g;
    float fa = getFA( evals );

    // zero length vectors are uninteresting. discard them
    if( isZero( evals.r, 0.01 ) ||
        isZero( evals.g, 0.01 ) )
    {
        discard;
    }
    if( fa < u_clipFA )
    {
        discard;
    }
    // project the vectors to image space as the input vector is in the tangent space
    vec2 evec1Projected = normalize( projectVector( evec1 ).xy );
    vec2 evec2Projected = normalize( projectVector( evec2 ).xy );

    // calculate lighting for the surface
    float light = blinnPhongIlluminationIntensity( normalize( v_normal ) );

    // MPI PAper Hack: {
    // vec4 cmap = colormapping();
    // gl_FragData[1] = vec4( vec3( cmap.r ), 1.0 );
    //
    // if( cmap.r < 0.15 )
    //     discard;
    // if( isZero( cmap.r - 0.2, 0.1 ) )
    //     gl_FragData[1] = vec4( 0.25, 0.0, 0.0, 1.0 );
    // if( isZero( cmap.r - 0.3, 0.1 ) )
    //     gl_FragData[1] = vec4( 0.5, 0.0, 0.0, 1.0 );
    // if( isZero( cmap.r - 1.0, 0.15 ) )
    //     gl_FragData[1] = vec4( 1.0, 0.0, 0.0, 1.0 );
    // }

    vec4 cmap = colormapping();

    // this is an example for avoiding certain pixels from beeing LIC'd
    // if(( cmap.r > 0.3 ) &&  ( cmap.g > 0.3 ) && ( cmap.b > 0.3 ) )
    //     discard;

    const float piHalf = 3.14159265 / 2.0;
    const float pi = 3.14159265;

    // is the vector very orthogonal to the surface? We scale our advection according to this. We use the angle between normal and vector but use
    // 1 - dot to get the angle between vector and surface which is 90 degree shifted against the dot of the normal and the vector.
    float geometryVecAngle = ( acos(
                                     abs(
                                          dot( normalize( v_normalObject ),
                                               normalize( evec1.xyz )
                                             )
                                        )
                                   ) / piHalf
                             );
    // we measured the angle between normal and vector, but we want the angle between surface tangent and vector, which is a shift of half PI /
    // halfPi = 1
    geometryVecAngle = 1.0 - geometryVecAngle;

    // the user defines an angle in degrees used as cutoff value. Angles above this one should be clipped
    float cutoffAngle = u_projectionAngleThreshold / 90.0;

    // scale down the vector -> the more it "comes out of the surface, the shorter the vector".
    float dotScale = cutoffAngle - geometryVecAngle;
    if( dotScale < 0 )
    {
        dotScale = 0;
    }
    else
    {
        dotScale = 1.0;
    }

    // scale our vector according to the above metric and output
    vec2 dotScaledVector1 = dotScale * scaleMaxToOne( evec1Projected ).xy;
    vec2 dotScaledVector2 = dotScale * scaleMaxToOne( evec2Projected ).xy;
    gl_FragData[0] = vec4( vec2( 0.5 ) + ( 0.5  * dotScaledVector1 ), vec2( 0.5 ) + ( 0.5  * dotScaledVector2 ) );
    gl_FragData[1] = vec4( light, noise, gl_FragCoord.z, 1.0 );
    gl_FragData[2] = vec4( abs( evals.r - evals.g ), fa, 0.0, 1.0 );
    gl_FragData[3] = cmap;

}

