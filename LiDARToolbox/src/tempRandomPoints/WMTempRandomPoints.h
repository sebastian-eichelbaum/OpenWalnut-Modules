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

#ifndef WMTEMPRANDOMPOINTS_H
#define WMTEMPRANDOMPOINTS_H


#include <liblas/liblas.hpp>
#include <string>
#include <vector>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include "core/kernel/WModule.h"

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WTriangleMesh.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/datastructures/octree/WOctree.h"





//!.Unnecessary imports
#include "core/common/WItemSelection.h"
#include "core/common/WItemSelector.h"

#include "core/kernel/WModuleOutputData.h"

#include <osg/Group>
#include <osg/Material>
#include <osg/StateAttribute>

#include "core/kernel/WKernel.h"
#include "core/common/exceptions/WFileNotFound.h"
#include "core/common/WColor.h"
#include "core/common/WPathHelper.h"
#include "core/common/WPropertyHelper.h"
#include "core/common/WItemSelectionItem.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WGERequirement.h"


#include "../common/math/leastSquares/WLeastSquares.h"


// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;

/**
 * Creates a cubical or spherical random set of points.
 * Use a transformation plugin to get a customized point set.
 * \ingroup modules
 */
class WMTempRandomPoints: public WModule
{
public:
    /**
     * Creates the module for drawing contour lines.
     */
    WMTempRandomPoints();

    /**
     * Destroys this module.
     */
    virtual ~WMTempRandomPoints();

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
     *Initialize the connectors this module is using.
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
     * Initializes progress bar settings.
     * \param steps Points count as reference to the progress bar.
     */
    void setProgressSettings( size_t steps );

    /**
     * Returns a pseudo random number. It depends on a counter. The aim is to get 
     * reproducable results.
     * \return Pseudo random number between 0 and 1.
     */
    double getNextRandomNumber();

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**
     * Returns a cropped data set corresponding to the selection. The selection is
     * set by m_<from/to>_<X/Y/Z>. m_cutInsteadOfCrop determines whether to crop to
     * a selection or to cut away a cube area.
     * \return The cropped or cut point data set.
     */
    boost::shared_ptr< WDataSetPoints > getRandomPoints();

    /**
     * Processed point data with cut off outliers.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetPoints > > m_output;

    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * Plugin progress status that is shared with the reader.
     */
    boost::shared_ptr< WProgress > m_progressStatus;

    /**
     * Input point coordinates to crop.
     */
    WDataSetPoints::VertexArray m_verts;

    /**
     * Colors of the input point data set that are also passed through.
     */
    WDataSetPoints::ColorArray m_colors;

    /**
     * The last random number It's stored to calculate a next number deterministically. 
     * Reproducable results are produced that way
     */
    double m_currentRandomNumber;

    /**
     * Settings for the random point generator.
     */
    WPropGroup m_groupRandomPointGenerator;

    /**
     * Organizes points in a spherical space instead of in a qubical one.
     */
    WPropBool m_isSphericalSpace;

    /**
     * Point count in the random point set.
     */
    WPropInt m_pointCount;

    /**
     * Initial number for the random number generator.
     */
    WPropDouble m_initRandomNumber;
};

#endif  // WMTEMPRANDOMPOINTS_H
