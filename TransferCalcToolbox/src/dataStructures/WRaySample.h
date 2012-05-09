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

#ifndef WRAYSAMPLE_H
#define WRAYSAMPLE_H

/**
 * Encapsulates all the possible sample data on a ray.
 */
class WRaySample
{
public:
    /**
     * Default constructor.
     */
    WRaySample();

    /**
     * Destructor.
     */
    virtual ~WRaySample();

    /**
     * Access the value of the sample point.
     *
     * \return the value reference.
     */
    const double& value() const;

    /**
     * Access the value of the sample point.
     *
     * \return the value reference.
     */
    double& value();

protected:
private:
    /**
     * The value at this sample point.
     */
    double m_value;
};

#endif  // WRAYSAMPLE_H

