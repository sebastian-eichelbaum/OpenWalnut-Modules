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
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>

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

#include "../interaction/WViewEventHandler.h"
#include "../interaction/WGetMatrixCallback.h"

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
    return "Calculates optimal settings for the Transfer Function after klicking " \
            "by casting rays through the given data and analysing their profiles, " \
            "e. g. with regard to value, gradient or gradient weight.";
}

void WMTransferCalc::connectors()
{
    m_inputData = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "Compulsory Data",
                                                                    "Data which will be checked for displaying." );
    m_inputFA = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "Fractional Anisotropy Data (optional)",
                                                                    "Input for additional data concerning fractional anisotropy of a dataset." );

    // call WModules initialization
    WModule::connectors();
}

void WMTransferCalc::properties()
{
    m_propCondition   = boost::shared_ptr< WCondition >( new WCondition() );
    m_color           = m_properties->addProperty(  "Color", "Color of the orbs.", WColor( 1.0, 0.0, 0.0, 1.0 ), m_propCondition );

    m_xPos            = m_properties->addProperty( "X position", "X coordinate of the ray origin.", 0.0, m_propCondition );
    m_yPos            = m_properties->addProperty( "Y position", "Y coordinate of the ray origin.", 0.0, m_propCondition );
    m_zPos            = m_properties->addProperty( "Z position", "Z coordinate of the ray origin.", 0.0, m_propCondition );

    m_interval        = m_properties->addProperty( "Interval", "Interval for sampling rays.", 0.33, m_propCondition );
    m_rayNumber       = m_properties->addProperty( "# of Rays", "Number of rays being casted.", 10, m_propCondition );
    m_radius          = m_properties->addProperty( "Radius", "Radius for circular vicinity in which the rays shall be casted.",
                                                   2, m_propCondition );

    m_RaySaveFilePath = m_properties->addProperty( "Save RayProfile to:", "Savefile for RayProfile.", WPathHelper::getAppPath(), m_propCondition );
    m_saveTrigger     = m_properties->addProperty( "Save", "Save RayProfile to given directory.", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    WPropertyHelper::PC_PATHEXISTS::addTo( m_RaySaveFilePath );
    WPropertyHelper::PC_ISDIRECTORY::addTo( m_RaySaveFilePath );

    WModule::properties();
}

void WMTransferCalc::requirements()
{
    m_requirements.push_back( new WGERequirement() );
}

bool WMTransferCalc::saveRayProfileTo( WPropFilename path, std::string filename, WRayProfile *profile )
{
    WRayProfile *prof;
    ( profile == NULL ) ? prof = &m_mainProfile : prof = profile;
    if( prof->size() == 0 )
    {
        return false; //empty profile
    }

    std::ofstream savefile( ( path->get( true ).string() + "/" + filename ).c_str() /*, ios::app */ );
    savefile << "# RayProfile data for OpenWalnut TransferCalcModule" << std::endl;
    savefile << "# ID \t dist \t value \t grad_x \t grad_y \t grad_z \t gradL \t FA" << std::endl;
    for( size_t sample = 0; sample < prof->size(); sample++ )
    {
        savefile << sample + 1 << "\t";                           // print id
        savefile << (*prof)[sample].distance() << "\t";         // print t
        savefile << (*prof)[sample].value() << "\t";            // print value
        savefile << (*prof)[sample].gradient()[0] << "\t";      // print gradient x
        savefile << (*prof)[sample].gradient()[1] << "\t";      // print gradient y
        savefile << (*prof)[sample].gradient()[2] << "\t";      // print gradient z
        savefile << (*prof)[sample].gradWeight() << "\t";       // print weight of gradient
        savefile << (*prof)[sample].fracA() << "\t";       // print fractional anisotropy
        savefile << std::endl;
    }
    savefile.close();
    return true;
}

void WMTransferCalc::moduleMain()
{
    debugLog() << "moduleMain started...";

    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_inputData->getDataChangedCondition() );
    m_moduleState.add( m_inputFA->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();
    debugLog() << "Module is now ready.";

    m_rootNode = new WGEManagedGroupNode( m_active );
    //m_rootNode->addUpdateCallback( new TranslateCallback( this ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

    // Create and add an event handler for the main view as we are interested in click events
    WViewEventHandler::RefPtr handler( new WViewEventHandler() );
    WKernel::getRunningKernel()->getGraphicsEngine()->getViewer()->getView()->addEventHandler( handler );
    handler->onLeftClick( boost::bind( &WMTransferCalc::onClick, this, _1 ) );

    // as we need the projection and view matrices: add a callback which queries this
    m_matrixCallback = new WGetMatrixCallback();
    m_rootNode->addCullCallback( m_matrixCallback );

    debugLog() << "Entering main loop";
    while( !m_shutdownFlag() )
    {
        debugLog() << "Waiting ...";
        m_moduleState.wait();
        debugLog() << "New Data Incoming ...";

        // woke up since the module is requested to finish
        if( m_shutdownFlag() )
        {
            debugLog() << "Closing module...";
            break;
        }

        // activates if user pushes save button to save current RayProfile
        if( m_saveTrigger->get( true ) == WPVBaseTypes::PV_TRIGGER_TRIGGERED )
        {
            debugLog() << "User saves RayProfile.";

            boost::shared_ptr< WProgress > saving = boost::shared_ptr< WProgress >( new WProgress( "Saving calculated RayProfiles." ) );
            m_progress->addSubProgress( saving );

            for( unsigned int id = 0; id < m_profiles.size(); id++ )
            {
                std::stringstream prepFilename;
                prepFilename << "RayProfile" << id + 1 << ".dat";
                std::string filename( prepFilename.str() );

                bool success = saveRayProfileTo( m_RaySaveFilePath, filename, &m_profiles[id] );
                if( !success )
                {
                    errorLog() << "RayProfile " << id + 1 << " could not be saved.";
                }
            }

            saving->finish();
            m_progress->removeSubProgress( saving );

            m_saveTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, false );
        }

        bool dataChanged = ( m_inputData->getData() != m_dataSet );
        bool additionalData = ( m_inputFA->getData() != m_FAdataSet );
        bool dataValid  = ( m_dataSet );
        m_FAisValid  = ( m_FAdataSet );

        if( dataChanged )
        {
            m_dataSet = m_inputData->getData();
            dataValid = ( m_dataSet );
            debugLog() << "New Data!";

            if( !dataValid )
            {
                debugLog() << "No valid data anymore. Cleaning up.";
                m_rootNode->clear();
                m_grid.reset();
            }
            else
            {
                // grab the grid
                m_grid = boost::shared_dynamic_cast< WGridRegular3D >( m_dataSet->getGrid() );
                if( !m_grid )
                {
                    errorLog() << "The dataset does not provide a regular grid. Ignoring dataset.";
                    dataValid = false;
                }
            }
        }

        if( additionalData )
        {
            m_FAdataSet = m_inputFA->getData();
            m_FAisValid = ( m_FAdataSet );
            debugLog() << "New FA Data!";

            if( !m_FAisValid )
            {
                debugLog() << "No valid FA data anymore. Cleaning up.";
                m_rootNode->clear();
                m_FAgrid.reset();
            }
            else
            {
                // grab the grid
                m_FAgrid = boost::shared_dynamic_cast< WGridRegular3D >( m_FAdataSet->getGrid() );
                if( !m_FAgrid )
                {
                    errorLog() << "The FA dataset does not provide a regular grid. Ignoring dataset.";
                    m_FAisValid = false;
                }
            }
        }

        bool propsChanged = m_xPos->changed() || m_yPos->changed() || m_zPos->changed()  ||
                            m_interval->changed() || m_rayNumber->changed() || m_radius->changed();

        if( ( propsChanged || dataChanged ) && dataValid )
        {
            debugLog() << "Data or properties changed.";

            // grid scale
            double x_scale = m_grid->getNbCoordsX();
            double y_scale = m_grid->getNbCoordsY();
            double z_scale = m_grid->getNbCoordsZ();

            // voxel scale
            double dist_x = m_grid->getOffsetX();
            double dist_y = m_grid->getOffsetY();
            double dist_z = m_grid->getOffsetZ();

            debugLog() << "x = " << x_scale << ", y = " << y_scale << ", z = " << z_scale;

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Calculating BoundingBox and OuterBounding
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if( dataChanged )
            {
                std::vector< WVector4d > bounding_box( 8 );  // BoundingBox of the dataset

                bounding_box[0] = WVector4d( 0,       0,       0,       1 );
                bounding_box[1] = WVector4d( x_scale, 0,       0,       1 );
                bounding_box[2] = WVector4d( 0,       y_scale, 0,       1 );
                bounding_box[3] = WVector4d( x_scale, y_scale, 0,       1 );
                bounding_box[4] = WVector4d( 0,       0,       z_scale, 1 );
                bounding_box[5] = WVector4d( x_scale, 0,       z_scale, 1 );
                bounding_box[6] = WVector4d( 0,       y_scale, z_scale, 1 );
                bounding_box[7] = WVector4d( x_scale, y_scale, z_scale, 1 );

                //std::vector< WVector4d > outer_bounding( 2,  bounding_box[0] );
                // BoundingBox parallel to the screen (just min and max)
                m_outer_bounding.resize( 2 );
                m_outer_bounding[0] = bounding_box[0];
                m_outer_bounding[1] = bounding_box[0];

                for( unsigned int k = 0; k < 8; k++)
                {
                    // minimal point in BoundingBox
                    m_outer_bounding[0][0] = std::min( bounding_box[k][0], m_outer_bounding[0][0] ); // x-min
                    m_outer_bounding[0][1] = std::min( bounding_box[k][1], m_outer_bounding[0][1] ); // y-min
                    m_outer_bounding[0][2] = std::min( bounding_box[k][2], m_outer_bounding[0][2] ); // z-min
                    m_outer_bounding[0][3] = 1;
    //                  debugLog() << "min-vec:" << m_outer_bounding[0];
                    // maximal point in BoundingBox
                    m_outer_bounding[1][0] = std::max( bounding_box[k][0], m_outer_bounding[1][0] ); // x-max
                    m_outer_bounding[1][1] = std::max( bounding_box[k][1], m_outer_bounding[1][1] ); // y-max
                    m_outer_bounding[1][2] = std::max( bounding_box[k][2], m_outer_bounding[1][2] ); // z-max
                    m_outer_bounding[1][3] = 1;
    //                  debugLog() << "max-vec:" << m_outer_bounding[1];
                }
            }

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Getting Data and setting Properties
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            m_xPos->setMin( 0.0 );
            m_xPos->setMax( x_scale );
            m_xPos->setRecommendedValue( 0.5 * x_scale );

            m_yPos->setMin( 0.0 );
            m_yPos->setMax( y_scale );
            m_yPos->setRecommendedValue( 0.5 * y_scale );

            m_zPos->setMin( 0.0 );
            m_zPos->setMax( z_scale );
            m_zPos->setRecommendedValue( 0.5 * z_scale );

            m_interval->setMin( 0.0 );
            m_interval->setMax( 1.0 );
            //m_interval->setMax( length( WVector4d( dist_x, dist_y, dist_z, 0.0 ) ) );
            m_interval->setRecommendedValue( 0.33 );

            m_rayNumber->setMin( 1 );
            m_rayNumber->setRecommendedValue( 1 );

            m_radius->setMin( 0 );
            m_radius->setRecommendedValue( 2 );

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Drawing current position of the future ray from properties (debugging)
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            if( false )
            {
                osg::ref_ptr< osg::Geode > rayGeode = new osg::Geode();
                rayGeode->addUpdateCallback( new SafeUpdateCallback( this ) );

                WVector4d cylStart = WVector4d( m_xPos->get( true ), m_yPos->get( true ), 0.0, 1.0 );

                osg::Cylinder* cylinder = new osg::Cylinder( getAs3D( cylStart + WVector4d( 0.0, 0.0, 0.5 * z_scale, 0.0 ) ), 0.5f, static_cast<float>( z_scale ) );
                osg::Quat rot;
                rot.makeRotate( osg::Vec3d( 0.0, 0.0, 1.0 ),
                                osg::Vec3d( 1.0, 0.0, 0.0 )   // <-- ray direction vector
                            );
                cylinder->setRotation( rot );

                osg::Cone* cone = new osg::Cone( getAs3D( cylStart + WVector4d( 0.0, 0.0, z_scale, 0.0 ) ), 1.0f, 5.0f );
                cone->setRotation( rot );

                rayGeode->addDrawable( new osg::ShapeDrawable( cylinder ) );
                rayGeode->addDrawable( new osg::ShapeDrawable( cone ) );

                m_rootNode->clear();
                m_rootNode->insert( rayGeode );
            }
        }
    }
    // we technically need to remove the event handler too but there is no method available to do this ...
    // WKernel::getRunningKernel()->getGraphicsEngine()->getViewer()->removeEventHandler( handler );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

void WMTransferCalc::onClick( WVector2i mousePos )
{
/*    debugLog() << "Left Click at " << mousePos;
    debugLog() << "Projection Matrix: " << m_matrixCallback->getProjectionMatrix()->get();
    debugLog() << "ModelView Matrix: " << m_matrixCallback->getModelViewMatrix()->get();
    debugLog() << "Viewport: " << m_matrixCallback->getViewportX()->get() << ", " << m_matrixCallback->getViewportY()->get() << ", "
                               << m_matrixCallback->getViewportWidth()->get() << ", " << m_matrixCallback->getViewportHeight()->get();
*/
    // get the both matrices inverted
    WMatrix4d projectionMatrixInverted = invert( m_matrixCallback->getProjectionMatrix()->get() );
    WMatrix4d modelviewMatrixInverted = invert( m_matrixCallback->getModelViewMatrix()->get() );
                               
    // get the clip-space coordinate:
    WVector4d pInClipSpace( ( 2.0 * mousePos.x() / m_matrixCallback->getViewportWidth()->get() ) - 1.0,
                            ( 2.0 * mousePos.y() / m_matrixCallback->getViewportHeight()->get() ) - 1.0,
                              0.0,
                              1.0 );
    WVector4d dirInClipSpace( 0.0,
                              0.0,
                              1.0,
                              0.0 );

    // unproject the clip-space vectors to the world space
    WVector4d pInWorldSpace   = projectionMatrixInverted * pInClipSpace;
    WVector4d dirInWorldSpace = projectionMatrixInverted * dirInClipSpace;
    // get back to model-space
    WVector4d pInObjectSpace   = modelviewMatrixInverted * pInWorldSpace;
    WVector4d dirInObjectSpace = modelviewMatrixInverted * dirInWorldSpace;

    // for multilication with a vertex
    WMatrix4d totalTransform = modelviewMatrixInverted * projectionMatrixInverted;

//     debugLog() << pInWorldSpace << " --- " << pInObjectSpace;
//     debugLog() << dirInWorldSpace << " --- " << dirInObjectSpace;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Ray Casting
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//     // start point in Object Coordinates
//     WVector4d start( m_xPos->get( true ), m_yPos->get( true ), 0.0, 1.0 );
//     WVector4d dir( 0.0, 0.0, 1.0, 0.0 );
//     // ray object - ray = start + t * direction
//     WRay ray( start, dir );
    
    // ray object - ray = start + t * direction
    WRay ray( pInObjectSpace, dirInObjectSpace );

    // debug "vector"
    osg::ref_ptr< osg::Geode > debugGeode = new osg::Geode();
    debugGeode->addUpdateCallback( new SafeUpdateCallback( this ) );
    debugGeode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( getAs3D( pInObjectSpace, true ), 2.5f ) ) );
    debugGeode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( getAs3D( pInObjectSpace + dirInObjectSpace, true ), 5.0f ) ) );
    m_rootNode->clear();
    m_rootNode->insert( debugGeode );

    // delete old profiles
    m_profiles.clear();

    double interval = m_interval->get( true );
    unsigned int samplesInVicinity = static_cast<unsigned int>( m_rayNumber->get( true ) );
    unsigned int radi = static_cast<unsigned int>( m_radius->get( true ) );

    osg::ref_ptr< osg::Geode > rayGeode = new osg::Geode();
    rayGeode->addUpdateCallback( new SafeUpdateCallback( this ) );

    unsigned int seed = time( 0 ); // for thread safe rand_r function
    for( unsigned int n = 0; n < samplesInVicinity; n++ )
    {
        if( n == 0 ) // the initial profile
        {
            m_mainProfile = castRay( ray, interval, rayGeode );
            m_profiles.push_back( m_mainProfile );
        }
        else // random profiles in vicinity
        {
            WRay vicinity( ray.start() +
                           ( totalTransform * WVector4d( std::sin( rand_r( &seed ) ) * radi, std::cos( rand_r( &seed ) ) * radi, 0.0, 0.0 ) ),
                           ray.direction() );
            m_profiles.push_back( castRay( vicinity, interval, rayGeode ) );
        }
    }
    m_rootNode->clear();
    m_rootNode->insert( rayGeode );
}

