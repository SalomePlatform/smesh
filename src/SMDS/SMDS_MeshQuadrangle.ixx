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
//  File   : SMDS_MeshQuadrangle.ixx
//  Module : SMESH

#include "SMDS_MeshQuadrangle.jxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

SMDS_MeshQuadrangle::~SMDS_MeshQuadrangle() {}
 


Standard_EXPORT Handle_Standard_Type& SMDS_MeshQuadrangle_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(SMDS_MeshFace);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(SMDS_MeshFace);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(SMDS_MeshElement);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(SMDS_MeshElement);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(SMDS_MeshObject);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(SMDS_MeshObject);
  static Handle_Standard_Type aType4 = STANDARD_TYPE(MMgt_TShared);
  if ( aType4.IsNull()) aType4 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType5 = STANDARD_TYPE(Standard_Transient);
  if ( aType5.IsNull()) aType5 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,aType4,aType5,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMDS_MeshQuadrangle",
			                                 sizeof(SMDS_MeshQuadrangle),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMDS_MeshQuadrangle) Handle(SMDS_MeshQuadrangle)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMDS_MeshQuadrangle) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMDS_MeshQuadrangle))) {
       _anOtherObject = Handle(SMDS_MeshQuadrangle)((Handle(SMDS_MeshQuadrangle)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMDS_MeshQuadrangle::DynamicType() const 
{ 
  return STANDARD_TYPE(SMDS_MeshQuadrangle) ; 
}
Standard_Boolean SMDS_MeshQuadrangle::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMDS_MeshQuadrangle) == AType || SMDS_MeshFace::IsKind(AType)); 
}
Handle_SMDS_MeshQuadrangle::~Handle_SMDS_MeshQuadrangle() {}

