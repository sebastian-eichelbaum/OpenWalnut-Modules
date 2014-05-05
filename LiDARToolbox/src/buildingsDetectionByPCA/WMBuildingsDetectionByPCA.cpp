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
#include "WMBuildingsDetectionByPCA.xpm"
#include "WMBuildingsDetectionByPCA.h"
#include "WPCADetector.h"
#include "../datastructures/octree/WOctree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMBuildingsDetectionByPCA )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMBuildingsDetectionByPCA::WMBuildingsDetectionByPCA():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMBuildingsDetectionByPCA::~WMBuildingsDetectionByPCA()
{
}

boost::shared_ptr< WModule > WMBuildingsDetectionByPCA::factory() const
{
    return boost::shared_ptr< WModule >( new WMBuildingsDetectionByPCA() );
}

const char** WMBuildingsDetectionByPCA::getXPMIcon() const
{
    return WMBuildingsDetectionByPCA_xpm;
}
const std::string WMBuildingsDetectionByPCA::getName() const
{
    return "Buildings Detection by PCA";
}

const std::string WMBuildingsDetectionByPCA::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMBuildingsDetectionByPCA::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_outputTrimesh = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
                new WModuleOutputData< WTriangleMesh >( shared_from_this(), "Variance intensity", "The loaded mesh." ) );

    addConnector( m_outputTrimesh );
//    addConnector( m_buildings );
    WModule::connectors();
}

void WMBuildingsDetectionByPCA::properties()
{
    // ---> Put the code for your properties here. See "src/modules/template/" for an extensively documented example.

    m_reloadData = m_properties->addProperty( "Reload data:",  "Execute", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );
    m_detailDepth = m_properties->addProperty( "Detail Depth 2^n m: ", "Resulting 2^n meters detail "
                            "depth for the octree search tree.", 0 );
    m_detailDepth->setMin( -3 );
    m_detailDepth->setMax( 4 );
    m_detailDepthLabel = m_properties->addProperty( "Detail Depth meters: ", "Resulting detail depth "
                            "in meters for the octree search tree.", 1.0  );
    m_detailDepthLabel->setPurpose( PV_PURPOSE_INFORMATION );

    m_showedIsotropicThresholdMin = m_properties->addProperty( "Eigen Value Quot. min.: ", "The minimal "
                            "that is displayed in the output triangle mesh using colors.", 0.0 );
    m_showedIsotropicThresholdMin->setMin( 0.0 );
    m_showedIsotropicThresholdMin->setMax( 0.998 );
    m_showedIsotropicThresholdMax = m_properties->addProperty( "Eigen Value Quot. max.: ", "The maximal "
                            "that is displayed in the output triangle mesh using colors.", 1.0  );
    m_showedIsotropicThresholdMax->setMin( 0.002 );
    m_showedIsotropicThresholdMax->setMax( 1.0 );

    m_maximalEigenValueQuotientToDraw = m_properties->addProperty( "Max. Eig. Quot. to draw: ",
                            "Maximal Eigen Value quotient of voxel's points to draw that area.", 1.0 );
    m_maximalEigenValueQuotientToDraw->setMin( 0.0 );
    m_maximalEigenValueQuotientToDraw->setMax( 1.0 );

    m_minPointsPerVoxelToDraw = m_properties->addProperty( "Min. points per voxel to draw: ",
                            "The minimal points per voxel that enables an area to draw.", 1 );
    m_minPointsPerVoxelToDraw->setMin( 1 );


    WModule::properties();
}

void WMBuildingsDetectionByPCA::requirements()
{
}

void WMBuildingsDetectionByPCA::moduleMain()
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

        m_showedIsotropicThresholdMax->setMin( m_showedIsotropicThresholdMin->get() + 0.002 );
        boost::shared_ptr< WDataSetPoints > points = m_input->getData();
//        std::cout << "Execute cycle\r\n";
        if  ( points )
        {
            WDataSetPoints::VertexArray inputVerts = points->getVertices();
            size_t count = inputVerts->size()/3;
            setProgressSettings( count );

            m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) );
            WOctree* pcaAnalysis = new WOctree( m_detailDepthLabel->get(), new WPcaDetectOctNode() );

            boost::shared_ptr< WTriangleMesh > tmpMesh( new WTriangleMesh( 0, 0 ) );
            for  ( size_t vertex = 0; vertex < count; vertex++)
            {
                m_progressStatus->increment( 1 );
                float x = inputVerts->at( vertex*3 );
                float y = inputVerts->at( vertex*3+1 );
                float z = inputVerts->at( vertex*3+2 );

                pcaAnalysis->registerPoint( x, y, z );
            }
            setProgressSettings( pcaAnalysis->getRootNode()->getTotalNodeCount() );

            WPCADetector detector( pcaAnalysis, m_progressStatus );
            detector.setDisplayedVarianceQuotientRange( m_showedIsotropicThresholdMin->get(), m_showedIsotropicThresholdMax->get() );
            detector.setMaximalEigenValueQuotientToDraw( m_maximalEigenValueQuotientToDraw->get() );
            detector.setMinPointsPerVoxelToDraw( m_minPointsPerVoxelToDraw->get() );
            detector.analyze();
            m_outputTrimesh->updateData( detector.getOutline() );

            m_progressStatus->finish();
        }
        m_reloadData->set( WPVBaseTypes::PV_TRIGGER_READY, true );
        m_reloadData->get( true );


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
void WMBuildingsDetectionByPCA::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
