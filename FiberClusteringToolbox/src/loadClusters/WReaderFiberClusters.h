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

#ifndef WREADERFIBERCLUSTERS_H
#define WREADERFIBERCLUSTERS_H

#include <string>

#include <core/dataHandler/io/WReader.h>
#include <core/dataHandler/WDataSetFiberClustering.h>

/**
 * Construct a WDataSetFiberClustering from a file.
 */
class WReaderFiberClusters : public WReader
{
public:
    /**
     * Constructor.
     *
     * \param fname The name of the file to read from.
     */
    explicit WReaderFiberClusters( std::string const& fname );

    /**
     * Reads the clusters from the file.
     *
     * \return The loaded dataset.
     */
    boost::shared_ptr< WDataSetFiberClustering > readClusters();
};

#endif  // WREADERFIBERCLUSTERS_H
