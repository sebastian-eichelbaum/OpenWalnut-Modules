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

#ifndef WLARIGEOPROPS_H
#define WLARIGEOPROPS_H

#include <vector>
#include "core/common/math/linearAlgebra/WVectorFixed.h"

using std::vector;

/**
 * This class holds all necessary parameters for each single point used by the process 
 * of the Lari/Habib (2014).
 */
class WLariGeoProps
{
public:
    explicit WLariGeoProps();

    virtual ~WLariGeoProps();
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

#endif  // WLARIGEOPROPS_H
