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

#ifndef WCUTOUTLIERSDEAMON_H
#define WCUTOUTLIERSDEAMON_H

#include <vector>
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "../common/datastructures/octree/WOctNode.h"

/**
 * This is an outliers cut algorithm it simply groups all the points in cube groups. 
 * After process all points of the largest voxel group are returned.
 */
class WCutOutliersDeamon
{
public:
    /**
     * The cut outliers constructor.
     */
    WCutOutliersDeamon();
    /**
     * The cut outliers destructor.
     */
    virtual ~WCutOutliersDeamon();
    /**
     * Cuts Outliers of a point data. At first all point cubes are grouped. After that only 
     * points of the group with the highest volume is returned
     * \param points Input WDataSetPoints to cut outliers off.
     * \return WDataSetPoints after cutting off outliers.
     */
    boost::shared_ptr< WDataSetPoints > cutOutliers(
            boost::shared_ptr< WDataSetPoints > points );
    /**
     * Sets the cube radius to determine cube neighborships. Not connected nodes are cut off.
     * \param detailDepth Cube radius in meters. Use only numbers that are in 2^n 
     * including negative n.
     */
    void setDetailDepth( double detailDepth );

private:
    /**
     * Counts voxels of each group. The method traverses recursively.
     * \param node Octree node to count voxels of each physically connected node group.
     */
    void countGroups( WOctNode* node );
    /**
     * Resizes the voxel group list representing corresponding voxel counts.
     * \param newSize Target list length to apply.
     */
    void resizeItemCountGroupList( size_t newSize );
    /**
     * Cube width bin points into.
     */
    double m_detailDepth;
    /**
     * Temporary voxel count array of each group.
     */
    std::vector<size_t> m_pointCounts;
};

#endif  // WCUTOUTLIERSDEAMON_H
