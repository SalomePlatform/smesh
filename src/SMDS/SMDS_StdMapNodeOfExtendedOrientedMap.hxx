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
//  File   : SMDS_StdMapNodeOfExtendedOrientedMap.hxx
//  Module : SMESH

#ifndef _SMDS_StdMapNodeOfExtendedOrientedMap_HeaderFile
#define _SMDS_StdMapNodeOfExtendedOrientedMap_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_StdMapNodeOfExtendedOrientedMap_HeaderFile
#include "Handle_SMDS_StdMapNodeOfExtendedOrientedMap.hxx"
#endif

#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class SMDS_MeshElement;
class SMDS_MeshOrientedElementMapHasher;
class SMDS_ExtendedOrientedMap;
class SMDS_MapIteratorOfExtendedOrientedMap;


class SMDS_StdMapNodeOfExtendedOrientedMap : public TCollection_MapNode {

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
Standard_EXPORT inline SMDS_StdMapNodeOfExtendedOrientedMap(const Handle(SMDS_MeshElement)& K,const TCollection_MapNodePtr& n);
Standard_EXPORT inline   Handle_SMDS_MeshElement& Key() const;
Standard_EXPORT ~SMDS_StdMapNodeOfExtendedOrientedMap();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_StdMapNodeOfExtendedOrientedMap_Type_();
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
Handle_SMDS_MeshElement myKey;


};

#define TheKey Handle_SMDS_MeshElement
#define TheKey_hxx <SMDS_MeshElement.hxx>
#define Hasher SMDS_MeshOrientedElementMapHasher
#define Hasher_hxx <SMDS_MeshOrientedElementMapHasher.hxx>
#define TCollection_StdMapNode SMDS_StdMapNodeOfExtendedOrientedMap
#define TCollection_StdMapNode_hxx <SMDS_StdMapNodeOfExtendedOrientedMap.hxx>
#define TCollection_MapIterator SMDS_MapIteratorOfExtendedOrientedMap
#define TCollection_MapIterator_hxx <SMDS_MapIteratorOfExtendedOrientedMap.hxx>
#define Handle_TCollection_StdMapNode Handle_SMDS_StdMapNodeOfExtendedOrientedMap
#define TCollection_StdMapNode_Type_() SMDS_StdMapNodeOfExtendedOrientedMap_Type_()
#define TCollection_Map SMDS_ExtendedOrientedMap
#define TCollection_Map_hxx <SMDS_ExtendedOrientedMap.hxx>

#include <TCollection_StdMapNode.lxx>

#undef TheKey
#undef TheKey_hxx
#undef Hasher
#undef Hasher_hxx
#undef TCollection_StdMapNode
#undef TCollection_StdMapNode_hxx
#undef TCollection_MapIterator
#undef TCollection_MapIterator_hxx
#undef Handle_TCollection_StdMapNode
#undef TCollection_StdMapNode_Type_
#undef TCollection_Map
#undef TCollection_Map_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
