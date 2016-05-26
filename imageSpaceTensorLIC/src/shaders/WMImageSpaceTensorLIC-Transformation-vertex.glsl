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

#include "WGEColormapping-vertex.glsl"

#include "WGETransformationTools.glsl"

#include "WMImageSpaceTensorLIC-Transformation-varyings.glsl"

/**
 * How much should the slice be moved along u_vertexShiftDirection?
 */
uniform int u_vertexShift = 0;

/**
 * The direction along which the slice gets moved
 */
uniform vec3 u_vertexShiftDirection = vec3( 0.0, 0.0, 0.0 );

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
 * Size of input noise texture in pixels
 */
uniform int u_texture1SizeX = 255;

/**
 * Size of input noise texture in pixels
 */
uniform int u_texture1SizeY = 255;

/**
 * Size of input noise texture in pixels
 */
uniform int u_texture1SizeZ = 255;

/**
 * Vertex Main. Simply transforms the geometry. The work is done per fragment.
 */
void main()
{
    // Calculate the real vertex coordinate in openwalnut-scene-space
    vec4 vertex = gl_Vertex;
#ifdef VERTEXSHIFT_ENABLED
    // Calculate the real vertex coordinate in openwalnut-scene-space
    vertex = ( vec4( u_vertexShiftDirection.xyz, 0.0 ) * u_vertexShift ) + gl_Vertex;
#endif

    // Allow the colormapper to do some precalculations with the real vertex coordinate in ow-scene-space
    colormapping( vertex );

    // for easy access to texture coordinates
    // NOTE: The vertex is specified in ow-scene-space. The texture matrix was set by WGEDataTexture for the dataset and transforms the vertex in
    // ow-scene-space to the textures space.
    gl_TexCoord[0] = gl_TextureMatrix[0] * vertex;
    gl_TexCoord[1] = gl_TextureMatrix[1] * vertex;
    gl_TexCoord[2] = gl_TextureMatrix[2] * vertex;
    gl_TexCoord[3] = gl_TextureMatrix[3] * vertex;

    // some light precalculations
    v_normal = gl_NormalMatrix * gl_Normal;
    v_normalObject = gl_Normal;
    v_vertex = gl_Vertex.xyz;

    /*
    // NOTE: we normalize the vec here although we need to normalize it again in the fragment shader since the projected vector might be very
    // small and we want to avoid numerical issues when interpolating.
    v_normalProjected = normalize( projectVector( vec4( v_normal, 0.0 ) ) );

    // if we use 3d noise textures and auto resolution feature:
    vec4 vec1 = vec4( float( u_texture0SizeX ) / float( u_texture1SizeX ),
                      float( u_texture0SizeY ) / float( u_texture1SizeY ),
                      float( u_texture0SizeZ ) / float( u_texture1SizeZ ), 0.0 );
    v_noiseScaleFactor = vec1.xyz * u_noiseResoultuion;
#ifdef NOISE3DAUTORES_ENABLED
    vec4 vecMV = gl_ModelViewMatrix * vec4( vec3( 1.0 ), 0.0 ); // if the resolution should be modified, use the scaling info from the MV matrix
    v_noiseScaleFactor *= u_noiseResoultuion * length( vecMV.xyz );
#endif
*/

    // also get the coordinates of the light
    vec4 lpos = gl_LightSource[0].position; // this simply doesn't work well with OSG
    lpos = vec4( 0.0, 0.0, 1000.0, 1.0 );
    v_lightSource = worldToLocal( lpos ).xyz;

    // transform the view direction to texture space, which equals object space
    // Therefore use two points, as we transform a vector
    vec4 camPos    = vec4( 0.0, 0.0, -1.0, 0.0 );
    v_viewDir = worldToLocal( camPos ).xyz;

    // transform position
    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}

