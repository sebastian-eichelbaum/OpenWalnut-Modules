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


#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include "WPointSaver.h"
WPointSaver::WPointSaver()
{
    m_hasGroupInfo = false;
    m_filePath = new stringstream();
    m_containsData = false;
    m_currentInputNumberLine.reserve( 0 );
    m_currentInputNumberLine.resize( 0 );
}

WPointSaver::~WPointSaver()
{
}

WDataSetPointsGrouped::VertexArray WPointSaver::getVertices()
{
    return m_verts;
}

WDataSetPointsGrouped::VertexArray WPointSaver::getColors()
{
    return m_colors;
}

WDataSetPointsGrouped::GroupArray WPointSaver::getGroups()
{
    return m_groups;
}

bool WPointSaver::loadWDataSetPoints()
{
    m_hasGroupInfo = false;
    return load();
}

bool WPointSaver::loadWDataSetPointsGrouped()
{
    m_hasGroupInfo = true;
    return load();
}

void WPointSaver::saveWDataSetPoints( WDataSetPointsGrouped::VertexArray vertices, WDataSetPointsGrouped::ColorArray colors )
{
    m_verts = vertices;
    m_colors = colors;
    m_hasGroupInfo = false;
    m_containsData = true;

    save();
}

void WPointSaver::saveWDataSetPointsGrouped( WDataSetPointsGrouped::VertexArray vertices,
        WDataSetPointsGrouped::ColorArray colors,  WDataSetPointsGrouped::GroupArray groups )
{
    m_verts = vertices;
    m_colors = colors;
    m_groups = groups;
    m_hasGroupInfo = true;
    m_containsData = true;

    save();
}

void WPointSaver::setFilePath( const char* path )
{
    delete m_filePath;
    m_filePath = new stringstream();
    *m_filePath << path;
}

bool WPointSaver::containsData()
{
    return m_containsData;
}





bool WPointSaver::correctFilePathByExtension()
{
    if( m_filePath->str().length() == 0 )
        return false;

    const char* extension = m_hasGroupInfo ?EXTENSION_WDATASETPOINTSGROUPED :EXTENSION_WDATASETPOINTS;
    if( !pathEndsWith( extension, m_filePath ) )
        *m_filePath << extension;

    return true;
}

bool WPointSaver::pathEndsWith( const char* nameTail, stringstream* filePath )
{
    const string tmp = filePath->str();
    const char* pathChars = tmp.c_str();

    stringstream extensionStream;
    extensionStream << nameTail;
    size_t pathLength = filePath->str().length();
    size_t tailLength = extensionStream.str().length();
    if( pathLength < tailLength )
        return false;
    for(size_t index = 0; index < tailLength; index++)
        if(pathChars[pathLength+index-tailLength] != nameTail[index])
            return false;
    return true;
}

