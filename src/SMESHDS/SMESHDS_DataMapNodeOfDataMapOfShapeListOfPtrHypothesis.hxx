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
//  File   : SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx
//  Module : SMESH

#ifndef _SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_HeaderFile
#define _SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_HeaderFile
#include "Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx"
#endif

#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _SMESHDS_ListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListOfPtrHypothesis.hxx"
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class TopoDS_Shape;
class SMESHDS_ListOfPtrHypothesis;
class TopTools_ShapeMapHasher;
class SMESHDS_DataMapOfShapeListOfPtrHypothesis;
class SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis;


class SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis : public TCollection_MapNode {

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
Standard_EXPORT inline SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis(const TopoDS_Shape& K,const SMESHDS_ListOfPtrHypothesis& I,const TCollection_MapNodePtr& n);
Standard_EXPORT inline   TopoDS_Shape& Key() const;
Standard_EXPORT inline   SMESHDS_ListOfPtrHypothesis& Value() const;
Standard_EXPORT ~SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_Type_();
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
SMESHDS_ListOfPtrHypothesis myValue;


};

#define TheKey TopoDS_Shape
#define TheKey_hxx <TopoDS_Shape.hxx>
#define TheItem SMESHDS_ListOfPtrHypothesis
#define TheItem_hxx <SMESHDS_ListOfPtrHypothesis.hxx>
#define Hasher TopTools_ShapeMapHasher
#define Hasher_hxx <TopTools_ShapeMapHasher.hxx>
#define TCollection_DataMapNode SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapNode_hxx <SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx>
#define TCollection_DataMapIterator SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapIterator_hxx <SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis.hxx>
#define Handle_TCollection_DataMapNode Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapNode_Type_() SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_Type_()
#define TCollection_DataMap SMESHDS_DataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMap_hxx <SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx>

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
