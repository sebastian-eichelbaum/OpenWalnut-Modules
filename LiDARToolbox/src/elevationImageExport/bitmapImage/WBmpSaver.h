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


#ifndef WBMPSAVER_H
#define WBMPSAVER_H

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "WBmpImage.h"

/**
 * Class that saves an image to a bmp file
 */
class WBmpSaver
{
public:
    /**
     * Bmp image saver constructor
     */
    WBmpSaver();
    /**
     * Bmp image saver destructor
     */
    virtual ~WBmpSaver();
    /**
     * Static method to save an image as a bmp file.
     * \param image The image to store.
     * \param path Target image file path.
     */
    static void saveImage( WBmpImage* image, const char* path );

private:
};

#endif  // WBMPSAVER_H
