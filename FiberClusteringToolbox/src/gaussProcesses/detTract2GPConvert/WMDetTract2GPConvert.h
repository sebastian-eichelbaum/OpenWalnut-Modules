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

#ifndef WMDETTRACT2GPCONVERT_H
#define WMDETTRACT2GPCONVERT_H

#include <string>

#include <osg/Geode>

#include "core/dataHandler/WDataSetDTI.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"
#include "../WDataSetGP.h"

/**
 * Converts each deterministic tract to a Gaussian process using the DTI information.
 * \ingroup modules
 */
class WMDetTract2GPConvert: public WModule
{
public:
    /**
     * Constructs a new module.
     */
    WMDetTract2GPConvert();

    /**
     * Destructs this module.
     */
    virtual ~WMDetTract2GPConvert();

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
     * Due to the prototype design pattern used to build modules, this method returns a new instance
     * of this method. NOTE: it should never be initialized or modified in some other way. A simple
     * new instance is required.
     *
     * \return the prototype used to create every module in OpenWalnut.
     */
    virtual boost::shared_ptr< WModule > factory() const;

    /**
     * Get the icon for this module in XPM format.
     * \return The icon.
     */
    virtual const char** getXPMIcon() const;

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

private:
    /**
     * Input connector for the deterministic tract dataset which is going to be converted into
     * Gaussian processes.
     */
    boost::shared_ptr< WModuleInputData< WDataSetFibers > > m_tractIC;

    /**
     * Input connector for the 2nd order tensor field, which is needed for creation of the Gaussian
     * Processes.
     */
    boost::shared_ptr< WModuleInputData< WDataSetDTI > > m_tensorIC;

    /**
     * Output connector for the tracts converted into Gaussian Proccesses.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetGP > > m_gpOC;
};

#endif  // WMDETTRACT2GPCONVERT_H
