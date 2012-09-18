//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#ifndef WRAYSAMPLE_H
#define WRAYSAMPLE_H

#include "core/common/math/WMatrix.h"
#include "core/common/math/linearAlgebra/WLinearAlgebra.h"

/**
 * Encapsulates all the possible sample data on a ray.
 */
class WRaySample
{
public:
    /**
     * Default constructor.
     */
    WRaySample();

    /**
     * Destructor.
     */
    virtual ~WRaySample();

    /**
     * Access the position of the sample point.
     *
     * \return the position reference.
     */
    const WVector4d& position() const;

    /**
     * Access the position of the sample point.
     *
     * \return the position reference.
     */
    WVector4d& position();

    /**
     * Access the value of the sample point.
     *
     * \return the value reference.
     */
    const double& value() const;

    /**
     * Access the value of the sample point.
     *
     * \return the value reference.
     */
    double& value();

    /**
     * Access the distance of the sample point to the XY plane (z=0).
     *
     * \return the distance reference.
     */
    const double& distance() const;

    /**
     * Access the distance of the sample point to the XY plane (z=0).
     *
     * \return the distance reference.
     */
    double& distance();

    /**
     * Access the value of the sample point.
     *
     * \return the value reference.
     */
    const WVector4d& gradient() const;

    /**
     * Access the gradient of the sample point.
     *
     * \return the gradient reference.
     */
    WVector4d& gradient();

    /**
     * Access the gradient length of the sample point.
     *
     * \return the gradient length reference.
     */
    const double& gradWeight() const;

    /**
     * Access the gradient length of the sample point.
     *
     * \return the gradient length reference.
     */
    double& gradWeight();

    /**
     * Access the fractional anisotropy of the sample point.
     *
     * \return the fractional anisotropy reference.
     */
    const double& fracA() const;

    /**
     * Access the fractional anisotropy of the sample point.
     *
     * \return the fractional anisotropy reference.
     */
    double& fracA();

    /**
     * Access the angle between ray and gradient of the sample point.
     *
     * \return the angle reference.
     */
    const double& angle() const;

    /**
     * Access the angle between ray and gradient of the sample point.
     *
     * \return the angle reference.
     */
    double& angle();

    /**
     * Access the mean curvature at the sample point.
     *
     * \return the mean curvature.
     */
    const double& meanCurv() const;

    /**
     * Access the mean curvature at the sample point.
     *
     * \return the mean curvature.
     */
    double& meanCurv();

    /**
     * Access the gauss curvature at the sample point.
     *
     * \return the gauss curvature.
     */
    const double& gaussCurv() const;

    /**
     * Access the gauss curvature at the sample point.
     *
     * \return the gauss curvature.
     */
    double& gaussCurv();

protected:
private:
    /**
     * The position of this sample point in the data grid.
     */
    WVector4d m_position;
    
    /**
     * The value at this sample point.
     */
    double m_value;

    /**
     * The distance at this sample point to the XY plane (z=0).
     */
    double m_distance;

    /**
     * The gradient at this sample point.
     */
    WVector4d m_gradient;

    /**
     * The gradient length at this sample point.
     */
    double m_gradWeight;

    /**
     * The fractional anisotropy at this sample point.
     */
    double m_fracAnisotropy;

    /**
     * The angle between ray and gradient at this sample point.
     */
    double m_angle;

    /**
     * The mean curvature at this sample point.
     */
    double m_meanCurv;

    /**
     * The gauss curvature at this sample point.
     */
    double m_gaussCurv;
};

#endif  // WRAYSAMPLE_H

