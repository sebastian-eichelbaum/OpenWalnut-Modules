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

#include <Eigen/Eigen>

#include "core/kernel/WKernel.h"
#include "core/common/WColor.h"
#include "core/common/WPathHelper.h"
#include "core/common/WPropertyHelper.h"
#include "core/common/math/WMatrix.h"
#include "core/common/math/linearAlgebra/WLinearAlgebra.h"
#include "core/common/WItemSelection.h"
#include "core/common/WItemSelectionItem.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/common/WItemSelector.h"
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
    m_inputDerivation = WModuleInputData< WDataSetVector >::createAndAdd( shared_from_this(), "Derivated Data",
                                                                    "Derivated data by using the module 'Spatial Derivative'." );
    m_inputFA = WModuleInputData< WDataSetScalar >::createAndAdd( shared_from_this(), "Fractional Anisotropy Data (optional)",
                                                                    "Input for additional data concerning fractional anisotropy of a dataset." );

    // output data
    m_curveMeanOut = WModuleOutputData< WDataSetScalar >::createAndAdd( shared_from_this(),
                                                                        "(a+b)/2", "The calculated curvature dataset in mean form." );
    m_curveGaussOut = WModuleOutputData< WDataSetScalar >::createAndAdd( shared_from_this(),
                                                                         "a*b", "The calculated curvature dataset in Gauss form." );

    // call WModules initialization
    WModule::connectors();
}

void WMTransferCalc::properties()
{
    m_propCondition   = boost::shared_ptr< WCondition >( new WCondition() );
    m_color           = m_properties->addProperty(  "Color", "Color of the ray.", WColor( 1.0, 0.0, 0.0, 1.0 ), m_propCondition );

    m_plotDataSelection = WItemSelection::SPtr( new WItemSelection() );
    m_plotDataSelection->addItem( MyItemType::create( 3, "Plot value.", "Chooses 'value' data for plotting." ) );
    m_plotDataSelection->addItem( MyItemType::create( 7, "Plot gradient weight.", "Chooses 'gradient weight' data for plotting." ) );
    m_plotDataSelection->addItem( MyItemType::create( 8, "Plot FA value.", "Chooses 'fractional anisotropy' data for plotting." ) );
    m_plotDataSelection->addItem( MyItemType::create( 9, "Plot angle.", "Chooses 'angle between ray direction and gradient' for plotting." ) );
    m_plotDataSelection->addItem( MyItemType::create( 10, "Plot mean curvature.", "Chooses 'mean curvature' data for plotting." ) );
    m_plotDataSelection->addItem( MyItemType::create( 11, "Plot gaussian curvature.", "Chooses 'Gaussian curvature' data for plotting." ) );

    m_multiPlotDataSelection = m_properties->addProperty( "Choose plot data", "Choose one of these for data plotting.",
                                                              m_plotDataSelection->getSelectorFirst(), m_propCondition );

    m_interval        = m_properties->addProperty( "Interval", "Interval for sampling rays.", 0.33, m_propCondition );
    m_rayNumber       = m_properties->addProperty( "# of Rays", "Number of rays being casted with one click.", 10, m_propCondition );
    m_radius          = m_properties->addProperty( "Radius", "Radius for circular vicinity in which the rays shall be casted.",
                                                   2, m_propCondition );

    m_RaySaveFilePath = m_properties->addProperty( "Save RayProfile to Folder:",
                                                   "Savefile for RayProfile.", WPathHelper::getAppPath(), m_propCondition );
    m_saveTrigger     = m_properties->addProperty( "Save", "Save RayProfile to given directory.", WPVBaseTypes::PV_TRIGGER_READY, m_propCondition );

    WPropertyHelper::PC_PATHEXISTS::addTo( m_RaySaveFilePath );
    WPropertyHelper::PC_ISDIRECTORY::addTo( m_RaySaveFilePath );
    WPropertyHelper::PC_NOTEMPTY::addTo( m_multiPlotDataSelection );

    WModule::properties();
}

void WMTransferCalc::requirements()
{
    m_requirements.push_back( new WGERequirement() );
}

