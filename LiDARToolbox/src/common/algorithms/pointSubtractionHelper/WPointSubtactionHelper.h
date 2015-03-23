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

#ifndef WPOINTSUBTACTIONHELPER_H
#define WPOINTSUBTACTIONHELPER_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "core/dataHandler/WDataSetPoints.h"
#include "../../datastructures/kdtree/WKdTreeND.h"
#include "../../datastructures/kdtree/WPointSearcher.h"
#include "../../math/vectors/WVectorMaths.h"


/**
 * The class tells wheter points exist around a radius by means of a D coordinate.
 * It is mainly used to subtract point coordinates from point datasets.
 */
class WPointSubtactionHelper
{
public:
    /**
     * Creates the subtractor instance
     */
    WPointSubtactionHelper();

    /**
     * Destroys the subtractor instance
     */
    virtual ~WPointSubtactionHelper();

    /**
     * Initializes the instance.
     * \param pointsToSubtract Point set to be tested whether its points are near a 
     *                         coordinate to be tested.
     * \param subtractionRadius Search radius for point existance.
     */
    void initSubtraction( boost::shared_ptr< WDataSetPoints > pointsToSubtract, double subtractionRadius );

    /**
     * Returns whether points exist near a coordinate by means of a radius.
     * \param coordinate Coordinate to be tested.
     * \return Points exist near a coordinate by means of a radius or not.
     */
    bool pointsExistNearCoordinate( const vector<double>& coordinate );

private:
    /**
     * Point search tree.
     */
    WKdTreeND* m_searchTree;

    /**
     * Point search instance.
     */
    WPointSearcher m_pointSearcher;
};

#endif  // WPOINTSUBTACTIONHELPER_H
