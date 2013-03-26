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

#ifndef WEDGEBUNDLINGINTERFACE_H
#define WEDGEBUNDLINGINTERFACE_H

#include <core/common/WProgress.h>
#include <core/common/WFlag.h>
#include <core/dataHandler/WDataSetFibers.h>
#include <core/dataHandler/WDataSetScalar.h>

/**
 * Interface all edge bundling algorithms for 3D fibers should follow.
 */
class WEdgeBundlingInterface
{
public:
    /**
     * Applies edge bundling on the given fibers.
     *
     * \param progress This will indicate bundling progress.
     * \param shutdown While computing bundling, it should abort also in case of shutdown.
     * \param fibers The fibers which should be bundled.
     * \param mask Optional mask defining allowed space within segments may be shifted.
     *
     * \return Bundled fibers
     */
    virtual WDataSetFibers::SPtr operator()( WProgress::SPtr progress, WBoolFlag const &shutdown, WDataSetFibers::SPtr fibers,
                                             WDataSetScalar::SPtr mask ) = 0;

    /**
     * Destructor.
     */
    virtual ~WEdgeBundlingInterface();
protected:
private:
};

#endif  // WEDGEBUNDLINGINTERFACE_H