bool WMTransferCalc::saveRayProfileTo( WPropFilename path, std::string filename, WRayProfile *profile )
{
    WRayProfile *prof;
    if( profile == NULL )
    {
        // if no profile is given, the main profile will be used as default
        prof = &m_mainProfile;
    }
    else
    {
        // use given profile
        prof = profile;
    }
    //( profile == NULL ) ? prof = &m_mainProfile : prof = profile;

    if( prof->size() == 0 )
    {
        return false; //  empty profiles will not be saved
    }

    // compose context of the soon-to-be-saved file which will be saved in given path with given filename
    std::ofstream savefile( ( path->get( true ).string() + "/" + filename ).c_str() /*, ios::app */ );
    // heading
    savefile << "# RayProfile data for OpenWalnut TransferCalcModule" << std::endl;
    savefile << "# ID \t dist \t value \t grad_x \t grad_y \t grad_z \t gradW \t FA \t angle \t mean \t gauss" << std::endl;
    // save data of each sample point
    for( size_t sample = 0; sample < prof->size(); sample++ )
    {
        savefile << sample + 1 << "\t";                         // print id
        savefile << (*prof)[sample].distance() << "\t";         // print t
        savefile << (*prof)[sample].value() << "\t";            // print value
        savefile << (*prof)[sample].gradient()[0] << "\t";      // print gradient x
        savefile << (*prof)[sample].gradient()[1] << "\t";      // print gradient y
        savefile << (*prof)[sample].gradient()[2] << "\t";      // print gradient z
        savefile << (*prof)[sample].gradWeight() << "\t";       // print weight of gradient
        savefile << (*prof)[sample].fracA() << "\t";            // print fractional anisotropy
        savefile << (*prof)[sample].angle() << "\t";            // print angle between gradient and ray
        savefile << (*prof)[sample].meanCurv() << "\t";         // print mean curvature
        savefile << (*prof)[sample].gaussCurv() << "\t";        // print gauss curvature
        savefile << std::endl;
    }
    savefile.close();
    return true;
}

