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


#ifndef WOCTNODE_H
#define WOCTNODE_H

/**
 * Octree node class designed for points data. Each node represents an area where at least
 * one data set point exists.
 */
class WOctNode
{
public:
    /**
     * Octree node constructor.
     * \param centerX X coordinate of the octree node center.
     * \param centerY Y coordinate of the octree node center.
     * \param centerZ Z coordinate of the octree node center.
     * \param radius Range from the center point that the node covers in each X/Y/Z direction.
     */
    WOctNode( double centerX, double centerY, double centerZ, double radius );
    /**
     * Destructor of the octree node
     */
    virtual ~WOctNode();
    /**
     * Returns an octree child object of a particular case.
     * \param drawer Corresponding index (0-7) of vX/vY/vZ which depict which octree node
     *               to return regarding the X/Y/Z coordinates.
     * \return Octree child node of that case.
     */
    WOctNode* getChild( size_t drawer );
    /**
     * Checks whether a coordinate fits into the octree node. If it's a root noe then 
     * it will indicate whether it should be expanded to fit in a point..
     * \param x X coordinate to determine whether the node covers it.
     * \param y Y coordinate to determine whether the node covers it.
     * \param z Z coordinate to determine whether the node covers it.
     * \return Shows whether the octree node covers the X/Y/Z coordinate.
     */
    bool fitsIn( double x, double y, double z );
    /**
     * Returns which child case index (0-7) is covered by a particular coordinate.
     * \param x X coordinate to analyze.
     * \param y Y coordinate to analyze.
     * \param z Z coordinate to analyze.
     * \return Case index regarding the X/Y/Z coordinate described by the vX/vY/vZ 
     *         constants.
     */
    size_t getFittingCase( double x, double y, double z );
    /**
     * Expands the octree by the double in each dimension. It should be applied only on
     * the root node: It's expanding the root cube in all 6 directions keeping its
     * center coordinate.
     */
    void expand();
    /**
     * Creates a new octree child node of the particular case if doesn't exist.
     * \param drawer The case of the child which X/Y/Z area correspond to the vX/vY/vZ
     *               constants
     */
    void touchNode( size_t drawer );
    /**
     * Returns the radius of the octree node.
     * \return Node range from its center.
     */
    double getRadius();
    /**
     * Returns center coordinates of an octree node.
     * \param dimension center dimension to return (0/1/2 = X/Y/Z)
     * \return the center coordinate to the corresponding dimension parameter
     */
    double getCenter( size_t dimension );
    /**
     * Returns the Node group ID that is calculated regarding the lear node neighbors.
     * \return The voxel group ID. Applyable only on leaf nodes.
     */
    size_t getGroupNr();
    /**
     * Sets the octree node group ID. It's usually calculated regarding the leaf voxel neighbors.
     * \param groupNr The voxel group ID.
     */
    void setGroupNr( size_t groupNr );

    /**
     * Determines which X coordinate axis case a m_child has.
     */
    static const size_t vX[];
    /**
     * Determines which Y coordinate axis case a m_child has.
     */
    static const size_t vY[];
    /**
     * Determines which Z coordinate axis case a m_child has.
     */
    static const size_t vZ[];

    /**
     * Updates minimal and maximal X/Y/Z value parameters. The point count is incremented by 1.
     * \param x X coordinate to update.
     * \param y Y coordinate to update.
     * \param z Z coordinate to update.
     */
    void updateMinMax( double x, double y, double z );
    /**
     * Returns the count of registered points.
     * \return Registered points count.
     */
    size_t getPointCount();
    /**
     * Returns the minimal X value.
     * \return The minimal X value.
     */
    double getXMin();
    /**
     * Returns the maximal X value.
     * \return The maximal X value.
     */
    double getXMax();
    /**
     * Returns the minimal Y value.
     * \return The minimal Y value.
     */
    double getYMin();
    /**
     * Returns the maximal Y value.
     * \return The maximal Y value.
     */
    double getYMax();
    /**
     * Returns the minimal Z value.
     * \return The minimal Z value.
     */
    double getZMin();
    /**
     * Returns the maximal Z value.
     * \return The maximal Z value.
     */
    double getZMax();

private:
    /**
     * Sets an octree node as a child of this node.
     * \param child Octree node to nest into
     * \param drawer Determines where the node is nested into. The cases are determined by
     *               the constants vX, vY and vZ of the dwawer index lying between 0 and 7.
     */
    void setChild( WOctNode* child, size_t drawer );

    /**
     * Children of the current octree node.
     */
    WOctNode* m_child[8];
    /**
     * Absolute center coordinate of the current octree node.
     */
    double m_center[3];
    /**
     * Radius from the center that is covered by the octree node area.
     */
    double m_radius;

    //TODO(schwarzkopf): Function parameters that don't belong to the main function don't belong to it.
    /**
     * The node group ID. This number usually corresponds to its voxel neighborship.
     */
    size_t m_groupNr; //TODO(schwarzkopf): Implement the following parameter another way somewhere else.
    /**
     * Point count of the node.
     */
    size_t m_pointCount;
    /**
     * Minimal X coordinate.
     */
    double m_xMin;
    /**
     * Maximal X coordinate.
     */
    double m_xMax;
    /**
     * Minimal Y coordinate.
     */
    double m_yMin;
    /**
     * Maximal Y coordinate.
     */
    double m_yMax;
    /**
     * Minimal Z coordinate.
     */
    double m_zMin;
    /**
     * Maximal Z coordinate.
     */
    double m_zMax;
};
#endif  // WOCTNODE_H
