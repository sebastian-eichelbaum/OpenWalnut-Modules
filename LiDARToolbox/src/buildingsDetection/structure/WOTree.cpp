//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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

#include <iostream>

#include "WOTree.h"

WOTree::WOTree( double detailDepth )
{
    m_root = new WONode( 0.0, 0.0, 0.0, 1.0 );
    m_detailLevel = detailDepth;
}

WOTree::~WOTree()
{
}

void WOTree::registerPoint( double x, double y, double z )
{
//    std::cout << "Inflating point: " << x << ", " << y << ", " << z << std::endl;
    while  ( !m_root->fitsIn( x, y, z ) )
        m_root->expand();

    WONode* node = m_root;
    while  ( node->getRadius() > m_detailLevel )
    {
        size_t drawer = node->getFittingCase( x, y, z );
        node->touchNode( drawer );
        node = node->getChild( drawer );
    }
}

boost::shared_ptr< WTriangleMesh > WOTree::getOutline()
{
    boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
    drawNode( m_root, tmpMesh );
    return tmpMesh;
}

void WOTree::drawNode( WONode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    if  ( node->getRadius() <= m_detailLevel )
    {
        size_t index = outputMesh->vertSize();
        for  ( size_t vertex = 0; vertex < 8; vertex++ )
        {
            double x = node->getCenter( 0 ) + node->getRadius() * ( static_cast<double>( WONode::vX[vertex] )*2.0 - 1.0 );
            double y = node->getCenter( 1 ) + node->getRadius() * ( static_cast<double>( WONode::vY[vertex] )*2.0 - 1.0 );
            double z = node->getCenter( 2 ) + node->getRadius() * ( static_cast<double>( WONode::vZ[vertex] )*2.0 - 1.0 );
            outputMesh->addVertex( x, y, z );
        }
        // Z = 0
        outputMesh->addTriangle( index + 0, index + 3, index + 1 );
        outputMesh->addTriangle( index + 2, index + 1, index + 3 );
        // X = 0
        outputMesh->addTriangle( index + 0, index + 4, index + 3 );
        outputMesh->addTriangle( index + 4, index + 7, index + 3 );
        // Y = 0
        outputMesh->addTriangle( index + 0, index + 1, index + 4 );
        outputMesh->addTriangle( index + 1, index + 5, index + 4 );
        // Z = 1
        outputMesh->addTriangle( index + 4, index + 5, index + 7 );
        outputMesh->addTriangle( index + 5, index + 6, index + 7 );
        // X = 1
        outputMesh->addTriangle( index + 1, index + 2, index + 5 );
        outputMesh->addTriangle( index + 2, index + 6, index + 5 );
        // Y = 1
        outputMesh->addTriangle( index + 3, index + 7, index + 2 );
        outputMesh->addTriangle( index + 7, index + 6, index + 2 );
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                drawNode( node->getChild( child ), outputMesh );
    }
}


