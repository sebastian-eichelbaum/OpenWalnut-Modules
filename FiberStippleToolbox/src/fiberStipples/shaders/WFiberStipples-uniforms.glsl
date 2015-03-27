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

/**
 * These two uniforms are needed to transform the vectors out of their texture
 * back to their original form as they are stored in RBGA (for example allowing
 * only values between 0..1 for components but no negative ones).
 */
// uniform float u_vectorsMin;
// uniform float u_vectorsScale;

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
 * Used for scaling the focal points, as otherwise the the stipple endings may not fit inside quad.
 */
uniform float u_scale;

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
 * How much slices with random quads are used.
 */
uniform int u_numDensitySlices;

/**
 * Scales the quad which is used later for the stipples.
 */
uniform float u_glyphSize;

/**
 * Color of the fiber stipples. This will be further combined with tract probability.
 */
uniform vec4 u_color;

/**
 * Scale the radius of the glyphs (aka stipples).
 */
uniform float u_glyphThickness;
uniform float u_outlineWidth;
uniform int u_outlineSteps;
uniform bool u_outlineDark;

/**
 * All probabilities below this probability are highlighted to the color of this probability.
 */
uniform float u_colorThreshold;

uniform vec3 u_aVec;
uniform vec3 u_bVec;

uniform bool u_outline;
uniform bool u_outlineInOut;
