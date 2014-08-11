//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

//Source code source: http://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "WBmpSaver.h"


WBmpSaver::WBmpSaver()
{
}

WBmpSaver::~WBmpSaver()
{
}

void WBmpSaver::saveImage( WBmpImage* image, const char* path )
{
    std::ofstream stream;
    stream.open( path );

    std::cout << "!!! this is Savefile !!!" << std::endl;

    // mimeType = "image/bmp";

    unsigned char file[14] = {
        'B', 'M', // magic
        0, 0, 0, 0, // size in bytes
        0, 0, // app data
        0, 0, // app data
        40+14, 0, 0, 0 // start of data offset
    };
    unsigned char info[40] = {
        40, 0, 0, 0, // info hd size
        0, 0, 0, 0, // width
        0, 0, 0, 0, // heigth
        1, 0, // number color planes
        24, 0, // bits per pixel
        0, 0, 0, 0, // compression is none
        0, 0, 0, 0, // image bits size
        0x13, 0x0B, 0, 0, // horz resoluition in pixel / m
        0x13, 0x0B, 0, 0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
        0, 0, 0, 0, // #colors in pallete
        0, 0, 0, 0, // #important colors
        };

    int w = image->getSizeX();
    int h = image->getSizeY();

    int padSize = w%4; //Fixed bug in original code. Earlier assign = (4-w%4)%4
    int sizeData = w*h*3 + h*padSize;
    int sizeAll = sizeData + sizeof( file ) + sizeof( info );

    file[2] = (unsigned char)( sizeAll );
    file[3] = (unsigned char)( sizeAll>> 8 );
    file[4] = (unsigned char)( sizeAll>>16 );
    file[5] = (unsigned char)( sizeAll>>24 );

    info[4] = (unsigned char)( w );
    info[5] = (unsigned char)( w>> 8);
    info[6] = (unsigned char)( w>>16);
    info[7] = (unsigned char)( w>>24);

    info[8] = (unsigned char)( h );
    info[9] = (unsigned char)( h>>8 );
    info[10] = (unsigned char)( h>>16 );
    info[11] = (unsigned char)( h>>24 );

    info[24] = (unsigned char)( sizeData );
    info[25] = (unsigned char)( sizeData>>8 );
    info[26] = (unsigned char)( sizeData>>16 );
    info[27] = (unsigned char)( sizeData>>24 );

    stream.write( reinterpret_cast<char*>( file ), sizeof( file ) );
    stream.write( reinterpret_cast<char*>( info ), sizeof( info ) );

    unsigned char pad[3] = {0, 0, 0};

    for( int y = 0; y < h; y++ )
    {
        for( int x = 0; x < w; x++ )
        {
            unsigned char pixel[3];
            pixel[0] = image->getB( x, y );
            pixel[1] = image->getG( x, y );
            pixel[2] = image->getR( x, y );

            stream.write( reinterpret_cast<char*>( pixel ), 3 );
        }
        stream.write( reinterpret_cast<char*>( pad ), padSize );
    }
    stream.close();
}
