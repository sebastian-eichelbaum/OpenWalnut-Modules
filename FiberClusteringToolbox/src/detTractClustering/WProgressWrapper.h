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

#ifndef WPROGRESSWRAPPER_H
#define WPROGRESSWRAPPER_H

struct ProgressWrapperData;

/**
 * Wraps the \ref WProgressCombiner and \ref WProgress classes so the cuda kernel does not need to
 * include \c boost::shared_ptr.
 */
class ProgressWrapper
{
public:
    /**
     * Constructs a wrapper instance
     *
     * \param d The data with the \ref WProgressCombiner and a member variable for placing the \ref
     * WProgress \c boost::shared_ptr.
     */
    explicit ProgressWrapper( ProgressWrapperData *d );

    /**
     * Destructs this wrapper.
     */
    ~ProgressWrapper();

    /**
     * Increments the encapsulated progress.
     *
     * \return
     */
    ProgressWrapper& operator++();

    /**
     * Wraps the WProgress construction so no std::string is used within the cuda kernel.
     *
     * \param description String describing this progress instance
     * \param count Max count indicating 100 percent.
     */
    void start( const char *description, int count );

    /**
     * Wraps the finish() call, indicating that the encapsulated progress has finished.
     */
    void finish();

private:
    /**
     * Reference to the combiner and the storage place for the progress. This needs to be
     * encapsulated once more, so it may be linked to the CUDA kernel, as far this is my
     * understanding by now.
     */
    ProgressWrapperData *d;
};

#endif  // WPROGRESSWRAPPER_H
