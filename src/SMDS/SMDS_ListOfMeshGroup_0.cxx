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
//  File   : SMDS_ListOfMeshGroup_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_ListOfMeshGroup.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMDS_ListIteratorOfListOfMeshGroup_HeaderFile
#include "SMDS_ListIteratorOfListOfMeshGroup.hxx"
#endif
#ifndef _SMDS_MeshGroup_HeaderFile
#include "SMDS_MeshGroup.hxx"
#endif
#ifndef _SMDS_ListNodeOfListOfMeshGroup_HeaderFile
#include "SMDS_ListNodeOfListOfMeshGroup.hxx"
#endif
 

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
#include <TCollection_List.gxx>

