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

#include <cmath>
#include <vector>

#include <osg/Vec3>

#include <core/common/math/WMath.h>

#include "WBoyGlobalColoring.h"

WBoyGlobalColoring::WBoyGlobalColoring()
    : WObjectNDIP< WColoring_I >( "Boy Global Coloring", "Using Boy's real projective plane immersion: http://dx.doi.org/10.1109/TVCG.2009.125" )
{
}

WBoyGlobalColoring::~WBoyGlobalColoring()
{
}

namespace
{
    // method used by Demiralp's algorithm
    inline double SS( double NA, double ND )
    {
        return ( NA * std::sin( ND * piDouble / 180.0 ) );
    }

    // method used by Demiralp's algorithm
    inline double CC( double NA, double ND )
    {
        return ( NA * std::cos( ND * piDouble / 180.0 ) );
    }

    // Algorithm and values taken from the matlab implementation of the algorithm suggested by C.Demiralp
    osg::Vec3 getColorFromBoysRPPImmersion( double x, double y, double z )
    {
        double xx2 = x * x;
        double yy2 = y * y;
        double zz2 = z * z;
        double xx3 = x * xx2;
        double yy3 = y * yy2;
        double zz3 = z * zz2;
        double zz4 = zz2 * zz2;
        double xy = x * y;
        double xz = x * z;
        double yz = y * z;

        double hh1 = 0.5 * ( 3.0 * zz2 - 1.0 ) / 1.58;
        double hh2 = 3.0 * xz / 2.745;
        double hh3 = 3.0 * yz / 2.745;
        double hh4 = 1.5 * ( xx2 - yy2 ) / 2.745;
        double hh5 = 6.0 * xy / 5.5;
        double hh6 = ( 1.0 / 1.176 ) * 0.125 * ( 35.0 * zz4 - 30.0 * zz2 + 3.0 );
        double hh7 = 2.5 * x * ( 7.0 * zz3 - 3.0 * z ) / 3.737;
        double hh8 = 2.5 * y * ( 7.0 * zz3 - 3.0 * z ) / 3.737;
        double hh9 = ( ( xx2 - yy2 ) * 7.5 * ( 7.0 * zz2 - 1.0 ) ) / 15.85;
        double hh10 = ( ( 2.0 * xy ) * ( 7.5 * ( 7.0 * zz2 - 1.0 ) ) ) / 15.85;
        double hh11 = 105.0 * (  4.0 * xx3 * z - 3.0 * xz * ( 1.0 - zz2 ) ) / 59.32;
        double hh12 = 105.0 * ( -4.0 * yy3 * z + 3.0 * yz * ( 1.0 - zz2 ) ) / 59.32;

        double s0 = -23.0;
        double s1 = 227.9;
        double s2 = 251.0;
        double s3 = 125.0;

        double ss23 = SS( 2.71, s0 );
        double cc23 = CC( 2.71, s0 );
        double ss45 = SS( 2.12, s1 );
        double cc45 = CC( 2.12, s1 );
        double ss67 = SS( 0.972, s2 );
        double cc67 = CC( 0.972, s2 );
        double ss89 = SS( 0.868, s3 );
        double cc89 = CC( 0.868, s3 );

        s0 = -23.0;
        s1 = 227.9;
        s2 = 251.0;
        s3 = 125.0;

        ss23 = SS( 2.71, s0 );
        cc23 = CC( 2.71, s0 );
        ss45 = SS( 2.12, s1 );
        cc45 = CC( 2.12, s1 );
        ss67 = SS( 0.972, s2 );
        cc67 = CC( 0.972, s2 );
        ss89 = SS( 0.868, s3 );
        cc89 = CC( 0.868, s3 );

        double X = 0.0;
        X += hh2 * cc23;
        X += hh3 * ss23;
        X += hh5 * cc45;
        X += hh4 * ss45;
        X += hh7 * cc67;
        X += hh8 * ss67;
        X += hh10 * cc89;
        X += hh9 * ss89;

        double Y = 0.0;
        Y += hh2 * -ss23;
        Y += hh3 * cc23;
        Y += hh5 * -ss45;
        Y += hh4 * cc45;
        Y += hh7 * -ss67;
        Y += hh8 * cc67;
        Y += hh10 * -ss89;
        Y += hh9 * cc89;

        double Z = 0.0;
        Z += hh1 * -2.8;
        Z += hh6 * -0.5;
        Z += hh11 * 0.3;
        Z += hh12 * -2.5;

        //scale and normalize to fit
        //in the rgb space
        double w_x = 4.1925;
        double trl_x = -2.0425;

        double w_y = 4.0217;
        double trl_y = -1.8541;

        double w_z = 4.0694;
        double trl_z = -2.1899;

        osg::Vec3 rgb( 0.0, 0.0, 0.0 );
        rgb[0] = 0.9 * std::abs( ( ( X - trl_x ) / w_x ) ) + 0.05;
        rgb[1] = 0.9 * std::abs( ( ( Y - trl_y ) / w_y ) ) + 0.05;
        rgb[2] = 0.9 * std::abs( ( ( Z - trl_z ) / w_z ) ) + 0.05;
        return rgb;
    }
}

