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

varying vec4 myColor;

/**
 * simple fragment shader that uses a texture on fibers
 */
void onlyNearFibers()
{
    float distance = gl_TexCoord[0].x;
    // float maxDist = gl_TexCoord[0].y;
    // float normalizedDistance = distance / maxDist;
    // normalizedDistance = clamp ( normalizedDistance, 0.0, 1.0 );
    vec4 color = myColor;
    color.a = .1;
    if( distance > 1 )
        color.a = 0;
    gl_FragColor = color;
}


void main()
{
    vec4 color = myColor;
    float distance = gl_TexCoord[0].x;
    float maxDist = gl_TexCoord[0].y;
    distance = clamp( distance, 0, maxDist );
    float alpha = 1 - ( distance / maxDist );
    alpha *= alpha;
    alpha *= alpha;
    alpha *= alpha;
    alpha *= alpha;
    alpha *= alpha;
    float darkness = clamp( alpha *.3, 0.0, 1.0 );
    // color = vec4( darkness, clamp( .5 - darkness, 0.0, 1.0), 1.0, .01 + alpha );
    color.a = .01 + alpha;

    gl_FragColor = color;

    // onlyNearFibers();
}
