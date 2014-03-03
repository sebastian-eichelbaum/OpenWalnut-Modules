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

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <osg/Geode>
#include <osg/LightModel>

#include "core/common/math/WMath.h"
#include "core/common/math/WPlane.h"
#include "core/common/WAssert.h"
#include "core/common/WColor.h"
#include "core/common/WHistogramBasic.h"
#include "core/graphicsEngine/WGEGeodeUtils.h"
#include "core/graphicsEngine/WGEGeometryUtils.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/kernel/WKernel.h"
#include "WMClusterSlicer.h"

WMClusterSlicer::WMClusterSlicer()
    : WModule(),
      m_rootNode( osg::ref_ptr< WGEManagedGroupNode > ( new WGEManagedGroupNode( m_active ) ) ),
      m_fullUpdate( new WCondition() ),
      m_maxMean( 0.0 ),
      m_minMean( 0.0 )
{
}

WMClusterSlicer::~WMClusterSlicer()
{
}

boost::shared_ptr< WModule > WMClusterSlicer::factory() const
{
    return boost::shared_ptr< WModule >( new WMClusterSlicer() );
}

const std::string WMClusterSlicer::getName() const
{
    return "Cluster Slicer";
}

const std::string WMClusterSlicer::getDescription() const
{
    return "Slices a cluster";
}

void WMClusterSlicer::connectors()
{
    m_fiberClusterIC = WModuleInputData< WFiberCluster >::createAndAdd( shared_from_this(), "clusterInput", "A cluster of fibers" );
    m_voxelizedClusterIC = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "clusterDSInput", "DataSet from cluster" );
    m_paramIC = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "paramInput", "DataSet of the parameters" );
    m_triangleMeshIC = WModuleInputData< WTriangleMesh >::createAndAdd( shared_from_this(), "meshInput", "TrianglMesh" );
    m_colorMapOC = WModuleOutputData< WColoredVertices >::createAndAdd( shared_from_this(), "colorMapOutput", "VertexID and colors" );
    m_triangleMeshOC = WModuleOutputData< WTriangleMesh >::createAndAdd( shared_from_this(), "meshOutput", "The Mesh to forward it for rendering" );

    WModule::connectors();
}

void WMClusterSlicer::properties()
{
    m_drawIsoVoxels     = m_properties->addProperty( "Show|Hide Iso Voxels", "Show|Hide voxels withing a given isosurface.", true );
    m_drawSlices        = m_properties->addProperty( "Show|Hide Slices", "Show|Hide slices along center line", false );
    m_isoValue          = m_properties->addProperty( "Iso value", "", 0.01 );
    m_meanSelector      = m_properties->addProperty( "Mean Type", "Selects the mean type, must be on of:"
                                                                   " 0==arithmetic, 1==geometric, 2==median", 2, m_fullUpdate );
    m_planeNumX         = m_properties->addProperty( "Planes #X-SamplePoints", "#samplePoints in first direction", 40, m_fullUpdate );
    m_planeNumY         = m_properties->addProperty( "Planes #Y-SamplePoints", "#samplePoints in second direction", 40, m_fullUpdate );
    m_planeStepWidth    = m_properties->addProperty( "Planes Step Width", "Distance between sample points", 0.5, m_fullUpdate );
    m_centerLineScale   = m_properties->addProperty( "#Planes", "Scales the center line to have more or less samples", 1.0, m_fullUpdate );
    m_selectBiggestComponentOnly = m_properties->addProperty( "Biggest Component Only",
       "If true, first the mesh is decomposed into its components (expensive!) and the biggest will be drawn", false );
    m_alternateColoring = m_properties->addProperty( "Alternate Mesh Coloring", "En/Disables the alternative mesh colorer", true, m_fullUpdate );

    m_meanSelector->setMin( 0 );
    m_meanSelector->setMax( 4 );
    m_isoValue->setMin( 0.0 );
    m_isoValue->setMax( 100.0 );
    m_planeNumX->setMin( 1 );
    m_planeNumY->setMin( 1 );
    m_planeStepWidth->setMin( 0.0 );
    m_customScale       = m_properties->addProperty( "Custom Scale", "", true, m_fullUpdate );
    m_minScale          = m_properties->addProperty( "MinScale", "Mean threshold below which is mapped to 0", 0.1, m_fullUpdate );
    m_maxScale          = m_properties->addProperty( "MaxScale", "Mean threshold above which is mapped to 1", 0.9, m_fullUpdate );
    m_minScale->setMin( 0.0 );
    m_minScale->setMax( 1.0 );
    m_maxScale->setMin( 0.0 );
    m_maxScale->setMax( 1.0 );
    m_minScaleColor     = m_properties->addProperty( "MinScaleColor", "", WColor( 1.0, 0.0, 0.0, 1.0 ), m_fullUpdate );
    m_maxScaleColor     = m_properties->addProperty( "MaxScaleColor", "", WColor( 1.0, 0.0, 0.0, 1.0 ), m_fullUpdate );

    WModule::properties();
}

