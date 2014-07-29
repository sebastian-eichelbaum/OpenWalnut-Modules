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

#ifndef WSURFACEDETECTORLARI_H
#define WSURFACEDETECTORLARI_H

#include <iostream>
#include <vector>

#include "core/dataHandler/WDataSetPoints.h"
#include "structure/WParameterDomainKdNode.h"
#include "structure/WSpatialDomainKdNode.h"
#include "../common/datastructures/kdtree/WPointSearcher.h"
#include "core/common/math/principalComponentAnalysis/WPrincipalComponentAnalysis.h"
#include "core/common/WRealtimeTimer.h"


using std::cout;
using std::endl;
using std::vector;

/**
 * Class that dues the surface detection using the approaches of Lari/abib (2014). This 
 * class is in a very early stage and no purpose at the moment.
 */
class WSurfaceDetectorLari
{
public:
    explicit WSurfaceDetectorLari();

    virtual ~WSurfaceDetectorLari();
    /**
     * Detects the surfaces using the approach of Lari/Habib (2014). At the moment it 
     * only colors points whether they have either planar or linear/cylindrical features.
     * \param inputPoints nput points to process.
     * \return The points that are added a color. Red points have either planar and blue 
     *         ones have linear/cylindrical features. Magenta ones meet both and grey 
     *         none of both criterias.
     */
    void analyzeData( vector<vector<double> >* inputPoints );
    /**
     * Returns the parameter domain points. Each parameter point depicts a best fitted 
     * plane formula of each corresponding input point of the spatial domain.
     * \return The whole set of points of the parameter domain.
     */
    WParameterDomainKdNode* getParameterDomain();
    /**
     * Returns the input points that belong to the spatial domain.
     * \return The whole point set of the spatial domain.
     */
    WSpatialDomainKdNode* getSpatialDomain();

    /**
     * Calculates whether a point's eigen values in relation to its neighbors have 
     * planar features.
     * \param eigenValues Eigen values to examine.
     * \return The point has planar featues or not.
     */
    bool calculateIsPlanarPoint( vector<double> eigenValues );
    /**
     * Calculates whether a point's eigen values in relation to its neighbors have 
     * linear/cylindrical features.
     * \param eigenValues Eigen values to examine.
     * \return The point has linear/cylindrical featues or not.
     */
    bool calculateIsCylindricalPoint( vector<double> eigenValues );
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
     * Returns the numbers sum of an array.
     * \param allNumbers The numbers to sum.
     * \return Result = number 1 + number 2 + number 3 + . . . ).
     */
    static double getVectorSum( vector<double> allNumbers );
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

private:
    /**
     * Links the corresponding points of the spatial domain to the parameter domain. 
     * Afterwards each point is assigned its best fitted plane's properties. And each 
     * parameter domain point will know which points of the spatial domain belong to it.
     */
    void linkSpatialAndParameterDomain();

    /**
     * The maximal count of analyzed neighbors of an examined input point.
     */
    size_t m_numberPointsK;
    /**
     * Maximal radius within which the nearest neighbors are examined.
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
    WSpatialDomainKdNode* m_spadialDomain;
    /**
     * The input points that belong to the spatial domain.
     */
    WParameterDomainKdNode* m_parameterDomain;
};

#endif  // WSURFACEDETECTORLARI_H
