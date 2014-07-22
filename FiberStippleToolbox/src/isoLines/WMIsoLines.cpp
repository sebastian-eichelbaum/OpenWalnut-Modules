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

#include <osg/Geometry>

#include "core/common/WItemSelectionItemTyped.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WMIsoLines.xpm"
#include "WMIsoLines.h"

WMIsoLines::WMIsoLines():
    WMAbstractSliceModule(),
    m_first( true )
{
}

WMIsoLines::~WMIsoLines()
{
}

boost::shared_ptr< WModule > WMIsoLines::factory() const
{
    return boost::shared_ptr< WModule >( new WMIsoLines() );
}

const char** WMIsoLines::getXPMIcon() const
{
    return WMIsoLines_xpm;
}
const std::string WMIsoLines::getName() const
{
    return "Isoline";
}

const std::string WMIsoLines::getDescription() const
{
    return "Renders isolines in a specific color.";
}

void WMIsoLines::connectors()
{
    m_scalarIC = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "scalarData", "Scalar data." );

    WMAbstractSliceModule::connectors();
}

void WMIsoLines::properties()
{
    m_isovalue = m_properties->addProperty( "Isovalue", "Value selecting the contours", 0.0 );
    m_isovalue->setMin( 0.0 );
    m_isovalue->setMax( 0.0 ); // make it unusable at first

    m_color = m_properties->addProperty( "Line color", "The color for the isoline", WColor( 0.0, 0.0, 0.0, 1.0 ) );
    m_resolution = m_properties->addProperty( "Resolution", "Quad size used for generating line segments", 1.0, m_propCondition );
    m_resolution->setMin( 0.01 );
    m_resolution->setMax( 10.0 );

    m_lineWidth = m_properties->addProperty( "Line Width", "The width of the isoline.", 0.1 );
    m_lineWidth->setMin( 0.0 );
    m_lineWidth->setMax( 1.0 );

    WMAbstractSliceModule::properties();
}

namespace
{
    osg::ref_ptr< osg::Geode > genQuadsPerCell( osg::Vec3 const& base, osg::Vec3 const& a, osg::Vec3 const& b,
            boost::shared_ptr< WDataSetScalar > data, const double resolution )
    {
        size_t maxA = a.length() / resolution;
        size_t maxB = b.length() / resolution;
        // the stuff needed by the OSG to create a geometry instance
        osg::ref_ptr< osg::Vec3Array > vertices = new osg::Vec3Array;
        osg::ref_ptr< osg::Vec3Array > texcoords0 = new osg::Vec3Array;
        osg::ref_ptr< osg::Vec3Array > texcoords1 = new osg::Vec3Array;
        osg::ref_ptr< osg::Vec3Array > normals = new osg::Vec3Array;
        osg::ref_ptr< osg::Vec4Array > colors = new osg::Vec4Array;

        osg::Vec3 aCrossB = a ^ b;
        aCrossB.normalize();
        osg::Vec3 aNorm = a;
        aNorm.normalize();
        osg::Vec3 bNorm = b;
        bNorm.normalize();

        boost::shared_ptr< WGridRegular3D > grid = boost::dynamic_pointer_cast< WGridRegular3D >( data->getGrid() );

        if( grid )
        {
            for( size_t i = 0; i < maxA; ++i )
            {
                for( size_t j = 0; j < maxB; ++j )
                {
                    for( int k = 0; k < 4; ++k )
                    {
                        vertices->push_back( base + aNorm * i * resolution + bNorm * j * resolution );
                    }

                    texcoords0->push_back( ( -aNorm + -bNorm ) * 0.5 * resolution );
                    texcoords0->push_back( (  aNorm + -bNorm ) * 0.5 * resolution );
                    texcoords0->push_back( (  aNorm +  bNorm ) * 0.5 * resolution );
                    texcoords0->push_back( ( -aNorm +  bNorm ) * 0.5 * resolution );

                    texcoords1->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
                    texcoords1->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
                    texcoords1->push_back( osg::Vec3( 1.0, 1.0, 0.0 ) );
                    texcoords1->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );
                }
            }
        }

        normals->push_back( aCrossB );
        colors->push_back( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );

