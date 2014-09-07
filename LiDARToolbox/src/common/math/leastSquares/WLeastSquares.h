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

#ifndef WLEASTSQUARES_H
#define WLEASTSQUARES_H

#include <vector>
#include "core/common/math/linearAlgebra/WPosition.h"
#include "core/common/math/linearAlgebra/WVectorFixed.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include <Eigen/Dense>

using std::abs;
using std::cout;
using std::endl;
using std::pow;
using std::vector;

/**
 * Class that does the least sqhares adjustment on arbitrary points with n dimensions.
 * The procedure finds a plane with the least errors sum. Each error is multiplied with 
 * itself (count by the power of two) to calculate the entire error amount. The class 
 * calculates a plane with the smallest total error.
 */
class WLeastSquares
{
public:
    WLeastSquares();
    /**
     * Establishs a new least squares adjustment instance using an arbitraryly 
     * dimensional space.
     * \param dimensions Dimension definition of the space for the calculation.
     */
    explicit WLeastSquares( size_t dimensions );
    /**
     * Destroys the least squares adjustment instance.
     */
    virtual ~WLeastSquares();
    /**
     * Launchs the least squares adjustment.
     * \param data Point data to analyze the best fitted plane for.
     */
    void analyzeData( vector<WPosition>* data );
    /**
     * Returns the plane formula for the best fitted plane.
     * \return The Hessian normal form of the best fitted plane. First n numbers (by the 
     *         dimensions coordinate count) represent the normal vector of the best 
     *         fitted plane. The last one is the perpendicular euclidian distance to the 
     *         coordinate system orign.
     */
    vector<double> getHessianNormalForm();
    /**
     * Returns a not normalized normal vector of the least squares adjustment result.
     * \return A not normalized normal vector of the least squares adjustment result.
     */
    vector<double> getNormalVectorNotNormalized();
    /**
     * Returns the parameters X_0, Y_0 and Z_0 etc. of the best fitted plane in an n 
     * dimensional space.
     * \return Parameter space coordinates corresponding to Lari/Habib 2014.
     */
    vector<double> getParametersXYZ0_();    //TODO(aschwarzkopf): consider removal

    /**
     * Calculates a plane formula of the parameter domain using the approach of 
     * Lari/Habib using the Hessian normal form as input plane formula.
     * \param hessianNormalForm The Hessian normal formula of a plane.
     * \return Parameters X_0, Y_0 and Z_0 that describe a plane using the approach of 
     *         Lari/Habib.
     */
    static vector<double> getParametersXYZ0( const vector<double>& hessianNormalForm );    //TODO(aschwarzkopf): Consider removal
    /**
     * Returns a point distance to the currently calculated plane.
     * analyzeData() must have been executes.
     * \param point Point from which the plane distance is calculated.
     * \return Perpendicular distance from a point to the plane calculated py the least 
     *         squares algorithm.
     */
    double getDistanceToPlane( WPosition point );
    /**
     * Returns a nearest point to the input point's coordinate that lies on the 
     * calculated plane.
     * \param point Point to get the nearest coordinate on the plane of.
     * \return The nearest coordinate on the calculates plane of an arbitrary point.
     */
    WPosition getNearestPointTo( WPosition point );
    /**
     * Returns a nearest point to the input point's coordinate that lies on the 
     * calculated plane.
     * \param planeHessianNormalForm Plane on which the nearest point is found.
     * \param point Point to get the nearest coordinate on the plane of.
     * \return The nearest coordinate on the calculates plane of an arbitrary point.
     */
    static WPosition getNearestPointTo( const vector<double>& planeHessianNormalForm, WPosition point );

private:
    /**
     * Returns the dimension with the smallest extent
     * \return Dimension with the biggest eigen vector coordonate extent of the smallest Eigen 
     *         Value.
     */
    void calculatePerpendicularDimension();
    /**
     * The algorithm solves the matrix formula A*x=B. This method calculates A and B.
     */
    void calculateMatrices();
    /**
     * The algorithm solves the matrix formula A*x=B. This method at first calculates 
     * the matrix x and then the Hessian Normal Form of the best fitted plane.
     */
    void calculateHessianNormalForm();

    /**
     * Dimension definition of the coordinate system.
     */
    size_t m_dimensions;
    /**
     * Points to calculate the best fitted plane for.
     */
    vector<WPosition>* m_positions;
    /**
     * Space for the calculated hessian normal form.
     */
    vector<double> m_hessianNormalForm;
    /**
     * Dimension with the biggest eigen vector coordonate extent of the smallest Eigen Value.
     */
    size_t m_verticalDimension;
    /**
     * Independent variable: Values of dimensions.
     */
    MatrixXd m_matrixX;
    /**
     * Values of the dependent variable.
     */
    MatrixXd m_matrixY;
};

#endif  // WLEASTSQUARES_H
