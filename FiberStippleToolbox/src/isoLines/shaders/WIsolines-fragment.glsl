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
#include "WGEUtils.glsl" // for distancePointLineSegment

/**
 * Line width.
 */
uniform float u_lineWidth;

/**
 * Line Color.
 */
uniform vec4 u_color = vec4( 1.0, 0.0, 0.0, 1.0 );

float distancePointLine( vec3 point, vec3 start, vec3 end )
{
    float segmentLength = length( start - end );
    if( segmentLength == 0.0 )
    {
        return distance( point, start );
    }

    vec3 n = normalize( end - start );
    vec3 res = ( start - point ) - dot( ( start - point ), n ) * n;
    return length( res );
    // // Consider the line extending the segment, parameterized as start + t (end - start):
    // // We find projection of point p onto the line: t = [(p-v) . (w-v)] / |w-v|^2
    // float t = dot( point - start, end - start ) / ( segmentLength * segmentLength );

    // if( t < 0.0 )      // Beyond the 'v' end of the segment
    // {
    //     return distance( point, start );
    // }
    // else if( t > 1.0 ) // Beyond the 'w' end of the segment
    // {
    //     return distance( point, end );
    // }
    // else
    // {
    //     vec3 projection = start + t * ( end - start );  // Projection falls on the segment
    //     return distance( point, projection );
    // }
}
/**
 * Draws a line from point p1 to point p2 with the given width and color.
 *
 * \param p1 Start point
 * \param p2 End point
 * \param width Line width
 * \param color Line color
 */
void drawLine( vec3 p1, vec3 p2, float width, vec4 color )
{
    // // debug: enable to make quad background color red
    // gl_FragColor = vec4( 1.0, 0.6, 0.6, 0.2 );

    // draw actually the line
    if( distancePointLineSegment( gl_TexCoord[1].xyz, p1, p2 ) < width )
    {
        gl_FragColor = u_color;
    }
    else {
      discard;
    }

    // // debug: enable to draw hit (where the isovalue hits the edge, exactly) point in blue
    // float radius = 0.06;
    // if( ( length( gl_TexCoord[1].xyz - p1 ) < radius ) || ( length( gl_TexCoord[1].xyz - p2 ) < radius ) )
    // {
    //     gl_FragColor = vec4( 0.0, 0.0, 1.0, 1.0 );
    // }
}

void main()
{
    bool edge0Hit = bool( edge0Hit_f ); // OpenGL does not allow bool varyings
    bool edge1Hit = bool( edge1Hit_f ); // OpenGL does not allow bool varyings
    bool edge2Hit = bool( edge2Hit_f ); // OpenGL does not allow bool varyings
    bool edge3Hit = bool( edge3Hit_f ); // OpenGL does not allow bool varyings

    if( sumHits < 15.0 )
    {
        if( edge0Hit && edge1Hit )
        {
            drawLine( hit0Pos, hit1Pos, u_lineWidth, u_color );
        }
        else if( edge1Hit && edge2Hit )
        {
            drawLine( hit1Pos, hit2Pos, u_lineWidth, u_color );
        }
        else if( edge2Hit && edge3Hit )
        {
            drawLine( hit2Pos, hit3Pos, u_lineWidth, u_color );
        }
        else if( edge3Hit && edge0Hit )
        {
            drawLine( hit3Pos, hit0Pos, u_lineWidth, u_color );
        }
        else if( edge3Hit && edge1Hit )
        {
            drawLine( hit3Pos, hit1Pos, u_lineWidth, u_color );
        }
        else if( edge0Hit && edge2Hit )
        {
            drawLine( hit0Pos, hit2Pos, u_lineWidth, u_color );
        }
    }
    else // if( sumHits == 15 ) // Yes, this is topological INCORRECT but fast as hell and good looking too
    {
        drawLine( hit0Pos, hit1Pos, u_lineWidth, u_color );
        drawLine( hit2Pos, hit3Pos, u_lineWidth, u_color );
    }

    // // Debug: enable this to see border in red
    // vec2 pos = v_quadScale * gl_TexCoord[1].xy;
    // float margin = 0.03;
    // float outline = 0.015;
    // if( pos.x <= outline || pos.y <= outline || pos.x >= v_quadScale - outline || pos.y >= v_quadScale - outline ) {
    //   discard;
    // }
    // margin = margin + outline;
    // if( pos.x <= margin || pos.y <= margin  || pos.x >= v_quadScale - margin || pos.y >= v_quadScale - margin )
    // {
    //     // gl_FragColor = v_col;
    //     gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
    // }
}
