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

#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include <vector>

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
#include "WMPointsTransform.xpm"
#include "WMPointsTransform.h"
#include "../common/datastructures/octree/WOctree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMPointsTransform )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMPointsTransform::WMPointsTransform():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMPointsTransform::~WMPointsTransform()
{
}

boost::shared_ptr< WModule > WMPointsTransform::factory() const
{
    return boost::shared_ptr< WModule >( new WMPointsTransform() );
}

const char** WMPointsTransform::getXPMIcon() const
{
    return WMPointsTransform_xpm;
}
const std::string WMPointsTransform::getName() const
{
    return "Points - Transform";
}

const std::string WMPointsTransform::getDescription() const
{
    return "Crops point data to a selection.";
}

void WMPointsTransform::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_output = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                        shared_from_this(), "points", "The loaded points." ) );

    addConnector( m_output );
    WModule::connectors();
}

void WMPointsTransform::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    double number_range = 1000000000.0;
    m_pointsCropGroup = m_properties->addPropertyGroup( "Point set cropping",
                                            "Options to crop the point set" );
    m_fromX = m_pointsCropGroup->addProperty( "X min.: ", "Cut boundary.", -number_range, m_propCondition  );
    m_toX = m_pointsCropGroup->addProperty( "X max.: ", "Cut boundary.", number_range, m_propCondition  );
    m_fromY = m_pointsCropGroup->addProperty( "Y min.: ", "Cut boundary.", -number_range, m_propCondition  );
    m_toY = m_pointsCropGroup->addProperty( "Y max.: ", "Cut boundary.", number_range, m_propCondition  );
    m_fromZ = m_pointsCropGroup->addProperty( "Z min.: ", "Cut boundary.", -number_range, m_propCondition  );
    m_toZ = m_pointsCropGroup->addProperty( "Z max.: ", "Cut boundary.", number_range, m_propCondition  );
    m_cutInsteadOfCrop = m_pointsCropGroup->addProperty( "Cut: ", "Cut instead of crop.", false, m_propCondition  );

    m_translatePointsGroup = m_properties->addPropertyGroup( "Point translation",
                                            "Translates the points by the following amount of XYZ offset after cropping." );
    m_translateX = m_translatePointsGroup->addProperty( "X offset: ", "Translates the point set across the X axis by "
                                                        "that offset.", 0.0, m_propCondition  );
    m_translateY = m_translatePointsGroup->addProperty( "Y offset: ", "Translates the point set across the Y axis by "
                                                        "that offset.", 0.0, m_propCondition  );
    m_translateZ = m_translatePointsGroup->addProperty( "Z offset: ", "Translates the point set across the Z axis by "
                                                        "that offset.", 0.0, m_propCondition  );

    m_groupRotation = m_properties->addPropertyGroup( "Rotation options",
                                            "Applis rotation across three planes in sequence." );
    m_rotation1AngleXY = m_groupRotation->addProperty( "1st rot. (plane XY)", "First applied rotation: Along the plane "
                                            "on the coordinate axis X and Y.", 0.0, m_propCondition  );
    m_rotation1AngleXY->setMin( -180.0 );
    m_rotation1AngleXY->setMax( 180.0 );
    m_rotation2AngleYZ = m_groupRotation->addProperty( "2nd rot. (plane YZ)", "First applied rotation: Along the plane "
                                            "on the coordinate axis Y and Z.", 0.0, m_propCondition  );
    m_rotation2AngleYZ->setMin( -180.0 );
    m_rotation2AngleYZ->setMax( 180.0 );
    m_rotation3AngleXZ = m_groupRotation->addProperty( "3rd rot. (plane XZ)", "First applied rotation: Along the plane "
                                            "on the coordinate axis X and Z.", 0.0, m_propCondition  );
    m_rotation3AngleXZ->setMin( -180.0 );
    m_rotation3AngleXZ->setMax( 180.0 );
    m_rotationAnchorX = m_groupRotation->addProperty( "X anchor: ", "X coordinate of the rotation anchor.",
                                                      0.0, m_propCondition  );
    m_rotationAnchorY = m_groupRotation->addProperty( "Y anchor: ", "Y coordinate of the rotation anchor.",
                                                      0.0, m_propCondition  );
    m_rotationAnchorZ = m_groupRotation->addProperty( "Z anchor: ", "Z coordinate of the rotation anchor.",
                                                      0.0, m_propCondition  );

    WModule::properties();
}

void WMPointsTransform::requirements()
{
}

