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

#include "WSampler2D.h"

// Anonymous namespace for some constants only used in here
namespace
{
    const double rndMax = RAND_MAX;
}

WSampler2DUniform::WSampler2DUniform(  size_t numSamples, double width, double height, RandomInit init )
    : WSampler2D(),
      m_width( width ),
      m_height( height )
{
    reserve( numSamples );

    if( init == CALL_SRAND )
    {
        srand( time( NULL ) );
    }

    for( size_t i = 0; i < numSamples; ++i )
    {
        push_back( WVector2d( std::rand() / rndMax * m_width, std::rand() / rndMax * m_height ) );
    }
}

WSampler2DPoissonFixed::WSampler2DPoissonFixed( boost::filesystem::path path )
    : WSampler2D()
{
    // debugLog() << ( m_localPath / "859035_in_[-1,1]^2.dat.bz2" ).c_str();
    // std::ifstream file( ( m_localPath / "859035_in_[-1,1]^2.dat.bz2" ).c_str(), std::ios_base::in | std::ios_base::binary);
    // boost::iostreams::filtering_streambuf< boost::iostreams::input > in;
    // in.push( boost::iostreams::bzip2_decompressor() );
    // in.push( file );

    std::ifstream file( path.c_str() );
    double x;
    double y;
    while( file ) {
        file >> x >> y;
        x = ( x + 1.0 ) / 2.0;
        y = ( y + 1.0 ) / 2.0;
        push_back( WVector2d( x, y ) );
    }
    file.close();
}
