//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
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

#ifndef WMCLUSTERSLICER_H
#define WMCLUSTERSLICER_H

#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/common/datastructures/WColoredVertices.h"
#include "core/common/math/WPlane.h"
#include "core/common/WCondition.h"
#include "core/dataHandler/datastructures/WFiberCluster.h"
#include "core/dataHandler/datastructures/WJoinContourTree.h"
#include "core/dataHandler/WDataSetScalar.h"
#include "core/graphicsEngine/WGEGroupNode.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"

/**
 * Creates slices of a fiber cluster along its center line.
 * \warning ATM there are race conditions possible, e.g. a new FiberCluster arrives while the corresponding dataset is still in processing some
 * where. Hence we need an ensurance that the given dataset belongs to the given cluster!
 * \ingroup modules
 */
class WMClusterSlicer: public WModule
{
public:
    /**
     * Default constructor.
     */
    WMClusterSlicer();

    /**
     * Destructor.
     */
    virtual ~WMClusterSlicer();

    /**
     * Gives back the name of this module.
     * \return the module's name.
     */
    virtual const std::string getName() const;

    /**
     * Gives back a description of this module.
     * \return description to module.
     */
    virtual const std::string getDescription() const;

    /**
     * Due to the prototype design pattern used to build modules, this method returns a new instance of this method. NOTE: it
     * should never be initialized or modified in some other way. A simple new instance is required.
     *
     * \return the prototype used to create every module in OpenWalnut.
     */
    virtual boost::shared_ptr< WModule > factory() const;

    /**
     * Get the icon for this module in XPM format.
     * \return The icon.
     */
    virtual const char** getXPMIcon() const;

protected:
    /**
     * Entry point after loading the module. Runs in separate thread.
     */
    virtual void moduleMain();

    /**
     * Initialize the connectors this module is using.
     */
    virtual void connectors();

    /**
     * Initialize the properties for this module.
     */
    virtual void properties();

    /**
     * Callback for m_active. Overwrite this in your modules to handle m_active changes separately.
     */
    virtual void activate();

    /**
     * Updates either the planes representing the slices or the isovoxels of the volume
     *
     * \param force If true the scene is updated even if no property changed
     */
    void updateDisplay( bool force = false );

    /**
     * Computes the slices which are used for the statistics.
     */
    void generateSlices();

    /**
     * Slices the given mesh with the color of the slices used for slicing.
     *
     * \param mesh Triangle mesh describing the isosurface
     */
    void sliceAndColorMesh( boost::shared_ptr< WTriangleMesh > mesh );

    /**
     * Computes the average of the positions inside the paramDS which are also inside the cluster volume main component.
     * All other positions will be deleted from this set.
     *
     * \param samplePoints Set of position where the parameter should be gained out of the paramDS, be aware
     *
     * \return The average value of those value where its positions are inside the Iso-Surface/Volume/Cluster. If no position is inside, 0.0 is returned.
     */
    WValue< double > meanParameter( boost::shared_ptr< std::set< WPosition > > samplePoints ) const;

    /**
     * Generates new geode for marking the volume voxels
     *
     * \return Newly constructed geode
     */
    osg::ref_ptr< osg::Geode > generateIsoVoxelGeode() const;

    /**
     * Computes how much percent of the points of the cluster are enclosed by the isosurface.
     *
     * \param isoValue The isovalue associated with the surface
     *
     * \return coverage in percent
     */
    double countTractPointsInsideVolume( double isoValue ) const;

    /**
     * Computes the isovalue for the given coverage. That means almost $coverage percent of the points of the tracts will be enclosed by the isosurface.
     *
     * \param coverage A value between 0.0 and 1.0 specifying the enclosion of the points of the cluster by the isosurface.
     *
     * \return the isovalue associated with the given coverage.
     */
    double computeOptimalIsoValue( double coverage = 0.95 ) const;

    /**
     * The root node used for this modules graphics.
     */
    osg::ref_ptr< WGEGroupNode > m_rootNode;

    /**
     * Separate geode for voxels of the cluster volume
     */
    osg::ref_ptr< osg::Geode >   m_isoVoxelGeode;

    /**
     * Separate geode for slices
     */
    osg::ref_ptr< WGEGroupNode > m_sliceGeode;

    /**
     * Separate geode for the sample Points
     */
    osg::ref_ptr< WGEGroupNode > m_samplePointsGeode;

    /**
     * InputConnector for a fiber cluster with its CenterLine
     */
    boost::shared_ptr< WModuleInputData< WFiberCluster > > m_fiberClusterIC;

    /**
     * InputConnector for the dataset derived from a voxelized cluster
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > > m_voxelizedClusterIC;

    /**
     * InputConnector for the dataset of parameters like FA etc.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > > m_paramIC;

    /**
     * InputConnector for the triangle mesh
     */
    boost::shared_ptr< WModuleInputData< WTriangleMesh > > m_triangleMeshIC;

    /**
     * OutputConnector to forward the color Map to TriangleMeshRenderer
     */
    boost::shared_ptr< WModuleOutputData< WColoredVertices > > m_colorMapOC;

    /**
     * OutputConnector to forwarde the selected Mesh (e.g. if component selection is enabled )
     */
    boost::shared_ptr< WModuleOutputData< WTriangleMesh > > m_triangleMeshOC;

