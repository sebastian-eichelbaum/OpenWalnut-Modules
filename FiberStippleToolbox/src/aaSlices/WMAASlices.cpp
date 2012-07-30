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

#include <cmath> // for logarithm function
#include <cstdlib>
#include <string>

#include <boost/array.hpp>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgManipulator/Translate1DDragger>
#include <osgManipulator/Dragger>
#include <osgManipulator/Command>

#include "core/common/math/WMath.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/dataHandler/WDataSetVector.h"
#include "core/dataHandler/WGridRegular3D.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/callbacks/WGENodeMaskCallback.h"
#include "core/graphicsEngine/callbacks/WGEPropertyUniformCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEColormapping.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/WGraphicsEngine.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WMAASlices.h"
#include "WMAASlices.xpm"

WMAASlices::WMAASlices()
    : WModule(),
      m_first( true )
{
}

WMAASlices::~WMAASlices()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMAASlices::factory() const
{
    return boost::shared_ptr< WModule >( new WMAASlices() );
}

const char** WMAASlices::getXPMIcon() const
{
    return WMAASlices_xpm;
}

const std::string WMAASlices::getName() const
{
    return "Axis Aligned Planes";
}

const std::string WMAASlices::getDescription() const
{
    return "Interactive axis aligned planes serving as \"selection-principle\" to other modules.";
}

void WMAASlices::connectors()
{
    // m_vectorIC = WModuleInputData< WDataSetVector >::createAndAdd( shared_from_this(), "vectors", "Principal diffusion direction." );
    // m_probIC = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "probTract", "Probabilistic tract." );

    // call WModule's initialization
    WModule::connectors();
}

void WMAASlices::properties()
{
    m_pos[0] = m_properties->addProperty( "Axial Slice", "Slice position of axial slice", 0.0 );
    m_pos[1] = m_properties->addProperty( "Coronal Slice", "Slice position of coronal slice", 0.0 );
    m_pos[2] = m_properties->addProperty( "Sagittal Slice", "Slice position of sagittal slice", 0.0 );

    m_showSlice[0]  = m_properties->addProperty( "Show Axial", "Whether to show or hide this slice", true );
    m_showSlice[1]  = m_properties->addProperty( "Show Coronal", "Whether to show or hide this slice", true );
    m_showSlice[2]  = m_properties->addProperty( "Show Sagittal", "Whether to show or hide this slice", true );

    m_color[0] = m_properties->addProperty( "Axial Color", "Color for this slice", WColor( 1.0, 0.0, 0.0, 0.3 ) );
    m_color[1] = m_properties->addProperty( "Coronal Color", "Color for this slice", WColor( 0.0, 1.0, 0.0, 0.3 ) );
    m_color[2] = m_properties->addProperty( "Sagittal Color", "Color for this slice", WColor( 0.0, 0.0, 1.0, 0.3 ) );

    // call WModule's initialization
    WModule::properties();
}



