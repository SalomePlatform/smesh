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
//  File   : SMDS_MapOfMeshElement.hxx
//  Module : SMESH

#ifndef _SMDS_MapOfMeshElement_HeaderFile
#define _SMDS_MapOfMeshElement_HeaderFile

#ifndef _SMDS_ExtendedMap_HeaderFile
#include "SMDS_ExtendedMap.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoSuchObject;
class SMDS_MeshElement;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDS_MapOfMeshElement  : public SMDS_ExtendedMap {

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
Standard_EXPORT inline SMDS_MapOfMeshElement(const Standard_Integer NbBuckets = 1);
Standard_EXPORT  const Handle_SMDS_MeshElement& Find(const Handle(SMDS_MeshElement)& K) const;
 const Handle_SMDS_MeshElement& operator()(const Handle(SMDS_MeshElement)& K) const
{
  return Find(K);
}

Standard_EXPORT   Handle_SMDS_MeshElement& ChangeFind(const Handle(SMDS_MeshElement)& K) ;
  Handle_SMDS_MeshElement& operator()(const Handle(SMDS_MeshElement)& K) 
{
  return ChangeFind(K);
}

Standard_EXPORT  const Handle_SMDS_MeshElement& FindID(const Standard_Integer ID) const;
 const Handle_SMDS_MeshElement& operator()(const Standard_Integer ID) const
{
  return FindID(ID);
}

Standard_EXPORT   Standard_Boolean ContainsID(const Standard_Integer ID) const;





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


};


#include "SMDS_MapOfMeshElement.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
