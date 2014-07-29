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
#include "WSpatialDomainKdNode.h"

WSpatialDomainKdNode::WSpatialDomainKdNode( size_t dimensions ) : WKdTreeND( dimensions )
{
}
WSpatialDomainKdNode::~WSpatialDomainKdNode()
{
}


vector<double> WSpatialDomainKdNode::getEigenValues()
{
    return m_eigenValues;
}

vector<double> WSpatialDomainKdNode::getHessescheNormalForm()
{
    return m_hessescheNormalForm;
}
vector<double> WSpatialDomainKdNode::getParametersXYZ0()
{
    return WLeastSquares::getParametersXYZ0( m_hessescheNormalForm );
}
WKdTreeND* WSpatialDomainKdNode::getNewInstance( size_t dimensions )
{
    return new WSpatialDomainKdNode( dimensions );
}


void WSpatialDomainKdNode::setEigenValues( vector<double> eigenValues )
{
    m_eigenValues = eigenValues;
}
void WSpatialDomainKdNode::setEigenVectors( vector<WVector3d> eigenVectors )
{
    m_eigenVectors = eigenVectors;
}
void WSpatialDomainKdNode::setHessescheNormalForm( vector<double> hessescheNormalForm )
{
    m_hessescheNormalForm = hessescheNormalForm;
}
