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

#ifndef WMELEVATIONIMAGEEXPORT_H
#define WMELEVATIONIMAGEEXPORT_H


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
#include "../common/datastructures/quadtree/WQuadTree.h"



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

#include "bitmapImage/WBmpImage.h"
#include "bitmapImage/WBmpSaver.h"

#include "WElevationImageOutliner.h"
#include "../common/datastructures/WDataSetPointsGrouped.h"

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WDataSetScalar;
class WGEManagedGroupNode;

/**
 * Module to export an elevation image bitmap
 * \ingroup modules
 */
class WMElevationImageExport: public WModule
{
public:
    /**
     * Constructs the module.
     */
    WMElevationImageExport();

    /**
     * Destroys this module.
     */
    virtual ~WMElevationImageExport();

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
     * WDataSetPoints data input (proposed for LiDAR data).
     */
    boost::shared_ptr< WModuleInputData< WDataSetPoints > > m_input;
    /**
     * Grouped points input data to highlight detected groupes using a set of colors.
     */
    boost::shared_ptr< WModuleInputData< WDataSetPointsGrouped > > m_pointGroups;
    /**
     * The output connector containing the elevation image outlined to the triangle mesh.
     */
    boost::shared_ptr< WModuleOutputData< WTriangleMesh > > m_elevationImageDisplay;

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**
     * Needed for recreating the geometry, incase when resolution changes.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * Info tab property: Input points count.
     */
    WPropInt m_nbPoints;
    /**
     * Info tab property: Minimal x value of input x coordunates.
     */
    WPropDouble m_xMin;
    /**
     * Info tab property: Maximal x value of input x coordunates.
     */
    WPropDouble m_xMax;
    /**
     * Info tab property: Minimal y value of input x coordunates.
     */
    WPropDouble m_yMin;
    /**
     * Info tab property: Maximal y value of input x coordunates.
     */
    WPropDouble m_yMax;
    /**
     * Info tab property: Minimal z value of input x coordunates.
     */
    WPropDouble m_zMin;
    /**
     * Info tab property: Maximal z value of input x coordunates.
     */
    WPropDouble m_zMax;

    /**
     * Determines the resolution of the smallest octree nodes in 2^n meters
     */
    WPropInt m_detailDepth;
    /**
     * Determines the resolution of the smallest octree nodes in meters
     */
    WPropDouble m_detailDepthLabel;

    /**
     * Mode of the elevation image to display
     * 0: Minimal Z value of each X/Y bin coordinate.
     * 1: Maximal Z value of each X/Y bin coordinate.
     * 2: Point count of each X/Y bin coordinate.
     */
    WPropSelection m_elevImageMode;

    /**
     * Elevation image export setting. 
     * Elevation height that will be displayed as the black color.
     */
    WPropDouble m_minElevImageZ;
    /**
     * Elevation image export setting. 
     * Count of intensity increases per meter.
     */
    WPropDouble m_intensityIncreasesPerMeter;

    /**
     * Path of the exportable elevation image *.bmp file.
     */
    WPropFilename m_elevationImageExportablePath; //!< Path of the exportable elevation image *.bmp file.
    WPropTrigger  m_exportTriggerProp; //!< This property triggers the actual reading,
    /**
     * If trigger set then the elevation will be displayed in the triangle mesh color.
     */
    WPropBool m_showElevationInMeshColor;
    /**
     * If trigger set then the elevation will be displayed in the triangle mesh height offset.
     */
    WPropBool m_showElevationInMeshOffset;

    /**
     * Plugin progress status that is shared with the reader.
     */
    boost::shared_ptr< WProgress > m_progressStatus;
    /**
     * This is the elevation image of the whole data set.
     * It depicts some statistical Z coordinate information of each X/Y-coordinate.
     */
    WQuadTree* m_elevationImage;
};

#endif  // WMELEVATIONIMAGEEXPORT_H
