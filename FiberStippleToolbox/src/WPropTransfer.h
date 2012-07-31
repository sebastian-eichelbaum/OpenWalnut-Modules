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

#ifndef WPROPTRANSFER_H
#define WPROPTRANSFER_H

#include <string>

#include "core/common/WProperties.h"
#include "core/common/WTransferable.h"

/**
 * Encapulates a WPropertyVariable for transfering it (shared_ptr) over module connectors.
 */
template< class T >
class WPropTransfer: public WTransferable
{
public:
    /**
     * Alias for a boost::shared_ptr on this.
     */
    typedef boost::shared_ptr< WPropTransfer< T > > SPtr;

    /**
     * Default constructor.
     */
    explicit WPropTransfer();

    /**
     * Create a new Transferable containing a link to a property.
     *
     * \param property
     */
    WPropTransfer( T property );

    /**
     * Destructor.
     */
    virtual ~WPropTransfer();

    /**
     * Gives back the name of this transferrable.
     * \return the name
     */
    virtual const std::string getName() const;

    /**
     * Gives back a description of this transferrable.
     * \return the description
     */
    virtual const std::string getDescription() const;

    /**
     * Returns a prototype instantiated with the true type of the deriving class.
     *
     * \return the prototype.
     */
    static boost::shared_ptr< WPrototyped > getPrototype();

    /**
     * Sets a new property, beeing transfered via this transferable.
     *
     * \param property
     */
    void setProperty( T property );

    /**
     * Retrieve the transfered property.
     *
     * \return Property.
     */
    T getProperty();

private:
    /**
     * The prototype as singleton.
     */
    static boost::shared_ptr< WPrototyped > m_prototype;

    /**
     * This is what is beeing shared with this transferable: a single property.
     */
    T m_property;
};

template< class T >
WPropTransfer< T >::WPropTransfer( T property )
    : m_property( property )
{
}

template< class T >
void WPropTransfer< T >::setProperty( T property )
{
    m_property = property;
}

template< class T >
T WPropTransfer< T >::getProperty()
{
    return m_property;
}

// prototype instance as singleton
template< class T >
boost::shared_ptr< WPrototyped > WPropTransfer< T >::m_prototype = boost::shared_ptr< WPrototyped >();

template< class T >
boost::shared_ptr< WPrototyped > WPropTransfer< T >::getPrototype()
{
    if( !m_prototype )
    {
        m_prototype = boost::shared_ptr< WPrototyped >( new WPropTransfer< T >() );
    }

    return m_prototype;
}

template< class T >
const std::string WPropTransfer< T >::getName() const
{
    return "Property transfer";
}

template< class T >
const std::string WPropTransfer< T >::getDescription() const
{
    return "Transfers a property trough connectors. Be careful as multiple threads could use this!";
}

template< class T >
WPropTransfer< T >::WPropTransfer()
{
}

template< class T >
WPropTransfer< T >::~WPropTransfer()
{
}

typedef WPropTransfer< WPropDouble > WPropDoubleTransfer;

#endif  // WPROPTRANSFER_H
