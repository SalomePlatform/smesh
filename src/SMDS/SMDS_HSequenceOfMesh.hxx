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
//  File   : SMDS_HSequenceOfMesh.hxx
//  Module : SMESH

#ifndef _SMDS_HSequenceOfMesh_HeaderFile
#define _SMDS_HSequenceOfMesh_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_HSequenceOfMesh_HeaderFile
#include "Handle_SMDS_HSequenceOfMesh.hxx"
#endif

#ifndef _SMDS_SequenceOfMesh_HeaderFile
#include "SMDS_SequenceOfMesh.hxx"
#endif
#ifndef _MMgt_TShared_HeaderFile
#include <MMgt_TShared.hxx>
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
class Standard_NoSuchObject;
class Standard_OutOfRange;
class SMDS_Mesh;
class SMDS_SequenceOfMesh;


class SMDS_HSequenceOfMesh : public MMgt_TShared {

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
Standard_EXPORT inline SMDS_HSequenceOfMesh();
Standard_EXPORT inline   Standard_Boolean IsEmpty() const;
Standard_EXPORT inline   Standard_Integer Length() const;
Standard_EXPORT   void Clear() ;
Standard_EXPORT   void Append(const Handle(SMDS_Mesh)& anItem) ;
Standard_EXPORT   void Append(const Handle(SMDS_HSequenceOfMesh)& aSequence) ;
Standard_EXPORT   void Prepend(const Handle(SMDS_Mesh)& anItem) ;
Standard_EXPORT   void Prepend(const Handle(SMDS_HSequenceOfMesh)& aSequence) ;
Standard_EXPORT   void Reverse() ;
Standard_EXPORT   void InsertBefore(const Standard_Integer anIndex,const Handle(SMDS_Mesh)& anItem) ;
Standard_EXPORT   void InsertBefore(const Standard_Integer anIndex,const Handle(SMDS_HSequenceOfMesh)& aSequence) ;
Standard_EXPORT   void InsertAfter(const Standard_Integer anIndex,const Handle(SMDS_Mesh)& anItem) ;
Standard_EXPORT   void InsertAfter(const Standard_Integer anIndex,const Handle(SMDS_HSequenceOfMesh)& aSequence) ;
Standard_EXPORT   void Exchange(const Standard_Integer anIndex,const Standard_Integer anOtherIndex) ;
Standard_EXPORT   Handle_SMDS_HSequenceOfMesh Split(const Standard_Integer anIndex) ;
Standard_EXPORT   void SetValue(const Standard_Integer anIndex,const Handle(SMDS_Mesh)& anItem) ;
Standard_EXPORT  const Handle_SMDS_Mesh& Value(const Standard_Integer anIndex) const;
Standard_EXPORT   Handle_SMDS_Mesh& ChangeValue(const Standard_Integer anIndex) ;
Standard_EXPORT   void Remove(const Standard_Integer anIndex) ;
Standard_EXPORT   void Remove(const Standard_Integer fromIndex,const Standard_Integer toIndex) ;
Standard_EXPORT inline  const SMDS_SequenceOfMesh& Sequence() const;
Standard_EXPORT inline   SMDS_SequenceOfMesh& ChangeSequence() ;
Standard_EXPORT   Handle_SMDS_HSequenceOfMesh ShallowCopy() const;
Standard_EXPORT ~SMDS_HSequenceOfMesh();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_HSequenceOfMesh_Type_();
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
SMDS_SequenceOfMesh mySequence;


};

#define Item Handle_SMDS_Mesh
#define Item_hxx "SMDS_Mesh.hxx"
#define TheSequence SMDS_SequenceOfMesh
#define TheSequence_hxx "SMDS_SequenceOfMesh.hxx"
#define TCollection_HSequence SMDS_HSequenceOfMesh
#define TCollection_HSequence_hxx "SMDS_HSequenceOfMesh.hxx"
#define Handle_TCollection_HSequence Handle_SMDS_HSequenceOfMesh
#define TCollection_HSequence_Type_() SMDS_HSequenceOfMesh_Type_()

#include <TCollection_HSequence.lxx>

#undef Item
#undef Item_hxx
#undef TheSequence
#undef TheSequence_hxx
#undef TCollection_HSequence
#undef TCollection_HSequence_hxx
#undef Handle_TCollection_HSequence
#undef TCollection_HSequence_Type_


// other inline functions and methods (like "C++: function call" methods)
//
inline Handle_SMDS_HSequenceOfMesh ShallowCopy(const Handle_SMDS_HSequenceOfMesh& me) {
 return me->ShallowCopy();
}



#endif
