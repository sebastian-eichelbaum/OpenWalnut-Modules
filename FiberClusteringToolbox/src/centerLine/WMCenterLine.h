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

#ifndef WMCENTERLINE_H
#define WMCENTERLINE_H

#include <string>

#include <core/common/WStrategyHelper.h>
#include <core/common/WObjectNDIP.h>
#include <core/kernel/WModule.h>
#include <core/dataHandler/WDataSetFibers.h>

template< class T > class WModuleInputData;
template< class T > class WModuleOutputData;
class WDataSetScalar;

/**
 * Computes centerline from fibers.
 * \ingroup modules
 */
class WMCenterLine: public WModule
{
public:
    WMCenterLine();

    /**
     * Cleans up!
     */
    virtual ~WMCenterLine();

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

    WDataSetFibers::SPtr computeCenterLine( WDataSetFibers::SPtr fibers );


private:
    /**
     * Fibers for which the center line should be computed.
     */
    boost::shared_ptr< WModuleInputData< WDataSetFibers > > m_fibersIC;

    /**
     * Centerline.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetFibers > > m_fibersOC;

    boost::shared_ptr< WModuleOutputData< WDataSetFibers > > m_unifibersOC;
};

#endif  // WMCENTERLINE_H
