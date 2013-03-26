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

#ifndef WEDGEBUNDLINGCPU_H
#define WEDGEBUNDLINGCPU_H

#include <core/common/WObjectNDIP.h>
#include <core/common/WProperties.h>

#include "WEdgeBundlingInterface.h"

class WEdgeBundlingCPU : public WObjectNDIP< WEdgeBundlingInterface >
{
public:
    WEdgeBundlingCPU();

    virtual ~WEdgeBundlingCPU();

    virtual WDataSetFibers::SPtr operator()( WProgress::SPtr progress, WBoolFlag const &shutdown, WDataSetFibers::SPtr fibers, WDataSetScalar::SPtr mask );
protected:
private:
    /**
     * Stiffness, 0 means all points of a fiber may change position, 1 means all points of a fiber may not change position.
     */
    WPropDouble m_stiffness;

    /**
     * If true, end points of fibers may not change position, even if stiffness would allow this.
     */
    WPropBool m_fixedEndings;

    /**
     * Maximal allowed curvature between a number of segments. 0 means, only straight segments are allowed, 1 means every possible cuvature is allowed.
     */
    WPropDouble m_maxCurvature;

    /**
     * Number of segements used for curvature computation. 0 disables curvature threshold, 1 means: last segment and
     * current segment is used, 2 means: the last two segments and current segments contributes to curvature and so on.
     */
    WPropInt m_curveSegments;

    /**
     * There should be a minimal distance between points. 0 means, two points may have exact the same position.
     */
    WPropDouble m_minDistance;

    /**
     * Percentage up to which a fiber may be elongated. 0 percent means, length must be conserved. 100 percent means length may be doubled.
     */
    WPropDouble m_maxExtension;

    /**
     * Percentage up to which a fiber may be shrunk. 0 percent means, length must be conserved. 50 percent means length may be halved.
     */
    WPropDouble m_maxContraction;

    /**
     * Only segments within this radius should contribute to attraction computation. 0 disables this feature and all segments may contribute.
     */
    WPropDouble m_maxRadius;

    /**
     * Strength of angle based attraction.
     */
    WPropDouble m_angleBasedAttraction;

};

#endif  // WEDGEBUNDLINGCPU_H
