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

#include <boost/bind.hpp>

#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/callbacks/WGEFunctorCallback.h"
#include "core/kernel/WKernel.h"

#include "WMGpView.h"

WMGpView::WMGpView():
    WModule(),
    m_newPlaneColors( false )
{
}

WMGpView::~WMGpView()
{
}

boost::shared_ptr< WModule > WMGpView::factory() const
{
    return boost::shared_ptr< WModule >( new WMGpView() );
}

const std::string WMGpView::getName() const
{
    return "Gaussian Process Viewer";
}

const std::string WMGpView::getDescription() const
{
    return "Displays Gaussian processes. It is intended to display espically GP representing deterministic tracts.";
}

void WMGpView::connectors()
{
    m_gpIC = WModuleInputData< WDataSetGP >::createAndAdd( shared_from_this(), "gpIn", "The Gaussian processes" );

    WModule::connectors();
}

void WMGpView::properties()
{
    m_normal = m_properties->addProperty( "#Plane normal", "The normal of the plane", WPosition( 1.0, 0.0, 0.0 ) );
    m_pos = m_properties->addProperty( "#Plane position", "The position of the plane", WPosition( 87, 20, 150 ) );
    m_scale = m_properties->addProperty( "Scaling factor", "How much the plane is streched", 91.0 );

    WModule::properties();
}

