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
//  File   : Handle_SMDS_Mesh.hxx
//  Module : SMESH

#ifndef _Handle_SMDS_Mesh_HeaderFile
#define _Handle_SMDS_Mesh_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_SMDS_MeshObject_HeaderFile
#include "Handle_SMDS_MeshObject.hxx"
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(SMDS_MeshObject);
class SMDS_Mesh;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDS_Mesh);

class Handle(SMDS_Mesh) : public Handle(SMDS_MeshObject) {
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
    Handle(SMDS_Mesh)():Handle(SMDS_MeshObject)() {} 
    Handle(SMDS_Mesh)(const Handle(SMDS_Mesh)& aHandle) : Handle(SMDS_MeshObject)(aHandle) 
     {
     }

    Handle(SMDS_Mesh)(const SMDS_Mesh* anItem) : Handle(SMDS_MeshObject)((SMDS_MeshObject *)anItem) 
     {
     }

    Handle(SMDS_Mesh)& operator=(const Handle(SMDS_Mesh)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDS_Mesh)& operator=(const SMDS_Mesh* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDS_Mesh* operator->() 
     {
      return (SMDS_Mesh *)ControlAccess();
     }

    SMDS_Mesh* operator->() const 
     {
      return (SMDS_Mesh *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDS_Mesh)();
 
   Standard_EXPORT static const Handle(SMDS_Mesh) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
