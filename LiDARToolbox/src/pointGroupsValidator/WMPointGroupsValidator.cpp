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
#include "WMPointGroupsValidator.xpm"
#include "WMPointGroupsValidator.h"
#include "../common/datastructures/octree/WOctree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMPointGroupsValidator )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMPointGroupsValidator::WMPointGroupsValidator():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMPointGroupsValidator::~WMPointGroupsValidator()
{
}

boost::shared_ptr< WModule > WMPointGroupsValidator::factory() const
{
    return boost::shared_ptr< WModule >( new WMPointGroupsValidator() );
}

const char** WMPointGroupsValidator::getXPMIcon() const
{
    return WMPointGroupsValidator_xpm;
}
const std::string WMPointGroupsValidator::getName() const
{
    return "Point Groups - Validator";
}

const std::string WMPointGroupsValidator::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMPointGroupsValidator::connectors()
{
    m_inputReferenceGroupPoints = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(),
            "Reference groups", "Points that depict an ideal state" );
    m_inputValidatedGroupPoints = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(),
            "Validated groups", "Points from a method to be validated" );


    m_outputNotSegmented = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
            new WModuleOutputData< WDataSetPoints >(
                    shared_from_this(), "Not segmented", "Points in reference that were not correctly segmented" ) );
    addConnector( m_outputNotSegmented );
    m_outputPointsOfMissingAreas = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
            new WModuleOutputData< WDataSetPoints >( shared_from_this(), "Missing areas",
                    "Reference points that are farer ""than by a threshold distance from validated points" ) );
    addConnector( m_outputPointsOfMissingAreas );
    m_outputFalseSegmented = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
            new WModuleOutputData< WDataSetPoints >(
                    shared_from_this(), "False groups", "Points that got the wrong group ID" ) );
    addConnector( m_outputFalseSegmented );


    WModule::connectors();
}

void WMPointGroupsValidator::properties()
{
    m_accuracyGroup = m_properties->addPropertyGroup( "Validated data accuracy",
            "Accuracy properties on which validated data may differ from reference data." );
    m_coordinateAccuracy = m_accuracyGroup->addProperty( "Point accuracy: ",
            "Distanse on which validated points may vary from reference points by coordinate.", 0.0, m_propCondition );
    m_coordinateAccuracy->setMin( 0.0 );
    m_coordinateAccuracy->setMax( 0.1 );


    m_detectionCertaintyGroup = m_properties->addPropertyGroup( "Properties of certainly detected surfaces",
            "Settings relevant to decide whether a point group was detected certainly." );
    m_pointAreaRadius = m_detectionCertaintyGroup->addProperty( "Point area radius: ",
            "Minimal distance from all detected points to be detected as not detected area.", 0.0, m_propCondition );
    m_pointAreaRadius->setMin( 0.0 );
    m_pointAreaRadius->setMax( 0.75 );
    m_minimalPointCompleteness = m_detectionCertaintyGroup->addProperty( "Min. completeness: ", "", 0.3, m_propCondition );
    m_minimalPointCompleteness->setMin( 0.0 );
    m_minimalPointCompleteness->setMax( 1.0 );
    m_minimalAreaPointCompleteness = m_detectionCertaintyGroup->addProperty( "Min. area compl.: ", "Matching type that is "
            "applied to figure out the amount of minimal positives.", 0.0, m_propCondition );
    m_minimalAreaPointCompleteness->setMin( 0.0 );
    m_minimalAreaPointCompleteness->setMax( 1.0 );
    m_minimalPointCorectness = m_detectionCertaintyGroup->addProperty( "Min. correctness: ", "", 0.0, m_propCondition );
    m_minimalPointCorectness->setMin( 0.0 );
    m_minimalPointCorectness->setMax( 1.0 );


    m_evaluationGroup = m_properties->addPropertyGroup( "Group evaluation settings",
            "Main evaluation settings." );
    m_cumulateGroups = m_evaluationGroup->addProperty( "Cumulate groups: ",
            "Merge group results with a simular group size.", true );
    m_outputFileCSV = m_evaluationGroup->addProperty( "CSV table: ", "", WPathHelper::getAppPath() );
    m_saveCSVTrigger = m_evaluationGroup->addProperty( "Export CSV:",  "Export", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    WModule::properties();
}

void WMPointGroupsValidator::requirements()
{
}

void WMPointGroupsValidator::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_inputReferenceGroupPoints->getDataChangedCondition() );
    m_moduleState.add( m_inputValidatedGroupPoints->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    // main loop
    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();

        boost::shared_ptr< WDataSetPointsGrouped > referenceGroups = m_inputReferenceGroupPoints->getData();
        boost::shared_ptr< WDataSetPointsGrouped > validatedPoints = m_inputValidatedGroupPoints->getData();
//        std::cout << "Execute cycle\r\n";
        if  ( referenceGroups && validatedPoints )
        {
            setProgressSettings( 10 );

            m_groupValidator.setCoordinateAccuracy( m_coordinateAccuracy->get() );
            m_groupValidator.setPointAreaRadius( m_pointAreaRadius->get() );
            m_groupValidator.setMinimalPointCompleteness( m_minimalPointCompleteness->get() );
            m_groupValidator.setMinimalpointAreaCompleteness( m_minimalAreaPointCompleteness->get() );
            m_groupValidator.setMinimalPointCorrectness( m_minimalPointCorectness->get() );
            m_groupValidator.validateGroups( referenceGroups, validatedPoints );

            m_outputNotSegmented->updateData( m_groupValidator.getOutlineNotSegmentedPoints() );
            m_outputFalseSegmented->updateData( m_groupValidator.getOutlinePointsBelongingForeignGroups() );
            m_outputPointsOfMissingAreas->updateData( m_groupValidator.getOutlineNotSegmentedGroupAreas() );

            m_exportCSV.setExportFilePath( m_outputFileCSV->get().c_str() );
            m_exportCSV.setGroupInfo( m_groupValidator.getGroupInfo() );
            m_exportCSV.setCumulateResultGroups( m_cumulateGroups->get() );
            if( m_saveCSVTrigger->get(true) )
                m_exportCSV.exportCSV();
            m_saveCSVTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, true );

            m_progressStatus->finish();
        }


        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPointsGrouped > points1 = m_inputReferenceGroupPoints->getData();
        boost::shared_ptr< WDataSetPointsGrouped > points2 = m_inputValidatedGroupPoints->getData();
        if  ( !points1 || !points2 )
        {
            continue;
        }
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}





void WMPointGroupsValidator::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}

