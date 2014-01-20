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
        xMin = xMax = yMin = yMax = zMin = zMax = 0;
        filePath = 0;
    }
    WLasReader::WLasReader( boost::shared_ptr< WProgressCombiner > progress )
    {
        this->m_associatedProgressCombiner = progress;
        xMin = xMax = yMin = yMax = zMin = zMax = 0;
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
        float zOffset = ( zMax - zMin ) / 2;
        for  ( size_t i = 0; i < count; i++ )
        {
            double x, y, z;
            int v;

            reader.ReadNextPoint();
            point = reader.GetPoint();
            x = point.GetX();
            y = point.GetY();
            z = point.GetZ();
            if  ( i == 0 )
            {
                xMin = xMax = x;
                yMin = yMax = y;
                zMin = zMax = z;
            }
            if  ( x < xMin ) xMin = x;
            if  ( x > xMax ) xMax = x;
            if  ( y < yMin ) yMin = y;
            if  ( y > yMax ) yMax = y;
            if  ( z < zMin ) zMin = z;
            if  ( z > zMax ) zMax = z;

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
                v = point.GetIntensity();
                for  ( int color = 0; color < 3; color++ )
                    colors->push_back( v );
                addedPoints++;
            }
//            std::cout << i << ": " << x << " " << " " << y << " " << z << " " << v << "\r\n";
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
    float WLasReader::getXMin()
    {
        return xMin;
    }
    float WLasReader::getXMax()
    {
        return xMax;
    }
    float WLasReader::getYMin()
    {
        return yMin;
    }
    float WLasReader::getYMax()
    {
        return yMax;
    }
} /* namespace std */
