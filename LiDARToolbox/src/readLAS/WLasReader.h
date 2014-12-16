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
using std::vector;

namespace laslibb
{
    /** 
     * Compound that reads a LiDAR file (see www.liblas.org).
     * These *.las files consist of a point set of X/Y/Z coordinates 
     * paired with the intensity or color.
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
         * Sets whether the reader should take the colored or greyscale channel. Mostly
         * only one data input is available.
         * \param colorsEnabled Use the colored channel instead the greyscale one.
         */
        void setColorsEnabled( bool colorsEnabled );

        /**
         * Sets the LAS input file path.
         * \param path Path of the read LAS file.
         */
        void setInputFilePath( const char* path );

        /**
         * Returns the read LiDAR data set points.
         * \return LiDAR data set points containing intensities or colors.
         */
        boost::shared_ptr< WDataSetPoints > getPoints();

        /**
         * Sets from which data set region the point data should be loaded.
         * \param selectionX Selection centre X coordinate.
         * \param selectionY Selection centre Y coordinate.
         * \param selectionRadius Selection radius.
         */
        void setDataSetRegion( double selectionX, double selectionY, double selectionRadius );

        /**
         * Sets whether to move the input data to the coordinate system center.
         * \param translateToCenter Move the data to the center of the coordinate system.
         */
        void setTranslateToCenter( bool translateToCenter );

        /**
         * Sets the value that multiplies the intensity of the color output.
         * \param contrast The value that multiplies the intensity of the color output.
         */
        void setContrast( double contrast );

        /**
         * Returns the minimal coordinate.
         * \return The minimal coordinate.
         */
        vector<double> getMinCoord();

        /**
         * Returns the maximal coordinate.
         * \return The maximal coordinate.
         */
        vector<double> getMaxCoord();

        /**
         * Returns the maximal color RGB values
         * \return Maximal color RGB values.
         */
        vector<double> getColorMin();

        /**
         * Returns the minimal color RGB values
         * \return Mimimal color RGB values.
         */
        vector<double> getColorMax();

        /**
         * Returns the minimal color intensity in LAS file.
         * \return Minimal color intensity in LAS file.
         */
        double getIntensityMin();

        /**
         * Returns the maximal color intensity in LAS file.
         * \return Maximal color intensity in LAS file.
         */
        double getIntensityMax();


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
         * Enables depicting colors instead of greyscale las file input.
         */
        bool m_colorsEnabled;

        /**
         * Input LAS file path.
         */
        const char* m_filePath;

        /**
         * Centre X coordinate of the input data selection.
         */
        double m_selectionX;

        /**
         * Centre Y coordinate of the input data selection.
         */
        double m_selectionY;

        /**
         * Radius of the selected input dataset.
         */
        double m_selectionRadius;

        /**
         * Setting whether to move the data to the center of the coordinate system.
         */
        bool m_translateToCenter;

        /**
         * The value that multiplies the intensity of the color output.
         */
        double m_contrast;

        /**
         * Minimal coordinate in LAS file.
         */
        vector<double> m_minCoord;

        /**
         * Maximal coordinate in LAS file.
         */
        vector<double> m_maxCoord;

        /**
         * Minimal color RGB values;
         */
        vector<double> m_minColor;

        /**
         * Maximal color RGB values;
         */
        vector<double> m_maxColor;

        /**
         * Minimal color intensity in LAS file.
         */
        double m_intensityMin;

        /**
         * Maximal color intensity in LAS file.
         */
        double m_intensityMax;
    };
} /* namespace butterfly */
#endif  // WLASREADER_H
