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

#ifndef WZHANGTHRESHOLDCULLING_H
#define WZHANGTHRESHOLDCULLING_H

#include <utility>

#include <core/common/WProgress.h>
#include <core/dataHandler/WDataSetFibers.h>
#include <core/common/WObjectNDIP.h>

#include "WCullingStrategyInterface.h"

/**
 * Culls out short fibers along long fibers. This is based on a point based distance measure from Zhang et al. http://dx.doi.org/10.1109/TVCG.2008.52.
 */
class WZhangThresholdCulling : public WObjectNDIP< WCullingStrategyInterface >
{
public:
    /**
     * Default constructor.
     */
    WZhangThresholdCulling();

    /**
     * Destructor.
     */
    virtual ~WZhangThresholdCulling();

    /**
     * Sort out fibers.
     *
     * \param fibers Fibers to check.
     * \param progress Progress object to report back the progress to the module
     *
     * \return First dataset contains all remaining fibers which survied, second all fibers which were sorted out.
     */
    virtual std::pair< WDataSetFibers::SPtr, WDataSetFibers::SPtr > operator()( WDataSetFibers::SPtr fibers, WProgress::SPtr progress );

protected:
    /**
     * If a pair of fiber have a distance below this threshold, the smaller one is culled out.
     */
    WPropDouble m_dSt;

    /**
     * Point distances below this threshold collapse to null.
     */
    WPropDouble m_proximity;
};

#endif  // WZHANGTHRESHOLDCULLING_H
