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
//  File   : Handle_SMDS_ListNodeOfListOfMeshGroup.hxx
//  Module : SMESH

#ifndef _Handle_SMDS_ListNodeOfListOfMeshGroup_HeaderFile
#define _Handle_SMDS_ListNodeOfListOfMeshGroup_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_TCollection_MapNode_HeaderFile
#include <Handle_TCollection_MapNode.hxx>
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(TCollection_MapNode);
class SMDS_ListNodeOfListOfMeshGroup;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDS_ListNodeOfListOfMeshGroup);

class Handle(SMDS_ListNodeOfListOfMeshGroup) : public Handle(TCollection_MapNode) {
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
    Handle(SMDS_ListNodeOfListOfMeshGroup)():Handle(TCollection_MapNode)() {} 
    Handle(SMDS_ListNodeOfListOfMeshGroup)(const Handle(SMDS_ListNodeOfListOfMeshGroup)& aHandle) : Handle(TCollection_MapNode)(aHandle) 
     {
     }

    Handle(SMDS_ListNodeOfListOfMeshGroup)(const SMDS_ListNodeOfListOfMeshGroup* anItem) : Handle(TCollection_MapNode)((TCollection_MapNode *)anItem) 
     {
     }

    Handle(SMDS_ListNodeOfListOfMeshGroup)& operator=(const Handle(SMDS_ListNodeOfListOfMeshGroup)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDS_ListNodeOfListOfMeshGroup)& operator=(const SMDS_ListNodeOfListOfMeshGroup* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDS_ListNodeOfListOfMeshGroup* operator->() 
     {
      return (SMDS_ListNodeOfListOfMeshGroup *)ControlAccess();
     }

    SMDS_ListNodeOfListOfMeshGroup* operator->() const 
     {
      return (SMDS_ListNodeOfListOfMeshGroup *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDS_ListNodeOfListOfMeshGroup)();
 
   Standard_EXPORT static const Handle(SMDS_ListNodeOfListOfMeshGroup) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
