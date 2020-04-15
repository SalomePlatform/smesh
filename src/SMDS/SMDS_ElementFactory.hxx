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
//  File   : SMDS_ElementFactory.hxx
//  Module : SMESH
//
#ifndef _SMDS_ElementFactory_HeaderFile
#define _SMDS_ElementFactory_HeaderFile

#include "SMDS_MeshCell.hxx"
#include "SMDS_Position.hxx"

#include <Utils_SALOME_Exception.hxx>

#include <boost/container/flat_set.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/make_shared.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>

#include <set>

#include <vtkType.h>

class SMDS_ElementChunk;
class SMDS_Mesh;
class SMDS_MeshCell;
class SMDS_MeshNode;

struct _ChunkCompare {
  bool operator () (const SMDS_ElementChunk* c1, const SMDS_ElementChunk* c2) const;
};
typedef boost::ptr_vector<SMDS_ElementChunk>       TChunkVector;
typedef std::set<SMDS_ElementChunk*,_ChunkCompare> TChunkPtrSet;

//------------------------------------------------------------------------------------
/*!
 * \brief Allocate SMDS_MeshElement's (SMDS_MeshCell's or SMDS_MeshNode's )
 *        and bind some attributes to elements:
 *        element ID, element VTK ID, sub-mesh ID, position on shape.
 *
 * Elements are allocated by chunks, so there are used and non-used elements
 */
class SMDS_ElementFactory
{
protected:
  bool                     myIsNodal;          // what to allocate: nodes or cells
  SMDS_Mesh*               myMesh;
  TChunkVector             myChunks;           // array of chunks of elements
  TChunkPtrSet             myChunksWithUnused; // sorted chunks having unused elements
  std::vector< vtkIdType > myVtkIDs;           // myVtkIDs[ smdsID-1 ] == vtkID
  std::vector< int >       mySmdsIDs;          // mySmdsIDs[ vtkID ] == smdsID - 1
  int                      myNbUsedElements;   // counter of elements

  friend class SMDS_ElementChunk;

public:

  SMDS_ElementFactory( SMDS_Mesh* mesh, const bool isNodal=false );
  virtual ~SMDS_ElementFactory();

  //! Return minimal ID of a non-used element
  int GetFreeID();

  //! Return maximal ID of an used element
  int GetMaxID();

  //! Return minimal ID of an used element
  int GetMinID();

  //! Return an element by ID. NULL if the element with the given ID is already used
  SMDS_MeshElement* NewElement( const int id );

  //! Return a SMDS_MeshCell by ID. NULL if the cell with the given ID is already used
  SMDS_MeshCell* NewCell( const int id ) { return static_cast<SMDS_MeshCell*>( NewElement( id )); }

  //! Return an used element by ID. NULL if the element with the given ID is not yet used
  const SMDS_MeshElement* FindElement( const int id ) const;

  //! Return a number of used elements
  int NbUsedElements() const { return myNbUsedElements; }

  //! Return an iterator on all element filtered using a given filter.
  //  nbElemsToReturn is used to optimize by stopping the iteration as soon as
  //  all elements satisfying filtering condition encountered.
  template< class ElemIterator >
  boost::shared_ptr< ElemIterator > GetIterator( SMDS_MeshElement::Filter* filter,
                                                 size_t nbElemsToReturn = -1 );

  //! Return an iterator on all element assigned to a given shape.
  //  nbElemsToReturn is used to optimize by stopping the iteration as soon as
  //  all elements assigned to the shape encountered.
  //  sm1stElem is used to quickly find the first chunk holding elements of the shape;
  //  it must have smallest ID between elements on the shape
  template< class ElemIterator >
  boost::shared_ptr< ElemIterator > GetShapeIterator( int                     shapeID,
                                                      size_t                  nbElemsToReturn,
                                                      const SMDS_MeshElement* sm1stElem );

  //! Mark the element as non-used
  void Free( const SMDS_MeshElement* );

