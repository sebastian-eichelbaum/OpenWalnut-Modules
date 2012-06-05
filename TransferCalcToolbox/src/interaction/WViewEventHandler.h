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

#ifndef WVIEWEVENTHANDLER_H
#define WVIEWEVENTHANDLER_H

#include <boost/signals2/signal.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <osg/ref_ptr>

#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIEventHandler>

#include <core/common/math/linearAlgebra/WVectorFixed.h>

/**
 * Class to handle events in a WGEView/widget.
 */
class WViewEventHandler: public osgGA::GUIEventHandler
{
public:

    /**
     * Convenience typedef for a osg::ref_ptr< WViewEventHandler >.
     */
    typedef osg::ref_ptr< WViewEventHandler > RefPtr;

    /**
     * Convenience typedef for a osg::ref_ptr< const WViewEventHandler >.
     */
    typedef osg::ref_ptr< const WViewEventHandler > ConstRefPtr;

    /**
     * Default constructor.
     */
    WViewEventHandler();

    /**
     * Destructor.
     */
    virtual ~WViewEventHandler();

    /**
     * Handle incoming events.
     *
     * \param ea event class for storing keyboard, mouse and window events
     * \param aa the action adapter, allowing events to trigger GUI operations
     *
     * \return true if handled, false otherwise
     */
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    /**
     * Generic handler for mouse events associated with a two dimensional coordinate.
     */
    typedef boost::function< void ( WVector2i ) > t_MouseHandlerType;

    /**
     * Add a handler for mouse click events.
     *
     * \param handler the function called
     *
     * \return the connection. Keep this and disconnect if you do not need it anymore!
     */
    boost::signals2::connection onLeftClick( t_MouseHandlerType handler );
protected:
private:

    /**
     * The signal for mouse events associated with a two dimensional coordinate.
     */
    typedef boost::signals2::signal< void ( WVector2i ) > t_MouseHandlerSignalType;

    /**
     * Signal for left click events.
     */
    t_MouseHandlerSignalType m_onLeftClick;

    /**
     * If true, the next release will be after a drag event
     */
    bool m_wasDrag;
};

#endif  // WVIEWEVENTHANDLER_H

