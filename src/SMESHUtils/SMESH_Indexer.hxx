// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
// File      : SMESH_Indexer.hxx
// Created   : Tue May 21 18:24:01 2019
// Author    : Edward AGAPOV (eap)


#ifndef __SMESH_Indexer_HXX__
#define __SMESH_Indexer_HXX__

//================================================================================
/*!
 * \brief Converter of a pair of indices to a sole index, useful to make
 *        1D array behave as 2D one
 */
struct SMESH_Indexer
{
  size_t _xSize, _ySize;

  //! Initialize with size in two directions
  SMESH_Indexer( size_t xSize=0, size_t ySize=0 ): _xSize(xSize), _ySize(ySize) {}

  //! set size
  void set(size_t xSize, size_t ySize ) { _xSize = xSize, _ySize = ySize; }

  //! \return size of 1D array
  size_t size() const { return _xSize * _ySize; }

  // \return 1D index by two indices
  size_t operator()(size_t x, size_t y) const { return y * _xSize + x; }
};

//================================================================================
/*!
 * \brief Converter of a triple of indices to a sole index, useful to make
 *        1D array behave as 3D one
 */
struct SMESH_Indexer3D
{
  size_t _xSize, _ySize, _zSize;

  //! Initialize with size in two directions
  SMESH_Indexer3D( size_t xSize=0, size_t ySize=0, size_t zSize=0 ):
    _xSize(xSize), _ySize(ySize), _zSize(zSize) {}

  //! set size
  void set(size_t xSize, size_t ySize, size_t zSize )
  { _xSize = xSize, _ySize = ySize, _zSize = zSize; }

  //! \return size of 1D array
  size_t size() const { return _xSize * _ySize * _zSize; }

  // \return 1D index by three indices
  size_t operator()(size_t x, size_t y, size_t z) const { return z*_xSize*_ySize +  y*_xSize + x; }
};

//================================================================================
/*!
 * \brief Oriented converter of a pair of integers to a sole index
 *
 * Allows virtual transformation of an 1D array viewed as 2D one.
 * Possible transformations are inverse in one or two directions and exchange of
 * the directions. Any combination of these transformations is allowed.
 *
 * The following code picks up a transformation such that two known array items
 * appear in desired positions:
 * \code
 * for ( int ori = 0; ori < SMESH_OrientedIndexer::MAX_ORI+1; ++ori )
 * {
 *   SMESH_OrientedIndexer oriIndex( index, ori );
 *   if ( item1 == array[ oriIndex( i1, j1 ) ] &&
 *        item2 == array[ oriIndex( i2, j2 ) ])
 *   {
 *     // needed transformation found
 *   }
 * }
 * \endcode
 */
class SMESH_OrientedIndexer : public SMESH_Indexer
{
  typedef SMESH_Indexer TFather;
public:
  enum OriFlags //!< transformation types
    {
      REV_X = 1, REV_Y = 2, SWAP_XY = 4, MAX_ORI = REV_X|REV_Y|SWAP_XY
    };

  SMESH_OrientedIndexer( const SMESH_Indexer& indexer, const int oriFlags ):
    TFather( indexer._xSize, indexer._ySize ),
    _xRevFun( (oriFlags & REV_X) ? & reverse : & lazy ),
    _yRevFun( (oriFlags & REV_Y) ? & reverse : & lazy ),
    _swapFun( (oriFlags & SWAP_XY ) ? & swap : & lazy ),
    _xSizeOriented( indexer._xSize ),
    _ySizeOriented( indexer._ySize )
  {
    (*_swapFun)( _xSizeOriented, _ySizeOriented );
  }

  //!< Return index by XY
  size_t operator()(size_t x, size_t y) const
  {
    (*_swapFun)( x, y );
    (*_xRevFun)( x, const_cast< size_t& >( _xSize ));
    (*_yRevFun)( y, const_cast< size_t& >( _ySize ));
    return TFather::operator()( x, y );
  }

  //!< Return index for a corner
  size_t corner(bool xMax, bool yMax) const
  {
    size_t x = xMax, y = yMax, size = 2;
    (*_swapFun)( x, y );
    (*_xRevFun)( x, size );
    (*_yRevFun)( y, size );
    return TFather::operator()( x ? _xSize-1 : 0,
                                y ? _ySize-1 : 0 );
  }
  size_t xSize() const { return _xSizeOriented; }
  size_t ySize() const { return _ySizeOriented; }

private:

  typedef void (*TFun)(size_t& x, size_t& y);
  TFun _xRevFun, _yRevFun, _swapFun;

  size_t _xSizeOriented, _ySizeOriented;

  static void lazy   (size_t&  , size_t& ) {}
  static void reverse(size_t& x, size_t& size) { x = size - x - 1; }
  static void swap   (size_t& x, size_t& y) { std::swap( x, y ); }
};


#endif