WDataSetFibers::SPtr WBoyGlobalColoring::operator()( WProgress::SPtr progress, WBoolFlag const & shutdown, WDataSetFibers::SPtr fibers )
{
    fibers->removeColorScheme( m_boyColors );

    if( !fibers )
    {
        wlog::debug( "WBoyGlobalColoring" ) << "Invalid fiber dataset";
        return fibers;
    }

    // Assume that there is a valid dataset
    WDataSetFibers::IndexArray   fibStart = fibers->getLineStartIndexes();
    WDataSetFibers::LengthArray  fibLen   = fibers->getLineLengths();
    WDataSetFibers::VertexArray  fibVerts = fibers->getVertices();
    WDataSetFibers::ColorArray   newColors( new std::vector< float > );
    newColors->reserve( fibVerts->size() );

    for( size_t fidx = 0; fidx < fibStart->size(); ++fidx ) // iterate over fibers
    {
        if( shutdown )
        {
            wlog::info( "WBoyGlobalColoring" ) << "Abort coloring due to shutdown request.";
            break;
        }
        ++*progress;

        size_t sidx = fibStart->at( fidx ) * 3;
        WVector3d vec;
        WVector3d col;

        // iterate over all segements and use their orientation to color all vertices. Last vertex has same color as previous vertex.
        if( fibLen->at( fidx ) > 1 )
        {
            WVector3d start( fibVerts->at( sidx + 0 ), fibVerts->at( sidx + 1 ), fibVerts->at( sidx +2 ) );
            WVector3d end( fibVerts->at( sidx + 3 * ( fibLen->at( fidx ) - 1 ) + 0 ),
                           fibVerts->at( sidx + 3 * ( fibLen->at( fidx ) - 1 ) + 1 ),
                           fibVerts->at( sidx + 3 * ( fibLen->at( fidx ) - 1 ) + 2 ) );
            vec = normalize( end - start );
            col = getColorFromBoysRPPImmersion( vec[0], vec[1], vec[2] );

            for( size_t k = 0; k < fibLen->at( fidx ); ++k )
            {
                newColors->push_back( col[0] );
                newColors->push_back( col[1] );
                newColors->push_back( col[2] );
            }
        }
        else
        {
            // just to ensure that each vertex has a corresponding color
            if( fibLen->at( fidx ) == 1 )
            {
                newColors->push_back( 0.0 );
                newColors->push_back( 0.0 );
                newColors->push_back( 0.0 );
            }
        }
    }
    m_boyColors = newColors;
    fibers->addColorScheme( newColors, "BoyGlobal", "Directional color encoding providing better discrimination than RGB" );

    return fibers;
}
