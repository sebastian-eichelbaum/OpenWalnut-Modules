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
    m_input.reserve( 5 );
    m_input.resize( 5 );
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
    m_input[0] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 1",
            "The first point set to display" );
    m_input[1] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 2",
            "The second point set to display" );
    m_input[2] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 3",
            "The third point set to display" );
    m_input[3] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 4",
            "The fourth point set to display" );
    m_input[4] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 5",
            "The fifth point set to display" );

    m_output = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                        shared_from_this(), "Transformed points", "The transformed point set." ) );

    addConnector( m_output );
    WModule::connectors();
}

void WMPointsTransform::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    double number_range = 1000000000.0;
    m_pointsCropGroup = m_properties->addPropertyGroup( "Point set cropping",
                                            "Options to crop the point set" );
    m_fromX = m_pointsCropGroup->addProperty( "X min.: ", "Cut boundary.", -number_range, m_propCondition );
    m_toX = m_pointsCropGroup->addProperty( "X max.: ", "Cut boundary.", number_range, m_propCondition );
    m_fromY = m_pointsCropGroup->addProperty( "Y min.: ", "Cut boundary.", -number_range, m_propCondition );
    m_toY = m_pointsCropGroup->addProperty( "Y max.: ", "Cut boundary.", number_range, m_propCondition );
    m_fromZ = m_pointsCropGroup->addProperty( "Z min.: ", "Cut boundary.", -number_range, m_propCondition );
    m_toZ = m_pointsCropGroup->addProperty( "Z max.: ", "Cut boundary.", number_range, m_propCondition );
    m_cutInsteadOfCrop = m_pointsCropGroup->addProperty( "Cut: ", "Cut instead of crop.", false, m_propCondition );

    m_translatePointsGroup = m_properties->addPropertyGroup( "Point translation",
                                            "Translates the points by the following amount of XYZ offset after cropping." );
    m_translateX = m_translatePointsGroup->addProperty( "X offset: ", "Translates the point set across the X axis by "
                                                        "that offset.", 0.0, m_propCondition  );
    m_translateY = m_translatePointsGroup->addProperty( "Y offset: ", "Translates the point set across the Y axis by "
                                                        "that offset.", 0.0, m_propCondition  );
    m_translateZ = m_translatePointsGroup->addProperty( "Z offset: ", "Translates the point set across the Z axis by "
                                                        "that offset.", 0.0, m_propCondition  );

    m_groupMultiplyPoints = m_properties->addPropertyGroup( "Coordinate multiplication",
                                            "Multiplies each X, Y and Z value by a factor." );
    m_factorX = m_groupMultiplyPoints->addProperty( "X factor: ", "Number which multiplies each X coordinate.",
                                                    1.0, m_propCondition );
    m_factorY = m_groupMultiplyPoints->addProperty( "Y factor: ", "Number which multiplies each X coordinate.",
                                                    1.0, m_propCondition );
    m_factorZ = m_groupMultiplyPoints->addProperty( "Z factor: ", "Number which multiplies each X coordinate.",
                                                    1.0, m_propCondition );

    m_groupRotation = m_properties->addPropertyGroup( "Rotation options",
                                            "Applis rotation across three planes in sequence." );
    m_rotation1AngleXY = m_groupRotation->addProperty( "1st rot. (plane XY)", "First applied rotation: Along the plane "
                                            "on the coordinate axis X and Y.", 0.0, m_propCondition );
    m_rotation1AngleXY->setMin( -180.0 );
    m_rotation1AngleXY->setMax( 180.0 );
    m_rotation2AngleYZ = m_groupRotation->addProperty( "2nd rot. (plane YZ)", "First applied rotation: Along the plane "
                                            "on the coordinate axis Y and Z.", 0.0, m_propCondition );
    m_rotation2AngleYZ->setMin( -180.0 );
    m_rotation2AngleYZ->setMax( 180.0 );
    m_rotation3AngleXZ = m_groupRotation->addProperty( "3rd rot. (plane XZ)", "First applied rotation: Along the plane "
                                            "on the coordinate axis X and Z.", 0.0, m_propCondition );
    m_rotation3AngleXZ->setMin( -180.0 );
    m_rotation3AngleXZ->setMax( 180.0 );
    m_rotationAnchorX = m_groupRotation->addProperty( "X anchor: ", "X coordinate of the rotation anchor.",
                                                      0.0, m_propCondition );
    m_rotationAnchorY = m_groupRotation->addProperty( "Y anchor: ", "Y coordinate of the rotation anchor.",
                                                      0.0, m_propCondition );
    m_rotationAnchorZ = m_groupRotation->addProperty( "Z anchor: ", "Z coordinate of the rotation anchor.",
                                                      0.0, m_propCondition );

    m_skipRatio = m_groupRotation->addProperty( "Point skip ratio: ", "Point count that is skipped after "
                                                "adding one point.", 0, m_propCondition  );
    m_skipRatio->setMin( 0 );
    m_skipRatio->setMax( 30 );

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
    for( size_t pointset = 0; pointset < m_input.size(); pointset++ )
        m_moduleState.add( m_input[pointset]->getDataChangedCondition() );
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

        bool addedPoints = false;
        WDataSetPoints::VertexArray newVertices(
                new WDataSetPoints::VertexArray::element_type() );
        m_outVerts = newVertices;
        WDataSetPoints::ColorArray newColors(
                new WDataSetPoints::ColorArray::element_type() );
        m_outColors = newColors;
        for(size_t pointset = 0; pointset < m_input.size(); pointset++)
        {
            boost::shared_ptr< WDataSetPoints > points = m_input[pointset]->getData();
    //        std::cout << "Execute cycle\r\n";
            if  ( points )
            {
                m_inVerts = points->getVertices();
                m_inColors = points->getColors();
                setProgressSettings( m_inVerts->size() * 2 / 3 );
                initBoundingBox( !addedPoints );
                addTransformedPoints();
                m_progressStatus->finish();
                addedPoints = true;
            }
        }
        for( size_t item = 0; !addedPoints && item < 3; item++ )
        {
            m_outVerts->push_back( 0 );
            m_outColors->push_back( 0 );
        }
        if( addedPoints && m_outVerts->size() > 0 )
        {
            setMinMax();
            boost::shared_ptr< WDataSetPoints > outputPoints(
                    new WDataSetPoints( m_outVerts, m_outColors ) );
            m_output->updateData( outputPoints );
        }

