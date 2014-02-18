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
#include "WMElevationImageExport.xpm"
#include "WMElevationImageExport.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMElevationImageExport )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMElevationImageExport::WMElevationImageExport():
    WModule(),
    m_propCondition( new WCondition() )
{
    m_elevationImage = new WQuadTree( 0 );
}

WMElevationImageExport::~WMElevationImageExport()
{
}

boost::shared_ptr< WModule > WMElevationImageExport::factory() const
{
    return boost::shared_ptr< WModule >( new WMElevationImageExport() );
}

const char** WMElevationImageExport::getXPMIcon() const
{
    return WMElevationImageExport_xpm;
}
const std::string WMElevationImageExport::getName() const
{
    return "Elevation image export";
}

const std::string WMElevationImageExport::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMElevationImageExport::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_pointGroups = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(),
            "Building groups", "Building groups that will be highlighted in the elevation image" );

    m_elevationImageDisplay = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
            new WModuleOutputData< WTriangleMesh >( shared_from_this(), "Elevation image preview",
                    "The previewable elevation image as triangle mesh" ) );

    addConnector( m_elevationImageDisplay );
    WModule::connectors();
}


//TODO(schwarzkopf): Feature - Elevation image preview as WTriangleMesh with option of Z schift in view.
void WMElevationImageExport::properties()
{
    m_nbPoints = m_infoProperties->addProperty( "Points: ", "Input points count.", 0 );
    m_xMin = m_infoProperties->addProperty( "X min.: ", "Minimal x coordinate of all input points.", 0.0 );
    m_xMax = m_infoProperties->addProperty( "X max.: ", "Maximal x coordinate of all input points.", 0.0 );
    m_yMin = m_infoProperties->addProperty( "Y min.: ", "Minimal y coordinate of all input points.", 0.0 );
    m_yMax = m_infoProperties->addProperty( "Y max.: ", "Maximal y coordinate of all input points.", 0.0 );
    m_zMin = m_infoProperties->addProperty( "Z min.: ", "Minimal z coordinate of all input points.", 0.0 );
    m_zMax = m_infoProperties->addProperty( "Z max.: ", "Maximal z coordinate of all input points.", 0.0 );


    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    m_detailDepth = m_properties->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0, m_propCondition );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_properties->addProperty( "Detail Depth meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", 1.0  );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );


    boost::shared_ptr< WItemSelection > imageModes( boost::shared_ptr< WItemSelection >( new WItemSelection() ) );
    imageModes->addItem( "Minimals", "Minimal elevation values." );
    imageModes->addItem( "Maximals", "Maximal elevation values." );
    imageModes->addItem( "Point count", "Store point count to each pixel." );
    m_elevImageMode = m_properties->addProperty( "Color mode", "Choose one of the available colorings.",
                                                 imageModes->getSelectorFirst(), m_propCondition );
    WPropertyHelper::PC_SELECTONLYONE::addTo( m_elevImageMode );
    m_minElevImageZ = m_properties->addProperty( "Min. Z value: ",
                    "Minimal Z value where image intensities begin to rise. Always watch in the "
                    "info the minimal Z value and enter a similar value.", 0.0, m_propCondition );
    m_intensityIncreasesPerMeter = m_properties->addProperty( "Increases per meter: ",
                    "The bitmap has a range of possible color intensities. This field determines "
                    "how many increases per meter are done.", 10.0, m_propCondition );
    m_elevationImageExportablePath = m_properties->addProperty( "Elev. image:",
                            "Target file path of the exportable elevation image *.bmp file",
                            WPathHelper::getAppPath() );
    m_exportTriggerProp = m_properties->addProperty( "Write: ",  "Export elevation image", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );


    WModule::properties();
}

void WMElevationImageExport::requirements()
{
}

void WMElevationImageExport::moduleMain()
{
    infoLog() << "Thrsholding example main routine started";

    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_input->getDataChangedCondition() );
    m_moduleState.add( m_pointGroups->getDataChangedCondition() );
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
        WItemSelector elevImageModeSelector = m_elevImageMode->get();
        if( points )
        {
            WDataSetPoints::VertexArray verts = points->getVertices();
            WElevationImageOutliner* m_elevationImageOutliner = new WElevationImageOutliner();
            size_t count = verts->size()/3;
            setProgressSettings( count );

            m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) );
            m_elevationImage = new WQuadTree( m_detailDepthLabel->get() );

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            for( size_t vertex = 0; vertex < count; vertex++)
            {
                float x = verts->at( vertex*3 );
                float y = verts->at( vertex*3+1 );
                float z = verts->at( vertex*3+2 );
                m_elevationImage->registerPoint( x, y, z );
                m_progressStatus->increment( 1 );
            }
            m_nbPoints->set( count );
            m_xMin->set( m_elevationImage->getRootNode()->getXMin() );
            m_xMax->set( m_elevationImage->getRootNode()->getXMax() );
            m_yMin->set( m_elevationImage->getRootNode()->getYMin() );
            m_yMax->set( m_elevationImage->getRootNode()->getYMax() );
            m_zMin->set( m_elevationImage->getRootNode()->getElevationMin() );
            m_zMax->set( m_elevationImage->getRootNode()->getElevationMax() );
            m_elevationImageOutliner->setExportElevationImageSettings(
                    m_minElevImageZ->get( true ), m_intensityIncreasesPerMeter->get() );
            m_elevationImageOutliner->importElevationImage( m_elevationImage,
                    elevImageModeSelector.getItemIndexOfSelected( 0 ) );
            m_elevationImageOutliner->highlightBuildingGroups( m_pointGroups->getData() );
            if( m_exportTriggerProp->get( true ) )
            {
                WBmpImage* image = new WBmpImage( 1, 1 );
                image->setExportElevationImageSettings(
                    m_minElevImageZ->get( true ), m_intensityIncreasesPerMeter->get() );
                image->importElevationImage( m_elevationImage,
                        elevImageModeSelector.getItemIndexOfSelected( 0 ) );
                image->highlightBuildingGroups( m_pointGroups->getData(), m_elevationImage );

                WBmpSaver::saveImage( image, m_elevationImageExportablePath->get().c_str() );
            }
            m_elevationImageDisplay->updateData( m_elevationImageOutliner->getOutputMesh() );
            m_exportTriggerProp->set( WPVBaseTypes::PV_TRIGGER_READY, true );
            m_progressStatus->finish();
        }


        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPoints > points2 = m_input->getData();
        if( !points2 )
        {
            continue;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMElevationImageExport::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
