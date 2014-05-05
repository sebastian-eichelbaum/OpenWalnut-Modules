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
#include "WMWallDetectionByPCA.xpm"
#include "WMWallDetectionByPCA.h"
#include "WPCAWallDetector.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMWallDetectionByPCA )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMWallDetectionByPCA::WMWallDetectionByPCA():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMWallDetectionByPCA::~WMWallDetectionByPCA()
{
}

boost::shared_ptr< WModule > WMWallDetectionByPCA::factory() const
{
    return boost::shared_ptr< WModule >( new WMWallDetectionByPCA() );
}

const char** WMWallDetectionByPCA::getXPMIcon() const
{
    return WMWallDetectionByPCA_xpm;
}
const std::string WMWallDetectionByPCA::getName() const
{
    return "Wall Detection by PCA";
}

const std::string WMWallDetectionByPCA::getDescription() const
{
    return "Should draw values above some threshold.";
}

void WMWallDetectionByPCA::connectors()
{
    m_input = WModuleInputData< WDataSetPoints >::createAndAdd( shared_from_this(), "input", "The mesh to display" );

    m_outputTrimesh = boost::shared_ptr< WModuleOutputData< WTriangleMesh > >(
                new WModuleOutputData< WTriangleMesh >( shared_from_this(), "Variance intensity", "The loaded mesh." ) );

    addConnector( m_outputTrimesh );
//    addConnector( m_buildings );
    WModule::connectors();
}

void WMWallDetectionByPCA::properties()
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

    m_wallMaxAngleToNeighborVoxel = m_properties->addProperty( "Wall - max angle to voxel: ", "The maximal "
                            "angle between two connected voxels to detect as a single wall.", 5.0  );
    m_wallMaxAngleToNeighborVoxel->setMin( 0.0 );
    m_wallMaxAngleToNeighborVoxel->setMax( 45.0 );

    m_showedVarianceQuotientMax = m_properties->addProperty( "Variance - Quotient max.: ", "The maximal "
                            "that is displayed in the output triangle mesh using colors.", 0.5  );
    m_showedVarianceQuotientMax->setMin( 0.0 );
    m_showedVarianceQuotientMax->setMax( 1.0 );

    WModule::properties();
}

void WMWallDetectionByPCA::requirements()
{
}

void WMWallDetectionByPCA::moduleMain()
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
        if  ( points )
        {
            WDataSetPoints::VertexArray inputVerts = points->getVertices();
            size_t count = inputVerts->size()/3;
            setProgressSettings( count );

            m_detailDepthLabel->set( pow( 2.0, m_detailDepth->get() ) );
            WWallDetectOctree* pcaAnalysis = new WWallDetectOctree( m_detailDepthLabel->get() );
            pcaAnalysis->setWallMaxAngleToNeighborVoxel( m_wallMaxAngleToNeighborVoxel->get() );

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
            pcaAnalysis->setMaxIsotropicThresholdForVoxelMerge( m_showedVarianceQuotientMax->get() );

            WPCAWallDetector detector( pcaAnalysis, m_progressStatus );
            detector.analyze();
            pcaAnalysis->groupNeighbourLeafsFromRoot();
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
void WMWallDetectionByPCA::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
