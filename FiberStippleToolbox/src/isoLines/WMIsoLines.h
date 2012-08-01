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

#ifndef WMISOLINES_H
#define WMISOLINES_H

#include <string>

#include "../WPropTransfer.h"
#include "core/kernel/WModule.h"

// forward declarations to reduce compile dependencies
class WDataSetScalar;
class WGEManagedGroupNode;
template< class T > class WItemSelectionItemTyped;
template< class T > class WModuleInputData;

/**
 * Computes contour lines (aka isolines) for the given data and render them on a 2D plane.
 * \ingroup modules
 */
class WMIsoLines: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMIsoLines();

    /**
     * Destroys this module.
     */
    virtual ~WMIsoLines();

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
     *
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

    /**
     * Initialize requirements for this module.
     */
    virtual void requirements();

private:
    /**
     * Initialize OSG root node for this module. All other nodes from this module should be attached to this root node.
     *
     * \param scalars The scalar data with grid giving bounding box and other information.
     * \param resolution The size of the quads used for generating line stipples.
     * \param axis The axis selecting the slice (axial, sagittal or coronal).
     */
    void initOSG( boost::shared_ptr< WDataSetScalar > scalars, const double resolution, size_t axis );

    /**
     * Input connector for scalar data.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > > m_scalarIC;

    /**
     * Connector for external WPropDouble, so the slice position of this module can be controlled from another model.
     */
    boost::shared_ptr< WModuleInputData< WPropDoubleTransfer > > m_propIC;

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_output;

    /**
     * The isovalue for the countour lines.
     */
    WPropDouble m_isovalue;

    /**
     * The position of the slice.
     */
    WPropDouble m_pos;

    /**
     * Color for the isoline.
     */
    WPropColor m_color;

    /**
     * Size of the quads used for rendering the isolines, aka resolution.
     */
    WPropDouble m_resolution;

    /**
     * The width of the isolines.
     */
    WPropDouble m_lineWidth;

    /**
     * We need a type for numbering the axis selections. Best would be size_t as then we could directly use the selected item as axis number.
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
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * External property controlling linear translation of the slice.
     */
    WPropDouble m_externPropSlider;

    /**
     * Controlls if the initial state. E.g. slice position.
     */
    bool m_first;
};

#endif  // WMISOLINES_H
