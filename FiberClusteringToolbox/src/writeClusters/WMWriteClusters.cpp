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

#include "WWriterFiberClusters.h"

#include "WMWriteClusters.h"

WMWriteClusters::WMWriteClusters()
{
}

WMWriteClusters::~WMWriteClusters()
{
}

boost::shared_ptr< WModule > WMWriteClusters::factory() const
{
    return boost::shared_ptr< WModule >( new WMWriteClusters );
}

const char** WMWriteClusters::getXPMIcon() const
{
    return NULL;
}

const std::string WMWriteClusters::getDescription() const
{
    return "Saves fiber cluster data to a file.";
}

const std::string WMWriteClusters::getName() const
{
    return "Write clusters";
}

void WMWriteClusters::connectors()
{
    m_input = boost::shared_ptr< WModuleInputData< WDataSetFiberClustering > >(
        new WModuleInputData< WDataSetFiberClustering >( shared_from_this(), "in", "The clusters to write to a file." ) );
    addConnector( m_input );

    WModule::connectors();
}

void WMWriteClusters::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_propCondition );
    m_moduleState.add( m_input->getDataChangedCondition() );

    ready();

    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        if( m_shutdownFlag() )
        {
            break;
        }

        if( m_writeTrigger->get( true ) == WPVBaseTypes::PV_TRIGGER_TRIGGERED && m_input->getData() )
        {
            std::string fn = m_propFilename->get( true ).string();
            if( fn != "/" )
            {
                WWriterFiberClusters clusterWriter( fn );
                clusterWriter.writeClusters( m_input->getData() );
            }

            m_writeTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, false );
        }
    }
}

void WMWriteClusters::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition );

    m_propFilename = m_properties->addProperty( "File path", "The file to write to.", boost::filesystem::path( "/" ), m_propCondition );

    m_writeTrigger = m_properties->addProperty( "Save", "Save the data to the file.", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    WModule::properties();
}

