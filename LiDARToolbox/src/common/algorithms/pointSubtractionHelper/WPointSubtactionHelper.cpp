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

#include <vector>
#include "WPointSubtactionHelper.h"


WPointSubtactionHelper::WPointSubtactionHelper()
{
    m_searchTree = new WKdTreeND( 3 );
    m_pointSearcher.setExaminedKdTree( m_searchTree );
    m_pointSearcher.setMaxResultPointCountInfinite();
    m_pointSearcher.setMaxSearchDistance( 0.0 );
}

WPointSubtactionHelper::~WPointSubtactionHelper()
{
}

void WPointSubtactionHelper::initSubtraction( boost::shared_ptr< WDataSetPoints > pointsToSubtract, double subtractionRadius )
{
    delete m_searchTree;
    m_searchTree = new WKdTreeND( 3 );

    if( !pointsToSubtract )
    {
        m_searchTree = 0;
        return;
    }
    vector<WKdPointND*>* points = new vector<WKdPointND*>();
    WDataSetPoints::VertexArray vertices = pointsToSubtract->getVertices();
    if( vertices->size() == 0 )
    {
        m_searchTree = 0;
        return;
    }

    for( size_t index = 0; index < vertices->size() / 3; index++ )
        points->push_back( new WKdPointND( vertices->at( index*3 + 0 ),
                                           vertices->at( index*3 + 1 ),
                                           vertices->at( index*3 + 2 ) ) );

    m_searchTree->add( points );
    m_pointSearcher.setExaminedKdTree( m_searchTree );
    m_pointSearcher.setMaxResultPointCountInfinite();
    m_pointSearcher.setMaxSearchDistance( subtractionRadius );
}

bool WPointSubtactionHelper::pointsExistNearCoordinate( const vector<double>& coordinate )
{
    if( m_searchTree == 0 )
        return false;
    m_pointSearcher.setSearchedPoint( coordinate );
    return m_pointSearcher.getNearestNeighborCount() > 0;
}
