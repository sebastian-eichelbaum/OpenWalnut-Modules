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
#include "WMPointGroupsTransform.xpm"
#include "WMPointGroupsTransform.h"
#include "../common/datastructures/octree/WOctree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMPointGroupsTransform )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMPointGroupsTransform::WMPointGroupsTransform():
    WModule(),
    m_propCondition( new WCondition() )
{
    m_input.reserve( 7 );
    m_input.resize( 7 );

    m_offsetVector.reserve( 3 );
    m_offsetVector.resize( 3 );
}

WMPointGroupsTransform::~WMPointGroupsTransform()
{
}

boost::shared_ptr< WModule > WMPointGroupsTransform::factory() const
{
    return boost::shared_ptr< WModule >( new WMPointGroupsTransform() );
}

const char** WMPointGroupsTransform::getXPMIcon() const
{
    return WMPointGroupsTransform_xpm;
}
const std::string WMPointGroupsTransform::getName() const
{
    return "Point Groups - Transform";
}

const std::string WMPointGroupsTransform::getDescription() const
{
    return "Crops point data to a selection.";
}

void WMPointGroupsTransform::connectors()
{
    m_input[0] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 1",
            "The first point set to display" );
    m_input[1] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 2",
            "The second point set to display" );
    m_input[2] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 3",
            "The third point set to display" );
    m_input[3] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 4",
            "The fourth point set to display" );
    m_input[4] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 5",
            "The fifth point set to display" );
    m_input[5] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 6",
            "The sixth point set to display" );
    m_input[6] = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(), "Input 7",
            "The seventh point set to display" );
    m_inputSubtraction = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "Subtraction",
            "Points that are subtracted from the output" );


    m_outputGroups = boost::shared_ptr< WModuleOutputData< WDataSetPointsGrouped > >(
                new WModuleOutputData< WDataSetPointsGrouped >(
                        shared_from_this(), "Transformed groups", "The transformed point set." ) );
    addConnector( m_outputGroups );
    m_outputPoints = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                        shared_from_this(), "Point outline", "The transformed point set." ) );
    addConnector( m_outputPoints );
    m_outputVoxels = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
                new WModuleOutputData< WTriangleMesh >( shared_from_this(),
                "Voxel outline.", "Voxel bases point group outline." ) );
    addConnector( m_outputVoxels );

    WModule::connectors();
}

void WMPointGroupsTransform::properties()
{
    m_infoRenderTimeSeconds = m_infoProperties->addProperty( "Wall time (s): ", "Time in seconds that the "
                                            "whole render process took.", 0.0 );
    m_infoInputPointCount = m_infoProperties->addProperty( "Input points: ", "", 0 );
    m_infoLastGroupID = m_infoProperties->addProperty( "Last group ID: ", "", 0 );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "X min: ", "", 0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "X max: ", "", 0 ) );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "Y min: ", "", 0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "Y max: ", "", 0 ) );
    m_infoBoundingBoxMin.push_back( m_infoProperties->addProperty( "Z min: ", "", 0 ) );
    m_infoBoundingBoxMax.push_back( m_infoProperties->addProperty( "Z max: ", "", 0 ) );


    m_pointsCropGroup = m_properties->addPropertyGroup( "Point set cropping",
                                            "Options to crop the point set" );
    m_pointSubtractionRadius = m_pointsCropGroup->addProperty( "Subtr. r.: ", "Radius of subtracted coordinates "
                                                        "(most right input connector)", 0.0, m_propCondition );


    m_outlinerGroup = m_properties->addPropertyGroup( "Group outlining",
                                            "Options of outlining point groups using group ID less output" );
    m_detailDepth = m_outlinerGroup->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0, m_propCondition );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_outlinerGroup->addProperty( "Voxel width meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", pow( 2.0, m_detailDepth->get() ) * 2.0 );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );
    m_highlightUsingColors = m_outlinerGroup->addProperty( "Hilight using colors: ",
                            "Hilights output ddata groups using colors.", true, m_propCondition );
    m_clearInputColor = m_outlinerGroup->addProperty( "Clear input color: ",
                            "Hilights output ddata groups using colors.", false, m_propCondition );

    m_loadAllGroups = m_outlinerGroup->addProperty( "Load all groups: ", "", true, m_propCondition );
    m_selectedShowableGroup = m_outlinerGroup->addProperty( "Showable group ID: ", "", 0, m_propCondition );
    m_selectedShowableGroup->setMin( 0 );
    m_selectedShowableGroup->setMax( 0 );
    m_groupIDInOriginalPointSet = m_outlinerGroup->addProperty( "Orig. group ID: ", "", 0 );
    m_groupIDInOriginalPointSet->setPurpose( PV_PURPOSE_INFORMATION );


    m_pointIDProcessingGroup = m_properties->addPropertyGroup( "Group ID processing", "" );
    m_mergeGroupIDsAllInputs = m_pointIDProcessingGroup->addProperty( "Merge IDs between inputs: ",
                            "Setting makes difference only with many inputs.", true, m_propCondition );
    m_groupSizeThreshold = m_pointIDProcessingGroup->addProperty( "Min. group size: ",
                            "Groups below that size are removed and group IDs decremented.", 0, m_propCondition );
    m_groupSizeThreshold->setMin( 0 );


    m_translatePointsGroup = m_properties->addPropertyGroup( "Point translation",
                                            "Translates the points by the following amount of XYZ offset after cropping." );
    m_offsetVector[0] = m_translatePointsGroup->addProperty( "X offset: ", "Translates the point set across the X axis by "
                                                        "that offset.", 0.0, m_propCondition  );
    m_offsetVector[1] = m_translatePointsGroup->addProperty( "Y offset: ", "Translates the point set across the Y axis by "
                                                        "that offset.", 0.0, m_propCondition  );
    m_offsetVector[2] = m_translatePointsGroup->addProperty( "Z offset: ", "Translates the point set across the Z axis by "
                                                        "that offset.", 0.0, m_propCondition  );

    m_groupFileOperations = m_properties->addPropertyGroup( "File options", "" );
    m_inputFile = m_groupFileOperations->addProperty( "Input path: ", "", WPathHelper::getAppPath() );
    WPropertyHelper::PC_PATHEXISTS::addTo( m_inputFile );
    m_reloadPointsTrigger = m_groupFileOperations->addProperty( "Load points:",  "Load from file", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );
    m_outputFile = m_groupFileOperations->addProperty( "Output path: ", "", WPathHelper::getAppPath() );
    m_savePointsTrigger = m_groupFileOperations->addProperty( "Save points:",  "Save to file", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );


    WModule::properties();
}