    /**
     * A cluster with its CenterLine
     */
    boost::shared_ptr< WFiberCluster >  m_cluster;

    /**
     * Dataset derived from a voxelized cluster
     */
    boost::shared_ptr< WDataSetScalar > m_clusterDS;

    /**
     * Dataset derived from a voxelized cluster
     */
    boost::shared_ptr< WDataSetScalar > m_paramDS;

    /**
     * stores all planes and their average parameters along centerLine
     */
    boost::shared_ptr< std::vector< std::pair< double, WPlane > > > m_slices;

    /**
     * Reference to the TriangleMesh to make intersections
     */
    boost::shared_ptr< WTriangleMesh > m_mesh;

    /**
     * Stores the color for vertices belonging to the intersection with the mesh and the planes
     */
    boost::shared_ptr< WColoredVertices > m_colorMap;

    /**
     * Stores the JoinTree
     */
    boost::shared_ptr< WJoinContourTree >   m_joinTree;

    /**
     * Stores the voxels belonging to the cluster volume of a certain iso value
     */
    boost::shared_ptr< std::set< size_t > > m_isoVoxels;

    /**
     * Mesh decomposed into connected components
     */
    boost::shared_ptr< std::list< boost::shared_ptr< WTriangleMesh > > > m_components;

    /**
     * Indicates a complete update of display and computed data (time consuming)
     */
    boost::shared_ptr< WCondition > m_fullUpdate;

    /**
     * En/Disable the display of cluster volume voxels
     */
    WPropBool   m_drawIsoVoxels;

    /**
     * En/Disable the display of slices along center line
     */
    WPropBool   m_drawSlices;

    /**
     * The isovalue selecting the size of the cluster volume
     */
    WPropDouble m_isoValue;

    /**
     * Selects the mean: 0 == arithmeticMean, 1 == geometricMean, 2 == median (default)
     */
    WPropInt    m_meanSelector;

    /**
     * how many sample points in first direction of the slice
     */
    WPropInt    m_planeNumX;

    /**
     * how many sample points in the second direction of the slice
     */
    WPropInt    m_planeNumY;

    /**
     * distance of the sample points on the slices
     */
    WPropDouble m_planeStepWidth;

    /**
     * rescales the centerline for using more or less slices.
     */
    WPropDouble m_centerLineScale;

    /**
     * If true, first the mesh is decomposed into its components (expensive!) & the biggest will be drawn
     */
    WPropBool   m_selectBiggestComponentOnly;

    /**
     * En/Disables alternative mesh coloring strategy
     */
    WPropBool   m_alternateColoring;

    /**
     * En/Disables custom color scaling
     */
    WPropBool   m_customScale;

    /**
     * A synthetic minMean value. All mean values below are mapped to 0
     */
    WPropDouble m_minScale;

    /**
     * color used for minMean
     */
    WPropColor  m_minScaleColor;

    /**
     * A synthetic maxMean value. All mean value above are mapped to 1
     */
    WPropDouble m_maxScale;

    /**
     * color used for maxMean
     */
    WPropColor  m_maxScaleColor;

    /**
     * maximum average (of sample points of a plane) parameter value over all planes
     */
    double m_maxMean;

    /**
     * minimum average (of sample points of a plane) parameter value over all planes
     */
    double m_minMean;

private:
    /**
     * A pair of plane indices. Just for convinience.
     */
    typedef std::pair< size_t, size_t > PlanePair;

    /**
     * Determines if the given vertex is in between the given two planes.
     *
     * \param vertex Which to test
     * \param pp Model the boundaries
     *
     * \return True if and only if the vertex is really in between the given two planes.
     */
    bool isInBetween( const WPosition& vertex, const PlanePair& pp ) const;

    /**
     * Determines all possible plane pairs then enclose the given vertex.
     *
     * \param vertex Which select the plane pairs
     *
     * \return A vector of plane pairs
     */
    std::vector< PlanePair > computeNeighbouringPlanePairs( const WPosition& vertex ) const;

    /**
     * Check for every consecutive pair of planes if it encloses the point better than any other consecutive pair. Also check
     * if the vertex is before the first plane or behind the last plane (e.g. is outside of the planes).
     *
     * \param pairs Vector of plane pairs
     * \param vertex The vertex selecting the closes plane pair
     *
     * \return The closes plane pair (pair of indices) or (0,0) if its outside of all planes.
     */
    PlanePair closestPlanePair( const std::vector< PlanePair >& pairs, const WPosition& vertex ) const;

    /**
     * Interpolates the color of two planes depending on which the vertex is closer.
     *
     * \param vertex Vertex to compute color for
     * \param pp The both planes
     *
     * \return The color of the vertex dependen on its distance to the planes and their colors.
     */
    WColor colorFromPlanePair( const WPosition& vertex, const PlanePair& pp ) const;

    /**
     * Maps the given mean value into an Interval [0,1] so later you may use this as a color channel.
     *
     * \param meanValue The value to map, e.g. relative minMean and maxMean
     *
     * \return A number between 0 and 1.
     */
    double mapMeanOntoScale( double meanValue ) const;
};

#endif  // WMCLUSTERSLICER_H

