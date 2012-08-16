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

#ifndef WSAMPLER2D_H
#define WSAMPLER2D_H

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include "core/common/math/linearAlgebra/WVectorFixed.h" // for WVector2d
#include "core/common/WMixinVector.h"

/**
 * Generates random points on a two dimensional disk (aka plane, aka slice).
 */
class WSampler2D : public WMixinVector< WVector2d >
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2D > SPtr;

};

/**
 * I hate bool flags, (as the calls yields in: foo( true, true, false )). To manage if we need to
 * call std::srand or not, this enum is used.
 */
enum RandomInit
{
    CALL_SRAND,
    DONT_CALL_SRAND
};

/**
 * Generates uniform distributed random positions within the given a rectangular domain.
 */
class WSampler2DUniform : public WSampler2D
{
public:
    /**
     * Shortcut for boost shared pointers on that objects.
     */
    typedef boost::shared_ptr< WSampler2DUniform > SPtr;

    /**
     * Generates random points distributed uniformly within the given rectangular 2D slice.
     *
     * \param numSamples How many samples.
     * \param width X dimension
     * \param height Y dimension
     * \param init Flag for calling srand or not
     */
    explicit WSampler2DUniform( size_t numSamples, double width = 1.0, double height = 1.0, RandomInit init = CALL_SRAND );

protected:
    /**
     * X dimension.
     */
    double m_width;

    /**
     * Y dimension.
     */
    double m_height;
};

/**
 * Loads points from a fixed poission disk sampling given in a (ASCII) file where each point is in a line and each line
 * is structured: x y.
 */
class WSampler2DPoissonFixed : public WSampler2D
{
public:
    explicit WSampler2DPoissonFixed( boost::filesystem::path path );
};

std::vector< WSampler2D > splitSampling( const WSampler2D& sampler, size_t numComponents );
std::vector< WSampler2D > splitSamplingPoisson( const WSampler2D& sampler, size_t numComponents );
#endif // WSAMPLER2D_H

