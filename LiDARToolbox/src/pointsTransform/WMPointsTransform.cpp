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
#include <limits>

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

WMPointsTransform::WMPointsTransform():
    WModule(),
    m_propCondition( new WCondition() )
{
    m_input.reserve( 7 );
    m_input.resize( 7 );

    m_minCoord.reserve( 3 );
    m_minCoord.resize( 3 );
    m_maxCoord.reserve( 3 );
    m_maxCoord.resize( 3 );
    m_minColorIntensity.reserve( 3 );
    m_minColorIntensity.resize( 3 );
    m_maxColorIntensity.reserve( 3 );
    m_maxColorIntensity.resize( 3 );
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
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "X min: ", "", 0.0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "X max: ", "", 0.0 ) );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "Y min: ", "", 0.0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "Y max: ", "", 0.0 ) );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "Z min: ", "", 0.0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "Z max: ", "", 0.0 ) );
    m_infoColorMin.push_back( m_infoProperties->addProperty( "Red min:", "", 0.0 ) );
    m_infoColorMax.push_back( m_infoProperties->addProperty( "Red max:", "", 0.0 ) );
    m_infoColorMin.push_back( m_infoProperties->addProperty( "Green min:", "", 0.0 ) );
    m_infoColorMax.push_back( m_infoProperties->addProperty( "Green max:", "", 0.0 ) );
    m_infoColorMin.push_back( m_infoProperties->addProperty( "Blue min:", "", 0.0 ) );
    m_infoColorMax.push_back( m_infoProperties->addProperty( "Blue max:", "", 0.0 ) );


    double number_range = 1000000000.0;
    m_pointsCropGroup = m_properties->addPropertyGroup( "Point set cropping",
                                            "Options to crop the point set" );
    m_fromCoord.push_back( m_pointsCropGroup->addProperty( "X min.: ", "Cut boundary.", -number_range, m_propCondition ) );
    m_toCoord.push_back( m_pointsCropGroup->addProperty( "X max.: ", "Cut boundary.", number_range, m_propCondition ) );
    m_fromCoord.push_back( m_pointsCropGroup->addProperty( "Y min.: ", "Cut boundary.", -number_range, m_propCondition ) );
    m_toCoord.push_back( m_pointsCropGroup->addProperty( "Y max.: ", "Cut boundary.", number_range, m_propCondition ) );
    m_fromCoord.push_back( m_pointsCropGroup->addProperty( "Z min.: ", "Cut boundary.", -number_range, m_propCondition ) );
    m_toCoord.push_back( m_pointsCropGroup->addProperty( "Z max.: ", "Cut boundary.", number_range, m_propCondition ) );
    m_invertCropping = m_pointsCropGroup->addProperty( "Invert crop: ", "Cut instead of crop.", false, m_propCondition );
    m_disablePointCrop = m_pointsCropGroup->addProperty( "Disable cuts: ", "", false, m_propCondition );
    m_pointSubtractionRadius = m_pointsCropGroup->addProperty( "Subtr. r.: ", "Radius of subtracted coordinates "
                                                        "(most right input connector)", 0.0, m_propCondition );
    m_invertSubtraction = m_pointsCropGroup->addProperty( "Invert subtr.: ", "Inverts point subtraction.", false, m_propCondition );

    m_translatePointsGroup = m_properties->addPropertyGroup( "Point translation",
                                            "Translates the points by the following amount of XYZ offset after cropping." );
    m_translationOffset.push_back( m_translatePointsGroup->addProperty( "X offset: ",
            "Translates the point set across the X axis by that offset.", 0.0, m_propCondition  ) );
    m_translationOffset.push_back( m_translatePointsGroup->addProperty( "Y offset: ",
            "Translates the point set across the Y axis by that offset.", 0.0, m_propCondition  ) );
    m_translationOffset.push_back( m_translatePointsGroup->addProperty( "Z offset: ",
            "Translates the point set across the Z axis by that offset.", 0.0, m_propCondition  ) );

    m_groupMultiplyPoints = m_properties->addPropertyGroup( "Coordinate multiplication",
                                            "Multiplies each X, Y and Z value by a factor." );
    m_coordFactor.push_back( m_groupMultiplyPoints->addProperty( "X factor: ",
            "Number which multiplies each X coordinate.", 1.0, m_propCondition ) );
    m_coordFactor.push_back( m_groupMultiplyPoints->addProperty( "Y factor: ",
            "Number which multiplies each X coordinate.", 1.0, m_propCondition ) );
    m_coordFactor.push_back( m_groupMultiplyPoints->addProperty( "Z factor: ",
            "Number which multiplies each X coordinate.", 1.0, m_propCondition ) );

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
    m_rotationAnchor.push_back( m_groupRotation->addProperty( "X anchor: ",
            "X coordinate of the rotation anchor.", 0.0, m_propCondition ) );
    m_rotationAnchor.push_back( m_groupRotation->addProperty( "Y anchor: ",
            "Y coordinate of the rotation anchor.", 0.0, m_propCondition ) );
    m_rotationAnchor.push_back( m_groupRotation->addProperty( "Z anchor: ",
            "Z coordinate of the rotation anchor.", 0.0, m_propCondition ) );

    m_skipRatio = m_groupRotation->addProperty( "Point skip ratio: ", "Point count that is skipped after "
                                                "adding one point.", 0, m_propCondition  );
    m_skipRatio->setMin( 0 );
    m_skipRatio->setMax( 30 );



    m_groupColorEqualizer = m_properties->addPropertyGroup( "Color equalizer", "" );
    m_contrast.push_back( m_groupColorEqualizer->addProperty( "Contrast red: ", "", 1.0, m_propCondition ) );
    m_colorOffset.push_back( m_groupColorEqualizer->addProperty( "Offset red: ", "", 0.0, m_propCondition ) );
    m_contrast.push_back( m_groupColorEqualizer->addProperty( "Contrast green: ", "", 1.0, m_propCondition ) );
    m_colorOffset.push_back( m_groupColorEqualizer->addProperty( "Offset green: ", "", 0.0, m_propCondition ) );
    m_contrast.push_back( m_groupColorEqualizer->addProperty( "Contrast blue: ", "", 1.0, m_propCondition ) );
    m_colorOffset.push_back( m_groupColorEqualizer->addProperty( "Offset blue: ", "", 0.0, m_propCondition ) );
    for( size_t color = 0; color < m_contrast.size(); color++ )
    {
        m_contrast[color]->setMin( 0.0 );
        m_contrast[color]->setMax( 3.0 );
        m_colorOffset[color]->setMin( -1.0 );
        m_colorOffset[color]->setMax( 1.0 );
    }
    boost::shared_ptr< WItemSelection > statisticalTypes( boost::shared_ptr< WItemSelection >( new WItemSelection() ) );
    statisticalTypes->addItem( "Automatical", "" );
    statisticalTypes->addItem( "Automatical, only contrast", "" );
    statisticalTypes->addItem( "Manual", "" );
    statisticalTypes->addItem( "Manual, joined", "" );
    statisticalTypes->addItem( "Manual, unbounded", "" );
    statisticalTypes->addItem( "Manual, unbounded, joined", "" );
    m_colorAdjustmentType = m_properties->addProperty( "Adjustment", "",
                                                 statisticalTypes->getSelector( M_COLOR_MANUAL_JOINED ), m_propCondition );
    WPropertyHelper::PC_SELECTONLYONE::addTo( m_colorAdjustmentType );

    boost::shared_ptr< WItemSelection > colorTypes( boost::shared_ptr< WItemSelection >( new WItemSelection() ) );
    colorTypes->addItem( "Colored", "" );
    colorTypes->addItem( "Greyscale - Perceptional", "" );
    colorTypes->addItem( "Greyscale - Proportional", "" );
    m_colorModeType = m_properties->addProperty( "Color mode", "",
            colorTypes->getSelector( M_COLOR_MODE_COLORED ), m_propCondition );
    WPropertyHelper::PC_SELECTONLYONE::addTo( m_colorModeType );


    m_groupFileOperations = m_properties->addPropertyGroup( "File proocessor", "" );
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
const size_t WMPointsTransform::M_COLOR_AUTO = 0;
const size_t WMPointsTransform::M_COLOR_AUTO_ONLY_CONTRAST = M_COLOR_AUTO + 1;
const size_t WMPointsTransform::M_COLOR_MANUAL = M_COLOR_AUTO_ONLY_CONTRAST + 1;
const size_t WMPointsTransform::M_COLOR_MANUAL_JOINED = M_COLOR_MANUAL + 1;
const size_t WMPointsTransform::M_COLOR_MANUAL_UNBOUNDED = M_COLOR_MANUAL_JOINED + 1;
const size_t WMPointsTransform::M_COLOR_MANUAL_UNBOUNDED_JOINED = M_COLOR_MANUAL_UNBOUNDED + 1;

