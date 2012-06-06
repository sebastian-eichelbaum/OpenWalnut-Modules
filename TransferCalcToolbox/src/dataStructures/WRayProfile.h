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

#ifndef WRAYPROFILE_H
#define WRAYPROFILE_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "WRaySample.h"

/**
 * Class which encapsulates a so called Ray-profile. This is a set of sample-profiles along a ray through some data.
 */
class WRayProfile
{
public:
    /**
     * Convenience typedef for a boost::shared_ptr< WRayProfile >.
     */
    typedef boost::shared_ptr< WRayProfile > SPtr;

    /**
     * Convenience typedef for a boost::shared_ptr< const WRayProfile >.
     */
    typedef boost::shared_ptr< const WRayProfile > ConstSPtr;

    /**
     * Default constructor.
     */
    WRayProfile();

    /**
     * Default constructor. Creates an profile with the given number of samples. The profiles are not filled.
     *
     * \param nbSamples the number of samples along the ray used for this profile.
     */
    explicit WRayProfile( size_t nbSamples );

    /**
     * Destructor.
     */
    virtual ~WRayProfile();

    /**
     * Access the specified ray sample. Provide a valid ray sample ID. Invalid IDs cause an exception.
     *
     * \param sampleID the id of the sample
     *
     * \return the sample
     */
    const WRaySample& operator[]( size_t sampleID ) const;

    /**
     * Access the specified ray sample. Provide a valid ray sample ID. Invalid IDs cause an exception.
     *
     * \param sampleID the id of the sample
     *
     * \return the sample
     */
    WRaySample& operator[]( size_t sampleID );

    /**
     * Get the size of the Profile - the number of samples.
     *
     * \return the sample
     */
    virtual size_t size();

protected:
private:
    /**
     * The profile is a set of samples.
     */
    std::vector< WRaySample > m_profile;
};

#endif  // WRAYPROFILE_H

