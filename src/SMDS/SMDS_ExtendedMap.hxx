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
//  File   : SMDS_ExtendedMap.hxx
//  Module : SMESH

#ifndef _SMDS_ExtendedMap_HeaderFile
#define _SMDS_ExtendedMap_HeaderFile

#ifndef _TCollection_BasicMap_HeaderFile
#include <TCollection_BasicMap.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Handle_SMDS_StdMapNodeOfExtendedMap_HeaderFile
#include "Handle_SMDS_StdMapNodeOfExtendedMap.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_DomainError;
class SMDS_MeshElement;
class SMDS_MeshElementMapHasher;
class SMDS_StdMapNodeOfExtendedMap;
class SMDS_MapIteratorOfExtendedMap;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDS_ExtendedMap  : public TCollection_BasicMap {

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
Standard_EXPORT SMDS_ExtendedMap(const Standard_Integer NbBuckets = 1);
Standard_EXPORT   SMDS_ExtendedMap& Assign(const SMDS_ExtendedMap& Other) ;
  SMDS_ExtendedMap& operator =(const SMDS_ExtendedMap& Other) 
{
  return Assign(Other);
}

Standard_EXPORT   void ReSize(const Standard_Integer NbBuckets) ;
Standard_EXPORT   void Clear() ;
~SMDS_ExtendedMap()
{
  Clear();
}

Standard_EXPORT   Standard_Boolean Add(const Handle(SMDS_MeshElement)& aKey) ;
Standard_EXPORT   Standard_Boolean Contains(const Handle(SMDS_MeshElement)& aKey) const;
Standard_EXPORT   Standard_Boolean Remove(const Handle(SMDS_MeshElement)& aKey) ;





protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMDS_ExtendedMap(const SMDS_ExtendedMap& Other);


 // Fields PRIVATE
 //


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
