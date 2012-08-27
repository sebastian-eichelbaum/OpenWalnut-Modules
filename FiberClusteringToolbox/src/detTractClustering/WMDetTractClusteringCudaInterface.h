//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS, Copyright 2010 RRZK University of Cologne
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

#ifndef WMDETTRACTCLUSTERINGCUDAINTERFACE_H
#define WMDETTRACTCLUSTERINGCUDAINTERFACE_H

#include <boost/shared_ptr.hpp>

class WProgressCombiner;
template< typename T >
class WMatrixSymImpl;
typedef WMatrixSymImpl< double > WMatrixSym;
class WDataSetFiberVector;

/**
 * Initialize dLtTable with distances according to Zhang's dLt metric for
 * tracts. The tracts have to be sorted by length (descending).
 */
bool initDLtTableCuda( boost::shared_ptr< WMatrixSym > dLtTable,
                       const boost::shared_ptr< WDataSetFiberVector > tracts,
                       double proximity_threshold,
                       boost::shared_ptr< WProgressCombiner > progressCombiner );

#endif  // WMDETTRACTCLUSTERINGCUDAINTERFACE_H
