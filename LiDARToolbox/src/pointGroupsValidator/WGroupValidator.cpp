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

#include <iostream>
#include <vector>
#include "WGroupValidator.h"

WGroupValidator::WGroupValidator()
{
    m_groupInfo = new vector<WGroupInfo*>();
}
WGroupValidator::~WGroupValidator()
{
}



boost::shared_ptr< WDataSetPoints > WGroupValidator::getOutlineNotSegmentedPoints()
{
    if( m_notSegmentedPointsVertices->size() == 0 )
        return getEmptyShowablePointSet();

    boost::shared_ptr< WDataSetPoints > notDetectedPointset( new WDataSetPoints(
            m_notSegmentedPointsVertices, m_notSegmentedPointsColors ) );
    return notDetectedPointset;
}

boost::shared_ptr< WDataSetPoints > WGroupValidator::getOutlinePointsBelongingForeignGroups()
{
    if( m_falseSegmentedVertices->size() == 0 )
        return getEmptyShowablePointSet();

    boost::shared_ptr< WDataSetPoints > wronglyDetectedPointset(
            new WDataSetPoints( m_falseSegmentedVertices, m_falseSegmentedColors ) );
    return wronglyDetectedPointset;
}

boost::shared_ptr< WDataSetPoints > WGroupValidator::getOutlineNotSegmentedGroupAreas()
{
    if( m_pointsOfNotSegmentedAreasVertices->size() == 0 )
        return getEmptyShowablePointSet();

    boost::shared_ptr< WDataSetPoints > pointsOfMissingAreas(
            new WDataSetPoints( m_pointsOfNotSegmentedAreasVertices, m_pointsOfNotSegmentedAreasColors ) );
    return pointsOfMissingAreas;
}

vector<WGroupInfo*>* WGroupValidator::getGroupInfo()
{
    return m_groupInfo;
}

void WGroupValidator::setCoordinateAccuracy( double maxEuclideanDeviance )
{
    m_coordinateAccuracy = maxEuclideanDeviance;
}

void WGroupValidator::setPointAreaRadius( double pointAreaRadius )
{
    m_pointAreaRadius = pointAreaRadius;
}

void WGroupValidator::setMinimalPointCompleteness( double minPointCompleteness )
{
    m_minimalPointCompleteness = minPointCompleteness;
}

void WGroupValidator::setMinimalpointAreaCompleteness( double minPointAreaCompleteness )
{
    m_minimalPointAreaCompleteness = minPointAreaCompleteness;
}

void WGroupValidator::setMinimalPointCorrectness( double minPointCorrectness )
{
    m_minimalPointCorrectness = minPointCorrectness;
}


void WGroupValidator::validateGroups(
    boost::shared_ptr< WDataSetPointsGrouped > referenceGroups, boost::shared_ptr< WDataSetPointsGrouped > validatedGroups )
{
    delete m_groupInfo;
    m_groupInfo = new vector<WGroupInfo*>();

    m_referencePoints = referenceGroups;
    m_validatedPoints = validatedGroups;

    WDataSetPointsGrouped::VertexArray correctlySegmentedVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray correctlySegmentedColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    m_correctlySegmentedVertices = correctlySegmentedVertices;
    m_correctlySegmentedColors = correctlySegmentedColors;

    WDataSetPointsGrouped::VertexArray falseSegmentedVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray falseSegmentedColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    m_falseSegmentedVertices = falseSegmentedVertices;
    m_falseSegmentedColors = falseSegmentedColors;

    WDataSetPointsGrouped::VertexArray areaNotSegmentedVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray areaNotSegmentedColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    m_pointsOfNotSegmentedAreasVertices = areaNotSegmentedVertices;
    m_pointsOfNotSegmentedAreasColors = areaNotSegmentedColors;

    m_referenceGroupEditor.initProocessBegin();
    m_referenceGroupEditor.setMergeGroups( false );
    m_referenceGroupEditor.setGroupSizeThreshold( 0 );
    m_referenceGroupEditor.mergeGroupSet( m_referencePoints );


    m_referenceVertices = m_referencePoints->getVertices();
    m_referenceColors = m_referencePoints->getColors();
    m_referenceGroups = m_referencePoints->getGroups();
    m_validatedVertices = m_validatedPoints->getVertices();
    m_validatedColors = m_validatedPoints->getColors();
    m_validatedGroups = m_validatedPoints->getGroups();


    setProgressSettings( m_referenceGroupEditor.getLastGroupID() );
    for( size_t refGroup = 0; refGroup <= m_referenceGroupEditor.getLastGroupID(); refGroup++ )
    {
        boost::shared_ptr< WDataSetPoints > referenceGroup = getPointsOfGroup( m_referencePoints, refGroup );
        size_t bestGroupIDInValidated = getBestMatchingGroupID( referenceGroup );
        validateGroup( refGroup, bestGroupIDInValidated );

        m_progressStatus->increment( 1 );
    }

    identifyNotSegmentedGroupPoints();
    m_progressStatus->finish();
}





