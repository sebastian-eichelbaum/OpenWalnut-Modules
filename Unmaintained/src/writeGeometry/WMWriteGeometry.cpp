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

#include <string>
#include <sstream>
#include <fstream>

#include "core/common/WPropertyHelper.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/kernel/WKernel.h"
#include "WMWriteGeometry.xpm"

#include "WMWriteGeometry.h"

WMWriteGeometry::WMWriteGeometry():
    WModule()
{
}

WMWriteGeometry::~WMWriteGeometry()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMWriteGeometry::factory() const
{
    // See "src/modules/template/" for an extensively documented example.
    return boost::shared_ptr< WModule >( new WMWriteGeometry() );
}

const char** WMWriteGeometry::getXPMIcon() const
{
    return WMWriteGeometry_xpm;
}
const std::string WMWriteGeometry::getName() const
{
    // Specify your module name here. This name must be UNIQUE!
    return "Write Geometry";
}

const std::string WMWriteGeometry::getDescription() const
{
    // Specify your module description here. Be detailed. This text is read by the user.
    // See "src/modules/template/" for an extensively documented example.
    return "Exports triangle meshes to certain file formats. VRML implemented so far.";
}

void WMWriteGeometry::connectors()
{
    m_input = boost::shared_ptr< WModuleInputData < WTriangleMesh  > >(
        new WModuleInputData< WTriangleMesh >( shared_from_this(), "Triangle Mesh", "The mesh that will be stored." )
        );

    addConnector( m_input );

    // call WModules initialization
    WModule::connectors();
}

void WMWriteGeometry::properties()
{
    m_savePath         = m_properties->addProperty( "Save Path", "Where to save the result", boost::filesystem::path( "/tmp/test.wrl" ) );
    WPropertyHelper::PC_NOTEMPTY::addTo( m_savePath );

    WModule::properties();
}

void WMWriteGeometry::moduleMain()
{
    m_moduleState.add( m_input->getDataChangedCondition() );

    // signal ready state
    ready();

    // loop until the module container requests the module to quit
    while( !m_shutdownFlag() )
    {
        if( !m_input->getData() )
        {
            // ok, the output has not yet sent data
            // NOTE: see comment at the end of this while loop for m_moduleState
            debugLog() << "Waiting for data ...";
            m_moduleState.wait();
            continue;
        }
        writeFile();

        // this waits for m_moduleState to fire. By default, this is only the m_shutdownFlag condition.
        // NOTE: you can add your own conditions to m_moduleState using m_moduleState.add( ... )
        m_moduleState.wait();
    }
}

void WMWriteGeometry::writeFile()
{
    boost::shared_ptr< WTriangleMesh > mesh = m_input->getData();
    using std::fstream;
    fstream out( m_savePath->get().string().c_str(), fstream::out | fstream::in | fstream::trunc | fstream::binary );
    if( !out || out.bad() )
    {
        throw WException( std::string( "Invalid file, or permission: " ) + m_savePath->get().string() );
    }

    std::string appearance = ""
        "appearance Appearance {\n"
        "material Material {\n"
        "diffuseColor 0.0 1.0 1.0\n"
        "}\n"
        "}\n";

    std::stringstream points;
    for( size_t i = 0; i < mesh->vertSize(); ++i )
    {
        WPosition pos = mesh->getVertex( i );
        points << "" << pos[0] << " " << pos[1] << " " << pos[2];
        if( i < mesh->vertSize() - 1 )
        {
            points << ",\n ";
        }
    }
    std::stringstream indices;
    for( size_t i = 0; i < mesh->triangleSize(); ++i )
    {
        indices << "" << mesh->getTriVertId0( i ) << ", ";
        indices << "" << mesh->getTriVertId1( i ) << ", ";
        indices << "" << mesh->getTriVertId2( i );
        if( i < mesh->triangleSize() - 1 )
        {
            indices << ", -1,\n ";
        }
    }
    out << "#VRML V2.0 utf8" << std::endl;
    out << "# Written with OpenWalnut ( http://www.openwalnut.org )." << std::endl;
    out << "Shape {" << std::endl;
    out << appearance << std::endl;
    out << "geometry IndexedFaceSet {" << std::endl;
    out << "coord DEF SurfaceCoords Coordinate {" << std::endl;
    out << "point [" << std::endl;
    out << points.str() << std::endl;
    out << "]" << std::endl;
    out << "}" << std::endl;
    out << "coordIndex [" << std::endl;
    out << indices.str() << std::endl;
    out << "]" << std::endl;
    out << "}" << std::endl;
    out << "}" << std::endl;

    out.close();
    debugLog() << "Data written to file.";
}
