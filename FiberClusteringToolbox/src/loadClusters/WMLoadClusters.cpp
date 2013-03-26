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

#include "WReaderFiberClusters.h"

#include "WMLoadClusters.h"

WMLoadClusters::WMLoadClusters()
{
}

WMLoadClusters::~WMLoadClusters()
{
}

boost::shared_ptr< WModule > WMLoadClusters::factory() const
{
    return boost::shared_ptr< WModule >( new WMLoadClusters );
}

const char** WMLoadClusters::getXPMIcon() const
{
    return NULL;
}

const std::string WMLoadClusters::getDescription() const
{
    return "Loads fiber clusters from a file.";
}

const std::string WMLoadClusters::getName() const
{
    return "Load clusters";
}

void WMLoadClusters::connectors()
{
    m_output = boost::shared_ptr< WModuleOutputData< WDataSetFiberClustering > >(
        new WModuleOutputData< WDataSetFiberClustering >( shared_from_this(), "out", "The loaded fiber cluster data." ) );
    addConnector( m_output );

    WModule::connectors();
}

void WMLoadClusters::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_propCondition );

    ready();

    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        if( m_shutdownFlag() )
        {
            break;
        }

        if( m_propFilename->changed() )
        {
            std::string fn = m_propFilename->get( true ).string();
            if( fn != "/" )
            {
                WReaderFiberClusters clusterReader( fn );
                boost::shared_ptr< WDataSetFiberClustering > clusters = clusterReader.readClusters();

                if( clusters )
                {
                    m_output->updateData( clusters );
                }
            }
        }
    }
}

void WMLoadClusters::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition );

    m_propFilename = m_properties->addProperty( "File path", "The file to load the clusters from.", boost::filesystem::path( "/" ), m_propCondition );

    WModule::properties();
}

