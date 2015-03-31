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

#include "WGEUtils.glsl" // for distancePointLineSegment(...)
#include "WGETextureTools.glsl"
#include "WFiberStipples-varyings.glsl"
#include "WFiberStipples-uniforms.glsl"

/**
 * Pseudo random number generator: http://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
 */
float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// /**
//  * Converts HSL colors to RGB colors. This is used in this shader later to draw from a single random, a whole color (hue).
//  */
// vec4 hsv_to_rgb(float h, float s, float v, float a)
// {
//     float c = v * s;
//     h = mod((h * 6.0), 6.0);
//     float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
//     vec4 color;
//
//     if (0.0 <= h && h < 1.0) {
//         color = vec4(c, x, 0.0, a);
//     } else if (1.0 <= h && h < 2.0) {
//         color = vec4(x, c, 0.0, a);
//     } else if (2.0 <= h && h < 3.0) {
//         color = vec4(0.0, c, x, a);
//     } else if (3.0 <= h && h < 4.0) {
//         color = vec4(0.0, x, c, a);
//     } else if (4.0 <= h && h < 5.0) {
//         color = vec4(x, 0.0, c, a);
//     } else if (5.0 <= h && h < 6.0) {
//         color = vec4(c, 0.0, x, a);
//     } else {
//         color = vec4(0.0, 0.0, 0.0, a);
//     }
//
//     color.rgb += v - c;
//
//     return color;
// }
//
// vec4 hotIron( in float value )
// {
//     // // BLUE <-> RED
//     // vec4 color8  = vec4( 255.0 / 255.0, 255.0 / 255.0, 204.0 / 255.0, 1.0 );
//     // vec4 color7  = vec4( 255.0 / 255.0, 237.0 / 255.0, 160.0 / 255.0, 1.0 );
//     // vec4 color6  = vec4( 254.0 / 255.0, 217.0 / 255.0, 118.0 / 255.0, 1.0 );
//     // vec4 color5  = vec4( 254.0 / 255.0, 178.0 / 255.0,  76.0 / 255.0, 1.0 );
//     // vec4 color4  = vec4( 253.0 / 255.0, 141.0 / 255.0,  60.0 / 255.0, 1.0 );
//     // vec4 color3  = vec4( 252.0 / 255.0,  78.0 / 255.0,  42.0 / 255.0, 1.0 );
//     // vec4 color2  = vec4( 205.0 / 255.0, 000.0 / 255.0, 000.0 / 255.0, 1.0 );
//     // vec4 color1  = vec4( 180.0 / 255.0,  85.0 / 255.0, 255.0 / 255.0, 1.0 );
//     // vec4 color0  = vec4( 000.0 / 255.0, 150.0 / 255.0, 255.0 / 255.0, 1.0 );
//
//     // HOT IRON
//     vec4 color8  = vec4( 255.0 / 255.0, 255.0 / 255.0, 204.0 / 255.0, 1.0 );
//     vec4 color7  = vec4( 255.0 / 255.0, 237.0 / 255.0, 160.0 / 255.0, 1.0 );
//     vec4 color6  = vec4( 254.0 / 255.0, 217.0 / 255.0, 118.0 / 255.0, 1.0 );
//     vec4 color5  = vec4( 254.0 / 255.0, 178.0 / 255.0,  76.0 / 255.0, 1.0 );
//     vec4 color4  = vec4( 253.0 / 255.0, 141.0 / 255.0,  60.0 / 255.0, 1.0 );
//     vec4 color3  = vec4( 252.0 / 255.0,  78.0 / 255.0,  42.0 / 255.0, 1.0 );
//     vec4 color2  = vec4( 227.0 / 255.0,  26.0 / 255.0,  28.0 / 255.0, 1.0 );
//     vec4 color1  = vec4( 189.0 / 255.0,   0.0 / 255.0,  38.0 / 255.0, 1.0 );
//     vec4 color0  = vec4( 128.0 / 255.0,   0.0 / 255.0,  38.0 / 255.0, 1.0 );
//
//     // In order to use a different color map use the colors in the BLUE <-> RED section above from color0 to colorN.
//     // Depending on how much colors you need, set then the colorNum to e.g. 2 when using only two colors.
//     //
//     // // HOT IRON
//     int colorNum = 8;
//     // // BLUE <-> RED
//     // int colorNum = 3;
//
//     float colorValue = value * float( colorNum );
//     int sel = int( floor( colorValue ) );
//
//     if( sel >= colorNum )
//     {
//         return color0;
//     }
//     else if( sel < 0 )
//     {
//         return color0;
//     }
//     else
//     {
//         colorValue -= float( sel );
//
//         if( sel < 1 )
//         {
//             return ( color1 * colorValue + color0 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 2 )
//         {
//             return ( color2 * colorValue + color1 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 3 )
//         {
//             return ( color3 * colorValue + color2 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 4 )
//         {
//             return ( color4 * colorValue + color3 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 5 )
//         {
//             return ( color5 * colorValue + color4 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 6 )
//         {
//             return ( color6 * colorValue + color5 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 7 )
//         {
//             return ( color7 * colorValue + color6 * ( 1.0 - colorValue ) );
//         }
//         else if( sel < 8 )
//         {
//             return ( color8 * colorValue + color7 * ( 1.0 - colorValue ) );
//         }
//         else
//         {
//             return color0;
//         }
//     }
// }
//