void WMGpView::moduleMain()
{
    m_moduleState.setResetable( true, true ); // remember actions when actually not waiting for actions
    m_moduleState.add( m_gpIC->getDataChangedCondition() );
    m_moduleState.add( m_scale->getCondition() );

    ready();

    m_rootNode = osg::ref_ptr< WGEManagedGroupNode > ( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );
    debugLog() << "Insert quad-plane";
    m_rootNode->clear();
    m_planeNode = genUnitSubdividedPlane( 100, 100, 0.01 );
    m_planeNode->addUpdateCallback( new WGEFunctorCallback< osg::Node >( boost::bind( &WMGpView::updatePlaneColors, this, _1 ) ) );
    m_rootNode->insert( m_planeNode );

    while( !m_shutdownFlag() ) // loop until the module container requests the module to quit
    {
        debugLog() << "Waiting..";
        m_moduleState.wait();
        if( !m_gpIC->getData().get() ) // ok, the output has not yet sent data
        {
            continue;
        }

        boost::shared_ptr< WDataSetGP > dataSet = m_gpIC->getData();
        // TODO(math): insert here cool stuff
        if( !dataSet )
        {
            debugLog() << "Invalid data--> continue";
            continue;
        }
        if( m_gpIC->handledUpdate() )
        {
            debugLog() << "Input has been updated...";
        }
        debugLog() << "Resetting the matrix.";
        osg::Matrixd transform = generateMatrix();
        m_rootNode->setMatrix( transform );
        m_newColors = generateNewColors( transform, dataSet );
        m_newPlaneColors = true;
    }
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

osg::Matrixd WMGpView::generateMatrix() const
{
    osg::Matrixd trans;
    trans.makeTranslate( m_pos->get().as< osg::Vec3d >() );

    osg::Matrixd scale;
    scale.makeScale( m_scale->get(), m_scale->get(), m_scale->get() );

    osg::Matrixd rot;
    rot.makeRotate( osg::Vec3d( 0.0, 0.0, 1.0 ), m_normal->get().as< osg::Vec3d >() );

    return scale * rot * trans; // order is important: first scale, then rotate and finally translate, since matrix multiply
}

osg::ref_ptr< osg::Vec4Array > WMGpView::generateNewColors( const osg::Matrixd& m, boost::shared_ptr< const WDataSetGP > dataset ) const
{
    osg::ref_ptr< osg::Vec4Array > newColors = new osg::Vec4Array;
    osg::ref_ptr< const osg::Vec3Array > oldCenters = m_planeNode->getCenterArray();

    for( size_t i = 0; i < oldCenters->size(); ++i )
    {
        // ATTENTION: Matrix is in OSG post multiply
        double mean = dataset->mean( WVector3d( ( *oldCenters )[i] * m ) );
        newColors->push_back( osg::Vec4( mean, mean, mean, 1.0 ) );
    }
    return newColors;
}

void WMGpView::updatePlaneColors( osg::Node* node )
{
    if( m_newPlaneColors )
    {
        WGESubdividedPlane *geode = dynamic_cast< WGESubdividedPlane* >( node );
        if( !geode )
        {
            errorLog() << "Invalid update callback on osg::Node which is not a WGESubdividedPlane";
            return;
        }
        osg::Geometry *geo = dynamic_cast< osg::Geometry* >( geode->getDrawable( 0 ) );
        WAssert( geo, "A WGESubdivededPlane geode must have a drawable!, but didn't => this is a bug!" );
        geo->setColorArray( m_newColors );
        // delete the reference in the module, so only this node has access to this color array
        m_newColors = osg::ref_ptr< osg::Vec4Array >( new osg::Vec4Array );
        m_newPlaneColors = false;
    }
}

osg::ref_ptr< WGESubdividedPlane > WMGpView::genUnitSubdividedPlane( size_t resX, size_t resY, double spacing )
{
    WAssert( resX > 0 && resY > 0, "A Plane with no quad is not supported, use another datatype for that!" );
    double dx = ( resX > 1 ? 1.0 / ( resX - 1 ) : 1.0 );
    double dy = ( resY > 1 ? 1.0 / ( resY - 1 ) : 1.0 );

    size_t numQuads = resX * resY;

    using osg::ref_ptr;
    ref_ptr< osg::Vec3Array > vertices = ref_ptr< osg::Vec3Array >( new osg::Vec3Array( numQuads * 4 ) );
    ref_ptr< osg::Vec3Array > centers = ref_ptr< osg::Vec3Array >( new osg::Vec3Array( numQuads ) );
    ref_ptr< osg::Vec4Array > colors   = ref_ptr< osg::Vec4Array >( new osg::Vec4Array( numQuads ) );

    for( size_t yQuad = 0; yQuad < resY; ++yQuad )
    {
        for( size_t xQuad = 0; xQuad < resX; ++xQuad )
        {
            size_t qIndex = yQuad * resX + xQuad;
            size_t vIndex = qIndex * 4; // since there are 4 corners
            vertices->at( vIndex     ) = osg::Vec3( xQuad * dx + spacing,      yQuad * dy + spacing,      0.0 );
            vertices->at( vIndex + 1 ) = osg::Vec3( xQuad * dx + dx - spacing, yQuad * dy + spacing,      0.0 );
            vertices->at( vIndex + 2 ) = osg::Vec3( xQuad * dx + dx - spacing, yQuad * dy + dy - spacing, 0.0 );
            vertices->at( vIndex + 3 ) = osg::Vec3( xQuad * dx + spacing,      yQuad * dy + dy - spacing, 0.0 );
            centers->at( qIndex ) = osg::Vec3( xQuad * dx + dx / 2.0, yQuad * dy + dy / 2.0, 0.0 );
            colors->at( qIndex ) = osg::Vec4( 0.1 +  static_cast< double >( qIndex ) / numQuads * 0.6,
                                              0.1 +  static_cast< double >( qIndex ) / numQuads * 0.6,
                                              1.0, 1.0 );
        }
    }

    ref_ptr< osg::Geometry >  geometry = ref_ptr< osg::Geometry >( new osg::Geometry );
    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, vertices->size() ) );
    geometry->setVertexArray( vertices );
    geometry->setColorArray( colors );
    geometry->setColorBinding( osg::Geometry::BIND_PER_PRIMITIVE ); // this will not compile on OSG 3.2

    ref_ptr< osg::Vec3Array > normals = ref_ptr< osg::Vec3Array >( new osg::Vec3Array );
    normals->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );
    geometry->setNormalArray( normals );
    geometry->setNormalBinding( osg::Geometry::BIND_OVERALL );
    osg::ref_ptr< WGESubdividedPlane > geode = osg::ref_ptr< WGESubdividedPlane >( new WGESubdividedPlane );
    geode->addDrawable( geometry );
    geode->setCenterArray( centers );

    // we need to disable light, since the order of the vertices may be wrong and with lighting you won't see anything but black surfaces
    osg::StateSet* state = geode->getOrCreateStateSet();
    state->setMode( GL_BLEND, osg::StateAttribute::ON );
    state->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );

    return geode;
}
