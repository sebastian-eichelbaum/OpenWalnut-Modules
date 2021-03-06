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

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <osg/Vec4>

#include <core/common/WPathHelper.h>

#include "WStartEndColoring.h"

WStartEndColoring::WStartEndColoring()
    : WObjectNDIP< WColoring_I >( "StartEnd Coloring", "Colorcoding segmentwise from start to end with color scale" )
{
  m_start = m_properties->addProperty( "Start Color", "From this color we start", WColor( 1.0, 0, 0, 1.0 ) );
  m_end = m_properties->addProperty( "Stop Color", "To this color we proceed", WColor( 1.0, 1.0, 1.0, 1.0 ) );
}

WStartEndColoring::~WStartEndColoring()
{
}

osg::Vec4 WStartEndColoring::segmentColor( double param ) const
{
    return m_start->get() * param + m_end->get() * ( 1 - param );
}

WDataSetFibers::SPtr WStartEndColoring::operator()( WProgress::SPtr progress, WBoolFlag const & shutdown, WDataSetFibers::SPtr fibers )
{
    fibers->removeColorScheme( m_StartEndColors );

    if( !fibers )
    {
        return fibers;
    }

    WDataSetFibers::IndexArray   fibStart = fibers->getLineStartIndexes();
    WDataSetFibers::LengthArray  fibLen   = fibers->getLineLengths();
    WDataSetFibers::VertexArray  fibVerts = fibers->getVertices();
    WDataSetFibers::ColorArray   newColors( new std::vector< float > );
    newColors->reserve( fibVerts->size() );

    for( size_t fidx = 0; fidx < fibStart->size(); ++fidx ) // iterate over fibers
    {
        if( shutdown )
        {
            wlog::info( "WStartEndColoring" ) << "Abort coloring due to shutdown request.";
            break;
        }
        ++*progress;

        size_t len = fibLen->at( fidx );
        for( size_t k = 0; k < len; ++k )
        {
            osg::Vec4 col = segmentColor( static_cast< double >( k ) / static_cast< double >( len ) );
            newColors->push_back( col[0] );
            newColors->push_back( col[1] );
            newColors->push_back( col[2] );
        }
    }

    m_StartEndColors = newColors;
    fibers->addColorScheme( newColors, "StartEnd Coloring", "Coloring to indicate first and last segments, and direction" );
    return fibers;
}
