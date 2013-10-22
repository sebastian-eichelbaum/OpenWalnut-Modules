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
// #include "WGEColormapping-vertex.glsl"
// #include "WGETransformationTools.glsl"

/**
 * These two uniforms are needed to transform the vectors out of their texture
 * back to their original form as they are stored in RBGA (for example allowing
 * only values between 0..1 for components but no negative ones).
 */
uniform float u_vectorsMin; uniform float u_vectorsScale;

/**
 * The matrix describes the transformation of gl_Vertex to OpenWalnut Scene Space
 */
uniform mat4 u_WorldTransform;

/**
 * Vector dataset as texture.
 */
uniform sampler3D u_vectorsSampler;

/**
 * Probabilistic tract as texture.
 */
uniform sampler3D u_probTractSampler;

uniform sampler3D u_colSampler;

/**
 * Number of voxels in X direction.
 */
uniform int u_probTractSizeX;

/**
 * Number of voxels in Y direction.
 */
uniform int u_probTractSizeY;

/**
 * Number of voxels in Z direction.
 */
uniform int u_probTractSizeZ;

// For correct transformation into texture space we also need the size of each voxel.
/**
 * Voxel size in X direction.
 */
uniform float u_pixelSizeX;

/**
 * Voxel size in Y direction.
 */
uniform float u_pixelSizeY;

/**
 * Voxel size in Z direction.
 */
uniform float u_pixelSizeZ;

/**
 * First plane vector spanning the plane of the quad.
 */
uniform vec3 u_aVec;

/**
 * Second plane vector spanning the plane of the quad.
 */
uniform vec3 u_bVec;

/**
 * Used for scaling the focal points, as otherwise the the stipple endings may not fit inside quad.
 */
uniform float scale = 0.8;

/**
 * Maximum connectivity score withing the probabilistic tract dataset. This is
 * needed for scaling the connectivities between 0.0 and 1.0.
 */
uniform float u_maxConnectivityScore;

/**
 * Probabilities below this threshold are ignored and discarded.
 */
uniform float u_threshold;

/**
 * Minimal density of the fiber stipples.
 */
uniform float u_minRange;

/**
 * Maximal density of the fiber stipples.
 */
uniform float u_maxRange;

/**
 * Middle point of the quad in texture coordinates, needed for scaling the
 * projection of the principal diffusion direction to fit inside quad.
 */
uniform vec3 middlePoint_tex = vec3( 0.5, 0.5, 0.0 );

/**
 * How much slices with random quads are used.
 */
uniform int u_numDensitySlices;

/**
 * Scales the quad which is used later for the stipples.
 */
uniform float u_glyphSize;

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
    texturePosition.x /= u_pixelSizeX * u_probTractSizeX;
    texturePosition.y /= u_pixelSizeY * u_probTractSizeY;
    texturePosition.z /= u_pixelSizeZ * u_probTractSizeZ;

    // get connectivity score from probTract (please not, it is already scaled between 0.0...1.0 from WDataTexture3D::createTexture
    probability = texture3D( u_probTractSampler, texturePosition ).r;
    col  = texture3D( u_colSampler, texturePosition ).r;

    // span quad incase of regions with high probablility
    if( probability > u_threshold && ( u_minRange + probability ) * u_maxRange * u_numDensitySlices >= gl_TexCoord[2].x + gl_TexCoord[3].x )
    // // Debug: fewer hierarchy levels and you may turn on and off the smooth level transition with the last multiplication to gl_TexCoord[3].x. 0.0 == disable 1.0 == enable
    // if( probability > u_threshold && ( u_minRange + probability ) * u_maxRange * int( u_numDensitySlices / 6 )  >= int( gl_TexCoord[2].x / 6 ) + 1.0* gl_TexCoord[3].x )
    // // Debug: pure rejection sampling
    // if( probability > u_threshold && ( u_minRange + probability ) * u_maxRange >= 1.0 * gl_TexCoord[3].x )
    // // Debug: bipartionate
    // if( probability < 0.4 && gl_TexCoord[2].x < 10  || probability > 0.4 )
    // // Debug: Draw seed points with repect to probability and their density-slice-number
    // if( probability > u_threshold &&  0 == gl_TexCoord[2].x )
    // // Debug: Draw all seed points. You need to disable else
    // if( 1 >= gl_TexCoord[2].x )
    {
         // transform position, the 4th component must be explicitly set, as otherwise they would have been scaled
         gl_Position = gl_ModelViewProjectionMatrix * ( vec4( u_glyphSize * gl_TexCoord[0].xyz + gl_Vertex.xyz, 1.0 ) );
    }
    else
    {
         gl_Position = ftransform(); // discard those vertices
    }
//    }
    // get principal diffusion direction
    vec3 diffusionDirection = normalize( texture3DUnscaled( u_vectorsSampler, texturePosition, u_vectorsMin, u_vectorsScale ).xyz );

    // project into plane (given by two vectors aVec and bVec)
    vec3 aVecNorm = normalize( u_aVec );
    vec3 bVecNorm = normalize( u_bVec );
    vec3 projectedDirectionTextCoords = 0.5 * vec3( dot( aVecNorm, diffusionDirection ), dot( bVecNorm, diffusionDirection ), 0.0 );

    scaledFocalPoint1 = middlePoint_tex + scale * projectedDirectionTextCoords;
    scaledFocalPoint2 = middlePoint_tex - scale * projectedDirectionTextCoords;
    focalPoint1 = middlePoint_tex + projectedDirectionTextCoords;
    focalPoint2 = middlePoint_tex - projectedDirectionTextCoords;
}
