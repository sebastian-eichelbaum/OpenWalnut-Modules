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

#ifndef WGETMATRIXCALLBACK_H
#define WGETMATRIXCALLBACK_H

#include <core/common/WProperties.h>

#include <osg/ref_ptr>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <osg/Node>


/**
 * This is a simple cull callback which grabs the view or projection matrix information for a certain node. It provides these matrices and camera
 * information via several properties. This allows you to also utilize camera properties in shaders (WGEPropertyUniform or
 * WGEShaderPropertyDefine).
 */
class WGetMatrixCallback: public osg::NodeCallback
{
public:
    /**
     * Reference counted pointer
     */
    typedef osg::ref_ptr< WGetMatrixCallback > RefPtr;

    /**
     * Reference counted pointer
     */
    typedef osg::ref_ptr< const WGetMatrixCallback > ConstRefPtr;

    /**
     * Default constructor.
     */
    WGetMatrixCallback();

    /**
     * Destructor.
     */
    virtual ~WGetMatrixCallback();

    /**
     * Operator called during each cull callback. This loads the matrix.
     *
     * \param node the node currently being processed
     * \param nv the visitor.
     */
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

    /**
     * Projection matrix property.
     *
     * \return the property.
     */
    WPropMatrix4X4 getProjectionMatrix() const;

    /**
     * Modelview matrix property.
     *
     * \return the property.
     */
    WPropMatrix4X4 getModelViewMatrix() const;

    /**
     * Viewport X.
     *
     * \return the property.
     */
    WPropDouble getViewportX() const;

    /**
     * Viewport Y.
     *
     * \return the property.
     */
    WPropDouble getViewportY() const;

    /**
     * Viewport width.
     *
     * \return the property.
     */
    WPropDouble getViewportWidth() const;

    /**
     * Viewport height.
     *
     * \return the property.
     */
    WPropDouble getViewportHeight() const;

protected:
private:
    /**
     * Current projection matrix.
     */
    WPropMatrix4X4 m_projectionMatrix;

    /**
     * Current ModelView matrix.
     */
    WPropMatrix4X4 m_modelViewMatrix;

    /**
     * The current modelview matrix. We keep this in this local variable although it already is stored in the appropriate property to avoid
     * unneeded locking during each frame.
     */
    osg::Matrix m_currentModelViewMatrix;

    /**
     * The current projection matrix. We keep this in this local variable although it already is stored in the appropriate property to avoid
     * unneeded locking during each frame.
     */
    osg::Matrix m_currentProjectionMatrix;

    /**
     * Width of the viewport.
     */
    WPropDouble m_viewportWidth;

    /**
     * Width of the viewport. Use this for faster testing of changes.
     */
    double m_currentViewportWidth;

    /**
     * Height of the viewport.
     */
    WPropDouble m_viewportHeight;

    /**
     * Height of the viewport. Use this for faster testing of changes.
     */
    double m_currentViewportHeight;

    /**
     * X coordinate of viewport.
     */
    WPropDouble m_viewportX;

    /**
     * X coordinate of viewport. Use this for faster testing of changes.
     */
    double m_currentViewportX;

    /**
     * Y coordinate of viewport.
     */
    WPropDouble m_viewportY;

    /**
     * Y coordinate of viewport. Use this for faster testing of changes.
     */
    double m_currentViewportY;
};

#endif  // WGETMATRIXCALLBACK_H
