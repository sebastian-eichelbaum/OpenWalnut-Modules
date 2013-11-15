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
#include "WMBuildingsDetection.xpm"
#include "WMBuildingsDetection.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMBuildingsDetection )

WMBuildingsDetection::WMBuildingsDetection():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMBuildingsDetection::~WMBuildingsDetection()
{
}

boost::shared_ptr< WModule > WMBuildingsDetection::factory() const
{
    return boost::shared_ptr< WModule >( new WMBuildingsDetection() );
}

const char** WMBuildingsDetection::getXPMIcon() const
{
    return WMBuildingsDetection_xpm;
}
const std::string WMBuildingsDetection::getName() const
{
    return "[ALPHA] Buildings Detection hahahahah";
}

const std::string WMBuildingsDetection::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMBuildingsDetection::connectors()
{
	m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

	m_output = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
                new WModuleOutputData< WTriangleMesh >( shared_from_this(), "output", "The loaded mesh." ) );

    addConnector( m_output );
    WModule::connectors();
}

void WMBuildingsDetection::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
	m_stubSize = m_properties->addProperty( "Cutoff threshold: ",
                        "Value that will be replaced with a dummy value.", 2.0, m_propCondition );
	m_stubSize->setMin( 0.0 );
	m_stubSize->setMax( 3.0 );
    WModule::properties();
}

void WMBuildingsDetection::requirements()
{
}

void WMBuildingsDetection::moduleMain()
{
    infoLog() << "Thrsholding example main routine started";

    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_input->getDataChangedCondition() );
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

		boost::shared_ptr< WDataSetPoints > points = m_input->getData();
		std::cout << "Execute cycle\r\n";
        if  ( points )
        {
        	WDataSetPoints::VertexArray verts = points->getVertices();
        	size_t count = verts->size()/3;

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            float a = m_stubSize->get();
            setProgressSettings( count );
            for  ( size_t i=0; i<count; i++)
			{
            	float x = verts->at(i*3);
            	float y = verts->at(i*3+1);
            	float z = verts->at(i*3+2);
//            	std::cout << "Point at\t" << x << "\t" << y << "\t" << z << "\r\n";
				tmpMesh->addVertex(0+x, 0+y, 0+z);
				tmpMesh->addVertex(a+x, 0+y, 0+z);
				tmpMesh->addVertex(0+x, a+y, 0+z);
				tmpMesh->addVertex(0+x, 0+y, a+z);
				size_t o = i*4;
				tmpMesh->addTriangle(0+o, 2+o, 1+o);
				tmpMesh->addTriangle(0+o, 1+o, 3+o);
				tmpMesh->addTriangle(0+o, 3+o, 2+o);
				tmpMesh->addTriangle(1+o, 2+o, 3+o);
				m_progressStatus->increment( 1 );
            }
			m_output->updateData(tmpMesh);
			m_progressStatus->finish();
        }

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPoints > points2 = m_input->getData();
        if  ( !points2 )
        {
            continue;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMBuildingsDetection::setProgressSettings( size_t steps )
{
	m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
