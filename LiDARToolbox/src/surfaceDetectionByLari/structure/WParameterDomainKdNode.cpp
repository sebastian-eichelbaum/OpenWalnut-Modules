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
#include "WParameterDomainKdNode.h"

WParameterDomainKdNode::WParameterDomainKdNode( size_t dimensions ) : WKdTreeND( dimensions )
{
}
WParameterDomainKdNode::~WParameterDomainKdNode()
{
}


vector<double> WParameterDomainKdNode::getEigenValues()
{
    return m_eigenValues;
}

WKdTreeND* WParameterDomainKdNode::getNewInstance( size_t dimensions )
{
    return new WParameterDomainKdNode( dimensions );
}


void WParameterDomainKdNode::setEigenValues( vector<double> eigenValues )
{
    m_eigenValues = eigenValues;
}
void WParameterDomainKdNode::setEigenVectors( vector<WVector3d> eigenVectors )
{
    m_eigenVectors = eigenVectors;
}
void WParameterDomainKdNode::setParametersXYZ0( vector<double> parametersXYZ0 )
{
    m_parametersXYZ0 = parametersXYZ0;
}
vector<double> WParameterDomainKdNode::getParametersXYZ0()
{
    return m_parametersXYZ0;
}
