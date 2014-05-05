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

#include <string>

#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include <vector>

#include <osg/Geometry>
#include "core/kernel/WModule.h"

#include "core/dataHandler/WDataSetScalar.h"
#include "core/graphicsEngine/callbacks/WGELinearTranslationCallback.h"
#include "core/graphicsEngine/shaders/WGEPropertyUniform.h"
#include "core/graphicsEngine/shaders/WGEShader.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WModuleInputData.h"
#include "WMPointsGroupSelector.xpm"
#include "WMPointsGroupSelector.h"
#include "WVoxelOutliner.h"
#include "../datastructures/octree/WOctree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMPointsGroupSelector )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMPointsGroupSelector::WMPointsGroupSelector():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMPointsGroupSelector::~WMPointsGroupSelector()
{
}

boost::shared_ptr< WModule > WMPointsGroupSelector::factory() const
{
    return boost::shared_ptr< WModule >( new WMPointsGroupSelector() );
}

const char** WMPointsGroupSelector::getXPMIcon() const
{
    return WMPointsGroupSelector_xpm;
}
const std::string WMPointsGroupSelector::getName() const
{
    return "Points - Group Selector";
}

const std::string WMPointsGroupSelector::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMPointsGroupSelector::connectors()
{
    m_input = WModuleInputData< WDataSetPointsGrouped >::createAndAdd( shared_from_this(),
            "Grouped data set points input.", "Brouped points that can be observed separately." );

    m_outputTrimesh = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
                new WModuleOutputData< WTriangleMesh >( shared_from_this(),
                "Triangle mesh output.", "Either cubes or tetraeders that depict buildings." ) );

    m_outputPoints = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >( shared_from_this(),
                "Data set points output.", "Data set points of groups." ) );

    addConnector( m_outputTrimesh );
    addConnector( m_outputPoints );
    WModule::connectors();
}

void WMPointsGroupSelector::properties()
{
    m_nbPoints = m_infoProperties->addProperty( "Points: ", "Input points count.", 0 );
    m_xMin = m_infoProperties->addProperty( "X min.: ", "Minimal x coordinate of all input points.", 0.0 );
    m_xMax = m_infoProperties->addProperty( "X max.: ", "Maximal x coordinate of all input points.", 0.0 );
    m_yMin = m_infoProperties->addProperty( "Y min.: ", "Minimal y coordinate of all input points.", 0.0 );
    m_yMax = m_infoProperties->addProperty( "Y max.: ", "Maximal y coordinate of all input points.", 0.0 );
    m_zMin = m_infoProperties->addProperty( "Z min.: ", "Minimal z coordinate of all input points.", 0.0 );
    m_zMax = m_infoProperties->addProperty( "Z max.: ", "Maximal z coordinate of all input points.", 0.0 );


    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.

    m_stubSize = m_properties->addProperty( "Stub size: ",
                            "Size of tetraeders that are used to depict points.", 0.6 );
    m_stubSize->setMin( 0.0 );
    m_stubSize->setMax( 3.0 );
    m_contrast = m_properties->addProperty( "Contrast: ",
                            "This is the value that multiplies the input colors before assigning to the output. "
                            "Note that the output has the range between 0.0 and 1.0.\r\nHint: Look ath the intensity "
                            "maximum param in the information tab of the ReadLAS plugin.", 0.005, m_propCondition );
    m_detailDepth = m_properties->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0, m_propCondition );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_properties->addProperty( "Detail Depth meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", 1.0  );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );
    m_showTetraedersInsteadOfOctreeCubes = m_properties->addProperty( "Tetraeders instead of cubes: ",
                            "Depicting the input data set points showing the point outline "
                            "instead of regions depicted as cubes that cover existing points. "
                            "Enabling this option you must have 32GB RAM depicting a 400MB"
                            "las file.", false, m_propCondition );
    m_highlightUsingColors = m_properties->addProperty( "Hilight using colors: ",
                            "Hilights output ddata groups using colors.", true, m_propCondition );

    m_selectedShowableBuilding = m_properties->addProperty( "Showable building idx: ",
            "Index of the showable building. 0 means all "
            "buildings without the ground. Other numbers are all each other buildings in "
            "one single piece", 0, m_propCondition );
    m_selectedShowableBuilding->setMin( 0 );
    m_selectedShowableBuilding->setMax( 1 );


    WModule::properties();
}

void WMPointsGroupSelector::requirements()
{
}

