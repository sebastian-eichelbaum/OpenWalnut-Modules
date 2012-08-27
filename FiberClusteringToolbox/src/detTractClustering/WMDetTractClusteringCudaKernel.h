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

#ifndef WMDETTRACTCLUSTERINGCUDAKERNEL_H
#define WMDETTRACTCLUSTERINGCUDAKERNEL_H

class ProgressWrapper;

/**
 * Initialize upper trigonal matrix distmat with Zhang metric. The longest tract has a length of maxlength points.
 *
 * \param distmat Upper trigonal matrix with Zhang metric.
 * \param ncoords Total number of coords (x,y,z interleaved)
 * \param ntracts How many tracts
 * \param sizes Array of number of coords per tract
 * \param offsets Where the coords of the tracts do start
 * \param threshold The 'proximity threshold
 * \param progress Wrapped WProgress indicator for reporting the progress
 *
 * \return True on success, false otherwise (e.g. if maxlength is longer than can be handled by the CUDA kernel)
 */
extern "C" bool distCuda( float *distmat,
                          const int ncoords,
                          const float *coords,
                          const int ntracts,
                          const int *offsets,
                          const int *sizes,
                          const float threshold,
                          ProgressWrapper *progress );

#endif  // WMDETTRACTCLUSTERINGCUDAKERNEL_H
