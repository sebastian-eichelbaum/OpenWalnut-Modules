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

#include <string>

#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osgDB/WriteFile>

#include "core/common/WAssert.h"
#include "core/common/WPathHelper.h"
#include "core/kernel/WKernel.h"
#include "core/graphicsEngine/WGEViewer.h"
#include "core/graphicsEngine/WPickHandler.h"

#include "WMHud.h"
#include "WMHud.xpm"

WMHud::WMHud()
    : m_updatedPickText( true )
{
}

WMHud::~WMHud()
{
}

boost::shared_ptr< WModule > WMHud::factory() const
{
    return boost::shared_ptr< WModule >( new WMHud() );
}

const char** WMHud::getXPMIcon() const
{
    return hud_xpm;
}

const std::string WMHud::getName() const
{
    return "HUD";
}

const std::string WMHud::getDescription() const
{
    return "This module provides a HUD (Head Up Display) for status displays";
}

void WMHud::connectors()
{
}

void WMHud::properties()
{
    WModule::properties();
}

void WMHud::moduleMain()
{
    // signal ready state
    ready();

    init();

    // Since the modules run in a separate thread: wait
    waitForStop();

    // clean up stuff
    // NOTE: ALWAYS remove your osg nodes!
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
    // disconnect from picking
    boost::shared_ptr< WGEViewer > viewer = WKernel::getRunningKernel()->getGraphicsEngine()->getViewerByName( "Main View" );
    if( viewer->getPickHandler() )
    {
        viewer->getPickHandler()->getPickSignal()->disconnect( boost::bind( &WMHud::updatePickText, this, _1 ) );
    }
}