/**
 * Computes voxel corner positions in pos[8] and retrieve vectors from u_vectorsSampler. This way we access all 8 surrunding vectors and may perform
 * custom interpolation with directional swapping. However, this method does not work as expected:
 *
 * Currently, the nearest neightbour (NN) interpolation of the GPU produces different results as our NN interpolation. Reasons are unknown and might
 * be wrong pos[8], wrong vectors (v[8]), or small offsets as linear GPU interpolation has a texture coordinate shift compared to NN GPU interpolation.
 *
 * \param textPos position for interpolation result
 * \param interpolDiff difference betwee our custom interpolation and GPU interpolation.
 *
 * \note You need to comment out the line: "result = v[nearest];" in order to enable trilinear custom interpol., and uncomment to enable custom NN interpol.
 *
 * @return interpolated vector at position textPos
 */
vec3 customInterpolate( vec3 texPos, out vec3 interpolDiff )
{
    texPos.x *= u_vectorsSizeX;
    texPos.y *= u_vectorsSizeY;
    texPos.z *= u_vectorsSizeZ;

    vec3 pos[8];
    pos[0] = vec3( floor( texPos.x ), floor( texPos.y ), floor( texPos.z ) );
    pos[1] = vec3(  ceil( texPos.x ), floor( texPos.y ), floor( texPos.z ) );
    pos[2] = vec3( floor( texPos.x ),  ceil( texPos.y ), floor( texPos.z ) );
    pos[3] = vec3(  ceil( texPos.x ),  ceil( texPos.y ), floor( texPos.z ) );
    pos[4] = vec3( floor( texPos.x ), floor( texPos.y ),  ceil( texPos.z ) );
    pos[5] = vec3(  ceil( texPos.x ), floor( texPos.y ),  ceil( texPos.z ) );
    pos[6] = vec3( floor( texPos.x ),  ceil( texPos.y ),  ceil( texPos.z ) );
    pos[7] = vec3(  ceil( texPos.x ),  ceil( texPos.y ),  ceil( texPos.z ) );

    vec3 localPos = texPos - pos[0];
    float lambdaX = localPos.x;
    float lambdaY = localPos.y;
    float lambdaZ = localPos.z;

    float h[8];
    h[0] = ( 1 - lambdaX ) * ( 1 - lambdaY ) * ( 1 - lambdaZ );
    h[1] = (     lambdaX ) * ( 1 - lambdaY ) * ( 1 - lambdaZ );
    h[2] = ( 1 - lambdaX ) * (     lambdaY ) * ( 1 - lambdaZ );
    h[3] = (     lambdaX ) * (     lambdaY ) * ( 1 - lambdaZ );
    h[4] = ( 1 - lambdaX ) * ( 1 - lambdaY ) * (     lambdaZ );
    h[5] = (     lambdaX ) * ( 1 - lambdaY ) * (     lambdaZ );
    h[6] = ( 1 - lambdaX ) * (     lambdaY ) * (     lambdaZ );
    h[7] = (     lambdaX ) * (     lambdaY ) * (     lambdaZ );

    vec3 result = vec3( 0, 0, 0 );
    vec3 v[8];
    // float dist = 9999999;
    // int nearest = 9999999;
    for( int i = 0; i < 8; ++i )
    {
      // if( length( pos[i] - textPos ) < dist )
      // {
      //   dist = length( pos[i] - textPos );
      //   nearest = i;
      // }
      pos[i].x /= u_vectorsSizeX;
      pos[i].y /= u_vectorsSizeY;
      pos[i].z /= u_vectorsSizeZ;
      v[i] = texture3DUnscaled( u_vectorsSampler, pos[i], u_vectorsMin, u_vectorsScale ).xyz;
      // for orientation swaps use the step function below instead of 1.0
      float sgn = step( 0, dot( v[0], v[i] ) ) * 2 - 1;
      result += h[i] * sgn * v[i];
    }

  // result = v[nearest];
  texPos.x /= u_vectorsSizeX;
  texPos.y /= u_vectorsSizeY;
  texPos.z /= u_vectorsSizeZ;

  interpolDiff = vec3( 1, 1, 1 ) -  abs( result - texture3DUnscaled( u_vectorsSampler, texPos, u_vectorsMin, u_vectorsScale ).xyz );

  return normalize( result );
}

