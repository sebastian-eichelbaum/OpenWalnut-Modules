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
#include "WGEShadingTools.glsl"
#include "WGETextureTools.glsl"

/**
 * The texture Unit for the advection texture
 */
uniform sampler2D u_texture0Sampler;

/**
 * The texture Unit for the edge, depht texture
 */
uniform sampler2D u_texture1Sampler;

/**
 * The texture Unit for the colormapping
 */
uniform sampler2D u_texture2Sampler;

/**
 * The texture Unit for the light, depht texture
 */
uniform sampler2D u_texture3Sampler;

/**
 * The texture Unit for the light, depht texture
 */
uniform sampler2D u_texture4Sampler;

/**
 * The texture Unit for the light, depht texture
 */
uniform sampler2D u_texture5Sampler;

uniform int u_texture5SizeX;
uniform int u_texture5SizeY;

/**
 * Used to en-/disable lighting.
 */
uniform bool u_useLight;

/**
 * Define the light intensity here.
 */
uniform float u_lightIntensity;

/**
 * Used to en-/disable blending of edges.
 */
uniform bool u_useEdges;

/**
 * Edge color to use
 */
uniform vec4 u_useEdgesColor;

/**
 * Step function border for blending in the edges.
 */
uniform float u_useEdgesStep;

/**
 * If true, the depth is blended in
 */
uniform bool u_useDepthCueing;

/**
 * Ratio between colormap and advected noise
 */
uniform float u_cmapRatio;

/**
 * How intensive should contrast enhancement be?
 */
uniform bool u_useHighContrast;

varying vec3 v_normalizedVertex;

/**
 * Main. Clips and Blends the final image space rendering with the previously acquired 3D information
 */
void main()
{
    vec2 texCoord = gl_TexCoord[0].st;
    float edge  = texture2D( u_texture1Sampler, texCoord ).r * ( u_useEdges ? 1.0 : 0.0 );
    float noise  = texture2D( u_texture1Sampler, texCoord ).b;
    float light  = texture2D( u_texture3Sampler, texCoord ).r * ( u_useLight ? 1.0 : 0.0 );
    float depth  = texture2D( u_texture3Sampler, texCoord ).b;
    float occupied  = texture2D( u_texture3Sampler, texCoord ).a;
    float advected1  = texture2D( u_texture0Sampler, texCoord ).r;
    float advected2  = texture2D( u_texture0Sampler, texCoord ).g;
    vec3 cmap = texture2D( u_texture2Sampler, texCoord ).rgb;
    float evDiff  = texture2D( u_texture4Sampler, texCoord ).r;
    float fa  = texture2D( u_texture4Sampler, texCoord ).g;
    vec3 mesh = texture2D( u_texture5Sampler, texCoord ).rgb;


    vec2 grad = getGradient( u_texture5Sampler, v_normalizedVertex.xy, 1.0 / u_texture5SizeX * 2 );
    vec3 normal = gl_NormalMatrix * vec3( grad, -1.0 );
    float phongBumplight = blinnPhongIlluminationIntensity( normalize( viewAlign( normal ) ) );
    vec3 outColorBump = vec3( phongBumplight * mesh );
    vec3 outColorRibbon = ( mesh.r + mesh.g ) * vec3( mesh.r * phongBumplight, mesh.g * phongBumplight, 0.0 );

    // trimmings for beautifying image
    //gl_FragColor *= 2.0*light;

    vec3 outColor = outColorRibbon;
    float alpha = 1.0;
    //outColor.r *= 0.05;

    gl_FragColor = vec4( outColor, alpha * occupied );
}

