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
//  File   : SMDSControl.hxx
//  Module : SMESH

#ifndef _SMDSControl_HeaderFile
#define _SMDSControl_HeaderFile

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
class SMDS_Mesh;
class SMDS_MeshElement;
class SMDSControl_MeshBoundary;
class SMDSControl_BoundaryEdges;
class SMDSControl_BoundaryFaces;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDSControl  {

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
Standard_EXPORT static  Standard_Integer ComputeNeighborFaces(const Handle(SMDS_Mesh)& M,const Handle(SMDS_MeshElement)& ME,const Standard_Integer idnode1,const Standard_Integer idnode2) ;
Standard_EXPORT static  Standard_Integer ComputeNeighborVolumes(const Handle(SMDS_Mesh)& M,const Handle(SMDS_MeshElement)& ME,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) ;
Standard_EXPORT static  Standard_Integer ComputeNeighborVolumes(const Handle(SMDS_Mesh)& M,const Handle(SMDS_MeshElement)& ME,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;





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

friend class SMDSControl_MeshBoundary;
friend class SMDSControl_BoundaryEdges;
friend class SMDSControl_BoundaryFaces;

};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
