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
#include "WMBuildingsDetection.xpm"
#include "WMBuildingsDetection.h"
#include "WBuildingDetector.h"
#include "../common/datastructures/octree/WOctree.h"

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
    return "Buildings Detection";
}

const std::string WMBuildingsDetection::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMBuildingsDetection::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_outputPointsGrouped = boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > >(
                new WModuleOutputData< WDataSetPointsGrouped >( shared_from_this(), "Grouped points", "The loaded mesh." ) );

    addConnector( m_outputPointsGrouped );
//    addConnector( m_buildings );
    WModule::connectors();
}

void WMBuildingsDetection::properties()
{
    m_nbPoints = m_infoProperties->addProperty( "Points: ", "Input points count.", 0 );
    m_xMin = m_infoProperties->addProperty( "X min.: ", "Minimal x coordinate of all input points.", 0.0 );
    m_xMax = m_infoProperties->addProperty( "X max.: ", "Maximal x coordinate of all input points.", 0.0 );
    m_yMin = m_infoProperties->addProperty( "Y min.: ", "Minimal y coordinate of all input points.", 0.0 );
    m_yMax = m_infoProperties->addProperty( "Y max.: ", "Maximal y coordinate of all input points.", 0.0 );
    m_zMin = m_infoProperties->addProperty( "Z min.: ", "Minimal z coordinate of all input points.", 0.0 );
    m_zMax = m_infoProperties->addProperty( "Z max.: ", "Maximal z coordinate of all input points.", 0.0 );


    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.

    m_reloadData = m_properties->addProperty( "Reload data:",  "Execute", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );
    m_detailDepth = m_properties->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0 );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_properties->addProperty( "Pixel width meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", pow( 2.0, m_detailDepth->get() ) * 2.0 );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );

    m_minSearchDetailDepth = m_properties->addProperty( "Detail Depth min. search: ",
            "Main building detection setting.\r\n"
            "Resolution of the relative minimum search image. Use only numbers depictable by 2^n "
            "where n can also be 0 or below. The bigger the pixels the greater are the areas "
            "searched from an examined X/Y area", 3 );
    m_minSearchDetailDepth->setMin( 2 );
    m_minSearchDetailDepth->setMax( 6 );
    m_minSearchCutUntilAbove = m_properties->addProperty( "Cut until above min: ", "Main building detection setting.\r\n"
            "Height that must exceed above an relative minimum to recognize it as a building pixel.", 4.0 );
    m_minSearchCutUntilAbove->setMin( 2.0 );
    m_minSearchCutUntilAbove->setMax( 20.0 );


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
//        std::cout << "Execute cycle\r\n";
        if  ( points )
        {
            WDataSetPoints::VertexArray inputVerts = points->getVertices();
            WDataSetPoints::ColorArray inputColors = points->getColors();
            WDataSetPointsGrouped::VertexArray outputVerts(
                    new WDataSetPointsGrouped::VertexArray::element_type() );
            WDataSetPointsGrouped::ColorArray outputColors(
                    new WDataSetPointsGrouped::ColorArray::element_type() );
            WDataSetPointsGrouped::GroupArray outputGroups(
                    new WDataSetPointsGrouped::GroupArray::element_type() );
            size_t count = inputVerts->size()/3;
            setProgressSettings( count );

            WBuildingDetector detector = WBuildingDetector();
            detector.setDetectionParams( m_detailDepth->get(), m_minSearchDetailDepth->get(),
                    m_minSearchCutUntilAbove->get() );
            detector.detectBuildings( points );
            WOctree* buildingGroups = detector.getBuildingGroups();

            m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) * 2.0 );
            WQuadTree* boundingBox = new WQuadTree( pow( 2.0, m_detailDepth->get() ) );

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            for  ( size_t vertex = 0; vertex < count; vertex++)
            {
                float x = inputVerts->at( vertex*3 );
                float y = inputVerts->at( vertex*3+1 );
                float z = inputVerts->at( vertex*3+2 );

                boundingBox->registerPoint( x, y, z );
                WOctNode* buildingVoxel = buildingGroups->getLeafNode( x, y, z );
                if( buildingVoxel != 0 )
                {
                    m_progressStatus->increment( 1 );
                    outputVerts->push_back( x );
                    outputVerts->push_back( y );
                    outputVerts->push_back( z );
                    for( size_t index = vertex*3; index < vertex*3+3; index++ )
                        outputColors->push_back( inputColors->at( index ) );
                    outputGroups->push_back( buildingVoxel->getGroupNr() );
                }
            }
            if( outputVerts->size() == 0)
            {
                for( size_t dimension = 0; dimension < 3; dimension++ )
                    outputVerts->push_back( 0.0 );
                for( size_t colorChannel = 0; colorChannel < 3; colorChannel++ )
                    outputColors->push_back( 0.0 );
                outputGroups->push_back( 0 );
            }
            boost::shared_ptr< WDataSetPointsGrouped > output(
                    new WDataSetPointsGrouped( outputVerts, outputColors, outputGroups ) );
            m_outputPointsGrouped->updateData( output );

            m_nbPoints->set( count );
            m_xMin->set( boundingBox->getRootNode()->getXMin() );
            m_xMax->set( boundingBox->getRootNode()->getXMax() );
            m_yMin->set( boundingBox->getRootNode()->getYMin() );
            m_yMax->set( boundingBox->getRootNode()->getYMax() );
            m_zMin->set( boundingBox->getRootNode()->getValueMin() );
            m_zMax->set( boundingBox->getRootNode()->getValueMax() );
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

void WMBuildingsDetection::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
