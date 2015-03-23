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

#include <vector>
#include "WLariOutliner.h"
#include "../common/math/leastSquares/WLeastSquares.h"


WLariOutliner::WLariOutliner( WLariPointClassifier* pointClassifier )
{
    m_pointClassifier = pointClassifier;
}

WLariOutliner::~WLariOutliner()
{
}

boost::shared_ptr< WDataSetPointsGrouped > WLariOutliner::outlineParameterDomain()
{
    WDataSetPointsGrouped::VertexArray outVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray outColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    WDataSetPointsGrouped::GroupArray outGroups(
            new WDataSetPointsGrouped::GroupArray::element_type() );
    vector<WKdPointND*>* parameters = m_pointClassifier->getParameterDomain()->getAllPoints();
    for( size_t index = 0; index < parameters->size(); index++ )
    {
        WParameterDomainKdPoint* parameter = static_cast<WParameterDomainKdPoint*>( parameters->at( index ) );
        if( m_pointClassifier->calculateIsPlanarPoint( parameter->getSpatialPoint()->getEigenValues() ) )
        {
            vector<double> parameterCoordinate = parameter->getCoordinate();
            for( size_t dimension = 0; dimension < parameterCoordinate.size(); dimension++ )
                outVertices->push_back( parameterCoordinate[dimension] );
            for( size_t colorCh = 0; colorCh < 3; colorCh++ )
                outColors->push_back( 1.0 );
            outGroups->push_back( parameter->getSpatialPoint()->getClusterID() );
        }
    }
    boost::shared_ptr< WDataSetPointsGrouped > outputPoints(
            new WDataSetPointsGrouped( outVertices, outColors, outGroups ) );
    return outputPoints;
}

boost::shared_ptr< WDataSetPointsGrouped > WLariOutliner::outlineSpatialDomainGroups()
{
    WDataSetPointsGrouped::VertexArray outVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray outColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    WDataSetPointsGrouped::GroupArray outGroups(
            new WDataSetPointsGrouped::GroupArray::element_type() );
    vector<WKdPointND*>* spatialDomainPoints = m_pointClassifier->getSpatialDomain()->getAllPoints();
    for( size_t index = 0; index < spatialDomainPoints->size(); index++ )
    {
        WSpatialDomainKdPoint* spatialPoint = static_cast<WSpatialDomainKdPoint*>( spatialDomainPoints->at( index ) );
        if( m_pointClassifier->calculateIsPlanarPoint( spatialPoint->getEigenValues() ) )
        {
            vector<double> spatialCoordinate = spatialPoint->getCoordinate();
            for( size_t dimension = 0; dimension < spatialCoordinate.size(); dimension++ )
                outVertices->push_back( spatialCoordinate[dimension] );
            outGroups->push_back( spatialPoint->getClusterID() );
            for( size_t colorCh = 0; colorCh < 3; colorCh++ )
                outColors->push_back( 1.0 );
        }
    }
    boost::shared_ptr< WDataSetPointsGrouped > outputPoints(
            new WDataSetPointsGrouped( outVertices, outColors, outGroups ) );
    return outputPoints;
}

boost::shared_ptr< WDataSetPoints > WLariOutliner::outlineSpatialDomainCategories()
{
    WDataSetPoints::VertexArray outVertices( new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors( new WDataSetPoints::ColorArray::element_type() );
    vector<WKdPointND*>* spatialDomainPoints = m_pointClassifier->getSpatialDomain()->getAllPoints();
    for( size_t index = 0; index < spatialDomainPoints->size(); index++ )
    {
        WSpatialDomainKdPoint* spatialPoint = static_cast<WSpatialDomainKdPoint*>( spatialDomainPoints->at( index ) );
        vector<double> spatialCoordinate = spatialPoint->getCoordinate();
        vector<double> eigenValues = spatialPoint->getEigenValues();

        for( size_t dimension = 0; dimension < spatialCoordinate.size(); dimension++ )
            outVertices->push_back( spatialCoordinate[dimension] );
        bool isPlanar = m_pointClassifier->calculateIsPlanarPoint( eigenValues );
        bool isCylindrical = m_pointClassifier->calculateIsCylindricalPoint( eigenValues );
            if( isPlanar || isCylindrical )
            {
                outColors->push_back( isPlanar ?1 :0 );
                outColors->push_back( isCylindrical ?0.5 :0 );
                outColors->push_back( isCylindrical ?1.0 :0 );
            }
            else
            {
                for( size_t colorCh = 0; colorCh < 3; colorCh++ )
                    outColors->push_back( 0.5 );
            }
    }
    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
}

boost::shared_ptr< WTriangleMesh > WLariOutliner::outlineLeastSquaresPlanes( double squaresWidth )
{
    boost::shared_ptr< WTriangleMesh > outputMesh( new WTriangleMesh( 0, 0 ) );
    vector<WKdPointND*>* spatialNodes = m_pointClassifier->getSpatialDomain()->getAllPoints();
    for( size_t index = 0; index < spatialNodes->size(); index++ )
    {
        WSpatialDomainKdPoint* spatialDomainPoint = static_cast<WSpatialDomainKdPoint*>( spatialNodes->at( index ) );
        vector<double> spatialCoordinate = spatialDomainPoint->getCoordinate();
        WPosition spatialPoint( 0.0, 0.0, 0.0 );
        for( size_t dimension = 0; dimension < spatialCoordinate.size(); dimension++ )
            spatialPoint[dimension] = spatialCoordinate[dimension];
        vector<double> planeFormula = spatialDomainPoint->getHessianNormalForm();

        WMTempLeastSquaresTest::outlineNormalPlane( planeFormula, spatialPoint, squaresWidth / 2.0, outputMesh );
    }
    return outputMesh;
}
