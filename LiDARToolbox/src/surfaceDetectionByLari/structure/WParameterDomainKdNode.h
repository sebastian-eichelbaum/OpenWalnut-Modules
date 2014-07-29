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

#ifndef WPARAMETERDOMAINKDNODE_H
#define WPARAMETERDOMAINKDNODE_H

#include <vector>
#include "../../common/datastructures/kdtree/WKdTreeND.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"

/**
 * This kd tree node enables nodes to hold metadata for each single point used by the 
 * process of the Lari/Habib (2014).
 */
class WParameterDomainKdNode : public WKdTreeND
{
public:
    /**
     * Instantiates the kd tree node
     * \param dimensions The dimensions count
     */
    explicit WParameterDomainKdNode( size_t dimensions );

    virtual ~WParameterDomainKdNode();
    /**
     * Returns the eigen value of the point in relation to its neighbor points.
     * \return Point's Eigen Values in relation to its neighbor points.
     */
    vector<double> getEigenValues();

    /**
     * Sets the eigens values of the point in relation to its neighbors.
     * \param eigenValues The eigen values of a poinnt in relation to its neighbors.
     */
    void setEigenValues( vector<double> eigenValues );
    /**
     * Sets the eigens vectors of the point in relation to its neighbors.
     * \param eigenVectors The eigen vectors of a poinnt in relation to its neighbors.
     */
    void setEigenVectors( vector<WVector3d> eigenVectors );
    /**
     * Sets the parameter domain coordinates of the point.
     * \param parametersXYZ0 The parameter domain coordinates of the point.
     */
    void setParametersXYZ0( vector<double> parametersXYZ0 );
    /**
     * Returns the parameter domain coordinates of the point.
     * \return The parameter domain coordinates of the point.
     */
    vector<double> getParametersXYZ0();

protected:
    /**
     * Enables the derived kd tree node class to create a new instance of that kd tree 
     * node class type.
     * \param dimensions The dimension count of the new kd tree node.
     * \return a new kd tree node instance of that class.
     */
    virtual WKdTreeND* getNewInstance( size_t dimensions );

private:
    /**
     * The eigen values of a poinnt in relation to its neighbors.
     */
    vector<double> m_eigenValues;
    /**
     * The eigen vectors of a poinnt in relation to its neighbors.
     */
    vector<WVector3d> m_eigenVectors;
    /**
     * Parameter domain coordinates of the point.
     */
    vector<double> m_parametersXYZ0;
};

#endif  // WPARAMETERDOMAINKDNODE_H
