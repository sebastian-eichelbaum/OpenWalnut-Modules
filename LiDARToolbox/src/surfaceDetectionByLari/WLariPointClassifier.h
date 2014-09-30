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

#ifndef WLARIPOINTCLASSIFIER_H
#define WLARIPOINTCLASSIFIER_H

#include <iostream>
#include <vector>
#include <string>
#include <boost/thread.hpp>

#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WParameterDomainKdPoint.h"
#include "structure/WSpatialDomainKdPoint.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include "core/common/WRealtimeTimer.h"
#include "../common/datastructures/kdtree/WKdTreeND.h"
#include "../common/datastructures/kdtree/WKdPointND.h"
#include "../common/datastructures/kdtree/WPointSearcher.h"
#include "../common/math/vectors/WVectorMaths.h"
#include "../tempLeastSquaresTest/WMTempLeastSquaresTest.h"


using std::cout;
using std::endl;
using std::vector;


/**
 * Class that adds classification meta to points in relation to its neighbor points. The 
 * meta contains such things as eigenvalues, eigenvectors (Both calculated by Principal 
 * Component Analysis) and the Hessian normal form (calculated using least squares 
 * adjustment).
 */
class WLariPointClassifier
{
public:
    explicit WLariPointClassifier();

    /**
     * Destroys the surface detection instance
     */
    virtual ~WLariPointClassifier();

    /**
     * Analyzes input point data.
     * \param inputPoints Input point data to analyze.
     */
    void analyzeData( vector<WSpatialDomainKdPoint*>* inputPoints );

    /**
     * Returns the parameter domain points. Each parameter point depicts a best fitted 
     * plane formula of each corresponding input point of the spatial domain.
     * \return The whole set of points of the parameter domain.
     */
    WKdTreeND* getParameterDomain();

    /**
     * Returns the input points that belong to the spatial domain.
     * \return The whole point set of the spatial domain.
     */
    WKdTreeND* getSpatialDomain();

    /**
     * Calculates whether a point's eigen values in relation to its neighbors have 
     * planar features.
     * \param eigenValues Eigen values to examine.
     * \return The point has planar featues or not.
     */
    bool calculateIsPlanarPoint( const vector<double>& eigenValues );

    /**
     * Calculates whether a point's eigen values in relation to its neighbors have 
     * linear/cylindrical features.
     * \param eigenValues Eigen values to examine.
     * \return The point has linear/cylindrical featues or not.
     */
    bool calculateIsCylindricalPoint( const vector<double>& eigenValues );

    /**
     * Setup method for the process - The maximal count of analyzed neighbors of an 
     * examined input point.
     * \param pointsCount The maximal count of the analyzed points (the exmined point + 
     *                    its nearest neighbors).
     */
    void setNumberPointsK( size_t pointsCount );

    /**
     * Sets the maximal radius within which the nearest neighbors are examined.
     * \param maxPointDistance The maximal distance to examined neighbors.
     */
    void setMaxPointDistanceR( double maxPointDistance );

    /**
     * Sets the applied CPU thread count.
     * \param cpuThreadCount Applied CPU thread count.
     */
    void setCpuThreadCount( size_t cpuThreadCount );

    /**
     * Returns the numbers sum of an array.
     * \param allNumbers The numbers to sum.
     * \return Result = number 1 + number 2 + number 3 + . . . ).
     */
    static double getVectorSum( const vector<double>& allNumbers );

    /**
     * Sets the normalized lambda range condition. All of them must be met in order to 
     * detect a point as planar.
     * \param lambdaIndex The lambda index to set. The first index represents the 
     *                    biggest eigen value.
     * \param min The lower limit of the lamba of that index.
     * \param max The higher limit of the lamba of that index.
     */
    void setPlanarNLambdaRange( size_t lambdaIndex, double min, double max );

    /**
     * Sets the normalized lambda range condition. All of them must be met in order to 
     * detect a point as linear/cylindrical.
     * \param lambdaIndex The lambda index to set. The first index represents the 
     *                    biggest eigen value.
     * \param min The lower limit of the lamba of that index.
     * \param max The higher limit of the lamba of that index.
     */
    void setCylindricalNLambdaRange( size_t lambdaIndex, double min, double max );


    /**
     * Assigns the progress combiner to depict the status during classifying.
     * \param progress Progress combiner to assign.
     */
    void assignProgressCombiner( boost::shared_ptr< WProgressCombiner > progress );

    /**
     * Initializes the current progress status.
     * \param iteration Current iteration number. Mostly cluster ID.
     * \param steps Step count of the process. Mostly it is the point count within a cluster.
     * \param headerText Gives information about the task
     */
    void setProgressSettings( size_t iteration, size_t steps, std::string headerText );

    /**
     * Increments the progress status by one unit.
     */
    void incrementProgress();

    /**
     * Finishes the progress.
     */
    void finishProgress();

private:
    /**
     * Classifies points using Eigen Value analyses (Eigen Values and Eigen Vectors) and 
     * least squares adjustment. The method uses multithreading.
     * \param spatialPoints Spatial domain points to analyze.
     * \param parameterPoints List of assigned parameter domain points that are 
     *                        initialized in this method.
     */
    void classifyPoints( vector<WSpatialDomainKdPoint*>* spatialPoints, vector<WParameterDomainKdPoint*>* parameterPoints );

    /**
     * Classifies points using Eigen Value analyses (Eigen Values and Eigen Vectors) and 
     * least squares adjustment. The method Apply this method for every thread index.
     * \param spatialPoints Spatial domain points to analyze.
     * \param threadIndex CPU thread index.
     */
    void classifyPointsAtThread( vector<WSpatialDomainKdPoint*>* spatialPoints, size_t threadIndex );



    /**
     * Sets the progress status.
     */

    /**
     * The maximal count of analyzed neighbors of an examined input point. It is the 
     * number of points that is required for a plane definition.
     */
    size_t m_numberPointsK;

    /**
     * Maximal radius, within which the nearest neighbors are examined.
     */
    double m_maxPointDistanceR;

    /**
     * The lower normalized lambda (eigen value) limit to detect point's feature as 
     * planar. The eigen values are sorted descending.
     */
    vector<double> m_planarNLambdaMin;

    /**
     * The higher normalized lambda (eigen value) limit to detect point's feature as 
     * planar. The eigen values are sorted descending.
     */
    vector<double> m_planarNLambdaMax;

    /**
     * The lower normalized lambda (eigen value) limit to detect point's feature as 
     * linear/cylindrical. The eigen values are sorted descending.
     */
    vector<double> m_cylindricalNLambdaMin;

    /**
     * The higher normalized lambda (eigen value) limit to detect point's feature as 
     * linear/cylindrical. The eigen values are sorted descending.
     */
    vector<double> m_cylindricalNLambdaMax;

    /**
     * the parameter domain points. Each parameter point depicts a best fitted plane 
     * formula of each corresponding input point of the spatial domain.
     */
    WKdTreeND* m_spatialDomain;

    /**
     * The input points that belong to the spatial domain.
     */
    WKdTreeND* m_parameterDomain;

    /**
     * CPU threads count for multithreading support.
     */
    size_t m_cpuThreadCount;

    /**
     * CPU threads object for multithreading support.
     */
    vector<boost::thread*> m_cpuThreads;


    /**
    * Progress combiner for changing the plugin status in the modules overview.
    */
    boost::shared_ptr< WProgressCombiner > m_associatedProgressCombiner;

    /**
    * Current progress status.
    */
    boost::shared_ptr< WProgress > m_progressStatus;
};

#endif  // WLARIPOINTCLASSIFIER_H
