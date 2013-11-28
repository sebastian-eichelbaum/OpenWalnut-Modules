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

#ifndef WDATASETMATRIXSYM_H
#define WDATASETMATRIXSYM_H

#include <string>

#include <boost/shared_ptr.hpp>

#include <core/common/math/WMatrixSym.h>
#include <core/dataHandler/WDataSet.h>

/**
 * This makes a symmetrical matrix transferrable and saveable and connectable.
 *
 * \tparam T Elementtype of the matrix.
 */
template< typename T >
class WDataSetMatrixSym : public WDataSet
{
public:
    /**
     * Shorthand for shared pointers on this type.
     */
    typedef boost::shared_ptr< WDataSetMatrixSym< T > > SPtr;

    /**
     * Default empty dataset for symmetric matrices.
     */
    WDataSetMatrixSym();

    /**
     * Dataset containing given matrix.
     *
     * \param matrix
     */
    explicit WDataSetMatrixSym( typename WMatrixSym< T >::SPtr matrix );

    /**
     * Gets the name of this prototype.
     *
     * \return the name.
     */
    virtual const std::string getName() const;

    /**
     * Gets the description for this prototype.
     *
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
     * Number of rows ( and cols, as only squared sym matrices are allowed).
     *
     * \return number of rows.
     */
    size_t size() const;

    /**
     * Return a pointer to the internal data.
     *
     * \return Pointer to data
     */
    typename WMatrixSym< T >::SPtr getData();
private:
    /**
     * Sets explicitly the internal data
     *
     * \param matrix which has the data to store
     */
    void setData( typename WMatrixSym< T >::SPtr matrix );

    /**
     * Data for the given symmetric Matrix.
     */
    typename WMatrixSym< T >::SPtr m_matrix;

    /**
     * The prototype as singleton.
     */
    static boost::shared_ptr< WPrototyped > m_prototype;
};

template< typename T >
inline typename WMatrixSym< T >::SPtr WDataSetMatrixSym< T >::getData()
{
    return m_matrix;
}

template< typename T >
inline size_t WDataSetMatrixSym< T >::size() const
{
    if( !m_matrix )
    {
        return 0;
    }
    return m_matrix->size();
}

template< typename T >
inline WDataSetMatrixSym< T >::WDataSetMatrixSym()
  : WDataSet()
{
}

template< typename T >
inline WDataSetMatrixSym< T >::WDataSetMatrixSym( typename WMatrixSym< T >::SPtr matrix )
  : WDataSet()
{
    setData( matrix );
}

template< typename T >
inline void WDataSetMatrixSym< T >::setData( typename WMatrixSym< T >::SPtr matrix )
{
  m_matrix = matrix;
}

// prototype instance as singleton
template< typename T >
boost::shared_ptr< WPrototyped > WDataSetMatrixSym< T >::m_prototype = boost::shared_ptr< WPrototyped >();

template< typename T >
inline boost::shared_ptr< WPrototyped > WDataSetMatrixSym< T >::getPrototype()
{
    if( !m_prototype )
    {
        m_prototype = boost::shared_ptr< WPrototyped >( new WDataSetMatrixSym< T >() );
    }

    return m_prototype;
}

template< typename T >
inline const std::string WDataSetMatrixSym< T >::getName() const
{
    return "Symmetrix Matrix";
}

template< typename T >
inline const std::string WDataSetMatrixSym< T >::getDescription() const
{
    return "Stores only the upper triangle matrix of a given symmetric matrix.";
}

typedef WDataSetMatrixSym< double > WDataSetMatrixSymDBL;
typedef WDataSetMatrixSym< int > WDataSetMatrixSymINT;
typedef WDataSetMatrixSym< float > WDataSetMatrixSymFLT;

#endif  // WDATASETMATRIXSYM_H
