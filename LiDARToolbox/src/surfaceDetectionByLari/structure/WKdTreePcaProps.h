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

#ifndef WKDTREEPCAPROPS_H
#define WKDTREEPCAPROPS_H

#include "../../common/datastructures/kdtree/WKdTreeND.h"
#include "WLariGeoProps.h"

/**
 * This kd tree node enables nodes to hold metadata for each single point used by the 
 * process of the Lari/Habib (2014).
 */
class WKdTreePcaProps : public WKdTreeND
{
public:
    /**
     * Instantiates the kd tree node
     * \param dimensions The dimensions count
     */
    explicit WKdTreePcaProps( size_t dimensions );

    virtual ~WKdTreePcaProps();
    /**
     * Adds default metadata for each single point used by the process of the Lari/Habib 
     * (2014).
     */
    void createGeoProps();
    /**
     * Returns the metadata for each single point used by the process of the Lari/Habib 
     * (2014).
     * \return The metadata for each single point used by the process of the Lari/Habib 
     *         (2014).
     */
    WLariGeoProps* getGeoProps();

protected:
    /**
     * Enables the derived kd tree node class to create a new instance of that kd tree 
     * node class type.
     * \param dimensions The dimension count of the new kd tree node.
     * \return a new kd tree node instance of that class.
     */
    virtual WKdTreeND* getNewInstance( size_t dimensions );

private:
    /**
     * The metadata for each single point used by the process of the Lari/Habib (2014).
     */
    WLariGeoProps* m_geoProps;
};

#endif  // WKDTREEPCAPROPS_H
