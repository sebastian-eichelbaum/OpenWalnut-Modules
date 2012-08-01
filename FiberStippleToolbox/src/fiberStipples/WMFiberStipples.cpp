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

#include <cstdlib>
#include <cmath> // for logarithm function
#include <string>

#include <osg/Geometry>
#include <osg/MatrixTransform>

#include "core/common/math/WMath.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/dataHandler/WDataSetVector.h"
#include "core/dataHandler/WGridRegular3D.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/callbacks/WGEPropertyUniformCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/WGraphicsEngine.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WMFiberStipples.h"
#include "WMFiberStipples.xpm"

WMFiberStipples::WMFiberStipples()
    : WModule(),
      m_propCondition( new WCondition() ),
      m_first( true )
{
}

WMFiberStipples::~WMFiberStipples()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMFiberStipples::factory() const
{
    return boost::shared_ptr< WModule >( new WMFiberStipples() );
}

const char** WMFiberStipples::getXPMIcon() const
{
    return WMFiberStipples_xpm;
}

const std::string WMFiberStipples::getName() const
{
    return "Fiber Stipples";
}

const std::string WMFiberStipples::getDescription() const
{
    return "Slice based probabilistic tract display using Fiber Stipples. (see http://dx.doi.org/10.1109/BioVis.2011.6094044)";
}

void WMFiberStipples::connectors()
{
    m_vectorIC = WModuleInputData< WDataSetVector >::createAndAdd( shared_from_this(), "vectors", "Principal diffusion direction." );
    m_probIC = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "probTract", "Probabilistic tract." );
    m_sliceIC = WModuleInputData< WPropDoubleTransfer >::createAndAdd( shared_from_this(), "slice", "Slice and its position." );

    // call WModule's initialization
    WModule::connectors();
}

void WMFiberStipples::properties()
{
//    m_sliceGroup     = m_properties->addPropertyGroup( "Slices",  "Slice based probabilistic tractogram display." );
//
    m_pos = m_properties->addProperty( "Slice position", "Slice position.", 0.0 );
    // we don't know anything about data dimensions yet => make slide unusable
    m_pos->setMax( 0 );
    m_pos->setMin( 0 );

    m_color = m_properties->addProperty( "Color", "Color for the fiber stipples", WColor( 1.0, 0.0, 0.0, 1.0 ) );
    m_threshold = m_properties->addProperty( "Threshold", "Connectivity scores below this threshold will be discarded.", 0.01 );
    m_threshold->setMin( 0.0 );
    m_threshold->setMax( 1.0 );

    m_minRange = m_properties->addProperty( "Min Range", "Minimal stipple density", 0.0 );
    m_minRange->setMin( 0.0 );
    m_minRange->setMax( 1.0 );

    m_maxRange = m_properties->addProperty( "Max Range", "Maximal stipple density", 0.5 );
    m_maxRange->setMin( 0.0 );
    m_maxRange->setMax( 1.0 );

    m_glyphThickness = m_properties->addProperty( "Glyph Thickness", "Line thickness of the glyphs", 1.0 );
    m_glyphThickness->setMin( 0.01 );
    m_glyphThickness->setMax( 3.0 );

    m_glyphSize = m_properties->addProperty( "Glyph Size", "Size of the quad used for generating the stipples", 1.0 );
    m_glyphSize->setMin( 0.01 );
    m_glyphSize->setMax( 10.0 );

    boost::shared_ptr< WItemSelection > axis( new WItemSelection() );
    axis->addItem( AxisType::create( 2, "Axial", "xy-slice" ) );
    axis->addItem( AxisType::create( 1, "Coronal", "xz-slice" ) );
    axis->addItem( AxisType::create( 0, "Sagittal", "yz-slice" ) );
    m_sliceSelection = m_properties->addProperty( "Slice:",  "Which slice (axial, coronal or sagittal)?", axis->getSelector( 1 ), m_propCondition );
    WPropertyHelper::PC_SELECTONLYONE::addTo( m_sliceSelection );

    // call WModule's initialization
    WModule::properties();
}

