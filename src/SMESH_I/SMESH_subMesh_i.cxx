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
using namespace std;
#include "SMESH_subMesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"
#include "OpUtil.hxx"
#include "Utils_ExceptHandlers.hxx"

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

  SMESHDS_SubMesh* aSubMeshDS = _mesh_i->_mapSubMesh[_localId]->GetSubMeshDS();
  if ( aSubMeshDS == NULL )
    return 0;

  return aSubMeshDS->NbElements();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_subMesh_i::GetNumberOfNodes()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetNumberOfNodes");
  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return 0;

  SMESHDS_SubMesh* aSubMeshDS = _mesh_i->_mapSubMesh[_localId]->GetSubMeshDS();
  if ( aSubMeshDS == NULL )
    return 0;

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

  SMESHDS_SubMesh* aSubMeshDS = _mesh_i->_mapSubMesh[_localId]->GetSubMeshDS();
  if ( aSubMeshDS == NULL )
    return aResult._retn();

  aResult->length( aSubMeshDS->NbElements() );
  SMDS_ElemIteratorPtr anIt = aSubMeshDS->GetElements();
  for ( int i = 0, n = aSubMeshDS->NbElements(); i < n && anIt->more(); i++ )
    aResult[i] = anIt->next()->GetID();

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

  SMESHDS_SubMesh* aSubMeshDS = _mesh_i->_mapSubMesh[_localId]->GetSubMeshDS();
  if ( aSubMeshDS == NULL )
    return aResult._retn();

  int i = 0;
  if ( theElemType == SMESH::ALL )
    aResult->length( aSubMeshDS->NbElements() + aSubMeshDS->NbNodes() );
  else if ( theElemType == SMESH::NODE )
    aResult->length( aSubMeshDS->NbNodes() );
  else
    aResult->length( aSubMeshDS->NbElements() );

  int n = aResult->length();

  if ( theElemType == SMESH::ALL || theElemType == SMESH::NODE ) {
    SMDS_NodeIteratorPtr anIt = aSubMeshDS->GetNodes();
    while ( i < n && anIt->more() )
      aResult[i++] = anIt->next()->GetID();
  }

  if ( theElemType == SMESH::ALL || theElemType != SMESH::NODE ) {
    SMDS_ElemIteratorPtr anIt = aSubMeshDS->GetElements();
    while ( i < n && anIt->more() ) {
      const SMDS_MeshElement* anElem = anIt->next();
      if ( theElemType == SMESH::ALL || anElem->GetType() == (SMDSAbs_ElementType)theElemType )
	aResult[i++] = anElem->GetID();
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
  SMESH::long_array_var aResult = new SMESH::long_array();

  if ( _mesh_i->_mapSubMesh.find( _localId ) == _mesh_i->_mapSubMesh.end() )
    return aResult._retn();

  SMESHDS_SubMesh* aSubMeshDS = _mesh_i->_mapSubMesh[_localId]->GetSubMeshDS();
  if ( aSubMeshDS == NULL )
    return aResult._retn();

  aResult->length( aSubMeshDS->NbNodes() );
  SMDS_NodeIteratorPtr anIt = aSubMeshDS->GetNodes();
  for ( int i = 0, n = aSubMeshDS->NbNodes(); i < n && anIt->more(); i++ )
    aResult[i] = anIt->next()->GetID();

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
}