void WMPointGroupsTransform::requirements()
{
}

void WMPointGroupsTransform::moduleMain()
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
        timer.reset();
        WDataSetPointsGrouped::VertexArray newVertices(
                new WDataSetPointsGrouped::VertexArray::element_type() );
        m_outVerts = newVertices;
        WDataSetPointsGrouped::ColorArray newColors(
                new WDataSetPointsGrouped::ColorArray::element_type() );
        m_outColors = newColors;
        WDataSetPointsGrouped::ColorArray newGroupColors(
                new WDataSetPointsGrouped::ColorArray::element_type() );
        m_outGroupColors = newGroupColors;
        WDataSetPointsGrouped::GroupArray newGroups(
                new WDataSetPointsGrouped::GroupArray::element_type() );
        m_outGroups = newGroups;

        m_voxelOutliner.setVoxelWidth( m_detailDepthLabel->get() );
        m_groupEditor.initProocessBegin();
        m_groupEditor.setGroupSizeThreshold( m_groupSizeThreshold->get() );
        m_groupEditor.setMergeGroups( m_mergeGroupIDsAllInputs->get() );
        m_pointSubtraction.initSubtraction( m_inputSubtraction->getData(), m_pointSubtractionRadius->get() );
        setProgressSettings( 10 );
        onFileLoad();
        for(size_t pointset = 0; pointset < m_input.size(); pointset++)
        {
            boost::shared_ptr< WDataSetPointsGrouped > points = m_input[pointset]->getData();
            if  ( points )
                m_groupEditor.mergeGroupSet( points );
        }
        setProgressSettings( m_groupEditor.getInputPointCount() + 1 );
        m_groupEditor.modifyGroupIDs();
        addTransformedPoints();
        bool addedPoints = m_outVerts->size();
        for( size_t item = 0; !addedPoints && item < 3; item++ )
        {
            m_outVerts->push_back( 0 );
            m_outColors->push_back( 0 );
            m_outGroupColors->push_back( 0 );
            if( m_outGroups->size() == 0)
                m_outGroups->push_back( 0 );
        }
        if( addedPoints > 0 && m_outVerts->size() > 0 )
        {
            boost::shared_ptr< WDataSetPointsGrouped > outputGroups(
                    new WDataSetPointsGrouped( m_outVerts, m_outColors, m_outGroups ) );
            m_outputGroups->updateData( outputGroups );
            boost::shared_ptr< WDataSetPoints > outputPoints(
                    new WDataSetPoints( m_outVerts, m_outGroupColors ) );
            m_outputPoints->updateData( outputPoints );
            m_outputVoxels->updateData( m_voxelOutliner.getOutline( m_highlightUsingColors->get() ) );

            onFileSave();
        }
        m_selectedShowableGroup->setMax( m_groupEditor.getLastGroupID() );
        m_infoLastGroupID->set( m_groupEditor.getLastGroupID() );
        m_infoInputPointCount->set( m_groupEditor.getInputPointCount() );
        m_infoRenderTimeSeconds->set( timer.elapsed() );
        m_progressStatus->finish();

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
void WMPointGroupsTransform::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
void WMPointGroupsTransform::addTransformedPoints()
{
    vector<double>* point = new vector<double>( 3, 0.0 );
    vector<double> coordsMin( 3, 0.0 );
    vector<double> coordsMax( 3, 0.0 );
    size_t count = m_groupEditor.getInputPointCount();


    for( size_t index = 0; index < count; index++)
    {
        for( size_t dimension = 0; dimension < 3; dimension++)
        {
            point->at( dimension ) = m_groupEditor.getVertex( index, dimension );
            point->at( dimension ) += m_offsetVector[dimension]->get();

            if( point->at( dimension ) < coordsMin[dimension] || index == 0 )
                coordsMin[dimension] = point->at( dimension );
            if( point->at( dimension ) > coordsMax[dimension] || index == 0 )
                coordsMax[dimension] = point->at( dimension );
        }

        size_t group = m_groupEditor.getNewGroupID( index );
        bool isGroupSelected = m_loadAllGroups->get();
        if( static_cast<size_t>( m_selectedShowableGroup->get() ) == group )
        {
            isGroupSelected = true;
            m_groupIDInOriginalPointSet->set( m_groupEditor.getOldGroupID( index ) );
        }
        if( isGroupSelected && m_groupEditor.isPointCollected( index )
                && !m_pointSubtraction.pointsExistNearCoordinate( *point ) )
        {
            for( size_t item = 0; item < 3; item++ )
            {
                m_outVerts->push_back( point->at( item ) );

                double color = m_groupEditor.getColor( index, item );
                m_outColors->push_back( color );

                double groupOutlineColor = color;
                if( m_clearInputColor->get() )
                    groupOutlineColor = 1.0;
                if( m_highlightUsingColors->get() )
                    groupOutlineColor *= WOctree::calcColor( group, item );
                m_outGroupColors->push_back( groupOutlineColor );
            }
            m_outGroups->push_back( m_groupEditor.getNewGroupID( index ) );

            m_voxelOutliner.registerPoint( point->at( 0 ), point->at( 1 ), point->at( 2 ) );
            m_voxelOutliner.getOctreeLeafNode( point->at( 0 ), point->at( 1 ), point->at( 2 ) )->setGroupNr( group );
        }
        m_progressStatus->increment( 1 );
    }
    for( size_t dimension = 0; dimension < 3; dimension++ )
        m_infoBoundingBoxMin[dimension]->set( coordsMin[dimension] );
    for( size_t dimension = 0; dimension < 3; dimension++ )
        m_infoBoundingBoxMax[dimension]->set( coordsMax[dimension] );
}
void WMPointGroupsTransform::onFileLoad()
{
    cout << endl << endl << "WMGroupsTransform::onFileLoad() - Start" << endl;
    m_pointInputFile.setFilePath( m_inputFile->get().c_str() );
    if( m_reloadPointsTrigger->get( true ) )
        m_pointInputFile.loadWDataSetPointsGrouped();
    if( m_pointInputFile.containsData() )
    {
        cout << "WMGroupsTransform::onFileLoad() - File detected" << endl;
        boost::shared_ptr< WDataSetPointsGrouped > outputGroups(
                new WDataSetPointsGrouped( m_pointInputFile.getVertices(),
                m_pointInputFile.getColors(), m_pointInputFile.getGroups() ) );
        m_groupEditor.mergeGroupSet( outputGroups );
    }
    m_reloadPointsTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );
}
void WMPointGroupsTransform::onFileSave()
{
    if( m_savePointsTrigger->get(true) )
    {
        m_pointOutputFile.setFilePath( m_outputFile->get().c_str() );
        m_pointOutputFile.saveWDataSetPointsGrouped( m_outVerts, m_outColors, m_outGroups );
    }
    m_savePointsTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );
}
