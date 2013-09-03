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
#include <vector>
#include <utility>

#include <boost/regex.hpp>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LightModel>
#include <osg/Material>
#include <osg/PolygonMode>
#include <osg/StateAttribute>
#include <osg/StateSet>

#include "core/common/WAssert.h"
#include "core/common/WPathHelper.h"
#include "core/common/WThreadedFunction.h"
#include "core/common/WStringUtils.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/common/algorithms/WMarchingCubesAlgorithm.h"
#include "core/common/algorithms/WMarchingLegoAlgorithm.h"
#include "core/graphicsEngine/WGEGroupNode.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WROI.h"
#include "core/graphicsEngine/WROIArbitrary.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WROIManager.h"
#include "WCreateSurfaceJob.h"
#include "WMAtlasSurfaces.h"
#include "WMAtlasSurfaces.xpm"

WMAtlasSurfaces::WMAtlasSurfaces():
    WModule(),
    m_dataSet(),
    m_moduleNode( new WGEGroupNode() ),
    m_dirty( false ),
    m_labelsLoaded( false )
{
}

WMAtlasSurfaces::~WMAtlasSurfaces()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMAtlasSurfaces::factory() const
{
    // See "src/modules/template/" for an extensively documented example.
    return boost::shared_ptr< WModule >( new WMAtlasSurfaces() );
}

const char** WMAtlasSurfaces::getXPMIcon() const
{
    return atlas_xpm;
}

const std::string WMAtlasSurfaces::getName() const
{
    // Specify your module name here. This name must be UNIQUE!
    return "Atlas Surfaces";
}

const std::string WMAtlasSurfaces::getDescription() const
{
    return "Use a scalar data set that stores numbers for atlas regions and a "
        "corresponding text file with labels for the numbers to create "
        "surfaces bounding the atlas regions. The surfaces can be picked to reveal "
        "their name in the HUD (module). Regions can be used as ROIs for fiber selection.";
}

void WMAtlasSurfaces::connectors()
{
    // initialize connectors
    m_input = boost::shared_ptr< WModuleInputData < WDataSetScalar > >(
        new WModuleInputData< WDataSetScalar >( shared_from_this(), "in", "Dataset to create atlas surfaces from." ) );
    // add it to the list of connectors. Please note, that a connector NOT added via addConnector will not work as expected.
    addConnector( m_input );

    // call WModules initialization
    WModule::connectors();
}

void WMAtlasSurfaces::properties()
{
    WPropertyBase::PropertyChangeNotifierType propertyCallback = boost::bind( &WMAtlasSurfaces::propertyChanged, this );
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition() );

    m_labelFile = m_properties->addProperty( "Label file", "", boost::filesystem::path( "" ), m_propCondition );
    WPropertyHelper::PC_PATHEXISTS::addTo( m_labelFile );

    m_propCreateRoiTrigger = m_properties->addProperty( "Create Roi",  "Press!", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    m_opacityProp = m_properties->addProperty( "Opacity %", "Opaqueness of surface.", 100 );
    m_opacityProp->setMin( 0 );
    m_opacityProp->setMax( 100 );

    WModule::properties();
}

