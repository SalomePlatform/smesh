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
//  File   : SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _SMESHDS_ListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListOfPtrHypothesis.hxx"
#endif
#ifndef _TopTools_ShapeMapHasher_HeaderFile
#include <TopTools_ShapeMapHasher.hxx>
#endif
#ifndef _SMESHDS_DataMapOfShapeListOfPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx"
#endif
#ifndef _SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis.hxx"
#endif
SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis::~SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis() {}
 


Standard_EXPORT Handle_Standard_Type& SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(TCollection_MapNode);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(TCollection_MapNode);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis",
			                                 sizeof(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis) Handle(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis))) {
       _anOtherObject = Handle(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis)((Handle(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis::DynamicType() const 
{ 
  return STANDARD_TYPE(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis) ; 
}
Standard_Boolean SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis) == AType || TCollection_MapNode::IsKind(AType)); 
}
Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis::~Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis() {}
#define TheKey TopoDS_Shape
#define TheKey_hxx <TopoDS_Shape.hxx>
#define TheItem SMESHDS_ListOfPtrHypothesis
#define TheItem_hxx <SMESHDS_ListOfPtrHypothesis.hxx>
#define Hasher TopTools_ShapeMapHasher
#define Hasher_hxx <TopTools_ShapeMapHasher.hxx>
#define TCollection_DataMapNode SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapNode_hxx <SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx>
#define TCollection_DataMapIterator SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapIterator_hxx <SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis.hxx>
#define Handle_TCollection_DataMapNode Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapNode_Type_() SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_Type_()
#define TCollection_DataMap SMESHDS_DataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMap_hxx <SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx>
#include <TCollection_DataMapNode.gxx>

