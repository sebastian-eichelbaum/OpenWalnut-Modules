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
    m_input.reserve( 7 );
    m_input.resize( 7 );
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
    m_input[5] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 6",
            "The sixth point set to display" );
    m_input[6] = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Input 7",
            "The seventh point set to display" );
    m_inputSubtraction = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Subtraction",
            "Points that are subtracted from the output" );

    m_output = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                        shared_from_this(), "Transformed points", "The transformed point set." ) );
    addConnector( m_output );
    m_outputPointsGrouped = boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > >(
                new WModuleOutputData< WDataSetPointsGrouped >(
                        shared_from_this(), "Transformed points with group ID", "The transformed point set with group IDs." ) );
    addConnector( m_outputPointsGrouped );

    WModule::connectors();
}

void WMPointsTransform::properties()
{
    m_infoRenderTimeSeconds = m_infoProperties->addProperty( "Wall time (s): ", "Time in seconds that the "
                                            "whole render process took.", 0.0 );
    m_infoInputPointCount = m_infoProperties->addProperty( "Input points: ", "", 0 );
    m_infoOutputPointCount = m_infoProperties->addProperty( "Output points: ", "", 0 );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "X min: ", "", 0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "X max: ", "", 0 ) );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "Y min: ", "", 0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "Y max: ", "", 0 ) );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "Z min: ", "", 0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "Z max: ", "", 0 ) );


    double number_range = 1000000000.0;
    m_pointsCropGroup = m_properties->addPropertyGroup( "Point set cropping",
                                            "Options to crop the point set" );
    m_fromX = m_pointsCropGroup->addProperty( "X min.: ", "Cut boundary.", -number_range, m_propCondition );
    m_toX = m_pointsCropGroup->addProperty( "X max.: ", "Cut boundary.", number_range, m_propCondition );
    m_fromY = m_pointsCropGroup->addProperty( "Y min.: ", "Cut boundary.", -number_range, m_propCondition );
    m_toY = m_pointsCropGroup->addProperty( "Y max.: ", "Cut boundary.", number_range, m_propCondition );
    m_fromZ = m_pointsCropGroup->addProperty( "Z min.: ", "Cut boundary.", -number_range, m_propCondition );
    m_toZ = m_pointsCropGroup->addProperty( "Z max.: ", "Cut boundary.", number_range, m_propCondition );
    m_cutInsteadOfCrop = m_pointsCropGroup->addProperty( "Invert: ", "Cut instead of crop.", false, m_propCondition );
    m_disablePointCrop = m_pointsCropGroup->addProperty( "Disable cuts: ", "", false, m_propCondition );
    m_pointSubtractionRadius = m_pointsCropGroup->addProperty( "Subtr. r.: ", "Radius of subtracted coordinates "
                                                        "(most right input connector)", 0.0, m_propCondition );

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



    m_groupColorEqualizer = m_properties->addPropertyGroup( "Color equalizer", "" );
    m_contrastRed = m_groupColorEqualizer->addProperty( "Contrast red: ", "", 1.0, m_propCondition );
    m_contrastRed->setMin( 0.0 );
    m_contrastRed->setMax( 3.0 );
    m_offsetRed = m_groupColorEqualizer->addProperty( "Offset red: ", "", 0.0, m_propCondition );
    m_offsetRed->setMin( -1.0 );
    m_offsetRed->setMax( 1.0 );
    m_contrastGreen = m_groupColorEqualizer->addProperty( "Contrast green: ", "", 1.0, m_propCondition );
    m_contrastGreen->setMin( 0.0 );
    m_contrastGreen->setMax( 3.0 );
    m_OffsetGreen = m_groupColorEqualizer->addProperty( "Offset green: ", "", 0.0, m_propCondition );
    m_OffsetGreen->setMin( -1.0 );
    m_OffsetGreen->setMax( 1.0 );
    m_contrastBlue = m_groupColorEqualizer->addProperty( "Contrast blue: ", "", 1.0, m_propCondition );
    m_contrastBlue->setMin( 0.0 );
    m_contrastBlue->setMax( 3.0 );
    m_offsetBlue = m_groupColorEqualizer->addProperty( "Offset blue: ", "", 0.0, m_propCondition );
    m_offsetBlue->setMin( -1.0 );
    m_offsetBlue->setMax( 1.0 );


    m_groupFileOperations = m_properties->addPropertyGroup( "File equalizer", "" );
    m_inputFile = m_groupFileOperations->addProperty( "Input path: ", "", WPathHelper::getAppPath() );
    WPropertyHelper::PC_PATHEXISTS::addTo( m_inputFile );
    m_reloadPointsTrigger = m_groupFileOperations->addProperty( "Load points:",  "Load from file", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );
    m_outputFile = m_groupFileOperations->addProperty( "Output path: ", "", WPathHelper::getAppPath() );
    m_savePointsTrigger = m_groupFileOperations->addProperty( "Save points:",  "Save to file", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );


    m_pointGroupOptionsGroup = m_properties->addPropertyGroup( "Options for point group output", "" );
    m_assignedGroupID = m_pointGroupOptionsGroup->addProperty( "Assigned point group: ", "", 0, m_propCondition );
    m_assignedGroupID->setMin( 0 );

    WModule::properties();
}

