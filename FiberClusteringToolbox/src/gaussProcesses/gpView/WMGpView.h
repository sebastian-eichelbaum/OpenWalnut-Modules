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

#ifndef WMGPVIEW_H
#define WMGPVIEW_H

#include <string>

#include <osg/Geode>

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/WGESubdividedPlane.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "../WDataSetGP.h"

/**
 * This module is intended for Gaussian Process display. A Slice is generated which is used to cut
 * the volume and depict the mean function color coded onto its surface.
 * \ingroup modules
 */
class WMGpView: public WModule
{
public:
    /**
     * Constructs a new GP viewer.
     */
    WMGpView();

    /**
     * Destructs this viewer.
     */
    virtual ~WMGpView();

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
     * Generates the transformation matrix to scale, rotate and translate the SubdividedPlane onto its place.
     *
     * \note This matrix must be multiplied from right in order to be working!
     *
     * \return Transformation matrix.
     */
    osg::Matrixd generateMatrix() const;

    /**
     * Incase of a new matrix the center points of the quads inside the subdivided plane geode will
     * be also transformed. Since they determine the Color of the whole quad, (mean function
     * evaluation and color mapping) we need to transform the center points to and generate a new
     * color array for the WGESubdividedPlane geode. To indicate that a new color array may be used,
     * the m_newPlaneColors flag is set to true.
     *
     * \param m The transformation matrix used to accomplish the transformation of the center
     * points.
     * \param dataset For each center point all mean functions must be evaluated, Hence we need this
     * reference to the dataset.
     *
     * \return New color array.
     */
    osg::ref_ptr< osg::Vec4Array > generateNewColors( const osg::Matrixd& m, boost::shared_ptr< const WDataSetGP > dataset ) const;

    /**
     * This update callback is used in case a new color array is present to change the colors on the
     * quads of the WGESubdividedPlane Quads.
     *
     * \param node Typically you won't call this member function explicit, but take it as update
     * callback for the WGESubdividedPlane geode.
     */
    void updatePlaneColors( osg::Node* node );

private:
    /**
     * Input connector for the Gaussian proccesses.
     */
    boost::shared_ptr< WModuleInputData< WDataSetGP > > m_gpIC;

    /**
     * The root node used for this modules graphics. For OSG nodes, always use osg::ref_ptr to ensure proper resource management.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**
     * In order to add an update callback we need this reference. Also we must retrieve sometimes
     * the vertex array of the centerpoints to apply the same transformation on them!
     */
    osg::ref_ptr< WGESubdividedPlane > m_planeNode;

    /**
     * This color array is build up in case of a transformation. Once it is finished and
     * m_newPlaneColors set to true, the update callback \ref updatePlaneColors will take
     * use of this new array and replace the old colors.
     */
    osg::ref_ptr< osg::Vec4Array > m_newColors;

    /**
     * The base point of the plane.
     */
    WPropPosition m_pos;

    /**
     * The normal of the plane
     */
    WPropPosition m_normal;

    /**
     * The scaling of the subdivided plane geode in x,y and z direction.
     */
    WPropDouble m_scale;

    /**
     * Flag to inidicate the update callback of the WGESubdividedPlane geode that a new color array
     * is ready to use.
     */
    bool m_newPlaneColors;

    /**
     * Generates a plane subdivided into quads.
     *
     * \param resX How many quads in x-direction
     * \param resY How many quads in y-direction
     * \param spacing Not implement yet
     *
     * \return The new uncolored plane geode
     */
    osg::ref_ptr< WGESubdividedPlane > genUnitSubdividedPlane( size_t resX, size_t resY, double spacing = 0.01 );
};

#endif  // WMGPVIEW_H
