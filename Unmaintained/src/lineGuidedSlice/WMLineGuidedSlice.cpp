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
#include <vector>
#include <utility>

#include <osg/LineWidth>

#include "core/common/math/linearAlgebra/WPosition.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"
#include "core/common/WAssert.h"
#include "core/common/WLogger.h"
#include "core/dataHandler/datastructures/WFiberCluster.h"
#include "core/dataHandler/WDataHandler.h"
#include "core/dataHandler/WSubject.h"
#include "core/graphicsEngine/WGEColormapping.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/kernel/WKernel.h"
#include "WMLineGuidedSlice.h"
#include "WMLineGuidedSlice.xpm"

WMLineGuidedSlice::WMLineGuidedSlice():
    WModule(),
    m_isPicked( false )
{
}

WMLineGuidedSlice::~WMLineGuidedSlice()
{
}

boost::shared_ptr< WModule > WMLineGuidedSlice::factory() const
{
    return boost::shared_ptr< WModule >( new WMLineGuidedSlice() );
}

const char** WMLineGuidedSlice::getXPMIcon() const
{
    return lineGuidedSlice_xpm;
}

const std::string WMLineGuidedSlice::getName() const
{
    return "Line Guided Slice";
}

const std::string WMLineGuidedSlice::getDescription() const
{
    return "<font color=\"#0000ff\"><b>[Experimental Status]</b></font> A slice that can be moved along a line. ";
}

void WMLineGuidedSlice::connectors()
{
    typedef WModuleInputData< const WFiberCluster > ClusterInputType; // just an alias
    m_input = boost::shared_ptr< ClusterInputType >( new ClusterInputType( shared_from_this(),
                                                                           "cluster",
                                                                           "A fiber cluster, that can"
                                                                           " be used to get the centerline from." ) );
    addConnector( m_input );

    // call WModules initialization
    WModule::connectors();
}

void WMLineGuidedSlice::properties()
{
    m_pos = m_properties->addProperty( "Slice position", "Position of ths slice along the line.", 0., true );
    m_pos->setMin( 0. );
    m_pos->setMax( 1. );

    WModule::properties();
}

void WMLineGuidedSlice::moduleMain()
{
    m_moduleState.add( m_input->getDataChangedCondition() );  // additional fire-condition: "data changed" flag
    boost::shared_ptr< WGraphicsEngine > ge = WGraphicsEngine::getGraphicsEngine();
    assert( ge );

    m_viewer = ge->getViewerByName( "Main View" );
    assert( m_viewer );
    m_viewer->getPickHandler()->getPickSignal()->connect( boost::bind( &WMLineGuidedSlice::setSlicePosFromPick, this, _1 ) );

    // signal ready state
    ready();

    m_rootNode = osg::ref_ptr< WGEGroupNode >( new WGEGroupNode() );

    while( !m_shutdownFlag() ) // loop until the module container requests the module to quit
    {
        if( !m_input->getData() ) // ok, the output has not yet sent data
        {
            m_moduleState.wait();
            continue;
        }

        updateCenterLine();

        create();

        m_moduleState.wait(); // waits for firing of m_moduleState ( dataChanged, shutdown, etc. )
    }

    // clean up stuff
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

void WMLineGuidedSlice::updateCenterLine()
{
    debugLog() << "Draw center line.";

    assert( m_rootNode );
    assert( m_input );
    assert( m_input->getData() );
    m_centerLine = m_input->getData()->getCenterLine();
    if( m_centerLine )
    {
        debugLog() << "Draw center line representation." << pathLength( *m_centerLine );
        m_centerLineGeode = generateLineStripGeode( *m_centerLine, 2.f );
    }
    else
    {
        warnLog() << "CenterLine update on non existing CenterLine (null)";
        m_centerLineGeode = osg::ref_ptr< osg::Geode >( new osg::Geode() );
    }
    m_rootNode->insert( m_centerLineGeode );
}

void WMLineGuidedSlice::create()
{
    m_sliceNode = osg::ref_ptr<osg::Geode>( new osg::Geode() );
    m_sliceNode->setDataVariance( osg::Object::DYNAMIC );

    m_sliceNode->setName( "Line Guided Slice" );

    m_sliceNode->addDrawable( createGeometry() );

    m_rootNode->insert( m_sliceNode );
    osg::ref_ptr< WGEShader > shader;
    shader = osg::ref_ptr< WGEShader >( new WGEShader( "WMLineGuidedSlice", m_localPath ) );
    shader->apply( m_sliceNode );

    // Colormapping
    WGEColormapping::apply( m_sliceNode, shader );

    osg::StateSet* sliceState = m_sliceNode->getOrCreateStateSet();
    sliceState->setMode( GL_BLEND, osg::StateAttribute::ON );

    m_sliceNode->setUserData( this );
    m_sliceNode->addUpdateCallback( new sliceNodeCallback );

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );
}

