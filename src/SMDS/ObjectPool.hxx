// Copyright (C) 2010-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#ifndef _OBJECTPOOL_HXX_
#define _OBJECTPOOL_HXX_

#include <vector>
#include <iostream>

#include "SMDS_Iterator.hxx"

namespace
{
  // assure deallocation of memory of a vector
  template<class Y> void clearVector(Y & v )
  {
    Y emptyVec; v.swap( emptyVec );
  }
}

template<class X> class ObjectPoolIterator;

template<class X> class ObjectPool
{

private:
  std::vector<X*>   _chunkList;
  std::vector<bool> _freeList;
  int               _nextFree;    // either the 1st hole or last added
  int               _maxAvail;    // nb allocated elements
  int               _chunkSize;
  int               _maxOccupied; // max used ID
  int               _nbHoles;
  int               _lastDelChunk;

  friend class ObjectPoolIterator<X>;

  int getNextFree()
  {
    // Don't iterate on the _freeList if all the "holes"
    // are filled. Go straight to the last occupied ID + 1
    if ( _nbHoles == 0 )
      return std::min(_maxOccupied + 1, _maxAvail);
    
    for (int i = _nextFree; i < _maxAvail; i++)
      if (_freeList[i] == true)
        {
          return i;
          break;
        }
    return _maxAvail;
  }

  void checkDelete(int chunkId)
  {
    int i0 = _chunkSize * chunkId;
    int i1 = _chunkSize * (chunkId + 1);
    for (int i = i0; i < i1; i++)
      if (_freeList[i] == false)
        return;
    std::cerr << "a chunk to delete" << std::endl;
    // compactage des vecteurs un peu lourd, pas necessaire
    //X* chunk = _chunkList[chunkId];
    //delete [] chunk;
  }

public:
  ObjectPool(int nblk = 1024)
  {
    _chunkSize    = nblk;
    _nextFree     = 0;
    _maxAvail     = 0;
    _maxOccupied  = -1;
    _nbHoles      = 0;
    _lastDelChunk = 0;
    _chunkList.clear();
    _freeList.clear();
  }

  virtual ~ObjectPool()
  {
    for (size_t i = 0; i < _chunkList.size(); i++)
      delete[] _chunkList[i];
  }

  X* getNew()
  {
    X *obj = 0;
    _nextFree = getNextFree();
    if (_nextFree == _maxAvail)
      {
        X* newChunk = new X[_chunkSize];
        _chunkList.push_back(newChunk);
        _freeList.insert(_freeList.end(), _chunkSize, true);
        _maxAvail += _chunkSize;
        _freeList[_nextFree] = false;
        obj = newChunk;
      }
    else
      {
        int chunkId = _nextFree / _chunkSize;
        int rank = _nextFree - chunkId * _chunkSize;
        _freeList[_nextFree] = false;
        obj = _chunkList[chunkId] + rank;
      }
    if (_nextFree <= _maxOccupied)
      {
        _nbHoles-=1;
      }
    else
      {
        _maxOccupied = _nextFree;
      }
    return obj;
  }

  void destroy(X* obj)
  {
    size_t i = 0;
    if ( obj >= _chunkList[ _lastDelChunk ] &&
         obj <  _chunkList[ _lastDelChunk ] + _chunkSize )
      i = _lastDelChunk;
    else
      for ( ; i < _chunkList.size(); i++ )
      {
        if ( obj >= _chunkList[ i ] &&
             obj <  _chunkList[ i ] + _chunkSize )
          break;
      }
    X*    chunk = _chunkList[i];
    long adrobj = (long) (obj);
    long adrmin = (long) (chunk);
    int  rank   = (adrobj - adrmin) / sizeof(X);
    int  toFree = i * _chunkSize + rank;
    _freeList[toFree] = true;
    if (toFree < _nextFree)
      _nextFree = toFree;
    if (toFree < _maxOccupied)
      ++_nbHoles;
    else
      --_maxOccupied;
    _lastDelChunk = i;
  }

  void clear()
  {
    _nextFree = 0;
    _maxAvail = 0;
    _maxOccupied = 0;
    _nbHoles = 0;
    _lastDelChunk = 0;
    for (size_t i = 0; i < _chunkList.size(); i++)
      delete[] _chunkList[i];
    clearVector( _chunkList );
    clearVector( _freeList );
  }

  // nb allocated elements
  size_t size() const
  {
    return _freeList.size();
  }

  // nb used elements
  size_t nbElements() const
  {
    return _maxOccupied + 1 - _nbHoles;
  }

  // return an element w/o any check
  const X* operator[]( size_t i ) const // i < size()
  {
    int chunkId = i / _chunkSize;
    int    rank = i - chunkId * _chunkSize;
    return _chunkList[ chunkId ] + rank;
  }

  // return only being used element
  const X* at( size_t i ) const // i < size()
  {
    if ( i >= size() || _freeList[ i ] )
      return 0;

    int chunkId = i / _chunkSize;
    int    rank = i - chunkId * _chunkSize;
    return _chunkList[ chunkId ] + rank;
  }

  //  void destroy(int toFree)
  //  {
  //    // no control 0<= toFree < _freeList.size()
  //    _freeList[toFree] = true;
  //    if (toFree < _nextFree)
  //      _nextFree = toFree;
  //  }

};

template<class X> class ObjectPoolIterator : public SMDS_Iterator<const X*>
{
  const ObjectPool<X>& _pool;
  int                  _i, _nbFound;
public:

  ObjectPoolIterator( const ObjectPool<X>& pool ) : _pool( pool ), _i( 0 ), _nbFound( 0 )
  {
    if ( more() && _pool._freeList[ _i ] == true )
    {
      next();
      --_nbFound;
    }
  }

  virtual bool more()
  {
    return ( _i <= _pool._maxOccupied && _nbFound < (int)_pool.nbElements() );
  }

  virtual const X* next()
  {
    const X* x = 0;
    if ( more() )
    {
      x = _pool[ _i ];

      ++_nbFound;

      for ( ++_i; _i <= _pool._maxOccupied; ++_i )
        if ( _pool._freeList[ _i ] == false )
          break;
    }
    return x;
  }
};

#endif