void WGroupValidator::validateGroup( size_t referenceGroupID, size_t validatedGroupID )
{
    cout << "WGroupValidator::validateGroup() - " << referenceGroupID << ", " << validatedGroupID << endl;
    boost::shared_ptr< WDataSetPoints > referenceGroup = getPointsOfGroup( m_referencePoints, referenceGroupID );
    WDataSetPoints::VertexArray referenceVertices = referenceGroup->getVertices();
    WDataSetPoints::ColorArray referenceColors = referenceGroup->getColors();
    boost::shared_ptr< WDataSetPoints > validatedGroup = getPointsOfGroup( m_validatedPoints, validatedGroupID );
    WDataSetPoints::VertexArray validatedVertices = validatedGroup->getVertices();
    WDataSetPoints::ColorArray validatedColors = validatedGroup->getColors();

    WPointSubtactionHelper referenceSearcher;
    referenceSearcher.initSubtraction( referenceGroup, m_coordinateAccuracy );
    WPointSubtactionHelper validatedSearcher;
    validatedSearcher.initSubtraction( validatedGroup, m_coordinateAccuracy );
    WPointSubtactionHelper validatedAreaSearcher;
    validatedAreaSearcher.initSubtraction( validatedGroup, m_pointAreaRadius * 2.0 );

    cout << "WGroupValidator::validateGroup() - Calculating completeness";
    size_t completenessPointCount = 0;
    size_t pointCountOfMissingAreas = 0;
    for( size_t index = 0; index < referenceVertices->size() / 3; index++ )
    {
        vector<double> refCoord = WVectorMaths::new3dVector( referenceVertices->at( index * 3 + 0 ),
                referenceVertices->at( index * 3 + 1 ), referenceVertices->at( index * 3 + 2 ) );

        if( !validatedAreaSearcher.pointsExistNearCoordinate( refCoord ) )
        {
            pointCountOfMissingAreas++;
            for( size_t dimension = 0; dimension < 3; dimension++ )
                m_pointsOfNotSegmentedAreasVertices->push_back( referenceVertices->at( index * 3 + dimension ) );
            m_pointsOfNotSegmentedAreasColors->push_back( 0.65 );
            m_pointsOfNotSegmentedAreasColors->push_back( 0.0 );
            m_pointsOfNotSegmentedAreasColors->push_back( 1.0 );
        }
        if( validatedSearcher.pointsExistNearCoordinate( refCoord ) )
        {
            completenessPointCount++;

            for( size_t dimension = 0; dimension < 3; dimension++ )
            {
                m_correctlySegmentedVertices->push_back( referenceVertices->at( index * 3 + dimension ) );
                m_correctlySegmentedColors->push_back( 1.0 );
            }
        }
    }

    cout << "WGroupValidator::validateGroup() - Calculating correctness";
    size_t uncorrectPoints = 0;
    for( size_t index = 0; index < validatedVertices->size() / 3; index++ )
    {
        vector<double> validatedCoord = WVectorMaths::new3dVector( validatedVertices->at( index * 3 + 0 ),
                validatedVertices->at( index * 3 + 1 ), validatedVertices->at( index * 3 + 2 ) );
        if( !referenceSearcher.pointsExistNearCoordinate( validatedCoord ) )
            uncorrectPoints++;
    }

    while( m_groupInfo->size() <= referenceGroupID )
        m_groupInfo->push_back( new WGroupInfo() );
    WGroupInfo* newGroup = m_groupInfo->at( referenceGroupID );
    newGroup->setReferenceGroupID( referenceGroupID );
    newGroup->setValidatedGroupID( validatedGroupID );
    newGroup->setReferenceGroupPointCount( referenceVertices->size() / 3 );
    newGroup->setCorrectlyDetectedPointCount( completenessPointCount );
    newGroup->setUncorrectlyDetectedPointCount( uncorrectPoints );
    newGroup->setPointCountOfMissingAreas( pointCountOfMissingAreas );

    newGroup->setMinimalPointCompleteness( m_minimalPointCompleteness );
    newGroup->setMinimalPointAreaCompleteness( m_minimalPointAreaCompleteness );
    newGroup->setMinimalPointCorrectness( m_minimalPointCorrectness );

    if( newGroup->isCertainlyDetected() )
        for( size_t index = 0; index < validatedVertices->size() / 3; index++ )
        {
            vector<double> validatedCoord = WVectorMaths::new3dVector( validatedVertices->at( index * 3 + 0 ),
                    validatedVertices->at( index * 3 + 1 ), validatedVertices->at( index * 3 + 2 ) );
            if( !referenceSearcher.pointsExistNearCoordinate( validatedCoord ) )
            {
                for( size_t dimension = 0; dimension < 3; dimension++ )
                    m_falseSegmentedVertices->push_back( validatedVertices->at( index * 3 + dimension ) );
                m_falseSegmentedColors->push_back( 1.0 );
                m_falseSegmentedColors->push_back( 0.0 );
                m_falseSegmentedColors->push_back( 0.0 );
            }
        }
}

