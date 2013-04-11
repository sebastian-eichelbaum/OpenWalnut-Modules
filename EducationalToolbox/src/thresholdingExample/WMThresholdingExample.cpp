//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#include <osg/Geometry>

#include "core/dataHandler/WDataSetScalar.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WMThresholdingExample.xpm"
#include "WMThresholdingExample.h"

WMThresholdingExample::WMThresholdingExample():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMThresholdingExample::~WMThresholdingExample()
{
}

boost::shared_ptr< WModule > WMThresholdingExample::factory() const
{
    return boost::shared_ptr< WModule >( new WMThresholdingExample() );
}

const char** WMThresholdingExample::getXPMIcon() const
{
    return WMThresholdingExample_xpm;
}
const std::string WMThresholdingExample::getName() const
{
    return "[EDU] Thresholding Example";
}

const std::string WMThresholdingExample::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMThresholdingExample::connectors()
{
    m_scalarIC = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "scalarData", "Scalar data." );

    WModule::connectors();
}

void WMThresholdingExample::properties()
{
    // Put the code for your properties here. See "src/modules/template/" for an extensively documented example.

    WModule::properties();
}

void WMThresholdingExample::requirements()
{
}

void WMThresholdingExample::moduleMain()
{
    infoLog() << "Thrsholding example main routine started";

    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_scalarIC->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

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

        // save data behind connectors since it might change during processing
        boost::shared_ptr< WDataSetScalar > scalarData = m_scalarIC->getData();

        if( !scalarData )
        {
            continue;
        }

        m_isovalue->setMin( 0.0 );
        m_isovalue->setMax( 1.0 );
        // m_isovalue->setMin( scalarData->getMin() );
        // m_isovalue->setMax( scalarData->getMax() );

        initOSG( scalarData, m_resolution->get() );

        wge::bindTexture( m_output, scalarData->getTexture(), 0, "u_scalarData" );

        // TODO(math): unbind textures, so we have a clean OSG root node for this module again
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_output );
}
