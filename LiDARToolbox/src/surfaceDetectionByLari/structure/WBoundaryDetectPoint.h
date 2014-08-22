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

#ifndef WBOUNDARYDETECTPOINT_H
#define WBOUNDARYDETECTPOINT_H

#include <vector>
#include "../../common/datastructures/kdtree/WKdPointND.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"
#include "../../common/math/leastSquares/WLeastSquares.h"
#include "WSpatialDomainKdPoint.h"


/**
 * Point meta data object for detecting two dimensional boundary points and others 
 * inside them. The data set is used for the modified convex hull clustering that 
 * separates points that were assigned to a similar planar formula but could be spatially disconnected.
 */
class WBoundaryDetectPoint : public WKdPointND
{
public:
    /**
     * Creates an instance for boundary detection and points inside.
     * \param coordinate A three dimensional point. The three dimensional coordinate 
     *                   isn't considered during the analysis.
     */
    explicit WBoundaryDetectPoint( vector<double> coordinate );
    /**
     * Creates an instance for boundary detection and points inside.
     * \param x X coordinate of the instantiated point.
     * \param y Y coordinate of the instantiated point.
     * \param z Z coordinate of the instantiated point that isn't used for analysis.
     */
    explicit WBoundaryDetectPoint( double x, double y, double z );
    /**
     * Destroys the point meta data set.
     */
    virtual ~WBoundaryDetectPoint();

    /**
     * Gets the assigned spatial domain point (coordinate equal to input points).
     * \return Corresponding spatial domain point.
     */
    WSpatialDomainKdPoint* getSpatialPoint();
    /**
     * Rells that a point is already finally assigned to a cluster during separating 
     * points that belong to the same planar formula potentially being spatially 
     * disconnected.
     * \return Point is added to a plane finally or not.
     */
    bool isAddedToPlane();
    /**
     * Sets whether a point is already finally assigned to a cluster during separating 
     * points that belong to the same planar formula potentially being spatially 
     * disconnected.
     * \param isAddedToPlane Point is added to a cluster finally or not.
     */
    void setIsAddedToPlane( bool isAddedToPlane );
    /**
     * Sets the assigned spatial domain point (coordinate equal to input points).
     * \param assignedSpatialPoint Assitned spatial domain point.
     */
    void setSpatialPoint( WSpatialDomainKdPoint* assignedSpatialPoint );

private:
    /**
     * Z coordinate that isn't used for further analysis.
     */
    double m_zCoordinate;
    /**
     * Spatial domain that is assigned to that data set. Its coordinate corresponds to 
     * an input point.
     */
    WSpatialDomainKdPoint* m_assignedSpatialPoint;
    /**
     * Point is added to a cluster finally or not during the modified convex hull 
     * clustering.
     */
    bool m_isAddedToPlane;
};

#endif  // WBOUNDARYDETECTPOINT_H
