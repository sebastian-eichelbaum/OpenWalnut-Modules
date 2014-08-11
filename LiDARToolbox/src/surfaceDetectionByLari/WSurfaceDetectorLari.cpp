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
#include "WSurfaceDetectorLari.h"
#include "../common/math/leastSquares/WLeastSquares.h"


WSurfaceDetectorLari::WSurfaceDetectorLari()
{
    m_numberPointsK = 12;
    m_maxPointDistanceR = 1.0;
    m_segmentationMaxAngleDegrees = 10.0;
    m_segmentationPlaneDistance = 1.0;
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
}

WSurfaceDetectorLari::~WSurfaceDetectorLari()
{
}

void WSurfaceDetectorLari::analyzeData( vector<WSpatialDomainKdPoint*>* inputPoints )
{
    //TODO(aschwarzkopf): After draft stage refine in several methods and remove comments
    cout << "Attempting to analyze " << inputPoints->size() << " points" << endl;
    delete m_spatialDomain;
    delete m_parameterDomain;
    m_spatialDomain = new WKdTreeND( 3 );
    m_parameterDomain = new WKdTreeND( 3 );
    m_spatialDomain->add( reinterpret_cast<vector<WKdPointND*>*>( inputPoints ) );
    vector<WParameterDomainKdPoint*>* parameterPoints = new vector<WParameterDomainKdPoint*>();

    WPointSearcher spatialSearcher( m_spatialDomain );
    spatialSearcher.setMaxResultPointCount( m_numberPointsK );
    spatialSearcher.setMaxSearchDistance( m_maxPointDistanceR );
    vector<WKdPointND*>* spatialPoints = m_spatialDomain->getAllPoints();

    //Generating properties for each existing point
    for( size_t index = 0; index < spatialPoints->size(); index++ )
    {
        //Getting nearest n points
        WSpatialDomainKdPoint* spatialPoint = static_cast<WSpatialDomainKdPoint*>( spatialPoints->at( index ) );
        vector<double> spatialCoordinate = spatialPoint->getCoordinate();
        spatialSearcher.setSearchedPoint( spatialCoordinate );
        vector<WPointDistance>* nearestPoints = spatialSearcher.getNearestPoints();
        vector<WPosition>* points = WPointSearcher::convertToPointSet( nearestPoints );

        //Calculating Eigen properties
        WPrincipalComponentAnalysis pca;
        pca.analyzeData( points );
        spatialPoint->setEigenVectors( pca.getDirections() );
        vector<double> eigenValues = pca.getEigenValues();
        spatialPoint->setEigenValues( eigenValues );

        //Adding parameter space information
        WLeastSquares leastSquares;
        leastSquares.analyzeData( points );
        spatialPoint->setHessescheNormalForm( leastSquares.getHessescheNormalForm() );
        if(calculateIsPlanarPoint(eigenValues) && spatialPoint->hasValidParameters() )
        {
            WParameterDomainKdPoint* newParameter =
                    new WParameterDomainKdPoint( spatialPoint->getParametersXYZ0() );
            newParameter->setSpatialPoint( spatialPoint );
            parameterPoints->push_back( newParameter );
        }

        delete nearestPoints;
        delete points;
    }
    cout << "Adding " << parameterPoints->size() << " parameter points" << endl;
    m_parameterDomain->add( reinterpret_cast<vector<WKdPointND*>*>( parameterPoints ) );
    initExtentSizes();
    detectClustersByBruteForce();
}
void WSurfaceDetectorLari::initExtentSizes()
{
    cout << "Initializing extent sizes:" << endl;
    vector<WKdPointND*>* parameterPoints = m_parameterDomain->getAllPoints();
    for( size_t index = 0; index < parameterPoints->size(); index++ )
    {
        WParameterDomainKdPoint* parameterPoint = static_cast<WParameterDomainKdPoint*>( parameterPoints->at( index ) );
        vector<double> parameterCoordinate = parameterPoint->getCoordinate();

        int extentPointSize = getParametersOfExtent( parameterCoordinate )->size();
        //cout << "Initializing extent " << index << "/" << parameterPoints->size() << ", size = " << extentPointSize << endl;
        parameterPoint->setExtentPointCount( extentPointSize );
    }
}
void WSurfaceDetectorLari::detectClustersByBruteForce()
{
    vector<WKdPointND*>* parameterNodes = m_parameterDomain->getAllPoints(); //AW: D A S   H E I S S T   K E I N E   W A R T E S C H L A N G E ! ! !
    vector<WParameterDomainKdPoint*>* checkedNodes = new vector<WParameterDomainKdPoint*>();
    size_t currentClusterID = 0;
    while( parameterNodes->size() > 0 )
    {
        //cout << "Detecting cluster - " << parameterNodes->size() << " left" << endl;
        WParameterDomainKdPoint* biggestExtentPoint = 0;
        int mostNeighborsCount = 0;
//        cout << "Looking for biggest extent" << endl;
        for( size_t index = 0; index < parameterNodes->size(); index++ )
        {
            WParameterDomainKdPoint* parameterPoint = static_cast<WParameterDomainKdPoint*>( parameterNodes->at( index ) );
            if( parameterPoint->getExtentPointCount() > mostNeighborsCount )
            {
                biggestExtentPoint = parameterPoint;
                mostNeighborsCount = parameterPoint->getExtentPointCount();
            }
        }
//        cout << "Looking for neighbors of biggest extent" << endl;
        vector<WParameterDomainKdPoint*>* extentPoints =
                getParametersOfExtent( biggestExtentPoint->getCoordinate() );
        cout << "Updating Data for extent (" << parameterNodes->size() << "/" << m_parameterDomain->getAllPoints()->size()
                << " left): " << biggestExtentPoint->getExtentPointCount() << "/" << extentPoints->size() << endl;
        for(size_t index = 0; index < extentPoints->size(); index++ )
        {
            WParameterDomainKdPoint* extentPoint = static_cast<WParameterDomainKdPoint*>( extentPoints->at( index ) );
            if( !extentPoint->isAddedToPlane() )
            {
                vector<WParameterDomainKdPoint*>* coExtent =
                    getParametersOfExtent( extentPoint->getCoordinate() );
                for(size_t paramIndex = 0; paramIndex < coExtent->size(); paramIndex++ )
                    coExtent->at( paramIndex )->tagToRefresh( true );

                WSpatialDomainKdPoint* assignedSpatialNode = extentPoint->getSpatialPoint();
                assignedSpatialNode->setClusterID( currentClusterID );

                extentPoint->setIsAddedToPlane( true );
            }
        }
        currentClusterID++;

        vector<WKdPointND*>* oldParameterNodes = parameterNodes;
        parameterNodes = new vector<WKdPointND*>();
        for(size_t index = 0; index < oldParameterNodes->size(); index++ )
            if( !( static_cast<WParameterDomainKdPoint*>( oldParameterNodes->at(index) ) )->isAddedToPlane() )
            {
                parameterNodes->push_back( oldParameterNodes->at( index ) );
            }
            else
            {
                checkedNodes->push_back( static_cast<WParameterDomainKdPoint*>( oldParameterNodes->at( index ) ) );
            }

        for(size_t index = 0; index < parameterNodes->size(); index++)
        {
            WParameterDomainKdPoint* refreshable = static_cast<WParameterDomainKdPoint*>( parameterNodes->at( index ) );
            if( refreshable->isTaggedToRefresh() )
            {
                vector<WParameterDomainKdPoint*>* coExtent =
                    getParametersOfExtent( refreshable->getCoordinate() );
                size_t extentPointCount = 0;
                for( size_t coIndex = 0; coIndex < coExtent->size(); coIndex++ )
                    if( coExtent->at(coIndex)->isAddedToPlane() == false )
                        extentPointCount++;
                refreshable->setExtentPointCount( extentPointCount );
                refreshable->tagToRefresh( false );
            }
        }
    }
    int neighsMin = 0;
    int neighsMax = 0;
    int neighsSum = 0;
    for( size_t index = 0; index < checkedNodes->size(); index++ )
    {
        int neighbors = checkedNodes->at( index )->getExtentPointCount();
        neighsSum += neighbors;
        if( index == 0 || neighbors < neighsMin )
            neighsMin = neighbors;
        if( index == 0 || neighbors > neighsMax )
            neighsMax = neighbors;
    }
    for( size_t index1 = 0; index1 < checkedNodes->size() - 1; index1++ )
        for( size_t index2 = index1 + 1; index2 < checkedNodes->size(); index2++ )
            if( checkedNodes->at(index1)->getExtentPointCount() > checkedNodes->at(index2)->getExtentPointCount() )
            {
                WParameterDomainKdPoint* tmp = checkedNodes->at( index1 );
                checkedNodes->at( index1 ) = checkedNodes->at( index2 );
                checkedNodes->at( index2 ) = tmp;
            }
    cout << "Neighbor count range (" << checkedNodes->size() << "): " << neighsMin << " ~ " << neighsMax << "; sum = " << neighsSum << endl;
}
WKdTreeND* WSurfaceDetectorLari::getParameterDomain()
{
    return m_parameterDomain;
}
WKdTreeND* WSurfaceDetectorLari::getSpatialDomain()
{
    return m_spatialDomain;
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
void WSurfaceDetectorLari::setSegmentationSettings( double maxAngleDegrees, double planeDistance )
{
    m_segmentationMaxAngleDegrees = maxAngleDegrees;
    m_segmentationPlaneDistance = planeDistance;
}
double WSurfaceDetectorLari::getMaxParameterDistance( vector<double> parametersXYZ0 )
{
    vector<double> extent( 3, 0.0 );
    vector<double> origin( 3, 0.0 );
    extent[0] = WPointDistance::getPointDistance( origin, parametersXYZ0 );
    double angle = m_segmentationMaxAngleDegrees / 90.0 * asin( 1.0 );
    double distanceNear = extent[0] - m_segmentationPlaneDistance;
    double distanceFar = extent[0] + m_segmentationPlaneDistance;
    double rotationX = cos( angle ) - sin( angle );
    double rotationY = sin( angle ) + cos( angle );
    vector<double> parameterNear( 3, 0.0 );
    vector<double> parameterFar( 3, 0.0 );
    parameterNear[0] = rotationX * distanceNear;
    parameterNear[1] = rotationY * distanceNear;
    parameterFar[0] = rotationX * distanceFar;
    parameterFar[1] = rotationY * distanceFar;
    distanceNear = WPointDistance::getPointDistance( extent, parameterNear );
    distanceFar = WPointDistance::getPointDistance( extent, parameterFar );
    //cout << " )\tDistance: " << distanceNear<<", "<<distanceFar<<"\t";
    return distanceNear > distanceFar ?distanceNear :distanceFar;
}
vector<WParameterDomainKdPoint*>* WSurfaceDetectorLari::getParametersOfExtent( vector<double> parametersXYZ0 )
{
    double maxParameterDistance = getMaxParameterDistance( parametersXYZ0 );
    WPointSearcher parameterSearcher( m_parameterDomain );
    parameterSearcher.setMaxResultPointCount( std::numeric_limits< size_t >::max() );
    parameterSearcher.setMaxSearchDistance( maxParameterDistance );
    parameterSearcher.setSearchedPoint( parametersXYZ0 );
    vector<WPointDistance>* nearestPoints = parameterSearcher.getNearestPoints();
    //cout << "Considering "<<nearestPoints->size()<<" nearest points\t";
    vector<WParameterDomainKdPoint*>* neighborParameters = new vector<WParameterDomainKdPoint*>();
    for( size_t index = 0; index < nearestPoints->size(); index++ )
        if( isParameterOfSameExtent( parametersXYZ0, nearestPoints->at(index).getComparedCoordinate() ) )
            neighborParameters->push_back( static_cast<WParameterDomainKdPoint*>
                    ( nearestPoints->at( index ).getComparedPoint() ) );
    return neighborParameters;
}
bool WSurfaceDetectorLari::isParameterOfSameExtent( vector<double> parameters1, vector<double> parameters2 )
{
    vector<double> origin( 3, 0 );
    //for( size_t index = 0; index < parameters1.size(); index++ )
    //    origin.push_back( 0.0 );
    double distance1 = WPointDistance::getPointDistance( origin, parameters1 );
    double distance2 = WPointDistance::getPointDistance( origin, parameters2 );
    if( distance1 + distance2 == 0.0 )
        return true;
    if( abs( distance1 - distance2 ) > m_segmentationPlaneDistance )
        return false;
    double angle = WVectorMaths::getAngleOfPlanes( parameters1, parameters2 );
    return angle <= m_segmentationMaxAngleDegrees;
}