void WMClusterSlicer::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_paramIC->getDataChangedCondition() );
    m_moduleState.add( m_triangleMeshIC->getDataChangedCondition() );
    m_moduleState.add( m_drawSlices->getCondition() );
    m_moduleState.add( m_drawIsoVoxels->getCondition() );
    m_moduleState.add( m_fullUpdate );
    m_moduleState.add( m_selectBiggestComponentOnly->getCondition() );

    ready();

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    while( !m_shutdownFlag() )
    {
        debugLog() << "Waiting...";
        m_moduleState.wait();

        if( m_shutdownFlag() )
        {
            break;
        }

        boost::shared_ptr< WDataSetScalar > newClusterDS = m_voxelizedClusterIC->getData();
        boost::shared_ptr< WFiberCluster >  newCluster   = m_fiberClusterIC->getData();
        boost::shared_ptr< WDataSetScalar > newParamDS   = m_paramIC->getData();
        boost::shared_ptr< WTriangleMesh > newMesh      = m_triangleMeshIC->getData();
        bool meshChanged = ( m_mesh != newMesh );
        bool paramDSChanged = ( m_paramDS != newParamDS );
        bool clusterChanged = ( m_cluster != newCluster );
        bool clusterDSChanged = ( m_clusterDS != newClusterDS );
        bool dataChanged = clusterDSChanged || clusterChanged || paramDSChanged || meshChanged;

        if( !( newClusterDS && newCluster && newParamDS && newMesh ) )
        {
            debugLog() << "Invalid data. Waiting for data change again.";
            continue;
        }

        m_clusterDS = newClusterDS;
        m_cluster = newCluster;
        m_paramDS = newParamDS;
        m_mesh = newMesh;

        if( dataChanged )
        {
            infoLog() << "Coverage for isovalue: " << m_isoValue->get() << " is: " << countTractPointsInsideVolume( m_isoValue->get() );
            // infoLog() << "Recommended isovalue for specified coverage: " << computeOptimalIsoValue();
        }

        if( dataChanged )
        {
            debugLog() << "Data changed...";
            if( meshChanged )
            {
                if( clusterDSChanged )
                {
                    debugLog() << "Building Join Tree";
                    m_joinTree = boost::shared_ptr< WJoinContourTree >( new WJoinContourTree( m_clusterDS ) );
                    m_joinTree->buildJoinTree();
                    debugLog() << "Finished building Join Tree";
                }
                // when the mesh has changed the isoValue must have had changed too
                m_isoVoxels = m_joinTree->getVolumeVoxelsEnclosedByIsoSurface( m_isoValue->get() );
                if( m_selectBiggestComponentOnly->get( true ) )
                {
                    debugLog() << "Start mesh decomposition";
                    m_components = tm_utils::componentDecomposition( *m_mesh );
                    debugLog() << "Decomposing mesh done";
                }
            }
        }

        if( meshChanged || paramDSChanged || m_meanSelector->changed() || m_planeNumX->changed() || m_alternateColoring->changed()
                        || m_planeNumY->changed() || m_planeStepWidth->changed() || m_centerLineScale->changed() || m_customScale->changed()
                        || m_minScale->changed() || m_maxScale->changed() || m_minScaleColor->changed() || m_maxScaleColor->changed() )
        {
            debugLog() << "Performing full update";
            generateSlices();
            sliceAndColorMesh( m_mesh );
            updateDisplay( true ); // force display update here (erasing invalid planes)
            debugLog() << "Full update done.";
        }
        else if( m_drawSlices->changed() || m_drawIsoVoxels->changed() )
        {
            updateDisplay();
        }
        else if( m_selectBiggestComponentOnly->changed() )
        {
            sliceAndColorMesh( m_mesh );
        }
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

WValue< double > WMClusterSlicer::meanParameter( boost::shared_ptr< std::set< WPosition > > samplePoints ) const
{
    std::vector< double > samples;
    samples.reserve( samplePoints->size() );

    boost::shared_ptr< WGridRegular3D > grid = boost::dynamic_pointer_cast< WGridRegular3D >( m_clusterDS->getGrid() );
    assert( grid != 0 );

    for( std::set< WPosition >::iterator pos = samplePoints->begin(); pos != samplePoints->end(); )
    {
        // ATTENTION: We don't interpolate in m_clusterDS since it might be the wrong component!!
        int id = grid->getVoxelNum( *pos );
        if( id >= 0 ) // check if the position is inside of the main component (biggest volume)
        {
            if( m_isoVoxels->count( static_cast< size_t >( id ) ) == 1 ) // check if its in main component
            {
                bool inParamGrid = false;
                double isoValue = m_clusterDS->interpolate( *pos, &inParamGrid );
                if( inParamGrid )
                {
                    if( isoValue > m_isoValue->get() )
                    {
                        inParamGrid = false;
                        double value = m_paramDS->interpolate( *pos, &inParamGrid );
                        if( inParamGrid ) // check if its in paramDS
                        {
                            samples.push_back( value );
                            ++pos;
                            continue;
                        }
                    }
                }
            }
        }
        samplePoints->erase( pos++ ); // erase in case the pos is not in main component or in paramDS or in clusterDS
    }
    double max = *std::max_element( samples.begin(), samples.end() );
    double min = *std::min_element( samples.begin(), samples.end() );

    double arithmeticMean = std::accumulate( samples.begin(), samples.end(), 0.0 );
    arithmeticMean = arithmeticMean / ( samples.empty() ? 1.0 : samples.size() );

    std::nth_element( samples.begin(), samples.begin() + samples.size() / 2, samples.end() );
    double median = ( samples.empty() ? 0.0 : samples[ samples.size() / 2 ] );

    // discard first all elements <= 0.0 since then the geometric mean does not make any sense

    std::vector< double >::iterator newEnd = std::remove_if( samples.begin(), samples.end(), std::bind2nd( std::less_equal< double >(), 0.0 ) );
    double geometricMean = std::accumulate( samples.begin(), newEnd, 1.0, std::multiplies< double >() );
    geometricMean = std::pow( geometricMean, ( samples.empty() ? 0.0 : 1.0 / samples.size() ) );

    WValue< double > result( 5 );
    result[0] = arithmeticMean;
    result[1] = geometricMean;
    result[2] = median;
    result[3] = max;
    result[4] = min;
    return result;
}

void WMClusterSlicer::generateSlices()
{
    debugLog() << "Generating Slices";
    WFiber centerLine( *m_cluster->getCenterLine() ); // copy the centerline
    if( centerLine.empty() )
    {
        errorLog() << "CenterLine of the bundle is empty => no slices are drawn";
        return;
    }
    centerLine.resampleByNumberOfPoints( static_cast< size_t >( m_centerLineScale->get( true ) * centerLine.size() ) );

    m_slices = boost::shared_ptr< std::vector< std::pair< double, WPlane > > >( new std::vector< std::pair< double, WPlane > > );
    m_maxMean = wlimits::MIN_DOUBLE;
    m_minMean = wlimits::MAX_DOUBLE;
    const int nbX = m_planeNumX->get( true );
    const int nbY = m_planeNumY->get( true );
    const double stepWidth = m_planeStepWidth->get( true );
    const int meanType = m_meanSelector->get( true );
    m_rootNode->remove( m_samplePointsGeode );
    m_samplePointsGeode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) ); // discard old geode

    WVector3d generator = ( centerLine.front() - midPoint( centerLine ) );
    generator = cross( generator, centerLine.back() - midPoint( centerLine ) );
    for( size_t i = 1; i < centerLine.size(); ++i )
    {
        WVector3d tangent = centerLine[i] - centerLine[i-1];
        WVector3d first = cross( tangent, generator );
        WVector3d second = cross( tangent, first );
        boost::shared_ptr< WPlane > p = boost::shared_ptr< WPlane >( new WPlane( tangent, centerLine[i-1], first, second ) );

        boost::shared_ptr< std::set< WPosition > > samplePoints = p->samplePoints( stepWidth, nbX, nbY  );
        double mean = meanParameter( samplePoints )[ meanType ];
        if( mean > m_maxMean )
        {
            m_maxMean = mean;
        }
        if( mean < m_minMean )
        {
            m_minMean = mean;
        }
        m_slices->push_back( std::make_pair( mean, WPlane( *p ) ) );
        if( m_drawSlices->get( true ) )
        {
            m_samplePointsGeode->insert( genPointBlobs( samplePoints, 0.1 ) );
        }
    }
    infoLog() << "Max-Mean: " << m_maxMean;
    infoLog() << "Min-Mean: " << m_minMean;
    if( m_drawSlices->get( true ) )
    {
        m_rootNode->insert( m_samplePointsGeode );
    }
}

