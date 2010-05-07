//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMDS : implementaion of Salome mesh data structure
// File      : SMDS_SetIterator.hxx
// Created   : Mon Feb 27 16:57:43 2006
// Author    : Edward AGAPOV (eap)
//
#ifndef SMDS_SetIterator_HeaderFile
#define SMDS_SetIterator_HeaderFile

#include "SMDS_Iterator.hxx"

///////////////////////////////////////////////////////////////////////////////
/// Accessors to value pointed by iterator
///////////////////////////////////////////////////////////////////////////////

namespace SMDS {

  template<typename VALUE,typename VALUE_SET_ITERATOR>
  struct SimpleAccessor {
    static VALUE value(VALUE_SET_ITERATOR it) { return (VALUE) *it; }
  };

  template<typename VALUE,typename VALUE_SET_ITERATOR>
  struct KeyAccessor {
    static VALUE value(VALUE_SET_ITERATOR it) { return (VALUE) it->first; }
  };

  template<typename VALUE,typename VALUE_SET_ITERATOR>
  struct ValueAccessor {
    static VALUE value(VALUE_SET_ITERATOR it) { return (VALUE) it->second; }
  };
}

///////////////////////////////////////////////////////////////////////////////
/// SMDS_Iterator iterating over abstract set of values like STL containers
///
/// BE CAREFUL: iterator pointed value is static_cast'ed to VALUE
///
///////////////////////////////////////////////////////////////////////////////

template<typename VALUE,
         typename VALUE_SET_ITERATOR,
         typename ACCESOR=SMDS::SimpleAccessor<VALUE,VALUE_SET_ITERATOR> >
class SMDS_SetIterator : public SMDS_Iterator<VALUE>
{
protected:
  VALUE_SET_ITERATOR _beg, _end;
public:
  SMDS_SetIterator(const VALUE_SET_ITERATOR & begin,
                   const VALUE_SET_ITERATOR & end)
  { init ( begin, end ); }

  /// Initialization
  virtual void init(const VALUE_SET_ITERATOR & begin,
                    const VALUE_SET_ITERATOR & end)
  { _beg = begin; _end = end; }

  /// Return true if and only if there are other object in this iterator
  virtual bool more() { return _beg != _end; }

  /// Return the current object and step to the next one
  virtual VALUE next() { return ACCESOR::value( _beg++ ); }
};

///////////////////////////////////////////////////////////////////////////////
/// map iterators
///////////////////////////////////////////////////////////////////////////////

#include <map>
/*!
 * \brief iterator on values of a map
 */
template<typename M>
struct SMDS_mapIterator : public SMDS_SetIterator< typename M::mapped_type, typename M::const_iterator,
                                                   SMDS::ValueAccessor<typename M::mapped_type,
                                                                       typename M::const_iterator> > {
  typedef SMDS_SetIterator< typename M::mapped_type, typename M::const_iterator,
                            SMDS::ValueAccessor<typename M::mapped_type,
                                                typename M::const_iterator> > parent_type;
  SMDS_mapIterator(const M& m):parent_type(m.begin(),m.end()) {}
};
/*!
 * \brief reverse iterator on values of a map
 */
template<typename M>
struct SMDS_mapReverseIterator : public SMDS_SetIterator< typename M::mapped_type,
                                                          typename M::const_reverse_iterator,
                                                          SMDS::ValueAccessor<typename M::mapped_type,
                                                                              typename M::const_reverse_iterator> > {
  typedef SMDS_SetIterator< typename M::mapped_type, typename M::const_reverse_iterator,
                            SMDS::ValueAccessor<typename M::mapped_type,
                                                typename M::const_reverse_iterator> > parent_type;
  SMDS_mapReverseIterator(const M& m):parent_type(m.rbegin(),m.rend()) {}
};
/*!
 * \brief iterator on keys of a map
 */
template<typename M>
struct SMDS_mapKeyIterator : public SMDS_SetIterator< typename M::key_type, typename M::const_iterator,
                                                      SMDS::KeyAccessor<typename M::key_type,
                                                                        typename M::const_iterator> > {
  typedef SMDS_SetIterator< typename M::key_type, typename M::const_iterator,
                            SMDS::KeyAccessor<typename M::key_type,
                                              typename M::const_iterator> > parent_type;
  SMDS_mapKeyIterator(const M& m):parent_type(m.begin(),m.end()) {}
};
/*!
 * \brief reverse iterator on keys of a map
 */
template<typename M>
struct SMDS_mapKeyReverseIterator : public SMDS_SetIterator< typename M::key_type, typename M::const_iterator,
                                                            SMDS::KeyAccessor<typename M::key_type,
                                                                              typename M::const_iterator> > {
  typedef SMDS_SetIterator< typename M::key_type, typename M::const_iterator,
                            SMDS::KeyAccessor<typename M::key_type,
                                              typename M::const_iterator> > parent_type;
  SMDS_mapKeyReverseIterator(const M& m):parent_type(m.rbegin(),m.rend()) {}
};

///////////////////////////////////////////////////////////////////////////////
// useful specifications
///////////////////////////////////////////////////////////////////////////////

#include <vector>

class SMDS_MeshElement;
class SMDS_MeshNode;

typedef const SMDS_MeshElement* SMDS_pElement;
typedef const SMDS_MeshNode*    SMDS_pNode;

// element iterators

typedef SMDS_SetIterator< SMDS_pElement, std::vector< SMDS_pElement >::const_iterator>
SMDS_ElementVectorIterator;


typedef SMDS_SetIterator< SMDS_pElement, SMDS_pElement const *>
SMDS_ElementArrayIterator;


typedef SMDS_SetIterator< SMDS_pElement, std::vector< SMDS_pNode >::const_iterator>
SMDS_NodeVectorElemIterator;


typedef SMDS_SetIterator< SMDS_pElement, SMDS_pNode const * >
SMDS_NodeArrayElemIterator;

// node iterators

typedef SMDS_SetIterator< SMDS_pNode, std::vector< SMDS_pNode >::const_iterator >
SMDS_NodeVectorIterator;


typedef SMDS_SetIterator< SMDS_pNode, SMDS_pNode const * >
SMDS_NodeArrayIterator;


#endif
