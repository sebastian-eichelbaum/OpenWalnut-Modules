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

#ifndef WMCLUSTERPARAMDISPLAY_H
#define WMCLUSTERPARAMDISPLAY_H

#include <set>
#include <string>

#include <boost/shared_ptr.hpp>

#include <osg/Geode>

#include "core/dataHandler/WDataSetScalar.h"
#include "core/dataHandler/WDataSetFibers.h"
#include "core/dataHandler/datastructures/WJoinContourTree.h"
#include "core/graphicsEngine/WGEGroupNode.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleContainer.h"
#include "core/kernel/WModuleInputForwardData.h"
#include "core/kernel/WModuleOutputForwardData.h"

/**
 * Displays certains parameters of a cluster or bundle.
 *
 * \ingroup modules
 */
class WMClusterParamDisplay: public WModuleContainer
{
public:
    /**
     * Default constructor.
     */
    WMClusterParamDisplay();

    /**
     * Destructor.
     */
    virtual ~WMClusterParamDisplay();

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

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

    // TODO(math): when deactivating a compound module ensure that all submodules are deactivated
    // /**
    //  * En/Disables display of the root node.
    //  */
    // virtual void activate();

    /**
     * Create and initialize submodule instances, wires them and forward connectors as well as some properties.
     */
    virtual void initSubModules();

private:
    /**
     * Input connector required by this module.
     */
    boost::shared_ptr< WModuleInputForwardData< WDataSetFibers > > m_fiberIC;

    /**
     * Input connector for the parameter dataset
     */
    boost::shared_ptr< WModuleInputForwardData< WDataSetScalar > > m_paramIC;

    /**
     * Submodule doing clustering of the fibers and center line generation
     */
    boost::shared_ptr< WModule > m_detTractClustering;

    /**
     * Submodule doing voxelization of a cluster
     */
    boost::shared_ptr< WModule > m_voxelizer;

    /**
     * Submodule blurring the generated voxelized dataset
     */
    boost::shared_ptr< WModule > m_gaussFiltering;

    /**
     * Selects the appropriate isovalue
     */
    boost::shared_ptr< WModule > m_isoSurface;

    /**
     * Module slicing the cluster volume
     */
    boost::shared_ptr< WModule > m_clusterSlicer;

    /**
     * Renders the triangle Mesh with a certain color
     */
    boost::shared_ptr< WModule > m_meshRenderer;


    /**
     * Isovalue for selecting the volume
     */
    WPropDouble m_isoValue;

    /**
     * En/Disable the display of the isosurface
     */
    WPropBool   m_drawIsoSurface;
};

#endif  // WMCLUSTERPARAMDISPLAY_H

