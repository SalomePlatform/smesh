// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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
//  File   : SMDS_ElementFactory.cxx
//  Module : SMESH
//

#include "SMDS_ElementFactory.hxx"

#include "ObjectPool.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_SpacePosition.hxx"
#include "SMDS_VertexPosition.hxx"

namespace
{
  // nb of elements allocated by SMDS_ElementChunk at once
  const int theChunkSize = 1024;

  const int theDefaultShapeDim = 3;

  // classes allowing to modify parameters of SMDS_Position stored in SMDS_ElementFactory

  struct _EdgePosition : public SMDS_EdgePosition
  {
    TParam* myUParameter;

    _EdgePosition( TParam* aUParam ) : myUParameter( aUParam )
    { SMDS_EdgePosition::SetUParameter( aUParam[0]); }
    virtual void   SetUParameter(double aUparam)
    { *myUParameter = (TParam) aUparam; SMDS_EdgePosition::SetUParameter( aUparam ); }
  };

  struct _FacePosition : public SMDS_FacePosition
  {
    TParam* myParameter;

    _FacePosition(TParam* aParam) : myParameter( aParam )
    { SMDS_FacePosition::SetParameters( aParam[0], aParam[1] ); }
    virtual void SetUParameter(double aUparam)
    { myParameter[0] = (TParam) aUparam; SMDS_FacePosition::SetUParameter( aUparam ); }
    virtual void SetVParameter(double aVparam)
    { myParameter[1] = (TParam) aVparam; SMDS_FacePosition::SetVParameter( aVparam ); }
    virtual void SetParameters(double aU, double aV)
    { myParameter[0] = aU; myParameter[1] = aV; SMDS_FacePosition::SetParameters( aU, aV ); }
  };
}

//================================================================================
/*!
 * \brief Create a factory of cells or nodes in a given mesh
 */
//================================================================================

