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
//  File   : SMDS_SequenceOfMesh.hxx
//  Module : SMESH

#ifndef _SMDS_SequenceOfMesh_HeaderFile
#define _SMDS_SequenceOfMesh_HeaderFile

#ifndef _TCollection_BaseSequence_HeaderFile
#include <TCollection_BaseSequence.hxx>
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _Handle_SMDS_SequenceNodeOfSequenceOfMesh_HeaderFile
#include "Handle_SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
class Standard_NoSuchObject;
class Standard_OutOfRange;
class SMDS_Mesh;
class SMDS_SequenceNodeOfSequenceOfMesh;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDS_SequenceOfMesh  : public TCollection_BaseSequence {

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
Standard_EXPORT inline SMDS_SequenceOfMesh();
Standard_EXPORT   void Clear() ;
~SMDS_SequenceOfMesh()
{
  Clear();
}

Standard_EXPORT  const SMDS_SequenceOfMesh& Assign(const SMDS_SequenceOfMesh& Other) ;
 const SMDS_SequenceOfMesh& operator =(const SMDS_SequenceOfMesh& Other) 
{
  return Assign(Other);
}

Standard_EXPORT   void Append(const Handle(SMDS_Mesh)& T) ;
Standard_EXPORT inline   void Append(SMDS_SequenceOfMesh& S) ;
Standard_EXPORT   void Prepend(const Handle(SMDS_Mesh)& T) ;
Standard_EXPORT inline   void Prepend(SMDS_SequenceOfMesh& S) ;
Standard_EXPORT inline   void InsertBefore(const Standard_Integer Index,const Handle(SMDS_Mesh)& T) ;
Standard_EXPORT inline   void InsertBefore(const Standard_Integer Index,SMDS_SequenceOfMesh& S) ;
Standard_EXPORT   void InsertAfter(const Standard_Integer Index,const Handle(SMDS_Mesh)& T) ;
Standard_EXPORT inline   void InsertAfter(const Standard_Integer Index,SMDS_SequenceOfMesh& S) ;
Standard_EXPORT  const Handle_SMDS_Mesh& First() const;
Standard_EXPORT  const Handle_SMDS_Mesh& Last() const;
Standard_EXPORT inline   void Split(const Standard_Integer Index,SMDS_SequenceOfMesh& Sub) ;
Standard_EXPORT  const Handle_SMDS_Mesh& Value(const Standard_Integer Index) const;
 const Handle_SMDS_Mesh& operator()(const Standard_Integer Index) const
{
  return Value(Index);
}

Standard_EXPORT   void SetValue(const Standard_Integer Index,const Handle(SMDS_Mesh)& I) ;
Standard_EXPORT   Handle_SMDS_Mesh& ChangeValue(const Standard_Integer Index) ;
  Handle_SMDS_Mesh& operator()(const Standard_Integer Index) 
{
  return ChangeValue(Index);
}

Standard_EXPORT   void Remove(const Standard_Integer Index) ;
Standard_EXPORT   void Remove(const Standard_Integer FromIndex,const Standard_Integer ToIndex) ;





protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMDS_SequenceOfMesh(const SMDS_SequenceOfMesh& Other);


 // Fields PRIVATE
 //


};

#define SeqItem Handle_SMDS_Mesh
#define SeqItem_hxx "SMDS_Mesh.hxx"
#define TCollection_SequenceNode SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_hxx "SMDS_SequenceNodeOfSequenceOfMesh.hxx"
#define Handle_TCollection_SequenceNode Handle_SMDS_SequenceNodeOfSequenceOfMesh
#define TCollection_SequenceNode_Type_() SMDS_SequenceNodeOfSequenceOfMesh_Type_()
#define TCollection_Sequence SMDS_SequenceOfMesh
#define TCollection_Sequence_hxx "SMDS_SequenceOfMesh.hxx"

#include <TCollection_Sequence.lxx>

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