const size_t WMPointsTransform::M_COLOR_MODE_COLORED = 0;
const size_t WMPointsTransform::M_COLOR_MODE_GREYSCALE_PERCEPTIONAL = M_COLOR_MODE_COLORED + 1;
const size_t WMPointsTransform::M_COLOR_MODE_GREYSCALE_PROPORTIONAL =
        M_COLOR_MODE_GREYSCALE_PERCEPTIONAL + 1;

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
        onColorIntensityCorrect();
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
        bool isFirstItem = index == 0 && isFirstPointSet;
        for( size_t dimension = 0; dimension < 3; dimension++ )
        {
            double coordinate = m_inVerts->at( index * 3 + dimension );
            if( coordinate < m_minCoord[dimension] || isFirstItem )
                m_minCoord[dimension] = coordinate;
            if( coordinate > m_maxCoord[dimension] || isFirstItem )
                m_maxCoord[dimension] = coordinate;

            double intensity = m_inColors->at( index * 3 + dimension );
            if( intensity < m_minColorIntensity[dimension] || isFirstItem )
                m_minColorIntensity[dimension] = intensity;
            if( intensity > m_maxColorIntensity[dimension] || isFirstItem )
                m_maxColorIntensity[dimension] = intensity;
        }
        m_progressStatus->increment( 1 );
    }
}

