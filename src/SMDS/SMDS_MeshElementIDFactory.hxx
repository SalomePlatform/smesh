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
//  File   : SMDS_MeshElementIDFactory.hxx
//  Module : SMESH

#ifndef _SMDS_MeshElementIDFactory_HeaderFile
#define _SMDS_MeshElementIDFactory_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_MeshElementIDFactory_HeaderFile
#include "Handle_SMDS_MeshElementIDFactory.hxx"
#endif

#ifndef _SMDS_DataMapOfIntegerMeshElement_HeaderFile
#include "SMDS_DataMapOfIntegerMeshElement.hxx"
#endif
#ifndef _SMDS_MeshIDFactory_HeaderFile
#include "SMDS_MeshIDFactory.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
class SMDS_MeshElement;


class SMDS_MeshElementIDFactory : public SMDS_MeshIDFactory {

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
Standard_EXPORT SMDS_MeshElementIDFactory();
Standard_EXPORT inline   Standard_Integer GetFreeID() ;
Standard_EXPORT inline   void ReleaseID(const Standard_Integer ID) ;
Standard_EXPORT inline   Standard_Boolean BindID(const Standard_Integer ID,const Handle(SMDS_MeshElement)& elem) ;
Standard_EXPORT inline   Handle_SMDS_MeshElement MeshElement(const Standard_Integer ID) const;
Standard_EXPORT ~SMDS_MeshElementIDFactory();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_MeshElementIDFactory_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	       IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //
SMDS_DataMapOfIntegerMeshElement myIDElements;


};


#include "SMDS_MeshElementIDFactory.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