/**
 * Compares two WTrianglesMeshes on their size of vertices. This is private here since it makes sense only to this module ATM.
 */
struct WMeshSizeComp
{
    /**
     * Comparator on num vertex of two WTriangleMeshes
     *
     * \param m First Mesh
     * \param n Second Mesh
     *
     * \return True if and only if the first Mesh has less vertices as the second mesh.
     */
    bool operator()( const boost::shared_ptr< WTriangleMesh >& m, const boost::shared_ptr< WTriangleMesh >& n ) const
    {
        return m->vertSize() < n->vertSize();
    }
};

void WMClusterSlicer::sliceAndColorMesh( boost::shared_ptr< WTriangleMesh > mesh )
{
    debugLog() << "Selecting mesh component...";
    boost::shared_ptr< WTriangleMesh > renderMesh = mesh;
    assert( renderMesh.get() );
    if( m_selectBiggestComponentOnly->get( true ) )
    {
        if( !m_components.get() )
        {
            debugLog() << "Start mesh decomposition";
            m_components = tm_utils::componentDecomposition( *renderMesh );
            debugLog() << "Decomposing mesh done";
        }
        renderMesh = *std::max_element( m_components->begin(), m_components->end(), WMeshSizeComp() );
    }

    debugLog() << "Mesh selected";

    if( renderMesh != m_triangleMeshOC->getData() )
    {
        m_triangleMeshOC->updateData( renderMesh );
    }

    debugLog() << "Building mesh color map...";
    m_colorMap = boost::shared_ptr< WColoredVertices >( new WColoredVertices );
    std::map< size_t, WColor > cmData;

    if( !m_alternateColoring->get( true ) )
    {
//        std::map< size_t, std::pair< double, int > > cm;
//
//        for( std::vector< std::pair< double, WPlane > >::const_iterator slice = m_slices->begin(); slice != m_slices->end(); ++slice )
//        {
//            boost::shared_ptr< std::set< size_t > > coloredVertices = tm_utils::intersection( *renderMesh, slice->second );
//            double scaledMean = mapMeanOntoScale( slice->first );
//            for( std::set< size_t >::const_iterator coloredVertex = coloredVertices->begin(); coloredVertex != coloredVertices->end(); ++coloredVertex ) // NOLINT
//            {
//                if( cm.find( *coloredVertex ) != cm.end() )
//                {
//                    cm[ *coloredVertex ].first += scaledMean;
//                    cm[ *coloredVertex ].second++;
//                }
//                else
//                {
//                    cm[ *coloredVertex ].first  = scaledMean;
//                    cm[ *coloredVertex ].second = 1;
//                }
//            }
//        }
//
//        for( std::map< size_t, std::pair< double, int > >::const_iterator vertexColor = cm.begin(); vertexColor != cm.end(); ++vertexColor )
//        {
//            cmData[ vertexColor->first ] = WColor( 0.0, vertexColor->second.first / vertexColor->second.second, 1.0, 1.0 );
//        }
    }
    else
    {
        osg::ref_ptr< osg::Vec3Array > vertices = renderMesh->getVertexArray();
        for( size_t i = 0; i < vertices->size(); ++i )
        {
            WAssert( m_slices->size() > 2, "We only support alternative coloring with more than 2 slices" );
            WPosition vertex( ( *vertices)[i].x(), ( *vertices)[i].y(), ( *vertices)[i].z() );
            std::vector< PlanePair > planePairs = computeNeighbouringPlanePairs( vertex );
            if( !planePairs.empty() )
            {
                PlanePair closestPlanes = closestPlanePair( planePairs, vertex );
                if( closestPlanes.first != 0 || closestPlanes.second != 0 ) // if(0,0) then it may be a boundary vertex
                {
                    cmData[ i ] = colorFromPlanePair( vertex, closestPlanes );
                }
            }
        }
    }

    m_colorMap->setData( cmData );
    debugLog() << "Done with color map building";
    m_colorMapOC->updateData( m_colorMap );
}

