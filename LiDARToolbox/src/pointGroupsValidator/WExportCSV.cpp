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

#include <iostream>
#include <vector>
#include <string>
#include "WExportCSV.h"

WExportCSV::WExportCSV()
{
    m_cumulateResultGroups = true;
    m_fileOutputStream = new ofstream();
    m_filePath = new stringstream();
}

WExportCSV::~WExportCSV()
{
}


void WExportCSV::setGroupInfo( vector<WGroupInfo*>* groupInfo )
{
    m_groupInfo = groupInfo;
}

void WExportCSV::setExportFilePath( const char* path )
{
    delete m_filePath;
    m_filePath = new stringstream();
    *m_filePath << path;
}

void WExportCSV::setCumulateResultGroups( bool cumulate )
{
    m_cumulateResultGroups = cumulate;
}

bool WExportCSV::exportCSV()
{
    if( !correctFilePathByExtension() )
        return false;

    delete m_fileOutputStream;
    m_fileOutputStream = new ofstream();

    const string tmp = m_filePath->str();
    const char* path = tmp.c_str();
    remove( path );
    m_fileOutputStream->open( path );

    if( m_cumulateResultGroups )
    {
        writeCumulatedGroupInfoToFile();
    }
    else
    {
        writeInfoToFileForAllGroups();
    }

    m_fileOutputStream->close();

    return true;
}




vector<WCumulatedGroupInfo*>* WExportCSV::generateCumulatedGroups()
{
    cout << "WExportCSV::generateCumulatedGroups() - Start" << endl;
    vector<WCumulatedGroupInfo*>* cumulatedGroups = new vector<WCumulatedGroupInfo*>();

    for( size_t index = 0; index < m_groupInfo->size(); index++ )
    {
        WGroupInfo* group = m_groupInfo->at( index );
        while( cumulatedGroups->size() == 0 ||
                cumulatedGroups->at( cumulatedGroups->size() - 1 )->getRangeMaxPointCount()
                < group->getReferenceGroupPointCount() )
        {
            size_t currentSize = cumulatedGroups->size();
            size_t exponent = 2;
            size_t rangeMinPointCount = pow( exponent, currentSize );
            size_t rangeMaxPointCount = pow( exponent, currentSize + 1 ) - 1;
            WCumulatedGroupInfo* cumulatedGroupInfo = new WCumulatedGroupInfo();
            cumulatedGroupInfo->setPointCountRange( rangeMinPointCount, rangeMaxPointCount );
            cumulatedGroups->push_back( cumulatedGroupInfo );
        }

        for( size_t cumulated = 0; cumulated < cumulatedGroups->size(); cumulated++ )
            if( cumulatedGroups->at( cumulated )->canBeCumulated( group ) )
                cumulatedGroups->at( cumulated )->cumulateGroup( group );
    }

    return cumulatedGroups;
}


void WExportCSV::writeCumulatedGroupInfoToFile()
{
    cout << "WExportCSV::writeCumulatedGroupInfoToFile() - Start" << endl;
    *m_fileOutputStream <<
            "Reference groups" << TAB <<
            "Point count from" << TAB <<
            //"Range to" << TAB <<

            "Certainly detected %" << TAB <<
            "Certain groups by completeness %" << TAB <<
            "Certain groups by area completeness %" << TAB <<
            "Certain groups by correctness %" << TAB <<

            "Point completeness %" << TAB <<
            "Area point completeness %" << TAB <<
            "Point correctness %" << endl;
    vector<WCumulatedGroupInfo*>* cumulatedGroups = generateCumulatedGroups();

    cout << "WExportCSV::writeCumulatedGroupInfoToFile() - Cumulated groups generated" << endl;

    for( size_t index = 0; index < cumulatedGroups->size(); index++ )
    {
        WCumulatedGroupInfo* group = cumulatedGroups->at( index );
        if( group->getGroupCount() > 0 )
        {
            *m_fileOutputStream <<
                    group->getGroupCount() << TAB <<
                    group->getRangeMinPointCount() << TAB <<
                    //group->getRangeMaxPointCount() * 100.0 << TAB <<

                    group->getCertainlyDetectedGroupsAmount() * 100.0 << TAB <<
                    group->getCertainGroupsAmountByCompleteness() * 100.0 << TAB <<
                    group->getCertainGroupsAmountByAreaCompleteness() * 100.0 << TAB <<
                    group->getCertainGroupsAmountByCorrectness() * 100.0 << TAB <<

                    group->getPointCompleteness() * 100.0 << TAB <<
                    group->getAreaPointCorrectness() * 100.0 << TAB <<
                    group->getPointCorrectness() * 100.0 << endl;
        }
    }
    delete cumulatedGroups;
}

void WExportCSV::writeInfoToFileForAllGroups()
{
    *m_fileOutputStream <<
            "Reference group" << TAB <<
            "Validated group" << TAB <<
            "Ref. group points" << TAB <<
            "Val. group points" << TAB <<

            "Points correctly detected" << TAB <<
            "Points of missing areas" << TAB <<
            "In false group" << TAB <<

            "Completeness %" << TAB <<
            "Areas completeness %" << TAB <<
            "Correctness %" << endl;

    for( size_t index = 0; index < m_groupInfo->size(); index++ )
    {
        WGroupInfo* group = m_groupInfo->at( index );
        if( group->getReferenceGroupPointCount() > 0 )
        {
            m_fileOutputStream->precision( 16 );
            *m_fileOutputStream <<
                    group->getReferenceGroupID() << TAB <<
                    group->getValidatedGroupID() << TAB <<
                    group->getReferenceGroupPointCount() << TAB <<
                    ( group->getCorrectlyDetectedPointCount() + group->getUncorrectlyDetectedPointCount() ) << TAB <<

                    group->getCorrectlyDetectedPointCount() << TAB <<
                    group->getAreaMissingPointCount() << TAB <<
                    group->getUncorrectlyDetectedPointCount() << TAB <<

                    group->getCompletess() * 100.0 << TAB <<
                    group->getAreaCompleteness() * 100.0 << TAB <<
                    group->getCorrectness() * 100.0 << endl;
        }
    }
}


bool WExportCSV::correctFilePathByExtension()
{
    if( m_filePath->str().length() == 0 )
        return false;

    if( !WPointSaver::pathEndsWith( CSV_TABLE_EXTENSION, m_filePath ) )
        *m_filePath << CSV_TABLE_EXTENSION;

    return true;
}

const char* WExportCSV::CSV_TABLE_EXTENSION = ".csv";

const char* WExportCSV::TAB = "\t";
