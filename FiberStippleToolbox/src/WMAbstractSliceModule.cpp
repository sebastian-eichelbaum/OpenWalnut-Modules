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

#include <boost/bind.hpp>

#include "core/common/WCondition.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WKernel.h"
#include "core/kernel/WSelectionManager.h"
#include "WMAbstractSliceModule.h"

WMAbstractSliceModule::WMAbstractSliceModule()
    : WModule(),
      m_propCondition( new WCondition() )
{
    // subscribe to slice positions of the kernel. Whenever they change, the currently selected axis is choosen to update m_pos
    WKernel::getRunningKernel()->getSelectionManager()->getPropAxialPos()->getUpdateCondition()->subscribeSignal( boost::bind( &WMAbstractSliceModule::updatePos, this ) );
    WKernel::getRunningKernel()->getSelectionManager()->getPropSagittalPos()->getUpdateCondition()->subscribeSignal( boost::bind( &WMAbstractSliceModule::updatePos, this ) );
    WKernel::getRunningKernel()->getSelectionManager()->getPropCoronalPos()->getUpdateCondition()->subscribeSignal( boost::bind( &WMAbstractSliceModule::updatePos, this ) );
}

WMAbstractSliceModule::~WMAbstractSliceModule()
{
}

void WMAbstractSliceModule::connectors()
{
    WModule::connectors();
}

void WMAbstractSliceModule::updatePos()
{
    WPropDouble pos;
    switch( m_sliceSelection->get( true ).at( 0 )->getAs< AxisType >()->getValue() )
    {
      case 0 : pos = WKernel::getRunningKernel()->getSelectionManager()->getPropSagittalPos(); break;
      case 1 : pos = WKernel::getRunningKernel()->getSelectionManager()->getPropCoronalPos(); break;
      case 2 : pos = WKernel::getRunningKernel()->getSelectionManager()->getPropAxialPos(); break;
      default : warnLog() << "This indicates a bug: no valid axis selected";
                return;
    }
    double offset = 0.001; // This offset is used to put the graphics slightly over the slice position not onto the same, as then
    // graphics would start to flicker

    m_pos->set( pos->get() + offset );
}

void WMAbstractSliceModule::properties()
{
    m_pos = m_properties->addProperty( "Slice Position", "Where the data shoulde be sliced for drawing contours", 0.0 );

    m_axes = boost::shared_ptr< WItemSelection >( new WItemSelection() );
    m_axes->addItem( AxisType::create( 2, "Axial", "xy-slice" ) );
    m_axes->addItem( AxisType::create( 1, "Coronal", "xz-slice" ) );
    m_axes->addItem( AxisType::create( 0, "Sagittal", "yz-slice" ) );
    m_sliceSelection = m_properties->addProperty( "Slice:",  "Which slice (axial, coronal or sagittal)?", m_axes->getSelector( 1 ), m_propCondition );
    WPropertyHelper::PC_SELECTONLYONE::addTo( m_sliceSelection );

    WModule::properties();
}

size_t WMAbstractSliceModule::selectAxis( const std::string& name ) const
{
    if( name == "Axial Slice" )
    {
        return 0;
    }
    else if( name == "Coronal Slice" )
    {
        return 1;
    }
    else if( name == "Sagittal Slice" )
    {
        return 2;
    }
    else // undefined
    {
        return wlimits::MAX_SIZE_T;
    }
}

std::pair< WVector3d, WVector3d > WMAbstractSliceModule::sliceBaseVectors( const WVector3d& sizes, const size_t axis ) const
{
    std::pair< WVector3d, WVector3d > result;
    result.first = WVector3d( sizes );
    result.first[axis] = 0.0;
    result.second = WVector3d( result.first );
    result.first[ axis == 2 ? 1 : 2 ] = 0.0;
    result.second[ axis == 0 ? 1 : 0 ] = 0.0;
    return result;
}
