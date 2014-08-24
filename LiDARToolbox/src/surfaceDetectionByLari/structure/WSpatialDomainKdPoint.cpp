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
#include "WSpatialDomainKdPoint.h"

WSpatialDomainKdPoint::WSpatialDomainKdPoint( vector<double> coordinate ) : WKdPointND( coordinate )
{
}
WSpatialDomainKdPoint::WSpatialDomainKdPoint( double x, double y, double z ) : WKdPointND( x, y, z )
{
}
WSpatialDomainKdPoint::~WSpatialDomainKdPoint()
{
}


size_t WSpatialDomainKdPoint::getClusterID()
{
    return m_clusterID;
}
vector<double> WSpatialDomainKdPoint::getEigenValues()
{
    return m_eigenValues;
}

vector<double> WSpatialDomainKdPoint::getHesseNormalForm()
{
    return m_hesseNormalForm;
}
vector<double> WSpatialDomainKdPoint::getParametersXYZ0()
{
    return WLeastSquares::getParametersXYZ0( m_hesseNormalForm );
}

double WSpatialDomainKdPoint::getDistanceToNthNearestNeighbor()
{
    return m_distanceToNthNearestNeighbor;
}
size_t WSpatialDomainKdPoint::getIndexInInputArray()
{
    return m_indexInInputArray;
}
double WSpatialDomainKdPoint::getLocalPointDensity()
{
    return m_kNearestPoints / ( M_PI * pow( m_distanceToNthNearestNeighbor, 2.0 ) );
}
bool WSpatialDomainKdPoint::hasValidParameters()
{
    double infinityPositive = 1.0 / 0.0;
    double infinityNegative = -1.0 / 0.0;
    vector<double> parameters = getParametersXYZ0();
    for(size_t index = 0; index < parameters.size(); index++)
        if( parameters[index] == infinityPositive || parameters[index] == infinityNegative || parameters[index] != parameters[index] )
            return false;
    return true;
}
void WSpatialDomainKdPoint::setClusterID( size_t clusterID )
{
    m_clusterID = clusterID;
}
void WSpatialDomainKdPoint::setDistanceToNthNearestNeighbor( double distanceToNthNearestNeighbor )
{
    m_distanceToNthNearestNeighbor = distanceToNthNearestNeighbor;
}
void WSpatialDomainKdPoint::setEigenValues( vector<double> eigenValues )
{
    m_eigenValues = eigenValues;
    for( size_t index = 0; index < m_eigenValues.size(); index++ )
        if( m_eigenValues[index] < 0.0 )
            m_eigenValues[index] = 0.0;
}
void WSpatialDomainKdPoint::setEigenVectors( vector<WVector3d> eigenVectors )
{
    m_eigenVectors = eigenVectors;
}
void WSpatialDomainKdPoint::setHesseNormalForm( vector<double> hesseNormalForm )
{
    m_hesseNormalForm = hesseNormalForm;
}
void WSpatialDomainKdPoint::setKNearestPoints( size_t kNearestPoints )
{
    m_kNearestPoints = kNearestPoints;
}
void WSpatialDomainKdPoint::setIndexInInputArray( size_t indexInInputArray )
{
    m_indexInInputArray = indexInInputArray;
}