void WGroupValidator::identifyNotSegmentedGroupPoints()
{
    WPointSubtactionHelper correctlyDetected;
    WPointSubtactionHelper wronglyDetected;
    WPointSubtactionHelper notDetectedAreas;

    if( m_correctlySegmentedVertices->size() > 0 )
    {
        boost::shared_ptr< WDataSetPoints > detectedPoints(
                new WDataSetPoints( m_correctlySegmentedVertices, m_correctlySegmentedColors ) );
        correctlyDetected.initSubtraction( detectedPoints, m_coordinateAccuracy );
    }
    if( m_falseSegmentedVertices->size() > 0 )
    {
        boost::shared_ptr< WDataSetPoints > falseGroupPoints(
                new WDataSetPoints( m_falseSegmentedVertices, m_falseSegmentedColors ) );
        wronglyDetected.initSubtraction( falseGroupPoints, m_coordinateAccuracy );
    }

    WDataSetPoints::VertexArray notDetectedVertices( new WDataSetPoints::VertexArray::element_type() );
    m_notSegmentedPointsVertices = notDetectedVertices;
    WDataSetPoints::ColorArray notDetectedColors( new WDataSetPoints::ColorArray::element_type() );
    m_notSegmentedPointsColors = notDetectedColors;

    for( size_t index = 0; index < m_referenceVertices->size() / 3; index++ )
    {
        vector<double> refCoord = WVectorMaths::new3dVector( m_referenceVertices->at( index * 3 + 0 ),
                m_referenceVertices->at( index * 3 + 1 ), m_referenceVertices->at( index * 3 + 2 ) );
        bool refHitsCorrectlySegmentedPoint = m_correctlySegmentedVertices->size() > 0
                && correctlyDetected.pointsExistNearCoordinate( refCoord );
        bool refHitsNotCorrectlySegmentedPoint = m_falseSegmentedVertices->size() > 0
                && wronglyDetected.pointsExistNearCoordinate( refCoord );
        if( !refHitsCorrectlySegmentedPoint && !refHitsNotCorrectlySegmentedPoint )
        {
            for( size_t dimension = 0; dimension < 3; dimension++ )
                m_notSegmentedPointsVertices->push_back( m_referenceVertices->at( index * 3 + dimension ) );
            m_notSegmentedPointsColors->push_back( 0.0 );
            m_notSegmentedPointsColors->push_back( 0.0 );
            m_notSegmentedPointsColors->push_back( 1.0 );
        }
    }
}

