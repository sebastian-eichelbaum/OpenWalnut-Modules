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
#include "WFiberStipples-varyings.glsl"

/**
 * Color of the fiber stipples. This will be further combined with tract probability.
 */
uniform vec4 u_color;

/**
 * Middle point of the quad in texture coordinates, needed for scaling the projection of the principal diffusion direction to fit inside quad.
 */
uniform vec3 middlePoint_tex = vec3( 0.5, 0.5, 0.0 );

/**
 * Scale the radius of the glyphs (aka stipples).
 */
uniform float u_glyphThickness;

/**
 * All probabilities below this probability are highlighted to the color of this probability.
 */
uniform float u_colorThreshold;

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
    vec4 color8  = vec4( 255.0 / 255.0, 255.0 / 255.0, 204.0 / 255.0, 1.0 );
    vec4 color7  = vec4( 255.0 / 255.0, 237.0 / 255.0, 160.0 / 255.0, 1.0 );
    vec4 color6  = vec4( 254.0 / 255.0, 217.0 / 255.0, 118.0 / 255.0, 1.0 );
    vec4 color5  = vec4( 254.0 / 255.0, 178.0 / 255.0,  76.0 / 255.0, 1.0 );
    vec4 color4  = vec4( 253.0 / 255.0, 141.0 / 255.0,  60.0 / 255.0, 1.0 );
    vec4 color3  = vec4( 252.0 / 255.0,  78.0 / 255.0,  42.0 / 255.0, 1.0 );
    vec4 color2  = vec4( 227.0 / 255.0,  26.0 / 255.0,  28.0 / 255.0, 1.0 );
    vec4 color1  = vec4( 189.0 / 255.0,   0.0 / 255.0,  38.0 / 255.0, 1.0 );
    vec4 color0  = vec4( 128.0 / 255.0,   0.0 / 255.0,  38.0 / 255.0, 1.0 );

    float colorValue = value * 8.0;
    int sel = int( floor( colorValue ) );

    if( sel >= 8 )
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

void main()
{
    // generally the area of a line stipple is a circle with radius R (each half for the endings of the line stipple) plus
    // a quad with height 2R and width length of the focalPoints v and w. hence we have this equation in R to solve:
    //
    //    R^2*PI + 2R*length(v, w) - A = 0
    //
    // where A is the area to fill.

    float area =  0.1; // this is arbitrarily set
    float l = distance( scaledFocalPoint1, scaledFocalPoint2 );
    float p2 = -l / 3.14159265;
    float q = area / 3.14159265;
    float r1 = p2 + sqrt( p2 * p2 + q );
    float r2 = p2 - sqrt( p2 * p2 + q );
    float radius = max( r1, r2 ) * u_glyphThickness;

    // // debug positions: This will draw only a circle if you want to see the positions. You need to disable all other stuff
    // vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    // // if( gl_TexCoord[2].x == 0 )
    // // {
    // //     color = vec4( 107.0 / 255.0, 138.0 / 255.0, 67.0 / 255.0, 1.0 );
    // // }
    // // else if( gl_TexCoord[2].x == 1 )
    // // {
    // //     color = vec4( 28.0 / 255.0, 74.0 / 255.0, 147.0 / 255.0, 1.0 );
    // // }
    // // else if( gl_TexCoord[2].x == 2 )
    // // {
    // //     color = vec4( 226.0 / 255.0, 180.0 / 255.0, 55.0 / 255.0, 1.0 );
    // // }
    // // else if( gl_TexCoord[2].x == 3 )
    // // {
    // //     color = vec4( 173.0 / 255.0, 72.0 / 255.0, 34.0 / 255.0, 1.0 );
    // // }
    // float lp = distance( gl_TexCoord[1].xyz, middlePoint_tex );
    // if( lp < 0.5 )
    // {
    //     gl_FragColor = color;
    // }

    vec4 white = vec4( 1, 1, 1, 1 );
    vec4 black = vec4( 0, 0, 0, 1 );

    // // opacity filtered color (default)
    vec4 c = u_color * pow( probability, 1.0 / (10.0 * u_colorThreshold) );

    // pure color
    // vec4 c = u_color;
    // mod: dark stipples in bright regions, bright stipples in dark regions
    // c.r = c.r - 0.1 + 0.1 * ( 1 - 2.0 * col );

    // // Debug: hotIron colormapping of the probability.
    // vec4 c = hotIron( 1.0 - probability );

    // // Feature: enable this if you want fiber stipples color from separate texture (where 'col' variable is refferring to).
    // vec4 c = hotIron( 1 - col );
    //
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

    if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < radius )
    {
        gl_FragColor = c;
    }
    else
    {
        vec4 bordercolor = white;

        // // Debug: Enable fade to black
        // float gray = 1.0;
        // float alpha = probability + 0.5;
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.040 ) ) { float gg = gray - 0.9; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.7 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.035 ) ) { float gg = gray - 0.7; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.6 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.030 ) ) { float gg = gray - 0.5; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.5 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.025 ) ) { float gg = gray - 0.4; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.4 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.020 ) ) { float gg = gray - 0.3; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.3 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.015 ) ) { float gg = gray - 0.2; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.2 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.010 ) ) { float gg = gray - 0.1; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.1 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.005 ) ) { float gg = gray - 0.0; gl_FragColor = vec4( c * gg ) + vec4( 0,0,0, 0.0 ); }

        // // Debug: Enable fade to black
        // float gray = 1.0;
        // float alpha = probability + 0.5;
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.040 ) ) { float gg = gray - 0.0; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.7 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.035 ) ) { float gg = gray - 0.1; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.6 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.030 ) ) { float gg = gray - 0.2; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.5 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.025 ) ) { float gg = gray - 0.3; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.4 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.020 ) ) { float gg = gray - 0.4; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.3 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.015 ) ) { float gg = gray - 0.5; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.2 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.010 ) ) { float gg = gray - 0.7; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.1 ); }
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.005 ) ) { float gg = gray - 0.9; gl_FragColor = vec4( white * gg ) + vec4( 0,0,0, 0.0 ); }

        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) > ( radius + 0.040 ) ) { 
        discard;
        // }
        // // Draw quad and inner cricle
        // gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
        // float lp = distance( gl_TexCoord[1].xyz, middlePoint_tex );
        // if( lp < 0.5 )
        // {
        //     gl_FragColor = vec4( 0.7, 0.7, 1.0, 1.0 ); // discard;
        // }
    }

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
    // if( distance( gl_TexCoord[1].xyz, middlePoint_tex ) <=  0.01 )
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