void WMAASlices::initOSG()
{
    debugLog() << "Init OSG";

    m_output->clear();

    // grab the current bounding box
    WBoundingBox bb = WGEColormapping::instance()->getBoundingBox();
    WVector3d minV = bb.getMin();
    WVector3d maxV = bb.getMax();
    WVector3d sizes = ( maxV - minV );
    WVector3d midBB = minV + ( sizes * 0.5 );

    boost::array< osg::ref_ptr< osg::Node >, 3 > slices;

    boost::array< osg::Vec3, 3 > base = {{ osg::Vec3( 1.0, 0.0, 0.0 ), osg::Vec3( 0.0, 1.0, 0.0 ), osg::Vec3( 0.0, 0.0, 1.0 ) }}; // NOLINT curley braces

    // we didn't put this into for loop as initialization becomes too bloated with if-else constructs
    slices[0] = wge::genFinitePlane( minV, base[1] * sizes[1], base[2] * sizes[2] );
    slices[1] = wge::genFinitePlane( minV, base[0] * sizes[0], base[2] * sizes[2] );
    slices[2] = wge::genFinitePlane( minV, base[0] * sizes[0], base[1] * sizes[1] );
//    slices[0] = wge::genFinitePlane( osg::Vec3( midBB[0], minV[1], minV[2] ), base[1] * sizes[1], base[2] * sizes[2] );
//    slices[1] = wge::genFinitePlane( osg::Vec3( minV[0], midBB[1], minV[2] ), base[0] * sizes[0], base[2] * sizes[2] );
//    slices[2] = wge::genFinitePlane( osg::Vec3( minV[0], minV[1], midBB[2] ), base[0] * sizes[0], base[1] * sizes[1] );

    boost::array< osg::ref_ptr< osg::MatrixTransform >, 3 > mT;
    boost::array< osg::ref_ptr< osg::Uniform >, 3 > sliceUniforms;

    for( int i = 0; i < 3; ++i )
    {
        m_pos[i]->setMin( minV[i] );
        m_pos[i]->setMax( maxV[i] );
        m_pos[i]->setHidden( false );

        if( m_first )
        {
            m_pos[i]->set( midBB[i] );
        }

        slices[i]->setCullingActive( false );
        slices[i]->addUpdateCallback( new WGENodeMaskCallback( m_showSlice[i] ) );
        sliceUniforms[i] = new osg::Uniform( "u_WorldTransform", osg::Matrix::identity() );
        osg::ref_ptr< osg::Uniform > u_color = new WGEPropertyUniform< WPropColor >( "u_color", m_color[i] );
        slices[i]->getOrCreateStateSet()->addUniform( sliceUniforms[i] );
        slices[i]->getOrCreateStateSet()->addUniform( u_color );

        // attach a dragger to the slice => taken from osg example: osgmanipulators
        osg::Group* sliceGroup = new osg::Group;
        osg::MatrixTransform* sliceTransform = new osg::MatrixTransform;
        sliceTransform->addChild( slices[i] );
        sliceTransform->addUpdateCallback( new WGELinearTranslationCallback< WPropDouble >( base[i], m_pos[i], sliceUniforms[i] ) );
        sliceGroup->addChild( sliceTransform );

        // dragger is a sibling to the last transform of the real slice geode
        osgManipulator::Translate1DDragger* d = new osgManipulator::Translate1DDragger( osg::Vec3( 0.0, 0.0, 0.0 ), base[i] * sizes[i] );
        d->setupDefaultGeometry();
        d->addTransformUpdating( sliceTransform ); // , osgManipulator::DraggerTransformCallback::HANDLE_TRANSLATE_IN_LINE );
        d->addDraggerCallback( new WMAASlices::PositionChangedCallback( m_pos[i], i, d ) );
        d->setHandleEvents(true);
        d->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_CTRL);
        d->setActivationKeyEvent('a');
        d->setMatrix( osg::Matrix::scale( 0.5, 0.5, 0.5 ) *
                      osg::Matrix::translate( slices[i]->getBound().center() ) * osg::Matrix::translate( base[i] * sizes[i] * -0.25 ) );
        d->addUpdateCallback( new WGENodeMaskCallback( m_showSlice[i] ) );
        // d->addConstraint( new PlaneConstraint() );
        d->addUpdateCallback( new WGELinearTranslationCallback< WPropDouble >( base[i], m_pos[i], sliceUniforms[i] ) );

        sliceGroup->addChild( d );

        mT[i] = new osg::MatrixTransform();
        mT[i]->addChild( sliceGroup );
        // mT[i]->addUpdateCallback( new WGELinearTranslationCallback< WPropDouble >( base[i], m_pos[i], sliceUniforms[i] ) );
        m_output->insert( mT[i] );
    }
    m_first = false;

    m_output->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    m_output->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    m_output->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    m_output->dirtyBound();
}

void WMAASlices::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( WGEColormapping::instance()->getChangeCondition() );

    ready();

    // graphics setup
    m_output = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_output );
    osg::ref_ptr< WGEShader > shader = new WGEShader( "WAASlices", m_localPath );
    shader->apply( m_output ); // this automatically applies the shader
    initOSG();

    // main loop
    while( !m_shutdownFlag() )
    {
        infoLog() << "Waitings ...";
        m_moduleState.wait();

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        for( int i = 0; i < 3; ++i )
        {
            m_pos[i]->setMin( WGEColormapping::instance()->getBoundingBox().getMin()[i] );
            m_pos[i]->setMax( WGEColormapping::instance()->getBoundingBox().getMax()[i] );
        }
    }

    m_output->clear();
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_output );
}
