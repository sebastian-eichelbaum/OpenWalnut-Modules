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

/**
 * Converts HSL colors to RGB colors. This is used in this shader later to draw from a single random, a whole color (hue).
 */
vec4 hsv_to_rgb(float h, float s, float v, float a)
{
    float c = v * s;
    h = mod((h * 6.0), 6.0);
    float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
    vec4 color;

    if (0.0 <= h && h < 1.0) {
        color = vec4(c, x, 0.0, a);
    } else if (1.0 <= h && h < 2.0) {
        color = vec4(x, c, 0.0, a);
    } else if (2.0 <= h && h < 3.0) {
        color = vec4(0.0, c, x, a);
    } else if (3.0 <= h && h < 4.0) {
        color = vec4(0.0, x, c, a);
    } else if (4.0 <= h && h < 5.0) {
        color = vec4(x, 0.0, c, a);
    } else if (5.0 <= h && h < 6.0) {
        color = vec4(c, 0.0, x, a);
    } else {
        color = vec4(0.0, 0.0, 0.0, a);
    }

    color.rgb += v - c;

    return color;
}

vec4 hotIron( in float value )
{
    // // BLUE <-> RED
    // vec4 color8  = vec4( 255.0 / 255.0, 255.0 / 255.0, 204.0 / 255.0, 1.0 );
    // vec4 color7  = vec4( 255.0 / 255.0, 237.0 / 255.0, 160.0 / 255.0, 1.0 );
    // vec4 color6  = vec4( 254.0 / 255.0, 217.0 / 255.0, 118.0 / 255.0, 1.0 );
    // vec4 color5  = vec4( 254.0 / 255.0, 178.0 / 255.0,  76.0 / 255.0, 1.0 );
    // vec4 color4  = vec4( 253.0 / 255.0, 141.0 / 255.0,  60.0 / 255.0, 1.0 );
    // vec4 color3  = vec4( 252.0 / 255.0,  78.0 / 255.0,  42.0 / 255.0, 1.0 );
    // vec4 color2  = vec4( 205.0 / 255.0, 000.0 / 255.0, 000.0 / 255.0, 1.0 );
    // vec4 color1  = vec4( 180.0 / 255.0,  85.0 / 255.0, 255.0 / 255.0, 1.0 );
    // vec4 color0  = vec4( 000.0 / 255.0, 150.0 / 255.0, 255.0 / 255.0, 1.0 );

    // HOT IRON
    vec4 color8  = vec4( 255.0 / 255.0, 255.0 / 255.0, 204.0 / 255.0, 1.0 );
    vec4 color7  = vec4( 255.0 / 255.0, 237.0 / 255.0, 160.0 / 255.0, 1.0 );
    vec4 color6  = vec4( 254.0 / 255.0, 217.0 / 255.0, 118.0 / 255.0, 1.0 );
    vec4 color5  = vec4( 254.0 / 255.0, 178.0 / 255.0,  76.0 / 255.0, 1.0 );
    vec4 color4  = vec4( 253.0 / 255.0, 141.0 / 255.0,  60.0 / 255.0, 1.0 );
    vec4 color3  = vec4( 252.0 / 255.0,  78.0 / 255.0,  42.0 / 255.0, 1.0 );
    vec4 color2  = vec4( 227.0 / 255.0,  26.0 / 255.0,  28.0 / 255.0, 1.0 );
    vec4 color1  = vec4( 189.0 / 255.0,   0.0 / 255.0,  38.0 / 255.0, 1.0 );
    vec4 color0  = vec4( 128.0 / 255.0,   0.0 / 255.0,  38.0 / 255.0, 1.0 );

    // In order to use a different color map use the colors in the BLUE <-> RED section above from color0 to colorN.
    // Depending on how much colors you need, set then the colorNum to e.g. 2 when using only two colors.
    //
    // // HOT IRON
    int colorNum = 8;
    // // BLUE <-> RED
    // int colorNum = 3;

    float colorValue = value * float( colorNum );
    int sel = int( floor( colorValue ) );

    if( sel >= colorNum )
    {
        return color0;
    }
    else if( sel < 0 )
    {
        return color0;
    }
    else
    {
        colorValue -= float( sel );

        if( sel < 1 )
        {
            return ( color1 * colorValue + color0 * ( 1.0 - colorValue ) );
        }
        else if( sel < 2 )
        {
            return ( color2 * colorValue + color1 * ( 1.0 - colorValue ) );
        }
        else if( sel < 3 )
        {
            return ( color3 * colorValue + color2 * ( 1.0 - colorValue ) );
        }
        else if( sel < 4 )
        {
            return ( color4 * colorValue + color3 * ( 1.0 - colorValue ) );
        }
        else if( sel < 5 )
        {
            return ( color5 * colorValue + color4 * ( 1.0 - colorValue ) );
        }
        else if( sel < 6 )
        {
            return ( color6 * colorValue + color5 * ( 1.0 - colorValue ) );
        }
        else if( sel < 7 )
        {
            return ( color7 * colorValue + color6 * ( 1.0 - colorValue ) );
        }
        else if( sel < 8 )
        {
            return ( color8 * colorValue + color7 * ( 1.0 - colorValue ) );
        }
        else
        {
            return color0;
        }
    }
}

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
vec3 customInterpolate( vec3 textPos, out vec3 interpolDiff )
{
  vec3 pos[8];
  pos[0] = vec3( floor( textPos.x ), floor( textPos.y ), floor( textPos.z ) );
  pos[1] = vec3(  ceil( textPos.x ), floor( textPos.y ), floor( textPos.z ) );
  pos[2] = vec3( floor( textPos.x ),  ceil( textPos.y ), floor( textPos.z ) );
  pos[3] = vec3(  ceil( textPos.x ),  ceil( textPos.y ), floor( textPos.z ) );
  pos[4] = vec3( floor( textPos.x ), floor( textPos.y ),  ceil( textPos.z ) );
  pos[5] = vec3(  ceil( textPos.x ), floor( textPos.y ),  ceil( textPos.z ) );
  pos[6] = vec3( floor( textPos.x ),  ceil( textPos.y ),  ceil( textPos.z ) );
  pos[7] = vec3(  ceil( textPos.x ),  ceil( textPos.y ),  ceil( textPos.z ) );

  vec3 localPos = textPos - pos[0];
  float lambdaX = localPos.x * u_pixelSizeX;
  float lambdaY = localPos.y * u_pixelSizeY;
  float lambdaZ = localPos.z * u_pixelSizeZ;

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
  float dist = 9999999;
  int nearest = 9999999;
  for( int i = 0; i < 8; ++i )
  {
    if( length( pos[i] - textPos ) < dist )
    {
      dist = length( pos[i] - textPos );
      nearest = i;
    }
    pos[i].x /= u_pixelSizeX * ( u_probTractSizeX -1 );
    pos[i].y /= u_pixelSizeY * ( u_probTractSizeY -1 );
    pos[i].z /= u_pixelSizeZ * ( u_probTractSizeZ -1 );
    v[i] = texture3DUnscaled( u_vectorsSampler, pos[i], u_vectorsMin, u_vectorsScale ).xyz;
    // for orientation swaps use the step function below instead of 1.0
    float sgn = 1.0; // step( 0, dot( v[0], v[i] ) )*2 - 1;
    result += h[i] * sgn * v[i];
  }

  result = v[nearest];
  textPos.x /= u_pixelSizeX * ( u_probTractSizeX);
  textPos.y /= u_pixelSizeY * ( u_probTractSizeY);
  textPos.z /= u_pixelSizeZ * ( u_probTractSizeZ);

  interpolDiff = vec3( 1, 1, 1 ) -  abs( result - texture3DUnscaled( u_vectorsSampler, textPos, u_vectorsMin, u_vectorsScale ).xyz );

  return normalize( result );
}

