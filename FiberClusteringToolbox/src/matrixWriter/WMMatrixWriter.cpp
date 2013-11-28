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

#include <fstream>
#include <string>
#include <vector>

#include <core/common/WPathHelper.h>
#include <core/kernel/WKernel.h>
#include <core/kernel/WModuleInputData.h>

#include "WMMatrixWriter.h"

WMMatrixWriter::WMMatrixWriter():
    WModule()
{
}

WMMatrixWriter::~WMMatrixWriter()
{
}

boost::shared_ptr< WModule > WMMatrixWriter::factory() const
{
    return boost::shared_ptr< WModule >( new WMMatrixWriter() );
}

const std::string WMMatrixWriter::getName() const
{
    return "Matrix Writer";
}

const std::string WMMatrixWriter::getDescription() const
{
    return "Writes a matrix to specified file.";
}

void WMMatrixWriter::connectors()
{
    m_fltMatrixIC = WModuleInputData< WDataSetMatrixSymFLT >::createAndAdd( shared_from_this(), "matrix", "Matrix which should be saved." );

    WModule::connectors();
}

void WMMatrixWriter::properties()
{
    // m_properties->addProperty( m_strategy.getProperties() );
    m_filename = m_properties->addProperty( "Filename", "Filename where to write the matrix to.",
                                             WPathHelper::getHomePath() );
    m_saveTrigger = m_properties->addProperty( "Save as full matrix",  "Save!", WPVBaseTypes::PV_TRIGGER_READY );
    m_saveTrigger->getCondition()->subscribeSignal( boost::bind( &WMMatrixWriter::save, this ) );

    // m_binary = m_properties->addProperty( "Binary", "If checked it is written as binary, otherwise text", false );

    WModule::properties();
}

void WMMatrixWriter::save()
{
    if( !m_fltMatrixIC->getData() )
    {
        warnLog() << "Cannot save invalid. Please connect valid data to connector and try again";
        return;
    }
    WMatrixSymFLT::SPtr data = m_fltMatrixIC->getData()->getData();
    WProgress::SPtr progress( new WProgress( "Writing Matrix", data->size() ) );
    m_progress->addSubProgress( progress );
    debugLog() << "Start writing...";

    std::string fname = m_filename->get().string();
    std::ofstream fs( fname.c_str() );
    fs << *data;
    fs.close();
    m_saveTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );

    progress->finish();
    m_progress->removeSubProgress( progress );
    debugLog() << "Finished writing";
}

void WMMatrixWriter::requirements()
{
}

void WMMatrixWriter::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_fltMatrixIC->getDataChangedCondition() );

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

        // TODO(math): nur ein IC sollte valide daten haben, und dieser wird dann auch geschrieben, sonst nichts.
        WDataSetMatrixSymFLT::SPtr matrix = m_fltMatrixIC->getData();

        if( !matrix )
        {
            continue;
        }
    }
}