void WMAtlasSurfaces::moduleMain()
{
    // use the m_input "data changed" flag
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_propCondition );
    m_moduleState.add( m_active->getUpdateCondition() );
    m_moduleState.add( m_input->getDataChangedCondition() );

    // signal ready state
    ready();

    // loop until the module container requests the module to quit
    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        if( m_shutdownFlag() )
        {
            break;
        }

        if( m_dataSet != m_input->getData() || m_labelFile->changed() )
        {
            // acquire data from the input connector
            m_dataSet = m_input->getData();

            if( m_labelFile->get( true ) == boost::filesystem::path( "" ) )
            {
                std::string fn = m_dataSet->getFilename();

                std::string ext( "" );
                if( fn.find( ".nii.gz" ) != std::string::npos )
                {
                    ext = ".nii.gz";
                }
                else if( fn.find( ".nii" ) != std::string::npos )
                {
                    ext = ".nii";
                }

                std::string csvExt( ".csv" );
                if( ext != "" )
                {
                    //                    fn.replace( fn.find( ext ), ext.size(), csvExt );
                }
                else
                {
                    fn.append( csvExt );
                }
                m_labelFile->set( fn );
            }

            if( !boost::filesystem::exists( m_labelFile->get() ) )
            {
                wlog::warn( "Atlas Surfaces" ) << "Expected label file does not exist! (" <<  m_labelFile->get().string() << ")";
                continue;
            }

            loadLabels( m_labelFile->get( true ).string() );

            switch( ( *m_dataSet ).getValueSet()->getDataType() )
            {
                case W_DT_UNSIGNED_CHAR:
                case W_DT_INT16:
                case W_DT_SIGNED_INT:
                    debugLog() << "Starting creating region meshes";
                    createSurfaces();
                    debugLog() << "Finished creating region meshes";
                    debugLog() << "Starting creating OSG nodes";
                    createOSGNode();
                    debugLog() << "Finished creating OSG nodes";
                    m_dirty = true;
                    break;
                case W_DT_FLOAT:
                case W_DT_DOUBLE:
                default:
                    WAssert( false, "Wrong data type in AtlasSurfaces module" );
            }
        }

        if( m_active->changed() )
        {
            if( m_active->get( true ) )
            {
                m_moduleNode->setNodeMask( 0xFFFFFFFF );
            }
            else
            {
                m_moduleNode->setNodeMask( 0x0 );
            }
        }

        if( m_propCreateRoiTrigger->get( true ) == WPVBaseTypes::PV_TRIGGER_TRIGGERED )
        {
             m_propCreateRoiTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, false );
             createRoi();
        }
    }
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_moduleNode );
}

void WMAtlasSurfaces::createSurfaces()
{
    boost::shared_ptr< WGridRegular3D > grid = boost::dynamic_pointer_cast< WGridRegular3D >( m_dataSet->getGrid() );

    boost::shared_ptr<WProgressCombiner> newProgress = boost::shared_ptr<WProgressCombiner>( new WProgressCombiner() );
    boost::shared_ptr<WProgress>pro = boost::shared_ptr<WProgress>( new WProgress( "dummy", m_dataSet->getMax() ) );
    m_progress->addSubProgress( pro );

    boost::shared_ptr<WThreadedJobs<WDataSetScalar, size_t> >job;

    m_regionMeshes2 = boost::shared_ptr< std::vector< boost::shared_ptr< WTriangleMesh > > >(
            new std::vector< boost::shared_ptr< WTriangleMesh > >( m_dataSet->getMax() + 1 ) );

    switch( ( *m_dataSet ).getValueSet()->getDataType() )
    {
        case W_DT_UNSIGNED_CHAR:
            job = boost::shared_ptr<WCreateSurfaceJob<unsigned char> >(
                    new WCreateSurfaceJob<unsigned char>( m_dataSet, m_regionMeshes2, newProgress, pro ) );
            break;
        case W_DT_INT16:
            job = boost::shared_ptr<WCreateSurfaceJob<int16_t> >(
                    new WCreateSurfaceJob<int16_t>( m_dataSet, m_regionMeshes2, newProgress, pro ) );
            break;
        case W_DT_SIGNED_INT:
            job = boost::shared_ptr<WCreateSurfaceJob<int> >(
                    new WCreateSurfaceJob<int>( m_dataSet, m_regionMeshes2, newProgress, pro ) );
            break;
        case W_DT_FLOAT:
        case W_DT_DOUBLE:
        default:
            WAssert( false, "Unknown data type in AtlasSurfaces module" );
    }

    WThreadedFunction< WThreadedJobs<WDataSetScalar, size_t> >threadPool( 4, job );
    threadPool.run();
    threadPool.wait();

    pro->finish();

    m_possibleSelections = boost::shared_ptr< WItemSelection >( new WItemSelection() );

    for(size_t i = 1; i < m_dataSet->getMax() + 1; ++i )
    {
        std::string label = string_utils::toString( i ) + std::string( " " ) + m_labels[i].second;
        m_possibleSelections->addItem( label, "" );
    }

    m_properties->removeProperty( m_aMultiSelection ); // clear before re-adding
    m_aMultiSelection  = m_properties->addProperty( "Regions", "Regions", m_possibleSelections->getSelectorFirst(),
                                                                        m_propCondition );
}

