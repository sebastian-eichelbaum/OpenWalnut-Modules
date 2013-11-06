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
#include <vector>

#include "core/dataHandler/WDataHandler.h"
#include "core/dataHandler/WDataSetFibers.h"
#include "core/dataHandler/WSubject.h"
#include "core/dataHandler/exceptions/WDHNoSuchSubject.h"
#include "core/graphicsEngine/WGEUtils.h"
#include "core/kernel/WKernel.h"
#include "WMSliceContext.h"
#include "WTransparentLinesDrawable.h"
#include "slicecontext.xpm"

WMSliceContext::WMSliceContext():
    WModule()
{
}

WMSliceContext::~WMSliceContext()
{
    // Cleanup!
}

boost::shared_ptr< WModule > WMSliceContext::factory() const
{
    // See "src/modules/template/" for an extensively documented example.
    return boost::shared_ptr< WModule >( new WMSliceContext() );
}

const char** WMSliceContext::getXPMIcon() const
{
    return slicecontext_xpm;
}

const std::string WMSliceContext::getName() const
{
    // Specify your module name here. This name must be UNIQUE!
    return "Slice Context";
}

const std::string WMSliceContext::getDescription() const
{
    return "Show the spatial context of the slices.";
}

void WMSliceContext::connectors()
{
    // The input fiber dataset
    m_fiberInput = boost::shared_ptr< WModuleInputData < const WDataSetFibers > >(
        new WModuleInputData< const WDataSetFibers >( shared_from_this(), "fibers", "The fiber dataset to use as context." )
    );

    // As properties, every connector needs to be added to the list of connectors.
    addConnector( m_fiberInput );

    // call WModules initialization
    WModule::connectors();
}

void WMSliceContext::properties()
{
    m_propCondition = boost::shared_ptr< WCondition >( new WCondition() );

    WPosition ch = WKernel::getRunningKernel()->getSelectionManager()->getCrosshair()->getPosition();
    m_crosshairProp = m_infoProperties->addProperty( "Slice Position", "Position where the three navigation slices intersect.", ch, m_propCondition );

    m_insideCountProp =
        m_properties
        ->addProperty( "Inside count",
                       "The number of positions of a fiber that have to be inside the context for the fiber to be considered.",
                       1,
                       m_propCondition );
    m_contextWidthProp = m_properties->addProperty( "Context width",
                                                    "The width of the area around the slice that is considere as context.",
                                                    1.0,
                                                    m_propCondition );
}

void WMSliceContext::moduleMain()
{
    m_moduleState.setResetable( true, true );
    m_moduleState.add( m_fiberInput->getDataChangedCondition() );
    m_moduleState.add( m_propCondition );

    ready();

    m_rootNode = new osg::Group();
    m_shaderFibers = osg::ref_ptr< WGEShader > ( new WGEShader( "WMSliceContext", m_localPath ) );

    // main loop
    while( !m_shutdownFlag() )
    {
        debugLog() << "Waiting ...";
        m_moduleState.wait();

        // woke up since the module is requested to finish?
        if( m_shutdownFlag() )
        {
            break;
        }

        // To query whether an input was updated, simply ask the input:
        bool dataUpdated = m_fiberInput->updated();
        boost::shared_ptr< const WDataSetFibers > fibers( m_fiberInput->getData() );
        bool dataValid = fibers;

        if( !( dataValid && dataUpdated ) )
        {
            continue;
        }

        // findFirstFiberDataSet();
        m_tracts = fibers;
        WAssert( m_tracts, "No fiber data set found." );

        boost::shared_ptr< WProgress > progress = boost::shared_ptr< WProgress >( new WProgress( "Slice Context", 1 ) );
        m_progress->addSubProgress( progress );

        m_rootNode->addUpdateCallback( new WMSliceContext::SliceContextUpdateCallback( this ) );

        WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->insert( m_rootNode );

        progress->finish();
    }

    // Since the modules run in a separate thread: wait
    waitForStop();

    WKernel::getRunningKernel()->getGraphicsEngine()->getScene()->remove( m_rootNode );
}

void WMSliceContext::SliceContextUpdateCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    m_module->update();
    traverse( node, nv );
}


osg::ref_ptr< WGEManagedGroupNode > WMSliceContext::paintTracts( const std::vector< size_t >& selectedTracts ) const
{
    osg::ref_ptr< WGEManagedGroupNode > result = osg::ref_ptr< WGEManagedGroupNode >( new WGEManagedGroupNode( m_active ) );
    result->insert( genTractGeode( selectedTracts ).get() );
    return result;
}

