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

#include "WSurfaceDetectorLari.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WParameterDomainKdNode.h"
#include "structure/WSpatialDomainKdNode.h"
#include "../common/datastructures/kdtree/WPointSearcher.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include "core/common/WRealtimeTimer.h"
#include "../common/math/leastSquares/WLeastSquares.h"
#include "../tempLeastSquaresTest/WMTempLeastSquaresTest.h"


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
     * Creates the instance that is used to segment surfaces using the approach of 
     * Lari/Habib.
     * \param surfaceDetector Surface detection instance that uses the approach of 
     *                        Lari/Habib.
     */
    explicit WLariOutliner( WSurfaceDetectorLari* surfaceDetector );
    /**
     * Destroys the result outlining instance.
     */
    virtual ~WLariOutliner();

    /**
     * Puts out points of the spatial domain.
     * \return Points of the spatiial domain. Red ones are planar and blue points belong 
     *         to the linear/cylindrical features.
     */
    boost::shared_ptr< WDataSetPoints > outlineParameterDomain();
    /**
     * Returns points of the parameter domain (Plane describing features of each 
     * corresponding spatial point).
     * \return Points of the parameter domain that describe each spatial point's fitted 
     *         plane.
     */
    boost::shared_ptr< WDataSetPoints > outlineSpatialDomain();
    /**
     * Outlines each point's best fitted plane.
     * \param squaresWidth Square width of the outlined best fitted planes of input 
     *                     points (in relation to its neighbors).
     * \return output triangle mesh that depicts best fitted planes of each input point.
     */
    boost::shared_ptr< WTriangleMesh > outlineLeastSquaresPlanes( double squaresWidth );

private:
    /**
     * Assigned Surface detection instance that uses the approach of Lari/Habib. It's 
     * used to retirieve a corresponding spatial or parameter domain.
     */
    WSurfaceDetectorLari* m_surfaceDetector;
};

#endif  // WLARIOUTLINER_H
