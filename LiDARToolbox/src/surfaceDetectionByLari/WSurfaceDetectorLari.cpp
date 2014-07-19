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
}

WSurfaceDetectorLari::~WSurfaceDetectorLari()
{
}

boost::shared_ptr< WDataSetPoints > WSurfaceDetectorLari::detectSurfaces( vector<vector<double> >* inputPoints )
{
    //TODO(aschwarzkopf): After draft stage refine in several methods and remove comments
    cout << "Attempting to analyze " << inputPoints->size() << " points" << endl;
    WRealtimeTimer timer;
    timer.reset();
    WKdTreePcaProps* deletableTree = new WKdTreePcaProps( 3 );
    deletableTree->add( inputPoints );
    cout << "WKdTreeND took " << timer.elapsed() << " seconds." << endl;

    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );

    timer.reset();
    WPointSearcher pointSearcher;
    pointSearcher.setExaminedKdTree( deletableTree );
    pointSearcher.setMaxResultPointCount( m_numberPointsK );
    pointSearcher.setMaxSearchDistance( m_maxPointDistanceR );
    WPrincipalComponentAnalysis pca;
    vector<WKdTreeND*>* processedPoints = deletableTree->getAllLeafNodes();
    cout << "Attempting PCA for " << processedPoints->size() << " points" << endl;

    //Generating properties for each existing point
    for( size_t index = 0; index < processedPoints->size(); index++ )
    {
        //Getting nearest n points
        WKdTreePcaProps* pointProps = ( WKdTreePcaProps*)(processedPoints->at( index ) );
        vector<double> pointOfInterest = pointProps->getNodePoints()->at( 0 );
        pointSearcher.setSearchedPoint( pointOfInterest );
        vector<WPointDistance>* nearestPoints = pointSearcher.getNearestPoints();
        vector<WPosition>* points = WPointSearcher::convertToPointSet( nearestPoints );

        //Calculating Eigen properties
        pca.analyzeData( points );
        pointProps->createGeoProps();
        WLariGeoProps* geoProps = pointProps->getGeoProps();
        geoProps->setEigenVectors( pca.getDirections() );
        vector<double> eigenValues = pca.getEigenValues();
        vector<double> nLambdaI;    //TODO(aschwarzkopf): Unused
        double sumLambda = 0;        //TODO(aschwarzkopf): Unused
        geoProps->setEigenValues( eigenValues );

        //Adding parameter space information
        WLeastSquares leastSquares;
        leastSquares.analyzeData( points );
        geoProps->setParametersXYZ0( leastSquares.getParametersXYZ0() );
        //for(size_t index = 0; index < geoProps->getParametersXYZ0().size(); index++)
        //    cout << geoProps->getParametersXYZ0()[index] << "    ";
        //cout << endl;

        //Displaying points and classification
        for( size_t i = 0; i < eigenValues.size(); i++ )
            sumLambda += eigenValues[i];
        for( size_t i = 0; i < eigenValues.size(); i++ )
            nLambdaI.push_back( eigenValues[i] / sumLambda );
        for( size_t dimension = 0; dimension < pointOfInterest.size(); dimension++ )
            outVertices->push_back( pointOfInterest[dimension] );
        bool isPlanar = calculateIsPlanarPoint( eigenValues );
        bool isCylindrical = calculateIsCylindricalPoint( eigenValues );
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

        delete nearestPoints;
        delete points;
        //outputPointSet.printNearestPoints(nearestPoints, "Default point set");
    }
    cout << "WPointSearcher took " << timer.elapsed() << " seconds." << endl;
    delete deletableTree;

    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );
    return outputPoints;
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
