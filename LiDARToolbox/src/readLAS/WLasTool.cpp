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
#include <liblas/point.hpp>
#include <liblas/capi/liblas.h>
#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include <string>
#include <vector>

#include "WLasTool.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WKernel.h"

namespace laslibb
{
    WLasTool::WLasTool()
    {
    }
    WLasTool::WLasTool( boost::shared_ptr< WProgressCombiner > progress )
    {
    	this->m_associatedProgressCombiner = progress;


        WDataSetPoints::VertexArray vertices(
        		new WDataSetPoints::VertexArray::element_type() );
        WDataSetPoints::ColorArray colors(
        		new WDataSetPoints::ColorArray::element_type() );

    	std::cout << "this is lasLibb:\r\n";

        std::ifstream ifs;
        ifs.open("/home/renegade/Dokumente/media/Dropbox/LiDAR2008/tiles/las/"
        		"316500_234500.las", std::ios::in | std::ios::binary);


		liblas::ReaderFactory f;
		liblas::Reader reader = f.CreateWithStream(ifs);

		liblas::Header const& header = reader.GetHeader();

		liblas::Point point;
		size_t count = header.GetPointRecordsCount();
//		count = 100000;
		setProgressSettings( count );
		float xMin = 0, xMax = 0, yMin = 0, yMax = 0, zMin = 0, zMax = 0;
		int vMin, vMax;
		for  ( size_t i=0; i<count; i++ ) {
			double x,y,z; int v;

			reader.ReadNextPoint();
			point = reader.GetPoint();
			x = point.GetX();
			y = point.GetY();
			z = point.GetZ();
			vertices->push_back( x );
			vertices->push_back( y );
			vertices->push_back( z );
			v = point.GetIntensity();
			for  ( int color = 0; color < 3; color++ )
				colors->push_back( v * 255 / 400 );
			if  ( i==0 )
			{
				xMin = x; xMax = x;
				yMin = y; yMax = y;
				zMin = z; zMax = z;
				vMin = z; vMax = z;
			}
			if (x<xMin) xMin = x; if (x>xMax) xMax = x;
			if (y<yMin) yMin = y; if (y>yMax) yMax = y;
			if (z<zMin) zMin = z; if (z>zMax) zMax = z;
			if (v<vMin) vMin = v; if (v>vMax) vMax = v;
//			std::cout << i << ": " << x << " " << " " << y << " " << z << " " << v << "\r\n";
			m_progressStatus->increment( 1 );
		}
		m_progressStatus->finish();
		std::cout << "X: " << xMin << " - " << xMax << "\r\n";
		std::cout << "Y: " << yMin << " - " << yMax << "\r\n";
		std::cout << "Z: " << zMin << " - " << zMax << "\r\n";
		std::cout << "V: " << vMin << " - " << vMax << "\r\n";
		std::cout << "Sets: " << count << "\r\n";

		std::cout << "Data set count = " << count << "\r\n";



        boost::shared_ptr< WDataSetPoints > tmpPoints(
        		new WDataSetPoints( vertices, colors ) );
        m_outputPoints = tmpPoints;
    }
    void WLasTool::setProgressSettings( size_t steps )
    {
        m_associatedProgressCombiner->removeSubProgress( m_progressStatus );
        std::string headerText = "Loading data";
        m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
        m_associatedProgressCombiner->addSubProgress( m_progressStatus );
    }


    boost::shared_ptr< WDataSetPoints > WLasTool::getPoints ()
    {
    	return m_outputPoints;
    }

    WLasTool::~WLasTool()
    {
        // TODO(schwarzkopf): Auto-generated destructor stub
    }
} /* namespace std */
