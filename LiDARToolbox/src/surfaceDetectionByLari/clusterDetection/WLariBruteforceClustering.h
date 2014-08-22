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

#ifndef WLARIBRUTEFORCECLUSTERING_H
#define WLARIBRUTEFORCECLUSTERING_H

#include <iostream>
#include <vector>
#include <boost/thread.hpp>

#include "core/dataHandler/WDataSetPoints.h"
#include "../structure/WParameterDomainKdPoint.h"
#include "../structure/WSpatialDomainKdPoint.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include "core/common/WRealtimeTimer.h"
#include "../../common/datastructures/kdtree/WKdTreeND.h"
#include "../../common/datastructures/kdtree/WKdPointND.h"
#include "../../common/datastructures/kdtree/WPointSearcher.h"
#include "../../common/math/vectors/WVectorMaths.h"
#include "../../tempLeastSquaresTest/WMTempLeastSquaresTest.h"
#include "../WLariPointClassifier.h"



using std::cout;
using std::endl;
using std::vector;

/**
 * Class that groups points that belong to the same planar formula. It uses the brute force peak detection approach.
 * 
 * It works as follows:
 *  A) While not all parameter domain points classified
 *      1) Calculate for every point how many parameters it would have in its extent if 
 *         the parameter domain point was the peak center
 *      2) Take the point with the highest parameter count as extent (peak center).
 *      3) Take the parameters and cluster it as one single planar patch
 *      4) Remove added points from waiting list
 * 
 * Note: Boundary detection has to be executed to separate spatially disconnected surfaces.
 */
class WLariBruteforceClustering
{
public:
    /**
     * Instantiates the brute force extent clustering instance.
     * \param classifier Point classification instance.
     */
    explicit WLariBruteforceClustering( WLariPointClassifier* classifier );
    /**
     * Destroys the surface detection instance
     */
    virtual ~WLariBruteforceClustering();

    /**
     * Detects surface clusters. It takes information of each parameter point 
     * (represents a plane formula) how many others it contains in the same planar 
     * extent group. At first biggest parameter extents are segmented to a plane until 
     * no parameter point remains
     */
    void detectClustersByBruteForce();
    /**
     * Sets the main segmentation settings of the segmentation algorithm. They regard 
     * the planar formula of each spatial point in relation to its 
     * neighborship.
     * \param maxAngleDegrees Maximal angular deviation of plane formulas between two 
     *                        points.
     * \param planeDistance Maximal difference between two plane perpendicular distances 
     *                      to the origin.
     */
    void setSegmentationSettings( double maxAngleDegrees, double planeDistance );
    /**
     * Multithreading setter.
     * \param cpuThreadCount CPU threads to apply.
     */
    void setCpuThreadCount( size_t cpuThreadCount );

private:
    /**
     * Returns the points within the parameter domain which belong to an extent of a 
     * plane.
     * \param parametersXYZ0 Peak center coordinate which depicts the region.
     * \return Parameter domain points that belong to the extent of a particular plane 
     *         formula.
     */
    vector<WParameterDomainKdPoint*>* getParametersOfExtent( const vector<double>& parametersXYZ0 );
    /**
     * Initializes the extent sizes of each parameter for those parameter domain points 
     * that are tagged to be refreshed. When points are clustered, neighbor parameter 
     * domain points neighbor counts change when points are removed from waiting list.
     * The method uses multithreading.
     * \param pointsToProcess Parameter domain points that should be processed.
     */
    void initExtentSizes( vector<WKdPointND*>* pointsToProcess );
    /**
     * Initializes the extent sizes of each parameter for those parameter domain points 
     * that are tagged to be refreshed. launches for one single thread. Execute it 
     * multiple times for all thread indices.
     * \param pointsToProcess Parameter domain points that should be processed.
     * \param threadIndex Thread index.
     */
    void initExtentSizesAtThread( vector<WKdPointND*>* pointsToProcess, size_t threadIndex );
    /**
     * Assignes parameter domain points to a single planar patch. These points are then 
     * removed from waiting list and its remaining neighbors are tagged to be refreshed 
     * by their extent point count. The method uses multithreading.
     * \param peakCenterPoint Peak center of the new planar patch that is clustered.
     * \param clusterID Current cluster ID that is assigned.
     */
    void addExtentCluster( WParameterDomainKdPoint* peakCenterPoint, size_t clusterID );
    /**
     * Assignes parameter domain points to a single planar patch. These points are then 
     * removed from waiting list and its remaining neighbors are tagged to be refreshed 
     * by their extent point count. The method has to be executed multiple times for 
     * every thread index.
     * \param extentPoints Points of the added extent (added planar patch).
     * \param clusterID Current assigned cluster ID.
     * \param threadIndex Thread index of multithreading.
     */
    void addExtentClusterAtThread( vector<WParameterDomainKdPoint*>* extentPoints, size_t clusterID, size_t threadIndex );
    /**
     * Returns the masimal euclidian distance within an extent from the peak center in 
     * the parameter domain kd tree.
     * \param parametersXYZ0 Parameter domain coordinate from which the maximal 
     *                       euclidian distance to the farest extent point is determined.
     * \return The maximal euclidian distance from the peak center to the farest 
     *         belonging parameter to the plane within the parameter domain.
     */
    double getMaxParameterDistance( const vector<double>& parametersXYZ0 );
    /**
     * Tells wehther two parameters can belong to the same extent or can belong to the 
     * same plane.
     * \param parameters1 First parameter to check.
     * \param parameters2 Second parameter to check.
     * \return parameters can belont to the same extent or not.
     */
    bool isParameterOfSameExtent( const vector<double>& parameters1, const vector<double>& parameters2 );
    /**
     * Parameter domain kd tree.
     */
    WKdTreeND* m_parameterDomain;
    /**
     * Setting that regards the planar formula of each spatial point in relation to its 
     * neighborship. This variable is the maximal angular deviation of plane formulas 
     * between two points.
     */
    double m_segmentationMaxAngleDegrees;
    /**
     * Setting that regards the planar formula of each spatial point in relation to its 
     * neighborship. This variable is the maximal difference between two plane normal 
     * distance to the origin.
     */
    double m_segmentationPlaneDistance;
    /**
     * CPU threads count for multithreading support.
     */
    size_t m_cpuThreadCount;
    /**
     * CPU threads object for multithreading support.
     */
    vector<boost::thread*> m_cpuThreads;
};

#endif  // WLARIBRUTEFORCECLUSTERING_H
