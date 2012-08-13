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

#ifndef WMTRANSFERCALC_H
#define WMTRANSFERCALC_H

// Some rules to the inclusion of headers:
//  * Ordering:
//    * C Headers
//    * C++ Standard headers
//    * External Lib headers (like OSG or Boost headers)
//    * headers of other classes inside OpenWalnut

#include <vector>
#include <algorithm>
#include <string>

#include <osg/Node>
#include <osg/Geode>
#include <osg/Uniform>

#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/dataHandler/WDataSetVector.h"
#include "core/common/math/linearAlgebra/WLinearAlgebra.h"
#include "core/common/math/WMatrix.h"

#include "../dataStructures/WRay.h"
#include "../dataStructures/WRaySample.h"
#include "../dataStructures/WRayProfile.h"
#include "../interaction/WGetMatrixCallback.h"
#include "../interaction/WViewEventHandler.h"

/**
 * --
 *
 * \ingroup modules
 */
class WMTransferCalc: public WModule
{
public:
    /**
     * Standard constructor.
     */
    WMTransferCalc();

    /**
     * Destructor.
     */
    virtual ~WMTransferCalc();

    /**
     * Gives back the name of this module.
     * \return the module's name.
     */
    virtual const std::string getName() const;

    /**
     * Gives back a description of this module.
     * \return description of module.
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

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

    /**
     * Initialize requirements for this module.
     */
    virtual void requirements();

    /**
     * The root node used for this modules graphics. For OSG nodes, always use osg::ref_ptr to ensure proper resource management.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_rootNode;

    /**
     * The geometry rendered by this module.
     */
    osg::ref_ptr< osg::Geode > m_geode;

private:
    /**
     * Handle click events in the main view
     *
     * \param mousePos the mouse position
     */
    void onClick( WVector2i mousePos );

    /**
     * Trilinear interpolation within the grid for a given position
     *
     * \param position Position for which the value should be determined.
     * \param grid Pointer to the grid, which shall be the base for the interpolation.
     *
     * \return interpolated value
     */
    virtual double interpolate( const WVector4d& position, boost::shared_ptr< WGridRegular3D > inter_grid );

    /**
     * Curvature calculation for given derivated dataset.
     */
    virtual void calculateCurvature();

    /**
     * Gradient calculation for a given position in the grid.
     *
     * \param position Position for which the gradient should be determined.
     *
     * \return gradient at the given position
     */
    virtual WVector4d getGradient( const WVector4d& position );

    /**
     * Cast a single ray through the current data.
     *
     * \param ray WRay object which holds the start and direction vector of the ray.
     * \param interval Interval for taking the samples on the ray.
     * \param rayGeode the method adds a drawable to this geode representing the ray that has been casted.
     *
     * \return complete profile of casted ray
     */
    virtual WRayProfile castRay( WRay ray, double interval, osg::ref_ptr< osg::Geode > rayGeode );

    /**
     * Calculates the nearest and farest intersection
     * of the ray and the m_outer_bounding box.
     *
     * \param ray Current ray.
     *
     * \return  An array with the nearest and farest t values.
     */
    virtual struct BoxIntersecParameter rayIntersectsBox( WRay ray );

    /**
     * Function to save a RayProfile to a given file.
     *
     * \param path Filename of the file where the data should be saved to
     * \param filename Name of the file.
     * \param profile Pointer to RayProfile that shall be saved.
     *                If this parameter is NULL, the current Profile will be saved if it exists.
     *
     * \return true if saving is successful, false otherwise
     */
    bool saveRayProfileTo( WPropFilename path, std::string filename, WRayProfile *profile = NULL );

    /**
     * Calculating a WVector3d out of a given WVector4d
     *
     * \param vec 4D vector which shall be transfered to 3D
     *
     * \return Calculated Vector.
     */
    WVector3d getAs3D( const WVector4d& vec, bool disregardW = false );

    /**
     * An input connector used to get datasets from other modules.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > >  m_inputData;

    /**
     * An input connector used to get the derivated dataset from the module 'Spacial Derivative'.
     */
    boost::shared_ptr< WModuleInputData< WDataSetVector > >  m_inputDerivation;

