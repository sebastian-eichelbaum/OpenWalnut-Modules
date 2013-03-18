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

#ifndef WMEDGEBUNDLING_H
#define WMEDGEBUNDLING_H

#include <string>

#include "core/kernel/WModule.h"

template< class T > class WModuleInputData;
template< class T > class WModuleOutputData;
class WDataSetFibers;
class WDataSetScalar;

/**
 * Bundles lines (aka fibers, tracts, streamlines, etc) using a force directed edge bundling method.
 * \ingroup modules
 */
class WMEdgeBundling: public WModule
{
public:
    /**
     * Creates a edge bundling module for fiber datasets.
     */
    WMEdgeBundling();

    /**
     * Cleans up!
     */
    virtual ~WMEdgeBundling();

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
    void initOpenCL();

    /**
     * Dataset of unbundled fibers.
     */
    boost::shared_ptr< WModuleInputData< WDataSetFibers > > m_fibersIC;

    /**
     * Dataset of bundled fibers.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetFibers > > m_fibersOC;

    /**
     * Dataset used as mask which fiber must not leave. Usually this is a white matter mask of the brain.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > > m_maskIC;

    /**
     * Stiffness, 0 means all points of a fiber may change position, 1 means all points of a fiber may not change position.
     */
    WPropDouble m_stiffness;

    /**
     * If true, end points of fibers may not change position, even if stiffness would allow this.
     */
    WPropBool m_fixedEndings;

    /**
     * Maximal allowed curvature between a number of segments. 0 means, only straight segments are allowed, 1 means every possible cuvature is allowed.
     */
    WPropDouble m_maxCurvature;

    /**
     * Number of segements used for curvature computation. 0 disables curvature threshold, 1 means: last segment and
     * current segment is used, 2 means: the last two segments and current segments contributes to curvature and so on.
     */
    WPropInt m_curveSegments;

    /**
     * There should be a minimal distance between points. 0 means, two points may have exact the same position.
     */
    WPropDouble m_minDistance;

    /**
     * Percentage up to which a fiber may be elongated. 0 percent means, length must be conserved. 100 percent means length may be doubled.
     */
    WPropDouble m_maxExtension;

    /**
     * Percentage up to which a fiber may be shrunk. 0 percent means, length must be conserved. 50 percent means length may be halved.
     */
    WPropDouble m_maxContraction;

    /**
     * Only segments within this radius should contribute to attraction computation. 0 disables this feature and all segments may contribute.
     */
    WPropDouble m_maxRadius;

    /**
     * Strength of angle based attraction.
     */
    WPropDouble m_angleBasedAttraction;

};

#endif  // WMEDGEBUNDLING_H
