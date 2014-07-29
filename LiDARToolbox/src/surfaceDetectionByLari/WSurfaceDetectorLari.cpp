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
#include "WSurfaceDetectorLari.h"
#include "../common/math/leastSquares/WLeastSquares.h"


WSurfaceDetectorLari::WSurfaceDetectorLari()
{
    m_numberPointsK = 12;
    m_maxPointDistanceR = 1.0;
    m_planarNLambdaMin.reserve( 3 );
    m_planarNLambdaMin.resize( 3 );
    m_planarNLambdaMax.reserve( 3 );
    m_planarNLambdaMax.resize( 3 );
    m_cylindricalNLambdaMin.reserve( 3 );
    m_cylindricalNLambdaMin.resize( 3 );
    m_cylindricalNLambdaMax.reserve( 3 );
    m_cylindricalNLambdaMax.resize( 3 );
    m_spadialDomain = new WSpatialDomainKdNode( 3 );
}

WSurfaceDetectorLari::~WSurfaceDetectorLari()
{
}

void WSurfaceDetectorLari::analyzeData( vector<vector<double> >* inputPoints )
{
    //TODO(aschwarzkopf): After draft stage refine in several methods and remove comments
    cout << "Attempting to analyze " << inputPoints->size() << " points" << endl;
    delete m_spadialDomain;
    m_spadialDomain = new WSpatialDomainKdNode( 3 );
    m_parameterDomain = new WParameterDomainKdNode( 3 );
    m_spadialDomain->add( inputPoints );
    vector<vector<double> >* parameterPoints = new vector<vector<double> >();

    WPointSearcher pointSearcher( m_spadialDomain );
    pointSearcher.setMaxResultPointCount( m_numberPointsK );
    pointSearcher.setMaxSearchDistance( m_maxPointDistanceR );
    vector<WKdTreeND*>* spatialNodes = m_spadialDomain->getAllLeafNodes();

    //Generating properties for each existing point
    for( size_t index = 0; index < spatialNodes->size(); index++ )
    {
        //Getting nearest n points
        WSpatialDomainKdNode* spatialDomainNode = ( WSpatialDomainKdNode* )( spatialNodes->at( index ) );
        vector<double> spatialCoordinate = spatialDomainNode->getNodePoints()->at( 0 );
        pointSearcher.setSearchedPoint( spatialCoordinate );
        vector<WPointDistance>* nearestPoints = pointSearcher.getNearestPoints();
        vector<WPosition>* points = WPointSearcher::convertToPointSet( nearestPoints );

        //Calculating Eigen properties
        WPrincipalComponentAnalysis pca;
        pca.analyzeData( points );
        spatialDomainNode->setEigenVectors( pca.getDirections() );
        vector<double> eigenValues = pca.getEigenValues();
        spatialDomainNode->setEigenValues( eigenValues );

        //Adding parameter space information
        WLeastSquares leastSquares;
        leastSquares.analyzeData( points );
        spatialDomainNode->setHessescheNormalForm( leastSquares.getHessescheNormalForm() );
        parameterPoints->push_back( leastSquares.getParametersXYZ0() );

        delete nearestPoints;
        delete points;
    }
    m_parameterDomain->add( parameterPoints );
    linkSpatialAndParameterDomain();
}
void WSurfaceDetectorLari::linkSpatialAndParameterDomain()
{
    WPointSearcher pointSearcher( m_spadialDomain );
    pointSearcher.setMaxResultPointCount( 1 );
    pointSearcher.setMaxSearchDistance( 0.0 );
    vector<WKdTreeND*>* parameterNodes = m_parameterDomain->getAllLeafNodes();
    size_t noSuccess = 0;
    size_t success = 0;
    for( size_t index = 0; index < parameterNodes->size(); index++ )
    {
        WParameterDomainKdNode* parameterDomainNode = ( WParameterDomainKdNode* )( parameterNodes->at( index ) );
        vector<double> parameterCoordinate = parameterDomainNode->getNodePoints()->at( 0 );
        pointSearcher.setSearchedPoint( parameterCoordinate );
        vector<WPointDistance>* nearestPoints = pointSearcher.getNearestPoints();
        noSuccess += nearestPoints->size() == 0 ?1 :0;
        success += nearestPoints->size() > 0 ?1 :0;
    }
    std::cout << "sussess: " << success << std::endl;
    std::cout << "no sussess: " << noSuccess << std::endl;
}
WParameterDomainKdNode* WSurfaceDetectorLari::getParameterDomain()
{
    return m_parameterDomain;
}
WSpatialDomainKdNode* WSurfaceDetectorLari::getSpatialDomain()
{
    return m_spadialDomain;
}
bool WSurfaceDetectorLari::calculateIsPlanarPoint( vector<double> eigenValues )
{
    double sum = getVectorSum( eigenValues );
    for( size_t index = 0; index < eigenValues.size(); index++ )
    {
        double value = eigenValues[index] / sum;
        if( value <= m_planarNLambdaMin[index]
                || value > m_planarNLambdaMax[index] )
            return false;
    }
    return true;
}
bool WSurfaceDetectorLari::calculateIsCylindricalPoint( vector<double> eigenValues )
{
    double sum = getVectorSum( eigenValues );
    for( size_t index = 0; index < eigenValues.size(); index++ )
    {
        double value = eigenValues[index] / sum;
        if( value <= m_cylindricalNLambdaMin[index]
                || value > m_cylindricalNLambdaMax[index] )
            return false;
    }
    return true;
}
double WSurfaceDetectorLari::getVectorSum( vector<double> allNumbers )
{
    double value = 0;
    for( size_t index = 0; index < allNumbers.size(); index++ )
        value += allNumbers[index];
    return value;
}
void WSurfaceDetectorLari::setNumberPointsK( size_t pointsCount )
{
    m_numberPointsK = pointsCount;
}
void WSurfaceDetectorLari::setMaxPointDistanceR( double maxPointDistance )
{
    m_maxPointDistanceR = maxPointDistance;
}
void WSurfaceDetectorLari::setPlanarNLambdaRange( size_t lambdaIndex, double min, double max )
{
    m_planarNLambdaMin[lambdaIndex] = min;
    m_planarNLambdaMax[lambdaIndex] = max;
}
void WSurfaceDetectorLari::setCylindricalNLambdaRange( size_t lambdaIndex, double min, double max )
{
    m_cylindricalNLambdaMin[lambdaIndex] = min;
    m_cylindricalNLambdaMax[lambdaIndex] = max;
}