    /**
     * An input connector used to get fractional anisotropy datasets from other modules.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > >  m_inputFA;

    /**
     * The output connector used to provide the calculated curvature data in mean form to other modules.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetScalar > > m_curveMeanOut;

    /**
     * The output connector used to provide the calculated curvature data in Gauss form to other modules.
     */
    boost::shared_ptr< WModuleOutputData< WDataSetScalar > > m_curveGaussOut;

    /**
     * A condition used to notify about changes in several properties.
     */
    boost::shared_ptr< WCondition > m_propCondition;

//     /**
//      * All profiles of the current dataset.
//      */
//     std::vector<WRayProfile> m_dataRays;

    /**
     * Current dataset
     */
    boost::shared_ptr< WDataSetScalar > m_dataSet;

    /**
     * Optinal: additional dataset with derivated data.
     */
    boost::shared_ptr< WDataSetVector > m_deriDataSet;

    /**
     * Optinal: additional dataset with fractional anisotropy data.
     */
    boost::shared_ptr< WDataSetScalar > m_FAdataSet;

    /**
     * Boolean to check if any derivated data is present and valid.
     */
    bool m_DeriIsValid;

    /**
     * Boolean to check if any FA data is present and valid.
     */
    bool m_FAisValid;

    /**
     * Contains the minimum and maximum values of the bounding box.
     * outer_bounding[0] = ( min_x, min_y, min_z )
     * outer_bounding[1] = ( max_x, max_y, max_z )
     */
    std::vector< WVector4d > m_outer_bounding;

    /**
     * Current grid
     */
    boost::shared_ptr< WGridRegular3D > m_grid;

    /**
     * Current grid of derivated data
     */
    boost::shared_ptr< WGridRegular3D > m_deriGrid;

    /**
     * Current FA grid
     */
    boost::shared_ptr< WGridRegular3D > m_FAgrid;

    /**
     * Collection of all calculated RayProfiles.
     */
    std::vector< WRayProfile > m_profiles;

    /**
     * Latest calculated RayProfile.
     */
    WRayProfile m_mainProfile;

    /**
     * Camera position, changes on click.
     */
    WVector2i m_currentCamPosition;

    /**
     * Filename for saving a RayProfile.
     */
    WPropFilename m_RaySaveFilePath;

    /**
     * Trigger for saving RayProfile to given directory m_RaySaveFilePath.
     */
    WPropTrigger  m_saveTrigger;

    /**
     * x position of the ray origin.
     */
    WPropDouble   m_xPos;

    /**
     * y position of the ray origin.
     */
    WPropDouble   m_yPos;

    /**
     * z position of the ray origin.
     */
    WPropDouble   m_zPos;

    /**
     * Interval for sampling rays.
     */
    WPropDouble   m_interval;

    /**
     * Number of rays being casted.
     */
    WPropInt   m_rayNumber;

    /**
     * Radius for circular vicinity in which the rays shall be casted.
     */
    WPropInt   m_radius;

    /**
     * A color.
     */
    WPropColor  m_color;

    /**
     * Node callback to change the color of the shapes inside the root node. For more details on this class, refer to the documentation in
     * moduleMain().
     */
    class SafeUpdateCallback : public osg::NodeCallback
    {
    public: // NOLINT
        /**
         * Constructor.
         *
         * \param module just set the creating module as pointer for later reference.
         */
        explicit SafeUpdateCallback( WMTransferCalc* module ): m_module( module ), m_initialUpdate( true )
        {
        };

        /**
         * operator () - called during the update traversal.
         *
         * \param node the osg node
         * \param nv the node visitor
         */
        virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

        /**
         * Pointer used to access members of the module to modify the node.
         * Please do not use shared_ptr here as this would prevent deletion of the module as the callback contains
         * a reference to it. It is safe to use a simple pointer here as callback get deleted before the module.
         */
        WMTransferCalc* m_module;

        /**
         * Denotes whether the update callback is called the first time. It is especially useful
         * to set some initial value even if the property has not yet changed.
         */
        bool m_initialUpdate;
    };

    /**
     * Needed to query the modelview and projection matrices of the m_rootNode;
     */
    WGetMatrixCallback::RefPtr m_matrixCallback;
};

struct BoxIntersecParameter
{
        double minimum_t;
        double maximum_t;
        bool isNull;
};

#endif  // WMTRANSFERCALC_H
