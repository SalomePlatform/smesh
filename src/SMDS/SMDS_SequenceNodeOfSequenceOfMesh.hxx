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
//  File   : SMDS_SequenceNodeOfSequenceOfMesh.hxx
//  Module : SMESH

#ifndef _SMDS_SequenceNodeOfSequenceOfMesh_HeaderFile
#define _SMDS_SequenceNodeOfSequenceOfMesh_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_SequenceNodeOfSequenceOfMesh_HeaderFile
#include "Handle_SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#endif

#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _TCollection_SeqNode_HeaderFile
#include <TCollection_SeqNode.hxx>
#endif
#ifndef _TCollection_SeqNodePtr_HeaderFile
#include <TCollection_SeqNodePtr.hxx>
#endif
class SMDS_Mesh;
class SMDS_SequenceOfMesh;


class SMDS_SequenceNodeOfSequenceOfMesh : public TCollection_SeqNode {

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
Standard_EXPORT inline SMDS_SequenceNodeOfSequenceOfMesh(const Handle(SMDS_Mesh)& I,const TCollection_SeqNodePtr& n,const TCollection_SeqNodePtr& p);
Standard_EXPORT inline   Handle_SMDS_Mesh& Value() const;
Standard_EXPORT ~SMDS_SequenceNodeOfSequenceOfMesh();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_SequenceNodeOfSequenceOfMesh_Type_();
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
Handle_SMDS_Mesh myValue;


};

#define SeqItem Handle_SMDS_Mesh
#define SeqItem_hxx "SMDS_Mesh.hxx"
#define TCollection_SequenceNode SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_hxx "SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#define Handle_TCollection_SequenceNode Handle_SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_Type_() SMDS_SequenceNodeOfSequenceOfMesh_Type_()
#define TCollection_Sequence SMDS_SequenceOfMesh
#define TCollection_Sequence_hxx "SMDS_SequenceOfMesh.hxx"

#include <TCollection_SequenceNode.lxx>

#undef SeqItem
#undef SeqItem_hxx
#undef TCollection_SequenceNode
#undef TCollection_SequenceNode_hxx
#undef Handle_TCollection_SequenceNode
#undef TCollection_SequenceNode_Type_
#undef TCollection_Sequence
#undef TCollection_Sequence_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