namespace
{
    osg::ref_ptr< osg::Geode > genScatteredDegeneratedQuads( size_t numSamples, osg::Vec3 const& base, osg::Vec3 const& a,
            osg::Vec3 const& b, size_t sliceNum )
    {
        // the stuff needed by the OSG to create a geometry instance
        osg::ref_ptr< osg::Vec3Array > vertices = new osg::Vec3Array( numSamples * 4 );
        osg::ref_ptr< osg::Vec3Array > texcoords0 = new osg::Vec3Array( numSamples * 4 );
        osg::ref_ptr< osg::Vec3Array > texcoords1 = new osg::Vec3Array( numSamples * 4 );
        osg::ref_ptr< osg::Vec3Array > texcoords2 = new osg::Vec3Array( numSamples * 4 );
        osg::ref_ptr< osg::Vec3Array > normals = new osg::Vec3Array;
        osg::ref_ptr< osg::Vec4Array > colors = new osg::Vec4Array;

        osg::Vec3 aCrossB = a ^ b;
        aCrossB.normalize();
        osg::Vec3 aNorm = a;
        aNorm.normalize();
        osg::Vec3 bNorm = b;
        bNorm.normalize();

        double lambda0, lambda1;
        const double rndMax = RAND_MAX;

        for( size_t i = 0; i < numSamples; ++i )
        {
            // The degenerated QUAD should have all points in its center
            lambda0 = rand() / rndMax; // NOLINT, we do not need thread safety here
            lambda1 = rand() / rndMax; // NOLINT, we do not need thread safety here
            osg::Vec3 quadCenter = base + a * lambda0 + b * lambda1;
            for( int j = 0; j < 4; ++j )
            {
                vertices->push_back( quadCenter );
                texcoords2->push_back( osg::Vec3( static_cast< double >( sliceNum ), 0.0, 0.0 ) );
            }

            texcoords0->push_back( ( -aNorm + -bNorm ) );
            texcoords0->push_back( (  aNorm + -bNorm ) );
            texcoords0->push_back( (  aNorm +  bNorm ) );
            texcoords0->push_back( ( -aNorm +  bNorm ) );

            texcoords1->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
            texcoords1->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
            texcoords1->push_back( osg::Vec3( 1.0, 1.0, 0.0 ) );
            texcoords1->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );
        }

        normals->push_back( aCrossB );
        colors->push_back( osg::Vec4( 1.0, 1.0, 1.0, 1.0 ) );

        // put it all together
        osg::ref_ptr< osg::Geometry > geometry = new osg::Geometry();
        geometry->setVertexArray( vertices );
        geometry->setTexCoordArray( 0, texcoords0 );
        geometry->setTexCoordArray( 1, texcoords1 );
        geometry->setTexCoordArray( 2, texcoords2 );
        geometry->setNormalBinding( osg::Geometry::BIND_OVERALL );
        geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
        geometry->setNormalArray( normals );
        geometry->setColorArray( colors );
        geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, vertices->size() ) );

        osg::ref_ptr< osg::Geode > geode = new osg::Geode();
        geode->addDrawable( geometry );
        return geode;
    }

    // unsigned int whichAxisAlignedPlane( const WVector3d& normal )
    // {
    //     int axis = 0; // the normal is no one of: (1,0,0), (0,1,0) or (0,0,1)
    //     if( dot( normal, WVector3d( 1.0, 0.0, 0.0 ) ) != 0.0 )
    //     {
    //         axis += 1;
    //     }
    //     if( dot( normal, WVector3d( 0.0, 1.0, 0.0 ) ) != 0.0 )
    //     {
    //         axis += 2;
    //     }
    //     if( dot( normal, WVector3d( 0.0, 0.0, 1.0 ) ) != 0.0 )
    //     {
    //         axis += 4;
    //     }
    //     if( axis != 1 || axis != 2 || axis != 4 )
    //     {
    //         return -1;
    //     }
    //     else
    //     {
    //         return std::log( axis ) / std::log( 2 );
    //     }
    // }
}