  //! Return an SMDS ID by a Vtk one
  int FromVtkToSmds( vtkIdType vtkID );

  //! De-allocate all elements
  virtual void Clear();

  //! Remove unused elements located not at the end of the last chunk.
  //  Minimize allocated memory
  virtual void Compact(std::vector<int>& idCellsOldToNew);

  //! Return true if Compact() will change IDs of elements
  virtual bool CompactChangePointers();

  //! Return a number of elements in a chunk
  static int ChunkSize();
};

//------------------------------------------------------------------------------------
/*!
 * \brief Allocate SMDS_MeshNode's
 */
class SMDS_NodeFactory : public SMDS_ElementFactory
{
  std::vector<char> myShapeDim; // dimension of shapes

public:

  SMDS_NodeFactory( SMDS_Mesh* mesh );
  ~SMDS_NodeFactory();

  //! Return a SMDS_MeshNode by ID. NULL if the node with the given ID is already used
  SMDS_MeshNode* NewNode( int id ) { return (SMDS_MeshNode*) NewElement(id); }

  //! Return an used node by ID. NULL if the node with the given ID is not yet used
  const SMDS_MeshNode* FindNode( int id ) { return (const SMDS_MeshNode*) FindElement(id); }

  //! Set a total number of sub-shapes in the main shape
  void SetNbShapes( size_t nbShapes );

  //! Return a dimension of a shape
  int  GetShapeDim( int shapeID ) const;

  //! Set a dimension of a shape
  void SetShapeDim( int shapeID, int dim );

  //! De-allocate all nodes
  virtual void Clear();

  //! Remove unused nodes located not at the end of the last chunk.
  //  Minimize allocated memory
  virtual void Compact(std::vector<int>& idNodesOldToNew);

  //! Return true if Compact() will change IDs of node
  virtual bool CompactChangePointers();
};

//------------------------------------------------------------------------------------
/*!
 * \brief Range of elements in a chunk having the same attribute value
 */
template< typename ATTR>
struct _Range
{
  typedef ATTR attr_t;

  attr_t myValue; // common attribute value
  int    my1st;   // index in the chunk of the 1st element 
  _Range( int i0 = 0, attr_t v = 0 ): myValue( v ), my1st( i0 ) {}

  bool operator < (const _Range& other) const { return my1st < other.my1st; }
};

typedef std::vector< std::pair< int, int > > TIndexRanges;

//------------------------------------------------------------------------------------
/*!
 * \brief Sorted set of ranges
 */
template< class RANGE >
struct _RangeSet
{
  typedef typename RANGE::attr_t              attr_t;
  typedef boost::container::flat_set< RANGE > set_t;
  typedef typename set_t::const_iterator      set_iterator;

  set_t mySet;

  _RangeSet() { mySet.insert( RANGE( 0, 0 )); }

  /*!
   * \brief Return a number of ranges
   */
  size_t Size() const { return mySet.size(); }

  /*!
   * \brief Return a mutable _Range::my1st of a range pointed by an iterator
   */
  int&   First( set_iterator rangePtr ) { return const_cast< int& >( rangePtr->my1st ); }

  /*!
   * \brief Return a number of elements in a range pointed by an iterator
   */
  size_t Size( set_iterator rangePtr ) const
  {
    int next1st =
      ( rangePtr + 1 == mySet.end() ) ? SMDS_ElementFactory::ChunkSize() : ( rangePtr + 1 )->my1st;
    return next1st - rangePtr->my1st;
  }

  /*!
   * \brief Return ranges of indices (from,to) of elements having a given value
   */
  bool GetIndices( const attr_t theValue, TIndexRanges & theIndices,
                   const attr_t* theMinValue = 0, const attr_t* theMaxValue = 0) const
  {
    bool isFound = false;

    // if ( sizeof( attr_t ) == sizeof( int ) && theMinValue )
    //   if ( theValue < *theMinValue || theValue > *theMaxValue )
    //     return isFound;

    for ( set_iterator it = mySet.begin(); it < mySet.end(); ++it )
    {
      if ( it->myValue == theValue )
      {
        theIndices.push_back( std::make_pair( it->my1st, it->my1st + Size( it )));
        isFound = true;
        ++it; // the next range value differs from theValue
      }
    }
    return isFound;
  }

