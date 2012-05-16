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

#include "core/common/WAssert.h"

#include "WRayProfile.h"

WRayProfile::WRayProfile( size_t nbSamples )
{
    // initialize members

    // reserve some space for all the profiles.
    m_profile.resize( nbSamples );

    // NOTE: the number of samples does not need to be stored as it is the number of elements in m_profile.
}

WRayProfile::~WRayProfile()
{
    // cleanup
}

// void WRayProfile::addSample( WRaySample smp )
// {
//     m_profile.push_back( smp );
// }

const WRaySample& WRayProfile::operator[]( size_t sampleID ) const
{
    WAssert( sampleID >= m_profile.size(), "Invalid sample ID." );
    return m_profile[ sampleID ];
}

WRaySample& WRayProfile::operator[]( size_t sampleID )
{
    WAssert( sampleID >= m_profile.size(), "Invalid sample ID." );
    return m_profile[ sampleID ];
}