osg::ref_ptr< osg::Geode > WMSliceContext::genTractGeode( const std::vector< size_t >& selectedTracts ) const
{
    using osg::ref_ptr;
    ref_ptr< osg::Vec3Array > vertices = ref_ptr< osg::Vec3Array >( new osg::Vec3Array );
    ref_ptr< WTransparentLinesDrawable > geometry = ref_ptr< WTransparentLinesDrawable >( new WTransparentLinesDrawable );
    geometry->setDataVariance( osg::Object::DYNAMIC );


    boost::shared_ptr< std::vector < size_t > > lineLengths = m_tracts->getLineLengths();
    size_t  insideCountLocal = m_insideCountProp->get();

    // for( size_t fiberId = 0; fiberId < selectedTracts.size(); ++fiberId )
    // {
    //     if( selectedTracts[fiberId] >= insideCountLocal )
    //     {
    //         size_t fiberLength = ( *lineLengths )[fiberId];
    //         for( size_t posId = 0; posId < fiberLength; ++posId )
    //         {
    //             WPosition pos;
    //             vertices->push_back( m_tracts->getPosition( fiberId, posId     ) );

    //         }
    //         geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINE_STRIP, vertices->size() - fiberLength, fiberLength ) );
    //     }
    // }

    for( size_t fiberId = 0; fiberId < selectedTracts.size(); ++fiberId )
    {
        if( selectedTracts[fiberId] >= insideCountLocal )
        {
            size_t fiberLength = ( *lineLengths )[fiberId];
            for( size_t posId = 0; posId < fiberLength - 1 ; ++posId )
            {
                WPosition pos;
                vertices->push_back( m_tracts->getPosition( fiberId, posId     ) );
                vertices->push_back( m_tracts->getPosition( fiberId, posId + 1 ) );
            }
        }
    }
    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, vertices->size()  ) );
    geometry->setUseDisplayList( false );
    geometry->setUseVertexBufferObjects( false );
    geometry->setVertexArray( vertices );

    ref_ptr< osg::Vec4Array > colors = ref_ptr< osg::Vec4Array >( new osg::Vec4Array );
    colors->push_back( WColor( 1.0, 0.0, 1.0, 1.0 ) );
    geometry->setColorArray( colors );
    geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
    osg::ref_ptr< osg::Geode > geode = osg::ref_ptr< osg::Geode >( new osg::Geode );
    geode->addDrawable( geometry.get() );

    osg::Vec3Array* texCoords = new osg::Vec3Array;
    texCoords->clear();
    for( size_t i = 0; i < vertices->size(); ++i )
    {
        double distance = fabs( m_crosshairProp->get()[0] - ( *vertices )[i][0] );
        texCoords->push_back( WPosition( distance, m_contextWidthProp->get(), 0.0 ) );
    }
    geometry->setTexCoordArray( 0, texCoords );

    osg::StateSet* state = geode->getOrCreateStateSet();
    state->setMode(  GL_BLEND, osg::StateAttribute::ON  );

    m_shaderFibers->apply( geode );
    geode->setDataVariance( osg::Object::DYNAMIC );

    return geode;
}

void WMSliceContext::checkContainment( std::vector< size_t >* selected, bool counting, double distance ) const
{
    boost::shared_ptr< std::vector < size_t > > lineLengths = m_tracts->getLineLengths();
    for( size_t fiberId = 0; fiberId < lineLengths->size(); ++fiberId )
    {
        for( size_t posId = 0; posId < ( *lineLengths )[fiberId]; ++posId )
        {
            WPosition pos = m_tracts->getPosition( fiberId, posId );
            if( fabs( m_crosshairProp->get()[0] - pos[0] ) < distance )
            {
                ++( ( *selected )[fiberId] );
                if( !counting )
                {
                    break;
                }
            }
        }
    }
}

void WMSliceContext::update()
{
    m_crosshairProp->set( WKernel::getRunningKernel()->getSelectionManager()->getCrosshair()->getPosition() );
        // TODO(wiebel): change to complete set of slices
    if( m_current[0] == m_crosshairProp->get()[0] && !m_insideCountProp->changed() && !m_contextWidthProp->changed() )
    {
        // nothing changed --> nothing to do
        return;
    }
    std::vector< size_t > selected( m_tracts->size(), 0 );
    const bool counting =  ( m_insideCountProp->get( true ) > 1 );
    checkContainment( &selected, counting, m_contextWidthProp->get( true ) );

    m_rootNode->removeChild( m_osgNode );
    m_osgNode = paintTracts( selected );
    m_rootNode->addChild( m_osgNode );

    m_current = WKernel::getRunningKernel()->getSelectionManager()->getCrosshair()->getPosition();
}