  /*!
   * \brief Return value of an element attribute
   *  \param [in] theIndex - element index
   *  \return attr_t - attribute value
   */
  attr_t GetValue( int theIndex ) const
  {
    set_iterator r = mySet.upper_bound( theIndex ) - 1;
    return r->myValue;
  }

  /*!
   * \brief Change value of an element attribute
   *  \param [in] theIndex - element index
   *  \param [in] theValue - attribute value
   *  \return attr_t - previous value
   */
  attr_t SetValue( int theIndex, attr_t theValue )
  {
    set_iterator rNext = mySet.end(); // case of adding elements
    set_iterator     r = rNext - 1;
    if ( r->my1st > theIndex )
    {
      rNext = mySet.upper_bound( theIndex );
      r     = rNext - 1;
    }
    int          rSize = Size( r ); // range size
    attr_t      rValue = r->myValue;
    if ( rValue == theValue )
      return rValue; // it happens while compacting

    if ( r->my1st == theIndex ) // theIndex is the first in the range
    {
      bool joinPrev = // can join theIndex to the previous range
        ( r->my1st > 0 && ( r-1 )->myValue == theValue );

      if ( rSize == 1 )
      {
        bool joinNext = // can join to the next range
          ( rNext != mySet.end() && rNext->myValue == theValue );

        if ( joinPrev )
        {
          if ( joinNext ) // && joinPrev
          {
            mySet.erase( r, r + 2 );
          }
          else // joinPrev && !joinNext
          {
            mySet.erase( r );
          }
        }
        else
        {
          if ( joinNext ) // && !joinPrev
          {
            r = mySet.erase( r ); // then r points to the next range
            First( r )--;
          }
          else // !joinPrev && !joinNext
          {
            const_cast< attr_t & >( r->myValue ) = theValue;
          }
        }
      }
      else // if rSize > 1
      {
        if ( joinPrev )
        {
          First( r )++;
        }
        else
        {
          r = mySet.insert( r, RANGE( theIndex + 1, rValue )) - 1;
          const_cast< attr_t & >( r->myValue ) = theValue;
        }
      }
    }
    else if ( r->my1st + rSize - 1 == theIndex ) // theIndex is last in the range
    {
      if ( rNext != mySet.end() && rNext->myValue == theValue ) // join to the next
      {
        First( rNext )--;
      }
      else
      {
        mySet.insert( r, RANGE( theIndex, theValue ));
      }
    }
    else // theIndex in the middle of the range
    {
      r = mySet.insert( r, RANGE( theIndex,     theValue ));
      r = mySet.insert( r, RANGE( theIndex + 1, rValue ));
    }
    return rValue;
  }
}; // struct _RangeSet


typedef _Range< int >  _ShapeIDRange; // sub-mesh ID range
typedef _Range< bool > _UsedRange;    // range of used elements

typedef _RangeSet< _ShapeIDRange > TSubIDRangeSet;
typedef _RangeSet< _UsedRange >    TUsedRangeSet;
typedef boost::dynamic_bitset<>    TBitSet;
//typedef float                       TParam;
typedef double                     TParam;
//typedef std::unordered_set<int>    TSubIDSet;

//------------------------------------------------------------------------------------
/*!
 * \brief Allocate SMDS_MeshElement's (SMDS_MeshCell's or SMDS_MeshNode's )
 *        and bind some attributes to elements:
 *        element ID, sub-shape ID, isMarked flag, parameters on shape
 */
