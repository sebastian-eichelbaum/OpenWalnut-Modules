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

#ifndef WCREATESURFACEJOB_H
#define WCREATESURFACEJOB_H

#include <vector>

#include "core/common/WProgress.h"
#include "core/common/WSharedObject.h"
#include "core/common/WThreadedJobs.h"

#include "core/dataHandler/WDataSetScalar.h"

#include "core/common/algorithms/WMarchingCubesAlgorithm.h"

#include "core/graphicsEngine/WTriangleMesh.h"

/**
 * Manager for parallel jobs that create parts of the surfaces enclosing the atlas regions.
 * Each job performs a Marching Cubes part.
 */
template< typename T >
class WCreateSurfaceJob : public WThreadedJobs< WDataSetScalar, size_t >
{
public:
    /**
     * constructor
     * \param input the dataset to work on
     * \param regionMeshes pointer to vector of triangle meshes, to store the result in
     * \param progressCombiner pointer for progress
     * \param pro pointer for progress
     */
    WCreateSurfaceJob( boost::shared_ptr< WDataSetScalar const > input,
                       boost::shared_ptr< std::vector< boost::shared_ptr< WTriangleMesh > > > regionMeshes,
                       boost::shared_ptr<WProgressCombiner> progressCombiner,
                       boost::shared_ptr<WProgress>pro );

    /**
     * destructor
     */
    ~WCreateSurfaceJob();

    /**
     * Abstract function for the job aquisition.
     *
     * \param job The job (output).
     * \return false, iff no more jobs need to be processed.
     */
    virtual bool getJob( size_t& job ); // NOLINT

    /**
     * Abstract function that performs the actual computation per job.
     *
     * \param input The input data.
     * \param job The current job.
     */
    virtual void compute( boost::shared_ptr< WDataSetScalar const > input, size_t const& job );

protected:
private:
    /**
     * cuts an area from a value set with a given index and creates an isosurface
     * \param vals
     * \param tempData
     * \param number
     */
    void cutArea( boost::shared_ptr< WValueSet< T > > vals, std::vector<float>& tempData, unsigned int number ); // NOLINT

    WSharedObject< size_t> m_counter; //!< job number

    boost::shared_ptr< WGridRegular3D> m_grid; //!< stores pointer to grid

    boost::shared_ptr< std::vector< boost::shared_ptr< WTriangleMesh > > > m_regionMeshes; //!< stores pointer

    boost::shared_ptr<WProgressCombiner> m_progressCombiner; //!< stores pointer to combiner for different progress parts

    boost::shared_ptr<WProgress> m_progress; //!< stores pointer to progress indicator
};

template< typename T >
WCreateSurfaceJob< T >::WCreateSurfaceJob( boost::shared_ptr< WDataSetScalar const > input,
                                           boost::shared_ptr< std::vector< boost::shared_ptr< WTriangleMesh > > > regionMeshes,
                                           boost::shared_ptr<WProgressCombiner> progressCombiner,
                                           boost::shared_ptr<WProgress>pro )
    : WThreadedJobs< WDataSetScalar, size_t >( input ),
    m_regionMeshes( regionMeshes ),
    m_progressCombiner( progressCombiner ),
    m_progress( pro )
{
    m_counter.getWriteTicket()->get() = 1;
    m_grid = boost::dynamic_pointer_cast< WGridRegular3D >( input->getGrid() );
    WAssert( m_grid, "" );
}

template< typename T >
WCreateSurfaceJob< T >::~WCreateSurfaceJob()
{
}

template< typename T >
bool WCreateSurfaceJob< T >::getJob( size_t& job ) // NOLINT
{
    WSharedObject< size_t>::WriteTicket ticket = m_counter.getWriteTicket();
    if( ticket->get() <= m_input->getMax() )
    {
        job = ( ticket->get() )++;
        return true;
    }
    return false;
}

template< typename T >
void WCreateSurfaceJob< T >::compute( boost::shared_ptr< WDataSetScalar const > input, size_t const& job )
{
    WAssert( job <= m_input->getMax(), "" );

    boost::shared_ptr< std::vector< float > > tempData =
        boost::shared_ptr< std::vector< float > >( new std::vector< float >( input->getGrid()->size() ) );

    cutArea( boost::dynamic_pointer_cast< WValueSet< T > >( m_input->getValueSet() ), ( *tempData.get() ), job );



    boost::shared_ptr< WValueSet< float > > newValueSet = boost::shared_ptr< WValueSet< float > >(
                                                       new WValueSet< float >( 0, 1, tempData, W_DT_FLOAT ) );
    WMarchingCubesAlgorithm mcAlgo;
    boost::shared_ptr< WTriangleMesh >triMesh = mcAlgo.generateSurface( m_grid->getNbCoordsX(), m_grid->getNbCoordsY(), m_grid->getNbCoordsZ(),
                                            m_grid->getTransformationMatrix(),
                                            newValueSet->rawDataVectorPointer(),
                                            0.9,
                                            m_progressCombiner );
    if( triMesh->vertSize() != 0 )
    {
        ( *m_regionMeshes )[job] = triMesh;
    }
    else
    {
        ( *m_regionMeshes )[job] = boost::shared_ptr< WTriangleMesh >( new WTriangleMesh( 0, 0 ) );
    }

    ++*m_progress;
}

template< typename T > void WCreateSurfaceJob<T>::cutArea( boost::shared_ptr< WValueSet< T > > vals,
                                                           std::vector<float>& tempData, unsigned int number )
{
    for( size_t k = 0; k < m_grid->size(); ++k )
    {
        if( static_cast< size_t >( vals->getScalar( k ) ) == number )
        {
            tempData[ k ] = 1.0;
        }
        else
        {
            tempData[ k ] = 0.0f;
        }
    }
}

#endif  // WCREATESURFACEJOB_H
