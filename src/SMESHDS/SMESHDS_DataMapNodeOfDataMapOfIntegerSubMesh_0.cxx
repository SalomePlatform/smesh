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
//  File   : SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMESHDS_SubMesh_HeaderFile
#include "SMESHDS_SubMesh.hxx"
#endif
#ifndef _TColStd_MapIntegerHasher_HeaderFile
#include <TColStd_MapIntegerHasher.hxx>
#endif
#ifndef _SMESHDS_DataMapOfIntegerSubMesh_HeaderFile
#include "SMESHDS_DataMapOfIntegerSubMesh.hxx"
#endif
#ifndef _SMESHDS_DataMapIteratorOfDataMapOfIntegerSubMesh_HeaderFile
#include "SMESHDS_DataMapIteratorOfDataMapOfIntegerSubMesh.hxx"
#endif
SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh::~SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh() {}
 


Standard_EXPORT Handle_Standard_Type& SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh",
			                                 sizeof(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh) Handle(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh))) {
       _anOtherObject = Handle(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh)((Handle(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh::DynamicType() const 
{ 
  return STANDARD_TYPE(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh) ; 
}
Standard_Boolean SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh::~Handle_SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh() {}
#define TheKey Standard_Integer
#define TheKey_hxx <Standard_Integer.hxx>
#define TheItem Handle_SMESHDS_SubMesh
#define TheItem_hxx <SMESHDS_SubMesh.hxx>
#define Hasher TColStd_MapIntegerHasher
#define Hasher_hxx <TColStd_MapIntegerHasher.hxx>
#define TCollection_DataMapNode SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh
#define TCollection_DataMapNode_hxx <SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh.hxx>
#define TCollection_DataMapIterator SMESHDS_DataMapIteratorOfDataMapOfIntegerSubMesh
#define TCollection_DataMapIterator_hxx <SMESHDS_DataMapIteratorOfDataMapOfIntegerSubMesh.hxx>
#define Handle_TCollection_DataMapNode Handle_SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh
#define TCollection_DataMapNode_Type_() SMESHDS_DataMapNodeOfDataMapOfIntegerSubMesh_Type_()
#define TCollection_DataMap SMESHDS_DataMapOfIntegerSubMesh
#define TCollection_DataMap_hxx <SMESHDS_DataMapOfIntegerSubMesh.hxx>
#include <TCollection_DataMapNode.gxx>

