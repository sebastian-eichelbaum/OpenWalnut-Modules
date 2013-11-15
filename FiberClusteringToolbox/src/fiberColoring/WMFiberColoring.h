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

#ifndef WMFIBERCOLORING_H
#define WMFIBERCOLORING_H

#include <string>

#include <core/common/WStrategyHelper.h>
#include <core/common/WObjectNDIP.h>
#include <core/kernel/WModule.h>

#include "WColoring_I.h"

template< class T > class WModuleInputData;
template< class T > class WModuleOutputData;
class WDataSetFibers;
class WDataSetScalar;


// \ingroup modules
class WMFiberColoring: public WModule
{
public:
    WMFiberColoring();

    /**
     * Cleans up!
     */
    virtual ~WMFiberColoring();

    /**
     * Gives back the name of this module.
     * \return the module's name.
     */
    virtual const std::string getName() const;

    /**
     * Gives back a description of this module.
     * \return description to module.
     */
    virtual const std::string getDescription() const;

    /**
     * Due to the prototype design pattern used to build modules, this method returns a new instance of this method. NOTE: it
     * should never be initialized or modified in some other way. A simple new instance is required.
     *
     * \return the prototype used to create every module in OpenWalnut.
     */
    virtual boost::shared_ptr< WModule > factory() const;

protected:
    /**
     * Entry point after loading the module. Runs in separate thread.
     */
    virtual void moduleMain();

    /**
     * Initialize the connectors this module is using.
     */
    virtual void connectors();

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

    /**
     * Initialize requirements for this module.
     */
    virtual void requirements();


private:
    /**
     * Fibers to color.
     */
    boost::shared_ptr< WModuleInputData< WDataSetFibers > > m_fibersIC;

    /**
     * Colored fibers.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetFibers > > m_fibersOC;

    /**
     * Strategies for coloring fibers.
     */
    WStrategyHelper< WObjectNDIP< WColoring_I > > m_strategy;
};

#endif  // WMFIBERCOLORING_H
