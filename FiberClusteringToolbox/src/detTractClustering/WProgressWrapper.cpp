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

#include "core/common/WProgress.h"
#include "core/common/WProgressCombiner.h"
#include "WProgressWrapper.h"
#include "WProgressWrapperData.h"

ProgressWrapper::ProgressWrapper( ProgressWrapperData *d )
    : d( d )
{
}

ProgressWrapper::~ProgressWrapper()
{
}

void ProgressWrapper::start( const char *description, int count )
{
    d->progress = boost::shared_ptr< WProgress >( new WProgress( description, count ) );
    if( d->progressCombiner )
    {
        d->progressCombiner->addSubProgress( d->progress );
    }
}

ProgressWrapper &ProgressWrapper::operator++()
{
    ++*( d->progress );
    return *this;
}

void ProgressWrapper::finish()
{
    d->progress->finish();
}