class SMDS_ElementChunk
{
  SMDS_ElementFactory* myFactory;     // holder of this chunk
  SMDS_MeshElement*    myElements;    // array of elements
  int                  my1stID;       // ID of myElements[0]
  TBitSet              myMarkedSet;   // mark some elements
  TUsedRangeSet        myUsedRanges;  // ranges of used/unused elements
  TSubIDRangeSet       mySubIDRanges; // ranges of elements on the same sub-shape
  //TSubIDSet*           mySubIDSet;    // set of sub-shape IDs
  // int                  myMinSubID;    // min sub-shape ID
  // int                  myMaxSubID;    // max sub-shape ID
  std::vector<TParam>  myPositions;   // UV parameters on shape: 2*param_t per an element

public:

  SMDS_ElementChunk( SMDS_ElementFactory* factory = 0, int id0 = 0 );
  ~SMDS_ElementChunk();

  //! Return an element by an index [0,ChunkSize()]
  SMDS_MeshElement* Element(int index) { return & myElements[index]; }

  //! Return an element by an index [0,ChunkSize()]
  const SMDS_MeshElement* Element(int index) const { return & myElements[index]; }

  //! Return ID of the first non-used element
  int  GetUnusedID() const;

  //! Mark an element as used
  void UseElement( const int index );

  //! Mark an element as non-used
  void Free( const SMDS_MeshElement* e );

  //! Check if a given range holds used or non-used elements
  static bool IsUsed( const _UsedRange& r ) { return r.myValue; }

  //! Return index of an element in the chunk
  int Index( const SMDS_MeshElement* e ) const { return e - myElements; }

  //! Return ID of the 1st element in the chunk
  int Get1stID() const { return my1stID; }

  //! Return pointer to on-shape-parameters of a node
  TParam* GetPositionPtr( const SMDS_MeshElement* node, bool allocate=false );

  //! Return ranges of used/non-used elements
  const TUsedRangeSet&  GetUsedRanges() const { return myUsedRanges; }
  const TUsedRangeSet&  GetUsedRangesMinMax( bool& min, bool& max ) const
  { min = false; max = true; return myUsedRanges; }

  //! Return ranges of elements assigned to sub-shapes and min/max of sub-shape IDs
  const TSubIDRangeSet& GetSubIDRangesMinMax( int& min, int& max ) const
  { /*min = myMinSubID; max = myMaxSubID;*/ return mySubIDRanges; }

  //! Minimize allocated memory
  void Compact();

  //! Print some data
  void Dump() const; // debug


  // Methods called by SMDS_MeshElement

  int  GetID( const SMDS_MeshElement* e ) const;

  int  GetVtkID( const SMDS_MeshElement* e ) const;
  void SetVTKID( const SMDS_MeshElement* e, const vtkIdType id );

  int  GetShapeID( const SMDS_MeshElement* e ) const;
  void SetShapeID( const SMDS_MeshElement* e, int shapeID ) const;

  bool IsMarked   ( const SMDS_MeshElement* e ) const;
  void SetIsMarked( const SMDS_MeshElement* e, bool is );

  SMDS_PositionPtr GetPosition( const SMDS_MeshNode* n ) const;
  void SetPosition( const SMDS_MeshNode* n, const SMDS_PositionPtr& pos, int shapeID );

  SMDS_Mesh* GetMesh() { return myFactory->myMesh; }
};

//------------------------------------------------------------------------------------
/*!
 * \brief Iterator on elements in chunks
 */
template< class ELEM_ITERATOR, class RANGE_SET >
struct _ChunkIterator : public ELEM_ITERATOR
{
  typedef typename ELEM_ITERATOR::value_type    element_type;
  typedef SMDS_MeshElement::Filter*             filter_ptr;
  typedef typename RANGE_SET::attr_t            attr_type;
  typedef const RANGE_SET& (SMDS_ElementChunk::*get_rangeset_fun)(attr_type&, attr_type&) const;