WRayProfile WMTransferCalc::castRay( WRay ray, double interval, osg::ref_ptr< osg::Geode > rayGeode )
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Calculating a single profile with samples
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // progress
    boost::shared_ptr< WProgress > prog = boost::shared_ptr< WProgress >( new WProgress( "Casting ray." ) );
    m_progress->addSubProgress( prog );

    //TODO(fjacob): the not set RaySamples may be deleted after collecting all profiles
    size_t max_nbSamples = ceil( length( m_outer_bounding[1] - m_outer_bounding[0] ) / interval );
    WRayProfile curProfile( max_nbSamples );
//   debugLog() << "Max samples: " << max_nbSamples;

    struct BoxIntersecParameter bounds = rayIntersectsBox( ray );
    if( bounds.isNull ) //no intersection with data grid
    {
        prog->finish();
        m_progress->removeSubProgress( prog );
        return curProfile; //empty profile
    }
    double start_t = bounds.minimum_t;
    double end_t   = bounds.maximum_t;

/*    debugLog() << start_t << " - " << end_t;
    prog->finish();
    m_progress->removeSubProgress( prog );
    return curProfile;
*/
    WVector4d geodeStartVec, geodeEndVec, cylinderVec;

    size_t sampleCount = 0;
    for( double sample_t = start_t; sample_t <= end_t; sample_t += interval )
    {
        WVector4d current = ray.getSpot( sample_t );
        if( sample_t == start_t )
        {
            geodeStartVec = current;
        }

        // do not calculate anything for vectors outside of the data grid
        if( !m_grid->encloses( getAs3D( current ) ) )
        {
            //debugLog() << "continue...";
            continue;
        }
        double val = interpolate( current, m_grid );
        curProfile[sampleCount].value() = val;
        curProfile[sampleCount].distance() = sample_t;

        if( m_FAisValid )
        {
            if( m_FAgrid->encloses( getAs3D( current ) ) )
            {
                double FA_val = interpolate( current, m_FAgrid );
                curProfile[sampleCount].fracA() = FA_val;
            }
        }

        //calculate gradient
        WVector4d grad = getGradient( current );
        double weight = length( grad );
        curProfile[sampleCount].gradWeight() = weight;
        if( weight != 0 ) //normalize
        {
            curProfile[sampleCount].gradient() = grad * ( 1 / weight );
        }

        if( sample_t + interval <= end_t )
        {
            geodeEndVec = current;
        }
        sampleCount++;
    }
    // draw ray (a cylinder and a cone)
    cylinderVec = geodeEndVec - geodeStartVec;
    
    osg::Cylinder* cylinder = new osg::Cylinder( getAs3D( geodeStartVec + ( 0.5 * cylinderVec ) ), 0.5f, length( cylinderVec ) );
    osg::Cone* cone = new osg::Cone( getAs3D( geodeEndVec ), 1.0f, 5.0f );

    osg::Quat rot;
    rot.makeRotate( osg::Vec3d( 0.0, 0.0, 1.0 ),
                    osg::Vec3d( ray.direction().x(), ray.direction().y(), ray.direction().z() ) //TODO(fjacob): cast from WVector4d to osg::Vec3d ?
                );
    cylinder->setRotation( rot );
    cone->setRotation( rot );

    rayGeode->addDrawable( new osg::ShapeDrawable( cylinder ) );
    rayGeode->addDrawable( new osg::ShapeDrawable( cone ) );

    prog->finish();
    m_progress->removeSubProgress( prog );

    return curProfile;
}