void WMAtlasSurfaces::createOSGNode()
{
    for( size_t i = 1; i < m_regionMeshes2->size(); ++i )
    {
        osg::Geometry* surfaceGeometry = new osg::Geometry();
        osg::ref_ptr< osg::Geode > outputGeode = osg::ref_ptr< osg::Geode >( new osg::Geode );

        std::string label = string_utils::toString( i ) + std::string( " " ) + m_labels[i].second;
        outputGeode->setName( label );

        surfaceGeometry->setUseDisplayList( false );
        surfaceGeometry->setUseVertexBufferObjects( true );

        surfaceGeometry->setVertexArray( ( *m_regionMeshes2 )[i]->getVertexArray() );

        // ------------------------------------------------
        // normals
        surfaceGeometry->setNormalArray( ( *m_regionMeshes2 )[i]->getVertexNormalArray() );
        surfaceGeometry->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

        // ------------------------------------------------
        // colors
        osg::Vec4Array* colors = new osg::Vec4Array;

        colors->push_back( wge::createColorFromIndex( i ) );

        surfaceGeometry->setColorArray( colors );
        surfaceGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

        osg::DrawElementsUInt* surfaceElement = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES, 0 );

        std::vector< size_t >tris = ( *m_regionMeshes2 )[i]->getTriangles();
        surfaceElement->reserve( tris.size() );

        for( unsigned int vertId = 0; vertId < tris.size(); ++vertId )
        {
            surfaceElement->push_back( tris[vertId] );
        }
        surfaceGeometry->addPrimitiveSet( surfaceElement );
        outputGeode->addDrawable( surfaceGeometry );

        osg::StateSet* state = outputGeode->getOrCreateStateSet();
        osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel();
        lightModel->setTwoSided( true );
        state->setAttributeAndModes( lightModel.get(), osg::StateAttribute::ON );
        state->setMode(  GL_BLEND, osg::StateAttribute::ON );
        state->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        state->setRenderBinDetails( 6001 + i, "RenderBin" );
        state->addUniform( new WGEPropertyUniform< WPropInt >( "u_opacity", m_opacityProp ) );
        m_moduleNode->insert( outputGeode );

        m_moduleNode->insert( outputGeode );
    }
    m_shader = osg::ref_ptr< WGEShader >( new WGEShader( "WMAtlasSurfaces", m_localPath ) );
    m_shader->apply( m_moduleNode );


    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_moduleNode );
    m_moduleNode->addUpdateCallback( new WGEFunctorCallback< osg::Node >( boost::bind( &WMAtlasSurfaces::updateGraphics, this ) ) );
}

void WMAtlasSurfaces::propertyChanged()
{
    m_dirty = true;
}

void WMAtlasSurfaces::updateGraphics()
{
    if( !m_dirty && !m_aMultiSelection->changed() )
    {
        return;
    }

    WItemSelector s = m_aMultiSelection->get( true );
    for( size_t i = 0; i < m_moduleNode->getNumChildren(); ++i )
    {
        m_moduleNode->getChild( i )->setNodeMask( 0x0 );

        for( size_t j = 0; j < s.size(); ++j )
        {
            if( s.getItemIndexOfSelected(j) == i )
            {
                m_moduleNode->getChild( i )->setNodeMask( 0xFFFFFFFF );
            }
        }
    }

    m_dirty = false;
}

std::vector< std::string > WMAtlasSurfaces::readFile( const std::string fileName )
{
    std::ifstream ifs( fileName.c_str(), std::ifstream::in );

    std::vector< std::string > lines;

    std::string line;

    while( !ifs.eof() )
    {
        getline( ifs, line );

        lines.push_back( std::string( line ) );
    }

    ifs.close();

    return lines;
}

