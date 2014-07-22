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

#include "core/common/WCondition.h"
#include "core/common/WPropTransfer.h"
#include "core/common/WItemSelectionItemTyped.h"
#include "core/graphicsEngine/WGEManagedGroupNode.h"
#include "core/kernel/WModuleInputData.h"
#include "WMAbstractSliceModule.h"

WMAbstractSliceModule::WMAbstractSliceModule()
    : WModule(),
      m_propCondition( new WCondition() )
{
}

WMAbstractSliceModule::~WMAbstractSliceModule()
{
}

void WMAbstractSliceModule::connectors()
{
    m_posIC = WModuleInputData< WPositionTransfer >::createAndAdd( shared_from_this(), "positions", "Slice positions in x,y,z" );

    WModule::connectors();
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
