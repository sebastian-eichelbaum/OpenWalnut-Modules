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
#include <limits>
#include "WLariPointClassifier.h"
#include "../common/math/leastSquares/WLeastSquares.h"


WLariPointClassifier::WLariPointClassifier()
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
    m_spatialDomain = new WKdTreeND( 3 );
    m_parameterDomain = new WKdTreeND( 3 );

    setCpuThreadCount( 8 );
}

WLariPointClassifier::~WLariPointClassifier()
{
}

void WLariPointClassifier::analyzeData( vector<WSpatialDomainKdPoint*>* inputPoints )
{
    cout << "Attempting to analyze " << inputPoints->size() << " points" << endl;
    delete m_spatialDomain;
    delete m_parameterDomain;
    m_spatialDomain = new WKdTreeND( 3 );
    m_parameterDomain = new WKdTreeND( 3 );
    m_spatialDomain->add( reinterpret_cast<vector<WKdPointND*>*>( inputPoints ) );
    vector<WParameterDomainKdPoint*>* parameterPoints = new vector<WParameterDomainKdPoint*>();

    classifyPoints( inputPoints, parameterPoints );

    cout << "Adding " << parameterPoints->size() << " parameter points" << endl;
    m_parameterDomain->add( reinterpret_cast<vector<WKdPointND*>*>( parameterPoints ) );
}
WKdTreeND* WLariPointClassifier::getParameterDomain()
{
    return m_parameterDomain;
}
WKdTreeND* WLariPointClassifier::getSpatialDomain()
{
    return m_spatialDomain;
}

void WLariPointClassifier::classifyPoints( vector<WSpatialDomainKdPoint*>* spatialPoints, vector<WParameterDomainKdPoint*>* parameterPoints )
{
    size_t threads = m_cpuThreadCount < spatialPoints->size() ?m_cpuThreadCount :spatialPoints->size();
    for( size_t thread = 0; thread < threads; thread++ )
        m_cpuThreads[thread] = new boost::thread( &WLariPointClassifier::
                classifyPointsAtThread, this, spatialPoints, thread );
    for( size_t thread = 0; thread < threads; thread++ )
        m_cpuThreads[thread]->join();

    for( size_t index = 0; index < spatialPoints->size(); index++ )
    {
        WSpatialDomainKdPoint* spatialPoint = spatialPoints->at( index );
        if( calculateIsPlanarPoint(spatialPoint->getEigenValues() ) && spatialPoint->hasValidParameters() )
        {
            WParameterDomainKdPoint* newParameter =
                    new WParameterDomainKdPoint( spatialPoint->getParametersXYZ0() );
            newParameter->setSpatialPoint( spatialPoint );
            parameterPoints->push_back( newParameter );
        }
        spatialPoint->setIndexInInputArray( index );
    }
}
void WLariPointClassifier::classifyPointsAtThread( vector<WSpatialDomainKdPoint*>* spatialPoints, size_t threadIndex )
{
    WPointSearcher spatialSearcher( m_spatialDomain );
    spatialSearcher.setMaxResultPointCount( m_numberPointsK );
    spatialSearcher.setMaxSearchDistance( m_maxPointDistanceR );
    for( size_t index = threadIndex; index < spatialPoints->size(); index += m_cpuThreadCount )
    {
        WSpatialDomainKdPoint* spatialPoint = spatialPoints->at( index );
        vector<double> spatialCoordinate = spatialPoint->getCoordinate();
        spatialSearcher.setSearchedPoint( spatialCoordinate );
        vector<WPointDistance>* nearestPoints = spatialSearcher.getNearestPoints();
        vector<WPosition>* points = WPointDistance::convertToPointSet( nearestPoints );
        spatialPoint->setKNearestPoints( points->size() );
        spatialPoint->setDistanceToNthNearestNeighbor( nearestPoints->at( points->size() - 1 ).getDistance() );

        WPrincipalComponentAnalysis pca;
        pca.analyzeData( *points );
        spatialPoint->setEigenVectors( pca.getEigenVectors() );
        vector<double> eigenValues = pca.getEigenValues();
        spatialPoint->setEigenValues( eigenValues );

        WLeastSquares leastSquares;
        leastSquares.analyzeData( points );
        spatialPoint->setHessianNormalForm( leastSquares.getHessianNormalForm() );

        delete nearestPoints;
        delete points;
    }
}
bool WLariPointClassifier::calculateIsPlanarPoint( vector<double> eigenValues )
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
bool WLariPointClassifier::calculateIsCylindricalPoint( vector<double> eigenValues )
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
double WLariPointClassifier::getVectorSum( vector<double> allNumbers )
{
    double value = 0;
    for( size_t index = 0; index < allNumbers.size(); index++ )
        value += allNumbers[index];
    return value;
}
void WLariPointClassifier::setNumberPointsK( size_t pointsCount )
{
    m_numberPointsK = pointsCount;
}
void WLariPointClassifier::setMaxPointDistanceR( double maxPointDistance )
{
    m_maxPointDistanceR = maxPointDistance;
}
void WLariPointClassifier::setCpuThreadCount( size_t cpuThreadCount )
{
    m_cpuThreadCount = cpuThreadCount;
    m_cpuThreads.reserve( m_cpuThreadCount );
    m_cpuThreads.resize( m_cpuThreadCount );
}
void WLariPointClassifier::setPlanarNLambdaRange( size_t lambdaIndex, double min, double max )
{
    m_planarNLambdaMin[lambdaIndex] = min;
    m_planarNLambdaMax[lambdaIndex] = max;
}
void WLariPointClassifier::setCylindricalNLambdaRange( size_t lambdaIndex, double min, double max )
{
    m_cylindricalNLambdaMin[lambdaIndex] = min;
    m_cylindricalNLambdaMax[lambdaIndex] = max;
}
