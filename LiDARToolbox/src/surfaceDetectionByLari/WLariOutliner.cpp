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


WLariOutliner::WLariOutliner( WSurfaceDetectorLari* surfaceDetector )
{
    m_surfaceDetector = surfaceDetector;
}

WLariOutliner::~WLariOutliner()
{
}
boost::shared_ptr< WDataSetPoints > WLariOutliner::outlineParameterDomain()
{
    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );
    vector<WKdTreeND*>* parameterNodes = m_surfaceDetector->getParameterDomain()->getAllLeafNodes();
    for( size_t index = 0; index < parameterNodes->size(); index++ )
    {
        WParameterDomainKdNode* parameterDomainNode = ( WParameterDomainKdNode* )( parameterNodes->at( index ) );
        vector<double> parameterCoordinate = parameterDomainNode->getNodePoints()->at( 0 );
        for( size_t dimension = 0; dimension < parameterCoordinate.size(); dimension++ )
            outVertices->push_back( parameterCoordinate[dimension] );
        for( size_t colorCh = 0; colorCh < 3; colorCh++ )
            outColors->push_back( 0.50 );
    }
    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
}
boost::shared_ptr< WDataSetPoints > WLariOutliner::outlineSpatialDomain()
{
    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );
    vector<WKdTreeND*>* spatialNodes = m_surfaceDetector->getSpatialDomain()->getAllLeafNodes();
    for( size_t index = 0; index < spatialNodes->size(); index++ )
    {
        WSpatialDomainKdNode* spatialDomainNode = ( WSpatialDomainKdNode* )( spatialNodes->at( index ) );
        vector<double> spatialCoordinate = spatialDomainNode->getNodePoints()->at( 0 );
        vector<double> eigenValues = spatialDomainNode->getEigenValues();

        double sumLambda = 0;        //TODO(aschwarzkopf): Unused
        for( size_t i = 0; i < eigenValues.size(); i++ )
            sumLambda += eigenValues[i];
        vector<double> nLambdaI;    //TODO(aschwarzkopf): Unused
        for( size_t i = 0; i < eigenValues.size(); i++ )
            nLambdaI.push_back( eigenValues[i] / sumLambda );
        for( size_t dimension = 0; dimension < spatialCoordinate.size(); dimension++ )
            outVertices->push_back( spatialCoordinate[dimension] );
        bool isPlanar = m_surfaceDetector->calculateIsPlanarPoint( eigenValues );
        bool isCylindrical = m_surfaceDetector->calculateIsCylindricalPoint( eigenValues );
        if( isPlanar || isCylindrical )
        {
            outColors->push_back( isPlanar ?1 :0 );
            outColors->push_back( isCylindrical ?0.5 :0 );
            outColors->push_back( isCylindrical ?1.0 :0 );
        }
        else
        {
            for( size_t colorCh = 0; colorCh < 3; colorCh++ )
                outColors->push_back( 0.50 );
        }
    }
    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
}
boost::shared_ptr< WTriangleMesh > WLariOutliner::outlineLeastSquaresPlanes( double squaresWidth )
{
    boost::shared_ptr< WTriangleMesh > outputMesh( new WTriangleMesh( 0, 0 ) );
    vector<WKdTreeND*>* spatialNodes = m_surfaceDetector->getSpatialDomain()->getAllLeafNodes();
    for( size_t index = 0; index < spatialNodes->size(); index++ )
    {
        WSpatialDomainKdNode* spatialDomainNode = ( WSpatialDomainKdNode* )( spatialNodes->at( index ) );
        vector<double> spatialCoordinate = spatialDomainNode->getNodePoints()->at( 0 );
        WPosition spatialPoint( 0.0, 0.0, 0.0 );
        for( size_t dimension = 0; dimension < spatialCoordinate.size(); dimension++ )
            spatialPoint[dimension] = spatialCoordinate[dimension];
        vector<double> planeFormula = spatialDomainNode->getHessescheNormalForm();

        WMTempLeastSquaresTest::outlineNormalPlane( planeFormula, spatialPoint, squaresWidth / 2.0, outputMesh );
    }
    return outputMesh;
}
