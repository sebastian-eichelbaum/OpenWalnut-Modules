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


#ifndef WPCADETECTOCTNODE_H
#define WPCADETECTOCTNODE_H
#include <vector>
#include "../../common/datastructures/octree/WOctNode.h"
#include "core/common/math/linearAlgebra/WPosition.h"
#include "core/dataHandler/WDataSetPoints.h"

using std::vector;

/**
 * Voxel Node type that holds an additional color parameter and input data point set.
 */
class WPcaDetectOctNode : public WOctNode
{
public:
    /**
     * Creates a new node
     */
    WPcaDetectOctNode();
    /**
     * Octree node constructor.
     * \param centerX X coordinate of the octree node center.
     * \param centerY Y coordinate of the octree node center.
     * \param centerZ Z coordinate of the octree node center.
     * \param radius Range from the center point that the node covers in each X/Y/Z 
     * direction.
     */
    WPcaDetectOctNode( double centerX, double centerY, double centerZ, double radius );
    /**
     * Destructor of the octree node
     */
    virtual ~WPcaDetectOctNode();
    /**
     * The octree instance uses this method to instantiate a new node of the same type.
     * \param centerX X coordinate of the octree node center.
     * \param centerY Y coordinate of the octree node center.
     * \param centerZ Z coordinate of the octree node center.
     * \param radius Range from the center point that the node covers in each X/Y/Z 
     *               direction.
     * \return New node instance of the same type.
     */
    virtual WOctNode* newInstance( double centerX, double centerY, double centerZ, double radius );
    /**
     * Adds a point to the inpuz data set.
     * This method is executed every time when this node is touched.
     * \param x X coordinate of the new point.
     * \param y Y coordinate of the new point.
     * \param z Z coordinate of the new point.
     */
    virtual void onTouchPosition( double x, double y, double z );
    /**
     * Returns input data points. covered by the node's area.
     * \return Input point data set.
     */
    vector<WPosition>* getInputPoints();
    /**
     * Sets the node color.
     * \param color Node color to assign.
     */
    void setColor( osg::Vec4 color );
    /**
     * returns the node color.
     * \return the color of the node.
     */
    osg::Vec4 getColor();
    /**
     * Sets the isotropic level of the node. More exactly said: Smallest point 
     * distribution direction strength divided by the biggest one (Smalles igen Value 
     * divided by the biggest).
     * \param eigenValueQuotient The value that describes the isotropic level.
     */
    void setEigenValueQuotient( double eigenValueQuotient );
    /**
     * Returns the isotropic level of the node. More exactly said: Smallest point 
     * distribution direction strength divided by the biggest one (Smalles igen Value 
     * divided by the biggest).
     * \return The value that describes the isotropic level.
     */
    double getEigenValueQuotient();
    /**
     * Says whether the node has got the information about the point's isotropic
     * level.
     * \return Point has got information about the istropic level or not.
     */
    bool hasEigenValueQuotient();
    /**
     * Clears the input point data. It mainly frees space after the Principal
     * Component Analysis.
     */
    void clearInputData();

private:
    /**
     * Input data set points covered by that node.
     */
    vector<WPosition>* m_inputPoints;
    /**
     * Quotient of the smallest Eigen Value over the biggest.
     */
    double m_eigenValueQuotient;
    /**
     * Marker that says whether the node has the information about its point's
     * isotropic level.
     */
    bool m_hasEigenValueQuotient;
};
#endif  // WPCADETECTOCTNODE_H
