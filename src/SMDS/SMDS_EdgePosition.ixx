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
//  File   : SMDS_EdgePosition.ixx
//  Module : SMESH

#include "SMDS_EdgePosition.jxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

SMDS_EdgePosition::~SMDS_EdgePosition() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_EdgePosition_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(SMDS_Position);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(SMDS_Position);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_EdgePosition",
			                                 sizeof(SMDS_EdgePosition),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_EdgePosition) Handle(SMDS_EdgePosition)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_EdgePosition) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_EdgePosition))) {
       _anOtherObject = Handle(SMDS_EdgePosition)((Handle(SMDS_EdgePosition)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_EdgePosition::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_EdgePosition) ; 
}
Standard_Boolean SMDS_EdgePosition::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_EdgePosition) == AType || SMDS_Position::IsKind(AType)); 
}
Handle_SMDS_EdgePosition::~Handle_SMDS_EdgePosition() {}

