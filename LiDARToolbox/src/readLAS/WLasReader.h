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

#ifndef WLASREADER_H
#define WLASREADER_H

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include <string>
#include <vector>

#include "core/kernel/WModule.h"
#include "core/kernel/WKernel.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "core/common/datastructures/WColoredVertices.h"

using osg::Vec3;

namespace laslibb
{
    /** 
     * Compound that reads a LiDAR file (see www.liblas.org).
     * These *.las files consist of a point set of X/Y/Z coordinates 
     * paired with the intensity/color.
     */
    class WLasReader
    {
    public:
        /**
         * Instance to get the reader not using a module.
         */
        WLasReader();

        /**
         * Instance to get the reader using a module.
         * \param progress Associated progress status bar.
         */
        explicit WLasReader( boost::shared_ptr< WProgressCombiner > progress );

        /**
         * Reader object destructor.
         */
        virtual ~WLasReader();

        /**
         * Sets the LAS input file path.
         * \param path Path of the read LAS file.
         */
        void setInputFilePath( const char* path );

        /**
         * Returns the read LiDAR data set points.
         * \param fromX The minimal read X coordinate.
         * \param fromY The minimal read Y coordinate.
         * \param dataSetWidth The width of the read daat set.
         *         No cropping is applied using the value 0.
         * \param moveToCenter Move the data to the center of the coordinate system.
         * \return LiDAR data set points containing intensities/colors.
         */
        boost::shared_ptr< WDataSetPoints > getPoints(
                size_t fromX, size_t fromY, size_t dataSetWidth, bool moveToCenter );

        /**
         * Returns the minimal X coordinate.
         * \return The minimal X coordinate.
         */
        float getXMin();
        /**
         * Returns the maximal X coordinate.
         * \return The maximal X coordinate.
         */
        float getXMax();
        /**
         * Returns the minimal Y coordinate.
         * \return The minimal Y coordinate.
         */
        float getYMin();
        /**
         * Returns the maximal Y coordinate.
         * \return The maximal Y coordinate.
         */
        float getYMax();

    private:
        /**
         * Sets the linked module progress bar settings.
         * \param steps Points count as reference to the progress bar.
         */
        void setProgressSettings( size_t steps );

        /**
         * Field to output the WDataSetPoints points data.
         */
        boost::shared_ptr< WDataSetPoints > m_outputPoints;
        /**
         * Linkd module progress bar.
         */
        boost::shared_ptr< WProgressCombiner > m_associatedProgressCombiner;
        /**
         * Progress bar status.
         */
        boost::shared_ptr< WProgress > m_progressStatus;

        /**
         * Input LAS file path.
         */
        const char* filePath;
        /**
         * Minimal X coordinate in LAS file.
         */
        float xMin;
        /**
         * Maximal X coordinate in LAS file.
         */
        float xMax;
        /**
         * Minimal Y coordinate in LAS file.
         */
        float yMin;
        /**
         * Maximal Y coordinate in LAS file.
         */
        float yMax;
        /**
         * Minimal Z coordinate in LAS file.
         */
        float zMin;
        /**
         * Maximal Z coordinate in LAS file.
         */
        float zMax;
    };
} /* namespace butterfly */
#endif  // WLASREADER_H
