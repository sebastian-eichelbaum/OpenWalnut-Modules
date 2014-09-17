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

#ifndef WPOINTSAVER_H
#define WPOINTSAVER_H

#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "../../datastructures/WDataSetPointsGrouped.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;

/**
 * Class that saves and loads point data using files. It supports WDataSetPoints and 
 * WDataSetPointsGrouped.
 * 
 * Workflow for saving:
 *   - setFilePath()
 *   - setWDataSetPoints() or setWDataSetPointsGrouped()
 * 
 * Workflow for loading:
 *   - setFilePath()
 *   - loadWDataSetPoints() or loadWDataSetPointsGrouped()
 *   - Check whether containsData()
 *   - getVertices(), getColors() and getGroups()
 * //TODO(aschwarzkopf): Still very very slow using very big datasets.
 */
class WPointSaver
{
public:
    /**
     * Creates the point file processing instance.
     */
    WPointSaver();
    /**
     * Destroys the point file processing instance.
     */
    virtual ~WPointSaver();

    /**
     * Tells whether a loaded point file has data or not.
     * \return Point data has points or not.
     */
    bool containsData();

    /**
     * Returns vertices of loaded data.
     * \return Loaded vertex data.
     */
    WDataSetPointsGrouped::VertexArray getVertices();
    /**
     * Returns colors of loaded data.
     * \return Loaded color data.
     */
    WDataSetPointsGrouped::ColorArray getColors();
    /**
     * Returns group IDs of loaded data. Use it only after loadWDataSetPointsGrouped().
     * \return Loaded group ID data.
     */
    WDataSetPointsGrouped::GroupArray getGroups();
    /**
     * Loads point data with color attributes from a file.
     * \return File was successfully loaded or not.
     */
    bool loadWDataSetPoints();
    /**
     * Loads point data with color attributes and group IDs from a file.
     * \return File was successfully loaded or not.
     */
    bool loadWDataSetPointsGrouped();
    /**
     * Saves a point data set with colors to a file.
     * \param vertices Vertex array to save.
     * \param colors Color array to save.
     */
    void saveWDataSetPoints( WDataSetPointsGrouped::VertexArray vertices, WDataSetPointsGrouped::ColorArray colors );
    /**
     * Saves grouped point data set with colors and group IDs to a file.
     * \param vertices Vertex array to save.
     * \param colors Color array to save.
     * \param groups Group ID array to save.
     */
    void saveWDataSetPointsGrouped( WDataSetPointsGrouped::VertexArray vertices,
            WDataSetPointsGrouped::ColorArray colors,  WDataSetPointsGrouped::GroupArray groups );
    /**
     * Sets a file path for loading and saving point datasets.
     * \param path File of a source or target point data file.
     */
    void setFilePath( const char* path );

    /**
     * Character belongs to a number or not.
     * \param sign Character to be tested.
     * \return Sign belongs to a number or not.
     */
    static bool isNumberChar( const char sign );
    /**
     * Parses a double number from a characer sequence.
     * \param charVector Character sequence to be parsed.
     * \return parsed double number.
     */
    static double parseDouble( vector<char> charVector );
    /**
     * Parses a size_t number from a characer sequence.
     * \param charVector Character sequence to be parsed.
     * \return parsed size_t number.
     */
    static size_t parseSizeT( vector<char> charVector );
    /**
     * Tests whether a path ends with a character sequence. Useful to test whether file 
     * names correspond to an extension.
     * \param nameTail Character sequence to be tested.
     * \param filePath File path to be tested.
     * \return File path ends with the character sequence or not.
     */
    static bool pathEndsWith( const char* nameTail, stringstream* filePath );

private:
    /**
     * Corrects a file path if it does not ents with an extension.
     * \return Tells about the success. Empty names are signaled not to be further 
     *         processed by returning "false".
     */
    bool correctFilePathByExtension();
    /**
     * Loads point dataset from a file.
     * \return File was successfully loaded or not.
     */
    bool load();
    /**
     * Writes point data to a file.
     */
    void save();
    /**
     * Separates a single string representing numbers to several character sets. Their 
     * count corresponds to the count of numbers. 
     * \param line Line to be separated
     * \return Set of several numbers that further have to be parsed.
     */
    vector<vector<char> > fetchNumberCharsFromLine( string line );


    /**
     * Vwertices that are loaded or saved.
     */
    WDataSetPointsGrouped::VertexArray m_verts;
    /**
     * Colors that are loaded or saved.
     */
    WDataSetPointsGrouped::ColorArray m_colors;
    /**
     * Point group IDs that are loaded or saved.
     */
    WDataSetPointsGrouped::GroupArray m_groups;

    /**
     * Indicator whether points have a group ID or not.
     */
    bool m_hasGroupInfo;
    /**
     * File processing instance contains point data or not.
     */
    bool m_containsData;
    /**
     * Source or target point data file path.
     */
    stringstream* m_filePath;

    /**
     * Extension for WDataSetPoints point data files.
     */
    static const char* EXTENSION_WDATASETPOINTS;
    /**
     * Extension for WDataSetPointsGrouped point data files.
     */
    static const char* EXTENSION_WDATASETPOINTSGROUPED;
};

#endif  // WPOINTSAVER_H
