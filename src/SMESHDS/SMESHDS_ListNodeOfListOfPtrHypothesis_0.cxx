//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESHDS_ListNodeOfListOfPtrHypothesis_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_ListNodeOfListOfPtrHypothesis.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMESHDS_ListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListOfPtrHypothesis.hxx"
#endif
#ifndef _SMESHDS_ListIteratorOfListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx"
#endif
SMESHDS_ListNodeOfListOfPtrHypothesis::~SMESHDS_ListNodeOfListOfPtrHypothesis() {}
 


Standard_EXPORT Handle_Standard_Type& SMESHDS_ListNodeOfListOfPtrHypothesis_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMESHDS_ListNodeOfListOfPtrHypothesis",
			                                 sizeof(SMESHDS_ListNodeOfListOfPtrHypothesis),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMESHDS_ListNodeOfListOfPtrHypothesis) Handle(SMESHDS_ListNodeOfListOfPtrHypothesis)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMESHDS_ListNodeOfListOfPtrHypothesis) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMESHDS_ListNodeOfListOfPtrHypothesis))) {
       _anOtherObject = Handle(SMESHDS_ListNodeOfListOfPtrHypothesis)((Handle(SMESHDS_ListNodeOfListOfPtrHypothesis)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMESHDS_ListNodeOfListOfPtrHypothesis::DynamicType() const 
{ 
  return STANDARD_TYPE(SMESHDS_ListNodeOfListOfPtrHypothesis) ; 
}
Standard_Boolean SMESHDS_ListNodeOfListOfPtrHypothesis::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMESHDS_ListNodeOfListOfPtrHypothesis) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMESHDS_ListNodeOfListOfPtrHypothesis::~Handle_SMESHDS_ListNodeOfListOfPtrHypothesis() {}
#define Item SMESHDS_PtrHypothesis
#define Item_hxx <SMESHDS_PtrHypothesis.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfPtrHypothesis
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfPtrHypothesis.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfPtrHypothesis
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfPtrHypothesis
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfPtrHypothesis_Type_()
#define TCollection_List SMESHDS_ListOfPtrHypothesis
#define TCollection_List_hxx <SMESHDS_ListOfPtrHypothesis.hxx>
#include <TCollection_ListNode.gxx>

