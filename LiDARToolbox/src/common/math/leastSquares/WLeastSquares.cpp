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
#include "WLeastSquares.h"
#include "../../math/vectors/WVectorMaths.h"

WLeastSquares::WLeastSquares()
{
    m_positions = 0;
    m_dimensions = 3;
    m_verticalDimension = 2;
}

WLeastSquares::WLeastSquares( size_t dimensions )
{
    m_positions = 0;
    m_dimensions = dimensions;
    m_verticalDimension = 2;
}

WLeastSquares::~WLeastSquares()
{
}

void WLeastSquares::analyzeData( vector<WPosition>* data )
{
    m_positions = data;
    calculatePerpendicularDimension();
    calculateMatrices();
    calculateHessianNormalForm();
}

vector<double> WLeastSquares::getHessianNormalForm()
{
    return m_hessianNormalForm;
}

vector<double> WLeastSquares::getNormalVectorNotNormalized()
{
    vector<double> normalVector;
    for( size_t dimension = 0; dimension < m_hessianNormalForm.size() - 1; dimension++ )
        normalVector.push_back( m_hessianNormalForm[dimension] );
    return normalVector;
}

void WLeastSquares::calculatePerpendicularDimension()
{
    WPrincipalComponentAnalysis pca;
    pca.analyzeData( *m_positions );
    vector<double> eigenValues = pca.getEigenValues();
    double smallestEigenValue = eigenValues[0];
    size_t perpendicularEigenVector = 0;

    for( size_t index = 0; index < eigenValues.size(); index++ )
        if( eigenValues[index] < smallestEigenValue )
        {
            smallestEigenValue = eigenValues[index];
            perpendicularEigenVector = index;
        }

    m_verticalDimension = 0;
    WVector3d eigenVector = pca.getEigenVectors()[perpendicularEigenVector];
    double biggestValue = abs( eigenVector[0] );
    for( size_t index = 0; index < eigenVector.size(); index++ )
        if( abs( eigenVector[index] ) > biggestValue )
        {
            biggestValue = abs( eigenVector[index] );
            m_verticalDimension = index;
        }
}

void WLeastSquares::calculateMatrices()
{
    if( m_positions->size() > 0 )
    {
        MatrixXd newMatrixX( m_positions->size(), m_dimensions );
        m_matrixX = newMatrixX;
        MatrixXd newMatrixY( m_positions->size(), 1 );
        m_matrixY = newMatrixY;

        for( size_t row = 0; row < m_positions->size(); row++ )
        {
            for( size_t col = 1; col < m_dimensions; col++ )
                m_matrixX( row, col ) = m_positions->at( row )
                        [col <= m_verticalDimension ?col - 1 :col];
            m_matrixX( row, 0 ) = 1.0;
            m_matrixY( row, 0 ) = m_positions->at( row )[m_verticalDimension];
        }
    }
}

void WLeastSquares::calculateHessianNormalForm()
{
    MatrixXd matrixXTranspose = m_matrixX.transpose();
    MatrixXd result = ( matrixXTranspose * m_matrixX ).inverse() * matrixXTranspose * m_matrixY;
    m_hessianNormalForm.reserve( m_dimensions + 1 );
    m_hessianNormalForm.resize( m_dimensions + 1 );
    m_hessianNormalForm[m_hessianNormalForm.size() - 1] = -result( 0, 0 );
    m_hessianNormalForm[m_verticalDimension] = 1.0;
    for( size_t index = 1; index < m_dimensions; index++ )
        m_hessianNormalForm[index <= m_verticalDimension ?index - 1 :index] =
                -result( index, 0 );
}

vector<double> WLeastSquares::getParametersXYZ0_()
{
    return getParametersXYZ0( m_hessianNormalForm );
}

vector<double> WLeastSquares::getParametersXYZ0( const vector<double>& hessianNormalForm )
{
    vector<double> parameters;
    double a = hessianNormalForm[0];
    double b = hessianNormalForm[1];
    double c = hessianNormalForm[2];
    double d = hessianNormalForm[3];
    double a2 = a * a;
    double b2 = b * b;
    double c2 = c * c;
    double sum = a2 + b2 + c2;

    parameters.push_back( -a * d / sum );
    parameters.push_back( -b * d / sum );
    parameters.push_back( -c * d / sum );
    return parameters;
}

double WLeastSquares::getDistanceToPlane( WPosition point )
{
    double normalAbsolute = 0;
    for( size_t dimension = 0; dimension < m_dimensions; dimension++ )
        normalAbsolute += pow( m_hessianNormalForm[dimension], 2.0 );
    normalAbsolute = pow( normalAbsolute, 0.5 );

    double distance = 0;
    for( size_t dimension = 0; dimension < m_dimensions; dimension++ )
        distance += point[dimension] * m_hessianNormalForm[dimension];
    return ( distance + m_hessianNormalForm[m_dimensions] ) / normalAbsolute;
}

WPosition WLeastSquares::getNearestPointTo( WPosition point )
{
    return getNearestPointTo( m_hessianNormalForm, point );
}

WPosition WLeastSquares::getNearestPointTo( const vector<double>& planeHessianNormalForm, WPosition point )
{
    double dimensions = planeHessianNormalForm.size() - 1;
    double amountN = planeHessianNormalForm[dimensions];
    double amountR = 0;
    for(size_t dimension = 0; dimension < dimensions; dimension++ )
    {
        amountN += planeHessianNormalForm[dimension] * point[dimension];
        amountR += planeHessianNormalForm[dimension] * planeHessianNormalForm[dimension];
    }
    double r = -amountN / amountR;

    WPosition cuttingPoint( point[0], point[1], point[2] );
    for( size_t dimension = 0; dimension < dimensions; dimension++ )
        cuttingPoint[dimension] += planeHessianNormalForm[dimension] * r;
    return cuttingPoint;
}