void WMPointsGroupSelector::moduleMain()
{
    infoLog() << "Thrsholding example main routine started";

    // get notified about data changes
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_input->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    // main loop
    while( !m_shutdownFlag() )
    {
        //infoLog() << "Waiting ...";
        m_moduleState.wait();

        boost::shared_ptr< WDataSetPointsGrouped > points = m_input->getData();
//        std::cout << "Execute cycle\r\n";
        if  ( points )
        {
            WDataSetPointsGrouped::VertexArray verts = points->getVertices();
            WDataSetPointsGrouped::ColorArray colors = points->getColors();
            WDataSetPointsGrouped::GroupArray groups = points->getGroups();
            WDataSetPoints::VertexArray outputVertices( new WDataSetPoints::VertexArray::element_type() );
            WDataSetPoints::ColorArray outputColors( new WDataSetPoints::ColorArray::element_type() );
            size_t count = verts->size()/3;
            setProgressSettings( count );

            size_t groupCount = 0;
            for( size_t index = 0; index < groups->size(); index++ )
                if( groups->at( index ) + 1 > groupCount )
                    groupCount = groups->at( index );
            m_selectedShowableBuilding->setMax( groupCount + 0 );
            size_t selectedGroup = m_selectedShowableBuilding->get();

            m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) );
            WOctree* m_tree = new WOctree( m_detailDepthLabel->get() );

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            float a = m_stubSize->get();
            float contrast = m_contrast->get();
            for  ( size_t vertex = 0; vertex < count; vertex++)
            {
                float x = verts->at( vertex*3 );
                float y = verts->at( vertex*3+1 );
                float z = verts->at( vertex*3+2 );

                size_t group = groups->at( vertex );
                if( selectedGroup == 0 || selectedGroup == group + 1 )
                {
                    float r = colors->at( vertex*3 );
                    float g = colors->at( vertex*3+1 );
                    float b = colors->at( vertex*3+2 );
                    if( m_highlightUsingColors->get() )
                    {
                        r *= WOctree::calcColor( group, 0 );
                        g *= WOctree::calcColor( group, 1 );
                        b *= WOctree::calcColor( group, 2 );
                    }
                    outputVertices->push_back( x );
                    outputVertices->push_back( y );
                    outputVertices->push_back( z );
                    outputColors->push_back( r*contrast );
                    outputColors->push_back( g*contrast );
                    outputColors->push_back( b*contrast );
                    if( m_showTetraedersInsteadOfOctreeCubes->get() )
                    {
                        osg::Vec4f* color = new osg::Vec4f( r*contrast,
                                g*contrast, b*contrast, 1.0f );
                        tmpMesh->addVertex( 0+x, 0+y, 0+z );
                        tmpMesh->addVertex( a+x, 0+y, 0+z );
                        tmpMesh->addVertex( 0+x, a+y, 0+z );
                        tmpMesh->addVertex( 0+x, 0+y, a+z );
                        size_t body = vertex*4;
                        tmpMesh->addTriangle( 0+body, 2+body, 1+body );
                        tmpMesh->addTriangle( 0+body, 1+body, 3+body );
                        tmpMesh->addTriangle( 0+body, 3+body, 2+body );
                        tmpMesh->addTriangle( 1+body, 2+body, 3+body );
                        tmpMesh->setVertexColor( body, *color );
                        tmpMesh->setVertexColor( body+1, *color );
                        tmpMesh->setVertexColor( body+2, *color );
                        tmpMesh->setVertexColor( body+3, *color );
                    }
                    m_progressStatus->increment( 1 );
                    m_tree->registerPoint( x, y, z );
                    WOctNode* voxel = m_tree->getLeafNode( x, y, z );
                    voxel->setGroupNr( group );
                }
            }
            m_tree->groupNeighbourLeafsFromRoot();
            if  ( outputVertices->size() == 0 ) //TODO(aschwarzkopf): Handle the problem in other way. When no points exist then the program crashes.
                for  ( size_t lfd = 0; lfd < 3; lfd++)
                {
                    outputVertices->push_back( 0 );
                    outputColors->push_back( 0 );
                }
            boost::shared_ptr< WDataSetPoints > outputPoints( new WDataSetPoints( outputVertices, outputColors ) );
            m_outputPoints->updateData( outputPoints );
            m_outputTrimesh->updateData( m_showTetraedersInsteadOfOctreeCubes->get( true )
                    ?tmpMesh :WVoxelOutliner::getOutline( m_tree, m_highlightUsingColors->get() ) );
            m_nbPoints->set( count );
            m_xMin->set( m_tree->getRootNode()->getXMin() );
            m_xMax->set( m_tree->getRootNode()->getXMax() );
            m_yMin->set( m_tree->getRootNode()->getYMin() );
            m_yMax->set( m_tree->getRootNode()->getYMax() );
            m_zMin->set( m_tree->getRootNode()->getZMin() );
            m_zMax->set( m_tree->getRootNode()->getZMax() );
            m_progressStatus->finish();
        }


        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPointsGrouped > points2 = m_input->getData();
        if  ( !points2 )
        {
            continue;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMPointsGroupSelector::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
