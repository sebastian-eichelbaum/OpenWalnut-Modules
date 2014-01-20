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

#ifndef WSTARTENDCOLORING_H
#define WSTARTENDCOLORING_H

#include <utility>
#include <vector>

#include <core/common/WObjectNDIP.h>
#include <core/common/WProperties.h>

#include "WColoring_I.h"

/**
 * Use a color scale to indicate start, end and orientation of the fiber segmentwisely.
 *
 * \ingroup modules
 */
class WStartEndColoring : public WObjectNDIP< WColoring_I >
{
public:
    /**
     * Constructor.
     */
    WStartEndColoring();

    /**
     * Destructor.
     */
    virtual ~WStartEndColoring();

    /**
     * Apply Start End coloring scheme on dataset.
     *
     * \copydetails WColoring_I::operator()()
     */
    virtual WDataSetFibers::SPtr operator()( WProgress::SPtr progress, WBoolFlag const &shutdown, WDataSetFibers::SPtr fibers );

protected:
    /**
     * Color codes the fiber by current color scale.
     *
     * \param param When near zero then start color is selected, when near one, end color.
     *
     * \return Color of the current color scale according to the parametrization given by param.
     */
    osg::Vec4 segmentColor( double param ) const;

private:
    /**
     * Array containing the colors per vertex.
     */
    WDataSetFibers::ColorArray m_StartEndColors;
};

#endif  // WSTARTENDCOLORING_H
