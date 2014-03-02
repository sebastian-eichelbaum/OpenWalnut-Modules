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
#include "WMPointsCutOutliers.xpm"
#include "WMPointsCutOutliers.h"
#include "../datastructures/octree/WOctree.h"
#include "WCutOutliersDeamon.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMPointsCutOutliers )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMPointsCutOutliers::WMPointsCutOutliers():
    WModule(),
    m_propCondition( new WCondition() )
{
    m_tree = new WOctree( 0 );
}

WMPointsCutOutliers::~WMPointsCutOutliers()
{
}

boost::shared_ptr< WModule > WMPointsCutOutliers::factory() const
{
    return boost::shared_ptr< WModule >( new WMPointsCutOutliers() );
}

const char** WMPointsCutOutliers::getXPMIcon() const
{
    return WMPointsCutOutliers_xpm;
}
const std::string WMPointsCutOutliers::getName() const
{
    return "Points - Cut outliers";
}

const std::string WMPointsCutOutliers::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMPointsCutOutliers::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_output = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                        shared_from_this(), "points", "The loaded points." ) );

    addConnector( m_output );
//    addConnector( m_buildings );
    WModule::connectors();
}

void WMPointsCutOutliers::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.
    m_detailDepth = m_properties->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0, m_propCondition );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_properties->addProperty( "Detail Depth meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", 1.0, m_propCondition  );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );

    WModule::properties();
}

void WMPointsCutOutliers::requirements()
{
}

void WMPointsCutOutliers::moduleMain()
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

        boost::shared_ptr< WDataSetPoints > points = m_input->getData();
//        std::cout << "Execute cycle\r\n";
        m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) );
        if  ( points )
        {
            WCutOutliersDeamon groups = WCutOutliersDeamon();
            groups.setDetailDepth( m_detailDepthLabel->get( true ) );
            setProgressSettings( 3 );

            boost::shared_ptr< WDataSetPoints > cutPoints = groups.cutOutliers( points );
            m_output->updateData( cutPoints );

            m_progressStatus->finish();
        }

//        std::cout << "this is WOTree " << std::endl;

        // woke up since the module is requested to finish?
        if  ( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetPoints > points2 = m_input->getData();
        if  ( !points2 )
        {
            continue;
        }

        // ---> Insert code doing the real stuff here
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMPointsCutOutliers::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
