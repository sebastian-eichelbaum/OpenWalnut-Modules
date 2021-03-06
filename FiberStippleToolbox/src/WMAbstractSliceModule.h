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

#ifndef WMABSTRACTSLICEMODULE_H
#define WMABSTRACTSLICEMODULE_H

#include <string>

#include "core/kernel/WModule.h"

// forward declarations to reduce compile dependencies
class WGEManagedGroupNode;
template< class T > class WItemSelectionItemTyped;

/**
 * Module containing convinience stuff for slice based modules.
 * \ingroup modules
 */
class WMAbstractSliceModule: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMAbstractSliceModule();

    /**
     * Destroys this module.
     */
    virtual ~WMAbstractSliceModule();

    /**
     * Gives back the name of this module.
     * \return the module's name.
     */
    virtual const std::string getName() const = 0;

    /**
     * Gives back a description of this module.
     * \return description to module.
     */
    virtual const std::string getDescription() const = 0;

    /**
     * Due to the prototype design pattern used to build modules, this method returns a new instance of this method. NOTE: it
     * should never be initialized or modified in some other way. A simple new instance is required.
     *
     * \return the prototype used to create every module in OpenWalnut.
     */
    virtual boost::shared_ptr< WModule > factory() const = 0;

    /**
     * Get the icon for this module in XPM format.
     *
     * \return The icon.
     */
    virtual const char** getXPMIcon() const = 0;

protected:
    /**
     * Entry point after loading the module. Runs in separate thread.
     */
    virtual void moduleMain() = 0;

    /**
     * If the slice positions in the kernel change, our m_pos is updated. This make all slice modules easy to use with navigation slices.
     * \note If there is no navigation slice module, there should be no trouble.
     */
    virtual void updatePos();

    /**
     * Initialize the connectors this module is using.
     */
    virtual void connectors();

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

    /**
     * Maps medical slice names to axis numbers, e.g. "Coronal Slice" is mapped to \e 1. This is needed to determine slice
     * type out of a slice property name.
     *
     * \param name One of "Axial Slice", "Coronal Slice", "Sagittal Slice".
     *
     * \return 0, 1, 2 as axis numbers corresponding to the same order as given above and wlimits::SIZE_T_MAX else.
     */
    size_t selectAxis( const std::string& name ) const;

    /**
     * Determines two vectors spanning the plane orthogonal to the given axis, where 0 means xAxis, 1 means yAxis and 2 zAxis.
     *
     * \param sizes Maximal dimensions in x, y and z direction.
     * \param axis Which of the axis (x,y or z) is orthogonal to the demanded slice.
     *
     * \return Two independent vectors, both orthogonal to the selected axis.
     */
    std::pair< WVector3d, WVector3d > sliceBaseVectors( const WVector3d& sizes, const size_t axis ) const;

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_output;

    /**
     * The position of the slice.
     */
    WPropDouble m_pos;

    /**
     * For numbering the axes selection.
     */
    typedef WItemSelectionItemTyped< size_t > AxisType;

    /**
     * Selection for axis / plane or slice. Meaning whether we should draw stipples on Axial, Cornoal or Sagittal slices.
     */
    WPropSelection m_sliceSelection;

    /**
     * Possible axes as a property selection list.
     */
    boost::shared_ptr< WItemSelection > m_axes;

    /**
     * Needed for recreating the geometry.
     */
    boost::shared_ptr< WCondition > m_propCondition;

private:
};

#endif  // WMABSTRACTSLICEMODULE_H
