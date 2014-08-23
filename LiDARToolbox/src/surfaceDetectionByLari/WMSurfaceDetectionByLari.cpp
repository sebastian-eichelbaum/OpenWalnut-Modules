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
#include "WMSurfaceDetectionByLari.xpm"
#include "WMSurfaceDetectionByLari.h"
#include "WLariPointClassifier.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMSurfaceDetectionByLari )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMSurfaceDetectionByLari::WMSurfaceDetectionByLari():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMSurfaceDetectionByLari::~WMSurfaceDetectionByLari()
{
}

boost::shared_ptr< WModule > WMSurfaceDetectionByLari::factory() const
{
    return boost::shared_ptr< WModule >( new WMSurfaceDetectionByLari() );
}

const char** WMSurfaceDetectionByLari::getXPMIcon() const
{
    return WMSurfaceDetectionByLari_xpm;
}
const std::string WMSurfaceDetectionByLari::getName() const
{
    return "Surface Detection by Lari";
}

const std::string WMSurfaceDetectionByLari::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMSurfaceDetectionByLari::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_outputSpatialDomainGroups = boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > >(
            new WModuleOutputData< WDataSetPointsGrouped >( shared_from_this(),
                    "Spatial domain point groups", "//TODO: description" ) );
    m_outputSpatialDomainCategories = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
            new WModuleOutputData< WDataSetPoints >( shared_from_this(),
                    "Spatial domain point categories", "//TODO: description" ) );
    m_outputParameterDomain = boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > >(
            new WModuleOutputData< WDataSetPointsGrouped >( shared_from_this(),
                    "Parameter domain points", "//TODO: description" ) );
    m_outputLeastSquaresPlanes = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
            new WModuleOutputData< WTriangleMesh >( shared_from_this(),
                    "Least Squares Planes", "//TODO: description" ) );

    addConnector( m_outputSpatialDomainGroups );
    addConnector( m_outputSpatialDomainCategories );
    addConnector( m_outputParameterDomain );
    addConnector( m_outputLeastSquaresPlanes );
//    addConnector( m_buildings );
    WModule::connectors();
}