        // put it all together
        osg::ref_ptr< osg::Geometry > geometry = new osg::Geometry();
        geometry->setVertexArray( vertices );
        geometry->setTexCoordArray( 0, texcoords0 );
        geometry->setTexCoordArray( 1, texcoords1 );
        geometry->setNormalBinding( osg::Geometry::BIND_OVERALL );
        geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
        geometry->setNormalArray( normals );
        geometry->setColorArray( colors );
        geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, vertices->size() ) );

        osg::ref_ptr< osg::Geode > geode = new osg::Geode();
        geode->addDrawable( geometry );
        return geode;
    }
}

void WMIsoLines::initOSG( boost::shared_ptr< WDataSetScalar > scalars, const double resolution, const size_t axis )
{
    debugLog() << "Init OSG";
    m_output->clear();

    // grab the current bounding box for computing the size of the slice
    WBoundingBox bb = scalars->getGrid()->getBoundingBox();
    WVector3d minV = bb.getMin();
    WVector3d maxV = bb.getMax();
    WVector3d sizes = ( maxV - minV );
    WVector3d midBB = minV + ( sizes * 0.5 );

    if( axis > 2 )
    {
        errorLog() << "Somehow an axis >= 2 was given (" << axis << "). This is a bug! Please report at openwalnut.org. Aborting.";
        return;
    }

    // determine other two plane vectors
    osg::Vec3 aVec( sliceBaseVectors( sizes, axis ).first );
    osg::Vec3 bVec( sliceBaseVectors( sizes, axis ).second );

    m_pos->setMin( minV[axis] );
    m_pos->setMax( maxV[axis] );

    if( m_first && !m_posIC->getData() )
    {
        m_first = false;
        m_pos->set( midBB[axis] );
    }

    osg::ref_ptr< osg::Uniform > u_WorldTransform = new osg::Uniform( "u_WorldTransform", osg::Matrix::identity() );
    wge::bindAsUniform( m_output, u_WorldTransform, "u_WorldTransform" );
    wge::bindAsUniform( m_output, m_isovalue, "u_isovalue" );
    wge::bindAsUniform( m_output, m_lineWidth, "u_lineWidth" );
    wge::bindAsUniform( m_output, m_color, "u_color" );
    wge::bindAsUniform( m_output, aVec, "u_aVec" );
    wge::bindAsUniform( m_output, bVec, "u_bVec" );
    wge::bindAsUniform( m_output, resolution, "u_resolution" );

    // each slice is child of an transformation node
    osg::ref_ptr< osg::MatrixTransform > mT = new osg::MatrixTransform();
    osg::ref_ptr< osg::Node > slice = genQuadsPerCell( minV, aVec, bVec, scalars, resolution );
    slice->setCullingActive( false );
    mT->addChild( slice );

    // Control transformation node by properties. We use an additional uniform here to provide the shader
    // the transformation matrix used to translate the slice.
    osg::Vec3 planeNormal( 0.0, 0.0, 0.0 );
    planeNormal[axis] = 1.0;
    mT->addUpdateCallback( new WGELinearTranslationCallback< WPropDouble >( planeNormal, m_pos, u_WorldTransform ) );

    m_output->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    m_output->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    m_output->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    m_output->insert( mT );
    m_output->dirtyBound();
}

void WMIsoLines::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_scalarIC->getDataChangedCondition() );
    m_moduleState.add( m_posIC->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_output = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_output );
    osg::ref_ptr< WGEShader > shader = new WGEShader( "WIsolines", m_localPath );
    shader->apply( m_output ); // this automatically applies the shader

    // main loop
    while( !m_shutdownFlag() )
    {
        infoLog() << "Waiting ...";
        m_moduleState.wait();

        // determine axis to draw contours for
        size_t axis = m_sliceSelection->get( true ).at( 0 )->getAs< AxisType >()->getValue();

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        if( m_posIC->getData() )
        {
          WPosition pos = m_posIC->getData()->getProperty();
          double offset = 0.0001; // when the geodes share the exact positions their graphic output will interfere
          if( m_pos->get() != pos[axis] + offset )
          {
                m_pos->set( pos[axis] + offset );
                continue;
            }
        }

        // save data behind connectors since it might change during processing
        boost::shared_ptr< WDataSetScalar > scalarData = m_scalarIC->getData();

        if( !scalarData )
        {
            continue;
        }

        // now something different has happened: We need to create new geometry
        debugLog() << "Handle scalar data update, regenerate geometry.";

        m_isovalue->setMin( 0.0 );
        m_isovalue->setMax( 1.0 );

        initOSG( scalarData, m_resolution->get(), axis );

        wge::bindTexture( m_output, scalarData->getTexture(), 0, "u_scalarData" );
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_output );
}