SMDS_ElementFactory::SMDS_ElementFactory( SMDS_Mesh* mesh, const bool isNodal )
  : myIsNodal( isNodal ), myMesh( mesh ), myNbUsedElements( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMDS_ElementFactory::~SMDS_ElementFactory()
{
  myChunksWithUnused.clear();
  myChunks.clear();
}

//================================================================================
/*!
 * \brief Return a number of elements in a chunk
 *  \return int - chunk size
 */
//================================================================================

int SMDS_ElementFactory::ChunkSize()
{
  return theChunkSize;
}

//================================================================================
/*!
 * \brief Return minimal ID of a non-used element
 *  \return smIdType - minimal element ID
 */
//================================================================================

smIdType SMDS_ElementFactory::GetFreeID()
{
  if ( myChunksWithUnused.empty() )
  {
    smIdType id0 = myChunks.size() * theChunkSize + 1;
    myChunks.push_back( new SMDS_ElementChunk( this, id0 ));
  }
  SMDS_ElementChunk * chunk = (*myChunksWithUnused.begin());
  return chunk->GetUnusedID();
}

//================================================================================
/*!
 * \brief Return maximal ID of an used element
 *  \return smIdType - element ID
 */
//================================================================================

smIdType SMDS_ElementFactory::GetMaxID()
{
  smIdType id = 0;
  TIndexRanges usedRanges;
  for ( smIdType i = myChunks.size() - 1; i >= 0; --i )
    if ( myChunks[i].GetUsedRanges().GetIndices( true, usedRanges ))
    {
      int index = usedRanges.back().second-1;
      id = myChunks[i].Get1stID() + index;
      break;
    }
  return id;
}

//================================================================================
/*!
 * \brief Return minimal ID of an used element
 *  \return smIdType - element ID
 */
//================================================================================

smIdType SMDS_ElementFactory::GetMinID()
{
  smIdType id = 0;
  TIndexRanges usedRanges;
  for ( size_t i = 0; i < myChunks.size(); ++i )
    if ( myChunks[i].GetUsedRanges().GetIndices( true, usedRanges ))
    {
      int index = usedRanges[0].first;
      id = myChunks[i].Get1stID() + index;
      break;
    }
  return id;
}

//================================================================================
/*!
 * \brief Return an element by ID. NULL if the element with the given ID is already used
 *  \param [in] id - element ID
 *  \return SMDS_MeshElement* - element pointer
 */
//================================================================================

SMDS_MeshElement* SMDS_ElementFactory::NewElement( const smIdType id )
{
  smIdType iChunk = ( id - 1 ) / theChunkSize;
  smIdType index  = ( id - 1 ) % theChunkSize;
  while ((smIdType) myChunks.size() <= iChunk )
  {
    smIdType id0 = myChunks.size() * theChunkSize + 1;
    myChunks.push_back( new SMDS_ElementChunk( this, id0 ));
  }
  SMDS_MeshElement* e = myChunks[iChunk].Element( FromSmIdType<int>(index) );
  if ( !e->IsNull() )
    return 0; // element with given ID already exists

  myChunks[iChunk].UseElement( FromSmIdType<int>(index) );
  ++myNbUsedElements;

  e->myHolder = & myChunks[iChunk];

  myMesh->setMyModified();

  return e;
}

//================================================================================
/*!
 * \brief Return an used element by ID. NULL if the element with the given ID is not yet used
 *  \param [in] id - element ID
 *  \return const SMDS_MeshElement* - element pointer
 */
//================================================================================

const SMDS_MeshElement* SMDS_ElementFactory::FindElement( const smIdType id ) const
{
  if ( id > 0 )
  {
    smIdType iChunk = ( id - 1 ) / theChunkSize;
    smIdType index  = ( id - 1 ) % theChunkSize;
    if ( iChunk < (smIdType) myChunks.size() )
    {
      const SMDS_MeshElement* e = myChunks[iChunk].Element( FromSmIdType<int>(index) );
      return e->IsNull() ? 0 : e;
    }
  }
  return 0;
}

//================================================================================
/*!
 * \brief Return an SMDS ID by a Vtk one
 *  \param [in] vtkID - Vtk ID
 *  \return smIdType - SMDS ID
 */
//================================================================================

smIdType SMDS_ElementFactory::FromVtkToSmds( vtkIdType vtkID )
{
  if ( vtkID >= 0 && vtkID < (vtkIdType)mySmdsIDs.size() )
    return mySmdsIDs[vtkID] + 1;
  return vtkID + 1;
}

//================================================================================
/*!
 * \brief Clear marked flag of all elements
 */
//================================================================================

void SMDS_ElementFactory::SetAllNotMarked()
{
  for ( SMDS_ElementChunk& chunk : myChunks )
    chunk.SetAllNotMarked();
}

//================================================================================
/*!
 * \brief Mark the element as non-used
 *  \param [in] e - element
 */
//================================================================================

void SMDS_ElementFactory::Free( const SMDS_MeshElement* e )
{
  if ( e != FindElement( e->GetID() ))
    SALOME_Exception("SMDS_ElementFactory::Free(): element of other mesh");

  if ( !myVtkIDs.empty() )
  {
    size_t    id = e->GetID() - 1;
    size_t vtkID = e->GetVtkID();
    if ( id < myVtkIDs.size() )
      myVtkIDs[ id ] = -1;
    if ( vtkID < mySmdsIDs.size() )
      mySmdsIDs[ vtkID ] = -1;
  }
  e->myHolder->Free( e );
  const_cast< SMDS_MeshElement*>( e )->myHolder = 0;
  --myNbUsedElements;

  myMesh->setMyModified();
}

//================================================================================
/*!
 * \brief De-allocate all elements
 */
//================================================================================

void SMDS_ElementFactory::Clear()
{
  myChunksWithUnused.clear();
  clearVector( myChunks );
  clearVector( myVtkIDs );
  clearVector( mySmdsIDs );
  myNbUsedElements = 0;
}

//================================================================================
/*!
 * \brief Remove unused elements located not at the end of the last chunk.
 *        Minimize allocated memory
 *  \param [out] theVtkIDsNewToOld - theVtkIDsNewToOld[ new VtkID ] = old VtkID
 */
//================================================================================

void SMDS_ElementFactory::Compact( std::vector<smIdType>& theVtkIDsNewToOld )
{
  smIdType  newNbCells = NbUsedElements();
  smIdType   maxCellID = GetMaxID();
  smIdType newNbChunks = newNbCells / theChunkSize + bool ( newNbCells % theChunkSize );

  theVtkIDsNewToOld.resize( newNbCells );

  if ( newNbCells == 0 ) // empty mesh
  {
    clearVector( myChunks );
  }
  else if ( newNbCells == maxCellID ) // no holes
  {
    smIdType newID, minLastID = std::min( myVtkIDs.size(), theVtkIDsNewToOld.size() );
    for ( newID = 0; newID < minLastID; ++newID )
      theVtkIDsNewToOld[ newID ] = myVtkIDs[ newID ];
    for ( ; newID < newNbCells; ++newID )
      theVtkIDsNewToOld[ newID ] = newID;
  }
  else // there are holes in SMDS IDs
  {
    smIdType newVtkID = 0; // same as new smds ID (-1)
    for ( smIdType oldID = 1; oldID <= maxCellID; ++oldID ) // smds IDs
    {
      const SMDS_MeshElement* oldElem = FindElement( oldID );
      if ( !oldElem ) continue;
      theVtkIDsNewToOld[ newVtkID++ ] = oldElem->GetVtkID(); // now newVtkID == new smds ID
      if ( oldID != newVtkID )
      {
        const SMDS_MeshElement* newElem = FindElement( newVtkID );
        if ( !newElem )
          newElem = NewElement( newVtkID );
        if ( int shapeID = oldElem->GetShapeID() )
          const_cast< SMDS_MeshElement* >( newElem )->setShapeID( shapeID );
        if ( oldID > newNbCells )
          Free( oldElem );
      }
    }
  }
  myChunks.resize( newNbChunks );

  myChunksWithUnused.clear();
  if ( !myChunks.empty() && myChunks.back().GetUsedRanges().Size() > 1 )
    myChunksWithUnused.insert( & myChunks.back() );

  for ( size_t i = 0; i < myChunks.size(); ++i )
    myChunks[i].Compact();

  clearVector( myVtkIDs );
  clearVector( mySmdsIDs );
}


//================================================================================
/*!
 * \brief Return true if Compact() will change IDs of elements
 */
//================================================================================

bool SMDS_ElementFactory::CompactChangePointers()
{
  // there can be VTK_EMPTY_CELL's in the VTK grid as well as "holes" in SMDS numeration
  return ( NbUsedElements() != GetMaxID() );
}

//================================================================================
/*!
 * \brief Create a factory of nodes in a given mesh
 */
//================================================================================

SMDS_NodeFactory::SMDS_NodeFactory( SMDS_Mesh* mesh )
  : SMDS_ElementFactory( mesh, /*isNodal=*/true )
{
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMDS_NodeFactory::~SMDS_NodeFactory()
{
  Clear();
}

//================================================================================
/*!
 * \brief Remove unused nodes located not at the end of the last chunk.
 *        Minimize allocated memory
 *  \param [out] theVtkIDsOldToNew - vector storing change of vtk IDs
 */
//================================================================================

void SMDS_NodeFactory::Compact( std::vector<smIdType>& theVtkIDsOldToNew )
{
  // IDs of VTK nodes always correspond to SMDS IDs but there can be "holes"
  // in the chunks. So we remove holes and report relocation in theVtkIDsOldToNew:
  // theVtkIDsOldToNew[ old VtkID ] = new VtkID

  smIdType  oldNbNodes = myMesh->GetGrid()->GetNumberOfPoints();
  smIdType  newNbNodes = NbUsedElements();
  smIdType newNbChunks = newNbNodes / theChunkSize + bool ( newNbNodes % theChunkSize );
  smIdType   maxNodeID = GetMaxID();

  theVtkIDsOldToNew.resize( oldNbNodes, -1 );

  if ( newNbNodes == 0 ) // empty mesh
  {
    clearVector( myChunks );
  }
  else if ( maxNodeID > newNbNodes ) // there are holes
  {
    size_t newID = 0;
    for ( size_t oldID = 0; oldID < theVtkIDsOldToNew.size(); ++oldID )
    {
      const SMDS_MeshElement* oldNode = FindNode( oldID+1 );
      if ( !oldNode )
        continue;
      theVtkIDsOldToNew[ oldID ] = newID;
      if ( oldID != newID )
      {
        const SMDS_MeshElement* newNode = FindElement( newID+1 );
        if ( !newNode )
          newNode = NewElement( newID+1 );
        int     shapeID = oldNode->GetShapeID();
        int    shapeDim = GetShapeDim( shapeID );
        smIdType iChunk = newID / theChunkSize;
        myChunks[ iChunk ].SetShapeID( newNode, shapeID );
        if ( shapeDim == 2 || shapeDim == 1 )
        {
          smIdType iChunkOld = oldID / theChunkSize;
          TParam* oldPos = myChunks[ iChunkOld ].GetPositionPtr( oldNode );
          TParam* newPos = myChunks[ iChunk    ].GetPositionPtr( newNode, /*allocate=*/true );
          if ( oldPos )
          {
            newPos[0] = oldPos[0];
            newPos[1] = oldPos[1];
          }
        }
        if ( oldNode->GetID() > newNbNodes )
          Free( oldNode );
      }
      ++newID;
    }
  }
  else // no holes
  {
    for ( smIdType i = 0; i < newNbNodes; ++i )
      theVtkIDsOldToNew[ i ] = i;
  }
  myChunks.resize( newNbChunks );

  myChunksWithUnused.clear();
  if ( !myChunks.empty() && myChunks.back().GetUsedRanges().Size() > 1 )
    myChunksWithUnused.insert( & myChunks.back() );

  for ( size_t i = 0; i < myChunks.size(); ++i )
    myChunks[i].Compact();

  ASSERT( newNbNodes == GetMaxID() );
  ASSERT( newNbNodes == NbUsedElements() );
}

//================================================================================
/*!
 * \brief Return true if Compact() will change IDs of elements
 */
//================================================================================

bool SMDS_NodeFactory::CompactChangePointers()
{
  // IDs of VTK nodes always correspond to SMDS IDs but there can be "holes" in SMDS numeration
  return ( NbUsedElements() != GetMaxID() );
}

//================================================================================
/*!
 * \brief De-allocate all nodes
 */
//================================================================================

void SMDS_NodeFactory::Clear()
{
  SMDS_ElementFactory::Clear();
}

//================================================================================
/*!
 * \brief Set a total number of sub-shapes in the main shape
 */
//================================================================================

void SMDS_NodeFactory::SetNbShapes( size_t nbShapes )
{
  clearVector( myShapeDim );
  myShapeDim.resize( nbShapes+1, theDefaultShapeDim );
}

//================================================================================
/*!
 * \brief Return a dimension of a shape
 */
//================================================================================

int SMDS_NodeFactory::GetShapeDim( int shapeID ) const
{
  return shapeID < (int)myShapeDim.size() ? myShapeDim[ shapeID ] : theDefaultShapeDim;
}

//================================================================================
/*!
 * \brief Set a dimension of a shape
 */
//================================================================================

void SMDS_NodeFactory::SetShapeDim( int shapeID, int dim )
{
  if ( shapeID >= (int)myShapeDim.size() )
    myShapeDim.resize( shapeID + 10, theDefaultShapeDim );
  myShapeDim[ shapeID ] = dim;
}

//================================================================================
/*!
 * \brief SMDS_ElementChunk constructor
 *  \param [in] factory - the factory
 *  \param [in] id0 - ID of the 1st element
 */
//================================================================================

SMDS_ElementChunk::SMDS_ElementChunk( SMDS_ElementFactory* factory, smIdType id0 ):
  myFactory( factory ),
  my1stID( id0 )//,
  //mySubIDSet( 0 )
  // myMinSubID( std::numeric_limits<int>::max() ),
  // myMaxSubID( 0 )
{
  if ( !myFactory )
    return;
  if ( myFactory->myIsNodal )
    myElements = new SMDS_MeshNode[ theChunkSize ];
  else
    myElements = new SMDS_MeshCell[ theChunkSize ];

  myUsedRanges.mySet.reserve(2);
  mySubIDRanges.mySet.insert( _ShapeIDRange( 0, 0 ));
  myUsedRanges.mySet.insert( _UsedRange( 0, false ));
  myFactory->myChunksWithUnused.insert( this );
}

//================================================================================
/*!
 * \brief SMDS_ElementChunk destructor
 */
//================================================================================

SMDS_ElementChunk::~SMDS_ElementChunk()
{
  delete [] myElements;
  myFactory->myChunksWithUnused.erase( this );
}

//================================================================================
/*!
 * \brief Mark an element as used
 */
//================================================================================

void SMDS_ElementChunk::UseElement( const int index )
{
  myUsedRanges.SetValue( index, true );
  if ( myUsedRanges.Size() == 1 ) // all elements used
    myFactory->myChunksWithUnused.erase( this );
}

//================================================================================
/*!
 * \brief Return ID of the first non-used element
 */
//================================================================================

smIdType SMDS_ElementChunk::GetUnusedID() const
{
  TUsedRangeSet::set_iterator r = myUsedRanges.mySet.begin();
  for ( ; r != myUsedRanges.mySet.end(); ++r )
    if ( !IsUsed( *r ))
      break;

  return my1stID + r->my1st;
}

//================================================================================
/*!
 * \brief Mark an element as non-used
 */
//================================================================================

void SMDS_ElementChunk::Free( const SMDS_MeshElement* e )
{
  bool hasHoles = ( myUsedRanges.Size() > 1 );
  myUsedRanges.SetValue( Index( e ), false );
  SetShapeID( e, 0 ); // sub-mesh must do it?
  SetIsMarked( e, false );
  if ( !hasHoles )
    myFactory->myChunksWithUnused.insert( this );

  if ( myUsedRanges.Size() == 1 )
  {
    clearVector( myMarkedSet );
    clearVector( myPositions );
  }
}

//================================================================================
/*!
 * \brief Return an SMDS ID of an element
 */
//================================================================================

smIdType SMDS_ElementChunk::GetID( const SMDS_MeshElement* e ) const
{
  return my1stID + Index( e );
}

//================================================================================
/*!
 * \brief Set a Vtk ID of an element
 */
//================================================================================

void SMDS_ElementChunk::SetVTKID( const SMDS_MeshElement* e, const vtkIdType vtkID )
{
  if ( e->GetID() - 1 != vtkID )
  {
    if ((smIdType) myFactory->myVtkIDs.size() <= e->GetID() - 1 )
    {
      vtkIdType i = (vtkIdType) myFactory->myVtkIDs.size();
      myFactory->myVtkIDs.resize( e->GetID() + 100 );
      vtkIdType newSize = (vtkIdType) myFactory->myVtkIDs.size();
      for ( ; i < newSize; ++i )
        myFactory->myVtkIDs[i] = i;
    }
    myFactory->myVtkIDs[ e->GetID() - 1 ] = vtkID;

    if ((vtkIdType) myFactory->mySmdsIDs.size() <= vtkID )
    {
      size_t i = myFactory->mySmdsIDs.size();
      myFactory->mySmdsIDs.resize( vtkID + 100 );
      for ( ; i < myFactory->mySmdsIDs.size(); ++i )
        myFactory->mySmdsIDs[i] = i;
    }
    myFactory->mySmdsIDs[ vtkID ] = e->GetID() - 1;
  }
  else
  {
    if ((size_t) e->GetID() <= myFactory->myVtkIDs.size() )
      myFactory->myVtkIDs[ e->GetID() - 1 ] = vtkID;
    if ((size_t) vtkID < myFactory->mySmdsIDs.size() )
      myFactory->mySmdsIDs[ vtkID ] = e->GetID() - 1;
  }
}

//================================================================================
/*!
 * \brief Return a Vtk ID of an element
 */
//================================================================================

vtkIdType SMDS_ElementChunk::GetVtkID( const SMDS_MeshElement* e ) const
{
  vtkIdType dfltVtkID = FromSmIdType<vtkIdType>(e->GetID() - 1);
  return ( dfltVtkID < (vtkIdType)myFactory->myVtkIDs.size() ) ? myFactory->myVtkIDs[ dfltVtkID ] : dfltVtkID;
}

//================================================================================
/*!
 * \brief Return ID of a shape an element is assigned to
 */
//================================================================================

int SMDS_ElementChunk::GetShapeID( const SMDS_MeshElement* e ) const
{
  return mySubIDRanges.GetValue( Index( e ));
}

//================================================================================
/*!
 * \brief Set ID of a shape an element is assigned to
 */
//================================================================================

void SMDS_ElementChunk::SetShapeID( const SMDS_MeshElement* e, int shapeID ) const
{
  //const size_t nbRanges = mySubIDRanges.Size();

  SMDS_ElementChunk* me = const_cast<SMDS_ElementChunk*>( this );
  int oldShapeID = me->mySubIDRanges.SetValue( Index( e ), shapeID );
  if ( oldShapeID == shapeID ) return;

  if ( const SMDS_MeshNode* n = dynamic_cast< const SMDS_MeshNode* >( e ))
    if ( TParam* uv = me->GetPositionPtr( n ))
    {
      uv[0] = 0.;
      uv[1] = 0.;
    }
  // update min/max
  // if (( nbRanges > mySubIDRanges.Size() ) &&
  //     ( myMinSubID == oldShapeID || myMaxSubID == oldShapeID ))
  // {
  //   me->myMinSubID = ( std::numeric_limits<int>::max() );
  //   me->myMaxSubID = 0;
  //   TSubIDRangeSet::set_iterator it;
  //   for ( it = mySubIDRanges.mySet.begin(); it < mySubIDRanges.mySet.end(); ++it )
  //     if ( it->myValue > 0 )
  //     {
  //       me->myMinSubID = std::min( myMinSubID, it->myValue );
  //       me->myMaxSubID = std::max( myMaxSubID, it->myValue );
  //     }
  // }
  // else if ( shapeID > 0 )
  // {
  //   me->myMinSubID = std::min( myMinSubID, shapeID );
  //   me->myMaxSubID = std::max( myMaxSubID, shapeID );
  // }
}

//================================================================================
/*!
 * \brief Set isMarked flag of an element
 */
//================================================================================

bool SMDS_ElementChunk::IsMarked( const SMDS_MeshElement* e ) const
{
  return ( !myMarkedSet.empty() && myMarkedSet[ Index( e )]);
}

//================================================================================
/*!
 * \brief Return isMarked flag of an element
 */
//================================================================================

void SMDS_ElementChunk::SetIsMarked( const SMDS_MeshElement* e, bool is )
{
  if ( !is && myMarkedSet.empty() ) return;
  if ( myMarkedSet.empty() ) myMarkedSet.resize( theChunkSize, false );
  myMarkedSet[ Index( e )] = is;
}

//================================================================================
/*!
 * \brief Clear marked flag of all elements
 */
//================================================================================

void SMDS_ElementChunk::SetAllNotMarked()
{
  clearVector( myMarkedSet );
}

//================================================================================
/*!
 * \brief Return SMDS_Position of a node on a shape
 */
//================================================================================

SMDS_PositionPtr SMDS_ElementChunk::GetPosition( const SMDS_MeshNode* n ) const
{
  int  shapeID = GetShapeID( n );
  int shapeDim = static_cast< SMDS_NodeFactory* >( myFactory )->GetShapeDim( shapeID );

  SMDS_ElementChunk* me = const_cast< SMDS_ElementChunk* >( this );

  switch ( shapeDim ) {
  case 2:
  {
    return SMDS_PositionPtr( new _FacePosition( me->GetPositionPtr( n )));
  }
  case 1:
  {
    return SMDS_PositionPtr( new _EdgePosition( me->GetPositionPtr( n )));
  }
  case 0:
    return SMDS_VertexPosition::StaticPosition();
  }

  return SMDS_SpacePosition::originSpacePosition();
}

//================================================================================
/*!
 * \brief Set SMDS_Position of a node on a shape
 */
//================================================================================

void SMDS_ElementChunk::SetPosition( const SMDS_MeshNode* n, const SMDS_PositionPtr& pos, int shapeID )
{
  int shapeDim = pos ? pos->GetDim() : theDefaultShapeDim;
  if ( shapeID < 1 )
  {
    if ( shapeDim == theDefaultShapeDim )
      return;
    shapeID = GetShapeID( n );
    if ( shapeID < 1 )
      throw SALOME_Exception("SetPosition() No shape ID provided");
  }

  static_cast< SMDS_NodeFactory* >( myFactory )->SetShapeDim( shapeID, shapeDim );

  switch ( shapeDim ) {
  case 2:
  {
    TParam* uv = GetPositionPtr( n, /*allocate=*/true );
    uv[0] = (TParam) pos->GetParameters()[0];
    uv[1] = (TParam) pos->GetParameters()[1];
    break;
  }
  case 1:
  {
    GetPositionPtr( n, /*allocate=*/true )[0] = (TParam) pos->GetParameters()[0];
    break;
  }
  }
}

//================================================================================
/*!
 * \brief Return pointer to on-shape-parameters of a node
 */
//================================================================================

TParam* SMDS_ElementChunk::GetPositionPtr( const SMDS_MeshElement* n, bool allocate )
{
  if ( myPositions.empty() && !allocate )
    return 0;

  myPositions.resize( theChunkSize * 2 );
  return myPositions.data() + 2 * Index( n );
}

//================================================================================
/*!
 * \brief Minimize allocated memory
 */
//================================================================================

void SMDS_ElementChunk::Compact()
{
  mySubIDRanges.mySet.shrink_to_fit();
  if ( myUsedRanges.mySet.capacity() > 2 )
    myUsedRanges.mySet.shrink_to_fit();

  clearVector( myMarkedSet );

  if ( !myPositions.empty() )
  {
    // look for the last position that must be kept
    TSubIDRangeSet::set_t::reverse_iterator it;
    for ( it = mySubIDRanges.mySet.rbegin(); it != mySubIDRanges.mySet.rend(); ++it )
    {
      int shapeDim = static_cast< SMDS_NodeFactory* >( myFactory )->GetShapeDim( it->myValue );
      if ( shapeDim == 1 || shapeDim == 2 )
        break;
    }
    if ( it == mySubIDRanges.mySet.rend() )
    {
      clearVector( myPositions );
    }
    else if ( it != mySubIDRanges.mySet.rbegin() )
    {
      int nbNodes = (it-1)->my1st;
      myPositions.resize( nbNodes * 2 );
      std::vector<TParam> newPos( myPositions.begin(), myPositions.end() );
      myPositions.swap( newPos );
    }
  }
}

//================================================================================
/*!
 * \brief Print some data for debug purposes
 */
//================================================================================

void SMDS_ElementChunk::Dump() const
{
  std::cout << "1stID: " << my1stID << std::endl;

  //std::cout << "SubID min/max: " << myMinSubID << ", " << myMaxSubID << std::endl;
  std::cout << "SubIDRanges: " << mySubIDRanges.Size() << " ";
  {
    TSubIDRangeSet::set_iterator i = mySubIDRanges.mySet.begin();
    for ( int cnt = 0; i != mySubIDRanges.mySet.end(); ++i, ++cnt )
      std::cout << "|" << cnt << " - (" << i->my1st << ", " << i->myValue << ") ";
    std::cout << std::endl;
  }
  {
    std::cout << "UsedRanges: " << myUsedRanges.Size() << " ";
    TUsedRangeSet::set_iterator i = myUsedRanges.mySet.begin();
    for ( int cnt = 0; i != myUsedRanges.mySet.end(); ++i, ++cnt )
      std::cout << cnt << " - (" << i->my1st << ", " << i->myValue << ") ";
    std::cout << std::endl;
  }
}

//================================================================================
/*!
 * \brief Compare SMDS_ElementChunk's
 */
//================================================================================

bool _ChunkCompare::operator () (const SMDS_ElementChunk* e1, const SMDS_ElementChunk* e2) const
{
  return e1->Get1stID() < e2->Get1stID();
}