void WMPointsTransform::requirements()
{
}

void WMPointsTransform::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    for( size_t pointset = 0; pointset < m_input.size(); pointset++ )
        m_moduleState.add( m_input[pointset]->getDataChangedCondition() );
    m_moduleState.add( m_inputSubtraction->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    // main loop
    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        WRealtimeTimer timer;
        setProgressSettings( 10 );
        timer.reset();
        WDataSetPoints::VertexArray newVertices(
                new WDataSetPoints::VertexArray::element_type() );
        m_outVerts = newVertices;
        WDataSetPoints::ColorArray newColors(
                new WDataSetPoints::ColorArray::element_type() );
        m_outColors = newColors;
        WDataSetPointsGrouped::GroupArray newGroups(
                new WDataSetPointsGrouped::GroupArray::element_type() );
        m_outGroups = newGroups;
        m_infoInputPointCount->set( 0 );
        m_pointSubtraction.initSubtraction( m_inputSubtraction->getData(), m_pointSubtractionRadius->get() );
        bool addedPoints = onFileLoad();
        for(size_t pointset = 0; pointset < m_input.size(); pointset++)
        {
            boost::shared_ptr< WDataSetPoints > points = m_input[pointset]->getData();
            if  ( points )
            {
                m_inVerts = points->getVertices();
                m_inColors = points->getColors();
                setProgressSettings( m_inVerts->size() * 2 / 3 );
                m_infoInputPointCount->set( m_infoInputPointCount->get() + m_inVerts->size() / 3 );
                initBoundingBox( !addedPoints );
                addTransformedPoints();
                if( points->size() > 0 )
                    addedPoints = true;
            }
        }
        m_infoOutputPointCount->set( m_outVerts->size() / 3 );
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

            boost::shared_ptr< WDataSetPointsGrouped > outputPointsGrouped(
                    new WDataSetPointsGrouped( m_outVerts, m_outColors, m_outGroups ) );
            m_outputPointsGrouped->updateData( outputPointsGrouped );

            onFileSave();
        }
        m_progressStatus->finish();
        m_infoRenderTimeSeconds->set( timer.elapsed() );

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        if  ( !addedPoints )
        {
            continue;
        }
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
        if( x < m_minX )
            m_minX = x;
        if( x > m_maxX )
            m_maxX = x;
        if( y < m_minY )
            m_minY = y;
        if( y > m_maxY )
            m_maxY = y;
        if( z < m_minZ )
            m_minZ = z;
        if( z > m_maxZ )
            m_maxZ = z;
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

    m_infoBoundingBoxMin[0]->set( m_minX );
    m_infoBoundingBoxMax[0]->set( m_maxX );
    m_infoBoundingBoxMin[1]->set( m_minY );
    m_infoBoundingBoxMax[1]->set( m_maxY );
    m_infoBoundingBoxMin[2]->set( m_minZ );
    m_infoBoundingBoxMax[2]->set( m_maxZ );
}
void WMPointsTransform::addTransformedPoints()
{
    double angleXY = m_rotation1AngleXY->get();
    double angleYZ = m_rotation2AngleYZ->get();
    double angleXZ = m_rotation3AngleXZ->get();
    vector<double>* point = new vector<double>( 3, 0.0 );
    size_t count = m_inVerts->size() / 3;
    vector<double> offset = WVectorMaths::new3dVector( m_translateX->get(), m_translateY->get(), m_translateZ->get() );
    vector<double> factor = WVectorMaths::new3dVector( m_factorX->get(), m_factorY->get(), m_factorZ->get() );
    vector<double> rotationAnchor = WVectorMaths::new3dVector(
            m_rotationAnchorX->get(), m_rotationAnchorY->get(), m_rotationAnchorZ->get() );
    vector<double>* rotationAnchorInverted = WVectorMaths::copyVectorForPointer( rotationAnchor );
    WVectorMaths::invertVector( rotationAnchorInverted );
    vector<double> contrast = WVectorMaths::new3dVector( m_contrastRed->get(), m_contrastGreen->get(), m_contrastBlue->get() );
    vector<double> colorOffset = WVectorMaths::new3dVector( m_offsetRed->get(), m_OffsetGreen->get(), m_offsetBlue->get() );
    for( size_t index = 0; index < count; index++)
    {
        point->at( 0 ) = m_inVerts->at( index * 3 );
        point->at( 1 ) = m_inVerts->at( index * 3 + 1 );
        point->at( 2 ) = m_inVerts->at( index * 3 + 2 );
        bool isInsideSelection = point->at( 0 ) >= m_fromX->get() && point->at( 0 ) <= m_toX->get()
                &&  point->at( 1 ) >= m_fromY->get() && point->at( 1 ) <= m_toY->get()
                &&  point->at( 2 ) >= m_fromZ->get() && point->at( 2 ) <= m_toZ->get();
        size_t modulo = m_skipRatio->get() + 1;
        bool isPointSkipped = index % ( modulo ) != 0;
        bool remainsAfterCropping = isInsideSelection != m_cutInsteadOfCrop->get() || m_disablePointCrop->get();
        if( remainsAfterCropping && !isPointSkipped && !m_pointSubtraction.pointsExistNearCoordinate( *point ) )
        {
            WVectorMaths::addVector( point, offset );
            WVectorMaths::multiplyVector( point, factor );

            WVectorMaths::addVector( point, *rotationAnchorInverted );
            WVectorMaths::rotateVector( point, 0, 1, angleXY );
            WVectorMaths::rotateVector( point, 1, 2, angleYZ );
            WVectorMaths::rotateVector( point, 0, 2, angleXZ );
            WVectorMaths::addVector( point, rotationAnchor );

            for( size_t item = 0; item < 3; item++ )
            {
                m_outVerts->push_back( point->at( item ) );

                double color = m_inColors->at( index * 3 + item ) * contrast[item] + colorOffset[item];
                m_outColors->push_back( color < 0.0 ?0.0 :( color > 1.0?1.0 :color ) );
            }
            m_outGroups->push_back( m_assignedGroupID->get() );
        }
        m_progressStatus->increment( 1 );
    }
    delete rotationAnchorInverted;
}
bool WMPointsTransform::onFileLoad()
{
    bool addedPoints = false;
    m_pointInputFile.setFilePath( m_inputFile->get().c_str() );
    setProgressSettings( 10 );
    if( m_reloadPointsTrigger->get() )
        m_pointInputFile.loadWDataSetPoints();
    if( m_pointInputFile.containsData() )
    {
        m_inVerts = m_pointInputFile.getVertices();
        m_inColors = m_pointInputFile.getColors();
        setProgressSettings( m_inVerts->size() * 2 / 3 );
        m_infoInputPointCount->set( m_infoInputPointCount->get() + m_inVerts->size() / 3 );
        initBoundingBox( !addedPoints );
        addTransformedPoints();
        if( m_inVerts->size() > 0 )
            addedPoints = true;
    }
    m_reloadPointsTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );
    return addedPoints;
}
void WMPointsTransform::onFileSave()
{
    if( m_savePointsTrigger->get(true) )
    {
        m_pointOutputFile.setFilePath( m_outputFile->get().c_str() );
        m_pointOutputFile.saveWDataSetPoints( m_outVerts, m_outColors );
    }
    m_savePointsTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );
}