struct BoxIntersecParameter WMTransferCalc::rayIntersectsBox( WRay ray )
{
    // ray = start + t * dir
    double tnear = - std::numeric_limits<double>::max();
    double tfar  =   std::numeric_limits<double>::max();
    double t0, t1;
    
    struct BoxIntersecParameter result;
    result.isNull = true;

    for( unsigned int coord = 0; coord < 3; coord++ ) // for x,y,z
    {
        if( ray.direction()[coord] == 0 ) // ray parallel to the min/max coord-planes
        {
            if( ray.start()[coord] < m_outer_bounding[0][coord] || ray.start()[coord] > m_outer_bounding[1][coord] )
            {
                return result;
            }
        }
        else // ray not parallel, so calculate intersections
        {
            // time at which ray intersects min plane
            t0 = ( m_outer_bounding[0][coord] - ray.start()[coord] ) / ray.direction()[coord];
            // time at which ray intersects max plane
            t1 = ( m_outer_bounding[1][coord] - ray.start()[coord] ) / ray.direction()[coord];
            if( t0 > t1 )
            {
                std::swap( t0, t1 );
            }
            if( t0 > tnear )
            {
                tnear = t0;
            }
            if( t1 < tfar  )
            {
                tfar  = t1;
            }
            if( tnear > tfar )
            {
                return result; // cube is missed
            }
            /*if( tfar < 0 )
            {
                 return result; // cube behind ray - possible case
            }*/
        }
    }
    result.minimum_t = tnear;
    result.maximum_t = tfar;
    result.isNull = false;
    return result;
}

