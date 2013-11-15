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

using osg::Vec3;

namespace laslibb
{
    class WLasTool
    {
    public:
        WLasTool();
        WLasTool( boost::shared_ptr< WProgressCombiner > progress );
        virtual ~WLasTool();
        boost::shared_ptr< WDataSetPoints > getPoints  ();

    private:
        void setProgressSettings( size_t steps );

        boost::shared_ptr< WDataSetPoints > m_outputPoints;
        boost::shared_ptr< WProgressCombiner > m_associatedProgressCombiner;
        boost::shared_ptr< WProgress > m_progressStatus;
    };
} /* namespace butterfly */
#endif  // WLASTOOL_H
