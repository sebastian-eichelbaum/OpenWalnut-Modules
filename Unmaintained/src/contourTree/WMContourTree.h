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

#ifndef WMCONTOURTREE_H
#define WMCONTOURTREE_H

#include <string>

#include <osg/Geode>

#include <core/kernel/WModule.h>
#include <core/kernel/WModuleInputData.h>
#include <core/kernel/WModuleOutputData.h>
#include <core/dataHandler/datastructures/WJoinContourTree.h>

/**
 * Computes the contour tree of a scalar field. Unless your not knowing what you do
 * take care that the field is:
 *  - the mesh or grid is simplicial
 *  - valule set has pairwise different values
 *
 * \note ATM there you only may have the JoinTree, SplitTree nor the whole ContourTree is not implemented yet.
 *
 * \ingroup modules
 */
class WMContourTree: public WModule
{
public:
    /**
     * Default constructor.
     */
    WMContourTree();

    /**
     * Destructor.
     */
    virtual ~WMContourTree();

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

    boost::shared_ptr< WModuleInputData< WDataSetSingle > >    m_input;  //!< Dataset to computate contour tree of
    boost::shared_ptr< WModuleOutputData< WJoinContourTree > > m_output; //!< The JoinTree for that dataset

private:
};

#endif  // WMCONTOURTREE_H

