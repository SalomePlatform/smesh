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
//  File   : SMESHDS_ListOfCommand_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_ListOfCommand.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMESHDS_ListIteratorOfListOfCommand_HeaderFile
#include "SMESHDS_ListIteratorOfListOfCommand.hxx"
#endif
#ifndef _SMESHDS_Command_HeaderFile
#include "SMESHDS_Command.hxx"
#endif
#ifndef _SMESHDS_ListNodeOfListOfCommand_HeaderFile
#include "SMESHDS_ListNodeOfListOfCommand.hxx"
#endif
 

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
#include <TCollection_List.gxx>

