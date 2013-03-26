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

#include "WWriterFiberClusters.h"

WWriterFiberClusters::WWriterFiberClusters( std::string const& fname, bool overwrite )
    : WWriter( fname, overwrite )
{
}

void WWriterFiberClusters::writeClusters( boost::shared_ptr< WDataSetFiberClustering > const& clusters )
{
    if( boost::filesystem::exists( boost::filesystem::path( m_fname ) ) && !m_overwrite )
    {
        wlog::error( "WWriterFiberClusters" ) << "File already exists, overwriting is not enabled.";
        return;
    }

    std::ofstream out( m_fname.c_str() );
    if( !out.good() )
    {
        wlog::error( "WWriterFiberClusters" ) << "Could not open file for writing!";
        out.close();
    }

    std::size_t numClusters = 0;
    for( WDataSetFiberClustering::ClusterMap::iterator it = clusters->begin(); it != clusters->end(); ++it )
    {
        ++numClusters;
    }

    out << numClusters << std::endl;

    for( WDataSetFiberClustering::ClusterMap::iterator it = clusters->begin(); it != clusters->end(); ++it )
    {
        out << it->first << " " << it->second->size();

        WColor col = it->second->getColor();
        for( std::size_t k = 0; k < 4; ++k )
        {
            out << " " << col[ k ];
        }
        out << std::endl;

        WFiberCluster::IndexList const& list = it->second->getIndices();
        for( WFiberCluster::IndexList::const_iterator it2 = list.begin(); it2 != list.end(); ++it2 )
        {
            out << *it2 << " ";
        }
        out << std::endl;
    }

    out.close();
}

