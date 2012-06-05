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

#include <osgUtil/CullVisitor>

#include "WGetMatrixCallback.h"

WGetMatrixCallback::WGetMatrixCallback():
    m_projectionMatrix( new WPVMatrix4X4( "Projection Matrix", "The OpenGL Projection Matrix.", WPVMatrix4X4::ValueType() ) ),
    m_modelViewMatrix( new WPVMatrix4X4( "ModelView Matrix", "The OpenGL Modelview Matrix.", WPVMatrix4X4::ValueType() ) )
{
    // initialize members
    m_projectionMatrix->setPurpose( PV_PURPOSE_INFORMATION );
    m_modelViewMatrix->setPurpose( PV_PURPOSE_INFORMATION );
}

WGetMatrixCallback::~WGetMatrixCallback()
{
    // cleanup
}

void WGetMatrixCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    // is this a cull visitor?
    osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
    if( cv )
    {
        // grab the matrices
        osg::Matrix mvMat = *( cv->getModelViewMatrix() );
        osg::Matrix projMat = *( cv->getProjectionMatrix() );

        // different from current?
        // We do this to avoid setting the same matrix if it is not needed. We therefore use some temporarily stored values instead of the
        // propertie's get() method to avoid locking
        if( mvMat != m_currentModelViewMatrix )
        {
            m_modelViewMatrix->set( mvMat );
            m_currentModelViewMatrix = mvMat;
        }
        if( projMat != m_currentProjectionMatrix )
        {
            m_projectionMatrix->set( projMat );
            m_currentProjectionMatrix = projMat;
        }
    }

    // allow nested callbacks to do their stuff and traverse the graph
    traverse( node, nv );
}

WPropMatrix4X4 WGetMatrixCallback::getProjectionMatrix() const
{
    return m_projectionMatrix;
}

WPropMatrix4X4 WGetMatrixCallback::getModelViewMatrix() const
{
    return m_modelViewMatrix;
}
