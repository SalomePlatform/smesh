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
//  File   : SMDSControl_BoundaryEdges.hxx
//  Module : SMESH

#ifndef _SMDSControl_BoundaryEdges_HeaderFile
#define _SMDSControl_BoundaryEdges_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDSControl_BoundaryEdges_HeaderFile
#include "Handle_SMDSControl_BoundaryEdges.hxx"
#endif

#ifndef _SMDS_MapOfMeshElement_HeaderFile
#include "SMDS_MapOfMeshElement.hxx"
#endif
#ifndef _SMDSControl_MeshBoundary_HeaderFile
#include "SMDSControl_MeshBoundary.hxx"
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
class SMDS_Mesh;


class SMDSControl_BoundaryEdges : public SMDSControl_MeshBoundary {

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
Standard_EXPORT SMDSControl_BoundaryEdges(const Handle(SMDS_Mesh)& M);
Standard_EXPORT virtual  void Compute() ;
Standard_EXPORT ~SMDSControl_BoundaryEdges();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDSControl_BoundaryEdges_Type_();
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
SMDS_MapOfMeshElement myBoundaryEdges;


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
