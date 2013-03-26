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

#ifndef WGAUSSPROCESS_H
#define WGAUSSPROCESS_H

#include <Eigen/Core>

#include "core/common/math/WMatrix.h"
#include "core/common/math/WValue.h"
#include "core/common/WBoundingBox.h"
#include "core/dataHandler/WDataSetDTI.h"
#include "core/dataHandler/WDataSetFibers.h"

class WFiber;

/**
 * Represents a basic Gaussian process with its mean- and covariance function. Basically this aims
 * to implement a part of the Gaussian process framework as presented by Wasserman et. al:
 * http://dx.doi.org/10.1016/j.neuroimage.2010.01.004
 */
class WGaussProcess
{
friend class WGaussProcessTest;
public:
    /**
     * Constructs a Gaussian process out of a fiber with the help of underlying diffusion tensor
     * information.
     *
     * \param tractID One deterministic tractogram ID
     * \param tracts All deterministic tractograms
     * \param tensors All 2nd order diffusion tensors
     * \param maxLevel The real number which represents the maximum intensity
     */
    WGaussProcess( size_t tractID,
                   boost::shared_ptr< const WDataSetFibers > tracts,
                   boost::shared_ptr< const WDataSetDTI > tensors,
                   double maxLevel = 1.0 );

    /**
     * Default destructor.
     */
    virtual ~WGaussProcess();

    /**
     * Computes the mean function describing this gausprocess as described in the Wassermann paper,
     * see equation (16) on page 12.
     *
     * \param p The point where to evaluate the Gauss process.
     *
     * \return The mean value of this Gaussian process at the point \e p.
     */
    double mean( const WPosition& p ) const;

    /**
     * Returns the reference to precomputed \f$ C_{ff}^{-1} * 1 * l \f$ see \ref m_Cff_1_l_product.
     *
     * \return Refercene to the vector.
     */
    const Eigen::VectorXd& getCff1lProduct() const;

    /**
     * Generates with the help of the \c WDataSetFibers and the \ref m_tractID a \c WFiber
     * instance, for easily accessing the points sequently.
     *
     * \note: This may be time and space consuming! Be careful!
     *
     * \return Copy of the \c WFiber instance reffered by \ref m_tractID
     */
    WFiber generateTract() const;

    /**
     * As each Gaussian process is associated with a \c WFiber it also hat the maximal segment length,
     * used as width for the Gaussian kernels around the base points.
     *
     * \return Copy of the maximal segment length this Gauss process is associated with.
     */
    double getRadius() const;

    /**
     * Covariance function of two points representing the smoothness of the tract.
     *
     * \param p1 First point
     * \param p2 Second point
     *
     * \return Real number indicating the covariance between two points representing the smoothness
     * of the tract
     */
    double cov_s( const WPosition& p1, const WPosition& p2 ) const;

    /**
     * Returns the precomputed bounding box, see \ref m_bb.
     *
     * \return Const reference to the AABB for read only access.
     */
    const WBoundingBox& getBB() const;

protected:
private:
    /**
     * Computes the covariance matrix and invert it. This shall always be possible since the
     * creating function is positive definit. (TODO(math): at least Demain said this)
     *
     * \param tract The tract as vector of points, which is easery to access
     * than to fiddle around with the indices inside the WDataSetFibers
     *
     * \return Copy of the Cff inverse. Is not saved as member since its almost never used and may be huge!
     */
    Eigen::MatrixXd generateCffInverse( const WFiber& tract );

    /**
     * Computes tau parameter representing the max diffusion time.
     *
     * \see m_tau
     *
     * \return The parameter tau
     */
    double generateTauParameter();

    /**
     * Covariance function of two points representing the blurring of the tract's diffusion.
     *
     * \param p1 First point
     * \param p2 Second point
     *
     * \return Real number indicating the covariance between two points representing the diffusion
     * associated blurring.
     */
    double cov_d( const WPosition& p1, const WPosition& p2 ) const;

    /**
     * Covariance function of this Gaussian process.
     *
     * \note The reason why this isn't realized as member is just simplicity. Maybe we have time to
     * change this!
     *
     * \param p1 First point
     * \param p2 Second point
     *
     * \return The sum of the cov_s and cov_d covariance function.
     */
    double cov( const WPosition& p1, const WPosition& p2 ) const;

    /**
     * The id of the tract inside the \c WDataSetFibers this Gaussian process is representing.
     * This is needed since the sample points of a tract will be needed for mean computation. (\f$
     * S_f(p) \f$ will need all \f$ f_i \f$i)
     *
     * \note: we can't make this a const member, since this instance cannot then be used inside of
     * std::vectors which needs a default assignment operator which will fail on const members.
     */
    size_t m_tractID;

    /**
     * Read-only reference to the tractogram dataset to save memory.
     */
    boost::shared_ptr< const WDataSetFibers > m_tracts;

    /**
     * Read-only reference to the tensor field used for the covariance function \ref cov.
     */
    boost::shared_ptr< const WDataSetDTI > m_tensors;

    /**
     * This is the vector defined by:  \f$ C_{ff}^{-1} * 1 * l \f$ as used for example in eq. (16)
     * of the appendix of the Demain Wassermann paper. Since it is used this way several times this
     * is a member.  Where Cff is the covariance matrix of all pairs of sample points of the tract
     * using the \ref cov function.
     */
    Eigen::VectorXd m_Cff_1_l_product;

    /**
     * The maximal diffusion time (tau) as described in the Wassermann paper line 790, page 12 after
     * equation (16).
     */
    double m_tau;

    /**
     * Max segment length of the tract describing the R environment.
     */
    double m_R;

    /**
     * The MaximumLevelSet, representing the real number which is used for blurring the sample
     * point. No other point has a bigger values after the blurring.
     */
    double m_maxLevel;

    /**
     * Axis aligned bounding box (AABB) for the tract, not including the radius of the gaussian
     * kernels denoted by \ref m_R.
     */
    WBoundingBox m_bb;
};

inline double WGaussProcess::getRadius() const
{
    return m_R;
}

inline double WGaussProcess::cov_s( const WPosition& p1, const WPosition& p2 ) const
{
    double r = length( p1 - p2 );
    if( r >  m_R )
    {
        return 0.0;
    }
    return 2 * std::abs( r * r * r ) - 3 * m_R * r * r + m_R * m_R * m_R;
}

inline double WGaussProcess::cov( const WPosition& p1, const WPosition& p2 ) const
{
    return cov_s( p1, p2 ); // not implemented the cov_d yet: + cov_d( p1, p2 );
}

inline const Eigen::VectorXd& WGaussProcess::getCff1lProduct() const
{
    return m_Cff_1_l_product;
}

namespace gauss
{
    /**
     * The inner product of two Gaussian processes. See appendix A.3 of the Demian Wassermann paper.
     *
     * \param p1 First Gaussian process
     * \param p2 Second Gaussian process
     *
     * \return The similarity of those two processes, aka inner product.
     */
    double innerProduct( const WGaussProcess& p1, const WGaussProcess& p2 );
}

#endif  // WGAUSSPROCESS_H
