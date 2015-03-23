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

#ifndef WEXPORTCSV_H
#define WEXPORTCSV_H

#include <math.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "structure/WCumulatedGroupInfo.h"
#include "structure/WGroupInfo.h"
#include "../common/algorithms/pointSaver/WPointSaver.h"

using std::vector;
using std::stringstream;

/**
 * Exports point segmentation evaluation results to a comma separated value table file.
 */
class WExportCSV
{
public:
    /**
     * Creates a segmentation result export instance.
     */
    explicit WExportCSV();

    /**
     * Destroys a segmentation result export instance.
     */
    virtual ~WExportCSV();

    /**
     * Assigns the validated groups to be exported.
     * \param groupInfo Group information to be exported.
     */
    void setGroupInfo( vector<WGroupInfo*>* groupInfo );

    /**
     * Sets the output path of the exported comma separated values table file.
     * \param path Output path of the exported comma separated values table file.
     */
    void setExportFilePath( const char* path );

    /**
     * Sets whether group evaluation results should be cumulated or not.
     * \param cumulate Cumulate or not.
     */
    void setCumulateResultGroups( bool cumulate );

    /**
     * Export group evaluation results to a comma separated values table file.
     * \return Saved successfully or not.
     */
    bool exportCSV();

private:
    /**
     * Cumulates information of evaluated groups.
     * \return Cumulated group evaluation information..
     */
    vector<WCumulatedGroupInfo*>* generateCumulatedGroups();

    /**
     * Writes cumulated evaluated group information to the table file.
     */
    void writeCumulatedGroupInfoToFile();

    /**
     * Writes evaluated group information to the table file.
     */
    void writeInfoToFileForAllGroups();

    /**
     * Puts the extension to the output file if not existing.
     * \return Returns "false" if file path is empty.
     */
    bool correctFilePathByExtension();

    /**
     * Comma separated values table output file path.
     */
    stringstream* m_filePath;

    /**
     * Evaluated groups information.
     */
    vector<WGroupInfo*>* m_groupInfo;

    /**
     * Cumulate evaluated groups or not.
     */
    bool m_cumulateResultGroups;

    /**
     * File output stream
     */
    ofstream* m_fileOutputStream;

    /**
     * Output file extension.
     */
    static const char* CSV_TABLE_EXTENSION;

    /**
     * Separation sign between table cells.
     */
    static const char* TAB;
};

#endif  // WEXPORTCSV_H
