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
#include "../../emptyIcon.xpm" // Please put a real icon here.

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

const char** WMGpView::getXPMIcon() const
{
    return emptyIcon_xpm; // Please put a real icon here.
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
    m_planeNode = wge::genUnitSubdividedPlane( 100, 100, 0.01 );
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