void WMAtlasSurfaces::loadLabels( std::string fileName )
{
    std::vector<std::string> lines;

    lines = readFile( fileName );

    if( lines.size() == 0 )
    {
        m_labelsLoaded = false;
        return;
    }

    std::vector<std::string>svec;

    try
    {
        for( size_t i = 0; i < lines.size(); ++i )
        {
            svec.clear();
            boost::regex reg( "," );
            boost::sregex_token_iterator it( lines[i].begin(), lines[i].end(), reg, -1 );
            boost::sregex_token_iterator end;
            while( it != end )
            {
                svec.push_back( *it++ );
            }
            if( svec.size() == 3 )
            {
                std::pair< std::string, std::string >newLabel( svec[1], svec[2] );
                m_labels[ string_utils::fromString< size_t >( svec[0] )] = newLabel;
            }
        }
        m_labelsLoaded = true;
    }
    catch( const std::exception& e )
    {
        // print this message AFTER creation of WException to have the backtrace before the message
        WLogger::getLogger()->addLogMessage(
            std::string( "Problem while loading label file. Probably not suitable content.  Message: " ) + e.what(),
            "Module (" + getName() + ")", LL_ERROR );
        m_labels.clear();
        m_labelsLoaded = false;
    }
}

void WMAtlasSurfaces::createRoi()
{
    WItemSelector s = m_aMultiSelection->get( true );
    for( size_t i = 0; i < m_moduleNode->getNumChildren(); ++i )
    {
        for( size_t j = 0; j < s.size(); ++j )
        {
            if( s.getItemIndexOfSelected(j) == i )
            {
                debugLog() << i << " selected";
                cutArea( i + 1 );
            }
        }
    }
}

void WMAtlasSurfaces::cutArea( int index )
{
    boost::shared_ptr< WGridRegular3D > grid = boost::dynamic_pointer_cast< WGridRegular3D >( m_dataSet->getGrid() );

    size_t order = ( *m_dataSet ).getValueSet()->order();
    size_t vDim = ( *m_dataSet ).getValueSet()->dimension();

    boost::shared_ptr< WValueSet< unsigned char > > vals;
    vals =  boost::dynamic_pointer_cast< WValueSet< unsigned char > >( ( *m_dataSet ).getValueSet() );

    boost::shared_ptr< std::vector< float > > newVals = boost::shared_ptr< std::vector< float > >( new std::vector< float >( grid->size(), 0 ) );

    for( size_t i = 0; i < newVals->size(); ++i )
    {
         if( static_cast<int>( vals->getScalar( i ) ) == index )
         {
             ( *newVals )[i] = 1.0;
         }
    }

    boost::shared_ptr< WValueSet< float > > newValueSet =
            boost::shared_ptr< WValueSet< float > >( new WValueSet< float >( order, vDim, newVals, W_DT_FLOAT ) );
    WMarchingLegoAlgorithm mlAlgo;

    osg::ref_ptr< WROI > newRoi = osg::ref_ptr< WROI >( new WROIArbitrary(  grid->getNbCoordsX(), grid->getNbCoordsY(), grid->getNbCoordsZ(),
                                                                            grid->getTransformationMatrix(),
                                                                            *newValueSet->rawDataVectorPointer(),
                                                                            1.0, wge::createColorFromIndex( index ) ) );
    if( m_labelsLoaded )
    {
        newRoi->setName( m_labels[index].second );
    }
    else
    {
        newRoi->setName( std::string( "region " ) + string_utils::toString( index ) );
    }

    if( WKernel::getRunningKernel()->getRoiManager()->getSelectedRoi() == NULL )
    {
        WKernel::getRunningKernel()->getRoiManager()->addRoi( newRoi );
    }
    else
    {
        WKernel::getRunningKernel()->getRoiManager()->addRoi( newRoi, WKernel::getRunningKernel()->getRoiManager()->getSelectedRoi() );
    }
}
