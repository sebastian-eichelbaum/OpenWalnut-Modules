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

#include <boost/shared_ptr.hpp>

#include <core/kernel/WModule.h>

#include "atlasCreator/WMAtlasCreator.h"
#include "atlasSurfaces/WMAtlasSurfaces.h"
#include "contourTree/WMContourTree.h"
#include "hud/WMHud.h"
#include "lineGuidedSlice/WMLineGuidedSlice.h"
#include "writeGeometry/WMWriteGeometry.h"
#include "sliceContext/WMSliceContext.h"

#include "WToolkit.h"

// This file's purpose is to provide a list of modules as entry point for OpenWalnut's module loader.
// Add your modules here. If you miss this step, OpenWalnut will not be able to load your modules.
extern "C" void WLoadModule( WModuleList& m ) // NOLINT
{
    m.push_back( boost::shared_ptr< WModule >( new WMAtlasCreator ) );
    m.push_back( boost::shared_ptr< WModule >( new WMAtlasSurfaces ) );
    m.push_back( boost::shared_ptr< WModule >( new WMContourTree ) );
    m.push_back( boost::shared_ptr< WModule >( new WMHud ) );
    m.push_back( boost::shared_ptr< WModule >( new WMLineGuidedSlice ) );
    m.push_back( boost::shared_ptr< WModule >( new WMWriteGeometry ) );
    m.push_back( boost::shared_ptr< WModule >( new WMSliceContext ) );
}

