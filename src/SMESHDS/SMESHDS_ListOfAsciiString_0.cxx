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
//  File   : SMESHDS_ListOfAsciiString_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_ListOfAsciiString.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMESHDS_ListIteratorOfListOfAsciiString_HeaderFile
#include "SMESHDS_ListIteratorOfListOfAsciiString.hxx"
#endif
#ifndef _SMESHDS_ListNodeOfListOfAsciiString_HeaderFile
#include "SMESHDS_ListNodeOfListOfAsciiString.hxx"
#endif
 

#define Item TCollection_AsciiString
#define Item_hxx <TCollection_AsciiString.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfAsciiString
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfAsciiString.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfAsciiString
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfAsciiString.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfAsciiString
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfAsciiString_Type_()
#define TCollection_List SMESHDS_ListOfAsciiString
#define TCollection_List_hxx <SMESHDS_ListOfAsciiString.hxx>
#include <TCollection_List.gxx>