WVector3d WMTransferCalc::getAs3D( const WVector4d& vec, bool disregardW )
{
    if( vec[3] == 0 || vec[3] == 1 || disregardW )
    {
        return WVector3d( vec[0], vec[1], vec[2] );
    }
    else
    {
        return WVector3d( vec[0] / vec[3], vec[1] / vec[3], vec[2] / vec[3] );
    }
}

WVector4d WMTransferCalc::getGradient( const WVector4d& position )
{
    // voxeldistance in each direction (usually 1)
    double dist_x = m_grid->getOffsetX();
    double dist_y = m_grid->getOffsetY();
    double dist_z = m_grid->getOffsetZ();
    double x_val, y_val, z_val;

    x_val = ( ( interpolate( position + WVector4d( 0.5 * dist_x, 0.0, 0.0, 0.0 ), m_grid ) )
            - ( interpolate( position - WVector4d( 0.5 * dist_x, 0.0, 0.0, 0.0 ), m_grid ) ) ) / dist_x;
    y_val = ( ( interpolate( position + WVector4d( 0.0, 0.5 * dist_y, 0.0, 0.0 ), m_grid ) )
            - ( interpolate( position - WVector4d( 0.0, 0.5 * dist_y, 0.0, 0.0 ), m_grid ) ) ) / dist_y;
    z_val = ( ( interpolate( position + WVector4d( 0.0, 0.0, 0.5 * dist_z, 0.0 ), m_grid ) )
            - ( interpolate( position - WVector4d( 0.0, 0.0, 0.5 * dist_z, 0.0 ), m_grid ) ) ) / dist_z;

    return WVector4d( x_val, y_val, z_val, 0 );
}

