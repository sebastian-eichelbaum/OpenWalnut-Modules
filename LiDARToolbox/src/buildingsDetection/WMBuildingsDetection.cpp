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

#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include <vector>

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
#include "structure/WOTree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMBuildingsDetection )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMBuildingsDetection::WMBuildingsDetection():
    WModule(),
    m_propCondition( new WCondition() )
{
    m_tree = new WOTree( 0 );
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
    return "Buildings Detection";
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
//    addConnector( m_buildings );
    WModule::connectors();
}

void WMBuildingsDetection::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    m_stubSize = m_properties->addProperty( "Stub size: ",
                            "Size of tetraeders that are used to depict points.", 0.6, m_propCondition );
    m_stubSize->setMin( 0.0 );
    m_stubSize->setMax( 3.0 );
    m_contrast = m_properties->addProperty( "Contrast: ",
                            "Color intensity multiplier.", 2.0, m_propCondition );
    m_detailDepth = m_properties->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0, m_propCondition );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_properties->addProperty( "Detail Depth meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", 1.0, m_propCondition  );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );
    m_showTrianglesInsteadOfOctreeCubes = m_properties->addProperty( "Triangles instead of cubes: ",
                            "Depicting the input data set points showing the point outline "
                            "instead of regions depicted as cubes that cover existing points. "
                            "Enabling this option you must have 32GB RAM depicting a 400MB"
                            "las file.", false, m_propCondition );

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
            m_tree = new WOTree( m_detailDepthLabel->get( true ) );
            WDataSetPoints::VertexArray verts = points->getVertices();
            WDataSetPoints::ColorArray colors = points->getColors();
            size_t count = verts->size()/3;

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            float a = m_stubSize->get();
            float contrast = m_contrast->get();
            setProgressSettings( count );
            for  ( size_t vertex = 0; vertex < count; vertex++)
            {
                float x = verts->at( vertex*3 );
                float y = verts->at( vertex*3+1 );
                float z = verts->at( vertex*3+2 );
                if  ( m_showTrianglesInsteadOfOctreeCubes->get( true ) )
                {
                    float r = colors->at( vertex*3 );
                    float g = colors->at( vertex*3+1 );
                    float b = colors->at( vertex*3+2 );
                    osg::Vec4f* color = new osg::Vec4f(
                            r/400.0f*contrast, g/400.0f*contrast, b/400.0f*contrast, 1.0f );
                    tmpMesh->addVertex( 0+x, 0+y, 0+z );
                    tmpMesh->addVertex( a+x, 0+y, 0+z );
                    tmpMesh->addVertex( 0+x, a+y, 0+z );
                    tmpMesh->addVertex( 0+x, 0+y, a+z );
                    size_t body = vertex*4;
                    tmpMesh->addTriangle( 0+body, 2+body, 1+body );
                    tmpMesh->addTriangle( 0+body, 1+body, 3+body );
                    tmpMesh->addTriangle( 0+body, 3+body, 2+body );
                    tmpMesh->addTriangle( 1+body, 2+body, 3+body );
                    tmpMesh->setVertexColor( body, *color );
                    tmpMesh->setVertexColor( body+1, *color );
                    tmpMesh->setVertexColor( body+2, *color );
                    tmpMesh->setVertexColor( body+3, *color );
                }
                m_progressStatus->increment( 1 );
                m_tree->registerPoint( x, y, z );
            }
            m_output->updateData( m_showTrianglesInsteadOfOctreeCubes->get( true )
                    ?tmpMesh :m_tree->getOutline( ) );
            m_progressStatus->finish();
        }

        m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) );
        std::cout << "this is WOTree " << std::endl;

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
