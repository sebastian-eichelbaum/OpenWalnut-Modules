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
#include "WMTempRandomPoints.xpm"
#include "WMTempRandomPoints.h"
#include "../common/datastructures/octree/WOctree.h"

// This line is needed by the module loader to actually find your module.
//W_LOADABLE_MODULE( WMTempRandomPoints )
//TODO(aschwarzkopf): Reenable above after solving the toolbox problem

WMTempRandomPoints::WMTempRandomPoints():
    WModule(),
    m_propCondition( new WCondition() )
{
}

WMTempRandomPoints::~WMTempRandomPoints()
{
    m_currentRandomNumber = 0.0;
}

boost::shared_ptr< WModule > WMTempRandomPoints::factory() const
{
    return boost::shared_ptr< WModule >( new WMTempRandomPoints() );
}

const char** WMTempRandomPoints::getXPMIcon() const
{
    return WMTempRandomPoints_xpm;
}
const std::string WMTempRandomPoints::getName() const
{
    return "Random Points";
}

const std::string WMTempRandomPoints::getDescription() const
{
    return "Crops point data to a selection.";
}

void WMTempRandomPoints::connectors()
{
    m_output = boost::shared_ptr< WModuleOutputData< WDataSetPoints > >(
                new WModuleOutputData< WDataSetPoints >(
                        shared_from_this(), "points", "The loaded points." ) );

    addConnector( m_output );
    WModule::connectors();
}

void WMTempRandomPoints::properties()
{
    m_groupRandomPointGenerator = m_properties->addPropertyGroup( "Random point generator settings",
                                            "Options that are applied on the random point generator." );
    m_isSphericalSpace = m_groupRandomPointGenerator->addProperty( "Is spherical: ", "Organize points in a spherical space.",
                                                    false, m_propCondition );
    m_pointCount = m_groupRandomPointGenerator->addProperty( "Random point count: ", "Count of random points.",
                                                    5, m_propCondition );
    m_pointCount->setMin( 1 );
    m_pointCount->setMax( 1000 );
    m_initRandomNumber = m_groupRandomPointGenerator->addProperty( "Init. random: ", "Number that initializes the "
                                                    "random number generator.", 234.0, m_propCondition );
    m_initRandomNumber->setMin( 0.0 );
    m_initRandomNumber->setMax( 1000.0 );
    WModule::properties();
}

void WMTempRandomPoints::requirements()
{
}

void WMTempRandomPoints::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_propCondition );

    ready();

    // graphics setup
    m_rootNode = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );


    while( !m_shutdownFlag() )
    {
        m_moduleState.wait();


        m_output->updateData( getRandomPoints() );


        if  ( m_shutdownFlag() )
        {
            break;
        }
    }

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}
void WMTempRandomPoints::setProgressSettings( size_t steps )
{
    m_progress->removeSubProgress( m_progressStatus );
    std::string headerText = "Loading data";
    m_progressStatus = boost::shared_ptr< WProgress >( new WProgress( headerText, steps ) );
    m_progress->addSubProgress( m_progressStatus );
}
boost::shared_ptr< WDataSetPoints > WMTempRandomPoints::getRandomPoints()
{
    setProgressSettings( m_pointCount->get() );

    WDataSetPoints::VertexArray outVertices(
            new WDataSetPoints::VertexArray::element_type() );
    WDataSetPoints::ColorArray outColors(
            new WDataSetPoints::ColorArray::element_type() );

    std::cout << std::endl << "Applying point set:" << std::endl;
    m_currentRandomNumber = m_initRandomNumber->get();
    double colorIntensity = 0.8;
    size_t count = m_pointCount->get();
    size_t addedPoints = 0;
    while( addedPoints < count )
    {
        double x = getNextRandomNumber() * 2.0 - 1.0;
        double y = getNextRandomNumber() * 2.0 - 1.0;
        double z = getNextRandomNumber() * 2.0 - 1.0;

        double distance = pow( pow( x, 2.0 ) + pow( y, 2.0 ) + pow( z, 2.0 ), 0.5 );
        if( distance <= 1.0 || !m_isSphericalSpace->get() )
        {
            outVertices->push_back( x );
            outVertices->push_back( y );
            outVertices->push_back( z );
            for( size_t item = 0; item < 3; item++ )
                outColors->push_back( colorIntensity );
            m_progressStatus->increment( 1 );
            addedPoints++;
        }
    }

    boost::shared_ptr< WDataSetPoints > outputPoints(
            new WDataSetPoints( outVertices, outColors ) );

    m_progressStatus->finish();
    return outputPoints;
}
double WMTempRandomPoints::getNextRandomNumber()
{
    if( m_currentRandomNumber == 0.0 )
        m_currentRandomNumber += 1.0;
    m_currentRandomNumber = sin( 8966.0 / m_currentRandomNumber );
    m_currentRandomNumber = asin( m_currentRandomNumber );
    m_currentRandomNumber = m_currentRandomNumber/acos( 0.0 ) / 2.0 + 0.5;
    return m_currentRandomNumber;
}
