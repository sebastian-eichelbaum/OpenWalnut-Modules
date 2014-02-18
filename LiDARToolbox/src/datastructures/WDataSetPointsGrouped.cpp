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

#include <algorithm>
#include <string>
#include <vector>

#include "core/common/exceptions/WOutOfBounds.h"
#include "core/common/WAssert.h"
#include "core/common/WColor.h"
#include "core/common/math/linearAlgebra/WPosition.h"

#include "WDataSetPointsGrouped.h"

// prototype instance as singleton
boost::shared_ptr< WPrototyped > WDataSetPointsGrouped::m_prototype = boost::shared_ptr< WPrototyped >();

WDataSetPointsGrouped::WDataSetPointsGrouped( WDataSetPointsGrouped::VertexArray vertices,
        WDataSetPointsGrouped::ColorArray colors, WDataSetPointsGrouped::GroupArray groups,
        WBoundingBox boundingBox ):
    m_vertices( vertices ),
    m_colors( colors ),
    m_groups( groups ),
    m_bb( boundingBox )
{
    WAssert( vertices->size() % 3 == 0, "Number of floats in the vertex array must be a multiple of 3" );
    if( colors )
    {
        size_t numPoints = vertices->size() / 3;
        WAssert( ( colors->size() / 4 == numPoints ) ||
                 ( colors->size() / 3 == numPoints ) ||
                 ( colors->size() / 1 == numPoints )
                , "Number of floats in the color array must be 1,3, or 4 per vertex" );
    }

    init();
}

WDataSetPointsGrouped::WDataSetPointsGrouped( WDataSetPointsGrouped::VertexArray vertices,
        WDataSetPointsGrouped::ColorArray colors, WDataSetPointsGrouped::GroupArray groups ):
    m_vertices( vertices ),
    m_colors( colors ),
    m_groups( groups )
{
    WAssert( vertices->size() % 3 == 0, "Number of floats in the vertex array must be a multiple of 3" );
    if( colors )
    {
        size_t numPoints = vertices->size() / 3;
        WAssert( ( colors->size() / 4 == numPoints ) ||
                 ( colors->size() / 3 == numPoints ) ||
                 ( colors->size() / 1 == numPoints )
                , "Number of floats in the color array must be 1,3, or 4 per vertex" );
    }

    init( true );
}

WDataSetPointsGrouped::WDataSetPointsGrouped()
{
    // dummy construction. Empty point and color list
}

WDataSetPointsGrouped::~WDataSetPointsGrouped()
{
    // cleanup
}

void WDataSetPointsGrouped::init( bool calcBB )
{
    // no colors specified? Use white as default
    if( !m_colors )
    {
        // Store 1 value for each point (gray scale colors)
        m_colors = ColorArray( new ColorArray::element_type( m_vertices->size() / 3, 1.0 ) );
    }

    // calculate the bounding box if needed
    if( calcBB && ( m_vertices->size() != 0 ) )
    {
        float minX = m_vertices->operator[]( 0 );
        float minY = m_vertices->operator[]( 1 );
        float minZ = m_vertices->operator[]( 2 );
        float maxX = minX;
        float maxY = minY;
        float maxZ = minZ;

        // go through each point
        for( size_t pointIdx = 3; pointIdx < m_vertices->size(); pointIdx+=3 )
        {
            minX = std::min( m_vertices->operator[]( pointIdx + 0 ), minX );
            minY = std::min( m_vertices->operator[]( pointIdx + 1 ), minY );
            minZ = std::min( m_vertices->operator[]( pointIdx + 2 ), minZ );
            maxX = std::max( m_vertices->operator[]( pointIdx + 0 ), maxX );
            maxY = std::max( m_vertices->operator[]( pointIdx + 1 ), maxY );
            maxZ = std::max( m_vertices->operator[]( pointIdx + 2 ), maxZ );
        }

        m_bb = WBoundingBox( minX, minY, minZ, maxX, maxY, maxZ );
    }

    // which colortype do we have?
    m_colorType = static_cast< ColorType >( m_colors->size() / ( m_vertices->size() / 3 ) );
}

size_t WDataSetPointsGrouped::size() const
{
    return m_vertices->size() / 3;
}

bool WDataSetPointsGrouped::isTexture() const
{
    return false;
}

const std::string WDataSetPointsGrouped::getName() const
{
    return "WDataSetPointsGrouped";
}

const std::string WDataSetPointsGrouped::getDescription() const
{
    return "Dataset which contains points without any topological relation.";
}

boost::shared_ptr< WPrototyped > WDataSetPointsGrouped::getPrototype()
{
    if( !m_prototype )
    {
        m_prototype = boost::shared_ptr< WPrototyped >( new WDataSetPointsGrouped() );
    }

    return m_prototype;
}

WDataSetPointsGrouped::VertexArray WDataSetPointsGrouped::getVertices() const
{
    return m_vertices;
}

WDataSetPointsGrouped::ColorArray WDataSetPointsGrouped::getColors() const
{
    return m_colors;
}

WDataSetPointsGrouped::GroupArray WDataSetPointsGrouped::getGroups() const
{
    return m_groups;
}

WBoundingBox WDataSetPointsGrouped::getBoundingBox() const
{
    return m_bb;
}

WPosition WDataSetPointsGrouped::operator[]( const size_t pointIdx ) const
{
    if( !isValidPointIdx( pointIdx ) )
    {
        throw WOutOfBounds( "The specified index is invalid." );
    }

    return WPosition( m_vertices->operator[]( pointIdx * 3 + 0 ),
                      m_vertices->operator[]( pointIdx * 3 + 1 ),
                      m_vertices->operator[]( pointIdx * 3 + 2 ) );
}

WPosition WDataSetPointsGrouped::getPosition( const size_t pointIdx ) const
{
    return operator[]( pointIdx );
}

WColor WDataSetPointsGrouped::getColor( const size_t pointIdx ) const
{
    if( !isValidPointIdx( pointIdx ) )
    {
        throw WOutOfBounds( "The specified index is invalid." );
    }

    switch( getColorType() )
    {
        case GRAY:
            return WColor( m_colors->operator[]( pointIdx * 1 + 0 ),
                           m_colors->operator[]( pointIdx * 1 + 0 ),
                           m_colors->operator[]( pointIdx * 1 + 0 ),
                           1.0 );
        case RGB:
            return WColor( m_colors->operator[]( pointIdx * 3 + 0 ),
                           m_colors->operator[]( pointIdx * 3 + 1 ),
                           m_colors->operator[]( pointIdx * 3 + 2 ),
                           1.0 );
        case RGBA:
            return WColor( m_colors->operator[]( pointIdx * 4 + 0 ),
                           m_colors->operator[]( pointIdx * 4 + 1 ),
                           m_colors->operator[]( pointIdx * 4 + 2 ),
                           m_colors->operator[]( pointIdx * 4 + 3 ) );
        default:
            return WColor( 1.0, 1.0, 1.0, 1.0 );
    }
}

bool WDataSetPointsGrouped::isValidPointIdx( const size_t pointIdx ) const
{
    return ( pointIdx < m_vertices->size() / 3 );
}

WDataSetPointsGrouped::ColorType WDataSetPointsGrouped::getColorType() const
{
    return m_colorType;
}
