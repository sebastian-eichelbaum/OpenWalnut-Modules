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

// This line is needed by the module loader to actually find your module. You need to add this to your module too. Do NOT add a ";" here.
// W_LOADABLE_MODULE( WMTransferCalc )

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
//     m_isoValue      = m_properties->addProperty( "ISO-Value", "Iso value to be rendered.", 100.0, m_propCondition );
//     m_epsilon       = m_properties->addProperty( "Epsilon", "Epsilon value to define the iso range.", 1.0, m_propCondition );
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
        boost::shared_ptr< WDataSetScalar > dataSet = m_inputData->getData();
        bool dataValid = ( dataSet );

        if( !dataValid )
        {
            debugLog() << "Data changed. No valid data anymore. Cleaning up.";
            WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
        }
        
        bool propsChanged = m_xPos->changed() || m_yPos->changed();  //m_isoValue->changed() || m_epsilon->changed();
        
        if( ( propsChanged || dataUpdated ) && dataValid )
        {
            debugLog() << "Received Data or Data changed.";
    
//             double min = dataSet->getMin();
//             double max = dataSet->getMax();
// 
//             m_isoValue->setMin( min );
//             m_isoValue->setMax( max );
//             m_isoValue->setRecommendedValue( min + ( 0.5 * max ) );
            
//            m_epsilon->setMin( 0 );
//            m_epsilon->setMax( 0.1 * (max - min) );
//            m_epsilon->setRecommendedValue( 0.005 * (max - min) );
            
//             double eps = m_epsilon->get( true );
//             double iso = m_isoValue->get( true );
//             
//             debugLog() << "Given epsilon: " << eps;
//             debugLog() << "Given iso value: " << iso;
            
            osg::ref_ptr< osg::Geode > newGeode = new osg::Geode();
            
            // grab the grid
            boost::shared_ptr< WGridRegular3D > grid = boost::shared_dynamic_cast< WGridRegular3D >( dataSet->getGrid() );
            if( !grid )
            {
                errorLog() << "The dataset does not provide a regular grid. Ignoring dataset.";
                continue;
            }
            
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Getting Data and Setting Properties
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            
            unsigned int x_scale = grid->getNbCoordsX();
            unsigned int y_scale = grid->getNbCoordsY();
            unsigned int z_scale = grid->getNbCoordsZ();
            
            debugLog() << "x = " << x_scale << ", y = " << y_scale << ", z = " << z_scale;
            
            m_xPos->setMin( 0 );
            m_xPos->setMax( x_scale );
            m_xPos->setRecommendedValue( 0.5 * x_scale );
            
            m_yPos->setMin( 0 );
            m_yPos->setMax( y_scale );
            m_yPos->setRecommendedValue( 0.5 * y_scale );

//            size_t s = dataSet->getValueSet()->rawSize();            
//            debugLog() << "Raw size of ValueSet: " << s ;
            
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Calculating Ray
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            
            // start point and direction of the ray
            double x = m_xPos->get( true ); 
            double y = m_yPos->get( true );
            double z = 0;
            
            WVector3d start( x, y, z );
            debugLog() << "Start: " << start;
            
            WVector3d dir( 0, 0, 1 );
            debugLog() << "Direction: " << dir;
            
            // ray object
            // ray = start + t * direction
            WRay ray ( start, dir );
            
            // step length
            double interval = 1;
            double distance = 0;
            
            // progress
            boost::shared_ptr< WProgress > prog = boost::shared_ptr< WProgress >( new WProgress( "Casting ray." ) );
            m_progress->addSubProgress( prog );
            
            bool calc = true;
            while( calc )
            {
                WVector3d current = ray.getSpot( distance );
                distance += interval;
                debugLog() << "Point: " << current;
                
                // do not calculate anything for vectors outside of the data grid
                if( current[0] > x_scale - 0.5 || current[1] > y_scale - 0.5 || current[2] > z_scale - 0.5 )
                {
                    calc = false;
                    break;
                }
                
                // static_cast< osg::Vec3 >( ~ )
                
                newGeode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( current, 0.5f ) ) );
                
                double val = dataSet->getValueSet()->getScalarDouble( grid->getVoxelNum( current ) );
                debugLog() << "Value: " << val;

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