//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2013 OpenWalnut Community, BSV-Leipzig and CNCF-CBS
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
     * \param centerX Xcoordinate of the octree node center.
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
     * \param drawer Corresponding index of vX/vY/vZ which depict which octree node
     *               to return regarding the X/Y/Z coordinates.
     * \return Octree child node of that case.
     */
    WOctNode* getChild( size_t drawer );
    /**
     * Checks whether a coordinate fits into the existing root octree node. A false value
     * indicates that the node should be expanded. This method should be applied only
     * using the root noce.
     * \param x X coordinate to determine whether the node covers it.
     * \param y Y coordinate to determine whether the node covers it.
     * \param z Z coordinate to determine whether the node covers it.
     * \return Shows whether the octree node covers the X/Y/Z coordinate.
     */
    bool fitsIn( double x, double y, double z );
    /**
     * Returns which child case index is covered by particular coordinates. An invalid
     * index is returned if ther's no possible child for that coordinate.
     * \param x X coordinate to analyze.
     * \param y Y coordinate to analyze.
     * \param z Z coordinate to analyze.
     * \return Case index regarding the x/Y/Z coordinates described by the vX/vY/vZ 
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
     * Range that the octree node covers in each dimension. Smallest possible dimension
     * value is included but the largest should be excluded in order to put in the octree
     * node standing next to the current.
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

private:
    /**
     * Sets an octree node as a child of this node.
     * \param child Octree node to nest into
     * \param drawer Determines where the node is nested into. The cases are determined by
     *               the constants vX, vY and vZ of the dwawer index lying between 0 and 7.
     */
    void setChild( WOctNode* child, size_t drawer );

    /**
     * Children of the current octree node
     */
    WOctNode* m_child[8];
    /**
     * Center of the current octree node
     */
    double m_center[3];
    /**
     * Range that the octree node covers in each dimension. Smallest possible dimension
     * value is included but the largest should be excluded in order to put in the octree
     * node standing next to the current.
     */
    double m_radius;
};
#endif  // WOCTNODE_H
