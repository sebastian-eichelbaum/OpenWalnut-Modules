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

#ifndef WCHECKCUDAERROR_H
#define WCHECKCUDAERROR_H

#include <cuda.h>
#include <cuda_runtime_api.h>

/**
 * Evaluates a \c cudaError_t variable to indicate if an error with CUDA involved happend or not. If
 * there is an error success will be set to false and returned. In case of an error an error message
 * is written to the \ref WLogger.
 *
 * \param success Input variable from CUDA
 * \param err The variable to check
 * \param msg An error message given from the CUDA system.
 *
 * \return The value of \ref success incase there was no CUDA error, false otherwise.
 */
bool checkCudaError( bool *success, cudaError_t err, const char *msg = NULL );

#endif  // WCHECKCUDAERROR_H