void WMHud::init()
{
    m_rootNode = osg::ref_ptr< osg::Projection >( new osg::Projection );
    m_rootNode->setName( "HUDNode" );

    // Initialize the projection matrix for viewing everything we
    // will add as descendants of this node. Use screen coordinates
    // to define the horizontal and vertical extent of the projection
    // matrix. Positions described under this node will equate to
    // pixel coordinates.
    m_rootNode->setMatrix( osg::Matrix::ortho2D( 0, 1024, 0, 768 ) );

    // For the HUD model view matrix use an identity matrix
    osg::ref_ptr< osg::MatrixTransform > HUDModelViewMatrix = new osg::MatrixTransform;
    HUDModelViewMatrix->setMatrix( osg::Matrix::identity() );

    // Make sure the model view matrix is not affected by any transforms
    // above it in the scene graph
    HUDModelViewMatrix->setReferenceFrame( osg::Transform::ABSOLUTE_RF );

    // Add the HUD projection matrix as a child of the root node
    // and the HUD model view matrix as a child of the projection matrix
    // Anything under this node will be viewed using this projection matrix
    // and positioned with this model view matrix.
    //root->addChild(HUDProjectionMatrix);
    m_rootNode->addChild( HUDModelViewMatrix );
    // Add the Geometry node to contain HUD geometry as a child of the
    // HUD model view matrix.

    m_HUDs = osg::ref_ptr< WGEGroupNode >( new WGEGroupNode() );

    // A geometry node for our HUD
    osg::ref_ptr<osg::Geode> HUDGeode = osg::ref_ptr<osg::Geode>( new osg::Geode() );

    HUDModelViewMatrix->addChild( m_HUDs );
    m_HUDs->insert( HUDGeode );

    // Set up geometry for the HUD and add it to the HUD
    osg::ref_ptr< osg::Geometry > HUDBackgroundGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > HUDBackgroundVertices = new osg::Vec3Array;
    HUDBackgroundVertices->push_back( osg::Vec3( 580, 0, -1 ) );
    HUDBackgroundVertices->push_back( osg::Vec3( 1024, 0, -1 ) );
    HUDBackgroundVertices->push_back( osg::Vec3( 1024, 100, -1 ) );
    HUDBackgroundVertices->push_back( osg::Vec3( 580, 100, -1 ) );

    osg::ref_ptr< osg::DrawElementsUInt > HUDBackgroundIndices = new osg::DrawElementsUInt( osg::PrimitiveSet::POLYGON, 0 );
    HUDBackgroundIndices->push_back( 0 );
    HUDBackgroundIndices->push_back( 1 );
    HUDBackgroundIndices->push_back( 2 );
    HUDBackgroundIndices->push_back( 3 );

    osg::ref_ptr< osg::Vec4Array > HUDcolors = new osg::Vec4Array;
    HUDcolors->push_back( osg::Vec4( 0.8f, 0.8f, 0.8f, 0.8f ) );

    osg::ref_ptr< osg::Vec3Array > HUDnormals = new osg::Vec3Array;
    HUDnormals->push_back( osg::Vec3( 0.0f, 0.0f, 1.0f ) );
    HUDBackgroundGeometry->setNormalArray( HUDnormals );
    HUDBackgroundGeometry->setNormalBinding( osg::Geometry::BIND_OVERALL );
    HUDBackgroundGeometry->addPrimitiveSet( HUDBackgroundIndices );
    HUDBackgroundGeometry->setVertexArray( HUDBackgroundVertices );
    HUDBackgroundGeometry->setColorArray( HUDcolors );
    HUDBackgroundGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    HUDGeode->addDrawable( HUDBackgroundGeometry );

    // Create and set up a state set using the texture from above
    osg::ref_ptr< osg::StateSet > HUDStateSet = new osg::StateSet();
    HUDGeode->setStateSet( HUDStateSet );

    // For this state set, turn blending on (so alpha texture looks right)
    HUDStateSet->setMode( GL_BLEND, osg::StateAttribute::ON );

    // Disable depth testing so geometry is draw regardless of depth values
    // of geometry already draw.
    HUDStateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
    HUDStateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    // Need to make sure this geometry is draw last. RenderBins are handled
    // in numerical order so set bin number to 11
    HUDStateSet->setRenderBinDetails( 11, "RenderBin" );

    m_osgPickText = osg::ref_ptr< osgText::Text >( new osgText::Text() );

    m_osgPickText->setCharacterSize( 14 );
    m_osgPickText->setFont( WPathHelper::getAllFonts().Default.string() );
    m_osgPickText->setText( "nothing picked" );
    m_osgPickText->setAxisAlignment( osgText::Text::SCREEN );
    m_osgPickText->setPosition( osg::Vec3( 600, 80, -1.5 ) );
    m_osgPickText->setColor( osg::Vec4( 0, 0, 0, 1 ) );

    m_rootNode->addUpdateCallback( new WGEFunctorCallback< osg::Node >( boost::bind( &WMHud::updateCallback, this ) ) );

    // Add the text here because we get a crash if we do it eralier
    HUDGeode->addDrawable( m_osgPickText );

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    if( m_active->get() )
    {
        m_rootNode->setNodeMask( 0xFFFFFFFF );
    }
    else
    {
        m_rootNode->setNodeMask( 0x0 );
    }

    // connect updateGFX with picking
    boost::shared_ptr< WGEViewer > viewer = WKernel::getRunningKernel()->getGraphicsEngine()->getViewerByName( "Main View" );
    WAssert( viewer, "Requested viewer (main) not found." );
    if(viewer->getPickHandler() )
    {
        viewer->getPickHandler()->getPickSignal()->connect( boost::bind( &WMHud::updatePickText, this, _1 ) );
    }

    {
        // Set first text
        WPickInfo initialInfo( std::string( "No information yet." ),
                               std::string(),
                               WPosition(),
                               std::make_pair( 0.0, 0.0 ),
                               WPickInfo::NONE );
        updatePickText( initialInfo );
    }
}

void WMHud::updatePickText( WPickInfo pickInfo )
{
    boost::unique_lock< boost::shared_mutex > lock;
    lock = boost::unique_lock< boost::shared_mutex >( m_updateLock );

    std::ostringstream os;
    os << std::setprecision( 5 )
       << "Picking Information\n"
       << "Name: " << pickInfo.getName() << "\n"
       << "Position: [" << pickInfo.getPickPosition()[0] << ", " << pickInfo.getPickPosition()[1] << ", " << pickInfo.getPickPosition()[2] << "]\n"
       << "Pixel coordinates: "  << pickInfo.getPickPixel().x() << " " << pickInfo.getPickPixel().y() << "\n"
       << "Object Normal: [" << pickInfo.getPickNormal()[0] << ", " << pickInfo.getPickNormal()[1] << ", " << pickInfo.getPickNormal()[2] << "]\n";

    m_pickText = os.str();

    m_updatedPickText = true;

    lock.unlock();
}

void WMHud::updateCallback()
{
    if( m_updatedPickText )
    {
        m_osgPickText->setText( m_pickText.c_str() );

        boost::unique_lock< boost::shared_mutex > lock;
        lock = boost::unique_lock< boost::shared_mutex >( m_updateLock );

        m_updatedPickText = false;

        lock.unlock();
    }
}

void WMHud::activate()
{
    if( m_active->get() )
    {
        m_rootNode->setNodeMask( 0xFFFFFFFF );
    }
    else
    {
        m_rootNode->setNodeMask( 0x0 );
    }

    WModule::activate();
}
