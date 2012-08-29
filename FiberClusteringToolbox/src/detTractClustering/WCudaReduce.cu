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


// NOTE: This is based on code from projects/reduction/reduction_kernel.cu from CUDA SDK 2.0

#ifndef WCUDAREDUCE_CU
#define WCUDAREDUCE_CU

/**
 * Performs parallel reduction of elements in \e shared memory, the reduction operation is defined by \c Op::eval.
 */
template< class Op >
class Reduce
{
public:
    /**
     * Result of the reduction is stored in s[0]
     */
    static inline __device__ void apply( volatile float *s, unsigned int n )
    {
        const unsigned int i = threadIdx.x;

        const int nthreads = min( 512, 1<<(31-__clz(min(blockDim.x, n))) );
        if( n > nthreads )
        {
            if( i < nthreads )
            {
                for( unsigned int j = nthreads+i; j<n; j += nthreads )
                {
                    s[i] = Op::eval( s[i], s[j] );
                }
            }
            __syncthreads();
        }

        if( nthreads == 512 )
            do_apply< 512 >( s );
        else if( nthreads == 256 )
            do_apply< 256 >( s );
        else if( nthreads == 128 )
            do_apply< 128 >( s );
        else if( nthreads == 64 )
            do_apply< 64 >( s );
        else if( nthreads == 32 )
            do_apply< 32 >( s );
        else if( nthreads == 16 )
            do_apply< 16 >( s );
        else if( nthreads == 8 )
            do_apply< 8 >( s );
        else if( nthreads == 4 )
            do_apply< 4 >( s );
        else if( nthreads == 2 )
            do_apply< 2 >( s );
    }

private:
   // This routine actually performs the reduction in shared memory
   // Result of the reduction is stored in s[0]
   template< unsigned int threads >
   static inline __device__ void do_apply( volatile float *s )
   {
      const unsigned int i = threadIdx.x;

      if( threads > 512 ) { if( i + 512 < threads ) s[i] = Op::eval( s[i], s[i + 512] ); __syncthreads(); }
      if( threads > 256 ) { if( i + 256 < threads ) s[i] = Op::eval( s[i], s[i + 256] ); __syncthreads(); }
      if( threads > 128 ) { if( i + 128 < threads ) s[i] = Op::eval( s[i], s[i + 128] ); __syncthreads(); }
      if( threads >  64 ) { if( i +  64 < threads ) s[i] = Op::eval( s[i], s[i +  64] ); __syncthreads(); }

      if( i < 32 )
      {
         // all this happens in one warp: no synchronization needed as s is declared volatile
         if( threads > 32 ) if( i + 32 < threads ) s[i] = Op::eval( s[i], s[i + 32] );
         if( threads > 16 ) if( i + 16 < threads ) s[i] = Op::eval( s[i], s[i + 16] );
         if( threads >  8 ) if( i +  8 < threads ) s[i] = Op::eval( s[i], s[i +  8] );
         if( threads >  4 ) if( i +  4 < threads ) s[i] = Op::eval( s[i], s[i +  4] );
         if( threads >  2 ) if( i +  2 < threads ) s[i] = Op::eval( s[i], s[i +  2] );
         if( threads >  1 ) if( i +  1 < threads ) s[i] = Op::eval( s[i], s[i +  1] );
      }
   }
};

#endif  // WCUDAREDUCE_CU
