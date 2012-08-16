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

#ifndef WMFIBERSTIPPLES_H
#define WMFIBERSTIPPLES_H

#include <string>

#include "../WMAbstractSliceModule.h"
#include "WSampler2D.h"

// forward declarations to reduce compile dependencies
class WDataSetScalar;
class WDataSetVector;

/**
 * Draws Fiber Stipples on slice in order to visualize probabilistic tractograms.
 *
 * \note For further information see http://dx.doi.org/10.1109/BioVis.2011.6094044.
 *
 * \ingroup modules
 */
class WMFiberStipples: public WMAbstractSliceModule
{
public:
    /**
     * Default constructor.
     */
    WMFiberStipples();

    /**
     * Destructor.
     */
    virtual ~WMFiberStipples();

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

private:
    osg::ref_ptr< osg::Geode > genScatteredDegeneratedQuads( const WSampler2D& glyphPositions, osg::Vec3 const& base,
            osg::Vec3 const& a, osg::Vec3 const& b, size_t sliceNum ) const;

    /**
     * Initialize OSG root node for this module. All other nodes from this module should be attached to this root node.
     *
     * \param probTract Pointer to dataset containing the connectivity scores or probabilities. We need this for two things:
     * First, determine the maxium of connectivity scores to, scale the tracts between 0.0...1.0 and secondly to determine the
     * boundingbox for the scene. (Furthermore it is assumed that the vectors are also available within this BB.)
     * \param axis Selects the axis aligned plane (aka slice) by number. 0 => sagittal, 1 => coronal, 2=> axial.
     */
    void initOSG( boost::shared_ptr< WDataSetScalar > probTract, const size_t axis );

    /**
     * The probabilistic tractogram input connector.
     */
    boost::shared_ptr< WModuleInputData< WDataSetScalar > > m_probIC;

    /**
     * Input connector for the largest eigen vector dataset.
     */
    boost::shared_ptr< WModuleInputData< WDataSetVector > > m_vectorIC;

    /**
     * Color for the fiber stipples.
     */
    WPropColor m_color;

    /**
     * Minimal density of the fiberstipples.
     */
    WPropDouble m_minRange;

    /**
     * Maximal density of the fiberstipples.
     */
    WPropDouble m_maxRange;

    /**
     * Connectivity scores below that threshold will not be rendered.
     */
    WPropDouble m_threshold;

    /**
     * Determines the size of the quad used to stamp out the stipples.
     */
    WPropDouble m_glyphSize;

    /**
     * For scale the thickness of the stipples.
     */
    WPropDouble m_glyphThickness;

    /**
     * For initial slice positioning we need to control if the module is in intial state or not.
     */
    bool m_first;

    // TODO(math): Remove this ugly hack as soon as possible
    std::vector< WSampler2D > m_samplers;
    WPropBool m_oldNew;
};

#endif  // WMFIBERSTIPPLES_H
