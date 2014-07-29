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
    clearMatrices();
    calculateMatrices();
    calculateHessescheNormalForm();
}
vector<double> WLeastSquares::getHessescheNormalForm()
{
    return m_hessescheNormalForm;
}
void WLeastSquares::calculatePerpendicularDimension()
{
    WPrincipalComponentAnalysis pca;
    pca.analyzeData( m_positions );
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
    WVector3d eigenVector = pca.getDirections()[perpendicularEigenVector];
    double biggestValue = abs( eigenVector[0] );
    for( size_t index = 0; index < eigenVector.size(); index++ )
        if( abs( eigenVector[index] ) > biggestValue )
        {
            biggestValue = abs( eigenVector[index] );
            m_verticalDimension = index;
        }
}
void WLeastSquares::clearMatrices()
{
    MatrixXd newMatrixA( m_dimensions, m_dimensions );
    m_matrixA = newMatrixA;
    MatrixXd newMatrixB( m_dimensions, 1 );
    m_matrixB = newMatrixB;

    for( size_t row = 0; row < m_dimensions; row++ )
    {
        for( size_t col = 0; col < m_dimensions; col++ )
            m_matrixA( row, col ) = 0.0;
        m_matrixB( row, 0 ) = 0.0;
    }
}
void WLeastSquares::calculateMatrices()
{
    for( size_t index = 0; index < m_positions->size(); index++ )
    {
        WPosition pos = m_positions->at( index );
        for( size_t row = 0; row < m_dimensions; row++ )
        {
            size_t rowMat = row <= m_verticalDimension ?row :row - 1;
            if( row == m_verticalDimension ) rowMat = m_dimensions - 1;
            for( size_t col = 0; col < m_dimensions; col++ )
            {
                size_t colMat = col <= m_verticalDimension ?col :col - 1;
                if( col == m_verticalDimension )
                    colMat = m_dimensions - 1;

                double resultA = 1.0;
                if( row != m_verticalDimension )
                    resultA *= pos[row];
                if( col != m_verticalDimension )
                    resultA *= pos[col];
                m_matrixA( rowMat, colMat ) += resultA;
            }
            double resultB = pos[m_verticalDimension];
            if( row != m_verticalDimension )
                resultB *= pos[row];
            m_matrixB( rowMat, 0 ) += resultB;
        }
    }
}
void WLeastSquares::calculateHessescheNormalForm()
{
    MatrixXd matrixX = m_matrixA.inverse()*m_matrixB;

    m_hessescheNormalForm.reserve( m_dimensions + 1 );
    m_hessescheNormalForm.resize( m_dimensions + 1 );
    for( size_t index = 0; index < m_hessescheNormalForm.size(); index++ )
    {
        size_t indexMat = index <= m_verticalDimension ?index : index - 1;

        m_hessescheNormalForm[index] =
                index != m_verticalDimension
                ?-matrixX( indexMat, 0 ) :1;
    }

    double sumSquared = 0;
    for( size_t index = 0; index < m_dimensions; index++ )
        sumSquared += pow( m_hessescheNormalForm[index], 2.0 );
    sumSquared = pow( sumSquared, 0.5 );
    //for( size_t index = 0; index < m_dimensions; index++ )
    //    m_hessescheNormalForm[index] /= sumSquared;
}
vector<double> WLeastSquares::getParametersXYZ0()
{
    return getParametersXYZ0( m_hessescheNormalForm );
}
vector<double> WLeastSquares::getParametersXYZ0( vector<double> hessescheNormalForm )
{
    vector<double> parameters;
    double a = hessescheNormalForm[0];
    double b = hessescheNormalForm[1];
    double c = hessescheNormalForm[2];
    double d = hessescheNormalForm[3];
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
        normalAbsolute += pow( m_hessescheNormalForm[dimension], 2.0 );
    normalAbsolute = pow( normalAbsolute, 0.5 );

    double distance = 0;
    for( size_t dimension = 0; dimension < m_dimensions; dimension++ )
        distance += point[dimension] * m_hessescheNormalForm[dimension];
    return ( distance + m_hessescheNormalForm[m_dimensions] ) / normalAbsolute;
}
WPosition WLeastSquares::getNearestPointTo( WPosition point )
{
    return getNearestPointTo( m_hessescheNormalForm, point );
}
WPosition WLeastSquares::getNearestPointTo( vector<double> planeHessescheNormalForm, WPosition point )
{
    double dimensions = planeHessescheNormalForm.size() - 1;
    double amountN = planeHessescheNormalForm[dimensions];
    double amountR = 0;
    for(size_t dimension = 0; dimension < dimensions; dimension++ )
    {
        amountN += planeHessescheNormalForm[dimension] * point[dimension];
        amountR += planeHessescheNormalForm[dimension] * planeHessescheNormalForm[dimension];
    }
    double r = -amountN / amountR;

    WPosition cuttingPoint( point[0], point[1], point[2] );
    for( size_t dimension = 0; dimension < dimensions; dimension++ )
        cuttingPoint[dimension] += planeHessescheNormalForm[dimension] * r;
    return cuttingPoint;
}
