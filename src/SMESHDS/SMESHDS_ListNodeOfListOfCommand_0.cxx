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
//  File   : SMESHDS_ListNodeOfListOfCommand_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_ListNodeOfListOfCommand.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMESHDS_Command_HeaderFile
#include "SMESHDS_Command.hxx"
#endif
#ifndef _SMESHDS_ListOfCommand_HeaderFile
#include "SMESHDS_ListOfCommand.hxx"
#endif
#ifndef _SMESHDS_ListIteratorOfListOfCommand_HeaderFile
#include "SMESHDS_ListIteratorOfListOfCommand.hxx"
#endif
SMESHDS_ListNodeOfListOfCommand::~SMESHDS_ListNodeOfListOfCommand() {}
 


Standard_EXPORT Handle_Standard_Type& SMESHDS_ListNodeOfListOfCommand_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMESHDS_ListNodeOfListOfCommand",
			                                 sizeof(SMESHDS_ListNodeOfListOfCommand),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMESHDS_ListNodeOfListOfCommand) Handle(SMESHDS_ListNodeOfListOfCommand)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMESHDS_ListNodeOfListOfCommand) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMESHDS_ListNodeOfListOfCommand))) {
       _anOtherObject = Handle(SMESHDS_ListNodeOfListOfCommand)((Handle(SMESHDS_ListNodeOfListOfCommand)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMESHDS_ListNodeOfListOfCommand::DynamicType() const 
{ 
  return STANDARD_TYPE(SMESHDS_ListNodeOfListOfCommand) ; 
}
Standard_Boolean SMESHDS_ListNodeOfListOfCommand::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMESHDS_ListNodeOfListOfCommand) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMESHDS_ListNodeOfListOfCommand::~Handle_SMESHDS_ListNodeOfListOfCommand() {}
#define Item Handle_SMESHDS_Command
#define Item_hxx <SMESHDS_Command.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfCommand
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfCommand.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfCommand
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfCommand.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfCommand
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfCommand_Type_()
#define TCollection_List SMESHDS_ListOfCommand
#define TCollection_List_hxx <SMESHDS_ListOfCommand.hxx>
#include <TCollection_ListNode.gxx>

