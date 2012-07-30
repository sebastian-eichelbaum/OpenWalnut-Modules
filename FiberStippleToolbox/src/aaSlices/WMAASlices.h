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

#ifndef WMAASLICES_H
#define WMAASLICES_H

#include <string>

#include <osgManipulator/Dragger>
#include <osgManipulator/Command>
#include <osgManipulator/Translate1DDragger>

#include "core/graphicsEngine/WPickInfo.h"
#include "core/kernel/WModule.h"

// forward declarations to reduce compile dependencies
template< class T > class WModuleInputData;
class WGEManagedGroupNode;

/**
 * Interactive axis aligned planes (aka axial-, sagittal- or coronal slices).
 * Serves as selection principle to other modules.
 *
 * \ingroup modules
 */
class WMAASlices: public WModule
{
public:
    /**
     * Default constructor.
     */
    WMAASlices();

    /**
     * Destructor.
     */
    virtual ~WMAASlices();

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
    class PositionChangedCallback : public osgManipulator::DraggerCallback
    {
    public:
        Klaus( WPropDouble pos, size_t axis, osgManipulator::Translate1DDragger* d ):
            m_dragger( d ),
            m_pos( pos ),
            m_axis( axis )
        {
        }

        /**
         * Receive motion commands. Returns true on success.
         */
        virtual bool receive( const osgManipulator::MotionCommand& m)
        {
            // OSG_NOTICE<<"MotionCommand:" << m.getMotionMatrix() << std::endl;
            double newPos = m_dragger->getMatrix()( 3, m_axis );
            if( newPos < m_pos->getMin()->getMin() || newPos > m_pos->getMax()->getMax() )
            {
                return false;
            }
            else
            {
                m_pos->set( newPos );
                return true;
            }
        }

    private:
        osgManipulator::Translate1DDragger* m_dragger;
        WPropDouble m_pos;
        size_t m_axis;
    };

    /**
     * Initialize OSG root node for this module. All other nodes from this module should be attached to this root node.
     */
    void initOSG();

    boost::array< WPropBool, 3 > m_showSlice;

    boost::array< WPropDouble, 3 > m_pos;

    boost::array< WPropColor, 3 > m_color;

    /**
     * The OSG root node for this module. All other geodes or OSG nodes will be attached on this single node.
     */
    osg::ref_ptr< WGEManagedGroupNode > m_output;

    bool m_first;
};

#endif  // WMAASLICES_H
