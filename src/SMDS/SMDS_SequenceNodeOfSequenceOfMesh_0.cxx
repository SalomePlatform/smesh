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
//  File   : SMDS_SequenceNodeOfSequenceOfMesh_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_SequenceNodeOfSequenceOfMesh.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMDS_Mesh_HeaderFile
#include "SMDS_Mesh.hxx"
#endif
#ifndef _SMDS_SequenceOfMesh_HeaderFile
#include "SMDS_SequenceOfMesh.hxx"
#endif
SMDS_SequenceNodeOfSequenceOfMesh::~SMDS_SequenceNodeOfSequenceOfMesh() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_SequenceNodeOfSequenceOfMesh_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_SeqNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_SeqNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_SequenceNodeOfSequenceOfMesh",
			                                 sizeof(SMDS_SequenceNodeOfSequenceOfMesh),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_SequenceNodeOfSequenceOfMesh) Handle(SMDS_SequenceNodeOfSequenceOfMesh)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_SequenceNodeOfSequenceOfMesh) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_SequenceNodeOfSequenceOfMesh))) {
       _anOtherObject = Handle(SMDS_SequenceNodeOfSequenceOfMesh)((Handle(SMDS_SequenceNodeOfSequenceOfMesh)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_SequenceNodeOfSequenceOfMesh::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_SequenceNodeOfSequenceOfMesh) ; 
}
Standard_Boolean SMDS_SequenceNodeOfSequenceOfMesh::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_SequenceNodeOfSequenceOfMesh) == AType || TCollection_SeqNode::IsKind(AType)); 
}
Handle_SMDS_SequenceNodeOfSequenceOfMesh::~Handle_SMDS_SequenceNodeOfSequenceOfMesh() {}
#define SeqItem Handle_SMDS_Mesh
#define SeqItem_hxx "SMDS_Mesh.hxx"
#define TCollection_SequenceNode SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_hxx "SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#define Handle_TCollection_SequenceNode Handle_SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_Type_() SMDS_SequenceNodeOfSequenceOfMesh_Type_()
#define TCollection_Sequence SMDS_SequenceOfMesh
#define TCollection_Sequence_hxx "SMDS_SequenceOfMesh.hxx"
#include <TCollection_SequenceNode.gxx>

