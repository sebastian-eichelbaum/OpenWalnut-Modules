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

//#include <map>
#include <string>
//#include <vector>

#include <osg/Node>
#include <osg/Geode>
#include <osg/Uniform>

#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"

//#include "core/common/math/linearAlgebra/WLinearAlgebra.h"

#include "core/dataHandler/WDataSetScalar.h"

/**
 * Operators for processing two WDataSetScalar.
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
     * An input connector used to get datasets from other modules.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > >  m_inputData;

    /**
     * A condition used to notify about changes in several properties.
     */
    boost::shared_ptr< WCondition > m_propCondition;

    /**
     * The iso value to be rendered.
     */
    WPropDouble   m_isoValue;
    
    /**
     * Epsilon value to define the range for the iso value.
     */
    WPropDouble   m_epsilon;
    
    /**
     * A color.
     */
    WPropColor    m_color;
    
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

};

#endif  // WMTRANSFERCALC_H
