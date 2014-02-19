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


#ifndef WQUADNODE_H
#define WQUADNODE_H

/**
 * Quadtree node class designed for points data. Each node represents an area where at least
 * one data set point exists.
 */
class WQuadNode
{
public:
    /**
     * Quadtree node constructor.
     * \param centerX X coordinate of the quadtree node center.
     * \param centerY Y coordinate of the quadtree node center.
     * \param radius Range from the center point that the node covers in each X/Y direction.
     */
    WQuadNode( double centerX, double centerY, double radius );
    /**
     * Destructor of the quadtree node
     */
    virtual ~WQuadNode();
    /**
     * Returns a quadtree child object of a particular case.
     * \param drawer Corresponding index of vX/vY which depicts which quadtree node
     *               to return regarding the X/Y coordinates.
     * \return Quadtree child node of that case.
     */
    WQuadNode* getChild( size_t drawer );
    /**
     * Checks whether a coordinate fits into the quadtree node. If it's a rot node then 
     * it indicates whether it should be expanded in order to access the coordinate.
     * \param x X coordinate to determine whether the node covers it.
     * \param y Y coordinate to determine whether the node covers it.
     * \return Shows whether the quadtree node covers the X/Y coordinate.
     */
    bool fitsIn( double x, double y );
    /**
     * Returns which child case index is covered by particular coordinates.
     * \param x X coordinate to analyze.
     * \param y Y coordinate to analyze.
     * \return Case index regarding the x/Y/Z coordinates described by the vX/vY/vZ 
     *         constants.
     */
    size_t getFittingCase( double x, double y );
    /**
     * Expands the quadtree by the double in each dimension. It should be applied only on
     * the root node: It's expanding the root cube in all 6 directions keeping its
     * center coordinate.
     */
    void expand();
    /**
     * Creates a new quadtree child node of the particular case if doesn't exist.
     * \param drawer The case of the child which X/Y/Z area correspond to the vX/vY/vZ
     *               constants
     */
    void touchNode( size_t drawer );
    /**
     * Returns the quadnode's radiius from the center.
     * \return Node range from its center.
     */
    double getRadius();
    /**
     * Returns center coordinates of an quadtree node.
     * \param dimension Center dimension to return (0/1 = X/Y)
     * \return The center coordinate to the corresponding dimension parameter.
     */
    double getCenter( size_t dimension );

    /**
     * Updates minimal and maximal X/Y/elevation value parameters. The point count is incremented by 1.
     * It doesn't alter or add any node.
     * \param x X coordinate to update.
     * \param y Y coordinate to update.
     * \param elevation Elevation to update.
     */
    void updateMinMax( double x, double y, double elevation );

    /**
     * Returns the count of registered points using updateMinMax().
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
     * Returns the minimal elevation.
     * \return The minimal elevation.
     */
    double getElevationMin();
    /**
     * Returns the maximal elevation.
     * \return The maximal elevation.
     */
    double getElevationMax();

    /**
     * Determines which X coordinate axis case an m_child has.
     */
    static const size_t vX[];
    /**
     * Determines which Y coordinate axis case an m_child has.
     */
    static const size_t vY[];

    /**
     * Sets an assignable ID.
     * \param id An assignable ID.
     */
    void setID( size_t id );
    /**
     * Returns an assignable ID.
     * \return An assignable ID.
     */
    size_t getID();

private:
    /**
     * Sets an quadtree node as a child of this node.
     * \param child Quadtree node to nest into.
     * \param drawer Determines where the node is nested into. The cases are determined by
     *               the constants vX and vY of the dwawer index lying between 0 and 3.
     */
    void setChild( WQuadNode* child, size_t drawer );

    /**
     * Children of the current quadtree node
     */
    WQuadNode* m_child[4];
    /**
     * Center coordinate of the current quadtree node
     */
    double m_center[2];
    /**
     * The radius of the quadnode which is covered of its area.
     */
    double m_radius;

    /**
     * Point count of the node registered by updateMinMax().
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
     * Minimal elevation.
     */
    double m_zMin;
    /**
     * Maximal elevation.
     */
    double m_zMax;

    //TODO(schwarzkopf): Implement the following parameter another way somewhere else.
    /**
     * An assignable ID parameter. It doesn't affect the quadtree function at all.
     */
    size_t m_id; //TODO(schwarzkopf): Function parameters that don't belong to the main function don't belong to it.
};
#endif  // WQUADNODE_H