double WMClusterSlicer::mapMeanOntoScale( double meanValue ) const
{
    if( m_customScale->get( true ) )
    {
        if( meanValue < m_minScale->get( true ) )
        {
            return 0.0;
        }
        else if( meanValue > m_maxScale->get( true ) )
        {
            return 1.0;
        }
        else
        {
            return ( m_maxScale->get() == m_minScale->get() ? 0.0 : ( meanValue - m_minScale->get() ) / ( m_maxScale->get() - m_minScale->get() ) );
        }
    }
    else
    {
        return ( m_maxMean == m_minMean ? 0.0 : ( meanValue - m_minMean ) / ( m_maxMean - m_minMean ) );
    }
}

bool WMClusterSlicer::isInBetween( const WPosition& vertex, const PlanePair& pp ) const
{
    const WPlane& p = ( *m_slices )[ pp.first ].second;
    const WPlane& q = ( *m_slices )[ pp.second ].second;
    double r = dot( p.getNormal(), vertex - p.getPosition() );
    double s = dot( q.getNormal(), vertex - q.getPosition() );
    if( signum( r ) != signum( s ) )
    {
        return true;
    }
    return false;
}

std::vector< WMClusterSlicer::PlanePair > WMClusterSlicer::computeNeighbouringPlanePairs( const WPosition& vertex ) const
{
    // assume base point/origin of every plane is on center line
    std::vector< PlanePair > result;
    for( size_t i = 1; i < m_slices->size(); ++i )
    {
        if( isInBetween( vertex, std::make_pair( i, i-1 ) ) )
        {
            result.push_back( std::make_pair( i, i-1 ) );
        }
    }
    return result;
}