void WMLineGuidedSlice::setSlicePosFromPick( WPickInfo pickInfo )
{
    if( pickInfo.getName() == "Line Guided Slice" )
    {
        boost::unique_lock< boost::shared_mutex > lock;
        lock = boost::unique_lock< boost::shared_mutex >( m_updateLock );

        WVector2d newPixelPos( pickInfo.getPickPixel() );
        if( m_isPicked )
        {
            float diff = newPixelPos.x() - m_oldPixelPosition.x();

            diff *= 0.01;

            m_pos->set( m_pos->get() + diff );

            lock.unlock();
        }
        m_oldPixelPosition = newPixelPos;
        m_isPicked |= true;
    }
    else
    {
        m_isPicked &= false;
    }
}

osg::ref_ptr<osg::Geometry> WMLineGuidedSlice::createGeometry()
{
    int posOnLine = m_pos->get() * m_centerLine->size();

    if( posOnLine > ( m_centerLine->size() - 2. ) )
    {
        posOnLine = m_centerLine->size() - 2;
    }

    if( posOnLine < 0 )
    {
        posOnLine = 0;
    }

    WAssert( m_centerLine->size() > 1, "To few positions in center line." );
    WPosition startPos = ( *m_centerLine )[posOnLine];
    WVector3d startSliceNormal = normalize( startPos - ( *m_centerLine )[posOnLine + 1] );
    WVector3d sliceVec1 = normalize( cross( WVector3d( 1, 0, 0 ), startSliceNormal ) );
    WVector3d sliceVec2 = normalize( cross( sliceVec1, startSliceNormal ) );

    osg::ref_ptr<osg::Geometry> sliceGeometry = osg::ref_ptr<osg::Geometry>( new osg::Geometry() );

    osg::Vec3Array* sliceVertices = new osg::Vec3Array;

    const double radius = 100;
    std::vector< WPosition > vertices;
    vertices.push_back( startPos + (      sliceVec1 + sliceVec2 ) * radius );
    vertices.push_back( startPos + ( -1 * sliceVec1 + sliceVec2 ) * radius );
    vertices.push_back( startPos + ( -1 * sliceVec1 - sliceVec2 ) * radius );
    vertices.push_back( startPos + (      sliceVec1 - sliceVec2 ) * radius );

    const size_t nbVerts = 4;
    for( size_t i = 0; i < nbVerts; ++i )
    {
        sliceVertices->push_back( vertices[i] );
    }
    sliceGeometry->setVertexArray( sliceVertices );

    osg::DrawElementsUInt* quad = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 0 );
    quad->push_back( 3 );
    quad->push_back( 2 );
    quad->push_back( 1 );
    quad->push_back( 0 );
    sliceGeometry->addPrimitiveSet( quad );

    return sliceGeometry;
}

void WMLineGuidedSlice::updateGeometry()
{
    boost::shared_lock<boost::shared_mutex> slock;
    slock = boost::shared_lock<boost::shared_mutex>( m_updateLock );

    osg::ref_ptr<osg::Geometry> sliceGeometry = createGeometry();

    osg::ref_ptr<osg::Drawable> old = osg::ref_ptr<osg::Drawable>( m_sliceNode->getDrawable( 0 ) );
    m_sliceNode->replaceDrawable( old, sliceGeometry );

    slock.unlock();
}

osg::ref_ptr< osg::Geode > WMLineGuidedSlice::generateLineStripGeode( const WLine& line, const float thickness, const WColor& color )
{
    using osg::ref_ptr;
    ref_ptr< osg::Vec3Array > vertices = ref_ptr< osg::Vec3Array >( new osg::Vec3Array );
    ref_ptr< osg::Vec4Array > colors   = ref_ptr< osg::Vec4Array >( new osg::Vec4Array );
    ref_ptr< osg::Geometry >  geometry = ref_ptr< osg::Geometry >( new osg::Geometry );

    for( size_t i = 1; i < line.size(); ++i )
    {
        vertices->push_back( osg::Vec3( line[i-1][0], line[i-1][1], line[i-1][2] ) );
        colors->push_back( wge::getRGBAColorFromDirection( line[i-1], line[i] ) );
    }
    vertices->push_back( osg::Vec3( line.back()[0], line.back()[1], line.back()[2] ) );
    colors->push_back( colors->back() );

    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINE_STRIP, 0, line.size() ) );
    geometry->setVertexArray( vertices );

    if( color != WColor( 0, 0, 0, 0 ) )
    {
        colors->clear();
        colors->push_back( color );
        geometry->setColorArray( colors );
        geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
    }
    else
    {
        geometry->setColorArray( colors );
        geometry->setColorBinding( osg::Geometry::BIND_PER_VERTEX );    // This will not work on OSG 3.2 you should compute the color per vertex
    }

    // line width
    osg::StateSet* stateset = geometry->getOrCreateStateSet();
    stateset->setAttributeAndModes( new osg::LineWidth( thickness ), osg::StateAttribute::ON );
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    osg::ref_ptr< osg::Geode > geode = osg::ref_ptr< osg::Geode >( new osg::Geode );
    geode->addDrawable( geometry );
    return geode;
}
