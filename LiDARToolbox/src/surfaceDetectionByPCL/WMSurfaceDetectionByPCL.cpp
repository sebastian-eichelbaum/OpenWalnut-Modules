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
#include "WMSurfaceDetectionByPCL.xpm"
#include "WMSurfaceDetectionByPCL.h"
#include "WSurfaceDetectorPCL.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMSurfaceDetectionByPCL )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMSurfaceDetectionByPCL::WMSurfaceDetectionByPCL():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMSurfaceDetectionByPCL::~WMSurfaceDetectionByPCL()
{
}

boost::shared_ptr< WModule > WMSurfaceDetectionByPCL::factory() const
{
    return boost::shared_ptr< WModule >( new WMSurfaceDetectionByPCL() );
}

const char** WMSurfaceDetectionByPCL::getXPMIcon() const
{
    return WMSurfaceDetectionByPCL_xpm;
}
const std::string WMSurfaceDetectionByPCL::getName() const
{
    return "Surface Detection by PCL";
}

const std::string WMSurfaceDetectionByPCL::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMSurfaceDetectionByPCL::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_outputPointsGrouped = boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > >(
                new WModuleOutputData< WDataSetPointsGrouped >( shared_from_this(), "Grouped points", "The loaded mesh." ) );

    addConnector( m_outputPointsGrouped );
    WModule::connectors();
}

void WMSurfaceDetectionByPCL::properties()
{
    m_infoNbPoints = m_infoProperties->addProperty( "Points: ", "Input points count.", 0 );
    m_infoRenderTimeMinutes = m_infoProperties->addProperty( "Wall time (min): ", "Time in seconds that the "
                                                            "whole render process took.", 0.0 );
    m_infoPointsPerSecond = m_infoProperties->addProperty( "Points per second: ",
                                                            "The current speed in points per second.", 0.0 );
    m_infoXMin = m_infoProperties->addProperty( "X min.: ", "Minimal x coordinate of all input points.", 0.0 );
    m_infoXMax = m_infoProperties->addProperty( "X max.: ", "Maximal x coordinate of all input points.", 0.0 );
    m_infoYMin = m_infoProperties->addProperty( "Y min.: ", "Minimal y coordinate of all input points.", 0.0 );
    m_infoYMax = m_infoProperties->addProperty( "Y max.: ", "Maximal y coordinate of all input points.", 0.0 );
    m_infoZMin = m_infoProperties->addProperty( "Z min.: ", "Minimal z coordinate of all input points.", 0.0 );
    m_infoZMax = m_infoProperties->addProperty( "Z max.: ", "Maximal z coordinate of all input points.", 0.0 );


    m_reloadData = m_properties->addProperty( "Reload data:",  "Execute", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );
    m_clusterSizeMin = m_properties->addProperty( "Cluster size min.: ",
            "Minimal size of a surface point set.", 50 );
    m_clusterSizeMax = m_properties->addProperty( "Cluster size max.: ",
                "Maximal size of a surface point set.", 1000 * 1000 );
    m_numberOfNeighbours = m_properties->addProperty( "Number of neighbors: ", "Resulting detail depth "
                            "in meters for the octree search tree.", 30 );

    m_smoothnessThresholdDegrees = m_properties->addProperty( "Smoothness threshold: ", "", 3.0 );
    m_smoothnessThresholdDegrees->setMin( 0.0 );
    m_smoothnessThresholdDegrees->setMax( 30 );
    m_curvatureThreshold = m_properties->addProperty( "Curvature Threshold: ", "", 1.0 );


    WModule::properties();
}

void WMSurfaceDetectionByPCL::requirements()
{
}

void WMSurfaceDetectionByPCL::moduleMain()
{
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
        m_moduleState.wait();

        boost::shared_ptr< WDataSetPoints > points = m_input->getData();
        if  ( points )
        {
            WRealtimeTimer timer;
            timer.reset();
            WDataSetPoints::VertexArray inputVerts = points->getVertices();
            size_t count = inputVerts->size()/3;
            setProgressSettings( count );

            WSurfaceDetectorPCL detector = WSurfaceDetectorPCL();
            detector.setClusterSizeRange( m_clusterSizeMin->get(), m_clusterSizeMax->get() );
            detector.setNumberOfNeighbors( m_numberOfNeighbours->get() );
            detector.setSmoothnessThreshold( m_smoothnessThresholdDegrees->get() );
            detector.setCurvatureThreshold( m_curvatureThreshold->get() );
            WQuadTree* boundingBox = new WQuadTree( 16 );

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            for  ( size_t vertex = 0; vertex < count; vertex++)
            {
                float x = inputVerts->at( vertex * 3 );
                float y = inputVerts->at( vertex * 3 + 1 );
                float z = inputVerts->at( vertex * 3 + 2 );

                boundingBox->registerPoint( x, y, z );
            }
            boost::shared_ptr< WDataSetPointsGrouped > outputPcl = detector.detectSurfaces( points );
            m_outputPointsGrouped->updateData( outputPcl );
            m_infoNbPoints->set( count );
            m_infoRenderTimeMinutes->set( timer.elapsed() / 60.0 );
            m_infoPointsPerSecond->set( m_infoRenderTimeMinutes->get() == 0.0 ?m_infoNbPoints->get()
                    :m_infoNbPoints->get() / ( m_infoRenderTimeMinutes->get() * 60.0 ) );
            m_infoXMin->set( boundingBox->getRootNode()->getXMin() );
            m_infoXMax->set( boundingBox->getRootNode()->getXMax() );
            m_infoYMin->set( boundingBox->getRootNode()->getYMin() );
            m_infoYMax->set( boundingBox->getRootNode()->getYMax() );
            m_infoZMin->set( boundingBox->getRootNode()->getValueMin() );
            m_infoZMax->set( boundingBox->getRootNode()->getValueMax() );
            m_progressStatus->finish();
        }
        m_reloadData->set( WPVBaseTypes::PV_TRIGGER_READY, true );
        m_reloadData->get( true );


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
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

void WMSurfaceDetectionByPCL::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