WMClusterSlicer::PlanePair WMClusterSlicer::closestPlanePair( const std::vector< PlanePair >& pairs, const WPosition& vertex ) const
{
    double minDistance = wlimits::MAX_DOUBLE;
    PlanePair result( 0, 0 );
    for( std::vector< PlanePair >::const_iterator pp = pairs.begin(); pp != pairs.end(); ++pp )
    {
        const WPlane& p = ( *m_slices )[ pp->first ].second;
        const WPlane& q = ( *m_slices )[ pp->second ].second;
        double sqDistance = std::min( length2( vertex - p.getPosition() ),  length2( vertex - q.getPosition() ) );
        if( minDistance > sqDistance )
        {
            minDistance = sqDistance;
            result = *pp;
        }
    }
    // check if coloring is necessary
    const WPlane& firstPlane = m_slices->front().second;
    const WPlane& lastPlane =  m_slices->back().second;

    double distanceToFirst = dot( firstPlane.getNormal(), vertex - firstPlane.getPosition() );
    double distanceToLast  = dot( lastPlane.getNormal(), vertex - lastPlane.getPosition() );
    if( ( distanceToFirst < 0 && length2( vertex - firstPlane.getPosition() ) < minDistance ) ||
        ( distanceToLast > 0 && length2( vertex - lastPlane.getPosition() ) < minDistance ) )
    {
        return std::make_pair( 0, 0 );
    }

    return result;
}

WColor WMClusterSlicer::colorFromPlanePair( const WPosition& vertex, const PlanePair& pp ) const
{
    const WPlane& p = ( *m_slices )[ pp.first ].second;
    const WPlane& q = ( *m_slices )[ pp.second ].second;
    double distanceToP = std::abs( dot( p.getNormal(), vertex - p.getPosition() ) );
    double distanceToQ = std::abs( dot( q.getNormal(), vertex - q.getPosition() ) );
    // std::cout << "distP, distQ: " << distanceToP << " " << distanceToQ << std::endl;
    double colorP = ( *m_slices )[ pp.first ].first;
    double colorQ = ( *m_slices )[ pp.second ].first;
    double vertexColor = colorQ * ( distanceToP / ( distanceToP + distanceToQ ) ) + colorP * ( distanceToQ / ( distanceToP + distanceToQ ) );
    // std::cout << "colorP, colorQ, vertexColor: " << colorP << " " << colorQ << " " << vertexColor << std::endl;
    return WColor( 0.0, mapMeanOntoScale( vertexColor ), 1.0, 1.0 );
}

