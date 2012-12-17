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
#include <vector>

#include <core/common/WProgress.h>
#include <core/dataHandler/WDataSetFibers.h>
#include <core/dataHandler/WDataSetFiberVector.h>

#include "WCullingStrategyInterface.h"
#include "WZhangThresholdCulling.h"

WZhangThresholdCulling::WZhangThresholdCulling() :
    WObjectNDIP< WCullingStrategyInterface >( "ZhangThreshold", "Cullout small tracts nearby long tracts." )
{
    m_dSt = m_properties->addProperty( "Min tract distance", "If below, the shorter tract is culled out", 6.5 );
    m_proximity = m_properties->addProperty( "Min point distance", "If below, point distance not considered for tract", 1.0 );
}

WZhangThresholdCulling::~WZhangThresholdCulling()
{
}

std::pair< WDataSetFibers::SPtr, WDataSetFibers::SPtr > WZhangThresholdCulling::operator()( WDataSetFibers::SPtr fibers, WProgress::SPtr progress )
{
     // Its easier to implement with vectors of WFiber instead of the index garbage, (yes, this consumes temporally more space and is a bit slower )
     WDataSetFiberVector::SPtr data( new WDataSetFiberVector( fibers ) );

    double proximity_t      = m_proximity->get();
    double dSt_culling_t    = m_dSt->get();

    size_t numTracts = data->size();

    data->sortDescLength(); // for speed up begin with the longest lines (#sample_points) first

    std::vector< bool > unusedTracts( numTracts, false );

    boost::function< double ( const WFiber& q, const WFiber& r ) > dSt; // NOLINT
    dSt = boost::bind( WFiber::distDST, proximity_t * proximity_t, _1, _2 );

    for( size_t q = 0; q < numTracts; ++q )
    {
        ++*progress;
        if( !unusedTracts[q] )
        {
            for( size_t r = q + 1; r < numTracts; ++r )
            {
                if( !unusedTracts[r] )
                {
                    double dst = dSt( (*data)[q], (*data)[r] );
                    if( dst < dSt_culling_t )  // cullout small tracts nearby long tracts
                    {
                        if( (*data)[q].size() < (*data)[r].size() )
                        {
                            unusedTracts[q] = true;
                            break;
                        }
                        else
                        {
                            unusedTracts[r] = true;
                        }
                    }
                }
            }
        }
    }
    progress->finish();

    WDataSetFiberVector::SPtr remaining = data->generateDataSetOutOfUsedFibers( unusedTracts );
    unusedTracts.flip(); // invert unusedTracts
    WDataSetFiberVector::SPtr culled = data->generateDataSetOutOfUsedFibers( unusedTracts );

    progress->finish();

    return std::make_pair( remaining->toWDataSetFibers(), culled->toWDataSetFibers() );
}
