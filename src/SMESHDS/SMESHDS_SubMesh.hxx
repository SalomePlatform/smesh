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
//  File   : SMESHDS_SubMesh.hxx
//  Module : SMESH

#ifndef _SMESHDS_SubMesh_HeaderFile
#define _SMESHDS_SubMesh_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_SubMesh_HeaderFile
#include "Handle_SMESHDS_SubMesh.hxx"
#endif

#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _SMDS_MapOfMeshElement_HeaderFile
#include "SMDS_MapOfMeshElement.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _TColStd_ListOfInteger_HeaderFile
#include <TColStd_ListOfInteger.hxx>
#endif
#ifndef _MMgt_TShared_HeaderFile
#include <MMgt_TShared.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Handle_SMDS_MeshNode_HeaderFile
#include "Handle_SMDS_MeshNode.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
class SMDS_Mesh;
class SMDS_MeshElement;
class SMDS_MeshNode;
class SMDS_MapOfMeshElement;
class TColStd_ListOfInteger;


class SMESHDS_SubMesh : public MMgt_TShared {

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
Standard_EXPORT SMESHDS_SubMesh(const Handle(SMDS_Mesh)& M);
Standard_EXPORT   void AddElement(const Handle(SMDS_MeshElement)& ME) ;
Standard_EXPORT   void RemoveElement(const Handle(SMDS_MeshElement)& ME) ;
Standard_EXPORT   void AddNode(const Handle(SMDS_MeshNode)& ME) ;
Standard_EXPORT   void RemoveNode(const Handle(SMDS_MeshNode)& ME) ;
Standard_EXPORT   Standard_Integer NbElements() ;
Standard_EXPORT  const SMDS_MapOfMeshElement& GetElements() ;
Standard_EXPORT  const TColStd_ListOfInteger& GetIDElements() ;
Standard_EXPORT   Standard_Integer NbNodes() ;
Standard_EXPORT  const SMDS_MapOfMeshElement& GetNodes() ;
Standard_EXPORT  const TColStd_ListOfInteger& GetIDNodes() ;
Standard_EXPORT ~SMESHDS_SubMesh();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_SubMesh_Type_();
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
Handle_SMDS_Mesh myMesh;
SMDS_MapOfMeshElement myElements;
SMDS_MapOfMeshElement myNodes;
Standard_Boolean myListOfEltIDIsUpdate;
TColStd_ListOfInteger myListOfEltID;
Standard_Boolean myListOfNodeIDIsUpdate;
TColStd_ListOfInteger myListOfNodeID;


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