  const SMDS_MeshElement* myElement;
  TIndexRanges            myRanges;
  int                     myRangeIndex;
  const TChunkVector&     myChunks;
  int                     myChunkIndex;
  get_rangeset_fun        myGetRangeSetFun;
  attr_type               myValue;
  attr_type               myMinValue;
  attr_type               myMaxValue;
  filter_ptr              myFilter;
  size_t                  myNbElemsToReturn;
  size_t                  myNbReturned;

  _ChunkIterator( const TChunkVector &      theChunks,
                  get_rangeset_fun          theGetRangeSetFun,
                  attr_type                 theAttrValue,
                  SMDS_MeshElement::Filter* theFilter,
                  size_t                    theNbElemsToReturn = -1,
                  int                       theChunkIndex = 0):
    myElement( 0 ),
    myRangeIndex( 0 ),
    myChunks( theChunks ),
    myChunkIndex( theChunkIndex-1 ),
    myGetRangeSetFun( theGetRangeSetFun ),
    myValue( theAttrValue ),
    myFilter( theFilter ),
    myNbElemsToReturn( theNbElemsToReturn ),
    myNbReturned( 0 )
  {
    next();
  }
  ~_ChunkIterator()
  {
    delete myFilter;
  }

  virtual bool more()
  {
    return myElement;
  }

  virtual element_type next()
  {
    element_type result = (element_type) myElement;
    myNbReturned += bool( result );

    myElement = 0;
    if ( myNbReturned < myNbElemsToReturn )
      while ( ! nextInRange() )
      {
        if ( ++myRangeIndex >= (int)myRanges.size() )
        {
          myRanges.clear();
          myRangeIndex = 0;
          while ( ++myChunkIndex < (int)myChunks.size() &&
                  !getRangeSet().GetIndices( myValue, myRanges, &myMinValue, &myMaxValue ))
            ;
          if ( myChunkIndex >= (int)myChunks.size() )
            break;
        }
      }
    return result;
  }

  bool nextInRange()
  {
    if ( myRangeIndex < (int)myRanges.size() )
    {
      std::pair< int, int > & range = myRanges[ myRangeIndex ];
      while ( range.first < range.second && !myElement )
      {
        myElement = myChunks[ myChunkIndex ].Element( range.first++ );
        if ( !(*myFilter)( myElement ))
          myElement = 0;
      }
    }
    return myElement;
  }

  const RANGE_SET& getRangeSet()
  {
    return ( myChunks[  myChunkIndex ].*myGetRangeSetFun )( myMinValue, myMaxValue );
  }
}; // struct _ChunkIterator


template< class ElemIterator >
boost::shared_ptr< ElemIterator >
SMDS_ElementFactory::GetIterator( SMDS_MeshElement::Filter* filter,
                                  size_t                    nbElemsToReturn )
{
  typedef _ChunkIterator< ElemIterator, TUsedRangeSet > TChuckIterator;
  return boost::make_shared< TChuckIterator >( myChunks,
                                               & SMDS_ElementChunk::GetUsedRangesMinMax,
                                               /*isUsed=*/true,
                                               filter,
                                               nbElemsToReturn );
}

template< class ElemIterator >
boost::shared_ptr< ElemIterator >
SMDS_ElementFactory::GetShapeIterator( int                     shapeID,
                                       size_t                  nbElemsToReturn,
                                       const SMDS_MeshElement* sm1stElem )
{
  int iChunk = sm1stElem ? (( sm1stElem->GetID() - 1 ) / ChunkSize()) : 0;
  typedef _ChunkIterator< ElemIterator, TSubIDRangeSet > TChuckIterator;
  return boost::make_shared< TChuckIterator >( myChunks,
                                               & SMDS_ElementChunk::GetSubIDRangesMinMax,
                                               /*shapeID=*/shapeID,
                                               new SMDS_MeshElement::NonNullFilter(),
                                               nbElemsToReturn,
                                               iChunk );
}

#endif
