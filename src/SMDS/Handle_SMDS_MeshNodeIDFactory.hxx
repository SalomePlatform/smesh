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
//  File   : Handle_SMDS_MeshNodeIDFactory.hxx
//  Module : SMESH

#ifndef _Handle_SMDS_MeshNodeIDFactory_HeaderFile
#define _Handle_SMDS_MeshNodeIDFactory_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_SMDS_MeshIDFactory_HeaderFile
#include "Handle_SMDS_MeshIDFactory.hxx"
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(SMDS_MeshIDFactory);
class SMDS_MeshNodeIDFactory;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDS_MeshNodeIDFactory);

class Handle(SMDS_MeshNodeIDFactory) : public Handle(SMDS_MeshIDFactory) {
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
    Handle(SMDS_MeshNodeIDFactory)():Handle(SMDS_MeshIDFactory)() {} 
    Handle(SMDS_MeshNodeIDFactory)(const Handle(SMDS_MeshNodeIDFactory)& aHandle) : Handle(SMDS_MeshIDFactory)(aHandle) 
     {
     }

    Handle(SMDS_MeshNodeIDFactory)(const SMDS_MeshNodeIDFactory* anItem) : Handle(SMDS_MeshIDFactory)((SMDS_MeshIDFactory *)anItem) 
     {
     }

    Handle(SMDS_MeshNodeIDFactory)& operator=(const Handle(SMDS_MeshNodeIDFactory)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDS_MeshNodeIDFactory)& operator=(const SMDS_MeshNodeIDFactory* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDS_MeshNodeIDFactory* operator->() 
     {
      return (SMDS_MeshNodeIDFactory *)ControlAccess();
     }

    SMDS_MeshNodeIDFactory* operator->() const 
     {
      return (SMDS_MeshNodeIDFactory *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDS_MeshNodeIDFactory)();
 
   Standard_EXPORT static const Handle(SMDS_MeshNodeIDFactory) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
