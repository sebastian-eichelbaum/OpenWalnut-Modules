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

// This file's purpose is to provide a list of modules and additional extensions as entry point for OpenWalnut's module loader.
// Both functAdd your modules here. If you miss this step, OpenWalnut will not be able to load your modules/extensions.

#include <boost/shared_ptr.hpp>

#include <core/kernel/WModule.h>

#include "WMImageSpaceTensorLIC.h"

/**
 * This function is called by OpenWalnut, when loading your library to learn about the modules you provide. The function is called with a given
 * list reference, where you add all of your modules. Modules which are not registered this way, cannot be used in OpenWalnut. As this is called
 * before loading any project file or running any module, it is ensured that you can rely on the modules provided here in your project files and
 * other modules.
 *
 * \note this function is optional. You can remove it if you do not need it.
 *
 * \param m the list of modules. Add you module instances into this list.
 */
extern "C" void WLoadModule( WModuleList& m ) // NOLINT
{
    // This line is needed by the module loader to actually find your module. You need to add this to your module too. Do NOT add a ";" here.
    m.push_back( boost::shared_ptr< WModule >( new WMImageSpaceTensorLIC ) );
}

