//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_subMesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_subMesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"
#include "OpUtil.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <BRepTools.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::SMESH_subMesh_i()
     : SALOME::GenericObj_i( PortableServer::POA::_nil() )
{
  MESSAGE("SMESH_subMesh_i::SMESH_subMesh_i default, not for use");
    ASSERT(0);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::SMESH_subMesh_i( PortableServer::POA_ptr thePOA,
				  SMESH_Gen_i*            gen_i,
 				  SMESH_Mesh_i*           mesh_i,
				  int                     localId )
     : SALOME::GenericObj_i( thePOA )
{
  MESSAGE("SMESH_subMesh_i::SMESH_subMesh_i");
  _gen_i = gen_i;
  _mesh_i = mesh_i;
  _localId = localId;
  thePOA->activate_object( this );
  // ****
}
//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::~SMESH_subMesh_i()
{
  MESSAGE("SMESH_subMesh_i::~SMESH_subMesh_i");
  // ****
}

//=======================================================================
//function : getSubMeshes
//purpose  : for a submesh on shape to which elements are not bound directly,
//           return submeshes containing elements
//=======================================================================

typedef list<SMESHDS_SubMesh*> TListOfSubMeshes;

bool getSubMeshes(::SMESH_subMesh*  theSubMesh,
                  TListOfSubMeshes& theSubMeshList)
{
  int size = theSubMeshList.size();

  SMESH_Mesh*      aMesh      = theSubMesh->GetFather();
  SMESHDS_Mesh*    aMeshDS    = aMesh->GetMeshDS();
  SMESHDS_SubMesh* aSubMeshDS = theSubMesh->GetSubMeshDS();

  // nodes can be bound to either vertex, edge, face or solid_or_shell
  TopoDS_Shape aShape = theSubMesh->GetSubShape();
  switch ( aShape.ShapeType() )
  {
  case TopAbs_SOLID: {
    // add submesh of solid itself
    aSubMeshDS = aMeshDS->MeshElements( aShape );
    if ( aSubMeshDS )
      theSubMeshList.push_back( aSubMeshDS );
    // and of the first shell
    TopExp_Explorer exp( aShape, TopAbs_SHELL );
    if ( exp.More() ) {
      aSubMeshDS = aMeshDS->MeshElements( exp.Current() );
      if ( aSubMeshDS )
        theSubMeshList.push_back( aSubMeshDS );
    }
    break;
  }
  case TopAbs_WIRE:
  case TopAbs_COMPOUND:
  case TopAbs_COMPSOLID: {
    // call getSubMeshes() for sub-shapes
    list<TopoDS_Shape> shapeList;
    shapeList.push_back( aShape );
    list<TopoDS_Shape>::iterator sh = shapeList.begin();
    for ( ; sh != shapeList.end(); ++sh ) {
      for ( TopoDS_Iterator it( *sh ); it.More(); it.Next() ) {
        ::SMESH_subMesh* aSubMesh = aMesh->GetSubMeshContaining( it.Value() );
        if ( aSubMesh )
          getSubMeshes( aSubMesh, theSubMeshList );
        else
          // no submesh for a compound inside compound
          shapeList.push_back( it.Value() );
      }
    }
    // return only unique submeshes
    set<SMESHDS_SubMesh*> smSet;
    TListOfSubMeshes::iterator sm = theSubMeshList.begin();
    while ( sm != theSubMeshList.end() ) {
      if ( !smSet.insert( *sm ).second )
        sm = theSubMeshList.erase( sm );
      else
        ++sm;
    }
    break;
  }
  }
  return size < theSubMeshList.size();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_subMesh_i::GetNumberOfElements()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetNumberOfElements");
  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return 0;

  ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];
  SMESHDS_SubMesh* aSubMeshDS = aSubMesh->GetSubMeshDS();

  int nbElems = aSubMeshDS ? aSubMeshDS->NbElements() : 0;

  // volumes are bound to shell
  TListOfSubMeshes smList;
  if ( nbElems == 0 && getSubMeshes( aSubMesh, smList ))
  {
    TListOfSubMeshes::iterator sm = smList.begin();
    for ( ; sm != smList.end(); ++sm )
      nbElems += (*sm)->NbElements();
  }
  return nbElems;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_subMesh_i::GetNumberOfNodes(CORBA::Boolean all)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetNumberOfNodes");
  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return 0;

  ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];
  SMESHDS_SubMesh* aSubMeshDS = aSubMesh->GetSubMeshDS();

  set<int> nodeIds;

  // nodes are bound to shell instead of solid
  TListOfSubMeshes smList;
  if ( all && getSubMeshes( aSubMesh, smList ))
  {
    TListOfSubMeshes::iterator sm = smList.begin();
    for ( ; sm != smList.end(); ++sm )
    {
      SMDS_ElemIteratorPtr eIt = (*sm)->GetElements();
      while ( eIt->more() ) {
        const SMDS_MeshElement* anElem = eIt->next();
        SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
        while ( nIt->more() )
          nodeIds.insert( nIt->next()->GetID() );
      }
    }
    return nodeIds.size();
  }

  if ( aSubMeshDS == NULL )
    return 0;

  if ( all ) { // all nodes of submesh elements
    SMDS_ElemIteratorPtr eIt = aSubMeshDS->GetElements();
    while ( eIt->more() ) {
      const SMDS_MeshElement* anElem = eIt->next();
      SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
      while ( nIt->more() )
        nodeIds.insert( nIt->next()->GetID() );
    }
    return nodeIds.size();
  }
    
  return aSubMeshDS->NbNodes();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::long_array* SMESH_subMesh_i::GetElementsId()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetElementsId");
  SMESH::long_array_var aResult = new SMESH::long_array();

  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return aResult._retn();

  ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];
  SMESHDS_SubMesh* aSubMeshDS = aSubMesh->GetSubMeshDS();

  int nbElems = aSubMeshDS ? aSubMeshDS->NbElements() : 0;
  TListOfSubMeshes smList;
  if ( nbElems )
    smList.push_back( aSubMeshDS );

  // volumes are bound to shell
  if ( nbElems == 0 && getSubMeshes( aSubMesh, smList ))
  {
    TListOfSubMeshes::iterator sm = smList.begin();
    for ( ; sm != smList.end(); ++sm )
      nbElems += (*sm)->NbElements();
  }

  aResult->length( nbElems );
  if ( nbElems )
  {
    TListOfSubMeshes::iterator sm = smList.begin();
    for ( int i = 0; sm != smList.end(); sm++ )
    {
      SMDS_ElemIteratorPtr anIt = (*sm)->GetElements();
      for ( int n = aSubMeshDS->NbElements(); i < n && anIt->more(); i++ )
        aResult[i] = anIt->next()->GetID();
    }
  }
  return aResult._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::long_array* SMESH_subMesh_i::GetElementsByType( SMESH::ElementType theElemType )
    throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetElementsByType");
  SMESH::long_array_var aResult = new SMESH::long_array();

  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return aResult._retn();

  ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];
  SMESHDS_SubMesh* aSubMeshDS = aSubMesh->GetSubMeshDS();

  // PAL5440, return all nodes belonging to elements of submesh
  set<int> nodeIds;
  int nbElems = aSubMeshDS ? aSubMeshDS->NbElements() : 0;

  // volumes may be bound to shell instead of solid
  TListOfSubMeshes smList;
  if ( nbElems == 0 && getSubMeshes( aSubMesh, smList ))
  {
    TListOfSubMeshes::iterator sm = smList.begin();
    for ( ; sm != smList.end(); ++sm )
    {
      if ( theElemType == SMESH::NODE )
      {
        SMDS_ElemIteratorPtr eIt = (*sm)->GetElements();
        while ( eIt->more() ) {
          const SMDS_MeshElement* anElem = eIt->next();
          SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
          while ( nIt->more() )
            nodeIds.insert( nIt->next()->GetID() );
        }
      }
      else
      {
        nbElems += (*sm)->NbElements();
      }
    }
    aSubMeshDS = 0;
  }
  else
  {
    if ( nbElems )
      smList.push_back( aSubMeshDS );
  }

  if ( theElemType == SMESH::NODE && aSubMeshDS )
  {
    SMDS_ElemIteratorPtr eIt = aSubMeshDS->GetElements();
    while ( eIt->more() ) {
      const SMDS_MeshElement* anElem = eIt->next();
      SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
      while ( nIt->more() )
        nodeIds.insert( nIt->next()->GetID() );
    }
  }

  if ( theElemType == SMESH::NODE )
    aResult->length( nodeIds.size() );
  else
    aResult->length( nbElems );

  int i = 0, n = aResult->length();

  if ( theElemType == SMESH::NODE && !nodeIds.empty() ) {
    set<int>::iterator idIt = nodeIds.begin();
    for ( ; i < n && idIt != nodeIds.end() ; i++, idIt++ )
      aResult[i] = *idIt;
  }

  if ( theElemType != SMESH::NODE ) {
    TListOfSubMeshes::iterator sm = smList.begin();
    for ( i = 0; sm != smList.end(); sm++ )
    {
      aSubMeshDS = *sm;
      SMDS_ElemIteratorPtr anIt = aSubMeshDS->GetElements();
      while ( i < n && anIt->more() ) {
        const SMDS_MeshElement* anElem = anIt->next();
        if ( theElemType == SMESH::ALL || anElem->GetType() == (SMDSAbs_ElementType)theElemType )
          aResult[i++] = anElem->GetID();
      }
    }
  }

  aResult->length( i );

  return aResult._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::long_array* SMESH_subMesh_i::GetNodesId()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetNodesId");
  SMESH::long_array_var aResult = GetElementsByType( SMESH::NODE );
  return aResult._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::SMESH_Mesh_ptr SMESH_subMesh_i::GetFather()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetFather");
  return _mesh_i->_this();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
