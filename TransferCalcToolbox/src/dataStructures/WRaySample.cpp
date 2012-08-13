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

#include "WRaySample.h"

WRaySample::WRaySample():
    m_value( 0.0 ), m_distance( -1.0 ), m_gradient( WVector4d( 0.0, 0.0, 0.0, 0.0 ) ), m_gradWeight( -1.0 ), m_fracAnisotropy( -1.0 )
{
    // initialize members
}

WRaySample::~WRaySample()
{
    // cleanup
}

const double& WRaySample::value() const
{
    return m_value;
}

double& WRaySample::value()
{
    return m_value;
}

const double& WRaySample::distance() const
{
    return m_distance;
}

double& WRaySample::distance()
{
    return m_distance;
}

const WVector4d& WRaySample::gradient() const
{
    return m_gradient;
}

WVector4d& WRaySample::gradient()
{
    return m_gradient;
}

const double& WRaySample::gradWeight() const
{
    return m_gradWeight;
}

double& WRaySample::gradWeight()
{
    return m_gradWeight;
}

const double& WRaySample::fracA() const
{
    return m_fracAnisotropy;
}

double& WRaySample::fracA()
{
    return m_fracAnisotropy;
}

const double& WRaySample::angle() const
{
    return m_angle;
}

double& WRaySample::angle()
{
    return m_angle;
}