void WMFiberStipples::initOSG( boost::shared_ptr< WDataSetScalar > probTract, const size_t axis )
{
    debugLog() << "Init OSG";

    m_output->clear();

    // grab the current bounding box for computing the size of the slice
    WBoundingBox bb = probTract->getGrid()->getBoundingBox();
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
    osg::Vec3 aVec( sizes );
    aVec[axis] = 0.0;
    osg::Vec3 bVec( aVec );
    size_t dim1 = ( axis == 2 ? 1 : 2 );
    size_t dim2 = ( axis == 0 ? 1 : 0 );
    aVec[dim1] = 0.0;
    bVec[dim2] = 0.0;


    m_pos->setMin( minV[axis] );
    m_pos->setMax( maxV[axis] );

    // if this is done the first time, set the slices to the center of the dataset
    if( m_first )
    {
        m_first = false;
        m_pos->set( midBB[axis] );
    }

    // each slice is child of an transformation node
    osg::ref_ptr< osg::MatrixTransform > mT = new osg::MatrixTransform();

    size_t numSlices = 20;
    std::srand( time( NULL ) ); // start random number generator here, as seeding within one second might not produce different vertices

    // create a new geode containing the slices
    for( size_t i = 0; i < numSlices; ++i )
    {
        osg::ref_ptr< osg::Node > slice = genScatteredDegeneratedQuads( i / 3.0 * 3000, minV, aVec, bVec, i );
        slice->setCullingActive( false );
        mT->addChild( slice );
    }

    osg::ref_ptr< osg::Uniform > u_aVec = new osg::Uniform( "u_aVec", aVec );
    osg::ref_ptr< osg::Uniform > u_bVec = new osg::Uniform( "u_bVec", bVec );
    osg::ref_ptr< osg::Uniform > u_WorldTransform = new osg::Uniform( "u_WorldTransform", osg::Matrix::identity() );
    boost::shared_ptr< const WGridRegular3D > grid = boost::shared_dynamic_cast< const WGridRegular3D >( probTract->getGrid() );
    if( !grid )
    {
        errorLog() << "This module can only process probabilistic Tracts with regular 3D grids, Hence you may see garbage from now on.";
    }
    boost::array< double, 3 > offsets = getOffsets( grid );
    osg::ref_ptr< osg::Uniform > u_pixelSizeX = new osg::Uniform( "u_pixelSizeX", static_cast< float >( offsets[0] ) );
    osg::ref_ptr< osg::Uniform > u_pixelSizeY = new osg::Uniform( "u_pixelSizeY", static_cast< float >( offsets[1] ) );
    osg::ref_ptr< osg::Uniform > u_pixelSizeZ = new osg::Uniform( "u_pixelSizeZ", static_cast< float >( offsets[2] ) );
    osg::ref_ptr< osg::Uniform > u_color = new WGEPropertyUniform< WPropColor >( "u_color", m_color );
    osg::ref_ptr< osg::Uniform > u_minRange = new WGEPropertyUniform< WPropDouble >( "u_minRange", m_minRange );
    osg::ref_ptr< osg::Uniform > u_maxRange = new WGEPropertyUniform< WPropDouble >( "u_maxRange", m_maxRange );
    osg::ref_ptr< osg::Uniform > u_threshold = new WGEPropertyUniform< WPropDouble >( "u_threshold", m_threshold );
    osg::ref_ptr< osg::Uniform > u_maxConnectivityScore = new osg::Uniform( "u_maxConnectivityScore", static_cast< float >( probTract->getMax() ) );
    osg::ref_ptr< osg::Uniform > u_numSlices = new osg::Uniform( "u_numSlices", static_cast< int >( numSlices ) );
    osg::ref_ptr< osg::Uniform > u_glyphThickness = new WGEPropertyUniform< WPropDouble >( "u_glyphThickness", m_glyphThickness );
    osg::ref_ptr< osg::Uniform > u_glyphSize = new WGEPropertyUniform< WPropDouble >( "u_glyphSize", m_glyphSize );

    osg::StateSet *states = m_output->getOrCreateStateSet();
    states->addUniform( u_aVec );
    states->addUniform( u_bVec );
    states->addUniform( u_WorldTransform );
    states->addUniform( u_pixelSizeX );
    states->addUniform( u_pixelSizeY );
    states->addUniform( u_pixelSizeZ );
    states->addUniform( u_color );
    states->addUniform( u_minRange );
    states->addUniform( u_maxRange );
    states->addUniform( u_threshold );
    states->addUniform( u_maxConnectivityScore );
    states->addUniform( u_numSlices );
    states->addUniform( u_glyphThickness );
    states->addUniform( u_glyphSize );

    // Control transformation node by properties. We use an additional uniform here to provide the shader
    // the transformation matrix used to translate the slice.
    osg::Vec3 planeNormal( 0.0, 0.0, 0.0 );
    planeNormal[axis] = 1.0;
    mT->addUpdateCallback( new WGELinearTranslationCallback< WPropDouble >( planeNormal, m_pos, u_WorldTransform ) );
    debugLog() << "Slice: " << planeNormal << " aVec: " << aVec << " bVec: " << bVec << " axis: " << axis;

    m_output->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    m_output->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    m_output->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    m_output->insert( mT );
    m_output->dirtyBound();
}

void WMFiberStipples::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_probIC->getDataChangedCondition() );
    m_moduleState.add( m_vectorIC->getDataChangedCondition() );
    m_moduleState.add( m_sliceIC->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_output = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_output );
    osg::ref_ptr< WGEShader > shader = new WGEShader( "WFiberStipples", m_localPath );
    shader->apply( m_output ); // this automatically applies the shader

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

        if( m_sliceIC->getData() && !m_externPropSlider )
        {
            m_externPropSlider = m_sliceIC->getData()->getProperty();
            m_pos->set( m_externPropSlider->get( true ) );
            m_moduleState.add( m_externPropSlider->getCondition() );
            WModule::properties();
            infoLog() << "Added external slice position control.";
        }

        if( !m_sliceIC->getData() && m_externPropSlider )
        {
            m_moduleState.remove( m_externPropSlider->getCondition() );
            m_externPropSlider.reset();
            infoLog() << "Removed external slice position control.";
        }

        if( m_externPropSlider && m_externPropSlider->changed() )
        {
            // update slice position
            debugLog() << "External slice position has changed.";
            m_pos->set( m_externPropSlider->get( true ) );
            continue; // do not regenerate geometry incase of slide updates
        }

        // save data behind connectors since it might change during processing
        boost::shared_ptr< WDataSetVector > vectors = m_vectorIC->getData();
        boost::shared_ptr< WDataSetScalar > probTract = m_probIC->getData();

        if( !( vectors && probTract ) || !boost::shared_dynamic_cast< WGridRegular3D >( probTract->getGrid() ) ) // if data valid
        {
            continue;
        }

        size_t axis = m_sliceSelection->get( true ).at( 0 )->getAs< AxisType >()->getValue();
        initOSG( probTract, axis );

        wge::bindTexture( m_output, vectors->getTexture(), 0, "u_vectors" );
        wge::bindTexture( m_output, probTract->getTexture(), 1, "u_probTract" );
    }

    m_output->clear();
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_output );
}
