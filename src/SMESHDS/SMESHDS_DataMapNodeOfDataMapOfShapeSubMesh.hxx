//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh.hxx
//  Module : SMESH

#ifndef _SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh_HeaderFile
#define _SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh_HeaderFile
#include "Handle_SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh.hxx"
#endif

#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _Handle_SMESHDS_SubMesh_HeaderFile
#include "Handle_SMESHDS_SubMesh.hxx"
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class SMESHDS_SubMesh;
class TopoDS_Shape;
class TopTools_ShapeMapHasher;
class SMESHDS_DataMapOfShapeSubMesh;
class SMESHDS_DataMapIteratorOfDataMapOfShapeSubMesh;


class SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh : public TCollection_MapNode {

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
Standard_EXPORT inline SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh(const TopoDS_Shape& K,const Handle(SMESHDS_SubMesh)& I,const TCollection_MapNodePtr& n);
Standard_EXPORT inline   TopoDS_Shape& Key() const;
Standard_EXPORT inline   Handle_SMESHDS_SubMesh& Value() const;
Standard_EXPORT ~SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh_Type_();
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
TopoDS_Shape myKey;
Handle_SMESHDS_SubMesh myValue;


};

#define TheKey TopoDS_Shape
#define TheKey_hxx <TopoDS_Shape.hxx>
#define TheItem Handle_SMESHDS_SubMesh
#define TheItem_hxx <SMESHDS_SubMesh.hxx>
#define Hasher TopTools_ShapeMapHasher
#define Hasher_hxx <TopTools_ShapeMapHasher.hxx>
#define TCollection_DataMapNode SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh
#define TCollection_DataMapNode_hxx <SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh.hxx>
#define TCollection_DataMapIterator SMESHDS_DataMapIteratorOfDataMapOfShapeSubMesh
#define TCollection_DataMapIterator_hxx <SMESHDS_DataMapIteratorOfDataMapOfShapeSubMesh.hxx>
#define Handle_TCollection_DataMapNode Handle_SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh
#define TCollection_DataMapNode_Type_() SMESHDS_DataMapNodeOfDataMapOfShapeSubMesh_Type_()
#define TCollection_DataMap SMESHDS_DataMapOfShapeSubMesh
#define TCollection_DataMap_hxx <SMESHDS_DataMapOfShapeSubMesh.hxx>

#include <TCollection_DataMapNode.lxx>

#undef TheKey
#undef TheKey_hxx
#undef TheItem
#undef TheItem_hxx
#undef Hasher
#undef Hasher_hxx
#undef TCollection_DataMapNode
#undef TCollection_DataMapNode_hxx
#undef TCollection_DataMapIterator
#undef TCollection_DataMapIterator_hxx
#undef Handle_TCollection_DataMapNode
#undef TCollection_DataMapNode_Type_
#undef TCollection_DataMap
#undef TCollection_DataMap_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
