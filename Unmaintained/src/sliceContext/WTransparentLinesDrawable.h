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

#ifndef WTRANSPARENTLINESDRAWABLE_H
#define WTRANSPARENTLINESDRAWABLE_H

#include <osg/Geometry>
#include <osg/RenderInfo>

/**
 * A custom osg::Drawable that can render lines depth sorted. This will help when
 * rendering them transparently.
 */
class WTransparentLinesDrawable : public osg::Geometry
{
public:
    /**
     * The actual sorting before drawing is done here. This method calls deprecated_osg::Geometry::drawImplementation
     * \param renderInfo This is also not documented in OSG docu
     */
    virtual void drawImplementation( osg::RenderInfo &renderInfo ) const; //NOLINT
protected:
private:
};

#endif  // WTRANSPARENTLINESDRAWABLE_H
