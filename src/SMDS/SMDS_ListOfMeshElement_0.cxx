//  SMESH SMDS : implementaion of Salome mesh data structure
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
//  File   : SMDS_ListOfMeshElement_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_ListOfMeshElement.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMDS_ListIteratorOfListOfMeshElement_HeaderFile
#include "SMDS_ListIteratorOfListOfMeshElement.hxx"
#endif
#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _SMDS_ListNodeOfListOfMeshElement_HeaderFile
#include "SMDS_ListNodeOfListOfMeshElement.hxx"
#endif
 

#define Item Handle_SMDS_MeshElement
#define Item_hxx <SMDS_MeshElement.hxx>
#define TCollection_ListNode SMDS_ListNodeOfListOfMeshElement
#define TCollection_ListNode_hxx <SMDS_ListNodeOfListOfMeshElement.hxx>
#define TCollection_ListIterator SMDS_ListIteratorOfListOfMeshElement
#define TCollection_ListIterator_hxx <SMDS_ListIteratorOfListOfMeshElement.hxx>
#define Handle_TCollection_ListNode Handle_SMDS_ListNodeOfListOfMeshElement
#define TCollection_ListNode_Type_() SMDS_ListNodeOfListOfMeshElement_Type_()
#define TCollection_List SMDS_ListOfMeshElement
#define TCollection_List_hxx <SMDS_ListOfMeshElement.hxx>
#include <TCollection_List.gxx>