void WMPointsTransform::setMinMax()
{
    for( size_t dimension = 0; dimension < m_minCoord.size(); dimension++ )
    {
        m_fromCoord[dimension]->setMin( m_minCoord[dimension] );
        m_fromCoord[dimension]->setMax( m_maxCoord[dimension] );
        m_toCoord[dimension]->setMin( m_fromCoord[dimension]->get() );
        m_toCoord[dimension]->setMax( m_maxCoord[dimension] );

        m_infoBoundingBoxMin[dimension]->set( m_minCoord[dimension] );
        m_infoBoundingBoxMax[dimension]->set( m_maxCoord[dimension] );

        m_infoColorMin[dimension]->set( m_minColorIntensity[dimension] );
        m_infoColorMax[dimension]->set( m_maxColorIntensity[dimension] );
    }
}

void WMPointsTransform::addTransformedPoints()
{
    double angleXY = m_rotation1AngleXY->get();
    double angleYZ = m_rotation2AngleYZ->get();
    double angleXZ = m_rotation3AngleXZ->get();
    vector<double>* point = new vector<double>( 3, 0.0 );
    vector<double> color( 3, 0.0 );
    size_t count = m_inVerts->size() / 3;
    vector<double> offset = WVectorMaths::new3dVector( m_translationOffset[0]->get(),
            m_translationOffset[1]->get(), m_translationOffset[2]->get() );
    vector<double> factor = WVectorMaths::new3dVector( m_coordFactor[0]->get(),
            m_coordFactor[1]->get(), m_coordFactor[2]->get() );
    vector<double> rotationAnchor = WVectorMaths::new3dVector( m_rotationAnchor[0]->get(),
            m_rotationAnchor[1]->get(), m_rotationAnchor[2]->get() );
    vector<double>* rotationAnchorInverted = WVectorMaths::copyVectorForPointer( rotationAnchor );
    WVectorMaths::invertVector( rotationAnchorInverted );
    vector<double> contrast = WVectorMaths::new3dVector( m_contrast[0]->get(),
            m_contrast[1]->get(), m_contrast[2]->get() );
    vector<double> colorOffset = WVectorMaths::new3dVector( m_colorOffset[0]->get(),
            m_colorOffset[1]->get(), m_colorOffset[2]->get() );
    size_t colorMode = m_colorModeType->get().getItemIndexOfSelected( 0 );
    for( size_t index = 0; index < count; index++)
    {
        point->at( 0 ) = m_inVerts->at( index * 3 );
        point->at( 1 ) = m_inVerts->at( index * 3 + 1 );
        point->at( 2 ) = m_inVerts->at( index * 3 + 2 );
        bool isInsideSelection = true;
        for( size_t dimension = 0; dimension < 3 && isInsideSelection; dimension++ )
            isInsideSelection = point->at( dimension ) >= m_fromCoord[dimension]->get()
                    && point->at( dimension ) <= m_toCoord[dimension]->get();
        size_t modulo = m_skipRatio->get() + 1;
        bool isPointSkipped = index % ( modulo ) != 0;
        bool remainsAfterCropping = isInsideSelection != m_invertCropping->get() || m_disablePointCrop->get();
        bool remainsAafterSubtraction = m_pointSubtraction.pointsExistNearCoordinate( *point ) == m_invertSubtraction->get();
        if( remainsAfterCropping && !isPointSkipped && remainsAafterSubtraction )
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
                color[item] = m_inColors->at( index * 3 + item ) * contrast[item] + colorOffset[item];
            }
            double intensity = colorMode == M_COLOR_MODE_GREYSCALE_PERCEPTIONAL
                    ?( color[0]*0.3 + color[1]*0.59 + color[2]*0.11 )
                    :( ( color[0] + color[1] + color[2] ) / 3.0 );
            for( size_t item = 0; item < 3; item++ )
                m_outColors->push_back( colorMode == M_COLOR_MODE_COLORED ?color[item] :intensity );
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
void WMPointsTransform::onColorIntensityCorrect()
{
    double intensityMin = m_infoColorMin[0]->get(), intensityMax = m_infoColorMax[0]->get();
    for( size_t dimension = 1; dimension < m_infoColorMin.size(); dimension++ )
    {
        if( m_infoColorMin[dimension]->get() < intensityMin )
            intensityMin = m_infoColorMin[dimension]->get();
        if( m_infoColorMax[dimension]->get() > intensityMax )
            intensityMax = m_infoColorMax[dimension]->get();
    }
    size_t type = m_colorAdjustmentType->get().getItemIndexOfSelected( 0 );
    bool isBounded = type == M_COLOR_MANUAL || type == M_COLOR_MANUAL_JOINED;
    bool isJoined = type == M_COLOR_MANUAL_JOINED || type == M_COLOR_MANUAL_UNBOUNDED_JOINED;
    bool isAutomatical = type == M_COLOR_AUTO || M_COLOR_AUTO_ONLY_CONTRAST;

    double contrast = m_contrast[0]->get();
    double offset = m_colorOffset[0]->get();
    size_t adjustedRow = 10000;
    for( size_t color = 0; color < m_infoColorMin.size(); color++ )
    {
        if( m_contrast[color]->changed( true ) )
            contrast = m_contrast[adjustedRow = color]->get();
        if( m_colorOffset[color]->changed( true ) )
            offset = m_colorOffset[adjustedRow = color]->get();
    }

    for( size_t color = 0; color < m_infoColorMin.size(); color++)
    {
        m_contrast[color]->setMin( isBounded ?0.0 :-numeric_limits<double>::max() );
        m_contrast[color]->setMax( isBounded ?3.0 :numeric_limits<double>::max() );
        m_colorOffset[color]->setMin( isBounded ?-1.0 :-numeric_limits<double>::max() );
        m_colorOffset[color]->setMax( isBounded ?1.0 :numeric_limits<double>::max() );

        if( type == M_COLOR_AUTO )
        {
            double slope = intensityMax - intensityMin;
            contrast = slope > 0.0 ?( 1.0 / slope ) :0.5;
            offset = slope > 0.0 ?( - contrast * intensityMin ) :0.5;
        }

        if( type == M_COLOR_AUTO_ONLY_CONTRAST )
        {
            double slope = intensityMax > 0.0 ?intensityMax :0.0;
            contrast = slope > 0.0 ?( 1.0 / slope ) :0.5;
            offset = slope > 0.0 ?0.0 :0.5;
        }

        if( ( isJoined && color != adjustedRow ) || isAutomatical )
        {
            m_contrast[color]->set( contrast );
            m_colorOffset[color]->set( offset );
            m_contrast[color]->changed( isJoined );
            m_colorOffset[color]->changed( isJoined );
        }
    }
}
