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

// Some rules to the inclusion of headers:
//  * Ordering:
//    * C Headers
//    * C++ Standard headers
//    * External Lib headers (like OSG or Boost headers)
//    * headers of other classes inside OpenWalnut
//    * your own header file

#include <string>
#include <vector>
#include <algorithm>

#include <osg/ShapeDrawable>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Material>
#include <osg/StateAttribute>
#include <osg/Vec3>

#include "core/kernel/WKernel.h"
#include "core/common/WColor.h"
#include "core/common/WPathHelper.h"
#include "core/common/WPropertyHelper.h"
#include "core/common/math/WMatrix.h"
#include "core/common/math/linearAlgebra/WLinearAlgebra.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/graphicsEngine/WGERequirement.h"

#include "WMTransferCalc.xpm"
#include "WMTransferCalc.h"

WMTransferCalc::WMTransferCalc():
    WModule()
{
    // In the constructor, you can initialize your members and all this stuff. You must not initialize connectors or properties here! You also
    // should avoid doing computationally expensive stuff, since every module has its own thread which is intended to be used for such calculations.
    // Please keep in mind, that every member initialized here is also initialized in the prototype, which may be a problem if the member is large,
    // and therefore, wasting a lot of memory in your module's prototype instance.
}

WMTransferCalc::~WMTransferCalc()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMTransferCalc::factory() const
{
    return boost::shared_ptr< WModule >( new WMTransferCalc() );
}

const char** WMTransferCalc::getXPMIcon() const
{
    return WMTransferCalc_xpm;
}

const std::string WMTransferCalc::getName() const
{
    return "Transfer Function Calculation";
}

const std::string WMTransferCalc::getDescription() const
{
    return "Will calculate optimal settings for the Transfer Function after klicking within the data.";
}

void WMTransferCalc::connectors()
{
    m_inputData = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "givenData", "Data which will be checked for displaying." );

    // call WModules initialization
    WModule::connectors();
}

void WMTransferCalc::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition() );
    m_color         = m_properties->addProperty(  "Color", "Color of the orbs.", WColor( 1.0, 0.0, 0.0, 1.0 ), m_propCondition );

    m_xPos          = m_properties->addProperty( "X position", "x coordinate of the ray origin", 0, m_propCondition );
    m_yPos          = m_properties->addProperty( "Y position", "y coordinate of the ray origin", 0, m_propCondition );

    WModule::properties();
}

void WMTransferCalc::requirements()
{
    m_requirements.push_back( new WGERequirement() );
}

