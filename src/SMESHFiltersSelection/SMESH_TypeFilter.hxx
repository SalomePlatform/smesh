//  SMESH SMESHFiltersSelection : filter selector for viewer
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
//  File   : SMESH_TypeFilter.hxx
//  Module : SMESH

#ifndef _SMESH_TypeFilter_HeaderFile
#define _SMESH_TypeFilter_HeaderFile

#ifndef _Handle_SMESH_TypeFilter_HeaderFile
#include "Handle_SMESH_TypeFilter.hxx"
#endif

#include "SALOME_InteractiveObject.hxx"
#include "SALOME_Filter.hxx"
#include "SMESH_Type.h"

// Open CASCADE Includes
#include <Standard.hxx>

class SMESH_TypeFilter : public SALOME_Filter {

public:

    inline void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    inline void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    inline void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }
//    inline void  operator delete(void *anAddress, size_t size) 
//      { 
//        if (anAddress) Standard::Free((Standard_Address&)anAddress,size); 
//      }
 // Methods PUBLIC
 // 
Standard_EXPORT SMESH_TypeFilter(MeshObjectType aType);
Standard_EXPORT virtual  Standard_Boolean IsOk(const Handle(SALOME_InteractiveObject)& anobj) const;
Standard_EXPORT ~SMESH_TypeFilter();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESH_TypeFilter_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	      IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //
MeshObjectType myKind;

private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