bool WPointSaver::load()
{
    cout << "WPointSaver::load() - Start" << endl;
    WDataSetPointsGrouped::VertexArray outVertices(
            new WDataSetPointsGrouped::VertexArray::element_type() );
    WDataSetPointsGrouped::ColorArray outColors(
            new WDataSetPointsGrouped::ColorArray::element_type() );
    WDataSetPointsGrouped::GroupArray outGroups(
            new WDataSetPointsGrouped::GroupArray::element_type() );
    m_verts = outVertices;
    m_colors = outColors;
    m_groups = outGroups;

    const char* extension = m_hasGroupInfo ?EXTENSION_WDATASETPOINTSGROUPED :EXTENSION_WDATASETPOINTS;
    const string tmp = m_filePath->str();
    const char* path = tmp.c_str();
    const size_t variablesPerLine = m_hasGroupInfo ?7 :6;
    if( !pathEndsWith( extension, m_filePath ) )
        return false;
    cout << "WPointSaver::load() - Extension matches" << endl;

    string line;
    ifstream myfile( path );
    while( getline( myfile, line ) )
    {
        if( ( m_verts->size() / 3 ) % 100000 == 0 )
            cout << "WPointSaver::load() - Current point count: " << m_verts->size() / 3 << endl;
        fetchNumberCharsFromLine( line );
        if( m_currentInputNumberLine.size() == variablesPerLine
                || m_currentInputNumberLine.size() == variablesPerLine - 2 )
        {
            m_containsData = true;
            for( size_t dimension = 0; dimension < 3; dimension++ )
            {
                m_verts->push_back( 0.0 );
                *m_currentInputNumberLine[dimension] >> m_verts->at( m_verts->size() - 1 );

                m_colors->push_back( 0.0 );
            }

            *m_currentInputNumberLine[3] >> m_colors->at( m_colors->size() - 3 );
            for( size_t colorChannel = 1; colorChannel < 3; colorChannel++ )
                if( m_currentInputNumberLine.size() == variablesPerLine )
                {
                    *m_currentInputNumberLine[3 + colorChannel]
                            >> m_colors->at( m_colors->size() - 3 + colorChannel );
                }
                else
                {
                    m_colors->at( m_colors->size() - 3 + colorChannel )
                            = m_colors->at( m_colors->size() - 3 );
                }

            if( m_hasGroupInfo )
            {
                m_groups->push_back( 0.0 );
                *m_currentInputNumberLine[ m_currentInputNumberLine.size() - 1 ]
                        >> m_groups->at( m_groups->size() - 1 );
            }
        }
    }
    myfile.close();

    cout << "Attempting to load file: " << path << endl;
    cout << "Vertices: " << m_verts->size() << endl;
    cout << "Colors: " << m_colors->size() << endl;
    cout << "Groups: " << m_groups->size() << endl;
    return m_verts->size() > 0;
}

void WPointSaver::save()
{
    cout << "Attempting to save pointfile" << endl;
    if( !correctFilePathByExtension() )
        return;

    ofstream stream;
    const string tmp = m_filePath->str();
    const char* path = tmp.c_str();
    remove( path );
    stream.open( path );

    size_t pointCount = m_verts->size() / 3;
    for( size_t index = 0; index < pointCount; index++ )
    {
        //TODO(aschwarzkopf): This saving method is unprecise because saved points have not the full resolution of double values.
        stream.precision( 16 );
        stream << m_verts->at( index * 3 + 0 ) << "\t";
        stream << m_verts->at( index * 3 + 1 ) << "\t";
        stream.precision( 6 );
        stream << m_verts->at( index * 3 + 2 ) << "\t";
        stream << m_colors->at( index * 3 + 0 ) << "\t";
        stream << m_colors->at( index * 3 + 1 ) << "\t";
        stream << m_colors->at( index * 3 + 2 );
        if( m_hasGroupInfo )
            stream << "\t" << m_groups->at( index );
        stream << endl;

        if( index > 0 && index % 100000 == 0 )
            cout << "Saving points. Current index: " << index << " of " << pointCount << endl;
    }

    stream.close();
    cout << "Saved point file: " << m_filePath->str() << endl;
}

bool WPointSaver::isNumberChar( const char sign )
{
    for( char index = '0'; index <= '9'; index++)
        if( index == sign )
            return true;
    return sign == '.' || sign == '-' || sign == 'e' || sign == 'E';
}

void WPointSaver::fetchNumberCharsFromLine( string line )
{
    size_t vectorSize = 0;
    size_t length = line.length();
    const char* text = line.c_str();

    bool wasNumberSign = false;
    for( size_t index = 0; index < length; index++ )
    {
        bool isNumberSign = WPointSaver::isNumberChar( text[index] );
        if( !wasNumberSign && isNumberSign )
        {
            vectorSize++;
            if( vectorSize > m_currentInputNumberLine.size() )
                m_currentInputNumberLine.push_back( new stringstream() );
            m_currentInputNumberLine[vectorSize - 1]->clear();
        }
        if( isNumberSign )
            *m_currentInputNumberLine[vectorSize - 1] << text[index];

        wasNumberSign = isNumberSign;
    }
    if( vectorSize < m_currentInputNumberLine.size() )
    {
        m_currentInputNumberLine.reserve( vectorSize );
        m_currentInputNumberLine.resize( vectorSize );
    }
}

const char* WPointSaver::EXTENSION_WDATASETPOINTS = ".points";

const char* WPointSaver::EXTENSION_WDATASETPOINTSGROUPED = ".groups";
