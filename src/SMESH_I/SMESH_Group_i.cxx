//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : SMESH_Group_i.cxx
//  Author : Sergey ANIKIN, OCC
//  Module : SMESH
//
#include "SMESH_Group_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMDSAbs_ElementType.hxx"

#include "SMESH_Filter_i.hxx"
#include "SMESH_PythonDump.hxx"

#include CORBA_SERVER_HEADER(SMESH_Filter)

#include "utilities.h"

using namespace SMESH;

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
  // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i,
  // servant activation is performed by SMESH_Mesh_i::createGroup()
  // thePOA->activate_object( this );
}

SMESH_Group_i::SMESH_Group_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID )
     : SALOME::GenericObj_i( thePOA ),
       SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
  //MESSAGE("SMESH_Group_i; this = "<<this );
}

SMESH_GroupOnGeom_i::SMESH_GroupOnGeom_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID )
     : SALOME::GenericObj_i( thePOA ),
       SMESH_GroupBase_i( thePOA, theMeshServant, theLocalID )
{
  //MESSAGE("SMESH_GroupOnGeom_i; this = "<<this );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_GroupBase_i::~SMESH_GroupBase_i()
{
  MESSAGE("~SMESH_GroupBase_i; this = "<<this );
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
  // Perform renaming
  ::SMESH_Group* aGroup = GetSmeshGroup();
  if (!aGroup) {
    MESSAGE("can't set name of a vague group");
    return;
  }

  if ( aGroup->GetName() && !strcmp( aGroup->GetName(), theName ) )
    return; // nothing to rename

  aGroup->SetName(theName);

  // Update group name in a study
  SMESH_Gen_i* aGen = myMeshServant->GetGen();
  aGen->SetName( aGen->ObjectToSObject( aGen->GetCurrentStudy(), _this() ), theName );
  
  // Update Python script
  TPythonDump() <<  _this() << ".SetName( '" << theName << "' )";
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
    case SMDSAbs_Node:      aType = SMESH::NODE;   break;
    case SMDSAbs_Edge:      aType = SMESH::EDGE;   break;
    case SMDSAbs_Face:      aType = SMESH::FACE;   break;
    case SMDSAbs_Volume:    aType = SMESH::VOLUME; break;
    case SMDSAbs_0DElement: aType = SMESH::ELEM0D; break;
    default:                aType = SMESH::ALL;    break;
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
  // Update Python script
  TPythonDump() << _this() << ".Clear()";

  // Clear the group
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
  // Update Python script
  TPythonDump() << "nbAdd = " << _this() << ".Add( " << theIDs << " )";

  // Add elements to the group
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

CORBA::Long SMESH_Group_i::Remove( const SMESH::long_array& theIDs )
{
  // Update Python script
  TPythonDump() << "nbDel = " << _this() << ".Remove( " << theIDs << " )";

  // Remove elements from the group
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

typedef bool (SMESHDS_Group::*TFunChangeGroup)(const int);

CORBA::Long 
ChangeByPredicate( SMESH::Predicate_i* thePredicate,
                   SMESHDS_GroupBase* theGroupBase,
                   TFunChangeGroup theFun)
{
  CORBA::Long aNb = 0;
  if(SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>(theGroupBase)){
    SMESH::Controls::Filter::TIdSequence aSequence;
    const SMDS_Mesh* aMesh = theGroupBase->GetMesh();
    SMESH::Filter_i::GetElementsId(thePredicate,aMesh,aSequence);
    
    CORBA::Long i = 0, iEnd = aSequence.size();
    for(; i < iEnd; i++)
      if((aGroupDS->*theFun)(aSequence[i]))
        aNb++;
    return aNb;
  }
  return aNb;
}

CORBA::Long 
SMESH_Group_i::
AddByPredicate( SMESH::Predicate_ptr thePredicate )
{
  if(SMESH::Predicate_i* aPredicate = SMESH::GetPredicate(thePredicate)){
    TPythonDump()<<_this()<<".AddByPredicate("<<aPredicate<<")";
    return ChangeByPredicate(aPredicate,GetGroupDS(),&SMESHDS_Group::Add);
  }
  return 0;
}

CORBA::Long 
SMESH_Group_i::
RemoveByPredicate( SMESH::Predicate_ptr thePredicate )
{
  if(SMESH::Predicate_i* aPredicate = SMESH::GetPredicate(thePredicate)){
    TPythonDump()<<_this()<<".RemoveByPredicate("<<aPredicate<<")";
    return ChangeByPredicate(aPredicate,GetGroupDS(),&SMESHDS_Group::Remove);
  }
  return 0;
}

CORBA::Long SMESH_Group_i::AddFrom( SMESH::SMESH_IDSource_ptr theSource )
{
  long nbAdd = 0;
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( GetGroupDS() );
  if (aGroupDS) {
    SMESH::long_array_var anIds;
    SMESH::SMESH_GroupBase_var group = SMESH::SMESH_GroupBase::_narrow(theSource);
    SMESH::SMESH_Mesh_var mesh       = SMESH::SMESH_Mesh::_narrow(theSource);
    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow(theSource);
    SMESH::Filter_var filter         = SMESH::Filter::_narrow(theSource);
    if ( !group->_is_nil())
      anIds = group->GetType()==GetType() ? theSource->GetIDs() :  new SMESH::long_array();
    else if ( !mesh->_is_nil() )
      anIds = mesh->GetElementsByType( GetType() );
    else if ( !submesh->_is_nil())
      anIds = submesh->GetElementsByType( GetType() );
    else if ( !filter->_is_nil() )
      anIds = filter->GetElementType()==GetType() ? theSource->GetIDs() : new SMESH::long_array();
    else 
      anIds = theSource->GetIDs();
    for ( int i = 0, total = anIds->length(); i < total; i++ ) {
      if ( aGroupDS->Add((int)anIds[i]) ) nbAdd++;
    }
  }

  // Update Python script
  TPythonDump() << "nbAdd = " << _this() << ".AddFrom( " << theSource << " )";

  return nbAdd;
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
SMESH::SMESH_Mesh_ptr SMESH_GroupBase_i::GetMesh()
{
  SMESH::SMESH_Mesh_var aMesh;
  if ( myMeshServant )
    aMesh = SMESH::SMESH_Mesh::_narrow( myMeshServant->_this() );
  return aMesh._retn();
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

//=============================================================================
/*!
 *
 */
//=============================================================================
SALOMEDS::Color SMESH_GroupBase_i::GetColor()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
  {
    Quantity_Color aQColor = aGroupDS->GetColor();
    SALOMEDS::Color aColor;
    aColor.R = aQColor.Red();
    aColor.G = aQColor.Green();
    aColor.B = aQColor.Blue();

    return aColor;
  }
  MESSAGE("get color of a group");
  return SALOMEDS::Color();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESH_GroupBase_i::SetColor(const SALOMEDS::Color& color)
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
  {
    Quantity_Color aQColor( color.R, color.G, color.B, Quantity_TOC_RGB );
    return aGroupDS->SetColor(aQColor);
  }
  MESSAGE("set color of a group");
  return ;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Long SMESH_GroupBase_i::GetColorNumber()
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->GetColorGroup();
  MESSAGE("get color number of a group");
  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESH_GroupBase_i::SetColorNumber(CORBA::Long color)
{
  SMESHDS_GroupBase* aGroupDS = GetGroupDS();
  if (aGroupDS)
    return aGroupDS->SetColorGroup(color);
  MESSAGE("set color number of a group");
  return ;
}

//=============================================================================
/*!
 * Returns statistic of mesh elements
 * Result array of number enityties
 * Inherited from SMESH_IDSource
 */
//=============================================================================
SMESH::long_array* SMESH_GroupBase_i::GetMeshInfo()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::Entity_Last);
  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = 0;

  SMESHDS_GroupBase* aGrpDS = GetGroupDS();
  if ( !aGrpDS )
    return aRes._retn();
  if ( GetType() == NODE )
    aRes[ SMESH::Entity_Node ] = aGrpDS->Extent();
  else
    SMESH_Mesh_i::CollectMeshInfo( aGrpDS->GetElements(), aRes);
  return aRes._retn();
}

//=======================================================================
//function : GetIDs
//purpose  : Returns ids of members
//=======================================================================

SMESH::long_array* SMESH_GroupBase_i::GetIDs()
{
  SMESH::long_array_var aResult = GetListOfID();
  return aResult._retn();
}

//=======================================================================
//function : GetTypes
//purpose  : Returns types of elements it contains
//=======================================================================

SMESH::array_of_ElementType* SMESH_GroupBase_i::GetTypes()
{
  SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;
  types->length( 1 );
  types[0] = GetType();
  return types._retn();
}

