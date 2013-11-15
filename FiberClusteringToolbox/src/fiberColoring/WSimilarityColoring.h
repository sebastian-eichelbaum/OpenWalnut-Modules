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

#ifndef WSIMILARITYCOLORING_H
#define WSIMILARITYCOLORING_H

#include <core/common/WObjectNDIP.h>
#include <core/common/WProperties.h>

#include "WColoring_I.h"

class WSimilarityColoring : public WObjectNDIP< WColoring_I >
{
public:
    /**
     * Constructor.
     */
    WSimilarityColoring();

    /**
     * Destructor.
     */
    virtual ~WSimilarityColoring();

    /**
     * Implements edge bundling on CPU.
     *
     * \copydetails WEdgeBundlingInterface::operator()()
     */
    virtual WDataSetFibers::SPtr operator()( WProgress::SPtr progress, WBoolFlag const &shutdown, WDataSetFibers::SPtr fibers );
protected:
private:
    void loadPositions( void );
    /**
     * The filename property -> where to write the nifty file
     */
    WPropFilename m_filename;

    WPropTrigger  m_loadTrigger; //!< This property triggers the actual reading

    WPropInt m_bb;

    WPropDouble m_r1;
    WPropDouble m_r2;
    WPropDouble m_L0;
    WPropDouble m_a0;
    WPropDouble m_b0;

    WDataSetFibers::ColorArray m_similarityColors;

    /**
     * Maximum number of iterations for computing the force directed layout. 0 means disabled and another criteria must be implemented to abort computation.
     */
    WPropInt m_maxIter;

    std::vector< std::pair< double, double > > m_2dpos;
};

#endif  // WSIMILARITYCOLORING_H
