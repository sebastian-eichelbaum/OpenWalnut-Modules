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

#ifndef WWRITERFIBERCLUSTERS_H
#define WWRITERFIBERCLUSTERS_H

#include <string>

#include <core/dataHandler/io/WWriter.h>

#include <core/dataHandler/WDataSetFiberClustering.h>

/**
 * A writer for fiber clusterings.
 */
class WWriterFiberClusters : public WWriter
{
public:
    /**
     * Constructor.
     *
     * \param fname The filename to write to.
     * \param overwrite Whether to overwrite the file if it exists.
     */
    WWriterFiberClusters( std::string const& fname, bool overwrite = false );

    /**
     * Write the data to the file.
     *
     * \param clusters The cluster data to write.
     */
    void writeClusters( boost::shared_ptr< WDataSetFiberClustering > const& clusters );
};

#endif  // WWRITERFIBERCLUSTERS_H
