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

#ifndef WSPATIALDOMAINKDNODE_H
#define WSPATIALDOMAINKDNODE_H

#include <vector>
#include "../../common/datastructures/kdtree/WKdTreeND.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"
#include "../../common/math/leastSquares/WLeastSquares.h"

/**
 * This kd tree node enables nodes to hold metadata for each single point used by the 
 * process of the Lari/Habib (2014).
 */
class WSpatialDomainKdNode : public WKdTreeND
{
public:
    /**
     * Instantiates the kd tree node
     * \param dimensions The dimensions count
     */
    explicit WSpatialDomainKdNode( size_t dimensions );

    virtual ~WSpatialDomainKdNode();
    /**
     * Returns the eigen value of the point in relation to its neighbor points.
     * \return Point's Eigen Values in relation to its neighbor points.
     */
    vector<double> getEigenValues();
    /**
     * Returns the plane formula for the best fitted plane.
     * \return The hessesche Normal Form of the best fitted plane. First n numbers (by 
     *         the dimensions coordinate count) represent the normal vector of the best 
     *         fitted plane. The last one is the perpendicular euclidian distance to the 
     *         coordinate system orign.
     */
    vector<double> getHessescheNormalForm();
    /**
     * Returns the parameter domain coordinates of the point.
     * \return The parameter domain coordinates of the point.
     */
    vector<double> getParametersXYZ0();

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
     * Sets the Hessesche normal formula of the point's best fitted plane in relation to 
     * its neighbors.
     * \param hessescheNormalForm The Hessesche normal formula of the point's best 
     *                            fitted plane.
     */
    void setHessescheNormalForm( vector<double> hessescheNormalForm );

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
     * Space for the calculated Hessesche Normal Form.
     */
    vector<double> m_hessescheNormalForm;
};

#endif  // WSPATIALDOMAINKDNODE_H
