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
    virtual ~WLeastSquares();
    /**
     * Launchs the least squares adjustment.
     * \param data Point data to analyze the best fitted plane for.
     */
    void analyzeData( vector<WPosition>* data );
    /**
     * Returns the plane formula for the best fitted plane.
     * \return The hessesche Normal Form of the best fitted plane. First n numbers (by 
     *         the dimensions coordinate count) represent the normal vector of the best 
     *         fitted plane. The last one is the perpendicular euclidian distance to the 
     *         coordinate system orign.
     */
    vector<double> getHessescheNormalForm();
    /**
     * Returns the parameters X_0, Y_0 and Z_0 etc. of the best fitted plane in an n 
     * dimensional space.
     * \return Parameter space coordinates corresponding to Lari/Habib 2014.
     */
    vector<double> getParametersXYZ0();

private:
    /**
     * Returns the dimension with the smallest extent
     * \return Dimension with the biggest eigen vector coordonate extent of the smallest Eigen 
     *         Value.
     */
    void calculatePerpendicularDimension();
    /**
     * Erases the matrices required during the calculation.
     */
    void clearMatrices();
    /**
     * The algorithm solves the matrix formula A*x=B. This method calculates A and B.
     */
    void calculateMatrices();
    /**
     * The algorithm solves the matrix formula A*x=B. This method at first calculates 
     * the matrix x and then the Hessesche Normal Form of the best fitted plane.
     */
    void calculateHessescheNormalForm();

    /**
     * Dimension definition of the coordinate system.
     */
    size_t m_dimensions;
    /**
     * Points to calculate the best fitted plane for.
     */
    vector<WPosition>* m_positions;
    /**
     * Space for the calculated Hessesche Normal Form.
     */
    vector<double> m_hessescheNormalForm;
    /**
     * Dimension with the biggest eigen vector coordonate extent of the smallest Eigen Value.
     */
    size_t m_verticalDimension;
    /**
     * Matrix A of the least squares adjustment,
     */
    MatrixXd m_matrixA;
    /**
     * Matrix A of the least squares adjustment,
     */
    MatrixXd m_matrixB;
};

#endif  // WLEASTSQUARES_H
