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
#include "WMTempLeastSquaresTest.xpm"
#include "WMTempLeastSquaresTest.h"
#include "../common/datastructures/octree/WOctree.h"

WMTempLeastSquaresTest::WMTempLeastSquaresTest():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMTempLeastSquaresTest::~WMTempLeastSquaresTest()
{
}

boost::shared_ptr< WModule > WMTempLeastSquaresTest::factory() const
{
    return boost::shared_ptr< WModule >( new WMTempLeastSquaresTest() );
}

const char** WMTempLeastSquaresTest::getXPMIcon() const
{
    return WMTempLeastSquaresTest_xpm;
}
const std::string WMTempLeastSquaresTest::getName() const
{
    return "[Temp.] Least squares test";
}

const std::string WMTempLeastSquaresTest::getDescription() const
{
    return "Crops point data to a selection.";
}

void WMTempLeastSquaresTest::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_output = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
                new WModuleOutputData< WTriangleMesh >(
                        shared_from_this(), "points", "The loaded points." ) );

    addConnector( m_output );
    WModule::connectors();
}

void WMTempLeastSquaresTest::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    WModule::properties();
}

void WMTempLeastSquaresTest::requirements()
{
}

void WMTempLeastSquaresTest::moduleMain()
{
    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_input->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    // main loop
    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();


        //m_output->updateData( getRandomPoints() );
        analyzeBestFittedPlane();

//        std::cout << "this is WOTree " << std::endl;

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPoints > points2 = m_input->getData();
        if  ( !points2 )
        {
            continue;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

void WMTempLeastSquaresTest::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}

void WMTempLeastSquaresTest::analyzeBestFittedPlane()
{
    boost::shared_ptr< WDataSetPoints > points = m_input->getData();

    if( points )
    {
        m_verts = points->getVertices();
        m_colors = points->getColors();
        vector<WPosition>* data = new vector<WPosition>();
        WPosition mean( 0, 0, 0 );
        for( size_t index = 0; index < m_verts->size() / 3; index++ )
        {
            data->push_back( WPosition( m_verts->at( index*3 ),
                                       m_verts->at( index*3 + 1 ),
                                       m_verts->at( index*3 + 2 ) ) );
            for( size_t dimension = 0; dimension < mean.size(); dimension++ )
                mean[dimension] += m_verts->at( index * 3 + dimension ) * 3.0 / m_verts->size();
        }
        WLeastSquares leastSquares;
        leastSquares.analyzeData( data );
        WPosition cuttingPoint = leastSquares.getNearestPointTo( mean );
        double maxDistance = 0.0;
        for( size_t index = 0; index < data->size(); index++ )
        {
            double distance = 0;
            for( size_t dimension = 0; dimension < cuttingPoint.size(); dimension++ )
            {
                distance += pow( cuttingPoint[dimension] - data->at( index )[dimension], 2.0 );
            }
            distance = pow( distance, 0.5 );
            if(distance > maxDistance)
                maxDistance = distance;
        }
        vector<double> planeFormula = leastSquares.getHessianNormalForm();

        boost::shared_ptr< WTriangleMesh > outputMesh( new WTriangleMesh( 0, 0 ) );
        outlineNormalPlane( planeFormula, mean, maxDistance * 1.5, outputMesh );

        m_output->updateData( outputMesh );

        std::cout << "analyzeBestFittedPlane() - Mean: " << mean << std::endl;

        std::cout << "analyzeBestFittedPlane() - Distance to mean: " << leastSquares.getDistanceToPlane( mean ) << std::endl;

        std::cout << "analyzeBestFittedPlane() - Nearest point:  " << cuttingPoint << std::endl;

        std::cout << "analyzeBestFittedPlane() - Its distance to plane: " << leastSquares.getDistanceToPlane( cuttingPoint ) << std::endl;

        std::cout << "analyzeBestFittedPlane() - Plane: ";
        for( size_t index = 0; index < planeFormula.size(); index++ )
            std::cout << planeFormula[index] << ", ";
        std::cout << endl << endl;
    }
}

void WMTempLeastSquaresTest::outlineNormalPlane( vector<double> planeHessianNormalForm,
        WPosition nearestPoint, double planeRadius, boost::shared_ptr< WTriangleMesh > targetTriangleMesh )
{
    WPosition cuttingPoint = WLeastSquares::getNearestPointTo( planeHessianNormalForm, nearestPoint );

    size_t strongestDimension = 0;
    double strongestDimensionExtent = 0;
    for( size_t dimension = 0; dimension < cuttingPoint.size(); dimension++ )
        if( planeHessianNormalForm[dimension] > strongestDimensionExtent )
        {
            strongestDimension = dimension;
            strongestDimensionExtent = planeHessianNormalForm[dimension];
        }
    size_t dimensionVec1 = strongestDimension == 0 ?1 :0;

    WVector3d normalVector( planeHessianNormalForm[0], planeHessianNormalForm[1], planeHessianNormalForm[2] );
    normalVector = getNormalizedVector( normalVector );
    WVector3d vector1( 0, 0, 0 );
    vector1[dimensionVec1] = normalVector[strongestDimension];
    vector1[strongestDimension] = - normalVector[dimensionVec1];
    vector1 = getNormalizedVector( vector1 );
    WPosition vector2( normalVector[1] * vector1[2] - normalVector[2] * vector1[1],
                        normalVector[2] * vector1[0] - normalVector[0] * vector1[2],
                        normalVector[0] * vector1[1] - normalVector[1] * vector1[0] );
    WPosition vector3( vector1[1] * vector2[2] - vector1[2] * vector2[1],
                        vector1[2] * vector2[0] - vector1[0] * vector2[2],
                        vector1[0] * vector2[1] - vector1[1] * vector2[0] );

    size_t count = targetTriangleMesh->vertSize();
    for( double factor = 1.0; factor >= -2.0; factor -= 2.0 )
        for( double vectorN = 1.0; vectorN >= -2.0; vectorN -= 2.0 )
        {
            WPosition vecN( 0.0, 0.0, 0.0 );
            for( size_t dimension = 0; dimension < cuttingPoint.size(); dimension++ )
                vecN[dimension] = cuttingPoint[dimension] + planeRadius * (
                        vector1[dimension] * vectorN + vector2[dimension] * factor );
            targetTriangleMesh->addVertex( vecN );
        }
    targetTriangleMesh->addTriangle( 2 + count, 0 + count, 1 + count );
    targetTriangleMesh->addTriangle( 3 + count, 2 + count, 1 + count );
}

WPosition WMTempLeastSquaresTest::getNormalizedVector( WVector3d vector )
{
    double sumSquared = 0;
    for( size_t index = 0; index < vector.size(); index++ )
        sumSquared += pow( vector[index], 2.0 );
    sumSquared = pow( sumSquared, 0.5 );
    for( size_t index = 0; index < vector.size(); index++ )
        vector[index] /= sumSquared;
    return vector;
}
