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
//  File   : SMDS_MeshElementsIterator.hxx
//  Module : SMESH

#ifndef _SMDS_MeshElementsIterator_HeaderFile
#define _SMDS_MeshElementsIterator_HeaderFile

#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _SMDS_MapIteratorOfExtendedOrientedMap_HeaderFile
#include "SMDS_MapIteratorOfExtendedOrientedMap.hxx"
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class SMDS_MeshElement;
class Standard_NoMoreObject;
class Standard_NoSuchObject;
class SMDS_Mesh;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDS_MeshElementsIterator  {

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
Standard_EXPORT virtual  void Delete() ;
Standard_EXPORT virtual ~SMDS_MeshElementsIterator(){Delete();}
Standard_EXPORT virtual  void Initialize(const Handle(SMDS_Mesh)& M)  = 0;
Standard_EXPORT inline   Standard_Boolean More() const;
Standard_EXPORT   void Next() ;
Standard_EXPORT inline  const Handle_SMDS_MeshElement& Value() const;





protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //
Handle_SMDS_MeshElement myCurrentMeshElement;
SMDS_MapIteratorOfExtendedOrientedMap myMapIterator;


private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //


};


#include "SMDS_MeshElementsIterator.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