void main()
{
    vec3 interpolDiff;
    vec3 diffusionDirection = customInterpolate( v_vecTexturePos, interpolDiff );

    // project into plane (given by two vectors aVec and bVec)
    vec3 aVecNorm = normalize( u_aVec );
    vec3 bVecNorm = normalize( u_bVec );
    vec3 projectedDirectionTextCoords = 0.5 * vec3( dot( aVecNorm, diffusionDirection ), dot( bVecNorm, diffusionDirection ), 0.0 );

    /**
     * Middle point of the quad in texture coordinates, needed for scaling the
     * projection of the principal diffusion direction to fit inside quad.
     */
    vec3 middlePoint_tex = vec3( 0.5, 0.5, 0.0 );

    vec3 scaledFocalPoint1 = middlePoint_tex + u_scale * projectedDirectionTextCoords;
    vec3 scaledFocalPoint2 = middlePoint_tex - u_scale * projectedDirectionTextCoords;

    float col = texture3D( u_colSampler, v_colorTexturePos ).r;

    // generally the area of a line stipple is a circle with radius R (each half for the endings of the line stipple) plus
    // a quad with height 2R and width length of the focalPoints v and w. hence we have this equation in R to solve:
    //
    //    R^2*PI + 2R*length(v, w) - A = 0
    //
    // where A is the area to fill.

    float area =  0.1 * u_glyphThickness; // this is arbitrarily set
    float l = distance( scaledFocalPoint1, scaledFocalPoint2 );
    float p2 = -l / 3.14159265;
    float q = area / 3.14159265;
    float r1 = p2 + sqrt( p2 * p2 + q );
    float r2 = p2 - sqrt( p2 * p2 + q );
    float radius = max( r1, r2 );// - 0.05;

    vec4 white = vec4( 1, 1, 1, 1 );
    vec4 black = vec4( 0, 0, 0, 1 );

    vec4 color = u_color;

    // opacity filtered color (default)
    vec4 c = color * pow( v_probability, 1.0 / (10.0 * u_colorThreshold) );

    if( u_colorThreshold >= 1.0 ) { // pure color
      c = color;
    }

    float dist = distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 );

    if( u_outlineInOut ) {
      if( dist < ( radius + u_outlineWidth ) ) {
        // // Debug: Show the direction of the stipple by using the first focal point
        // c = vec4( normalize( gl_TexCoord[1].xyz - scaledFocalPoint1 ).xyz, 1.0 );
        // // Debug: Shows the dfference between two interpolation results as color in RGB
        // gl_FragColor = vec4( interpolDiff, 1.0 );
        // // Debug: colored outline only
        // c.a=0;

        if( dist < radius ) {
          gl_FragColor = c;
        }
        else {
          if( u_outline ) {
            dist = dist - radius;
            for( int i = u_outlineSteps; i > 0; --i ) {
              float f = ( i / float( u_outlineSteps) );
              if( u_outlineDark ) {
                if( dist < f * u_outlineWidth ) {
                  gl_FragColor =vec4( (c * (1-f) ).rgb, c.a );
                }
              }
              else {
                if( dist <  f * u_outlineWidth ) {
                  gl_FragColor = vec4( ( c + vec4(f,f,f,0) ).rgb, c.a );
                }
              }
            }
          }
          else {
            discard;
          }
        }
      }
      else {
        discard;
      }

    }
    else {
      if( dist < radius ) {
        // // Debug: Show the direction of the stipple by using the first focal point
        // c = vec4( normalize( gl_TexCoord[1].xyz - scaledFocalPoint1 ).xyz, 1.0 );
        // // Debug: Shows the dfference between two interpolation results as color in RGB
        // gl_FragColor = vec4( interpolDiff, 1.0 );
        // // Debug: colored outline only
        // c.a=0;

        if( dist < ( radius - u_outlineWidth ) ) {
          gl_FragColor = c;
        }
        else {
          if( u_outline ) {
            for( int i = 0; i < u_outlineSteps; ++i ) {
              float f = ( i / float(u_outlineSteps) );
              if( u_outlineDark ) {
                if( dist < ( radius - f * u_outlineWidth ) ) {
                  gl_FragColor =vec4( (c * f ).rgb, c.a );
                }
              }
              else {
                if( dist < ( radius - f * u_outlineWidth ) ) {
                  gl_FragColor = vec4( c + vec4(1-f,1-f,1-f,0) );
                }
              }
            }
          }
          else {
            gl_FragColor = c;
          }
        }
      }
      else {
        discard;
      }
    }
}

