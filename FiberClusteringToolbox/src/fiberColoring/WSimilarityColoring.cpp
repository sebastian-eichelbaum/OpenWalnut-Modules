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

#include <vector>
#include <fstream>

#include <boost/lexical_cast.hpp>

#include <core/common/WPathHelper.h>

#include "WSimilarityColoring.h"

WSimilarityColoring::WSimilarityColoring()
    : WObjectNDIP< WColoring_I >( "Similarity Coloring", "Color codes the similarity based on 2D embedding" )
{
    m_filename = m_properties->addProperty( "Filename", "Filename where to write the NIfTI file to.",
                                             WPathHelper::getAppPath() );
    WPropertyHelper::PC_PATHEXISTS::addTo( m_filename );
    m_loadTrigger = m_properties->addProperty( "Read fiber Positions",  "Read!", WPVBaseTypes::PV_TRIGGER_READY );
    m_loadTrigger->getCondition()->subscribeSignal( boost::bind( &WSimilarityColoring::loadPositions, this ) );

    m_bb = m_properties->addProperty( "BB", "BoundingBox squared assumed", 1007 );
    m_bb->setMin( 1 );
    m_r1 = m_properties->addProperty( "R1", "First Radius", 45.0 );
    m_r2 = m_properties->addProperty( "R2", "Second Radius", 25.0 );
    m_L0 = m_properties->addProperty( "L0", "L0", 70.0 );
    m_a0 = m_properties->addProperty( "a0", "a0", 10.0 );
    m_b0 = m_properties->addProperty( "b0", "b0", 25.0 );
}

WSimilarityColoring::~WSimilarityColoring()
{
}

void WSimilarityColoring::loadPositions() {

    m_2dpos.clear();

    wlog::debug( "WSimilarity Coloring" ) << "Filename: " << m_filename->get().string();
    std::ifstream file( m_filename->get().string().c_str() );
    std::string str;
    while (std::getline(file, str))
    {
        std::vector< std::string > cols = string_utils::tokenize( str );
        if( cols.size() < 2 ) {
            wlog::error( "WSimilarityColoring" ) << "Invalid positions file, abort";
            std::cout << str << std::endl;        // Process str
            return;
        }
        else {
            std::pair< double, double > pos = std::make_pair( 0.0, 0.0 );
            try
            {
                pos.first = boost::lexical_cast<double>(cols[0]) / m_bb->get();
                pos.second = boost::lexical_cast<double>(cols[1]) / m_bb->get();
            }
            catch (boost::bad_lexical_cast const&)
            {
                wlog::error( "WSimilarityColoring" ) << "Error during reading ASCII pos file ( lexical cast )";
            }
            m_2dpos.push_back( pos );
      }
    }
}

namespace {
    osg::Vec3f xyz2rgb( osg::Vec3f xyz ) {
        double var_X = xyz[0] / 100.0;
        double var_Y = xyz[1] / 100.0;
        double var_Z = xyz[2] / 100.0;

        double var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
        double var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
        double var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

        if ( var_R > 0.0031308 )
            var_R = 1.055 * std::pow( var_R, 1 / 2.4 ) - 0.055;
        else
            var_R = 12.92 * var_R;
        if ( var_G > 0.0031308 )
            var_G = 1.055 * ( std::pow( var_G, 1 / 2.4 ) ) - 0.055;
        else
            var_G = 12.92 * var_G;
        if ( var_B > 0.0031308 )
            var_B = 1.055 * ( std::pow( var_B, 1 / 2.4 ) ) - 0.055;
        else
            var_B = 12.92 * var_B;

        return osg::Vec3f( var_R, var_G, var_B );
    }

    osg::Vec3f lab2xyz( osg::Vec3f lab ) {
        double var_Y = ( lab[0] + 16 ) / 116.0;
        double var_X = lab[1] / 500.0 + var_Y;
        double var_Z = var_Y - lab[2] / 200.0;

        if ( std::pow( var_Y, 3.0 ) > 0.008856 )
            var_Y = var_Y * var_Y * var_Y;
        else
            var_Y = ( var_Y - 16 / 116 ) / 7.787;
        if ( std::pow( var_X, 3.0 ) > 0.008856 )
            var_X = var_X * var_X * var_X;
        else
            var_X = ( var_X - 16 / 116 ) / 7.787;
        if ( std::pow( var_Z, 3.0 ) > 0.008856 )
            var_Z = var_Z * var_Z * var_Z;
        else
            var_Z = ( var_Z - 16 / 116 ) / 7.787;

        return osg::Vec3f( 95.047 * var_X, 100.0 * var_Y, 108.883 * var_Z );
    }
}
WDataSetFibers::SPtr WSimilarityColoring::operator()( WProgress::SPtr progress, WBoolFlag const & shutdown, WDataSetFibers::SPtr fibers )
{
    fibers->removeColorScheme( m_similarityColors );

    if( !fibers || m_2dpos.empty() )
    {
        wlog::debug( "WSimilarityColoring" ) << "No positions available or invalid fiber dataset";
        return fibers;
    }
    if( fibers->getLineLengths()->size() != m_2dpos.size() ) {
        wlog::error( "WSimilarityColoring" ) << "#positions: " << m_2dpos.size() << " does not match #fibers: " << fibers->getLineLengths()->size();
    }

    // Assume that there is a valid dataset
    WDataSetFibers::IndexArray   fibStart = fibers->getLineStartIndexes();
    WDataSetFibers::LengthArray  fibLen   = fibers->getLineLengths();
    WDataSetFibers::VertexArray  fibVerts = fibers->getVertices();
    WDataSetFibers::ColorArray   newColors( new std::vector< float > );


    for( size_t fidx = 0; fidx < fibStart->size(); ++fidx ) // iterate over fibers
    {
        if( shutdown )
        {
            wlog::info( "WSimilarityColoring" ) << "Abort coloring due to shutdown request.";
            break;
        }
        ++*progress;

        size_t len = fibLen->at( fidx );
        for( size_t k = 0; k < len; ++k )
        {
            std::pair< double, double > pos = m_2dpos[fidx];
            pos.first = 2 * M_PI * pos.first;
            pos.second = 2 * M_PI * pos.second;
            osg::Vec4 donut( m_r1->get() * std::cos( pos.first ),
                             m_r1->get() * std::sin( pos.first ),
                             m_r2->get() * std::cos( pos.second ),
                             m_r2->get() * std::sin( pos.second ) );
            osg::Vec3f lab( m_L0->get() + donut[3],
                            m_a0->get() + m_r1->get() + donut[0] + donut[2],
                            m_b0->get() + donut[1] );

            osg::Vec3f col = xyz2rgb( lab2xyz( lab ) );

            newColors->push_back( col[0] );
            newColors->push_back( col[1] );
            newColors->push_back( col[2] );
        }
    }
    m_similarityColors = newColors;
    fibers->addColorScheme( newColors, "Similarity Coloring", "Used 2d embedding of a similarity matrix into LAB colorspace" );
    m_loadTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );
    return fibers;
}