void WMPointsTransform::moduleMain()
{
    infoLog() << "Thrsholding example main routine started";

    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_input->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    // main loop
    while( !m_shutdownFlag() )
    {
        //infoLog() << "Waiting ...";
        m_moduleState.wait();

        boost::shared_ptr< WDataSetPoints > points = m_input->getData();
//        std::cout << "Execute cycle\r\n";
        if  ( points )
        {
            m_verts = points->getVertices();
            m_colors = points->getColors();
            setProgressSettings( m_verts->size() * 2 / 3 );
            initBoundingBox();
            m_output->updateData( getTransformedPointSet() );
            m_progressStatus->finish();
        }

//        std::cout << "this is WOTree " << std::endl;

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPoints > points2 = m_input->getData();
        if  ( !points2 )
        {
            continue;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMPointsTransform::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
void WMPointsTransform::initBoundingBox()
{
    size_t count = m_verts->size() / 3;
    if( m_verts->size() == 0 )
        return;
    for( size_t index = 0; index < count; index++ )
    {
        double x = m_verts->at( index * 3 );
        double y = m_verts->at( index * 3 + 1 );
        double z = m_verts->at( index * 3 + 2 );
        if( index == 0 || x < m_minX ) m_minX = x;
        if( index == 0 || x > m_maxX ) m_maxX = x;
        if( index == 0 || y < m_minY ) m_minY = y;
        if( index == 0 || y > m_maxY ) m_maxY = y;
        if( index == 0 || z < m_minZ ) m_minZ = z;
        if( index == 0 || z > m_maxZ ) m_maxZ = z;
        m_progressStatus->increment( 1 );
    }
    m_fromX->setMin( m_minX );
    m_fromX->setMax( m_maxX );
    m_toX->setMin( m_fromX->get() );
    m_toX->setMax( m_maxX );

    m_fromY->setMin( m_minY );
    m_fromY->setMax( m_maxY );
    m_toY->setMin( m_fromY->get() );
    m_toY->setMax( m_maxY );

    m_fromZ->setMin( m_minZ );
    m_fromZ->setMax( m_maxZ );
    m_toZ->setMin( m_fromZ->get() );
    m_toZ->setMax( m_maxZ );
}
boost::shared_ptr< WDataSetPoints > WMPointsTransform::getTransformedPointSet()
{
    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );

    size_t count = m_verts->size() / 3;
    WPosition offset( m_translateX->get(), m_translateY->get(), m_translateZ->get() );
    for( size_t index = 0; index < count; index++)
    {
        double x = m_verts->at( index * 3 );
        double y = m_verts->at( index * 3 + 1 );
        double z = m_verts->at( index * 3 + 2 );
        double angleXY = m_rotation1AngleXY->get() * M_PI / 180.0;
        double angleYZ = m_rotation2AngleYZ->get() * M_PI / 180.0;
        double angleXZ = m_rotation3AngleXZ->get() * M_PI / 180.0;
        bool isInsideSelection = x >= m_fromX->get() && x <= m_toX->get()
                &&  y >= m_fromY->get() && y <= m_toY->get()
                &&  z >= m_fromZ->get() && z <= m_toZ->get();
        if( isInsideSelection != m_cutInsteadOfCrop->get() )
        {
            x += offset[0];
            y += offset[1];
            z += offset[2];

            x -= m_rotationAnchorX->get();
            y -= m_rotationAnchorY->get();
            z -= m_rotationAnchorZ->get();
            double old = x;
            x = x*cos( angleXY ) - y*sin( angleXY );
            y = old*sin( angleXY ) + y*cos( angleXY );
            old = y;
            y = y*cos( angleYZ ) - z*sin( angleYZ );
            z = old*sin( angleYZ ) + z*cos( angleYZ );
            old = x;
            x = x*cos( angleXZ ) - z*sin( angleXZ );
            z = old*sin( angleXZ ) + z*cos( angleXZ );
            x += m_rotationAnchorX->get();
            y += m_rotationAnchorY->get();
            z += m_rotationAnchorZ->get();

            outVertices->push_back( x );
            outVertices->push_back( y );
            outVertices->push_back( z );
            for( size_t item = 0; item < 3; item++ )
                outColors->push_back( m_colors->at( index * 3 + item ) );
        }
        m_progressStatus->increment( 1 );
    }
    if( outVertices->size() == 0 )
    {
        for( size_t item = 0; item < 3; item++ )
        {
            outVertices->push_back( 0 );
            outColors->push_back( 0 );
        }
    }
    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
}