//        std::cout << "this is WOTree " << std::endl;

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        if  ( !addedPoints )
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
void WMPointsTransform::initBoundingBox( bool isFirstPointSet )
{
    size_t count = m_inVerts->size() / 3;
    if( m_inVerts->size() == 0 )
        return;
    for( size_t index = 0; index < count; index++ )
    {
        double x = m_inVerts->at( index * 3 );
        double y = m_inVerts->at( index * 3 + 1 );
        double z = m_inVerts->at( index * 3 + 2 );
        if( index == 0 && isFirstPointSet )
        {
            m_minX = m_maxX = x;
            m_minY = m_maxY = y;
            m_minZ = m_maxZ = z;
        }
        if( x < m_minX ) m_minX = x;
        if( x > m_maxX ) m_maxX = x;
        if( y < m_minY ) m_minY = y;
        if( y > m_maxY ) m_maxY = y;
        if( z < m_minZ ) m_minZ = z;
        if( z > m_maxZ ) m_maxZ = z;
        m_progressStatus->increment( 1 );
    }
}
void WMPointsTransform::setMinMax()
{
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
void WMPointsTransform::addTransformedPoints()
{
    double angleXY = m_rotation1AngleXY->get() * M_PI / 180.0;
    double angleYZ = m_rotation2AngleYZ->get() * M_PI / 180.0;
    double angleXZ = m_rotation3AngleXZ->get() * M_PI / 180.0;
    size_t count = m_inVerts->size() / 3;
    WPosition offset( m_translateX->get(), m_translateY->get(), m_translateZ->get() );
    for( size_t index = 0; index < count; index++)
    {
        double x = m_inVerts->at( index * 3 );
        double y = m_inVerts->at( index * 3 + 1 );
        double z = m_inVerts->at( index * 3 + 2 );
        bool isInsideSelection = x >= m_fromX->get() && x <= m_toX->get()
                &&  y >= m_fromY->get() && y <= m_toY->get()
                &&  z >= m_fromZ->get() && z <= m_toZ->get();
        size_t modulo = m_skipRatio->get() + 1;
        bool isPointSkipped = index % ( modulo ) != 0;
        if( isInsideSelection != m_cutInsteadOfCrop->get() && !isPointSkipped )
        {
            x += offset[0];
            y += offset[1];
            z += offset[2];

            x *= m_factorX->get();
            y *= m_factorY->get();
            z *= m_factorZ->get();

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

            m_outVerts->push_back( x );
            m_outVerts->push_back( y );
            m_outVerts->push_back( z );
            for( size_t item = 0; item < 3; item++ )
                m_outColors->push_back( m_inColors->at( index * 3 + item ) );
        }
        m_progressStatus->increment( 1 );
    }
}
