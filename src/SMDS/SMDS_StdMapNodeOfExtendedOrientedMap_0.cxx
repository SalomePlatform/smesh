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
//  File   : SMDS_StdMapNodeOfExtendedOrientedMap_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_StdMapNodeOfExtendedOrientedMap.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _SMDS_MeshOrientedElementMapHasher_HeaderFile
#include "SMDS_MeshOrientedElementMapHasher.hxx"
#endif
#ifndef _SMDS_ExtendedOrientedMap_HeaderFile
#include "SMDS_ExtendedOrientedMap.hxx"
#endif
#ifndef _SMDS_MapIteratorOfExtendedOrientedMap_HeaderFile
#include "SMDS_MapIteratorOfExtendedOrientedMap.hxx"
#endif
SMDS_StdMapNodeOfExtendedOrientedMap::~SMDS_StdMapNodeOfExtendedOrientedMap() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_StdMapNodeOfExtendedOrientedMap_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_StdMapNodeOfExtendedOrientedMap",
			                                 sizeof(SMDS_StdMapNodeOfExtendedOrientedMap),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_StdMapNodeOfExtendedOrientedMap) Handle(SMDS_StdMapNodeOfExtendedOrientedMap)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_StdMapNodeOfExtendedOrientedMap) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_StdMapNodeOfExtendedOrientedMap))) {
       _anOtherObject = Handle(SMDS_StdMapNodeOfExtendedOrientedMap)((Handle(SMDS_StdMapNodeOfExtendedOrientedMap)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_StdMapNodeOfExtendedOrientedMap::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_StdMapNodeOfExtendedOrientedMap) ; 
}
Standard_Boolean SMDS_StdMapNodeOfExtendedOrientedMap::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_StdMapNodeOfExtendedOrientedMap) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMDS_StdMapNodeOfExtendedOrientedMap::~Handle_SMDS_StdMapNodeOfExtendedOrientedMap() {}
#define TheKey Handle_SMDS_MeshElement
#define TheKey_hxx <SMDS_MeshElement.hxx>
#define Hasher SMDS_MeshOrientedElementMapHasher
#define Hasher_hxx <SMDS_MeshOrientedElementMapHasher.hxx>
#define TCollection_StdMapNode SMDS_StdMapNodeOfExtendedOrientedMap
#define TCollection_StdMapNode_hxx <SMDS_StdMapNodeOfExtendedOrientedMap.hxx>
#define TCollection_MapIterator SMDS_MapIteratorOfExtendedOrientedMap
#define TCollection_MapIterator_hxx <SMDS_MapIteratorOfExtendedOrientedMap.hxx>
#define Handle_TCollection_StdMapNode Handle_SMDS_StdMapNodeOfExtendedOrientedMap
#define TCollection_StdMapNode_Type_() SMDS_StdMapNodeOfExtendedOrientedMap_Type_()
#define TCollection_Map SMDS_ExtendedOrientedMap
#define TCollection_Map_hxx <SMDS_ExtendedOrientedMap.hxx>
#include <TCollection_StdMapNode.gxx>

