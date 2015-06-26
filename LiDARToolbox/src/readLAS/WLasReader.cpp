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

#include <liblas/liblas.hpp>
#include <fstream>  // std::ifstream

#include <string>
#include <vector>

#include "WLasReader.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WKernel.h"
#include "../common/math/vectors/WVectorMaths.h"

namespace laslibb
{
    WLasReader::WLasReader()
    {
        m_minCoord.reserve( 3 );
        m_minCoord.resize( 3 );
        m_maxCoord.reserve( 3 );
        m_maxCoord.resize( 3 );
        m_minColor.reserve( 3 );
        m_minColor.resize( 3 );
        m_maxColor.reserve( 3 );
        m_maxColor.resize( 3 );
        m_filePath = 0;
        m_selectionX = 0;
        m_selectionY = 0;
        m_selectionRadius = 100;
        m_translateToCenter = true;
        m_contrast = 0.005;
    }

    WLasReader::WLasReader( boost::shared_ptr< WProgressCombiner > progress )
    {
        this->m_associatedProgressCombiner = progress;
        m_minCoord.reserve( 3 );
        m_minCoord.resize( 3 );
        m_maxCoord.reserve( 3 );
        m_maxCoord.resize( 3 );
        m_minColor.reserve( 3 );
        m_minColor.resize( 3 );
        m_maxColor.reserve( 3 );
        m_maxColor.resize( 3 );
        m_filePath = 0;
    }

    WLasReader::~WLasReader()
    {
    }

    void WLasReader::setProgressSettings( size_t steps )
    {
        m_associatedProgressCombiner->removeSubProgress( m_progressStatus );
        std::string headerText = "Loading data";
        m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
        m_associatedProgressCombiner->addSubProgress( m_progressStatus );
    }

    void WLasReader::setColorsEnabled( bool colorsEnabled )
    {
        m_colorsEnabled = colorsEnabled;
    }

    void WLasReader::setInputFilePath( const char* path )
    {
        m_filePath = path;
    }

    boost::shared_ptr< WDataSetPoints > WLasReader::getPoints()
    {
        WDataSetPoints::VertexArray vertices(
                new WDataSetPoints::VertexArray::element_type() );
        WDataSetPoints::ColorArray colors(
                new WDataSetPoints::ColorArray::element_type() );

        std::ifstream ifs;
        ifs.open( m_filePath, std::ios::in | std::ios::binary );


        liblas::ReaderFactory f;
        liblas::Reader reader = f.CreateWithStream( ifs );

        liblas::Header const& header = reader.GetHeader();

        liblas::Color colorLas;
        size_t count = header.GetPointRecordsCount();
        setProgressSettings( count );
        size_t addedPoints = 0;
        vector<double> offset = WVectorMaths::new3dVector( m_selectionX, m_selectionY,
                ( m_maxCoord[2] - m_minCoord[2] ) / 2.0 );
        vector<double> color( 3, 0.0 );

        for  ( size_t i = 0; i < count; i++ )
        {
            reader.ReadNextPoint();

            liblas::Point point = reader.GetPoint();
            vector<double> coord = WVectorMaths::new3dVector( point.GetX(), point.GetY(), point.GetZ() );

            double intensity = point.GetIntensity(); //TODO(schwarzkopf): Still had no colored data set to check some liblas functions.
            colorLas = point.GetColor();
            color[0] = colorLas.GetRed();
            color[1] = colorLas.GetGreen();
            color[2] = colorLas.GetBlue();


            for( size_t dimension = 0; dimension < 3; dimension++ )
            {
                if( coord[dimension] < m_minCoord[dimension] || i == 0 )
                    m_minCoord[dimension] = coord[dimension];
                if( coord[dimension] > m_maxCoord[dimension] || i == 0 )
                    m_maxCoord[dimension] = coord[dimension];

                if( color[dimension] < m_minColor[dimension] || i == 0 )
                    m_minColor[dimension] = color[dimension];
                if( color[dimension] > m_maxColor[dimension] || i == 0 )
                    m_maxColor[dimension] = color[dimension];
            }

            if  ( intensity < m_intensityMin ) m_intensityMin = intensity;
            if  ( intensity > m_intensityMax ) m_intensityMax = intensity;

            if  ( m_selectionRadius == 0
                    || ( coord[0] >= m_selectionX - m_selectionRadius
                            && coord[0] <= m_selectionX + m_selectionRadius
                            && coord[1] >= m_selectionY - m_selectionRadius
                            && coord[1] <= m_selectionY + m_selectionRadius ) )
            {
                for( size_t dimension = 0; m_translateToCenter && dimension < 3; dimension++ )
                    coord[dimension] -= offset[dimension];

                for( size_t dimension = 0; dimension < 3; dimension++ )
                    vertices->push_back( coord[dimension] );
                for  ( int colorIndex = 0; colorIndex < 3; colorIndex++ )
                    colors->push_back( m_colorsEnabled ?( color[colorIndex] * m_contrast )
                            :( intensity * m_contrast ) );
                addedPoints++;
            }
            m_progressStatus->increment( 1 );
        }
        m_progressStatus->finish();

        if  ( addedPoints == 0 )
        {
            //TODO(aschwarzkopf): Handle the problem in other way. When no points exist then the program crashes.
            for  ( size_t lfd = 0; lfd < 3; lfd++)
            {
                vertices->push_back( 0 );
                colors->push_back( 0 );
            }
        }
        boost::shared_ptr< WDataSetPoints > outputPoints(
                new WDataSetPoints( vertices, colors ) );
        m_outputPoints = outputPoints;

        return m_outputPoints;
    }

    void WLasReader::setDataSetRegion( double selectionX, double selectionY, double selectionRadius )
    {
        m_selectionX = selectionX;
        m_selectionY = selectionY;
        m_selectionRadius = selectionRadius;
    }

    void WLasReader::setTranslateToCenter( bool translateToCenter )
    {
        m_translateToCenter = translateToCenter;
    }

    void WLasReader::setContrast( double contrast )
    {
        m_contrast = contrast;
    }

    vector<double> WLasReader::getMinCoord()
    {
        return m_minCoord;
    }

    vector<double> WLasReader::getMaxCoord()
    {
        return m_maxCoord;
    }

    double WLasReader::getIntensityMin()
    {
        return m_intensityMin;
    }

    double WLasReader::getIntensityMax()
    {
        return m_intensityMax;
    }

    vector<double> WLasReader::getColorMin()
    {
        return m_minColor;
    }

    vector<double> WLasReader::getColorMax()
    {
        return m_maxColor;
    }
} /* namespace laslibb */