void WMTransferCalc::moduleMain()
{
    debugLog() << "moduleMain started...";

    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_inputData->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();
    debugLog() << "Module is now ready.";

    m_rootNode = new WGEManagedGroupNode( m_active );
    //m_rootNode->addUpdateCallback( new TranslateCallback( this ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );


    debugLog() << "Entering main loop";
    while( !m_shutdownFlag() )
    {
        debugLog() << "Waiting ...";
        m_moduleState.wait();

        // woke up since the module is requested to finish
        if( m_shutdownFlag() )
        {
            debugLog() << "Closing module...";
            break;
        }

        bool dataUpdated = m_inputData->updated();
        bool dataValid = ( m_dataSet );

        if( dataUpdated )
        {
            if( m_dataSet != m_inputData->getData() )
            {
                m_dataSet = m_inputData->getData();
                dataValid = ( m_dataSet );
                debugLog() << "New Data!";

                if( !dataValid )
                {
                    debugLog() << "No valid data anymore. Cleaning up.";
                    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
                }

                // grab the grid
                m_grid = boost::shared_dynamic_cast< WGridRegular3D >( m_dataSet->getGrid() );
                if( !m_grid )
                {
                    errorLog() << "The dataset does not provide a regular grid. Ignoring dataset.";
                    dataValid = false;
                }
            }
        }
        bool propsChanged = m_xPos->changed() || m_yPos->changed();
        if( propsChanged && dataValid )
        {
            debugLog() << "Properties changed.";

            osg::ref_ptr< osg::Geode > newGeode = new osg::Geode();

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Getting Data and setting Properties
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            unsigned int x_scale = m_grid->getNbCoordsX();
            unsigned int y_scale = m_grid->getNbCoordsY();
            unsigned int z_scale = m_grid->getNbCoordsZ();

            debugLog() << "x = " << x_scale << ", y = " << y_scale << ", z = " << z_scale;

            m_xPos->setMin( 0 );
            m_xPos->setMax( x_scale );
            m_xPos->setRecommendedValue( 0.5 * x_scale );

            m_yPos->setMin( 0 );
            m_yPos->setMax( y_scale );
            m_yPos->setRecommendedValue( 0.5 * y_scale );

            //m_dataRays.clear();

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Calculating BoundingBox and OuterBounding
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            WMatrix4d c = WMatrix4d::identity();
            WMatrix4d d = m_grid->getTransform();
            // BEISPIEL
//             WVector4d v;
//             WVector4d ergebnis = c * d * v;

            std::vector< WVector4d > bounding_box( 8 );  // BoundingBox of the dataset

            bounding_box[0] = c * d * WVector4d( 0,       0,       0,       1 );
            bounding_box[1] = c * d * WVector4d( x_scale, 0,       0,       1 );
            bounding_box[2] = c * d * WVector4d( 0,       y_scale, 0,       1 );
            bounding_box[3] = c * d * WVector4d( x_scale, y_scale, 0,       1 );
            bounding_box[4] = c * d * WVector4d( 0,       0,       z_scale, 1 );
            bounding_box[5] = c * d * WVector4d( x_scale, 0,       z_scale, 1 );
            bounding_box[6] = c * d * WVector4d( 0,       y_scale, z_scale, 1 );
            bounding_box[7] = c * d * WVector4d( x_scale, y_scale, z_scale, 1 );

            std::vector< WVector4d > outer_bounding( 2,  bounding_box[0] );  // BoundingBox parallel to the screen (just min and max)
//             debugLog() << "Outer1: " << outer_bounding[0];
//             debugLog() << "Outer2: " << outer_bounding[1];

            for( unsigned int k = 0; k < 8; k++)
            {
                // minimal point in BoundingBox
                outer_bounding[0][0] = std::min( bounding_box[k][0], outer_bounding[0][0] ); // x-min
                outer_bounding[0][1] = std::min( bounding_box[k][1], outer_bounding[0][1] ); // y-min
                outer_bounding[0][2] = std::min( bounding_box[k][2], outer_bounding[0][2] ); // z-min
                outer_bounding[0][3] = 1;
//                 debugLog() << "min-vec:" << outer_bounding[0];
                // maximal point in BoundingBox
                outer_bounding[1][0] = std::max( bounding_box[k][0], outer_bounding[1][0] ); // x-max
                outer_bounding[1][1] = std::max( bounding_box[k][1], outer_bounding[1][1] ); // y-max
                outer_bounding[1][2] = std::max( bounding_box[k][2], outer_bounding[1][2] ); // z-max
                outer_bounding[1][3] = 1;
//                 debugLog() << "max-vec:" << outer_bounding[1];
            }

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Ray Data
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            // start point and direction of the ray
            double x = m_xPos->get( true );
            double y = m_yPos->get( true );
            double z = 0;

            WVector4d start( x, y, z, 1 );
            debugLog() << "Start: " << start;

            WVector4d dir( 0, 0, 1, 0 );
            debugLog() << "Direction: " << dir;

            // ray object
            // ray = start + t * direction
            WRay ray( start, dir );

            // step length
            double interval = 0.33;
            // start position
            double sample_start = outer_bounding[0][2]; // z-min
            // end position
            double sample_end = outer_bounding[1][2]; // z-max

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Calculating Profiles and Samples
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            size_t max_nbSamples = ceil( length( outer_bounding[1] - outer_bounding[0] ) / interval );
            WRayProfile curProfile( max_nbSamples );
//             debugLog() << "Max samples: " << max_nbSamples;
            size_t sampleC = 0;

            // progress
            boost::shared_ptr< WProgress > prog = boost::shared_ptr< WProgress >( new WProgress( "Casting ray." ) );
            m_progress->addSubProgress( prog );

            double sample_t = sample_start - 1;
            while( true )
            {
                WVector4d current = ray.getSpot( sample_t );
                debugLog() << "Point: " << current;

                sample_t += interval;
                if( sample_t > sample_end + 1 ) break;

                // do not calculate anything for vectors outside of the data grid
                if( !m_grid->encloses( getAs3D( current ) ) )
                {
                    debugLog() << "continue...";
                    continue;
                }
                newGeode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( getAs3D( current ), 0.5f ) ) );
//                 double not_int_val = m_dataSet->getValueSet()->getScalarDouble( m_grid->getVoxelNum( getAs3D( current ) ) );
//                 debugLog() << "Old value: " << not_int_val;
                double val = interpolate( current );
                debugLog() << "Value: " << val;

                WRaySample testSample;
                testSample.value() = val;
//                 debugLog() << "Value of Sample: " << testSample.value();
//                 debugLog() << "Sample ID: " << sampleC;
                curProfile[sampleC] = testSample;
                sampleC++;
            }
            m_rootNode->remove( m_geode );
            m_geode = newGeode;

            m_geode->addUpdateCallback( new SafeUpdateCallback( this ) );

            m_rootNode->insert( m_geode );
            prog->finish();
            m_progress->removeSubProgress( prog );
        }
    }
}

