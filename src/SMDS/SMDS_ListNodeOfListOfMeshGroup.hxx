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
//  File   : SMDS_ListNodeOfListOfMeshGroup.hxx
//  Module : SMESH

#ifndef _SMDS_ListNodeOfListOfMeshGroup_HeaderFile
#define _SMDS_ListNodeOfListOfMeshGroup_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_ListNodeOfListOfMeshGroup_HeaderFile
#include "Handle_SMDS_ListNodeOfListOfMeshGroup.hxx"
#endif

#ifndef _Handle_SMDS_MeshGroup_HeaderFile
#include "Handle_SMDS_MeshGroup.hxx"
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class SMDS_MeshGroup;
class SMDS_ListOfMeshGroup;
class SMDS_ListIteratorOfListOfMeshGroup;


class SMDS_ListNodeOfListOfMeshGroup : public TCollection_MapNode {

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
Standard_EXPORT inline SMDS_ListNodeOfListOfMeshGroup(const Handle(SMDS_MeshGroup)& I,const TCollection_MapNodePtr& n);
Standard_EXPORT inline   Handle_SMDS_MeshGroup& Value() const;
Standard_EXPORT ~SMDS_ListNodeOfListOfMeshGroup();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_ListNodeOfListOfMeshGroup_Type_();
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
Handle_SMDS_MeshGroup myValue;


};

#define Item Handle_SMDS_MeshGroup
#define Item_hxx "SMDS_MeshGroup.hxx"
#define TCollection_ListNode SMDS_ListNodeOfListOfMeshGroup
#define TCollection_ListNode_hxx "SMDS_ListNodeOfListOfMeshGroup.hxx"
#define TCollection_ListIterator SMDS_ListIteratorOfListOfMeshGroup
#define TCollection_ListIterator_hxx "SMDS_ListIteratorOfListOfMeshGroup.hxx"
#define Handle_TCollection_ListNode Handle_SMDS_ListNodeOfListOfMeshGroup
#define TCollection_ListNode_Type_() SMDS_ListNodeOfListOfMeshGroup_Type_()
#define TCollection_List SMDS_ListOfMeshGroup
#define TCollection_List_hxx "SMDS_ListOfMeshGroup.hxx"

#include <TCollection_ListNode.lxx>

#undef Item
#undef Item_hxx
#undef TCollection_ListNode
#undef TCollection_ListNode_hxx
#undef TCollection_ListIterator
#undef TCollection_ListIterator_hxx
#undef Handle_TCollection_ListNode
#undef TCollection_ListNode_Type_
#undef TCollection_List
#undef TCollection_List_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
