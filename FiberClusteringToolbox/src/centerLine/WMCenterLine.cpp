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

#include <string>
#include <vector>

#include <core/common/WStrategyHelper.h>
#include <core/dataHandler/WDataSetFibers.h>
#include <core/dataHandler/WDataSetFiberVector.h>
#include <core/dataHandler/WDataSetScalar.h>
#include <core/kernel/WKernel.h>
#include <core/kernel/WModuleInputData.h>

#include "WMCenterLine.h"

WMCenterLine::WMCenterLine():
    WModule()
{
}

WMCenterLine::~WMCenterLine()
{
}

boost::shared_ptr< WModule > WMCenterLine::factory() const
{
    return boost::shared_ptr< WModule >( new WMCenterLine() );
}

const std::string WMCenterLine::getName() const
{
    return "Center Line";
}

const std::string WMCenterLine::getDescription() const
{
    return "Computes the center line.";
}

void WMCenterLine::connectors()
{
    m_fibersIC = WModuleInputData< WDataSetFibers >::createAndAdd( shared_from_this(), "fibers", "The fibers" );
    m_fibersOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(), "centerline", "The center line." );
    m_unifibersOC = WModuleOutputData< WDataSetFibers >::createAndAdd( shared_from_this(), "unifibs", "Unified fibs" );

    WModule::connectors();
}

void WMCenterLine::properties()
{
    WModule::properties();
}

void WMCenterLine::requirements()
{
}

WDataSetFibers::SPtr WMCenterLine::computeCenterLine( WDataSetFibers::SPtr fibers )
{
    boost::shared_ptr< const WDataSetFiberVector > tracts( new WDataSetFiberVector( fibers ) );

    if( !tracts || tracts->empty() ) // invalid data produces invalid center lines
    {
        return WDataSetFibers::SPtr( new WDataSetFibers() );
    }

    size_t avgTractSize = 0;
    for( WDataSetFiberVector::const_iterator cit = tracts->begin(); cit != tracts->end(); ++cit )
    {
        avgTractSize += cit->size();
    }
    avgTractSize /= tracts->size();

    WFiber firstTract( tracts->front() );
    firstTract.resampleByNumberOfPoints( avgTractSize );
    boost::shared_ptr< WFiber > result( new WFiber( firstTract ) ); // copy the first tract into result centerline

    for( size_t tractIndex = 1; tractIndex < tracts->size(); ++tractIndex )
    {
        WFiber other( tracts->at( tractIndex ) );
        other.resampleByNumberOfPoints( avgTractSize );
        other.unifyDirectionBy( firstTract );

        for( size_t pointIndex = 0; pointIndex < avgTractSize; ++pointIndex )
        {
            result->at( pointIndex ) += other[ pointIndex ];
        }
    }

    for( size_t pointIndex = 0; pointIndex < avgTractSize; ++pointIndex )
    {
        result->at( pointIndex ) /= static_cast< double >( tracts->size() );
    }

    WDataSetFiberVector::SPtr unifibs( new WDataSetFiberVector( fibers ) );
    for( size_t tractIndex = 1; tractIndex < unifibs->size(); ++tractIndex )
    {
        unifibs->at( tractIndex ).resampleByNumberOfPoints( avgTractSize );
        unifibs->at( tractIndex ).unifyDirectionBy( firstTract );
    }

    m_unifibersOC->updateData( unifibs->toWDataSetFibers() );

    WDataSetFiberVector::SPtr ds( new WDataSetFiberVector( boost::shared_ptr< std::vector< WFiber > >( new std::vector< WFiber >( 1, *result ) ) ) );
    return ds->toWDataSetFibers();
}

void WMCenterLine::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_fibersIC->getDataChangedCondition() );

    ready();

    // main loop
    while( !m_shutdownFlag() )
    {
        infoLog() << "Waiting ...";
        m_moduleState.wait();

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        WDataSetFibers::SPtr fibers = m_fibersIC->getData();

        if( !fibers )
        {
            continue;
        }

        m_fibersOC->updateData( computeCenterLine( m_fibersIC->getData() ) );
        // WProgress::SPtr progress( new WProgress( "Coloring Fibers", fibers->size() ) );
        // m_progress->addSubProgress( progress );
        // debugLog() << "Start coloring";
        // m_fibersOC->updateData( m_strategy()->operator()( progress, m_shutdownFlag, fibers ) );
        // progress->finish();
        // m_progress->removeSubProgress( progress );
        // debugLog() << "Finished coloring";
    }
}
