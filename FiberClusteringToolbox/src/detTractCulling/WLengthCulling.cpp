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

#include <utility>

#include <core/common/WProgress.h>
#include <core/dataHandler/WDataSetFibers.h>
#include <core/dataHandler/WDataSetFiberVector.h>

#include "WCullingStrategyInterface.h"
#include "WLengthCulling.h"

WLengthCulling::WLengthCulling() :
    WObjectNDIP< WCullingStrategyInterface >( "Length", "Culls out fibers below a given length." )
{
    m_minLength = m_properties->addProperty( "Minimal Length", "Fibers shorter than this will be culled out", 0.0 );
    m_minLength->setMin( 0.0 );
    m_minLength->setMax( 1.0 );
}

WLengthCulling::~WLengthCulling()
{
}

std::pair< WDataSetFibers::SPtr, WDataSetFibers::SPtr > WLengthCulling::operator()( WDataSetFibers::SPtr fibers, WProgress::SPtr progress )
{
    // Its easier to implement with vectors of WFiber instead of the index garbage, (yes, this consumes temporally more space and is a bit slower )
    WDataSetFiberVector::SPtr data( new WDataSetFiberVector( fibers ) );
    WDataSetFiberVector::SPtr remaining( new WDataSetFiberVector() );
    WDataSetFiberVector::SPtr culled( new WDataSetFiberVector() );

    double minLength = m_minLength->get( true );
    double maxLengthSoFar = 0.0;
    for( WDataSetFiberVector::const_iterator it = data->begin(); it != data->end(); ++it )
    {
        if( pathLength( *it ) > maxLengthSoFar )
        {
            maxLengthSoFar = pathLength( *it );
        }
    }

    for( WDataSetFiberVector::const_iterator it = data->begin(); it != data->end(); ++it )
    {
        if( pathLength( *it ) < minLength * maxLengthSoFar )
        {
            culled->push_back( *it );
        }
        else
        {
            remaining->push_back( *it );
        }
        ++*progress;
    }

    progress->finish();

    return std::make_pair( remaining->toWDataSetFibers(), culled->toWDataSetFibers() );
}
