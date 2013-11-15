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

//Brings me at least very few forward: http://osgeo-org.1560.x6.nabble.com/Liblas-devel-c-liblas-problem-td4919064.html

#include <string>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include <osg/Geometry>
#include "core/kernel/WModule.h"

#include "core/dataHandler/WDataSetScalar.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WMReadLAS.xpm"
#include "WMReadLAS.h"
#include "WLasTool.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMReadLAS )

WMReadLAS::WMReadLAS():
    WModule(),
    m_propCondition( new WCondition() )
{
    reader = laslibb::WLasTool( m_progress );
}

WMReadLAS::~WMReadLAS()
{
}

boost::shared_ptr< WModule > WMReadLAS::factory() const
{
    return boost::shared_ptr< WModule >( new WMReadLAS() );
}

const char** WMReadLAS::getXPMIcon() const
{
    return WMReadLAS_xpm;
}
const std::string WMReadLAS::getName() const
{
    return "[ALPHA] Read LAS";
}

const std::string WMReadLAS::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMReadLAS::connectors()
{
    m_output = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                		shared_from_this(), "points", "The loaded points." ) );

    addConnector( m_output );
    WModule::connectors();
}

void WMReadLAS::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    m_cutoffThreshold = m_properties->addProperty( "Cutoff threshold: ",
                        "Value that will be replaced with a dummy value.", 100, m_propCondition );
    m_cutoffThreshold->setMin( 0 );
    m_cutoffThreshold->setMax( 100 );
    m_cutoffThresholdCount = m_properties->addProperty( "cutoff; kept voxels: ", "voxel count.", std::string( "--; --" ) );
    m_cutoffThresholdCount->setPurpose( PV_PURPOSE_INFORMATION );
    WModule::properties();
}

void WMReadLAS::requirements()
{
}

void WMReadLAS::moduleMain()
{
    infoLog() << "Thrsholding example main routine started";

    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );



    // main loop
    while( !m_shutdownFlag() )
    {
        //infoLog() << "Waiting ...";
        m_moduleState.wait();

        boost::shared_ptr< WDataSetPoints > tmpPointSet = reader.getPoints();
        WDataSetPoints::VertexArray points = tmpPointSet->getVertices();
        WDataSetPoints::ColorArray colors = tmpPointSet->getColors();
        m_output->updateData( tmpPointSet );
        std::cout << "placing " << points->size() << " point things\r\n";
        std::cout << "placing " << colors->size() << " color things\r\n";
        std::cout << "LAS read";

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
