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
//  File   : SMDS_ListOfMesh_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_ListOfMesh.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMDS_ListIteratorOfListOfMesh_HeaderFile
#include "SMDS_ListIteratorOfListOfMesh.hxx"
#endif
#ifndef _SMDS_Mesh_HeaderFile
#include "SMDS_Mesh.hxx"
#endif
#ifndef _SMDS_ListNodeOfListOfMesh_HeaderFile
#include "SMDS_ListNodeOfListOfMesh.hxx"
#endif
 

#define Item Handle_SMDS_Mesh
#define Item_hxx <SMDS_Mesh.hxx>
#define TCollection_ListNode SMDS_ListNodeOfListOfMesh
#define TCollection_ListNode_hxx <SMDS_ListNodeOfListOfMesh.hxx>
#define TCollection_ListIterator SMDS_ListIteratorOfListOfMesh
#define TCollection_ListIterator_hxx <SMDS_ListIteratorOfListOfMesh.hxx>
#define Handle_TCollection_ListNode Handle_SMDS_ListNodeOfListOfMesh
#define TCollection_ListNode_Type_() SMDS_ListNodeOfListOfMesh_Type_()
#define TCollection_List SMDS_ListOfMesh
#define TCollection_List_hxx <SMDS_ListOfMesh.hxx>
#include <TCollection_List.gxx>

