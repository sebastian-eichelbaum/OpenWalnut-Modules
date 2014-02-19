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
#include <iostream> // std::cout

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
    }
    WLasReader::WLasReader( boost::shared_ptr< WProgressCombiner > progress )
    {
        this->m_associatedProgressCombiner = progress;
        m_xMin = m_xMax = m_yMin = m_yMax = m_zMin = m_zMax = 1;
        filePath = 0;
    }
    WLasReader::~WLasReader()
    {
        // TODO(schwarzkopf): Auto-generated destructor stub
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

    boost::shared_ptr< WDataSetPoints > WLasReader::getPoints(
            size_t fromX, size_t fromY, size_t dataSetWidth, bool moveToCenter )
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
        float xOffset = fromX + dataSetWidth / 2;
        float yOffset = fromY + dataSetWidth / 2;
        float zOffset = ( m_zMax - m_zMin ) / 2;

//        double oldTime = 0.0; //TODO(schwarzkopf): remove that commented lines in final version
//        size_t oldI = 0;
//        size_t timeIdx = 0;
//        size_t crashCount = 0;

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
//            { //TODO(schwarzkopf): remove that commented lines in final version
//                double thisTime = point.GetTime();
//                if(oldTime != thisTime)
//                {
//                    if(oldTime>thisTime)
//                    {
//                        crashCount++;
//                        std::cout << "crash!! " << oldTime << "\t" << thisTime-oldTime << std::endl;
//                    }
//                    double diff = thisTime-oldTime;
//                    std::cout << diff << "\t" << ( i - oldI ) << std::endl;
//                    oldTime = thisTime;
//                    oldI = i;
//                    timeIdx++;
//                }
//                //std::cout << "Point stuff: " << point.GetFlightLineEdge() << "\t" <<
//                //        point.GetPointSourceID() << "\t" << point.GetRawX() << "\t" <<
//                //        timeIdx << std::endl;
//            }
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

            if  ( dataSetWidth == 0 || ( x >= fromX && x < fromX+dataSetWidth
                    && y >= fromY && y < fromY+dataSetWidth ) )
            {
                if  ( moveToCenter )
                {
                    x -= xOffset;
                    y -= yOffset;
                    z -= zOffset;
                }
                vertices->push_back( x );
                vertices->push_back( y );
                vertices->push_back( z );
                for  ( int color = 0; color < 3; color++ )
                    colors->push_back( v );
                addedPoints++;
            }
//            std::cout << i << ": " << x << " " << " " << y << " " << z << " " << v << "\r\n";
            m_progressStatus->increment( 1 );
        }
//        std::cout << "Time/point count: " << timeIdx << " / " << count <<
//                "\tInvalid order occurances: " << crashCount<< std::endl;
        m_progressStatus->finish();
//        std::cout << "Added points: " << addedPoints << std::endl;

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
} /* namespace std */
