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

#ifndef WMIMAGESPACETENSORLIC_H
#define WMIMAGESPACETENSORLIC_H

#include <string>
#include <vector>

#include "core/dataHandler/WDataSetVector.h"

#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"

#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"

/**
 * This module takes a second order symmetric tensor dataset and uses it to apply an image space based (fast) fabric-like LIC to an arbitrary
 * surface. The surface can be specified as tri mesh or, if not specified, slices.
 *
 * \ingroup modules
 */
class WMImageSpaceTensorLIC: public WModule
{
public:
    /**
     * Default constructor.
     */
    WMImageSpaceTensorLIC();

    /**
     * Destructor.
     */
    virtual ~WMImageSpaceTensorLIC();

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

private:
    /**
     * Initializes the needed geodes, transformations and vertex arrays. This needs to be done once for each new dataset.
     *
     * \param grid the grid to places the slices in
     * \param mesh the mesh to use if not NULL and m_useSlices is false
     */
    void initOSG( boost::shared_ptr< WGridRegular3D > grid, boost::shared_ptr< WTriangleMesh > mesh );

    /**
     * The input connector containing the DTI field whose derived field is used for LIC.
     */
    boost::shared_ptr< WModuleInputData< WDataSetVector > > m_evec1In;

    /**
     * The input connector containing the DTI field whose derived field is used for LIC.
     */
    boost::shared_ptr< WModuleInputData< WDataSetVector > > m_evec2In;

    /**
     * The input connector containing the DTI field whose derived field is used for LIC.
     */
    boost::shared_ptr< WModuleInputData< WDataSetVector > > m_evalsIn;

    /**
     * The input containing the surface on which the LIC should be applied on
     */
    boost::shared_ptr< WModuleInputData< WTriangleMesh > > m_meshIn;

    /**
     * A property allowing the user to select whether the slices or the mesh should be used
     */
    WPropSelection m_geometrySelection;

    /**
     * A list of items that can be selected using m_geometrySelection.
     */
    boost::shared_ptr< WItemSelection > m_geometrySelections;

    /**
     * A condition used to notify about changes in several properties.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * The Geode containing all the slices and the mesh
     */
    osg::ref_ptr< WGEGroupNode > m_output;

    /**
     * Scene root node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_root;

    WPropGroup    m_sliceGroup; //!< the group contains several slice properties

    WPropGroup    m_geometryGroup; //!< the group contains several input geometry parameters

    WPropGroup    m_licGroup; //!< the group contains several LIC properties

    WPropBool     m_useSlices; //!< indicates whether the vector should be shown on slices or input geometry

    WPropInt      m_xPos; //!< x position of the slice

    WPropInt      m_yPos; //!< y position of the slice

    WPropInt      m_zPos; //!< z position of the slice

    WPropBool     m_showonX; //!< indicates whether the vector should be shown on slice X

    WPropBool     m_showonY; //!< indicates whether the vector should be shown on slice Y

    WPropBool     m_showonZ; //!< indicates whether the vector should be shown on slice Z

    WPropBool     m_showHUD; //!< indicates whether to show the texture HUD

    WPropBool     m_useEdges; //!< indicates whether to show the edges

    WPropColor    m_useEdgesColor; //!< indicated whether the edges (if enabled) should be black or white or green or red or ....

    WPropDouble   m_useEdgesStep; //!< define the steepness of the step function used to blend in the edge color.

    WPropBool     m_useLight; //!< indicates whether to use Phong

    WPropDouble   m_lightIntensity; //!< light intensity

    WPropInt      m_numIters; //!< the number of iterations done per frame

    WPropDouble   m_cmapRatio; //!< the ratio between colormap and LIC

    WPropDouble   m_projectionAngleThreshold; //!< the angle threshold between surface and vector before clipping the vector.

    /**
     * The group for more advanced LIC features
     */
    WPropGroup    m_advancedLicGroup;

    /**
     * The resolution scaling for the noise
     */
    WPropDouble   m_noiseRes;

    /**
     * Clipp according to FA.
     */
    WPropDouble   m_faClip;
};

#endif  // WMIMAGESPACETENSORLIC_H

