//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#include <fstream>
#include <string>
#include <list>
#include <vector>

#include <core/common/WAssert.h>

#include "WReaderFiberClusters.h"

WReaderFiberClusters::WReaderFiberClusters( std::string const& fname )
    : WReader( fname )
{
}

boost::shared_ptr< WDataSetFiberClustering > WReaderFiberClusters::readClusters()
{
    boost::shared_ptr< WDataSetFiberClustering > clusters( new WDataSetFiberClustering );

    std::ifstream in( m_fname.c_str() );
    WAssert( in.good(), "" );

    std::size_t numClusters = 0;
    in >> numClusters;
    WAssert( in.good(), "" );

    for( std::size_t k = 0; k < numClusters; ++k )
    {
        std::size_t idx = 0;
        in >> idx;
        WAssert( in.good(), "" );

        std::size_t size = 0;
        in >> size;
        WAssert( in.good(), "" );

        WColor col;
        for( std::size_t j = 0; j < 4; ++j )
        {
            in >> col[ j ];
            WAssert( in.good(), "" );
        }

        std::vector< std::size_t > indices( size );

        for( std::size_t j = 0; j < size; ++j )
        {
            in >> indices[ j ];
            WAssert( in.good(), "" );
        }

        std::list< std::size_t > ilist( indices.begin(), indices.end() );
        boost::shared_ptr< WFiberCluster > fc( new WFiberCluster( ilist, col ) );
        clusters->setCluster( idx, fc );
    }

    in.close();

    return clusters;
}

