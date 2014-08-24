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

#ifndef WPARAMETERDOMAINKDPOINT_H
#define WPARAMETERDOMAINKDPOINT_H

#include <vector>
#include "../../common/datastructures/kdtree/WKdPointND.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"
#include "WSpatialDomainKdPoint.h"

/**
 * This kd tree point enables nodes to hold metadata for each single parameter domain 
 * point used by the process of the Lari/Habib (2014).
 */
class WParameterDomainKdPoint : public WKdPointND
{
public:
    /**
     * Instantiates a parameter domain point in an n dimensional space. A parameter 
     * coordinate used here is calculated using the Hesse normal form as follows: 
     * X_0=(-A*D)(A^2+B^2+C^2), Y_0=(-B*D)(A^2+B^2+C^2), Z_0=(-C*D)(A^2+B^2+C^2), ...
     * \param coordinate Parameter space coordinate corresponding to the Lari/Habib 
     *                   approach.
     */
    explicit WParameterDomainKdPoint( vector<double> coordinate );
    /**
     * Instantiates a parameter domain point in an three dimensional space. A parameter 
     * coordinate used here is calculated using the Hesse normal form as follows: 
     * X_0=(-A*D)(A^2+B^2+C^2), Y_0=(-B*D)(A^2+B^2+C^2), Z_0=(-C*D)(A^2+B^2+C^2), ...
     * \param x0 X_0 coordinate of the parameter space-
     * \param y0 Y_0 coordinate of the parameter space-
     * \param z0 Z_0 coordinate of the parameter space-
     */
    explicit WParameterDomainKdPoint( double x0, double y0, double z0 );
    /**
     * Destroys a parameter space point object.
     */
    virtual ~WParameterDomainKdPoint();
    /**
     * Assigns a spatial domain point to a corresponding parameter domain point so that 
     * the parameter domain point can access its spatial point meta data.
     * \param spatialPoint Spatial domain point that should be assigned to its 
     *                     corresponding parameter domain point.
     */
    void setSpatialPoint( WSpatialDomainKdPoint* spatialPoint );
    /**
     * returns the point count of an extent which would have its peak center 
     * within that parameter domain coordinate.
     * \return Parameter domain point count within the extent with the coordinate of 
     *         this object's parameter coordinate as peak center
     */
    int getExtentPointCount();
    /**
     * Returns the parameter spatial domain point object that is assigned to this 
     * parameter domain point.
     * \return The spatial domain point that is assigned to this parameter domain point.
     */
    WSpatialDomainKdPoint* getSpatialPoint();
    /**
     * Says whether the parameter domain point is already added to a plane point cluster.
     * \return Parameter domain point is added to a plane or not.
     */
    bool isAddedToPlane();
    /**
     * During the brute force extent estimation extent sizes change after removing 
     * parameters from the list that wait for assignment and lie in an extent that can 
     * possibly intersect with an already added extent. So parameters have to be marked 
     * for extent point count recalculation.
     * \return Point is tagged to refresh or not.
     */
    bool isTaggedToRefresh();
    /**
     * During the brute force peak center estimation each parameter domain point is 
     * considered as a peak center. So for each one the extent point count is calculated 
     * to take the biggest one within each round.
     * This method sets the xtent point count for each parameter domain point.
     * \param extentPointCount Extent point count of an extent with the coordinate of 
     *                         this parameter domain point as peak center.
     */
    void setExtentPointCount( int extentPointCount );
    /**
     * Sets whether a parameter that is added to plane or not. Assigned points to a 
     * plane cluster aren't treated further.
     * \param isAddedToPlane The parameter domain point is added to a plane or not.
     */
    void setIsAddedToPlane( bool isAddedToPlane );
    /**
     * During the brute force plane point cluster extent estimation points are removed 
     * from the list of parameter points which wait for assignment to a plane cluster to 
     * be calculated. Each round the biggest extent is taken. When added points are 
     * removed from the list of the points of the waiting list, so neighbor parameters 
     * must be marked to recalculate the extent point count.
     * \param setToRefreshed Parameter domain point count within the extent with the 
     *                       peak center with this parameter coordinate as peak center 
     *                       should be recalculated or not.
     */
    void tagToRefresh( bool setToRefreshed );

private:
    /**
     * Link to the corresponding spatial domain point.
     */
    WSpatialDomainKdPoint* m_assignedSpatialNode;
    /**
     * The parameter domain point count within an extent with the coordinate of this 
     * object as peak center. After each brute force plane point cluster detection 
     * estimation the biggest extent is picket out.
     */
    int m_extentPointCount;
    /**
     * Temporary tag which tells whether a parameter domain point is added to a plane or 
     * not. Each brute force extent estimation round already added parameters aren't 
     * treated further.
     */
    bool m_isAddedToPlane;
    /**
     * After removing parameters from the waiting list points neighbor still remain. 
     * After removing parameters the extent point count has to be tagged to be 
     * recalculated before the next extent estimation round.
     */
    bool m_markedToRefresh;
};

#endif  // WPARAMETERDOMAINKDPOINT_H
