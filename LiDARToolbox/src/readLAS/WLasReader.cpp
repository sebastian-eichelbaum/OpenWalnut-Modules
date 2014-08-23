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

namespace laslibb
{
    WLasReader::WLasReader()
    {
        m_xMin = m_xMax = m_yMin = m_yMax = m_zMin = m_zMax = 0;
        filePath = 0;
        m_fromX = 0;
        m_fromY = 0;
        m_dataSetWidth = 200;
        m_translateToCenter = true;
        m_contrast = 0.005;
    }
    WLasReader::WLasReader( boost::shared_ptr< WProgressCombiner > progress )
    {
        this->m_associatedProgressCombiner = progress;
        m_xMin = m_xMax = m_yMin = m_yMax = m_zMin = m_zMax = 1;
        filePath = 0;
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

    void WLasReader::setInputFilePath( const char* path )
    {
        filePath = path;
    }

    boost::shared_ptr< WDataSetPoints > WLasReader::getPoints()
    {
        WDataSetPoints::VertexArray vertices(
                new WDataSetPoints::VertexArray::element_type() );
        WDataSetPoints::ColorArray colors(
                new WDataSetPoints::ColorArray::element_type() );

        std::ifstream ifs;
        ifs.open( filePath, std::ios::in | std::ios::binary );


        liblas::ReaderFactory f;
        liblas::Reader reader = f.CreateWithStream( ifs );

        liblas::Header const& header = reader.GetHeader();

        liblas::Point point;
        size_t count = header.GetPointRecordsCount();
        setProgressSettings( count );
        size_t addedPoints = 0;
        float xOffset = m_fromX + m_dataSetWidth / 2;
        float yOffset = m_fromY + m_dataSetWidth / 2;
        float zOffset = ( m_zMax - m_zMin ) / 2;

        for  ( size_t i = 0; i < count; i++ )
        {
            double x, y, z;
            int v;

            reader.ReadNextPoint();
            point = reader.GetPoint();
            x = point.GetX();
            y = point.GetY();
            z = point.GetZ();
            v = point.GetIntensity(); //TODO(schwarzkopf): Still had no colored data set to check some liblas functions.
            if  ( i == 0 )
            {
                m_xMin = m_xMax = x;
                m_yMin = m_yMax = y;
                m_zMin = m_zMax = z;
                m_intensityMin = m_intensityMax = v;
            }
            if  ( x < m_xMin ) m_xMin = x;
            if  ( x > m_xMax ) m_xMax = x;
            if  ( y < m_yMin ) m_yMin = y;
            if  ( y > m_yMax ) m_yMax = y;
            if  ( z < m_zMin ) m_zMin = z;
            if  ( z > m_zMax ) m_zMax = z;
            if  ( v < m_intensityMin ) m_intensityMin = v;
            if  ( v > m_intensityMax ) m_intensityMax = v;

            if  ( m_dataSetWidth == 0 || ( x >= m_fromX && x < m_fromX+m_dataSetWidth
                    && y >= m_fromY && y < m_fromY+m_dataSetWidth ) )
            {
                if  ( m_translateToCenter )
                {
                    x -= xOffset;
                    y -= yOffset;
                    z -= zOffset;
                }
                vertices->push_back( x );
                vertices->push_back( y );
                vertices->push_back( z );
                for  ( int color = 0; color < 3; color++ )
                    colors->push_back( v * m_contrast );
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
    void WLasReader::setDataSetRegion( size_t fromX, size_t fromY, size_t dataSetWidth )
    {
        m_fromX = fromX;
        m_fromY = fromY;
        m_dataSetWidth = dataSetWidth;
    }
    void WLasReader::setTranslateToCenter( bool translateToCenter )
    {
        m_translateToCenter = translateToCenter;
    }
    void WLasReader::setContrast( double contrast )
    {
        m_contrast = contrast;
    }
    float WLasReader::getXMin()
    {
        return m_xMin;
    }
    float WLasReader::getXMax()
    {
        return m_xMax;
    }
    float WLasReader::getYMin()
    {
        return m_yMin;
    }
    float WLasReader::getYMax()
    {
        return m_yMax;
    }
    float WLasReader::getZMin()
    {
        return m_zMin;
    }
    float WLasReader::getZMax()
    {
        return m_zMax;
    }
    float WLasReader::getIntensityMin()
    {
        return m_intensityMin;
    }
    float WLasReader::getIntensityMax()
    {
        return m_intensityMax;
    }
} /* namespace laslibb */
