//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//
//  Copyright (C) 2004  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_Group_i.cxx
//  Author : Sergey ANIKIN, OCC
//  Module : SMESH
//  $Header$

#include "SMESH_Group_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include <SMESH_Group.hxx>
#include <SMESHDS_Group.hxx>
#include <SMDSAbs_ElementType.hxx>
#include <utilities.h>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Group_i::SMESH_Group_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID )
: SALOME::GenericObj_i( thePOA ),
  myMeshServant( theMeshServant ), 
  myLocalID( theLocalID )
{
  thePOA->activate_object( this );
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Group_i::~SMESH_Group_i()
{
  MESSAGE("~SMESH_Group_i;" );
  if ( myMeshServant )
    myMeshServant->removeGroup(myLocalID);
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Group_i::SetName( const char* theName )
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      aGroup->SetName(theName);

      // Update group name in a study
      SALOMEDS::Study_var aStudy = myMeshServant->GetGen()->GetCurrentStudy();
      if ( !aStudy->_is_nil() ) {
	SALOMEDS::SObject_var aGroupSO = aStudy->FindObjectIOR( SMESH_Gen_i::GetORB()->object_to_string( _this() ) );
	if ( !aGroupSO->_is_nil() ) {
	  SALOMEDS::StudyBuilder_var aBuilder = aStudy->NewBuilder();
	  aBuilder->SetName( aGroupSO, theName );
	}
      }
      return;
    }
  }
  MESSAGE("can't set name of a vague group");
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

char* SMESH_Group_i::GetName()
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup)
      return CORBA::string_dup (aGroup->GetName());
  }
  MESSAGE("get name of a vague group");
  return CORBA::string_dup( "NO_NAME" );
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::ElementType SMESH_Group_i::GetType()
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      SMDSAbs_ElementType aSMDSType = aGroup->GetGroupDS()->GetType();
      SMESH::ElementType aType;
      switch (aSMDSType) {
      case SMDSAbs_Node:   aType = SMESH::NODE; break;
      case SMDSAbs_Edge:   aType = SMESH::EDGE; break;
      case SMDSAbs_Face:   aType = SMESH::FACE; break;
      case SMDSAbs_Volume: aType = SMESH::VOLUME; break;
      default:             aType = SMESH::ALL; break;
      }
      return aType;
    }
  }
  MESSAGE("get type of a vague group");
  return SMESH::ALL;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Group_i::Size()
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      int aSize = aGroup->GetGroupDS()->Extent();
      return aSize;
    }
  }
  MESSAGE("get size of a vague group");
  return 0;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_Group_i::IsEmpty()
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      bool isEmpty = aGroup->GetGroupDS()->IsEmpty();
      return isEmpty;
    }
  }
  MESSAGE("checking IsEmpty of a vague group");
  return true;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Group_i::Clear()
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      // a SMDS group forgets its type after clearing, so we must re-set it
      SMDSAbs_ElementType aSMDSType = aGroup->GetGroupDS()->GetType();
      aGroup->GetGroupDS()->Clear();
      aGroup->GetGroupDS()->SetType(aSMDSType);
      return;
    }
  }
  MESSAGE("attempt to clear a vague group");
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_Group_i::Contains( CORBA::Long theID )
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      bool res = aGroup->GetGroupDS()->Contains(theID);
      return res;
    }
  }
  MESSAGE("attempt to check contents of a vague group");
  return false;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Group_i::Add( const SMESH::long_array& theIDs )
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      SMESHDS_Group* aGroupDS = aGroup->GetGroupDS();
      int nbAdd = 0;
      for (int i = 0; i < theIDs.length(); i++) {
        int anID = (int) theIDs[i];
        if (aGroupDS->Add(anID))
          nbAdd++;
      }
      return nbAdd;
    }
  }
  MESSAGE("attempt to add elements to a vague group");
  return 0;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Group_i::GetID( CORBA::Long theIndex )
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      int anID = aGroup->GetGroupDS()->GetID(theIndex);
      return anID;
    }
  }
  MESSAGE("attempt to iterate on a vague group");
  return -1;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::long_array* SMESH_Group_i::GetListOfID()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      SMESHDS_Group* aGroupDS = aGroup->GetGroupDS();
      int aSize = aGroupDS->Extent();
      aRes->length(aSize);
      for (int i = 0; i < aSize; i++)
        aRes[i] = aGroupDS->GetID(i+1);
      return aRes._retn();
    }
  }
  MESSAGE("get list of IDs of a vague group");
  return aRes._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Group_i::Remove( const SMESH::long_array& theIDs )
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    ::SMESH_Group* aGroup = aMesh.GetGroup(myLocalID);
    if (aGroup) {
      // a SMDS group forgets its type after clearing, so we must re-set it
      // if the group becomes empty
      SMDSAbs_ElementType aSMDSType = aGroup->GetGroupDS()->GetType();
      SMESHDS_Group* aGroupDS = aGroup->GetGroupDS();
      int nbDel = 0;
      for (int i = 0; i < theIDs.length(); i++) {
        int anID = (int) theIDs[i];
        if (aGroupDS->Remove(anID))
          nbDel++;
      }
      if (aGroupDS->IsEmpty())
        aGroupDS->SetType(aSMDSType);
      return nbDel;
    }
  }
  MESSAGE("attempt to remove elements from a vague group");
  return 0;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESH_Group_i::GetMesh()
{
  MESSAGE("SMESH_Group_i::GetMesh(): mesh servant = " << myMeshServant );
  SMESH::SMESH_Mesh_var aMesh;
  if ( myMeshServant )
    aMesh = SMESH::SMESH_Mesh::_narrow( myMeshServant->_this() );
  return aMesh._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Group_i::GetLocalID()
{
  return myLocalID;
}
