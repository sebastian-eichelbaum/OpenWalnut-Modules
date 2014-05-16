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

#include <iostream>
#include <vector>
#include "WPCADetector.h"

WPCADetector::WPCADetector( WOctree* analyzableOctree, boost::shared_ptr< WProgress > progressStatus )
{
    m_analyzableOctree = analyzableOctree;
    m_progressStatus = progressStatus;
}

WPCADetector::~WPCADetector()
{
}
void WPCADetector::setDisplayedVarianceQuotientRange( double showedIsotropicThresholdMin, double showedIsotropicThresholdMax )
{
    m_showedisotropicThresholdMin = showedIsotropicThresholdMin;
    m_showedisotropicThresholdMax = showedIsotropicThresholdMax;
}
void WPCADetector::setMaximalEigenValueQuotientToDraw( double maximalEigenValueQuotientToDraw )
{
    m_maximalEigenValueQuotientToDraw = maximalEigenValueQuotientToDraw;
}
void WPCADetector::setMinPointsPerVoxelToDraw( size_t minPointsPerVoxelToDraw )
{
    m_minPointsPerVoxelToDraw = minPointsPerVoxelToDraw;
}
void WPCADetector::analyze()
{
    analyzeNode( ( WPcaDetectOctNode* )( m_analyzableOctree->getRootNode() ) );
}
void WPCADetector::analyzeNode( WPcaDetectOctNode* node )
{
    m_progressStatus->increment( 1 );
    if  ( node->getRadius() <= m_analyzableOctree->getDetailLevel() )
    {
        if(node->getPointCount() >= 3 )
        {
            WPrincipalComponentAnalysis pca;
            pca.analyzeData( node->getInputPoints() );
            vector<double> eigenValues = pca.getEigenValues();
            if( eigenValues[0] > 0.0 )
                node->setEigenValueQuotient( eigenValues[2] / eigenValues[0] );
        }
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                analyzeNode( ( WPcaDetectOctNode* )( node->getChild( child ) ) );
    }
}
boost::shared_ptr< WTriangleMesh > WPCADetector::getOutline()
{
    boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
    drawNode( ( WPcaDetectOctNode* )( m_analyzableOctree->getRootNode() ), tmpMesh );
    return tmpMesh;
}

void WPCADetector::drawNode( WPcaDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    if  ( node->getRadius() <= m_analyzableOctree->getDetailLevel() )
    {
        if( node->getEigenValueQuotient() > m_maximalEigenValueQuotientToDraw )
            return;
        if( node->getPointCount() < m_minPointsPerVoxelToDraw )
            return;

        drawLeafNode( node, outputMesh );
    }
    else
    {
        for  ( int child = 0; child < 8; child++ )
            if  ( node->getChild( child ) != 0 )
                drawNode( ( WPcaDetectOctNode* )(node->getChild( child ) ), outputMesh );
    }
}
void WPCADetector::drawLeafNode( WPcaDetectOctNode* node, boost::shared_ptr< WTriangleMesh > outputMesh )
{
    size_t index = outputMesh->vertSize();
    for( size_t vertex = 0; vertex <= 8; vertex++ )
    {
        double iX = vertex % 2;
        double iY = ( vertex / 2 ) % 2;
        double iZ = ( vertex / 4 ) % 2;
        double x = node->getCenter( 0 ) + node->getRadius() * ( iX * 2.0 - 1.0 );
        double y = node->getCenter( 1 ) + node->getRadius() * ( iY * 2.0 - 1.0 );
        double z = node->getCenter( 2 ) + node->getRadius() * ( iZ * 2.0 - 1.0 );
        outputMesh->addVertex( x, y, z );
        outputMesh->setVertexColor( index+vertex, calculateColorForNode( node ) );
    }
    // Z = 0
    outputMesh->addTriangle( index + 0, index + 2, index + 1 );
    outputMesh->addTriangle( index + 3, index + 1, index + 2 );
    // X = 0
    outputMesh->addTriangle( index + 0, index + 4, index + 2 );
    outputMesh->addTriangle( index + 4, index + 6, index + 2 );
    // Y = 0
    outputMesh->addTriangle( index + 0, index + 1, index + 4 );
    outputMesh->addTriangle( index + 1, index + 5, index + 4 );
    // Z = 1
    outputMesh->addTriangle( index + 4, index + 5, index + 6 );
    outputMesh->addTriangle( index + 5, index + 7, index + 6 );
    // X = 1
    outputMesh->addTriangle( index + 1, index + 3, index + 5 );
    outputMesh->addTriangle( index + 3, index + 7, index + 5 );
    // Y = 1
    outputMesh->addTriangle( index + 2, index + 6, index + 3 );
    outputMesh->addTriangle( index + 6, index + 7, index + 3 );
}
osg::Vec4 WPCADetector::calculateColorForNode( WPcaDetectOctNode* node )
{
    if( node->hasEigenValueQuotient() )
    {
        double quotient = node->getEigenValueQuotient();
        quotient = ( quotient - m_showedisotropicThresholdMin ) /
                ( m_showedisotropicThresholdMax - m_showedisotropicThresholdMin );
        if( quotient < 0.0 ) quotient = 0.0;
        if( quotient > 1.0 ) quotient = 1.0;
        return osg::Vec4( quotient, ( 1.0 - quotient )*0.7, 1.0 - quotient, 1.0 );
    }
    else
    {
        return osg::Vec4( 0.3, 0.3, 0.3, 1 );
    }
}

