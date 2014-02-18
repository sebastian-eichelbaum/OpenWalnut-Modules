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

#ifndef WDATASETPOINTSGROUPED_H
#define WDATASETPOINTSGROUPED_H

#include <string>
#include <utility>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "core/common/WBoundingBox.h"
#include "core/dataHandler/WDataSet.h"

/**
 * Dataset to store a bunch of points without order or topology.
 */
class WDataSetPointsGrouped : public WDataSet // NOLINT
{ //TODO(schwarzkopf): Extend WDataSetPoints instead of writing lines again. Make Code cleaner that way.
public:
    // some type alias for the used arrays.
    /**
     * Pointer to dataset.
     */
    typedef boost::shared_ptr< WDataSetPointsGrouped > SPtr;

    /**
     * Pointer to const dataset.
     */
    typedef boost::shared_ptr< const WDataSetPointsGrouped > ConstSPtr;

    /**
     * List of vertex coordinates in term of components of vertices.
     */
    typedef boost::shared_ptr< std::vector< float > > VertexArray;

    /**
     * Colors for each vertex in VertexArray.
     */
    typedef boost::shared_ptr< std::vector< float > > ColorArray;

    /**
     * Group IDs for each vertex in VertexArray
     */
    typedef boost::shared_ptr< std::vector< size_t > > GroupArray;

    /**
     * Constructs a new set of points. If no color is specified, white is used for all points.
     *
     * \note the number of floats in vertices must be a multiple of 3
     * \note the number of floats in colors (if not NULL) must be vertices->size() / 3  times one of 1,3, or 4
     *
     * \param vertices the vertices of the points, stored in x1,y1,z1,x2,y2,z2, ..., xn,yn,zn scheme
     * \param colors The colors of each vertex. Can be NULL.. Stored as R1,G1,B1,A1, ... Rn,Gn,Bn,An
     * \param groups The group IDs of each vertex.
     * \param boundingBox The bounding box of the points (first minimum, second maximum).
     */
    WDataSetPointsGrouped( VertexArray vertices, ColorArray colors, GroupArray groups,
                    WBoundingBox boundingBox );

    /**
     * Constructs a new set of points. The bounding box is calculated during construction. If no color is specified, white is used for all
     * points.
     *
     * \note the number of floats in vertices must be a multiple of 3
     * \note the number of floats in colors (if not NULL) must be vertices->size() / 3  times one of 1,3, or 4
     *
     * \param vertices the vertices of the points, stored in x1,y1,z1,x2,y2,z2, ..., xn,yn,zn scheme
     * \param colors the colors of each vertex. Can be NULL.. Stored as R1,[G1,B1,[A1,]] ... Rn,[Gn,Bn,[An]]
     * \param groups The group IDs of each vertex.
     */
    WDataSetPointsGrouped( VertexArray vertices, ColorArray colors, GroupArray groups );

    /**
     * Constructs a new set of points. The constructed instance is empty..
     */
    WDataSetPointsGrouped();

    /**
     * Destructor.
     */
    virtual ~WDataSetPointsGrouped();

    /**
     * Get number of points in this data set.
     *
     * \return number of points
     */
    size_t size() const;

    /**
     * Determines whether this dataset can be used as a texture.
     *
     * \return true if usable as texture.
     */
    virtual bool isTexture() const;

    /**
     * Gets the name of this prototype.
     *
     * \return the name.
     */
    virtual const std::string getName() const;

    /**
     * Gets the description for this prototype.
     *
     * \return the description
     */
    virtual const std::string getDescription() const;

    /**
     * Returns a prototype instantiated with the true type of the deriving class.
     *
     * \return the prototype.
     */
    static boost::shared_ptr< WPrototyped > getPrototype();

    /**
     * Getter for the point vertices
     * \return The vertices
     */
    VertexArray getVertices() const;

    /**
     * Getter for the point colors
     * \return The colors
     */
    ColorArray getColors() const;

    /**
     * Returns the groups array of the vertex set.
     * \return The Group ID array where each item corresponds to a vertex.
     */
    GroupArray getGroups() const;

    /**
     * Get the bounding box.
     * \return The bounding box of all points.
     */
    WBoundingBox getBoundingBox() const;

    /**
     * Query coordinates of a given point.
     *
     * \throw WOutOfBounds if invalid index is used.
     * \param pointIdx the point index.
     *
     * \return the coordinates
     */
    WPosition operator[]( const size_t pointIdx ) const;

    /**
     * Query coordinates of a given point.
     *
     * \throw WOutOfBounds if invalid index is used.
     * \param pointIdx the point index.
     *
     * \return the coordinates
     */
    WPosition getPosition( const size_t pointIdx ) const;

    /**
     * The color of a given point.
     *
     * \throw WOutOfBounds if invalid index is used.
     * \param pointIdx the point index.
     *
     * \return the color
     */
    WColor getColor( const size_t pointIdx ) const;

    /**
     * Is this a valid point index?
     *
     * \param pointIdx the index to check
     *
     * \return true if yes.
     */
    bool isValidPointIdx( const size_t pointIdx ) const;

    /**
     * The type of colors we have for each point.
     */
    enum ColorType
    {
        GRAY = 1,
        RGB = 3,
        RGBA =4
    };

    /**
     * Check the type of color.
     *
     * \return the type
     */
    ColorType getColorType() const;

protected:
    /**
     * The prototype as singleton.
     */
    static boost::shared_ptr< WPrototyped > m_prototype;

private:
    /**
     * Point vector for all points
     */
    VertexArray m_vertices;

    /**
     * An array of the colors per vertex.
     */
    ColorArray m_colors;

    /**
     * Group index parameter for each vertex.
     */
    GroupArray m_groups;

    /**
     * Which colortype do we use in m_colors.
     */
    ColorType m_colorType;

    /**
     * Axis aligned bounding box for all point-vertices of this dataset.
     */
    WBoundingBox m_bb;

    /**
     * Initialize arrays and bbox if needed. Used during construction.
     *
     * \param calcBB if true, the bounding box is calculated
     */
    void init( bool calcBB = false );
};

#endif  // WDATASETPOINTSGROUPED_H
