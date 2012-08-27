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

#include <string>

#include "WDataSetGP.h"

boost::shared_ptr< WPrototyped > WDataSetGP::m_prototype = boost::shared_ptr< WPrototyped >();

WDataSetGP::WDataSetGP()
    : WMixinVector< WGaussProcess >(),
      WDataSet()
{
}

WDataSetGP::WDataSetGP( boost::shared_ptr< const WDataSetFibers > tracts,
                        boost::shared_ptr< const WDataSetDTI > tensors,
                        const WBoolFlag& shutdownFlag,
                        boost::shared_ptr< WProgress > progress )
    : WMixinVector< WGaussProcess >(),
      WDataSet()
{
    init( tracts, tensors, shutdownFlag, progress );
}

void WDataSetGP::init( boost::shared_ptr< const WDataSetFibers > tracts,
                       boost::shared_ptr< const WDataSetDTI > tensors,
                       const WBoolFlag& shutdownFlag,
                       boost::shared_ptr< WProgress > progress )
{
    reserve( tracts->size() );
    for( size_t i = 0; i < tracts->size(); ++i )
    {
        if( shutdownFlag() )
        {
            clear();
            break;
        }
        ++*progress;
        push_back( WGaussProcess( i, tracts, tensors ) );
    }
}

WDataSetGP::~WDataSetGP()
{
}

double WDataSetGP::mean( const WPosition& p ) const
{
    double avg = 0.0;
    for( const_iterator cit = begin(); cit != end(); ++cit )
    {
        avg += cit->mean( p );
    }
    return ( avg < 1.0 ? avg : 1.0 ); // real averaging would be bad when to many processes comes into account
}

const std::string WDataSetGP::getName() const
{
    return "WDataSetGP";
}

const std::string WDataSetGP::getDescription() const
{
    return "Contains Gaussian processes representing deterministic tracks.";
}

boost::shared_ptr< WPrototyped > WDataSetGP::getPrototype()
{
    if( !m_prototype )
    {
        m_prototype = boost::shared_ptr< WPrototyped >( new WDataSetGP() );
    }

    return m_prototype;
}
