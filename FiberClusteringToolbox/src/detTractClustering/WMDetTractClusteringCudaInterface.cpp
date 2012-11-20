//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS, Copyright 2010 RRZK University of Cologne
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

#include "core/common/datastructures/WFiber.h"
#include "core/common/math/WMatrixSym.h"
#include "core/common/WProgressCombiner.h"
#include "core/dataHandler/WDataSetFiberVector.h"
#include "WMDetTractClusteringCudaInterface.h"
#include "WMDetTractClusteringCudaKernel.h"
#include "WProgressWrapper.h"
#include "WProgressWrapperData.h"

bool initDLtTableCuda( boost::shared_ptr< WMatrixSym > dLtTable,
                       const boost::shared_ptr< WDataSetFiberVector > tracts,
                       double proximity_threshold,
                       boost::shared_ptr< WProgressCombiner > progressCombiner )
{
    // Since fibers have differing point counts, compute offset into fiber array
    const size_t ntracts = tracts->size();
    int* offsets = new int[ntracts];
    int* lengths = new int[ntracts];
    unsigned int nextoffset = 0;
    const unsigned align = 16 * 4 * sizeof( float );
    size_t maxlength = 0;
    for( size_t i = 0; i < ntracts; ++i )
    {
        const WFiber &fib = ( *tracts )[i];
        offsets[i] = nextoffset;
        lengths[i] = fib.size();
        nextoffset = ( nextoffset + fib.size() + ( align - 1 ) ) & ~( align - 1 );

        if( fib.size() > maxlength )
        {
            maxlength = fib.size();
        }
    }

    // copy fiber coordinates into plain array
    const unsigned int ncoords = nextoffset;
    float* coords = new float[ncoords*3];
    for( size_t i = 0; i < ntracts; ++i )
    {
        const WFiber &fib = ( *tracts )[i];
        int k = offsets[i]*3;
        for( int j = 0; j < lengths[i]; ++j )
        {
            coords[k++] = fib[j][0];
            coords[k++] = fib[j][1];
            coords[k++] = fib[j][2];
        }
    }

    // wrap WProgress in order to avoid boost in .cu files
    ProgressWrapperData progressData;
    progressData.progressCombiner = progressCombiner;
    ProgressWrapper *progress = new ProgressWrapper( &progressData );

    float* distmat = new float[ntracts * ntracts];
    // TODO(math): This is disabled as CUDA code won't compile anymore with the new CMakeProjects
    bool result; // = distCuda( distmat, ncoords, coords, ntracts, offsets, lengths, proximity_threshold, progress );

    // copy results to OpenWalnut structure
    for( size_t q = 0; q < ntracts; ++q )
    {
        for( size_t r = q + 1; r < ntracts; ++r )
        {
            ( *dLtTable )( q, r ) = fmaxf( distmat[q * ntracts + r], distmat[r * ntracts + q] );
        }
    }

    delete progress;

    delete[] coords;
    delete[] offsets;
    delete[] lengths;
    delete[] distmat;

    return result;
}
