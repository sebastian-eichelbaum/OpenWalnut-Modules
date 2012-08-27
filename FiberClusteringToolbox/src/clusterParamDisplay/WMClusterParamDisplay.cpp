//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#include <set>
#include <string>

#include "core/common/WColor.h"
#include "core/kernel/WModuleFactory.h"
#include "WMClusterParamDisplay.h"
#include "WMClusterParamDisplay.xpm"

// This line is needed by the module loader to actually find your module.
W_LOADABLE_MODULE( WMClusterParamDisplay )

WMClusterParamDisplay::WMClusterParamDisplay()
    : WModuleContainer( "Cluster Param Display", "Displays various parameters on a cluster surface." )
{
}

WMClusterParamDisplay::~WMClusterParamDisplay()
{
}

boost::shared_ptr< WModule > WMClusterParamDisplay::factory() const
{
    return boost::shared_ptr< WModule >( new WMClusterParamDisplay() );
}

const char** WMClusterParamDisplay::getXPMIcon() const
{
    return clusterParamDisplay_xpm;
}

void WMClusterParamDisplay::connectors()
{
    m_fiberIC = WModuleInputForwardData< WDataSetFibers >::createAndAdd( shared_from_this(), "fiberInput", "DataSetFibers to cluster and display" );
    m_paramIC = WModuleInputForwardData< WDataSetScalar >::createAndAdd( shared_from_this(), "paramInput", "Parameter Dataset such as FA" );

    WModule::connectors();
}

void WMClusterParamDisplay::properties()
{
    m_isoValue = m_properties->addProperty( "Iso value", "", 0.2 );
    m_isoValue->setMin( 0.0 );
    m_isoValue->setMax( 100.0 );
    m_drawIsoSurface = m_properties->addProperty( "Iso Surface", "En/Disables the display of the iso surface", true );

    WModule::properties();
}

void WMClusterParamDisplay::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_isoValue->getCondition() );
    m_moduleState.add( m_drawIsoSurface->getCondition() );

    initSubModules();

    // initial values
    m_detTractClustering->getProperties()->getProperty( "Start clustering" )->toPropTrigger()->set( WPVBaseTypes::PV_TRIGGER_TRIGGERED );

    ready();

    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        if( m_shutdownFlag() )
        {
            break;
        }

        if( m_isoValue->changed() )
        {
            std::cout <<  m_isoValue->get() << std::endl;
            m_isoSurface->getProperties()->getProperty( "Iso value" )->toPropDouble()->set( m_isoValue->get() );
            m_clusterSlicer->getProperties()->getProperty( "Iso value" )->toPropDouble()->set( m_isoValue->get( true ) );
        }

        if( m_drawIsoSurface->changed() )
        {
            m_meshRenderer->getProperties()->getProperty( "active" )->toPropBool()->set( m_drawIsoSurface->get( true ) );
        }
    }
}

