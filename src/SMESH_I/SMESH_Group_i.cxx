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
#include "SMESH_Group.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMDSAbs_ElementType.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_GroupBase_i::SMESH_GroupBase_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID )
: SALOME::GenericObj_i( thePOA ),
  myMeshServant( theMeshServant ), 
  myLocalID( theLocalID )
{
  thePOA->activate_object( this );
}

SMESH_Group_i::SMESH_Group_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID )
: SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
}

SMESH_GroupOnGeom_i::SMESH_GroupOnGeom_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID )
: SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_GroupBase_i::~SMESH_GroupBase_i()
{
  MESSAGE("~SMESH_GroupBase_i;" );
  if ( myMeshServant )
    myMeshServant->removeGroup(myLocalID);
}

//=======================================================================
//function : GetSmeshGroup
//purpose  : 
//=======================================================================

::SMESH_Group* SMESH_GroupBase_i::GetSmeshGroup() const
{
  if ( myMeshServant ) {
    ::SMESH_Mesh& aMesh = myMeshServant->GetImpl();
    return aMesh.GetGroup(myLocalID);
  }
  return 0;
}

//=======================================================================
//function : GetGroupDS
//purpose  : 
//=======================================================================

SMESHDS_GroupBase* SMESH_GroupBase_i::GetGroupDS() const
{
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if ( aGroup )
    return aGroup->GetGroupDS();
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_GroupBase_i::SetName( const char* theName )
{
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if (aGroup) {
    aGroup->SetName(theName);

    // Update group name in a study
    SMESH_Gen_i* aGen = myMeshServant->GetGen();
    aGen->SetName( aGen->ObjectToSObject( aGen->GetCurrentStudy(), _this() ), theName );
    return;
  }
  MESSAGE("can't set name of a vague group");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

char* SMESH_GroupBase_i::GetName()
{
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if (aGroup)
    return CORBA::string_dup (aGroup->GetName());
  MESSAGE("get name of a vague group");
  return CORBA::string_dup( "NO_NAME" );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::ElementType SMESH_GroupBase_i::GetType()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS) {
    SMDSAbs_ElementType aSMDSType = aGroupDS->GetType();
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
  MESSAGE("get type of a vague group");
  return SMESH::ALL;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_GroupBase_i::Size()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->Extent();
  MESSAGE("get size of a vague group");
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_GroupBase_i::IsEmpty()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->IsEmpty();
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
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    aGroupDS->Clear();
    return;
  }
  MESSAGE("attempt to clear a vague group");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Boolean SMESH_GroupBase_i::Contains( CORBA::Long theID )
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->Contains(theID);
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
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    int nbAdd = 0;
    for (int i = 0; i < theIDs.length(); i++) {
      int anID = (int) theIDs[i];
      if (aGroupDS->Add(anID))
        nbAdd++;
    }
    return nbAdd;
  }
  MESSAGE("attempt to add elements to a vague group");
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_GroupBase_i::GetID( CORBA::Long theIndex )
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->GetID(theIndex);
  MESSAGE("attempt to iterate on a vague group");
  return -1;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::long_array* SMESH_GroupBase_i::GetListOfID()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS) {
    int aSize = aGroupDS->Extent();
    aRes->length(aSize);
    for (int i = 0; i < aSize; i++)
      aRes[i] = aGroupDS->GetID(i+1);
    return aRes._retn();
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
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    int nbDel = 0;
    for (int i = 0; i < theIDs.length(); i++) {
      int anID = (int) theIDs[i];
      if (aGroupDS->Remove(anID))
        nbDel++;
    }
    return nbDel;
  }
  MESSAGE("attempt to remove elements from a vague group");
  return 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESH_GroupBase_i::GetMesh()
{
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
SMESH::long_array* SMESH_GroupBase_i::GetIDs()
{
  SMESH::long_array_var aResult = GetListOfID();
  return aResult._retn();
}

//=======================================================================
//function : GetShape
//purpose  : 
//=======================================================================

GEOM::GEOM_Object_ptr SMESH_GroupOnGeom_i::GetShape()
{
  GEOM::GEOM_Object_var aGeomObj;
  SMESHDS_GroupOnGeom* aGroupDS = dynamic_cast<SMESHDS_GroupOnGeom*>( GetGroupDS() );
  if ( aGroupDS ) {
    SMESH_Gen_i* aGen = GetMeshServant()->GetGen();
    aGeomObj = aGen->ShapeToGeomObject( aGroupDS->GetShape() );
  }
  return aGeomObj._retn();
}

