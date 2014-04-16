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

#include "WIsolines-varyings.glsl"

/**
 * The matrix describes the transformation of gl_Vertex to OpenWalnut Scene Space
 */
uniform mat4 u_WorldTransform;

/**
 * Scalar dataset to draw contours from as texture
 */
uniform sampler3D u_scalarDataSampler;

/**
 * Isovalue
 */
uniform float u_isovalue;


// Dimensions of the dataset, given by wge::bindTexture
/**
 * Dimension in X
 */
uniform int u_scalarDataSizeX;

/**
 * Dimension in Y
 */
uniform int u_scalarDataSizeY;

/**
 * Dimension in Z
 */
uniform int u_scalarDataSizeZ;

/**
 * First direction of the slice.
 */
uniform vec3 u_aVec;

/**
 * Second direction of the slice
 */
uniform vec3 u_bVec;

/**
 * Quad size
 */
uniform float u_resolution; // could be calculated from gl_TexCoord[1], but for better readability

/**
 * Computes the texture coordinates out of world coordinates.
 *
 * \param p Position in Worldspace.
 *
 * \return Position in Texture space.
 */
vec3 textPos( vec3 p )
{
    // compute texture coordinates from worldspace coordinates for texture access
    vec3 texturePosition = ( u_WorldTransform * vec4( p, 1.0 ) ).xyz;
    texturePosition.x /= u_scalarDataSizeX;
    texturePosition.y /= u_scalarDataSizeY;
    texturePosition.z /= u_scalarDataSizeZ;

    return texturePosition;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(13.9898,78.233))) * 43758.5453);
}

/**
 * Vertex Main.
 */
void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0; // for distinguishing the verties of the quad,
    gl_TexCoord[1] = gl_MultiTexCoord1; // providing coordinates for the fragment shader

    vec3 u_normA = normalize( u_aVec );
    vec3 u_normB = normalize( u_bVec );

    // gather data from all corners
    float d0 = texture3D( u_scalarDataSampler, textPos( gl_Vertex.xyz + ( u_normA * -1.0 + -u_normB ) * 0.5 * u_resolution ) ).r;
    float d1 = texture3D( u_scalarDataSampler, textPos( gl_Vertex.xyz + ( u_normA + -u_normB ) * 0.5 * u_resolution ) ).r;
    float d2 = texture3D( u_scalarDataSampler, textPos( gl_Vertex.xyz + ( u_normA +  u_normB ) * 0.5 * u_resolution ) ).r;
    float d3 = texture3D( u_scalarDataSampler, textPos( gl_Vertex.xyz + ( u_normA * -1.0 +  u_normB ) * 0.5 * u_resolution ) ).r;

    // check which edges of the quad were hit
    edge0Hit_f = float( d0 >= u_isovalue != d1 >= u_isovalue );
    edge1Hit_f = float( d1 >= u_isovalue != d2 >= u_isovalue );
    edge2Hit_f = float( d2 >= u_isovalue != d3 >= u_isovalue );
    edge3Hit_f = float( d3 >= u_isovalue != d0 >= u_isovalue );

    float overlap = 1.6;
    // float overlap = 0.0;
    v_quadScale = 1.000 + overlap;

    // determine the position where the corresponding edge was hitten (in 0,1 clamped relative coordinates)
    hit0Pos = vec3( clamp( ( d0 - u_isovalue ) / ( d0 - d1 ), 0.0, 1.0 ), 0.0, 0.0 );
    hit1Pos = vec3( 0.0, clamp( ( d1 - u_isovalue ) / ( d1 - d2 ), 0.0, 1.0 ), 0.0 );
    hit2Pos = vec3( clamp( ( d3 - u_isovalue ) / ( d3 - d2 ), 0.0, 1.0 ), 0.0, 0.0 );
    hit3Pos = vec3( 0.0, clamp( ( d0 - u_isovalue ) / ( d0 - d3 ), 0.0, 1.0 ), 0.0 );

    // incase of overlapping quads we need to rescale the hit positions
    vec3 a = vec3( -0.5, -0.5, 0.0 ) / v_quadScale + vec3( 0.5, 0.5, 0.0 );
    vec3 b = vec3(  0.5, -0.5, 0.0 ) / v_quadScale + vec3( 0.5, 0.5, 0.0 );
    vec3 c = vec3(  0.5,  0.5, 0.0 ) / v_quadScale + vec3( 0.5, 0.5, 0.0 );
    vec3 d = vec3( -0.5,  0.5, 0.0 ) / v_quadScale + vec3( 0.5, 0.5, 0.0 );
    hit0Pos = a + hit0Pos / v_quadScale;
    hit1Pos = b + hit1Pos / v_quadScale;
    hit2Pos = d + hit2Pos / v_quadScale;
    hit3Pos = a + hit3Pos / v_quadScale;

    sumHits = float( int( edge0Hit_f ) + int( edge1Hit_f ) * 2 + int( edge2Hit_f ) * 4 + int( edge3Hit_f ) * 8 );

    if( sumHits > 0.0 ) // only render quads when there is an isovalue nearby
    {
        gl_Position = gl_ModelViewProjectionMatrix * (  vec4( v_quadScale * gl_TexCoord[0].xyz + gl_Vertex.xyz, 1.0 ) );
    }
    else
    {
        gl_Position = ftransform();
    }
    v_col = vec4( rand( gl_Vertex.xy ), rand( gl_Vertex.yz ), rand( gl_Vertex.xz ), 1.0 );
}
