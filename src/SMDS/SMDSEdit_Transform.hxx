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
//  File   : SMDSEdit_Transform.hxx
//  Module : SMESH

#ifndef _SMDSEdit_Transform_HeaderFile
#define _SMDSEdit_Transform_HeaderFile

#ifndef _Handle_SMDS_Mesh_HeaderFile
#include "Handle_SMDS_Mesh.hxx"
#endif
#ifndef _gp_Trsf_HeaderFile
#include <gp_Trsf.hxx>
#endif
#ifndef _SMDS_ListOfMeshElement_HeaderFile
#include "SMDS_ListOfMeshElement.hxx"
#endif
class SMDS_Mesh;
class gp_Trsf;
class SMDS_ListOfMeshElement;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDSEdit_Transform  {

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
Standard_EXPORT SMDSEdit_Transform(const Handle(SMDS_Mesh)& aMesh,const gp_Trsf& aTrsf);
Standard_EXPORT SMDSEdit_Transform(const Handle(SMDS_Mesh)& aMesh,const SMDS_ListOfMeshElement& aListOfME,const gp_Trsf& aTrsf);
Standard_EXPORT   void Perform() ;
Standard_EXPORT   void SetTrsf(const gp_Trsf& aTrsf) ;
Standard_EXPORT   gp_Trsf GetTrsf() const;





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
gp_Trsf myTrsf;
SMDS_ListOfMeshElement myListOfME;


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
