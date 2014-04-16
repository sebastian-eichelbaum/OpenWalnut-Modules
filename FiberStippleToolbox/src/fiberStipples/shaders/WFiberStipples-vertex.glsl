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

#include "WGETextureTools.glsl"
#include "WFiberStipples-varyings.glsl"
#include "WFiberStipples-uniforms.glsl"
// #include "WGETransformationTools.glsl"

/**
 * Vertex Main. Simply transforms the geometry and computes the projected diffusion direction.
 */
void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0; // for distinguishing the verties of the quad
    gl_TexCoord[1] = gl_MultiTexCoord1; // for coordinate system within fragment shader (enable unit quad coordinates)
    gl_TexCoord[2] = gl_MultiTexCoord2; // for selecting the noisy vertex slice
    gl_TexCoord[3] = gl_MultiTexCoord3; // for selecting quads within a single hierarchy level

    // compute texture coordinates from worldspace coordinates for texture access
    vec3 texturePosition = ( u_WorldTransform * gl_Vertex ).xyz;
    v_textPos = texturePosition;
    texturePosition.x /= u_pixelSizeX * u_probTractSizeX;
    texturePosition.y /= u_pixelSizeY * u_probTractSizeY;
    texturePosition.z /= u_pixelSizeZ * u_probTractSizeZ;

    // get connectivity score from probTract (please not, it is already scaled between 0.0...1.0 from WDataTexture3D::createTexture
    v_probability = texture3D( u_probTractSampler, texturePosition ).r;

    // // span quad incase of regions with high probablility
    if( v_probability > u_threshold && ( u_minRange + v_probability ) * u_maxRange * u_numDensitySlices >= gl_TexCoord[2].x + gl_TexCoord[3].x )
    // // Debug: fewer hierarchy levels and you may turn on and off the smooth level transition with the last multiplication to gl_TexCoord[3].x. 0.0 == disable 1.0 == enable
    // if( v_probability > u_threshold && ( u_minRange + v_probability ) * u_maxRange * int( u_numDensitySlices / 10 )  > int( gl_TexCoord[2].x / 10 ) + 0.0* gl_TexCoord[3].x )
    // // Debug: pure rejection sampling
    // if( v_probability > u_threshold && ( u_minRange + v_probability ) * u_maxRange >= 1.0 * gl_TexCoord[3].x )
    // // Debug: bipartionate
    // if( v_probability < 0.5 && gl_TexCoord[2].x < 10  || v_probability > 0.5 )
    // // Debug: Draw seed points with repect to probability and their density-slice-number
    // if( v_probability > u_threshold &&  0 == gl_TexCoord[2].x )
    // // Debug: Draw all seed points. You need to disable else
    // if( true )
    {
         // transform position, the 4th component must be explicitly set, as otherwise they would have been scaled
         gl_Position = gl_ModelViewProjectionMatrix * ( vec4( u_glyphSize * gl_TexCoord[0].xyz + gl_Vertex.xyz, 1.0 ) );
    }
    else
    {
         gl_Position = ftransform(); // discard those vertices
    }
}
