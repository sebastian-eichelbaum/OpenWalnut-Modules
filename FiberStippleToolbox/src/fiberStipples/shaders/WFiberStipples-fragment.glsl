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

    if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < radius )
    {
        gl_FragColor = u_color * pow( probability, 1.0 / (10.0 * u_colorThreshold) );
//        gl_FragColor = clamp( u_color * clamp( u_colorThreshold + probability, 0.0, 1.0 ) - vec4( 0.0, 0.0, 0.0, 0.2 ), 0.0, 1.0 );
    }
    else
    {
        // if( distancePointLineSegment( gl_TexCoord[1].xyz, scaledFocalPoint1, scaledFocalPoint2 ) < ( radius + 0.01 ) )
        // {
        //     gl_FragColor = vec4( 1.0, 1.0, 1.0, gl_Color.w );
        // }

        discard;

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
