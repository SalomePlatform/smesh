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
//  See http://www.opencascade.org or email : webmaster@opencascade.org 
//
//
//
//  File   : Handle_SMDSControl_BoundaryEdges.hxx
//  Module : SMESH

#ifndef _Handle_SMDSControl_BoundaryEdges_HeaderFile
#define _Handle_SMDSControl_BoundaryEdges_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_SMDSControl_MeshBoundary_HeaderFile
#include "Handle_SMDSControl_MeshBoundary.hxx"
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(SMDSControl_MeshBoundary);
class SMDSControl_BoundaryEdges;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDSControl_BoundaryEdges);

class Handle(SMDSControl_BoundaryEdges) : public Handle(SMDSControl_MeshBoundary) {
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
    Handle(SMDSControl_BoundaryEdges)():Handle(SMDSControl_MeshBoundary)() {} 
    Handle(SMDSControl_BoundaryEdges)(const Handle(SMDSControl_BoundaryEdges)& aHandle) : Handle(SMDSControl_MeshBoundary)(aHandle) 
     {
     }

    Handle(SMDSControl_BoundaryEdges)(const SMDSControl_BoundaryEdges* anItem) : Handle(SMDSControl_MeshBoundary)((SMDSControl_MeshBoundary *)anItem) 
     {
     }

    Handle(SMDSControl_BoundaryEdges)& operator=(const Handle(SMDSControl_BoundaryEdges)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDSControl_BoundaryEdges)& operator=(const SMDSControl_BoundaryEdges* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDSControl_BoundaryEdges* operator->() 
     {
      return (SMDSControl_BoundaryEdges *)ControlAccess();
     }

    SMDSControl_BoundaryEdges* operator->() const 
     {
      return (SMDSControl_BoundaryEdges *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDSControl_BoundaryEdges)();
 
   Standard_EXPORT static const Handle(SMDSControl_BoundaryEdges) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
