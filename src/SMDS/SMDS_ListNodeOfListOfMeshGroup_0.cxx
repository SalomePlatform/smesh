//  SMESH SMDS : implementaion of Salome mesh data structure
//
//  Copyright (C) 2003  OPEN CASCADE
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
//  See http://www.opencascade.org or email : webmaster@opencascade.org 
//
//
//
//  File   : SMDS_ListNodeOfListOfMeshGroup_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_ListNodeOfListOfMeshGroup.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMDS_MeshGroup_HeaderFile
#include "SMDS_MeshGroup.hxx"
#endif
#ifndef _SMDS_ListOfMeshGroup_HeaderFile
#include "SMDS_ListOfMeshGroup.hxx"
#endif
#ifndef _SMDS_ListIteratorOfListOfMeshGroup_HeaderFile
#include "SMDS_ListIteratorOfListOfMeshGroup.hxx"
#endif
SMDS_ListNodeOfListOfMeshGroup::~SMDS_ListNodeOfListOfMeshGroup() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_ListNodeOfListOfMeshGroup_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_ListNodeOfListOfMeshGroup",
			                                 sizeof(SMDS_ListNodeOfListOfMeshGroup),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_ListNodeOfListOfMeshGroup) Handle(SMDS_ListNodeOfListOfMeshGroup)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_ListNodeOfListOfMeshGroup) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_ListNodeOfListOfMeshGroup))) {
       _anOtherObject = Handle(SMDS_ListNodeOfListOfMeshGroup)((Handle(SMDS_ListNodeOfListOfMeshGroup)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_ListNodeOfListOfMeshGroup::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_ListNodeOfListOfMeshGroup) ; 
}
Standard_Boolean SMDS_ListNodeOfListOfMeshGroup::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_ListNodeOfListOfMeshGroup) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMDS_ListNodeOfListOfMeshGroup::~Handle_SMDS_ListNodeOfListOfMeshGroup() {}
#define Item Handle_SMDS_MeshGroup
#define Item_hxx "SMDS_MeshGroup.hxx"
#define TCollection_ListNode SMDS_ListNodeOfListOfMeshGroup
#define TCollection_ListNode_hxx "SMDS_ListNodeOfListOfMeshGroup.hxx"
#define TCollection_ListIterator SMDS_ListIteratorOfListOfMeshGroup
#define TCollection_ListIterator_hxx "SMDS_ListIteratorOfListOfMeshGroup.hxx"
#define Handle_TCollection_ListNode Handle_SMDS_ListNodeOfListOfMeshGroup
#define TCollection_ListNode_Type_() SMDS_ListNodeOfListOfMeshGroup_Type_()
#define TCollection_List SMDS_ListOfMeshGroup
#define TCollection_List_hxx "SMDS_ListOfMeshGroup.hxx"
#include <TCollection_ListNode.gxx>

