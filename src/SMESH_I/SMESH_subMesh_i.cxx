// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_subMesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "SMESH_subMesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_PreMeshInfo.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"
#include "OpUtil.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <TopoDS_Iterator.hxx>
#include <TopExp_Explorer.hxx>

using namespace std;

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
  _gen_i = gen_i;
  _mesh_i = mesh_i;
  _localId = localId;
  _preMeshInfo = NULL;
}
//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_subMesh_i::~SMESH_subMesh_i()
{
  MESSAGE("SMESH_subMesh_i::~SMESH_subMesh_i");
  if ( _preMeshInfo ) delete _preMeshInfo;
  _preMeshInfo = NULL;
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
        if ( ::SMESH_subMesh* aSubMesh = aMesh->GetSubMeshContaining( it.Value() ))
          getSubMeshes( aSubMesh, theSubMeshList ); // add found submesh or explore deeper
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
  default:
    if ( aSubMeshDS )
      theSubMeshList.push_back( aSubMeshDS );
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

  if ( _preMeshInfo )
    return _preMeshInfo->NbElements();

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

  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return 0;

  if ( _preMeshInfo )
  {
    if ( all ) return _preMeshInfo->NbNodes();
    else _preMeshInfo->FullLoadFromFile();
  }
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
      if ( eIt->more() ) {
        while ( eIt->more() ) {
          const SMDS_MeshElement* anElem = eIt->next();
          SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
          while ( nIt->more() )
            nodeIds.insert( nIt->next()->GetID() );
        }
      } else {
        SMDS_NodeIteratorPtr nIt = (*sm)->GetNodes();
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
    if ( eIt->more() ) {
      while ( eIt->more() ) {
        const SMDS_MeshElement* anElem = eIt->next();
        SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
        while ( nIt->more() )
          nodeIds.insert( nIt->next()->GetID() );
      }
    } else {
      SMDS_NodeIteratorPtr nIt = aSubMeshDS->GetNodes();
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

  SMESH::long_array_var aResult = new SMESH::long_array();

  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return aResult._retn();

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
      for ( ; i < nbElems && anIt->more(); i++ )
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

  SMESH::long_array_var aResult = new SMESH::long_array();

  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return aResult._retn();

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
        if ( eIt->more() ) {
          while ( eIt->more() ) {
            const SMDS_MeshElement* anElem = eIt->next();
            SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
            while ( nIt->more() )
              nodeIds.insert( nIt->next()->GetID() );
          }
        } else {
          SMDS_NodeIteratorPtr nIt = (*sm)->GetNodes();
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
    if ( eIt->more() ) {
      while ( eIt->more() ) {
        const SMDS_MeshElement* anElem = eIt->next();
        SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
        while ( nIt->more() )
          nodeIds.insert( nIt->next()->GetID() );
      }
    } else {
      SMDS_NodeIteratorPtr nIt = aSubMeshDS->GetNodes();
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
  return _mesh_i->_this();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
CORBA::Long SMESH_subMesh_i::GetId()
{
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
      if ( !S.IsNull() ) {
        aShapeObj = _gen_i->ShapeToGeomObject( S );
        //mzn: N7PAL16232, N7PAL16233
        //In some cases it's possible that GEOM_Client contains the shape same to S, but
        //with another orientation.
        if (aShapeObj->_is_nil())
          aShapeObj = _gen_i->ShapeToGeomObject( S.Reversed() );
      }
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();
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
  return GetElementsId();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::ElementType SMESH_subMesh_i::GetElementType( const CORBA::Long id, const bool iselem )
  throw (SALOME::SALOME_Exception)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();
  return GetFather()->GetElementType( id, iselem );
}


//=============================================================================
/*!
 * Returns statistic of mesh elements
 * Result array of number enityties
 * Inherited from SMESH_IDSource
 */
//=============================================================================
SMESH::long_array* SMESH_subMesh_i::GetMeshInfo()
{
  if ( _preMeshInfo )
    return _preMeshInfo->GetMeshInfo();

  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::Entity_Last);
  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = 0;
  
  // get number of nodes
  aRes[ SMESH::Entity_Node ] = GetNumberOfNodes(true);
 
  ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];

  // get statistic from child sub-meshes
  TListOfSubMeshes smList;
  if ( getSubMeshes( aSubMesh, smList ) )
    for ( TListOfSubMeshes::iterator sm = smList.begin(); sm != smList.end(); ++sm )
      SMESH_Mesh_i::CollectMeshInfo( (*sm)->GetElements(), aRes );

  return aRes._retn();
}


//=======================================================================
//function : GetTypes
//purpose  : Returns types of elements it contains
//=======================================================================

SMESH::array_of_ElementType* SMESH_subMesh_i::GetTypes()
{
  if ( _preMeshInfo )
    return _preMeshInfo->GetTypes();

  SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;

  ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];
  if ( SMESHDS_SubMesh* smDS = aSubMesh->GetSubMeshDS() )
  {
    SMDS_ElemIteratorPtr eIt = smDS->GetElements();
    if ( eIt->more() )
    {
      types->length( 1 );
      types[0] = SMESH::ElementType( eIt->next()->GetType());
    }
    else if ( smDS->GetNodes()->more() )
    {
      TopoDS_Shape shape = aSubMesh->GetSubShape();
      while ( !shape.IsNull() && shape.ShapeType() == TopAbs_COMPOUND )
      {
        TopoDS_Iterator it( shape );
        shape = it.More() ? it.Value() : TopoDS_Shape();
      }
      if ( !shape.IsNull() && shape.ShapeType() == TopAbs_VERTEX )
      {
        types->length( 1 );
        types[0] = SMESH::NODE;
      }
    }
  }
  return types._retn();
}

//=======================================================================
//function : GetMesh
//purpose  : interface SMESH_IDSource
//=======================================================================

SMESH::SMESH_Mesh_ptr SMESH_subMesh_i::GetMesh()
{
  return GetFather();
}

//=======================================================================
//function : IsMeshInfoCorrect
//purpose  : * Returns false if GetMeshInfo() returns incorrect information that may
//           * happen if mesh data is not yet fully loaded from the file of study.
//=======================================================================

bool SMESH_subMesh_i::IsMeshInfoCorrect()
{
  return _preMeshInfo ? _preMeshInfo->IsMeshInfoCorrect() : true;
}
