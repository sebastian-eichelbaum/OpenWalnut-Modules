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

vector<double> WSpatialDomainKdPoint::getHessescheNormalForm()
{
    return m_hessescheNormalForm;
}
vector<double> WSpatialDomainKdPoint::getParametersXYZ0()
{
    return WLeastSquares::getParametersXYZ0( m_hessescheNormalForm );
}


void WSpatialDomainKdPoint::setEigenValues( vector<double> eigenValues )
{
    m_eigenValues = eigenValues;
}
void WSpatialDomainKdPoint::setEigenVectors( vector<WVector3d> eigenVectors )
{
    m_eigenVectors = eigenVectors;
}
void WSpatialDomainKdPoint::setHessescheNormalForm( vector<double> hessescheNormalForm )
{
    m_hessescheNormalForm = hessescheNormalForm;
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
