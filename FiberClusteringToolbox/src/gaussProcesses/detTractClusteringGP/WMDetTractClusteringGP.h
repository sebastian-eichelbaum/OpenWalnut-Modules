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

#ifndef WMDETTRACTCLUSTERINGGP_H
#define WMDETTRACTCLUSTERINGGP_H

#include <string>
#include <map>
#include <utility>

#include <osg/Geode>

#include "core/common/math/WMatrixSym.h"
#include "core/kernel/WModule.h"
#include "core/kernel/WModuleInputData.h"
#include "core/kernel/WModuleOutputData.h"
#include "../WDataSetGP.h"

class WDendrogram;

/**
 * Module for clustering Gaussian processes which representing deterministic tracts.
 *
 * \ingroup modules
 */
class WMDetTractClusteringGP: public WModule
{
public:
    /**
     * Constructs a new clustering instance.
     */
    WMDetTractClusteringGP();

    /**
     * Destructs this.
     */
    virtual ~WMDetTractClusteringGP();

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
     * Computes the distant matrix for all pairs of Gaussian processes.
     *
     * \warning This function may leave an invalid matrix when the \ref m_shutdownFlag becomes true!
     *
     * \param dataSet The dataset of Gaussian processes.
     *
     * \return The similarity or also called distant matrix.
     */
    void computeDistanceMatrix( boost::shared_ptr< const WDataSetGP > dataSet );

    /**
     * Constructs a dendrogram out of the m_similarity matrix. Please note that this member function needs a valid similarity
     * matrix to operate correctly and it will leave an invalid matrix afterwards!
     *
     * \warning This function may return and leave an invalid matrix when the \ref m_shutdownFlag becomes true!
     *
     * \param n How many tracts
     *
     * \return The dendrogram.
     */
    boost::shared_ptr< WDendrogram > computeDendrogram( size_t n );

    /**
     * Input Connector for the Gaussian processes which are about to be clustered.
     */
    boost::shared_ptr< WModuleInputData< WDataSetGP > > m_gpIC;

    /**
     * Output Connector for the dendrogram which is about to be created with this module.
     */
    boost::shared_ptr< WModuleOutputData< WDendrogram > > m_dendOC;

    /**
     * Distant matrix of all pairs of Gaussian processes. This is float to save more space!
     */
    WMatrixSymFLT m_similarities;

private:
};

#endif  // WMDETTRACTCLUSTERINGGP_H
