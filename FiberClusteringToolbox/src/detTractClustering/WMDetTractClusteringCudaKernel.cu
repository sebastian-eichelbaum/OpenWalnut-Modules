//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2010 RRZK, University of Cologne
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

#include <deque>

#include <cuda.h>

#include "WCheckCudaError.h"
#include "WCudaReduce.cu"
#include "WMDetTractClusteringCudaKernel.h"
#include "WProgressWrapper.h"

const int nthreads = 192;       // number of cuda threads
const int gridsize = 32;        // don't schedule too large tasks - watchdog timer will kill long-running ones
const int maxkernels = 4;       // maximum number of concurrent kernels,
                                // avoids possible problems because of too many pending kernels
const int allowedlength = 1024; // maximum allowable length of a fiber in __shared__ memory
const int warpsize = 32;        // you should not need to change this

const float LARGE_VALUE = 3.402823466e+38f; // FLT_MAX

/* reduction operation: sum */
struct ReduceOpSum
{
   static inline __device__ float eval(const float x, const float y)
   {
      return x + y;
   }
};
static inline __device__ void reducesum(volatile float *s, unsigned int n)
{
   Reduce<ReduceOpSum>::apply(s, n);
}


/* compute asymmetric metric from one fiber to another,
 * threshold2 is the square of the proximity threshold */
static __device__ float distasym( const int qsize, const float3 *q,
        const int rsize, const float3 *r,
        const float threshold2 )
{
    if( rsize <= 0 || qsize <= 0 )
        return 0.f;

    extern float __shared__ minaccum[];
    minaccum[threadIdx.x] = 0.;

    // for every point in the first fiber q...
    for( unsigned int i = threadIdx.x; i < qsize; i += blockDim.x )
    {
        // ...find the squared distance to closest point on fiber q
        float mind2 = LARGE_VALUE;
        for( unsigned int j = 0; j < rsize; ++j )
        {
            const float dx = q[i].x - r[j].x;
            const float dy = q[i].y - r[j].y;
            const float dz = q[i].z - r[j].z;

            const float d2 = dx*dx + dy*dy + dz*dz;
            mind2 = fminf( mind2, d2 );
        }

        if( mind2 > threshold2 )
            minaccum[threadIdx.x] += sqrtf( mind2 );
    }

    // add closest distances for all points on r
    __syncthreads();
    reducesum( minaccum, min(qsize, blockDim.x) );

    return minaccum[0] / qsize;
}

/**
 * copy a fiber from global to __shared__ memory
 */
static __device__ void loadFiber( float3 *x, const float *coords, const int start, const int length )
{
    for( unsigned int i = threadIdx.x; i < length; i += blockDim.x )
    {
        x[i].x = coords[(start+i)*3];
        x[i].y = coords[(start+i)*3 + 1];
        x[i].z = coords[(start+i)*3 + 2];
    }
}

/**
 * compute asymmetric Zhang metric for fibers with numbers corresponding to thread block grid coordinate 
 *
 * nshmfibs:     number of fibers to copy to __shared__ memory (have to be short enough)
 * maxlength:    maximum length of a fiber to be loaded into __shared__ memory
 * distmat:      output distance matrix (ntracts x ntracts)
 * tilex, tiley: ntracts x ntracts fiber array has been split into tiles,
 *               starting coordinates of fiber tile to be processed by this kernel call
 * coords:       interleaved (x[0], y[0], z[0], x[0], ...) coordinate array
 * ntracts:      total no. of fibers
 * offsets:      start indices of fibers
 * lengths:      fiber lengths
 * threshold2:   ignore points closer than the square root of this distance
 */
template< int nshmfibs, int maxlength >
static __global__ void distKernel( float *distmat,
        const size_t tilex, const size_t tiley,
        const float *coords,
        const int ntracts, const int *offsets, const int *lengths,
        const float threshold2 )
{
    float3 __shared__ q[nshmfibs > 1 ? maxlength : 0];
    float3 __shared__ r[nshmfibs > 0 ? maxlength : 0]; // store fiber coordinates

    // compute fiber index for this thread block
    const int qidx = tilex + blockIdx.x;
    const int ridx = tiley + blockIdx.y;
    const int qsize = lengths[qidx];
    const int rsize = lengths[ridx];

    // copy fibers to shared memory
    if( nshmfibs > 1 )
        loadFiber( q, coords, offsets[qidx], qsize );
    if( nshmfibs > 0 )
    {
        loadFiber( r, coords, offsets[ridx], rsize );
        __syncthreads();
    }

    // compute both asymmetric distances
    const float distqr =
        nshmfibs > 1
        ? distasym( qsize, q, rsize, r, threshold2 )
        : nshmfibs > 0
          ? distasym( qsize, (float3 *)&coords[offsets[qidx]*3], rsize, r, threshold2 )
          : distasym( qsize, (float3 *)&coords[offsets[qidx]*3],
                  rsize, (float3 *)&coords[offsets[ridx]*3], threshold2 );

    if( threadIdx.x == 0 )
        distmat[qidx*ntracts + ridx] = distqr;
}