size_t WGroupValidator::getBestMatchingGroupID( boost::shared_ptr< WDataSetPoints > referenceGroup )
{
    WDataSetPointsGrouped::VertexArray validatedVerts(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray validatedColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    WDataSetPointsGrouped::GroupArray validatedGroups(
            new WDataSetPointsGrouped::GroupArray::element_type() );

    WPointSubtactionHelper referencePoints;
    referencePoints.initSubtraction( referenceGroup, m_coordinateAccuracy );

    for( size_t index = 0; index < m_validatedVertices->size() / 3; index++ )
    {
        vector<double> coordinate = WVectorMaths::new3dVector( m_validatedVertices->at( index * 3 + 0 ),
                m_validatedVertices->at( index * 3 + 1 ), m_validatedVertices->at( index * 3 + 2 ) );
        if( referencePoints.pointsExistNearCoordinate( coordinate ) )
        {
            for( size_t dimension = 0; dimension < 3; dimension++ )
            {
                validatedVerts->push_back( m_validatedVertices->at( index * 3 + dimension ) );
                validatedColors->push_back( m_validatedColors->at( index * 3 + dimension ) );
            }
            validatedGroups->push_back( m_validatedGroups->at( index ) );
        }
    }

    if( validatedVerts->size() > 0 )
    {
        boost::shared_ptr< WDataSetPointsGrouped > validatedPoints(
                new WDataSetPointsGrouped( validatedVerts, validatedColors, validatedGroups ) );
        WGroupEdit groupEdit;
        groupEdit.initProocessBegin();
        groupEdit.setMergeGroups( false );
        groupEdit.setGroupSizeThreshold( 0 );
        groupEdit.mergeGroupSet( validatedPoints );
        size_t biggestGroup = 0;
        size_t maxGroupSize = 0;

        for( size_t groupID = 0; groupID <= groupEdit.getLastGroupID(); groupID++ )
            if( groupID == 0 || groupEdit.getGroupSize( groupID ) > maxGroupSize)
            {
                biggestGroup = groupID;
                maxGroupSize = groupEdit.getGroupSize( groupID );
            }
        return biggestGroup;
    }
    return 0;
}

boost::shared_ptr< WDataSetPoints > WGroupValidator::getPointsOfGroup(
        boost::shared_ptr< WDataSetPointsGrouped > groupedPoints, size_t groupID )
{
    WDataSetPointsGrouped::VertexArray origVertices = groupedPoints->getVertices();
    WDataSetPointsGrouped::ColorArray origColors = groupedPoints->getColors();
    WDataSetPointsGrouped::GroupArray origGroups = groupedPoints->getGroups();

    WDataSetPoints::VertexArray newVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPoints::ColorArray newColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );

    for( size_t index = 0; index < origVertices->size() / 3; index++ )
    {
        if( origGroups->at( index ) == groupID )
        {
            for( size_t dimension = 0; dimension < 3; dimension++ )
            {
                newVertices->push_back( origVertices->at( index * 3 + dimension ) );
                newColors->push_back( origColors->at( index * 3 + dimension ) );
            }
        }
    }

    if( newVertices->size() > 0 )
    {
        boost::shared_ptr< WDataSetPoints > outputPoints(
                new WDataSetPoints( newVertices, newColors ) );
        return outputPoints;
    }
    else
    {
        boost::shared_ptr< WDataSetPoints > outputPoints = getEmptyShowablePointSet();
        return outputPoints;
    }
}

boost::shared_ptr< WDataSetPoints > WGroupValidator::getEmptyShowablePointSet()
{
    WDataSetPoints::VertexArray newVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray newColors(
            new WDataSetPoints::ColorArray::element_type() );

    for( size_t dimension = 0; dimension < 3; dimension++ )
    {
        newVertices->push_back( 0 );
        newColors->push_back( 0 );
    }
    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( newVertices, newColors ) );
    return outputPoints;
}


void WGroupValidator::assignProgressCombiner( boost::shared_ptr< WProgressCombiner > progress )
{
    m_associatedProgressCombiner = progress;
}

void WGroupValidator::setProgressSettings( size_t referenceGroupCount )
{
    m_associatedProgressCombiner->removeSubProgress( m_progressStatus );
    std::ostringstream headerLabel;
    headerLabel << "Validating groups: " << referenceGroupCount;
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerLabel.str(), referenceGroupCount ) );
    m_associatedProgressCombiner->addSubProgress( m_progressStatus );
}
