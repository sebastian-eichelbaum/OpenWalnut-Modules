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

#ifndef WLARIOUTLINER_H
#define WLARIOUTLINER_H

#include <iostream>
#include <vector>

#include "WLariPointClassifier.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WParameterDomainKdPoint.h"
#include "structure/WSpatialDomainKdPoint.h"
#include "../common/datastructures/kdtree/WPointSearcher.h"
#include "../common/datastructures/kdtree/WKdTreeND.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include "core/common/WRealtimeTimer.h"
#include "../common/math/leastSquares/WLeastSquares.h"
#include "../tempLeastSquaresTest/WMTempLeastSquaresTest.h"
#include "../common/datastructures/WDataSetPointsGrouped.h"


using std::cout;
using std::endl;
using std::vector;

/**
 * Class that is used to outline the segmentation results of the approach of Lari/Habib.
 */
class WLariOutliner
{
public:
    /**
     * Instantiates the Lari/Habib segmentation result outliner.
     * \param pointClassifier Lari/Habib segmentation insgance to access the spatial and 
     *                        parameter domain.
     */
    explicit WLariOutliner( WLariPointClassifier* pointClassifier );

    /**
     * Destroys the result outlining instance.
     */
    virtual ~WLariOutliner();

    /**
     * Outlines the point plane segmentation result using the context of the parameter 
     * domain.
     * \return Point plane segmentation result using the context of the parameter domain.
     */
    boost::shared_ptr< WDataSetPointsGrouped > outlineParameterDomain();

    /**
     * Outlines the point plane segmentation result using the context of the spatial 
     * domain.
     * \return Point plane segmentation result that depicts segmented plane groups.
     */
    boost::shared_ptr< WDataSetPointsGrouped > outlineSpatialDomainGroups();

    /**
     * Gives the classification result of each spatial point. Red points are detected as 
     * planar, blue as linear/cylindrical, magenta points belong to both and grey to 
     * none of them.
     * \return Outline of the parameter domain points.
     */
    boost::shared_ptr< WDataSetPoints > outlineSpatialDomainCategories();

    /**
     * Outlines each point's best fitted plane.
     * \param squaresWidth Square width of the outlined best fitted planes of input 
     *                     points (in relation to its neighbors).
     * \return output triangle mesh that depicts best fitted planes of each input point.
     */
    boost::shared_ptr< WTriangleMesh > outlineLeastSquaresPlanes( double squaresWidth );

private:
    /**
     * Point classification instance. By this way spatial and parameter domain points 
     * are fetched. But also points can be classified whether they areplanar, 
     * cylindrical or rough surface.
     */
    WLariPointClassifier* m_pointClassifier;
};

#endif  // WLARIOUTLINER_H
