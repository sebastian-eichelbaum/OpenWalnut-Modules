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

#ifndef WCULLINGSTRATEGYINTERFACE_H
#define WCULLINGSTRATEGYINTERFACE_H

#include <utility>

#include "core/common/WProgress.h"
#include "core/dataHandler/WDataSetFibers.h"

/**
 * Every fiber culling strategy must implement this interface inorder to be used within the culling module.
 */
class WCullingStrategyInterface
{
public:
    /**
     * Sort out fibers.
     *
     * \param fibers Fibers to check.
     * \param progress Progress object to report back the progress to the module.
     *
     * \return First dataset contains all remaining fibers which survied, second all fibers which were sorted out.
     */
    virtual std::pair< WDataSetFibers::SPtr, WDataSetFibers::SPtr > operator()( WDataSetFibers::SPtr fibers, WProgress::SPtr progress ) = 0;

    /**
     * Destructor.
     */
    virtual ~WCullingStrategyInterface();
};

#endif  // WCULLINGSTRATEGYINTERFACE_H
