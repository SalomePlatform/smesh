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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_subMesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "SMESH_subMesh_i.hxx"

#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_PreMeshInfo.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"
#include "OpUtil.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <TopoDS_Iterator.hxx>

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
  size_t size = theSubMeshList.size();

  // check all child sub-meshes of one complexity,
  // if no elements found and no algo assigned, go to children of lower complexity

  TopoDS_Shape            shape = theSubMesh->GetSubShape();
  TopAbs_ShapeEnum     mainType = SMESH_MesherHelper::GetGroupType( shape, /*noCompound=*/true );
  TopAbs_ShapeEnum    shapeType = shape.ShapeType();
  bool            elementsFound = false;
  bool                algoFound = false;
  SMESH_subMeshIteratorPtr smIt = theSubMesh->getDependsOnIterator(/*includeSelf=*/true,
                                                                   /*complexFirst=*/true);
  while ( smIt->more() )
  {
    ::SMESH_subMesh* sm = smIt->next();
    if ( sm->GetSubShape().ShapeType() != shapeType )
    {
      if ( elementsFound || algoFound )
        break;
      if ( sm->GetSubShape().ShapeType() == TopAbs_VERTEX &&
           mainType != TopAbs_VERTEX )
        break;
    }
    shapeType = sm->GetSubShape().ShapeType();
    if ( !sm->IsEmpty() )
    {
      elementsFound = true;
      theSubMeshList.push_back( sm->GetSubMeshDS() );
    }
    if ( sm->GetAlgo() )
      algoFound = true;
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

  int nbElems = 0;

  TListOfSubMeshes smList;
  if ( getSubMeshes( aSubMesh, smList ))
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

  if ( aSubMeshDS && aSubMeshDS->IsComplexSubmesh() )
  {
    // sub-mesh on a geom group, always return all nodes
    return aSubMeshDS->NbNodes();
  }
  if ( aSubMeshDS && !all )
  {
    // return anything we have
    return aSubMeshDS->NbNodes();
  }
  if ( all ) // get nodes from aSubMesh and all child sub-meshes
  {
    int nbNodes = 0;
    SMESH_subMeshIteratorPtr smIt = aSubMesh->getDependsOnIterator( /*includeSelf=*/true );
    while ( smIt->more() )
    {
      aSubMesh = smIt->next();
      if (( aSubMeshDS = aSubMesh->GetSubMeshDS() ))
        nbNodes += aSubMeshDS->NbNodes();
    }
    return nbNodes;
  }

  return aSubMeshDS ? aSubMeshDS->NbNodes() : 0;
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

  int nbElems = 0;
  TListOfSubMeshes smList;
  if ( getSubMeshes( aSubMesh, smList ))
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

  // PAL5440, return all nodes belonging to elements of the sub-mesh
  set<int> nodeIds;
  int nbElems = 0;

  // volumes may be bound to shell instead of solid
  TListOfSubMeshes smList;
  if ( getSubMeshes( aSubMesh, smList ))
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
      SMDS_ElemIteratorPtr anIt = (*sm)->GetElements();
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
/*
 * Returns number of mesh elements of each \a EntityType
 * @return array of number of elements per \a EntityType
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
/*
 * Returns number of mesh elements of each \a ElementType
 */
//=======================================================================

SMESH::long_array* SMESH_subMesh_i::GetNbElementsByType()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::NB_ELEMENT_TYPES);
  for (int i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
    if ( _preMeshInfo )
      aRes[ i ] = _preMeshInfo->NbElements( SMDSAbs_ElementType( i ));
    else
      aRes[ i ] = 0;

  if ( !_preMeshInfo )
  {
    aRes[ SMESH::NODE ] = GetNumberOfNodes(true);

    ::SMESH_subMesh* aSubMesh = _mesh_i->_mapSubMesh[_localId];
    if ( SMESHDS_SubMesh* smDS = aSubMesh->GetSubMeshDS() )
    {
      SMDS_ElemIteratorPtr eIt = smDS->GetElements();
      if ( eIt->more() )
        aRes[ eIt->next()->GetType() ] = smDS->NbElements();
    }
  }
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

//=======================================================================
//function : GetVtkUgStream
//purpose  : Return data vtk unstructured grid (not implemented)
//=======================================================================

SALOMEDS::TMPFile* SMESH_subMesh_i::GetVtkUgStream()
{
  SALOMEDS::TMPFile_var SeqFile;
  return SeqFile._retn();
}
