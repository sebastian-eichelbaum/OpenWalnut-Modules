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

#include <boost/shared_ptr.hpp>

#include <core/kernel/WModule.h>

#include "clusterParamDisplay/WMClusterParamDisplay.h"
#include "clusterSlicer/WMClusterSlicer.h"
#include "detTractClustering/WMDetTractClustering.h"
#include "detTractCulling/WMDetTractCulling.h"
#include "directionHistogram/WMDirectionHistogram.h"
#include "edgeBundling/WMEdgeBundling.h"
#include "gaussProcesses/detTract2GPConvert/WMDetTract2GPConvert.h"
#include "gaussProcesses/detTractClusteringGP/WMDetTractClusteringGP.h"
#include "gaussProcesses/gpView/WMGpView.h"
#include "WToolkit.h"

// This file's purpose is to provide a list of modules as entry point for OpenWalnut's module loader.
// Add your modules here. If you miss this step, OpenWalnut will not be able to load your modules.
extern "C" void WLoadModule( WModuleList& m ) // NOLINT
{
    m.push_back( WModule::SPtr( new WMClusterParamDisplay ) );
    m.push_back( WModule::SPtr( new WMClusterSlicer ) );
    m.push_back( WModule::SPtr( new WMDetTract2GPConvert ) );
    m.push_back( WModule::SPtr( new WMDetTractClustering ) );
    m.push_back( WModule::SPtr( new WMDetTractClusteringGP ) );
    m.push_back( WModule::SPtr( new WMDetTractCulling ) );
    m.push_back( WModule::SPtr( new WMDirectionHistogram ) );
    m.push_back( WModule::SPtr( new WMEdgeBundling ) );
    m.push_back( WModule::SPtr( new WMGpView ) );
}