void WMClusterSlicer::updateDisplay( bool force )
{
    debugLog() << "Forced updating display: " << force;
    if( m_drawIsoVoxels->changed() || force )
    {
        m_rootNode->remove( m_isoVoxelGeode );
        m_isoVoxelGeode = osg::ref_ptr< osg::Geode >( new osg::Geode() ); // discard old geode
        if( m_drawIsoVoxels->get( true ) )
        {
            assert( m_isoVoxels && "JoinTree cannot be valid since there is no valid m_clusterDS." );
            m_isoVoxelGeode = generateIsoVoxelGeode();
            m_rootNode->insert( m_isoVoxelGeode );
        }
    }

    if( m_drawSlices->changed() || force )
    {
        m_rootNode->remove( m_sliceGeode );

        generateSlices(); // for recomputation of sample point geode

        m_sliceGeode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) ); // discard old geode
        if( m_drawSlices->get( true ) ) // regenerate
        {
            const double width = m_planeNumX->get() * m_planeStepWidth->get();
            const double height = m_planeNumY->get() * m_planeStepWidth->get();
            for( std::vector< std::pair< double, WPlane > >::const_iterator cit = m_slices->begin(); cit != m_slices->end(); ++cit )
            {
                double scaledMean = mapMeanOntoScale( cit->first );
                WColor color( scaledMean, scaledMean, 1.0, 1.0 );
                m_sliceGeode->insert( wge::genFinitePlane( width, height, cit->second, color, true ) );
            }
            m_rootNode->insert( m_sliceGeode );
        }
    }
}

osg::ref_ptr< osg::Geode > WMClusterSlicer::generateIsoVoxelGeode() const
{
    boost::shared_ptr< std::set< WPosition > > points( new std::set< WPosition > );
    boost::shared_ptr< WGridRegular3D > grid = boost::dynamic_pointer_cast< WGridRegular3D >( m_clusterDS->getGrid() );
    assert( grid != 0 );
    for( std::set< size_t >::const_iterator cit = m_isoVoxels->begin(); cit != m_isoVoxels->end(); ++cit )
    {
        points->insert( grid->getPosition( *cit ) );
    }
    return genPointBlobs< std::set< WPosition > >( points, grid->getOffsetX() );
}


double WMClusterSlicer::countTractPointsInsideVolume( double isoValue ) const
{
    const std::list< size_t >& indices = m_cluster->getIndices();
    boost::shared_ptr< const WDataSetFiberVector > tracts = m_cluster->getDataSetReference();
    std::list< size_t >::const_iterator iter;
    size_t pointCounter = 0;
    size_t pointsInside = 0;
    for( iter = indices.begin(); iter != indices.end(); ++iter )
    {
        const WFiber& tract = ( *tracts )[ *iter ];
        WFiber::const_iterator pos;
        for( pos = tract.begin(); pos != tract.end(); ++pos, ++pointCounter )
        {
            bool inGrid = false;
            double value = m_clusterDS->interpolate( *pos, &inGrid );
            if( inGrid && value >= isoValue )
            {
                ++pointsInside;
            }
        }
    }
    return static_cast< double >( pointsInside ) / static_cast< double >( pointCounter );
}

double WMClusterSlicer::computeOptimalIsoValue( double coverage ) const
{
    const std::list< size_t >& indices = m_cluster->getIndices();
    boost::shared_ptr< const WDataSetFiberVector > tracts = m_cluster->getDataSetReference();
    std::list< size_t >::const_iterator iter;

    WHistogramBasic h( m_clusterDS->getMin(), m_clusterDS->getMax() );

    for( iter = indices.begin(); iter != indices.end(); ++iter )
    {
        const WFiber& tract = ( *tracts )[ *iter ];
        WFiber::const_iterator pos;
        for( pos = tract.begin(); pos != tract.end(); ++pos )
        {
            bool inGrid = false;
            double value = m_clusterDS->interpolate( *pos, &inGrid );
            if( inGrid )
            {
                h.insert( value );
            }
        }
    }
    size_t valuesSoFar = 0;
    int64_t index = h.size() - 1;
    while( static_cast< double >( valuesSoFar ) / static_cast< double >( h.valuesSize() ) < coverage && index >= 0 )
    {
        valuesSoFar += h[index--];
    }
    return  m_clusterDS->getMin() + std::abs( m_clusterDS->getMin() - m_clusterDS->getMax() ) * index / static_cast< double >( h.size() );
}
