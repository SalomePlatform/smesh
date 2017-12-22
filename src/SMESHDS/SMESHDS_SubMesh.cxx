// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESHDS : management of mesh data and SMESH document
//  File   : SMESH_SubMesh.cxx
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 
//
#include "SMESHDS_SubMesh.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_ElementFactory.hxx"

#include <utilities.h>

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHDS_SubMesh::SMESHDS_SubMesh(SMESHDS_Mesh *parent, int index)
{
  myParent = parent;
  myIndex = index;
  myNbElements = 0;
  myNbNodes = 0;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

SMESHDS_SubMesh::~SMESHDS_SubMesh()
{
}

//=======================================================================
//function : AddElement
//purpose  :
//=======================================================================

void SMESHDS_SubMesh::AddElement(const SMDS_MeshElement * elem)
{
  if (!IsComplexSubmesh())
  {
    if ( elem->GetType() == SMDSAbs_Node )
    {
      AddNode( static_cast< const SMDS_MeshNode* >( elem ));
      return;
    }
    int oldShapeId = elem->GetShapeID();
    if ( oldShapeId > 0 )
    {
      if (oldShapeId != myIndex)
      {
        throw SALOME_Exception
          (LOCALIZED("add element in subshape already belonging to a subshape"));
      }
    }

    elem->setShapeID( myIndex );
    myNbElements++;
  }
}

//=======================================================================
//function : RemoveElement
//purpose  :
//=======================================================================

bool SMESHDS_SubMesh::RemoveElement(const SMDS_MeshElement * elem )
{
  if ( !elem || elem->IsNull() || elem->getshapeId() != myIndex )
  {
    return false;
  }
  if ( !IsComplexSubmesh() )
  {
    elem->setShapeID( 0 );
    myNbElements--;
    return true;
  }
  return false;
}

//=======================================================================
//function : AddNode
//purpose  :
//=======================================================================

void SMESHDS_SubMesh::AddNode(const SMDS_MeshNode * N)
{
  if ( !IsComplexSubmesh() )
  {
    const int shapeId = N->getshapeId();
    if ( shapeId > 0 )
    {
      if ( shapeId != myIndex )
        throw SALOME_Exception
          (LOCALIZED("a node being in sub-mesh is added to another sub-mesh"));
      return; // already in
    }
    N->setShapeID( myIndex );
    myNbNodes++;
  }
}

//=======================================================================
//function : RemoveNode
//purpose  :
//=======================================================================

bool SMESHDS_SubMesh::RemoveNode(const SMDS_MeshNode * N)
{
  if ( !N || N->getshapeId() != myIndex )
  {
    return false;
  }
  if ( !IsComplexSubmesh() )
  {
    N->setShapeID( 0 );
    myNbNodes--;
    return true;
  }
  return false;
}

//=======================================================================
//function : NbElements
//purpose  :
//=======================================================================

int SMESHDS_SubMesh::NbElements() const
{
  if ( !IsComplexSubmesh() )
    return myNbElements;

  int nbElems = 0;
  TSubMeshSet::const_iterator it = mySubMeshes.begin();
  for ( ; it != mySubMeshes.end(); it++ )
    nbElems += (*it)->NbElements();

  return nbElems;
}

//=======================================================================
//function : NbNodes
//purpose  :
//=======================================================================

int SMESHDS_SubMesh::NbNodes() const
{
  if ( !IsComplexSubmesh() )
    return myNbNodes;

  int nbElems = 0;
  TSubMeshSet::const_iterator it = mySubMeshes.begin();
  for ( ; it != mySubMeshes.end(); it++ )
    nbElems += (*it)->NbNodes();

  return nbElems;
}

/*!
 * Template class used for iteration on vector of elements which can resize
 * during iteration. The iterator returns only elements present upon its creation.
 */
template <class ELEM, typename TSET> class MySetIterator : public SMDS_Iterator<ELEM>
{
protected:
  int _iCur, _iEnd, _iDelta;
  const TSET& _table;
public:
  MySetIterator(const TSET& table, bool reverse): _table( table )
  {
    if ( reverse )
    {
      _iCur = _table.size()-1;
      _iEnd = -1;
      _iDelta = -1;
    }
    else
    {
      _iCur = 0;
      _iEnd = _table.size();
      _iDelta = 1;
    }
    if ( more() && !_table[ _iCur ])
      next();
  }

  virtual bool more()
  {
    return ( _iEnd - _iCur ) * _iDelta > 0;
  }

  virtual ELEM next()
  {
    ELEM e = more() ? _table[ _iCur ] : 0;
    _iCur += _iDelta;
    while ( more() && !_table[ _iCur ])
      _iCur += _iDelta;
    return e;
  }
};

// =====================
// class MyIterator
// =====================

template<typename VALUE> class MyIterator : public SMDS_Iterator<VALUE>
{
public:
  MyIterator (const TSubMeshSet& theSubMeshes)
    : myMore(false), mySubIt( theSubMeshes.begin() ), mySubEnd( theSubMeshes.end() )
  {}
  bool more()
  {
    while (( !myElemIt.get() || !myElemIt->more() ) && mySubIt != mySubEnd)
    {
      myElemIt = getElements(*mySubIt);
      mySubIt++;
    }
    myMore = myElemIt.get() && myElemIt->more();
    return myMore;
  }
  VALUE next()
  {
    VALUE elem = 0;
    if ( myMore )
      elem = myElemIt->next();
    return elem;
  }
protected:
  virtual boost::shared_ptr< SMDS_Iterator<VALUE> >
  getElements(const SMESHDS_SubMesh*) const = 0;

private:
  bool                                      myMore;
  TSubMeshSet::const_iterator               mySubIt, mySubEnd;
  boost::shared_ptr< SMDS_Iterator<VALUE> > myElemIt;
};

// =====================
// class MyElemIterator
// =====================

class MyElemIterator: public MyIterator<const SMDS_MeshElement*>
{
public:
  MyElemIterator (const TSubMeshSet& theSubMeshes)
    :MyIterator<const SMDS_MeshElement*>( theSubMeshes ) {}
  SMDS_ElemIteratorPtr getElements(const SMESHDS_SubMesh* theSubMesh) const
  { return theSubMesh->GetElements(); }
};

// =====================
// class MyNodeIterator
// =====================

class MyNodeIterator: public MyIterator<const SMDS_MeshNode*>
{
public:
  MyNodeIterator (const TSubMeshSet& theSubMeshes)
    :MyIterator<const SMDS_MeshNode*>( theSubMeshes ) {}
  SMDS_NodeIteratorPtr getElements(const SMESHDS_SubMesh* theSubMesh) const
  { return theSubMesh->GetNodes(); }
};

//=======================================================================
//function : GetElements
//purpose  :
//=======================================================================

SMDS_ElemIteratorPtr SMESHDS_SubMesh::GetElements() const
{
  if ( IsComplexSubmesh() )
    return SMDS_ElemIteratorPtr( new MyElemIterator( mySubMeshes ));

  return myParent->shapeElementsIterator( myIndex, myNbElements );
}

//=======================================================================
//function : GetNodes
//purpose  :
//=======================================================================

SMDS_NodeIteratorPtr SMESHDS_SubMesh::GetNodes() const
{
  if ( IsComplexSubmesh() )
    return SMDS_NodeIteratorPtr( new MyNodeIterator( mySubMeshes ));

  return myParent->shapeNodesIterator( myIndex, myNbNodes );
}

//=======================================================================
//function : Contains
//purpose  : check if elem or node is in
//=======================================================================

bool SMESHDS_SubMesh::Contains(const SMDS_MeshElement * ME) const
{
  if ( !ME || ME->IsNull() )
    return false;

  if ( IsComplexSubmesh() )
  {
    TSubMeshSet::const_iterator aSubIt = mySubMeshes.begin();
    for (; aSubIt != mySubMeshes.end(); aSubIt++)
      if ((*aSubIt)->Contains(ME))
        return true;
    return false;
  }
  return ME->getshapeId() == myIndex;
}

//=======================================================================
//function : IsQuadratic
//purpose  : Return true if my 1st element is quadratic
//=======================================================================

bool SMESHDS_SubMesh::IsQuadratic() const
{
  if ( IsComplexSubmesh() )
  {
    TSubMeshSet::const_iterator aSubIt = mySubMeshes.begin();
    for (; aSubIt != mySubMeshes.end(); aSubIt++)
      if ((*aSubIt)->IsQuadratic())
        return true;
    return false;
  }

  if ( myNbElements == 0 )
    return false;

  SMDS_ElemIteratorPtr it = GetElements();
  return it->more() && it->next()->IsQuadratic();
}

//=======================================================================
//function : AddSubMesh
//purpose  :
//=======================================================================

void SMESHDS_SubMesh::AddSubMesh( const SMESHDS_SubMesh* theSubMesh )
{
  ASSERT( theSubMesh );
  mySubMeshes.insert( theSubMesh );
}

//=======================================================================
//function : RemoveSubMesh
//purpose  : 
//=======================================================================

bool SMESHDS_SubMesh::RemoveSubMesh( const SMESHDS_SubMesh* theSubMesh )
{
  return mySubMeshes.erase( theSubMesh );
}

//=======================================================================
//function : RemoveAllSubmeshes
//purpose  : 
//=======================================================================

void SMESHDS_SubMesh::RemoveAllSubmeshes()
{
  mySubMeshes.clear();
}

//=======================================================================
//function : ContainsSubMesh
//purpose  :
//=======================================================================

bool SMESHDS_SubMesh::ContainsSubMesh( const SMESHDS_SubMesh* theSubMesh ) const
{
  return mySubMeshes.find( theSubMesh ) != mySubMeshes.end();
}

//=======================================================================
//function : GetSubMeshIterator
//purpose  :
//=======================================================================

SMESHDS_SubMeshIteratorPtr SMESHDS_SubMesh::GetSubMeshIterator() const
{
  typedef SMDS_SetIterator< const SMESHDS_SubMesh*, TSubMeshSet::const_iterator > TIterator;
  return boost::make_shared< TIterator >( mySubMeshes.begin(), mySubMeshes.end());
}

//=======================================================================
//function : Clear
//purpose  : remove the contents
//=======================================================================

void SMESHDS_SubMesh::Clear()
{
  if ( myParent && myParent->NbNodes() > 0 )
  {
    if ( myNbElements > 0 )
      for ( SMDS_ElemIteratorPtr it = GetElements(); it->more(); )
      {
        const SMDS_MeshElement * elem = it->next();
        elem->setShapeID( 0 );
      }
    if ( myNbNodes > 0 )
      for ( SMDS_NodeIteratorPtr it = GetNodes(); it->more(); )
      {
        const SMDS_MeshNode * elem = it->next();
        elem->setShapeID( 0 );
      }
  }

  myNbElements = 0;
  myNbNodes = 0;
  if ( NbSubMeshes() > 0 )
  {
    SMESHDS_SubMeshIteratorPtr sub = GetSubMeshIterator();
    while ( sub->more() ) {
      if ( SMESHDS_SubMesh* sm = (SMESHDS_SubMesh*) sub->next())
        sm->Clear();
    }
  }
}