bool distCuda(float *h_distmat,
        const int ncoords, const float *h_coords,
        const int ntracts, const int *h_offsets, const int *h_lengths,
        const float threshold,
        ProgressWrapper *progress )
{
    // allocate memory on device
    float *d_coords = NULL;
    int *d_lengths = NULL;
    int *d_offsets = NULL;
    float *d_distmat = NULL;

    cudaMalloc( &d_coords, sizeof( float ) * ncoords * 3 );
    cudaMalloc( &d_lengths, sizeof( int ) * ntracts );
    cudaMalloc( &d_offsets, sizeof( int ) * ntracts );
    cudaMalloc( &d_distmat, sizeof( float ) * ntracts * ntracts );
    cudaMemset( d_distmat, 0, sizeof( float ) * ntracts * ntracts );

    // copy data to device
    cudaMemcpy( d_coords, h_coords, sizeof( float ) * ncoords * 3, cudaMemcpyHostToDevice );
    cudaMemcpy( d_lengths, h_lengths, sizeof( int ) * ntracts, cudaMemcpyHostToDevice );
    cudaMemcpy( d_offsets, h_offsets, sizeof( int ) * ntracts, cudaMemcpyHostToDevice );

    // number of kernel calls (corresponding to "tiles" from tract pair matrix),
    const int numBlocks = (ntracts+gridsize-1)/gridsize * (ntracts+gridsize-1)/gridsize;
    progress->start( "Tract distances", numBlocks );
    std::deque<cudaEvent_t> kernels; // store scheduled, but not yet terminated kernels

    bool success = true;
    for( int i = 0; i < ntracts; i += gridsize )
    {
        for( int j = 0; j < ntracts; j += gridsize )
        {
            dim3 grid(std::min(gridsize, ntracts-i), std::min(gridsize, ntracts-j));

            cudaEvent_t kernel;
            kernels.push_back( kernel );
            cudaEventCreate( &kernels.back() );
            cudaEventRecord( kernels.back(), 0 );

#define DISTKERNEL( nshmfibs, maxlength ) \
            distKernel< nshmfibs, maxlength > \
            <<< grid, min(nthreads,maxlength), sizeof(float)*min(nthreads,maxlength) >>> \
                ( d_distmat, \
                  i, j, \
                  d_coords, \
                  ntracts, d_offsets, d_lengths, \
                  threshold*threshold )

            if( max( h_lengths[i], h_lengths[j] ) <= warpsize )
            {
                DISTKERNEL( 2, warpsize );
            }
            else if( h_lengths[j] <= allowedlength )
            {
                if( h_lengths[j] <= 32 )
                    DISTKERNEL( 1, 32 );
                else if( h_lengths[j] <= 64 )
                    DISTKERNEL( 1, 64 );
                else if( h_lengths[j] <= 128 )
                    DISTKERNEL( 1, 128 );
                else if( h_lengths[j] <= 256 )
                    DISTKERNEL( 1, 256 );
                else if( h_lengths[j] <= 512 )
                    DISTKERNEL( 1, 512 );
                else
                    DISTKERNEL( 1, allowedlength );
            }
            else
            {
                DISTKERNEL( 0, 0 );
            }
            if( !checkCudaError(&success, cudaGetLastError()) )
                break;

            if( kernels.size() > maxkernels )
            {
                cudaEventSynchronize( kernels.front() );
                if( !checkCudaError(&success, cudaGetLastError()) )
                    break;

                kernels.pop_front();
                ++*progress;
            }
        }

        if( !success )
            break;
    }

    // wait for all kernels to finish
    while( success && !kernels.empty() )
    {
        cudaEventSynchronize( kernels.front() );
        if( !checkCudaError(&success, cudaGetLastError()) )
            break;

        kernels.pop_front();
        ++*progress;
    }
    cudaThreadSynchronize();
    checkCudaError(&success, cudaGetLastError());

    // copy result data back to host
    cudaMemcpy( h_distmat, d_distmat, sizeof( float ) * ntracts * ntracts, cudaMemcpyDeviceToHost );

    cudaFree( d_coords );
    cudaFree( d_distmat );
    cudaFree( d_lengths );
    cudaFree( d_offsets );

    progress->finish();

    return success;
}