WVector3d WMTransferCalc::getAs3D( WVector4d vec )
{
    if( vec[3] == 0 )
    {
        return WVector3d( vec[0], vec[1], vec[2] );
    }
    else
    {
        return WVector3d( vec[0]/vec[3], vec[1]/vec[3], vec[2]/vec[3] );
    }
}


double WMTransferCalc::interpolate( WVector4d position )
{
    double vox_x = static_cast< double >( m_grid->getXVoxelCoord( getAs3D( position ) ) );
    double vox_y = static_cast< double >( m_grid->getYVoxelCoord( getAs3D( position ) ) );
    double vox_z = static_cast< double >( m_grid->getZVoxelCoord( getAs3D( position ) ) );
    WVector4d voxel_center( vox_x, vox_y, vox_z, 1 );

    // distance between voxels in each direction
    double x_offset = m_grid->getOffsetX();
    double y_offset = m_grid->getOffsetY();
    double z_offset = m_grid->getOffsetZ();
//     debugLog() << "Offsets: " << x_offset << ", " << y_offset << ", " << z_offset;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Calculating all relevant neighbours for the interpolation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // vector with all 8 neighbours to the position
    std::vector< WVector4d > neighbours( 8 );
    // vectors with min / max coordinates of neighbours
    // { min_x, min_y, min_z }
    std::vector< double > min_val( 3 );
    std::vector< double > max_val( 3 );

    // positive x direction
    if( position[0] >= voxel_center[0] )
    {
        // positive y direction
        if( position[1] >= voxel_center[1] )
        {
            // positive z direction
            if( position[2] >= voxel_center[2] )
            {
                // Quadrant I
                neighbours[0] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[1] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[2] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]         , 1 );
                neighbours[3] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]+y_offset, voxel_center[2]         , 1 );
                neighbours[4] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]+z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]+z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]+z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]+y_offset, voxel_center[2]+z_offset, 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0]+x_offset;
                max_val[1] = voxel_center[1]+y_offset;
                max_val[2] = voxel_center[2]+z_offset;
            }
            else // negative z direction
            {
                // Quadrant V
                neighbours[0] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]+y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[5] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[6] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]         , 1 );
                neighbours[7] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]+y_offset, voxel_center[2]         , 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2]-z_offset;

                max_val[0] = voxel_center[0]+x_offset;
                max_val[1] = voxel_center[1]+y_offset;
                max_val[2] = voxel_center[2];
            }
        }
        else // negative y direction
        {
            // positive z direction
            if( position[2] >= voxel_center[2] )
            {
                // Quadrant IV
                neighbours[0] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[1] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[2] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[3] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[4] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]+z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]-y_offset, voxel_center[2]+z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]+z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]+z_offset, 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1]-y_offset;
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0]+x_offset;
                max_val[1] = voxel_center[1];
                max_val[2] = voxel_center[2]+z_offset;
            }
            else // negative z direction
            {
                // Quadrant VIII
                neighbours[0] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]-y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[5] = WVector4d( voxel_center[0]+x_offset, voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[6] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[7] = WVector4d( voxel_center[0]+x_offset, voxel_center[1],          voxel_center[2]         , 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1]-y_offset;
                min_val[2] = voxel_center[2]-z_offset;

                max_val[0] = voxel_center[0]+x_offset;
                max_val[1] = voxel_center[1];
                max_val[2] = voxel_center[2];
            }
        }
    }
    else // negative x direction
    {
        // positive y direction
        if( position[1] >= voxel_center[1] )
        {
            // positive z direction
            if( position[2] >= voxel_center[2] )
            {
                // Quadrant II
                neighbours[0] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[1] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[2] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]+y_offset, voxel_center[2]         , 1 );
                neighbours[3] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]         , 1 );
                neighbours[4] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]+z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]+z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]+y_offset, voxel_center[2]+z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]+z_offset, 1 );

                min_val[0] = voxel_center[0]-x_offset;
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1]+y_offset;
                max_val[2] = voxel_center[2]+z_offset;
            }
            else // negative z direction
            {
                // Quadrant VI
                neighbours[0] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]+y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[5] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[6] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]+y_offset, voxel_center[2]         , 1 );
                neighbours[7] = WVector4d( voxel_center[0],          voxel_center[1]+y_offset, voxel_center[2]         , 1 );

                min_val[0] = voxel_center[0]-x_offset;
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2]-z_offset;

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1]+y_offset;
                max_val[2] = voxel_center[2];
            }
        }
        else // negative y direction
        {
            // positive z direction
            if( position[2] >= voxel_center[2] )
            {
                // Quadrant III
                neighbours[0] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[1] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[2] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[3] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[4] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]-y_offset, voxel_center[2]+z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]+z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]+z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]+z_offset, 1 );

                min_val[0] = voxel_center[0]-x_offset;
                min_val[1] = voxel_center[1]-y_offset;
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1];
                max_val[2] = voxel_center[2]+z_offset;
            }
            else // negative z direction
            {
                // Quadrant VII
                neighbours[0] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]-y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]-z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]-z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0]-x_offset, voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[5] = WVector4d( voxel_center[0],          voxel_center[1]-y_offset, voxel_center[2]         , 1 );
                neighbours[6] = WVector4d( voxel_center[0]-x_offset, voxel_center[1],          voxel_center[2]         , 1 );
                neighbours[7] = WVector4d( voxel_center[0],          voxel_center[1],          voxel_center[2]         , 1 );

                min_val[0] = voxel_center[0]-x_offset;
                min_val[1] = voxel_center[1]-y_offset;
                min_val[2] = voxel_center[2]-z_offset;

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1];
                max_val[2] = voxel_center[2];
            }
        }
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Interpolation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // for storing the values of the 8 neighbours
    std::vector< double > values( 8 );

    // check, if all neighbours are inside the grid
    for( unsigned int i = 0; i < neighbours.size(); i++)
    {
        if( !m_grid->encloses( getAs3D( neighbours[i] ) ) )
        {
            values[i] = 0;  // TODO(fjacob): any better?
        }
        else
        {
            values[i] = m_dataSet->getValueSet()->getScalarDouble( m_grid->getVoxelNum( getAs3D( neighbours[i] ) ) );
        }
    }

    // trilinear interpolation from wikipedia

    double x_dif = ( position[0] - min_val[0] ) / ( max_val[0] - min_val[0] );
    double y_dif = ( position[1] - min_val[1] ) / ( max_val[1] - min_val[1] );
    double z_dif = ( position[2] - min_val[2] ) / ( max_val[2] - min_val[2] );

    double c_00  = values[0]*( 1-x_dif ) + values[1]*x_dif;
    double c_10  = values[2]*( 1-x_dif ) + values[3]*x_dif;
    double c_01  = values[4]*( 1-x_dif ) + values[5]*x_dif;
    double c_11  = values[6]*( 1-x_dif ) + values[7]*x_dif;

    double c_0   = c_00*( 1-y_dif ) + c_10*y_dif;
    double c_1   = c_01*( 1-y_dif ) + c_11*y_dif;

    return c_0*( 1-z_dif ) + c_1*z_dif;
}

void WMTransferCalc::SafeUpdateCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    // One note about m_aColor: As you might have notices, changing one of the properties, which recreate the OSG node, causes the material to be
    // gray again. This is simply caused by m_aColor->changed() is still false! To resolve this problem, use some m_osgNeedsUpdate boolean which
    // gets set in your thread main and checked here or, as done in this module, by checking whether the callback is called the first time.
    if( m_module->m_color->changed() || m_initialUpdate )
    {
        // Set the diffuse color and material:
        osg::ref_ptr< osg::Material > mat = new osg::Material();
        mat->setDiffuse( osg::Material::FRONT, m_module->m_color->get( true ) );
        node->getOrCreateStateSet()->setAttribute( mat, osg::StateAttribute::ON );
    }
    traverse( node, nv );
}
