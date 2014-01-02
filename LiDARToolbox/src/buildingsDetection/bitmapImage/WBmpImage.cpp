//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "WBmpImage.h"


WBmpImage::WBmpImage( size_t sizeX, size_t sizeY )
{
    resizeImage( sizeX, sizeY );
}

WBmpImage::~WBmpImage()
{
    resizeImage( 0, 0 );
}
size_t WBmpImage::getSizeX()
{
    return m_sizeX;
}
size_t WBmpImage::getSizeY()
{
    return m_sizeY;
}
void WBmpImage::resizeImage( size_t sizeX, size_t sizeY )
{
    m_sizeX = sizeX;
    m_sizeY = sizeY;
    size_t size = sizeX * sizeY;
    m_dataR.reserve( size );
    m_dataG.reserve( size );
    m_dataB.reserve( size );
    m_dataR.resize( size );
    m_dataG.resize( size );
    m_dataB.resize( size );
    for( size_t index = 0; index < size; index++ )
    {
        m_dataB[index] = 0;
        m_dataG[index] = 0;
        m_dataB[index] = 0;
    }
}
size_t WBmpImage::getR( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return m_dataR[getIndex( x, y )];
}
size_t WBmpImage::getG( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return m_dataG[getIndex( x, y )];
}
size_t WBmpImage::getB( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return m_dataB[getIndex( x, y )];
}
size_t WBmpImage::getA( size_t x, size_t y )
{
    if( x >= m_sizeX || y >= m_sizeY ) return 0;
    return 255;
}
void WBmpImage::setPixel( size_t x, size_t y, size_t intensity )
{
    setPixel( x, y, intensity, intensity, intensity );
}
void WBmpImage::setPixel( size_t x, size_t y, size_t r, size_t g, size_t b )
{
    if( x >= m_sizeX || y >= m_sizeY ) return;
    size_t index = getIndex( x, y );
    m_dataR[index] = r < 256 ?r :255;
    m_dataG[index] = g < 256 ?g :255;
    m_dataB[index] = b < 256 ?b :255;
}
size_t WBmpImage::getIndex( size_t x, size_t y )
{
    return x + m_sizeX * y;
}