CORBA::Long SMESH_subMesh_i::GetId()
{
  MESSAGE("SMESH_subMesh_i::GetId");
  return _localId;
}

//=======================================================================
//function : GetSubShape
//purpose  : 
//=======================================================================

GEOM::GEOM_Object_ptr SMESH_subMesh_i::GetSubShape()
     throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  GEOM::GEOM_Object_var aShapeObj;
  try {
    if ( _mesh_i->_mapSubMesh.find( _localId ) != _mesh_i->_mapSubMesh.end()) {
      TopoDS_Shape S = _mesh_i->_mapSubMesh[ _localId ]->GetSubShape();
      if ( !S.IsNull() )
        aShapeObj = _gen_i->ShapeToGeomObject( S );
    }
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return aShapeObj._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SALOME_MED::FAMILY_ptr SMESH_subMesh_i::GetFamily()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SALOME_MED::MESH_var MEDMesh = GetFather()->GetMEDMesh();

  SALOME_MED::Family_array_var families = 
    MEDMesh->getFamilies(SALOME_MED::MED_NODE);
    
  for ( int i = 0; i < families->length(); i++ ) {
    if ( families[i]->getIdentifier() == ( _localId ) )
      return families[i];
  }
  
  return SALOME_MED::FAMILY::_nil();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::long_array* SMESH_subMesh_i::GetIDs()
{
  SMESH::long_array_var aResult = GetElementsId();
  return aResult._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::ElementType SMESH_subMesh_i::GetElementType( const CORBA::Long id, const bool iselem )
  throw (SALOME::SALOME_Exception)
{
  return GetFather()->GetElementType( id, iselem );
}