void WMSurfaceDetectionByLari::properties()
{
    m_nbPoints = m_infoProperties->addProperty( "Points: ", "Input points count.", 0 );
    m_infoRenderTimeSeconds = m_infoProperties->addProperty( "Wall time (s): ", "Time in seconds that the "
                                            "whole render process took.", 0.0 );
    m_infoPointsPerSecond = m_infoProperties->addProperty( "Points per second: ",
                                            "The current speed in points per second.", 0.0 );
    m_xMin = m_infoProperties->addProperty( "X min.: ", "Minimal x coordinate of all input points.", 0.0 );
    m_xMax = m_infoProperties->addProperty( "X max.: ", "Maximal x coordinate of all input points.", 0.0 );
    m_yMin = m_infoProperties->addProperty( "Y min.: ", "Minimal y coordinate of all input points.", 0.0 );
    m_yMax = m_infoProperties->addProperty( "Y max.: ", "Maximal y coordinate of all input points.", 0.0 );
    m_zMin = m_infoProperties->addProperty( "Z min.: ", "Minimal z coordinate of all input points.", 0.0 );
    m_zMax = m_infoProperties->addProperty( "Z max.: ", "Maximal z coordinate of all input points.", 0.0 );

    m_reloadData = m_properties->addProperty( "Reload data:",  "Execute", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    double minRange = 0.01;
    m_segmentationPlaneDistance = m_properties->addProperty( "Plane distance", "", .7, m_propCondition );
    m_segmentationMaxAngleDegrees = m_properties->addProperty( "Plane angle", "", 7.0, m_propCondition );
    m_numberPointsK = m_properties->addProperty( "Number points K=", "", 12, m_propCondition );
    m_maxPointDistanceR = m_properties->addProperty( "Max point distance r=", "", 1.0, m_propCondition );

    m_applyBoundaryDetection = m_properties->addProperty( "Detect boundries: ", "", true, m_propCondition );

    m_squareWidth = m_properties->addProperty( "Plane outline size: ", "", 0.2, m_propCondition );

    //TODO(aschwarzkopf): Resolve changing the thread count during execution.
    m_cpuThreadCount = m_properties->addProperty( "CPU threads: ", "", 8, m_propCondition );
    m_cpuThreadCount->setMin( 4 );
    m_cpuThreadCount->setMax( 24 );

    m_planarGroup = m_properties->addPropertyGroup( "Planar feature properties",
                                            "All conditions must be met to detect as a surface." );
    m_surfaceNLambda1Min = m_planarGroup->addProperty( "N Lambda 1 >=", "", 0.3, m_propCondition );
    m_surfaceNLambda1Max = m_planarGroup->addProperty( "N Lambda 1 <", "", 0.7, m_propCondition );
    m_surfaceNLambda2Min = m_planarGroup->addProperty( "N Lambda 2 >=", "", 0.0, m_propCondition );
    m_surfaceNLambda2Max = m_planarGroup->addProperty( "N Lambda 2 <", "", 1.0, m_propCondition );
    m_surfaceNLambda3Min = m_planarGroup->addProperty( "N Lambda 3 >=", "", 0.0, m_propCondition );
    m_surfaceNLambda3Max = m_planarGroup->addProperty( "N Lambda 3 <", "", 1.0, m_propCondition );

    m_surfaceNLambda1Min->setMin( 0.0 );
    m_surfaceNLambda1Min->setMax( 1.0 - minRange );
    m_surfaceNLambda1Max->setMin( m_surfaceNLambda1Min->get() );
    m_surfaceNLambda1Max->setMax( 1.0 );

    m_surfaceNLambda2Min->setMin( 0.0 );
    m_surfaceNLambda2Min->setMax( 1.0 - minRange );
    m_surfaceNLambda2Max->setMin( m_surfaceNLambda2Min->get() );
    m_surfaceNLambda2Max->setMax( 1.0 );

    m_surfaceNLambda3Min->setMin( 0.0 );
    m_surfaceNLambda3Min->setMax( 1.0 - minRange );
    m_surfaceNLambda3Max->setMin( m_surfaceNLambda3Min->get() );
    m_surfaceNLambda3Max->setMax( 1.0 );

    m_cylindricalGroup = m_properties->addPropertyGroup( "Linear and Cylindrical feature properties",
                                            "All conditions must be met to detect as a surface." );
    m_cylNLambda1Min = m_cylindricalGroup->addProperty( "N Lambda 1 >=", "", 0.8, m_propCondition );
    m_cylNLambda1Max = m_cylindricalGroup->addProperty( "N Lambda 1 <", "", 1.0, m_propCondition );
    m_cylNLambda2Min = m_cylindricalGroup->addProperty( "N Lambda 2 >=", "", 0.0, m_propCondition );
    m_cylNLambda2Max = m_cylindricalGroup->addProperty( "N Lambda 2 <", "", 1.0, m_propCondition );
    m_cylNLambda3Min = m_cylindricalGroup->addProperty( "N Lambda 3 >=", "", 0.0, m_propCondition );
    m_cylNLambda3Max = m_cylindricalGroup->addProperty( "N Lambda 3 <", "", 1.0, m_propCondition );

    m_cylNLambda1Min->setMin( 0.0 );
    m_cylNLambda1Min->setMax( 1.0 - minRange );
    m_cylNLambda1Max->setMin( m_cylNLambda1Min->get() );
    m_cylNLambda1Max->setMax( 1.0 );

    m_cylNLambda2Min->setMin( 0.0 );
    m_cylNLambda2Min->setMax( 1.0 - minRange );
    m_cylNLambda2Max->setMin( m_cylNLambda2Min->get() );
    m_cylNLambda2Max->setMax( 1.0 );

    m_cylNLambda3Min->setMin( 0.0 );
    m_cylNLambda3Min->setMax( 1.0 - minRange );
    m_cylNLambda3Max->setMin( m_cylNLambda3Min->get() );
    m_cylNLambda3Max->setMax( 1.0 );



    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.


    WModule::properties();
}

void WMSurfaceDetectionByLari::requirements()
{
}

void WMSurfaceDetectionByLari::moduleMain()
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

        m_surfaceNLambda1Max->setMin( m_surfaceNLambda1Min->get() );
        m_surfaceNLambda2Max->setMin( m_surfaceNLambda2Min->get() );
        m_surfaceNLambda3Max->setMin( m_surfaceNLambda3Min->get() );
        m_cylNLambda1Max->setMin( m_cylNLambda1Min->get() );
        m_cylNLambda2Max->setMin( m_cylNLambda2Min->get() );
        m_cylNLambda3Max->setMin( m_cylNLambda3Min->get() );

        boost::shared_ptr< WDataSetPoints > points = m_input->getData();
        if  ( points )
        {
            WRealtimeTimer timer;
            timer.reset();
            WDataSetPoints::VertexArray inputVerts = points->getVertices();
            size_t count = inputVerts->size()/3;
            setProgressSettings( count );

            WQuadTree* boundingBox = new WQuadTree( pow( 2.0, 3 ) );

            vector<WSpatialDomainKdPoint*>* inputPoints = new vector<WSpatialDomainKdPoint*>();
            for  ( size_t vertex = 0; vertex < count; vertex++)
            {
                float x = inputVerts->at( vertex*3 );
                float y = inputVerts->at( vertex*3+1 );
                float z = inputVerts->at( vertex*3+2 );
                inputPoints->push_back( new WSpatialDomainKdPoint( x, y, z ) );
                boundingBox->registerPoint( x, y, z );
                m_progressStatus->increment( 1 );
            }

            WLariPointClassifier* classifier = new WLariPointClassifier();
            classifier->setNumberPointsK( m_numberPointsK->get() );
            classifier->setMaxPointDistanceR( m_maxPointDistanceR->get() );
            classifier->setCpuThreadCount( m_cpuThreadCount->get() );
            classifier->setPlanarNLambdaRange( 0, m_surfaceNLambda1Min->get(), m_surfaceNLambda1Max->get() );
            classifier->setPlanarNLambdaRange( 1, m_surfaceNLambda2Min->get(), m_surfaceNLambda2Max->get() );
            classifier->setPlanarNLambdaRange( 2, m_surfaceNLambda3Min->get(), m_surfaceNLambda3Max->get() );
            classifier->setCylindricalNLambdaRange( 0, m_cylNLambda1Min->get(), m_cylNLambda1Max->get() );
            classifier->setCylindricalNLambdaRange( 1, m_cylNLambda2Min->get(), m_cylNLambda2Max->get() );
            classifier->setCylindricalNLambdaRange( 2, m_cylNLambda3Min->get(), m_cylNLambda3Max->get() );
            classifier->analyzeData( inputPoints );

            WLariBruteforceClustering* clustering = new WLariBruteforceClustering( classifier );
            clustering->setSegmentationSettings( m_segmentationMaxAngleDegrees->get(), m_segmentationPlaneDistance->get() );
            clustering->setCpuThreadCount( m_cpuThreadCount->get() );
            clustering->detectClustersByBruteForce();

            WLariBoundaryDetector* boundaryDetector = new WLariBoundaryDetector();
            if( m_applyBoundaryDetection->get() )
            {
                boundaryDetector->setMaxPointDistanceR( m_maxPointDistanceR->get() );
                boundaryDetector->detectBoundaries( classifier->getParameterDomain() );
            }

            WLariOutliner* outliner = new WLariOutliner( classifier );
            cout << "Outlining spatial domain" << endl;
            m_outputSpatialDomainGroups->updateData( outliner->outlineSpatialDomainGroups() );
            cout << "Outlining parameter domain" << endl;
            m_outputSpatialDomainCategories->updateData( outliner->outlineSpatialDomainCategories() );
            cout << "Outlining parameter domain" << endl;
            m_outputParameterDomain->updateData( outliner->outlineParameterDomain() );
            cout << "Outlining point planes" << endl;
            m_outputLeastSquaresPlanes->updateData( outliner->outlineLeastSquaresPlanes( m_squareWidth->get() ) );
            cout << "Outlining done" << endl;

            delete classifier;
            delete inputPoints;
            delete clustering;
            delete outliner;
            delete boundaryDetector;

            m_nbPoints->set( count );
            m_infoPointsPerSecond->set( m_infoRenderTimeSeconds->get() == 0.0 ?m_nbPoints->get()
                    :m_nbPoints->get() / m_infoRenderTimeSeconds->get() );
            m_xMin->set( boundingBox->getRootNode()->getXMin() );
            m_xMax->set( boundingBox->getRootNode()->getXMax() );
            m_yMin->set( boundingBox->getRootNode()->getYMin() );
            m_yMax->set( boundingBox->getRootNode()->getYMax() );
            m_zMin->set( boundingBox->getRootNode()->getElevationMin() );
            m_zMax->set( boundingBox->getRootNode()->getElevationMax() );
            delete boundingBox;
            m_infoRenderTimeSeconds->set( timer.elapsed() );
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

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMSurfaceDetectionByLari::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
