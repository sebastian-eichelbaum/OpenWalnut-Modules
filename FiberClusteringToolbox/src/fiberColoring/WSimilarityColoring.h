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

#include <utility>
#include <vector>

#include <core/common/WObjectNDIP.h>
#include <core/common/WProperties.h>

#include "WColoring_I.h"

/**
 * Strategy to color code the fibers on their pariwise similarity.
 * Paper Title: "Similarity Coloring of DTI Fiber Tracts" by Demiralp and Laidlaw 2009.
 *
 * \ingroup modules
 */
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
     * \copydetails WColoring_I::operator()()
     */
    virtual WDataSetFibers::SPtr operator()( WProgress::SPtr progress, WBoolFlag const &shutdown, WDataSetFibers::SPtr fibers );
protected:
private:
    /**
     * Load text file with x and y positions of each fiber. i'th line corresponds to i'th fiber.
     * First the x position and second the y positions. This 2D embedding (from graphviz) is used to
     * color code the fibers with similarity. See Demiralp Paper on similarity coloring.
     */
    void loadPositions( void );

    /**
     * The filename property -> where to write the nifty file
     */
    WPropFilename m_filename;

    /**
     * This property triggers the actual reading
     */
    WPropTrigger  m_loadTrigger;

    /**
     * Bounding Box value to scale X \e and Y dimensions.
     */
    WPropInt m_bb;

    /**
     * First radius of the torus.
     */
    WPropDouble m_r1;

    /**
     * Second radius of the torus.
     */
    WPropDouble m_r2;

    /**
     * Center in LAB space, L value.
     */
    WPropDouble m_L0;

    /**
     * Center in LAB space, a value.
     */
    WPropDouble m_a0;

    /**
     * Center in LAB space, b value.
     */
    WPropDouble m_b0;

    /**
     * Array containing the colors per vertex.
     */
    WDataSetFibers::ColorArray m_similarityColors;

    /**
     * Maximum number of iterations for computing the force directed layout. 0 means disabled and another criteria must be implemented to abort computation.
     */
    WPropInt m_maxIter;

    /**
     * Fiber positions in 2D embedding, computed by graphviz.
     */
    std::vector< std::pair< double, double > > m_2dpos;
};

#endif  // WSIMILARITYCOLORING_H
