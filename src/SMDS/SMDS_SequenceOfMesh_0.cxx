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
//  File   : SMDS_SequenceOfMesh_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_SequenceOfMesh.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _Standard_OutOfRange_HeaderFile
#include <Standard_OutOfRange.hxx>
#endif
#ifndef _SMDS_Mesh_HeaderFile
#include "SMDS_Mesh.hxx"
#endif
#ifndef _SMDS_SequenceNodeOfSequenceOfMesh_HeaderFile
#include "SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#endif
 

#define SeqItem Handle_SMDS_Mesh
#define SeqItem_hxx "SMDS_Mesh.hxx"
#define TCollection_SequenceNode SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_hxx "SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#define Handle_TCollection_SequenceNode Handle_SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_Type_() SMDS_SequenceNodeOfSequenceOfMesh_Type_()
#define TCollection_Sequence SMDS_SequenceOfMesh
#define TCollection_Sequence_hxx "SMDS_SequenceOfMesh.hxx"
#include <TCollection_Sequence.gxx>

