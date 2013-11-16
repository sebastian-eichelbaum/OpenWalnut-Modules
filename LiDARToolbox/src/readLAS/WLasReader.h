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

#ifndef WLASTOOL_H
#define WLASTOOL_H

#include <fstream>  // std::ifstream
#include <iostream> // std::cout

#include <string>
#include <vector>

#include "core/kernel/WModule.h"
#include "core/kernel/WKernel.h"
#include "core/graphicsEngine/WTriangleMesh.h"
#include "core/dataHandler/WDataSetPoints.h"
#include "core/common/datastructures/WColoredVertices.h"

using osg::Vec3;

namespace laslibb
{
    /** 
     * Compound that reads a LiDAR file (see www.liblas.org).
     * These *.las files consist of a point set of X/Y/Z coordinates 
     * paired with the intensity/color.
     */
    class WLasReader
    {
    public:
	/**
	 * Instance to get the reader not using a module.
	 */
        WLasReader();
	
	/**
	 * Instance to get the reader using a module.
	 * \param progress Associated progress status bar.
	 */
        explicit WLasReader( boost::shared_ptr< WProgressCombiner > progress );
	
	/**
	 * Reader object destructor.
	 */
        virtual ~WLasReader();
	
	/**
	 * Returns the read LiDAR data set points.
	 * \param filePath LiDAR (*.las) file path to read.
	 * \return LiDAR data set points containing intensities/colors.
	 */
        boost::shared_ptr< WDataSetPoints > getPoints( const char* filePath );

    private:
        /**
         * Sets the linked module progress bar settings.
	 * \param steps Points count as reference to the progress bar.
	 */
        void setProgressSettings( size_t steps );

	/**
	 * Field to output the WDataSetPoints points data.
	 */
        boost::shared_ptr< WDataSetPoints > m_outputPoints;
	/**
	 * Linkd module progress bar.
	 */
        boost::shared_ptr< WProgressCombiner > m_associatedProgressCombiner;
	/**
	 * Progress bar status.
	 */
        boost::shared_ptr< WProgress > m_progressStatus;
    };
} /* namespace butterfly */
#endif  // WLASTOOL_H
