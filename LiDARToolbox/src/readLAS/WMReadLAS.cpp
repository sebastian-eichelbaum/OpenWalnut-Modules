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

#include <fstream>
#include <iostream>
#include <limits>

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
#include "WLasReader.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMReadLAS )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMReadLAS::WMReadLAS():
    WModule(),
    m_propCondition( new WCondition() )
{
    reader = laslibb::WLasReader( m_progress );
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
    return "Read LAS";
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
    m_lasFile = m_properties->addProperty( "LiDAR file", "", WPathHelper::getAppPath() );
    WPropertyHelper::PC_PATHEXISTS::addTo( m_lasFile );

    m_reloadData = m_properties->addProperty( "Reload data:",  "Refresh", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    m_outputDataWidth = m_properties->addProperty( "Data set width: ",
                            "Outpt area size which is of the area input*input meters^2.",
                            200, m_propCondition );
    m_outputDataWidth->setMin( 0 );

    m_sliderX = m_properties->addProperty( "Scope selection X: ",
                            "X range will be drawn between input and input+'Data set "
                            "width'.", 0, m_propCondition );

    m_sliderY = m_properties->addProperty( "Scope selection Y: ",
                            "YY range will be drawn between input and input+'Data set "
                            "width'.", 0, m_propCondition );

    m_translateDataToCenter = m_properties->addProperty( "Translate to center: ",
                            "Translates X and Y coordinates to center. Minimal and maximal "
                            "possible coordinates are -/+'Data set width'/2."
                            ".", true, m_propCondition );
    m_contrast = m_properties->addProperty( "Contrast: ",
                            "This is the value that multiplies the input colors before assigning to the output. "
                            "Note that the output has the range between 0.0 and 1.0.\r\nHint: Look ath the intensity "
                            "maximum param in the information tab of the ReadLAS plugin.", 0.005, m_propCondition );

    m_nbVertices = m_infoProperties->addProperty( "Points", "The number of vertices in the loaded scan.", 0 );
    m_nbVertices->setMax( std::numeric_limits< int >::max() );
    m_xMin = m_infoProperties->addProperty( "X min.: ", "Minimal x coordinate of all input points.", 0.0 );
    m_xMax = m_infoProperties->addProperty( "X max.: ", "Maximal x coordinate of all input points.", 0.0 );
    m_yMin = m_infoProperties->addProperty( "Y min.: ", "Minimal y coordinate of all input points.", 0.0 );
    m_yMax = m_infoProperties->addProperty( "Y max.: ", "Maximal y coordinate of all input points.", 0.0 );
    m_zMin = m_infoProperties->addProperty( "Z min.: ", "Minimal z coordinate of all input points.", 0.0 );
    m_zMax = m_infoProperties->addProperty( "Z max.: ", "Maximal z coordinate of all input points.", 0.0 );
    m_intensityMin = m_infoProperties->addProperty( "Intensity min.: ", "Minimal intensity of all input points.", 0.0 );
    m_intensityMax = m_infoProperties->addProperty( "Intensity max.: ", "Maximal intensity of all input points.", 0.0 );

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
        m_moduleState.wait();

        reader.setInputFilePath( m_lasFile->get().c_str() );
        try
        {
            reader.setDataSetRegion( m_sliderX->get( true ),
                                     m_sliderY->get( true ),
                                     m_outputDataWidth->get( true ) );
            reader.setTranslateToCenter( m_translateDataToCenter->get( true ) );
            reader.setContrast( m_contrast->get() );
            boost::shared_ptr< WDataSetPoints > tmpPointSet = reader.getPoints();
            WDataSetPoints::VertexArray points = tmpPointSet->getVertices();
            WDataSetPoints::ColorArray colors = tmpPointSet->getColors();
            m_nbVertices->set( tmpPointSet->size() );
            m_xMin->set( reader.getXMin() );
            m_xMax->set( reader.getXMax() );
            m_yMin->set( reader.getYMin() );
            m_yMax->set( reader.getYMax() );
            m_zMin->set( reader.getZMin() );
            m_zMax->set( reader.getZMax() );
            m_intensityMin->set( reader.getIntensityMin() );
            m_intensityMax->set( reader.getIntensityMax() );

            m_output->updateData( tmpPointSet );
        } catch( ... )
        {
        }
        refreshScrollBars();

        m_reloadData->set( WPVBaseTypes::PV_TRIGGER_READY, true );
        m_reloadData->get( true );

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMReadLAS::refreshScrollBars()
{
    size_t dataWidth = ( m_outputDataWidth->get() + 1 ) / 2;
    m_outputDataWidth->set( dataWidth = dataWidth * 2 );

    if  ( m_outputDataWidth == 0 ) return;

    size_t dividend = reader.getXMin() / dataWidth;
    m_sliderX->setMin( dividend * dataWidth );
    dividend = reader.getXMax() / dataWidth;
    m_sliderX->setMax( dividend * dataWidth );

    dividend = reader.getYMin() / dataWidth;
    m_sliderY->setMin( dividend * dataWidth );
    dividend = reader.getYMax() / dataWidth;
    m_sliderY->setMax( dividend * dataWidth );

    m_sliderX->get( true );
    m_sliderY->get( true );
}
