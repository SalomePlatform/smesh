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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com

#include "SMESH_TypeFilter.jxx"

#ifndef _Standard_TypeMismatch_HeaderFile
#include <Standard_TypeMismatch.hxx>
#endif

SMESH_TypeFilter::~SMESH_TypeFilter() {}
 


Standard_EXPORT Handle_Standard_Type& SMESH_TypeFilter_Type_()
{

    static Handle_Standard_Type aType1 = STANDARD_TYPE(SALOME_Filter);
  if ( aType1.IsNull()) aType1 = STANDARD_TYPE(SALOME_Filter);
  static Handle_Standard_Type aType2 = STANDARD_TYPE(MMgt_TShared);
  if ( aType2.IsNull()) aType2 = STANDARD_TYPE(MMgt_TShared);
  static Handle_Standard_Type aType3 = STANDARD_TYPE(Standard_Transient);
  if ( aType3.IsNull()) aType3 = STANDARD_TYPE(Standard_Transient);
 

  static Handle_Standard_Transient _Ancestors[]= {aType1,aType2,aType3,NULL};
  static Handle_Standard_Type _aType = new Standard_Type("SMESH_TypeFilter",
			                                 sizeof(SMESH_TypeFilter),
			                                 1,
			                                 (Standard_Address)_Ancestors,
			                                 (Standard_Address)NULL);

  return _aType;
}


// DownCast method
//   allow safe downcasting
//
const Handle(SMESH_TypeFilter) Handle(SMESH_TypeFilter)::DownCast(const Handle(Standard_Transient)& AnObject) 
{
  Handle(SMESH_TypeFilter) _anOtherObject;

  if (!AnObject.IsNull()) {
     if (AnObject->IsKind(STANDARD_TYPE(SMESH_TypeFilter))) {
       _anOtherObject = Handle(SMESH_TypeFilter)((Handle(SMESH_TypeFilter)&)AnObject);
     }
  }

  return _anOtherObject ;
}
const Handle(Standard_Type)& SMESH_TypeFilter::DynamicType() const 
{ 
  return STANDARD_TYPE(SMESH_TypeFilter) ; 
}
Standard_Boolean SMESH_TypeFilter::IsKind(const Handle(Standard_Type)& AType) const 
{ 
  return (STANDARD_TYPE(SMESH_TypeFilter) == AType || SALOME_Filter::IsKind(AType)); 
}
Handle_SMESH_TypeFilter::~Handle_SMESH_TypeFilter() {}

