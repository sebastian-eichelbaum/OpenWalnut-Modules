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

#ifndef WMSLICECONTEXT_H
#define WMSLICECONTEXT_H

#include <string>
#include <vector>

#include <osg/Geode>

#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"
#include "core/kernel/WSelectionManager.h"

/**
 * Someone should add some documentation here.
 * Probably the best person would be the module's
 * creator, i.e. "wiebel".
 *
 * This is only an empty template for a new module. For
 * an example module containing many interesting concepts
 * and extensive documentation have a look at "src/modules/template"
 *
 * \ingroup modules
 */
class WMSliceContext: public WModule
{
public:
    /**
     *
     */
    WMSliceContext();

    /**
     *
     */
    virtual ~WMSliceContext();

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
     * \return the icon.
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
     * Updates the visualization produced by this module.
     */
    virtual void update();

    /**
     * Generates an osg geode for the given tract selection with the given color
     *
     * \param selectedTracts This vector specifies which tracts should be used to generate the node.
     * \return geode containing the graphical representation of the tracts
     */
    osg::ref_ptr< osg::Geode > genTractGeode( const std::vector< size_t >& selectedTracts ) const;

    /**
     * Choose colors and build new OSG node for chosen tracts.
     *
     * \param selectedTracts This vector specifies which tracts should be painted
     * \return The OSG group node with all tracts
     */
    osg::ref_ptr< WGEManagedGroupNode > paintTracts( const std::vector< size_t >& selectedTracts ) const;

    /**
     * Runs through all fiber an test them against the slice. If counting is true the method
     * is slower but it counts the number of points of each tract inside the slice context.
     *
     * \param selected number of points of each fiber inside the context of the slice.
     * \param counting if this is false, only the first containments is counted. This makes the method faster.
     * \param distance Defines the with of the region that is considered for the slice context.
     */
    void checkContainment( std::vector< size_t >* selected, bool counting, double distance ) const;

    boost::shared_ptr< WModuleInputData< const WDataSetFibers > > m_fiberInput; //!< The fiber dataset which is going to be filtered.

    WPropPosition m_crosshairProp; //!< position of the navigation
    WPropInt m_insideCountProp; //!< The number of positions of a fiber that have to be inside the context for the fiber to be considered.
    WPropDouble m_contextWidthProp; //!< The width of the area around the slice that is considere as context.

    boost::shared_ptr< WCondition > m_propCondition; //!< A condition used to notify about changes in several properties.

    WPosition m_current; //!< The current position of the slices.
    boost::shared_ptr< const WDataSetFibers > m_tracts; //!< The fiber data set used for the context.
    osg::ref_ptr< WGEManagedGroupNode > m_osgNode; //!< OSG node for this module.
    osg::ref_ptr< osg::Group > m_rootNode; //!< OSG node for this module.

    osg::ref_ptr< WGEShader > m_shaderFibers; //!< The shader object for rendering fibers

    /**
     * Node callback to call the update routine
     */
    class SliceContextUpdateCallback : public osg::NodeCallback
    {
    public: // NOLINT
        /**
         * Constructor.
         *
         * \param module just set the creating module as pointer for later reference.
         */
        explicit SliceContextUpdateCallback( WMSliceContext* module ): m_module( module )
        {
        };

        /**
         * operator () - called during the update traversal of OSG.
         *
         * \param node the osg node
         * \param nv the node visitor
         */
        virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

        /**
         * Pointer used to access members of the module to modify the node.
         */
        WMSliceContext* m_module;
    };
};

#endif  // WMSLICECONTEXT_H