void main()
{
    // get principal diffusion direction
    vec3 interpolDiff;
    vec3 diffusionDirection = customInterpolate( v_textPos, interpolDiff );

    // project into plane (given by two vectors aVec and bVec)
    vec3 aVecNorm = normalize( u_aVec );
    vec3 bVecNorm = normalize( u_bVec );
    vec3 projectedDirectionTextCoords = 0.5 * vec3( dot( aVecNorm, diffusionDirection ), dot( bVecNorm, diffusionDirection ), 0.0 );

    vec3 scaledFocalPoint1 = middlePoint_tex + scale * projectedDirectionTextCoords;
    vec3 scaledFocalPoint2 = middlePoint_tex - scale * projectedDirectionTextCoords;
    vec3 focalPoint1 = middlePoint_tex + projectedDirectionTextCoords;
    vec3 focalPoint2 = middlePoint_tex - projectedDirectionTextCoords;

    vec3 texturePosition = v_textPos;
    texturePosition.x /= u_pixelSizeX * u_probTractSizeX;
    texturePosition.y /= u_pixelSizeY * u_probTractSizeY;
    texturePosition.z /= u_pixelSizeZ * u_probTractSizeZ;

    float col = texture3D( u_colSampler, texturePosition ).r;

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

    // // Debug: for fixed radius
    // radius = 0.01* u_glyphThickness;

    // // debug positions: This will draw only a circle if you want to see the positions. You need to disable all other stuff
    // vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    // //if( gl_TexCoord[2].x == 0 )
    // //{
    // //    color = vec4( 107.0 / 255.0, 138.0 / 255.0, 67.0 / 255.0, 1.0 );
    // //}
    // //else if( gl_TexCoord[2].x == 1 )
    // //{
    // //    color = vec4( 28.0 / 255.0, 74.0 / 255.0, 147.0 / 255.0, 1.0 );
    // //}
    // //else if( gl_TexCoord[2].x == 2 )
    // //{
    // //    color = vec4( 226.0 / 255.0, 180.0 / 255.0, 55.0 / 255.0, 1.0 );
    // //}
    // //else if( gl_TexCoord[2].x == 3 )
    // //{
    // //    color = vec4( 173.0 / 255.0, 72.0 / 255.0, 34.0 / 255.0, 1.0 );
    // //}
    // // else {
    // //   discard;
    // // }
    // float lp = distance( gl_TexCoord[1].xyz, middlePoint_tex );
    // if( lp < 0.5 )
    // {
    //     gl_FragColor = color;
    // }

    vec4 white = vec4( 1, 1, 1, 1 );
    vec4 black = vec4( 0, 0, 0, 1 );

    // opacity filtered color (default)
    vec4 c = u_color * pow( v_probability, 1.0 / (10.0 * u_colorThreshold) );

    if( u_colorThreshold >= 1.0 ) { // pure color
      c = u_color;
    }

    // mod: dark stipples in bright regions, bright stipples in dark regions
    // c.r = c.r - 0.1 + 0.1 * ( 1 - 2.0 * col );

    // // Debug: hotIron colormapping of the probability.
    // vec4 c = hotIron( 1.0 - v_probability );

    // Feature: enable this if you want fiber stipples color from separate texture (where 'col' variable is refferring to).
    // vec4 c = hotIron( col );
    // // with opacity or with out
    // c.a = probability;

    // // if you want thresholding
    // if( col < 0.51 )
    //     c = vec4(0,0,0.8, 1 );
    // else
    //     c = hotIron( col * 2 - 1.0 );
    // c.a = 0.2 + probability;

    // vec4 c = white;

    // vec4 c = black;

    // // Debug: random color
    // float r = rand( vec2( scaledFocalPoint1.xy ) );
    // vec4 c = hsv_to_rgb( r, 1, 1, 1 );

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
              float f = ( i / float(u_outlineSteps) );
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
//    else
//    {
//        if( u_outline > 0.0 )
//        {
//            // Debug: Enable fade to black
//            float gray = 1.0;
//            float alpha = v_probability + 0.5;
//            float u_outlineWidth = 0.019;
//            if( dist < ( radius + 0.0 + 8.0 * u_outlineWidth ) ) { float gg = gray - 0.9; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.7 ); }
//            if( dist < ( radius + 0.0 + 7.0 * u_outlineWidth ) ) { float gg = gray - 0.7; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.6 ); }
//            if( dist < ( radius + 0.0 + 6.0 * u_outlineWidth ) ) { float gg = gray - 0.5; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.5 ); }
//            if( dist < ( radius + 0.0 + 5.0 * u_outlineWidth ) ) { float gg = gray - 0.4; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.4 ); }
//            if( dist < ( radius + 0.0 + 4.0 * u_outlineWidth ) ) { float gg = gray - 0.3; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.3 ); }
//            if( dist < ( radius + 0.0 + 3.0 * u_outlineWidth ) ) { float gg = gray - 0.2; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.2 ); }
//            if( dist < ( radius + 0.0 + 2.0 * u_outlineWidth ) ) { float gg = gray - 0.1; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.1 ); }
//            if( dist < ( radius + 0.0 + 1.0 * u_outlineWidth ) ) { float gg = gray - 0.0; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.0 ); }
//
//            // // Debug: colored outline only
//            // c.a=1.0;
//            // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 1.0 * u_outlineWidth ) ) { float gg = gray - 0.0; gl_FragColor = vec4( c ) + vec4( 0,0,0, 0.0 ); }
//
//           // // Debug: Enable fade to black
//           // float gray = 1.0;
//           // float alpha = v_probability + 0.5;
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.040 ) ) { float gg = gray - 0.0; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.7 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.035 ) ) { float gg = gray - 0.1; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.6 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.030 ) ) { float gg = gray - 0.2; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.5 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.025 ) ) { float gg = gray - 0.3; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.4 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.020 ) ) { float gg = gray - 0.4; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.3 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.015 ) ) { float gg = gray - 0.5; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.2 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.010 ) ) { float gg = gray - 0.7; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.1 ); }
//           // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.005 ) ) { float gg = gray - 0.9; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.0 ); }
//
//           if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) > ( radius + 8 * u_outlineWidth ) ) { 
//               discard;
//           }
//        }
//        else {
//            // Debug: Enable fade to black
//            float gray = 1.0;
//            float alpha = v_probability + 0.5;
//            float u_outlineWidth = 0.009;
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 8.0 * u_outlineWidth ) ) { float gg = gray - 0.9; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 7.0 * u_outlineWidth ) ) { float gg = gray - 0.7; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 6.0 * u_outlineWidth ) ) { float gg = gray - 0.5; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 5.0 * u_outlineWidth ) ) { float gg = gray - 0.4; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 4.0 * u_outlineWidth ) ) { float gg = gray - 0.3; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 3.0 * u_outlineWidth ) ) { float gg = gray - 0.2; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 2.0 * u_outlineWidth ) ) { float gg = gray - 0.1; gl_FragColor = vec4( c );}
//            if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.0 + 1.0 * u_outlineWidth ) ) { float gg = gray - 0.0; gl_FragColor = vec4( c );}
//
//             if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) > ( radius + 8 * u_outlineWidth ) ) { 
//                 discard;
//             }
//        }
//
//        // // Draw quad and inner cricle
//        // gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
//        // float lp = distance( gl_TexCoord[1].xyz, middlePoint_tex );
//        // if( lp < 0.5 )
//        // {
//        //     gl_FragColor = vec4( 0.7, 0.7, 1.0, 1.0 ); // discard;
//        // }
//    }

    // // Draw Line and focal points
    // if( distancePointLineSegment( gl_TexCoord[1].xyz, focalPoint1, focalPoint2 ) < 0.01 )
    // {
    //     if( l <= 1.1 )
    //     {
    //         gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
    //     }
    //     else if( l < 1.225 )
    //     {
    //         gl_FragColor = vec4( 0.0, 1.0, 0.0, 1.0 );
    //     }
    //     else
    //     {
    //         gl_FragColor = vec4( 0.0, 0.0, 1.0, 1.0 );
    //     }
    // }
    // // display middle point
    // if( distance( gl_TexCoord[1].xyz, middlePoint_tex ) <=  0.02 )
    // {
    //     gl_FragColor = vec4( 0.0, 1.0, 0.0, 1.0 ); // green
    // }

    // // display new focal points
    // if( ( distance( gl_TexCoord[1].xyz, scaledFocalPoint1 ) < 0.01 ) )
    // {
    //     gl_FragColor = vec4( 0.0, 1.0, 1.0, 1.0 ); // cyan
    // }
    // if( ( distance( gl_TexCoord[1].xyz, scaledFocalPoint2 ) < 0.01 ) )
    // {
    //     gl_FragColor = vec4( 0.0, 0.0, 1.0, 1.0 ); // blue
    // }

    // // display evec end points
    // if( ( distance( gl_TexCoord[1].xyz, focalPoint1 ) < 0.01 ) )
    // {
    //     gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 ); // red
    // }

    // if( ( distance( gl_TexCoord[1].xyz, focalPoint2 ) < 0.01 ) )
    // {
    //     gl_FragColor = vec4( 1.0, 1.0, 0.0, 1.0 ); // yellow
    // }
}
