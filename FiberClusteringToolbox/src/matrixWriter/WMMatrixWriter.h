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

#ifndef WMMATRIXWRITER_H
#define WMMATRIXWRITER_H

#include <string>

#include <core/common/math/WMatrixSym.h>
#include <core/kernel/WModule.h>

#include "../WDataSetMatrixSym.h"

template< class T > class WModuleInputData;
template< class T > class WModuleOutputData;

/**
 * Writes a symmetrical Matrix down to file.
 * \ingroup modules
 */
class WMMatrixWriter : public WModule
{
public:
    /**
     * Default module ctor.
     */
    WMMatrixWriter();

    /**
     * Default module dtor.
     */
    virtual ~WMMatrixWriter();

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
     * Input connector for the Symmetric Matrix.
     */
    boost::shared_ptr< WModuleInputData< WDataSetMatrixSymFLT > > m_fltMatrixIC;

    /**
     * Save data to file.
     */
    void save();

    /**
     * The filename property -> where to write the nifty file
     */
    WPropFilename m_filename;

    /**
     * This property triggers the actual writing
     */
    WPropTrigger  m_saveTrigger;

    // /**
    //  * Trigger binary or text mode.
    //  */
    // WPropBool m_binary;
};

#endif  // WMMATRIXWRITER_H
