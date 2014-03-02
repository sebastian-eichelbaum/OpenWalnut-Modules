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

#ifndef WMLINEGUIDEDSLICE_H
#define WMLINEGUIDEDSLICE_H

#include <string>
#include <vector>
#include <utility>

#include <osg/Node>

#include "core/common/math/WLine.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEViewer.h"
#include "core/graphicsEngine/WPickHandler.h"

// forward declarations
class WFiberCluster;
class WFiber;

/**
 * Module to compute a plane along a line and color it with a scalar parameter.
 * \ingroup modules
 */
class WMLineGuidedSlice : public WModule, public osg::Referenced
{
public:
    /**
     *
     */
    WMLineGuidedSlice();

    /**
     *
     */
    virtual ~WMLineGuidedSlice();

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

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();


private:
    /**
     * Updates the positions of the slice
     */
    void updateGeometry();

    /**
     * Creates the inital geometry.
     */
    void create();

    /**
     * Removes or inserts geode for the center line of the current cluster into this modules group node.
     */
    void updateCenterLine();

    /**
     * Sets slice position from interaction in the main GL widget
     * \param pickInfo The information provided by the pick handler
     */
    void setSlicePosFromPick( WPickInfo pickInfo );

    /**
     * Initial creation function for the slice geometry
     *
     * \return the geometry
     */
    osg::ref_ptr< osg::Geometry > createGeometry();

    WPropDouble m_pos; //!< Slice position along line.

    WVector2d m_oldPixelPosition; //!< Caches the old picked position to a allow for cmoparison
    bool m_isPicked; //!< Indicates whether a slice is currently picked or not.
    boost::shared_ptr< WGEViewer > m_viewer; //!< Stores reference to the main viewer
    boost::shared_ptr< WModuleInputData< const WFiberCluster > > m_input; //!< Input connector for a fiber cluster

    osg::ref_ptr< WGEGroupNode > m_rootNode; //!< The root node for this module.
    osg::ref_ptr< osg::Geode > m_sliceNode; //!< OSG node for slice.
    osg::ref_ptr< osg::Geode > m_centerLineGeode; //!< OSG center line of the current cluster geode.
    boost::shared_mutex m_updateLock; //!< Lock to prevent concurrent threads trying to update the osg node.
    /**
     * OSG Uniform for the transformation matrix which transforms the mesh. Needed for the colormapper
    */
    osg::ref_ptr< osg::Uniform > m_colorMapTransformation;
    /**
     * The shader for the mesh
     */
    osg::ref_ptr< WGEShader > m_shader;

    boost::shared_ptr< WFiber > m_centerLine; //!< The line that guides the slice.

    /**
     * Node callback to handle updates properly
     */
    class sliceNodeCallback : public osg::NodeCallback
    {
    public: // NOLINT
        /**
         * operator ()
         *
         * \param node the osg node
         * \param nv the node visitor
         */
        virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
        {
            osg::ref_ptr< WMLineGuidedSlice > module = static_cast< WMLineGuidedSlice* > ( node->getUserData() );

            if( module )
            {
                module->updateGeometry();
            }
            traverse( node, nv );
        }
    };


    /**
     * Generates a line geode with thickness and color as parameters.
     *
     * \param line sequence of points
     * \param thickness How thick the line strip should be
     * \param color If present this color is used for the whole line, otherwise local coloring is used
     *
     * \return The new assembled geode for this line
     */
    osg::ref_ptr< osg::Geode > generateLineStripGeode( const WLine& line,
                                                       const float thickness = 3.0f,
                                                       const WColor& color = WColor( 0, 0, 0, 0 ) );
};

#endif  // WMLINEGUIDEDSLICE_H