double WMTransferCalc::interpolate( const WVector4d& position, boost::shared_ptr< WGridRegular3D > inter_grid  )
{
    double vox_x = static_cast< double >( inter_grid->getXVoxelCoord( getAs3D( position ) ) );
    double vox_y = static_cast< double >( inter_grid->getYVoxelCoord( getAs3D( position ) ) );
    double vox_z = static_cast< double >( inter_grid->getZVoxelCoord( getAs3D( position ) ) );
    WVector4d voxel_center( vox_x, vox_y, vox_z, 1 );

    // distance between voxels in each direction
    double x_offset = inter_grid->getOffsetX();
    double y_offset = inter_grid->getOffsetY();
    double z_offset = inter_grid->getOffsetZ();
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
                neighbours[0] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[1] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[2] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2]           , 1 );
                neighbours[3] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] + y_offset, voxel_center[2]           , 1 );
                neighbours[4] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] + z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2] + z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2] + z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] + y_offset, voxel_center[2] + z_offset, 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0] + x_offset;
                max_val[1] = voxel_center[1] + y_offset;
                max_val[2] = voxel_center[2] + z_offset;
            }
            else // negative z direction
            {
                // Quadrant V
                neighbours[0] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] + y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[5] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[6] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2]           , 1 );
                neighbours[7] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] + y_offset, voxel_center[2]           , 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2] - z_offset;

                max_val[0] = voxel_center[0] + x_offset;
                max_val[1] = voxel_center[1] + y_offset;
                max_val[2] = voxel_center[2];
            }
        }
        else // negative y direction
        {
            // positive z direction
            if( position[2] >= voxel_center[2] )
            {
                // Quadrant IV
                neighbours[0] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[1] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[2] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[3] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[4] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2] + z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] - y_offset, voxel_center[2] + z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] + z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2] + z_offset, 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1] - y_offset;
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0] + x_offset;
                max_val[1] = voxel_center[1];
                max_val[2] = voxel_center[2] + z_offset;
            }
            else // negative z direction
            {
                // Quadrant VIII
                neighbours[0] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] - y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[5] = WVector4d( voxel_center[0] + x_offset, voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[6] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[7] = WVector4d( voxel_center[0] + x_offset, voxel_center[1],            voxel_center[2]           , 1 );

                min_val[0] = voxel_center[0];
                min_val[1] = voxel_center[1] - y_offset;
                min_val[2] = voxel_center[2] - z_offset;

                max_val[0] = voxel_center[0] + x_offset;
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
                neighbours[0] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[1] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[2] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] + y_offset, voxel_center[2]           , 1 );
                neighbours[3] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2]           , 1 );
                neighbours[4] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2] + z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] + z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] + y_offset, voxel_center[2] + z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2] + z_offset, 1 );

                min_val[0] = voxel_center[0] - x_offset;
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1] + y_offset;
                max_val[2] = voxel_center[2] + z_offset;
            }
            else // negative z direction
            {
                // Quadrant VI
                neighbours[0] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] + y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[5] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[6] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] + y_offset, voxel_center[2]           , 1 );
                neighbours[7] = WVector4d( voxel_center[0],            voxel_center[1] + y_offset, voxel_center[2]           , 1 );

                min_val[0] = voxel_center[0] - x_offset;
                min_val[1] = voxel_center[1];
                min_val[2] = voxel_center[2] - z_offset;

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1] + y_offset;
                max_val[2] = voxel_center[2];
            }
        }
        else // negative y direction
        {
            // positive z direction
            if( position[2] >= voxel_center[2] )
            {
                // Quadrant III
                neighbours[0] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[1] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[2] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[3] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[4] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] - y_offset, voxel_center[2] + z_offset, 1 );
                neighbours[5] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2] + z_offset, 1 );
                neighbours[6] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2] + z_offset, 1 );
                neighbours[7] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] + z_offset, 1 );

                min_val[0] = voxel_center[0] - x_offset;
                min_val[1] = voxel_center[1] - y_offset;
                min_val[2] = voxel_center[2];

                max_val[0] = voxel_center[0];
                max_val[1] = voxel_center[1];
                max_val[2] = voxel_center[2] + z_offset;
            }
            else // negative z direction
            {
                // Quadrant VII
                neighbours[0] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] - y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[1] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2] - z_offset, 1 );
                neighbours[2] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[3] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2] - z_offset, 1 );
                neighbours[4] = WVector4d( voxel_center[0] - x_offset, voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[5] = WVector4d( voxel_center[0],            voxel_center[1] - y_offset, voxel_center[2]           , 1 );
                neighbours[6] = WVector4d( voxel_center[0] - x_offset, voxel_center[1],            voxel_center[2]           , 1 );
                neighbours[7] = WVector4d( voxel_center[0],            voxel_center[1],            voxel_center[2]           , 1 );

                min_val[0] = voxel_center[0] - x_offset;
                min_val[1] = voxel_center[1] - y_offset;
                min_val[2] = voxel_center[2] - z_offset;

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
        if( !inter_grid->encloses( getAs3D( neighbours[i] ) ) )
        {
            values[i] = 0;  // TODO(fjacob): any better?
        }
        else
        {
            values[i] = m_dataSet->getValueSet()->getScalarDouble( inter_grid->getVoxelNum( getAs3D( neighbours[i] ) ) );
        }
    }

    // trilinear interpolation from wikipedia

    double x_dif = ( position[0] - min_val[0] ) / ( max_val[0] - min_val[0] );
    double y_dif = ( position[1] - min_val[1] ) / ( max_val[1] - min_val[1] );
    double z_dif = ( position[2] - min_val[2] ) / ( max_val[2] - min_val[2] );

    double c_00  = values[0] * ( 1-x_dif ) + values[1] * x_dif;
    double c_10  = values[2] * ( 1-x_dif ) + values[3] * x_dif;
    double c_01  = values[4] * ( 1-x_dif ) + values[5] * x_dif;
    double c_11  = values[6] * ( 1-x_dif ) + values[7] * x_dif;

    double c_0   = c_00 * ( 1-y_dif ) + c_10 * y_dif;
    double c_1   = c_01 * ( 1-y_dif ) + c_11 * y_dif;

    return c_0 * ( 1-z_dif ) + c_1 * z_dif;
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