void WMClusterParamDisplay::initSubModules()
{
    // instantiation
    m_detTractClustering = WModuleFactory::getModuleFactory()->create( WModuleFactory::getModuleFactory()->getPrototypeByName( "Deterministic Tract Clustering" ) ); // NOLINT
    add( m_detTractClustering );
    m_detTractClustering->isReady().wait();

    m_voxelizer = WModuleFactory::getModuleFactory()->create( WModuleFactory::getModuleFactory()->getPrototypeByName( "Voxelizer" ) );
    add( m_voxelizer );
    m_voxelizer->isReady().wait();

    m_gaussFiltering = WModuleFactory::getModuleFactory()->create( WModuleFactory::getModuleFactory()->getPrototypeByName( "Gauss Filtering" ) );
    add( m_gaussFiltering );
    m_gaussFiltering->isReady().wait();

    m_isoSurface = WModuleFactory::getModuleFactory()->create( WModuleFactory::getModuleFactory()->getPrototypeByName( "Isosurface" ) );
    add( m_isoSurface );
    m_isoSurface->isReady().wait();

    m_clusterSlicer = WModuleFactory::getModuleFactory()->create( WModuleFactory::getModuleFactory()->getPrototypeByName( "Cluster Slicer" ) );
    add( m_clusterSlicer );
    m_clusterSlicer->isReady().wait();

    m_meshRenderer = WModuleFactory::getModuleFactory()->create( WModuleFactory::getModuleFactory()->getPrototypeByName( "Triangle Mesh Renderer" ) );
    add( m_meshRenderer );
    m_meshRenderer->isReady().wait();

    // preset properties
    debugLog() << "Start step submodule properties";
    m_detTractClustering->getProperties()->getProperty( "active" )->toPropBool()->set( false, true );
    m_voxelizer->getProperties()->getProperty( "active" )->toPropBool()->set( false, true );
    m_gaussFiltering->getProperties()->getProperty( "Iterations" )->toPropInt()->set( 3, true );
    m_clusterSlicer->getProperties()->getProperty( "Show|Hide Iso Voxels" )->toPropBool()->set( false );
    m_clusterSlicer->getProperties()->getProperty( "Biggest Component Only" )->toPropBool()->set( false );
    m_isoSurface->getProperties()->getProperty( "active" )->toPropBool()->set( false, true );
    m_isoSurface->getProperties()->getProperty( "Iso value" )->toPropDouble()->set( 0.2, true );
    m_clusterSlicer->getProperties()->getProperty( "Iso value" )->toPropDouble()->set(  0.2, true );
    debugLog() << "Submodule properties set";

    // wiring
    debugLog() << "Start wiring";
    m_paramIC->forward( m_clusterSlicer->getInputConnector( "paramInput" ) );

    m_gaussFiltering->getInputConnector( "in" )->connect( m_voxelizer->getOutputConnector( "voxelOutput" ) );
    m_isoSurface->getInputConnector( "values" )->connect( m_gaussFiltering->getOutputConnector( "out" ) );
    m_clusterSlicer->getInputConnector( "clusterInput" )->connect( m_detTractClustering->getOutputConnector( "clusterOutput" ) );
    m_clusterSlicer->getInputConnector( "clusterDSInput" )->connect( m_gaussFiltering->getOutputConnector( "out" ) );
    m_clusterSlicer->getInputConnector( "meshInput" )->connect( m_isoSurface->getOutputConnector( "surface mesh" ) );
    m_meshRenderer->getInputConnector( "mesh" )->connect( m_clusterSlicer->getOutputConnector( "meshOutput" ) );
    m_meshRenderer->getInputConnector( "colorMap" )->connect( m_clusterSlicer->getOutputConnector( "colorMapOutput" ) );

    m_voxelizer->getInputConnector( "tractInput" )->connect( m_detTractClustering->getOutputConnector( "clusterOutput" ) );
    m_fiberIC->forward( m_detTractClustering->getInputConnector( "tractInput" ) ); // init rippling
    debugLog() << "Wiring done";

    // forward properties
    m_properties->addProperty( m_detTractClustering->getProperties()->getProperty( "Output cluster ID" ) );
    m_properties->addProperty( m_detTractClustering->getProperties()->getProperty( "Max cluster distance" ) );
    m_properties->addProperty( m_detTractClustering->getProperties()->getProperty( "Min point distance" ) );
    m_properties->addProperty( m_gaussFiltering->getProperties()->getProperty( "Iterations" ) );
    m_properties->addProperty( m_meshRenderer->getProperties()->getProperty( "Coloring" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Show|Hide Iso Voxels" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Mean Type" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Show|Hide Slices" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Planes #X-SamplePoints" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Planes #Y-SamplePoints" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Planes Step Width" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "#Planes" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Biggest Component Only" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "Custom Scale" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "MinScale" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "MaxScale" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "MinScaleColor" ) );
    m_properties->addProperty( m_clusterSlicer->getProperties()->getProperty( "MaxScaleColor" ) );
    m_properties->addProperty( m_voxelizer->getProperties()->getProperty( "Voxels per Unit" ) );
    m_properties->addProperty( m_detTractClustering->getProperties()->getProperty( "Start clustering" ) );
}
