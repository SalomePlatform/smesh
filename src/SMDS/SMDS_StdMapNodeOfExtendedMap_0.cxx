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
//  File   : SMDS_StdMapNodeOfExtendedMap_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_StdMapNodeOfExtendedMap.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _SMDS_MeshElementMapHasher_HeaderFile
#include "SMDS_MeshElementMapHasher.hxx"
#endif
#ifndef _SMDS_ExtendedMap_HeaderFile
#include "SMDS_ExtendedMap.hxx"
#endif
#ifndef _SMDS_MapIteratorOfExtendedMap_HeaderFile
#include "SMDS_MapIteratorOfExtendedMap.hxx"
#endif
SMDS_StdMapNodeOfExtendedMap::~SMDS_StdMapNodeOfExtendedMap() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_StdMapNodeOfExtendedMap_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_StdMapNodeOfExtendedMap",
			                                 sizeof(SMDS_StdMapNodeOfExtendedMap),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_StdMapNodeOfExtendedMap) Handle(SMDS_StdMapNodeOfExtendedMap)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_StdMapNodeOfExtendedMap) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_StdMapNodeOfExtendedMap))) {
       _anOtherObject = Handle(SMDS_StdMapNodeOfExtendedMap)((Handle(SMDS_StdMapNodeOfExtendedMap)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_StdMapNodeOfExtendedMap::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_StdMapNodeOfExtendedMap) ; 
}
Standard_Boolean SMDS_StdMapNodeOfExtendedMap::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_StdMapNodeOfExtendedMap) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMDS_StdMapNodeOfExtendedMap::~Handle_SMDS_StdMapNodeOfExtendedMap() {}
#define TheKey Handle_SMDS_MeshElement
#define TheKey_hxx <SMDS_MeshElement.hxx>
#define Hasher SMDS_MeshElementMapHasher
#define Hasher_hxx <SMDS_MeshElementMapHasher.hxx>
#define TCollection_StdMapNode SMDS_StdMapNodeOfExtendedMap
#define TCollection_StdMapNode_hxx <SMDS_StdMapNodeOfExtendedMap.hxx>
#define TCollection_MapIterator SMDS_MapIteratorOfExtendedMap
#define TCollection_MapIterator_hxx <SMDS_MapIteratorOfExtendedMap.hxx>
#define Handle_TCollection_StdMapNode Handle_SMDS_StdMapNodeOfExtendedMap
#define TCollection_StdMapNode_Type_() SMDS_StdMapNodeOfExtendedMap_Type_()
#define TCollection_Map SMDS_ExtendedMap
#define TCollection_Map_hxx <SMDS_ExtendedMap.hxx>
#include <TCollection_StdMapNode.gxx>

