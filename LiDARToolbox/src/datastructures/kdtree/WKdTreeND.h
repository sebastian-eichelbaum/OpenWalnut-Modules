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

#ifndef WKDTREEND_H
#define WKDTREEND_H

#include <vector>
#include <algorithm>

using std::vector;
using std::size_t;
using std::cout;
using std::endl;

/**
 * This is a unidimensional kd tree compound. It is proposed not only to work with two 
 * or three dimensional kd trees but also with a single or more than three dimensions.
 */
class WKdTreeND
{
public:
    WKdTreeND();
    /**
     * Instantiates a kd tree of a arbitrary dimension count.
     * \param dimensions The dimension count of the kd tree.
     */
    explicit WKdTreeND( size_t dimensions );
    virtual ~WKdTreeND();
    /**
     * Adds points to the kd tree. It's better to add all the points into an empty tree.
     * The tree is compatible to add one by one point but it will result a heavily 
     * unbalanced binary tree inside adding a larger amount of points.
     * \param addables Points to add.
     */
    void add( vector<vector<double> >* addables );
    /**
     * Says whether a kd tree node can be split or not. Firstly the method to determine 
     * the splitting dimension should be executed. A true value is returned if the 
     * calculated splitting dimension is within the total dimensions count.
     * \return The kd tree node can be split or not.
     */
    bool canSplit();
    /**
     * Fetches all the points from a kd tree node into a list.
     * \param targetPointSet Target point list to put points to.
     */
    void fetchPoints( vector<vector<double> >* targetPointSet );
//    double getAreaMin(size_t dimension); //TODO(aschwarzkopf): Consider to purge in future
//    double getAreaMax(size_t dimension); //TODO(aschwarzkopf): Consider to purge in future
    /**
     * Returns all the leaf nodes of the kd tree.
     * \return All the leaf nodes of the kd tree.
     */
    vector<WKdTreeND*>* getAllLeafNodes();
    /**
     * Returns the dimension count of the kd tree.
     * \return The dimension count of the kd tree.
     */
    size_t getDimensions();
//    bool getHasBoundMin(size_t dimension); //TODO(aschwarzkopf): Consider to purge in future
//    bool getHasBoundMax(size_t dimension); //TODO(aschwarzkopf): Consider to purge in future
    /**
     * Returns the child node that lies on the lower scale across the splitting 
     * dimension.
     * \return The child node that lies on the lower scale across the splitting 
     * dimension.
     */
    WKdTreeND* getLowerChild();
    /**
     * Returns the child node that lies on the higher scale across the splitting 
     * dimension.
     * \return The child node that lies on the higher scale across the splitting 
     * dimension.
     */
    WKdTreeND* getHigherChild();
    /**
     * Returns the points that are registered to the kd tree node. Usually all the 
     * poinnts have the same coordinate if nodes are subdivided to the lowest point 
     * count and no points of the same coordinates exist. Only leaf nodes can have and 
     * return points.
     * \return The points of a kd tree node.
     */
    vector<vector<double> >* getNodePoints();
    /**
     * Returns the dimension across which the current node is split.
     * \return The dimension across which the current node is split.
     */
    size_t getSplittingDimension();
    /**
     * Returns the splitting position of the splitted dimension.
     * \return The splitting position of the splitted dimension.
     */
    double getSplittingPosition();

protected:
    /**
     * Returns a new instance of the kd tree. You should overwrite this method when you 
     * derive your own class from this so that the kd tree can instantiate new objects 
     * of your type.
     * \param dimensions The dimensions count of the new node.
     * \return A new kd tree node instance of your node type.
     */
    virtual WKdTreeND* getNewInstance( size_t dimensions );

private:
    /**
     * This method is used by the method to add new points. It puts points either to the 
     * node of the lower or higher position across the splitting dimension. The method is 
     * executed after the splitting dimension and position are calculated.
     * \param newPoints Points to append to child nodes.
     */
    void addPointsToChildren( vector<vector<double> >* newPoints );
    /**
     * Calculates the splitting position between the two child nodes. It calculates the 
     * median of all input points across the splitting dimension between two children. 
     * Always check out the splitting dimension before that.
     * \param points Input points to calculate the median across the splitted dimension.
     */
    void calculateSplittingPosition( vector<vector<double> >* points );
    /**
     * Determines the new splitting dimension between two child nodes. Afterwards the 
     * splitting position can be calculated across that dimension after executing that 
     * method.
     * \param inputPoints Input points to analyse the most optimal splitting dimension.
     * \return The kd tree node can be split or not.
     */
    bool determineNewSplittingDimension( vector<vector<double> >* inputPoints );
    /**
     * Fetches all kd tree leaf nodes into a node list.
     * \param targetNodeList The target list where leaf nodes are put.
     */
    void fetchAllLeafNodes( vector<WKdTreeND*>* targetNodeList );
    /**
     * Tells whether a kd tree node has a parent or not.
     * \return The node has a parent or not.
     */
    bool hasParent();
    /**
     * Initializes newly created child kd tree nodes.
     */
    void initSubNodes();

    //void clearPointVector(vector<vector<double> >* deletables); //TODO(aschwarzkopf): Consider to purge in future

    //TODO(aschwarzkopf): There are implementations with only 4 members!
    /**
     * The splitting dimension of the parent kd tree node. It is considered to determine 
     * the splitting dimension of the current kd tree node.
     */
    size_t m_parentSplittingDimension;    //ggf. wegschmeißen
    //TODO(aschwarzkopf): Ggf. Sinn: Nicht bis ins Letzte unterteilen, ggf. nur über einem Threshold
    /**
     * The kd tree node child which is on the lower position across the splitting 
     * dimension.
     */
    WKdTreeND* m_lowerChild;
    /**
     * The kd tree node child which is on the higher position across the splitting 
     * dimension.
     */
    WKdTreeND* m_higherChild;
    /**
     * The dimension count of the kd tree.
     */
    size_t m_dimensions; //TODO(aschwarzkopf): Consider whether you need it.
    /**
     * Children of a kd tree node are separated by a plane. The splitting dimension axis * is perpendicular to that.
     */
    size_t m_splittingDimension;
    /**
     * Children of a kd tree node are separated by a plane. The splitting dimension axis * is perpendicular to that. This position variable is the intersection between this 
     * plane and axis.
     */
    double m_splittingPosition;
    /**
     * Points of same coordinates can be added or not.
     */
    bool m_allowDoubles; //TODO(aschwarzkopf): Still neither used nor implemented
    /**
     * Points covered by a kd tree node.
     */
    vector<vector<double> >* m_points;


//    vector<double> areaMin;    //TODO(aschwarzkopf): Decide later whether to keep or not: It can be easily purged.
//    vector<double> areaMax;    //TODO(aschwarzkopf): Decide later whether to keep or not: It can be easily purged.
//    vector<double> hasBoundMin;    //TODO(aschwarzkopf): Decide later whether to keep or not: It can be easily purged.
//    vector<double> hasBoundMax;    //TODO(aschwarzkopf): Decide later whether to keep or not: It can be easily purged.
};

#endif  // WKDTREEND_H
