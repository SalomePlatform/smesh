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
//  File   : SMDS_MeshGroup.hxx
//  Module : SMESH

#ifndef _SMDS_MeshGroup_HeaderFile
#define _SMDS_MeshGroup_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_MeshGroup_HeaderFile
#include "Handle_SMDS_MeshGroup.hxx"
#endif

#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _SMDSAbs_ElementType_HeaderFile
#include "SMDSAbs_ElementType.hxx"
#endif
#ifndef _SMDS_MapOfMeshElement_HeaderFile
#include "SMDS_MapOfMeshElement.hxx"
#endif
#ifndef _Handle_SMDS_MeshGroup_HeaderFile
#include "Handle_SMDS_MeshGroup.hxx"
#endif
#ifndef _SMDS_ListOfMeshGroup_HeaderFile
#include "SMDS_ListOfMeshGroup.hxx"
#endif
#ifndef _SMDS_MeshObject_HeaderFile
#include "SMDS_MeshObject.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
class SMDS_Mesh;
class Standard_NoSuchObject;
class Standard_TypeMismatch;
class SMDS_MeshElement;
class SMDS_MapOfMeshElement;


class SMDS_MeshGroup : public SMDS_MeshObject {

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
Standard_EXPORT SMDS_MeshGroup(const Handle(SMDS_Mesh)& aMesh);
Standard_EXPORT   Handle_SMDS_MeshGroup AddSubGroup() ;
Standard_EXPORT virtual  Standard_Boolean RemoveSubGroup(const Handle(SMDS_MeshGroup)& aGroup) ;
Standard_EXPORT virtual  Standard_Boolean RemoveFromParent() ;
Standard_EXPORT   void Clear() ;
Standard_EXPORT   void Add(const Handle(SMDS_MeshElement)& ME) ;
Standard_EXPORT   void Remove(const Handle(SMDS_MeshElement)& ME) ;
Standard_EXPORT   Standard_Boolean IsEmpty() const;
Standard_EXPORT   Standard_Integer Extent() const;
Standard_EXPORT   SMDSAbs_ElementType Type() const;
Standard_EXPORT   Standard_Boolean Contains(const Handle(SMDS_MeshElement)& ME) const;
Standard_EXPORT inline  const SMDS_MapOfMeshElement& Elements() const;
Standard_EXPORT ~SMDS_MeshGroup();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_MeshGroup_Type_();
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
Standard_EXPORT SMDS_MeshGroup(const Handle(SMDS_MeshGroup)& parent);


 // Fields PRIVATE
 //
Handle_SMDS_Mesh myMesh;
SMDSAbs_ElementType myType;
SMDS_MapOfMeshElement myElements;
Handle_SMDS_MeshGroup myParent;
SMDS_ListOfMeshGroup myChildren;


};


#include "SMDS_MeshGroup.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
