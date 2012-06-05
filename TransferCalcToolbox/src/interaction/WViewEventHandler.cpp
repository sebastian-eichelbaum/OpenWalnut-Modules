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

#include <core/common/WLogger.h>

#include "WViewEventHandler.h"

WViewEventHandler::WViewEventHandler():
    osgGA::GUIEventHandler(),
    m_wasDrag( false )
{
    // initialize members
}

WViewEventHandler::~WViewEventHandler()
{
    // cleanup
}

bool WViewEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // handle the interesting events
    switch( ea.getEventType() )
    {
    case osgGA::GUIEventAdapter::PUSH:
        break;
    case osgGA::GUIEventAdapter::RELEASE:
        // middle and right button initiates dragging. Store initial mouse coordinates
        if( ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
        {
            // if this was a drag, ignore it as onLeftClick only reacts on pure clicks
            if( !m_wasDrag )
            {
                m_onLeftClick( WVector2i( ea.getX(), ea.getY() ) );
            }
            m_wasDrag = false;
        }
        break;
    case osgGA::GUIEventAdapter::DOUBLECLICK:
        break;
    case osgGA::GUIEventAdapter::DRAG:
        m_wasDrag = true;
        break;
    default:
        break;
    }

    // in ALL cases, allow standard handling
    return osgGA::GUIEventHandler::handle( ea, aa );
}

boost::signals2::connection WViewEventHandler::onLeftClick( t_MouseHandlerType handler )
{
    return m_onLeftClick.connect( handler );
}