void WMTransferCalc::moduleMain()
{
    debugLog() << "moduleMain started...";

    // init observation of properties
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_inputData->getDataChangedCondition() );
    m_moduleState.add( m_inputDerivation->getDataChangedCondition() );
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
            debugLog() << "User saves RayProfile(s).";

            // start progress for GUI
            boost::shared_ptr< WProgress > saving = boost::shared_ptr< WProgress >( new WProgress( "Saving calculated RayProfiles." ) );
            m_progress->addSubProgress( saving );

            // save data and plot file for every profile
            unsigned int nrProfiles = m_profiles.size();
            for( unsigned int id = 0; id < nrProfiles; id++ )
            {
                // generate header
                // first a general header for setting default options
                std::stringstream prepGNUPlotScript;
                prepGNUPlotScript << "# Measured RayProfile data for Profile "<< id + 1 << std::endl
                                << "#===========================================" << std::endl << std::endl
                                << "set title 'RayProfile " << id + 1 << "'" << std::endl
                                << "set pointsize 0.5" << std::endl
                                << "set grid" << std::endl
                                << "set xlabel 'uniform distance on ray'" << std::endl;

                // generate filename of data file
                std::stringstream prepFilename;
                prepFilename << "RayProfile" << id + 1 << ".dat";
                std::string filename( prepFilename.str() );

                // save data which will be plotted with the plot script with this method
                bool success = saveRayProfileTo( m_RaySaveFilePath, filename, &m_profiles[id] );
                if( !success )
                {
                    errorLog() << "RayProfile " << id + 1 << " was empty or could not be saved.";
                }
                else // saving successful
                {
                    // check selector for the chosen data to plot
                    WItemSelector s = m_multiPlotDataSelection->get( true );
                    unsigned int count = s.size();
                    prepGNUPlotScript << "plot ";
                    // each selected option gets an individual command line in the later script
                    for( size_t i = 0; i < s.size(); ++i )
                    {
//                         infoLog() << "The user chose " << s.at( i )->getName()
//                                 << " with the value " << s.at( i )->getAs< MyItemType >()->getValue();

                        // switch works with an enumeration
                        switch( s.at( i )->getAs< MyItemType >()->getValue() )
                        {
                            case VALUE:
                                prepGNUPlotScript << "'" << filename << "' using 2:3 title 'value' with points";
                                if( count - 1 != 0 )
                                {
                                    prepGNUPlotScript << ", \\" << std::endl;
                                }
                                count--;
                                break;
                            case WEIGHT:
                                prepGNUPlotScript << "'" << filename << "' using 2:7 title 'gradient weight' with points";
                                if( count - 1 != 0 )
                                {
                                    prepGNUPlotScript << ", \\" << std::endl;
                                }
                                count--;
                                break;
                            case FA:
                                prepGNUPlotScript << "'" << filename << "' using 2:8 title 'FA value' with points";
                                if( count - 1 != 0 )
                                {
                                    prepGNUPlotScript << ", \\" << std::endl;
                                }
                                count--;
                                break;
                            case ANGLE:
                                prepGNUPlotScript << "'" << filename << "' using 2:($9*(180/3.14159265)) title 'angle' with points";
                                if( count - 1 != 0 )
                                {
                                    prepGNUPlotScript << ", \\" << std::endl;
                                }
                                count--;
                                break;
                            case MEAN:
                                prepGNUPlotScript << "'" << filename << "' using 2:10 title 'mean curvature' with points";
                                if( count - 1 != 0 )
                                {
                                    prepGNUPlotScript << ", \\" << std::endl;
                                }
                                count--;
                                break;
                            case GAUSS:
                                prepGNUPlotScript << "'" << filename << "' using 2:11 title 'gaussian curvature' with points";
                                if( count - 1 != 0 )
                                {
                                    prepGNUPlotScript << ", \\" << std::endl;
                                }
                                count--;
                                break;
                            default:
                                errorLog() << "No properties selected!";
                        }
                    }
                    // generate filename for plot script
                    std::stringstream prepPlotFilename;
                    prepPlotFilename << "/plot" << id + 1 << ".gsc";
                    std::string plotfilename( prepPlotFilename.str() );
                    // save plot script
                    std::ofstream plotFile( ( m_RaySaveFilePath->get( true ).string() + plotfilename ).c_str() );
                    plotFile << prepGNUPlotScript.str();
                    plotFile.close();
                }
            }
            // end progress
            saving->finish();
            m_progress->removeSubProgress( saving );
            // reactivate trigger
            m_saveTrigger->set( WPVBaseTypes::PV_TRIGGER_READY, false );
        }

        // check input connectors for new datasets
        bool dataChanged = ( m_inputData->getData() != m_dataSet );
        bool additionalData = ( m_inputFA->getData() != m_FAdataSet );
        bool newDerivative = ( m_inputDerivation->getData() != m_deriDataSet );
        bool dataValid  = ( m_dataSet );
        m_DeriIsValid = ( m_deriDataSet );
        m_FAisValid  = ( m_FAdataSet );

        if( dataChanged ) // main data, compulsory
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

        if( newDerivative ) // optional: dataset with derivated data
        {
            m_deriDataSet = m_inputDerivation->getData();
            m_DeriIsValid = ( m_deriDataSet );
            debugLog() << "New Derivated Data!";

            if( !m_DeriIsValid )
            {
                debugLog() << "No valid derivated data anymore. Cleaning up.";
                m_rootNode->clear();
                m_deriGrid.reset();
                m_curveMeanOut->reset();
                m_curveGaussOut->reset();
            }
            else
            {
                // grab the grid
                m_deriGrid = boost::shared_dynamic_cast< WGridRegular3D >( m_deriDataSet->getGrid() );
                if( !m_deriGrid )
                {
                    errorLog() << "The derivated dataset does not provide a regular grid. Ignoring dataset.";
                    m_DeriIsValid = false;
                }
            }

            if( m_DeriIsValid )
            {
                // reset old calculations
                m_meanCurvDataSet.reset();
                m_gaussCurvDataSet.reset();
                // this may take some time, depending on the data
                calculateCurvature();
            }
        }

        if( additionalData ) // optional: fractional anisotropy data
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

        bool propsChanged = m_interval->changed() || m_rayNumber->changed() || m_radius->changed();

        // observes properties and connectors
        // main calculation of the module starts in onClick function
        if( ( propsChanged || dataChanged ) && dataValid )
        {
            debugLog() << "Data or properties changed.";

            // grid scale
            double x_scale = m_grid->getNbCoordsX();
            double y_scale = m_grid->getNbCoordsY();
            double z_scale = m_grid->getNbCoordsZ();

            // voxel scale
//             double dist_x = m_grid->getOffsetX();
//             double dist_y = m_grid->getOffsetY();
//             double dist_z = m_grid->getOffsetZ();

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

                // BoundingBox parallel to the screen (just min and max)
                m_outer_bounding.resize( 2 );
                m_outer_bounding[0] = bounding_box[0];
                m_outer_bounding[1] = bounding_box[0];

                for( unsigned int k = 0; k < 8; k++ )
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
            // Getting Data and setting Properties for GUI
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            m_interval->setMin( 0.0 );
            m_interval->setMax( 1.0 );
            //m_interval->setMax( length( WVector4d( dist_x, dist_y, dist_z, 0.0 ) ) );
            m_interval->setRecommendedValue( 0.33 );

            m_rayNumber->setMin( 1 );
            m_rayNumber->setRecommendedValue( 1 );

            m_radius->setMin( 1 );
            m_radius->setRecommendedValue( 2 );
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Drawing current position of the future ray from properties (debugging)
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if( false ) //just for debugging
            {
                osg::ref_ptr< osg::Geode > rayGeode = new osg::Geode();
                rayGeode->addUpdateCallback( new SafeUpdateCallback( this ) );

                WVector4d cylStart = WVector4d( 0.5 * x_scale, 0.5 * y_scale, 0.0, 1.0 );

                osg::Cylinder* cylinder = new osg::Cylinder( getAs3D( cylStart + WVector4d( 0.0, 0.0, 0.5 * z_scale, 0.0 ) ),
                                                                0.5f, static_cast<float>( z_scale ) );
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
    // get both matrices inverted
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
    //WMatrix4d totalTransform = modelviewMatrixInverted * projectionMatrixInverted;

//     debugLog() << pInWorldSpace << " --- " << pInObjectSpace;
//     debugLog() << dirInWorldSpace << " --- " << dirInObjectSpace;

    // debug "vector"
/*    osg::ref_ptr< osg::Geode > debugGeode = new osg::Geode();
    debugGeode->addUpdateCallback( new SafeUpdateCallback( this ) );
    debugGeode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( getAs3D( pInObjectSpace, true ), 2.5f ) ) );
    debugGeode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( getAs3D( pInObjectSpace + dirInObjectSpace, true ), 5.0f ) ) );
    m_rootNode->clear();
    m_rootNode->insert( debugGeode );
*/
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Ray Casting
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // ray object calculated from click position: ray = start + t * direction
    WRay ray( pInObjectSpace, dirInObjectSpace );
    // delete old profiles
    m_profiles.clear();

    // get user defined properties
    double interval = m_interval->get( true );
    unsigned int samplesInVicinity = static_cast<unsigned int>( m_rayNumber->get( true ) );
    unsigned int radi = static_cast<unsigned int>( m_radius->get( true ) );

    osg::ref_ptr< osg::Geode > rayGeode = new osg::Geode();
    rayGeode->addUpdateCallback( new SafeUpdateCallback( this ) );

    // seed for random
    std::srand( time( 0 ) );
    // cast additional rays if user defined more than one ray in properties
    for( unsigned int n = 0; n < samplesInVicinity; n++ )
    {
        if( n == 0 ) // the initial profile
        {
            m_mainProfile = castRay( ray, interval, rayGeode );
            m_profiles.push_back( m_mainProfile );
        }
        else // random profiles in vicinity
        {
            // http://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
            double angle = static_cast< double >( ( std::rand() % 255 + 1 ) / 255.0 ) * 360.0;
            double dis = static_cast< double >( ( std::rand() % 255 + 1 ) / 255.0 ) * radi +
                         static_cast< double >( ( std::rand() % 255 + 1 ) / 255.0 ) * radi;
            double random_rad;
            if( dis > radi )
            {
                random_rad = ( 2 * radi ) - dis;
            }
            else
            {
                random_rad = dis;
            }
            // create orthogonal plane to ray for calculating rays in vicinity
            // (the ray gets transformed and scaled before this step,
            // so we have to calculate this plane to get a uniform scale for the radius)
            double coord = std::max( ray.direction().x(), std::max( ray.direction().y(), ray.direction().z() ) );
            WVector3d helper( coord, coord, coord );
            WVector3d ax1 = cross( getAs3D( ray.direction(), true ), helper );
            WVector3d ax2 = cross( getAs3D( ray.direction(), true ), ax1 );
            ax1 = normalize( ax1 );
            ax2 = normalize( ax2 );
            // base vectors of the calculated plane
            WVector4d ax1_4( ax1.x(), ax1.y(), ax1.z(), 0.0 );
            WVector4d ax2_4( ax2.x(), ax2.y(), ax2.z(), 0.0 );

            WVector4d vicPoint = ray.start() + ( ax1_4 * random_rad * std::cos( angle ) ) + ( ax2_4 * random_rad * std::sin( angle ) );
            WRay vicinity( vicPoint, ray.direction() );
            m_profiles.push_back( castRay( vicinity, interval, rayGeode ) );
        }
    }
    m_rootNode->clear();
    m_rootNode->insert( rayGeode );
}

WRayProfile WMTransferCalc::castRay( WRay ray, double interval, osg::ref_ptr< osg::Geode > rayGeode )
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Calculating a single profile with all samples
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // start progress for GUI
    boost::shared_ptr< WProgress > prog = boost::shared_ptr< WProgress >( new WProgress( "Casting ray." ) );
    m_progress->addSubProgress( prog );

    //TODO(fjacob): the not set RaySamples may be deleted after collecting all profiles
    size_t max_nbSamples = ceil( length( m_outer_bounding[1] - m_outer_bounding[0] ) / interval );
    WRayProfile curProfile( max_nbSamples );
//   debugLog() << "Max samples: " << max_nbSamples;

    // BoxIntersecParameter contains two values, the first and second intersection parameter with the grid
    struct BoxIntersecParameter bounds = rayIntersectsBox( ray );
    if( bounds.isNull ) //no intersection with data grid
    {
        prog->finish();
        m_progress->removeSubProgress( prog );
        return curProfile; //empty profile
    }
    double start_t = bounds.minimum_t;
    double end_t   = bounds.maximum_t;

    // helpers for visualization
    WVector4d geodeStartVec, geodeEndVec, cylinderVec;

    size_t sampleCount = 0;
    for( double sample_t = start_t; sample_t <= end_t; sample_t += interval )
    {
        WVector4d current = ray.getSpot( sample_t );
        WVector3d cur3D = getAs3D( current );

        // do not calculate anything for vectors outside of the data grid
        // (should not be neccessary anymore, though)
        if( !m_grid->encloses( cur3D ) )
        {
            //debugLog() << "continue...";
            continue;
        }
        // save position of the sample point in profile
        double vox_x = static_cast< double >( m_grid->getVoxelNum( cur3D ) );
        double vox_y = static_cast< double >( m_grid->getVoxelNum( cur3D ) );
        double vox_z = static_cast< double >( m_grid->getVoxelNum( cur3D ) );
        curProfile[sampleCount].position() = WVector4d( vox_x, vox_y, vox_z, 1 );

        // set start vector for drawing the ray
        if( sample_t == start_t )
        {
            geodeStartVec = current;
        }

        // get interpolated value at current sample point
        double val = interpolate( current, m_grid, m_dataSet );
        // save value
        curProfile[sampleCount].value() = val;
        // save distance t
        curProfile[sampleCount].distance() = sample_t;

        // if there is any FA data available, save it
        if( m_FAisValid )
        {
            if( m_FAgrid->encloses( cur3D ) )
            {
                // save fractional anisotropy
                curProfile[sampleCount].fracA() = interpolate( current, m_FAgrid, m_FAdataSet );
            }
        }
        // if there is any derivated data available, save calculated curvature
        if( m_DeriIsValid )
        {
            if( m_deriGrid->encloses( cur3D ) )
            {
                if( m_meanCurvDataSet )
                {
                    curProfile[sampleCount].meanCurv() = interpolate( current, m_deriGrid, m_meanCurvDataSet );
                }
                if( m_gaussCurvDataSet )
                {
                    curProfile[sampleCount].gaussCurv() = interpolate( current, m_deriGrid, m_gaussCurvDataSet );
                }
            }
        }

        // calculate gradient
        WVector4d grad = getGradient( current );
        double weight = length( grad );
        // save gradient weight
        curProfile[sampleCount].gradWeight() = weight;
        // normalize
        if( weight != 0 )
        {
            // save gradient
            curProfile[sampleCount].gradient() = grad * ( 1 / weight );
        }

        // calculate angle between gradient and ray
        // gradVec and rayVec are already normalized
        Eigen::Vector3d gradVec( curProfile[sampleCount].gradient()[0], curProfile[sampleCount].gradient()[1],
                                    curProfile[sampleCount].gradient()[2] );
        Eigen::Vector3d rayVec( ray.direction()[0], ray.direction()[1], ray.direction()[2] );
//         curProfile[sampleCount].angle() = std::min( std::acos( gradVec.dot( rayVec ) ),
//                                                     std::acos( gradVec.dot( rayVec * -1 ) ) );
        curProfile[sampleCount].angle() = std::acos( gradVec.dot( rayVec * -1 ) );

        // set helper for visualization
        if( sample_t + interval <= end_t )
        {
            geodeEndVec = current;
        }
        sampleCount++;
    }
    // draw ray (a cylinder and a cone)
    // (it is a little tricky because osg does not support
    // native rotation of its drawables on initialization)
    cylinderVec = geodeEndVec - geodeStartVec;

    // set cylinder center in the middle of the ray (will be rotation center)
    osg::Cylinder* cylinder = new osg::Cylinder( getAs3D( geodeStartVec + ( 0.5 * cylinderVec ) ), 0.5f, length( cylinderVec ) );
    osg::Cone* cone = new osg::Cone( getAs3D( geodeEndVec ), 1.0f, 5.0f );

    // let osg calculate the neccessary rotation matrix and rotate the shapes
    osg::Quat rotation;
    rotation.makeRotate( osg::Vec3d( 0.0, 0.0, 1.0 ),
                    osg::Vec3d( ray.direction().x(), ray.direction().y(), ray.direction().z() )
                );
    cylinder->setRotation( rotation );
    cone->setRotation( rotation );

    // draw it
    rayGeode->addDrawable( new osg::ShapeDrawable( cylinder ) );
    rayGeode->addDrawable( new osg::ShapeDrawable( cone ) );

    // end progress
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
        if( ray.direction()[coord] == 0 ) // ray parallel to the min/max coordinate planes
        {
            if( ray.start()[coord] < m_outer_bounding[0][coord] || ray.start()[coord] > m_outer_bounding[1][coord] )
            {
                return result; // ray not within min and max values of current coordinate
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
                 return result; // cube behind ray - possible case here (because of unknown click position)
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
//     if( m_DeriIsValid )
//     {
//         // not usable yet (has to be interpolated)
//         return static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( getAs3D( position ) ) );
//     }

    // voxel distance in each direction (usually 1)
    double dist_x = m_grid->getOffsetX();
    double dist_y = m_grid->getOffsetY();
    double dist_z = m_grid->getOffsetZ();
    double x_val, y_val, z_val;

    // central difference quotient in each direction
    x_val = ( ( interpolate( position + WVector4d( 0.5 * dist_x, 0.0, 0.0, 0.0 ), m_grid, m_dataSet ) )
            - ( interpolate( position - WVector4d( 0.5 * dist_x, 0.0, 0.0, 0.0 ), m_grid, m_dataSet ) ) ) / dist_x;
    y_val = ( ( interpolate( position + WVector4d( 0.0, 0.5 * dist_y, 0.0, 0.0 ), m_grid, m_dataSet ) )
            - ( interpolate( position - WVector4d( 0.0, 0.5 * dist_y, 0.0, 0.0 ), m_grid, m_dataSet ) ) ) / dist_y;
    z_val = ( ( interpolate( position + WVector4d( 0.0, 0.0, 0.5 * dist_z, 0.0 ), m_grid, m_dataSet ) )
            - ( interpolate( position - WVector4d( 0.0, 0.0, 0.5 * dist_z, 0.0 ), m_grid, m_dataSet ) ) ) / dist_z;

    return WVector4d( x_val, y_val, z_val, 0 );
}

double WMTransferCalc::interpolate( const WVector4d& position, boost::shared_ptr< WGridRegular3D > inter_grid,
                                                               boost::shared_ptr< WDataSetScalar > inter_dataSet )
{
    // determine center of hit voxel
    double vox_x = static_cast< double >( inter_grid->getXVoxelCoord( getAs3D( position ) ) );
    double vox_y = static_cast< double >( inter_grid->getYVoxelCoord( getAs3D( position ) ) );
    double vox_z = static_cast< double >( inter_grid->getZVoxelCoord( getAs3D( position ) ) );
    WVector4d voxel_center( vox_x, vox_y, vox_z, 1 );

    // distance between voxels in each direction (usually 1)
    double x_offset = inter_grid->getOffsetX();
    double y_offset = inter_grid->getOffsetY();
    double z_offset = inter_grid->getOffsetZ();
//     debugLog() << "Offsets: " << x_offset << ", " << y_offset << ", " << z_offset;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Calculating all relevant neighbours for the interpolation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // vector with all 8 direct neighbours
    std::vector< WVector4d > neighbours( 8 );
    // vectors with min / max coordinates of neighbours { min_x, min_y, min_z }
    std::vector< double > min_val( 3 );
    std::vector< double > max_val( 3 );

    // determine quadrant for trilinear interpolation to choose right neighbours
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

    // check, if all neighbours are inside the grid and get the corresponding value
    for( unsigned int i = 0; i < neighbours.size(); i++)
    {
        if( !inter_grid->encloses( getAs3D( neighbours[i] ) ) )
        {
            values[i] = 0;  // temporary solution
        }
        else
        {
            values[i] = inter_dataSet->getValueSet()->getScalarDouble( inter_grid->getVoxelNum( getAs3D( neighbours[i] ) ) );
        }
    }

    // trilinear interpolation from wikipedia
    double x_dif = ( position[0] - min_val[0] ) / ( max_val[0] - min_val[0] );
    double y_dif = ( position[1] - min_val[1] ) / ( max_val[1] - min_val[1] );
    double z_dif = ( position[2] - min_val[2] ) / ( max_val[2] - min_val[2] );

    double c_00  = values[0] * ( 1 - x_dif ) + values[1] * x_dif;
    double c_10  = values[2] * ( 1 - x_dif ) + values[3] * x_dif;
    double c_01  = values[4] * ( 1 - x_dif ) + values[5] * x_dif;
    double c_11  = values[6] * ( 1 - x_dif ) + values[7] * x_dif;

    double c_0   = c_00 * ( 1 - y_dif ) + c_10 * y_dif;
    double c_1   = c_01 * ( 1 - y_dif ) + c_11 * y_dif;

    return c_0 * ( 1 - z_dif ) + c_1 * z_dif;
}

void WMTransferCalc::calculateCurvature()
{
    // additional data for derivation must be available
    if( !m_DeriIsValid )
    {
        return;
    }
    // init progress for GUI
    boost::shared_ptr< WProgress > calcCurvProg = boost::shared_ptr< WProgress >( new WProgress( "Calculating curvature values." ) );
    m_progress->addSubProgress( calcCurvProg );

    // if this is false, the method of the paper will be used
    //bool useEigenvalues = true; //TODO(fjacob): not working right now

    // scales of the current grid
    double x_scale = m_deriGrid->getNbCoordsX();
    double y_scale = m_deriGrid->getNbCoordsY();
    double z_scale = m_deriGrid->getNbCoordsZ();
    // voxel dimensions (usually 1 in each direction)
    double dist_x  = m_deriGrid->getOffsetX();
    double dist_y  = m_deriGrid->getOffsetY();
    double dist_z  = m_deriGrid->getOffsetZ();

    // vector for creating dataset with the calculated mean curvature values
    boost::shared_ptr< std::vector< double > > mean_values = boost::shared_ptr< std::vector< double > >(
                                                            new std::vector< double >( m_deriDataSet->getValueSet()->size(), 0.0 )
                                                        );
    // vector for creating dataset with the calculated gaussian curvature values
    boost::shared_ptr< std::vector< double > > gauss_values = boost::shared_ptr< std::vector< double > >(
                                                            new std::vector< double >( m_deriDataSet->getValueSet()->size(), 0.0 )
                                                        );
    // eigenvalues which will be calculated
    double k1, k2, k3;
    // index of current position in the grid to adopt the same position in the new datagrid
    size_t id = 0;
    // vectors for all neighbour positions for derivation (in x/y/z direction)
    std::vector< WVector3d > derivDirection( 6 );
    // calculated position in the current grid (negative, if outside grid)
    std::vector< int > derivIds( 6 );

    //debugLog() << "Test " << x_scale << " - " << y_scale << " - " << z_scale ;
    //debugLog() << "Size: " << m_deriDataSet->getValueSet()->size();
    //debugLog() << "Rawsize: " << m_deriDataSet->getValueSet()->rawSize();
    //debugLog() << "Distances: " << dist_x << "-" << dist_x << "-" << dist_x;

    // traverse dataset/grid
    for( unsigned int zdir = 0; zdir < z_scale; zdir++ )
    {
        for( unsigned int ydir = 0; ydir < y_scale; ydir++ )
        {
            for( unsigned int xdir = 0; xdir < x_scale; xdir++ )
            {
                WVector3d position( xdir, ydir, zdir );
                // get gradient of current position
                WVector3d curGrad = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( id ) );
                // skip grid point, if there is no gradient
                if( length( curGrad ) == 0 )
                {
                    id++;
                    continue;
                }
                //size_t id = m_deriGrid->getVoxelNum( position );
                //debugLog() << id;

                // calculate grid ids of neighbours (will be -1 if outside of the grid)
                derivIds[0] = m_deriGrid->getVoxelNum( position + WVector3d( -dist_x,  0,  0 ) );
                derivIds[1] = m_deriGrid->getVoxelNum( position + WVector3d(  dist_x,  0,  0 ) );
                derivIds[2] = m_deriGrid->getVoxelNum( position + WVector3d(  0, -dist_y,  0 ) );
                derivIds[3] = m_deriGrid->getVoxelNum( position + WVector3d(  0,  dist_y,  0 ) );
                derivIds[4] = m_deriGrid->getVoxelNum( position + WVector3d(  0,  0, -dist_z ) );
                derivIds[5] = m_deriGrid->getVoxelNum( position + WVector3d(  0,  0,  dist_z ) );
                // get gradient vectors for each neighbour if this neighbour is inside the grid
                derivDirection[0] = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( ( derivIds[0] == -1 ) ? id : derivIds[0] ) );
                derivDirection[1] = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( ( derivIds[1] == -1 ) ? id : derivIds[1] ) );
                derivDirection[2] = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( ( derivIds[2] == -1 ) ? id : derivIds[2] ) );
                derivDirection[3] = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( ( derivIds[3] == -1 ) ? id : derivIds[3] ) );
                derivDirection[4] = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( ( derivIds[4] == -1 ) ? id : derivIds[4] ) );
                derivDirection[5] = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( ( derivIds[5] == -1 ) ? id : derivIds[5] ) );

                // derivate in each direction again (central differential quotient) to get hessian matrix
                Eigen::Matrix3d hesse;
                            // x-values form x,y,z direction
                hesse <<    ( derivDirection[1][0] - derivDirection[0][0] ) / 2.0 * dist_x,
                            ( derivDirection[3][0] - derivDirection[2][0] ) / 2.0 * dist_y,
                            ( derivDirection[5][0] - derivDirection[4][0] ) / 2.0 * dist_z,
                            // y-values form x,y,z direction
                            ( derivDirection[1][1] - derivDirection[0][1] ) / 2.0 * dist_x,
                            ( derivDirection[3][1] - derivDirection[2][1] ) / 2.0 * dist_y,
                            ( derivDirection[5][1] - derivDirection[4][1] ) / 2.0 * dist_z,
                            // z-values form x,y,z direction
                            ( derivDirection[1][2] - derivDirection[0][2] ) / 2.0 * dist_x,
                            ( derivDirection[3][2] - derivDirection[2][2] ) / 2.0 * dist_y,
                            ( derivDirection[5][2] - derivDirection[4][2] ) / 2.0 * dist_z;
                //debugLog() << hesse;

                // currently not working in another way
                if( true /*useEigenvalues*/ )
                {
                    // let Eigen calculate eigen values
                    Eigen::EigenSolver< Eigen::Matrix3d > eigenResults( hesse, false );
//                     debugLog()  << "The eigenvalues of the 3x3 matrix are:"
//                                 << std::endl << eigenResults.eigenvalues()
//                                 << std::endl << std::endl << eigenResults.eigenvalues()[0].real();
                    k1 = eigenResults.eigenvalues()[0].real();
                    k2 = eigenResults.eigenvalues()[1].real();
                    k3 = eigenResults.eigenvalues()[2].real();
                    // sort absolute eigenvalues
                    if( std::abs( k1 ) <= std::abs( k2 ) ) std::swap( k1, k2 );
                    if( std::abs( k2 ) <= std::abs( k3 ) ) std::swap( k2, k3 );
                    if( std::abs( k1 ) <= std::abs( k2 ) ) std::swap( k1, k2 );
                    //debugLog() << k1 << " - " << k2 << " - " << k3;

                    // calculate mean and gaussian curvature
                    (*mean_values)[id] = ( k1 + k2 ) * 0.5;
                    (*gauss_values)[id] =  k1 * k2;
                    id++;
                }
                else
                {
                    // method of the paper
                    // we need:
                    //      current gradient g
                    //      normal n = -g/|g|
                    //      P = E - nn^T
                    //      nabla-n^T = -PH / |g|   (where H is the hessian matrix)
                    //      G = nabla-n^T*P
                    WVector3d g = static_cast< WVector3d >( m_deriDataSet->getValueSet()->getWVector( m_deriGrid->getVoxelNum( position ) ) );
                    double g_weight = length( g );
                    WVector3d n = ( g * -1 ) / g_weight;
                    Eigen::Matrix3d E, nnT, P, nabla_nT, G, help;
                    E <<    1, 0, 0,
                            0, 1, 0,
                            0, 0, 1;
                    help << 1, 0, 0,
                            0, 1, 0,
                            0, 0, 0;
                    nnT <<  n[0] * n[0], n[0] * n[1], n[0] * n[2],
                            n[1] * n[0], n[1] * n[1], n[1] * n[2],
                            n[2] * n[0], n[2] * n[1], n[2] * n[2];
                    P = E - nnT;
                    nabla_nT = ( P * hesse ) * ( -1 / g_weight );
                    G = nabla_nT * help;

                    debugLog()  << "g  : " << g << std::endl
                                << "g_l: " << g_weight << std::endl
                                << "n  : " << n << std::endl
                                << "E  : " << E << std::endl
                                << "nnT: " << nnT << std::endl
                                << "P  : " << P << std::endl
                                << "nab: " << nabla_nT << std::endl
                                << "G  : " << G << std::endl;
                }
            }
        }
    }
    // corresponding valueset to calculated list of values for mean curvature
    boost::shared_ptr< WValueSet< double > > mean_valueset = boost::shared_ptr< WValueSet< double > >(
                                                            new WValueSet< double >( 0, 1, mean_values, W_DT_DOUBLE )
                                                        );
    // corresponding valueset to calculated list of values for gaussian curvature
    boost::shared_ptr< WValueSet< double > > gauss_valueset = boost::shared_ptr< WValueSet< double > >(
                                                            new WValueSet< double >( 0, 1, gauss_values, W_DT_DOUBLE )
                                                        );
    // create mean curvature output data with calculated valueset
    m_meanCurvDataSet = boost::shared_ptr< WDataSetScalar >( new WDataSetScalar( mean_valueset, m_deriGrid ) );
    m_curveMeanOut->updateData( m_meanCurvDataSet );
    // create gaussian curvature output data with calculated valueset
    m_gaussCurvDataSet = boost::shared_ptr< WDataSetScalar >( new WDataSetScalar( gauss_valueset, m_deriGrid ) );
    m_curveGaussOut->updateData( m_gaussCurvDataSet );
    // end progress
    calcCurvProg->finish();
    m_progress->removeSubProgress( calcCurvProg );
}

// not in use right now, but maybe later
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
