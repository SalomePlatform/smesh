//  SMESH SMDS : implementaion of Salome mesh data structure
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
//  File   : SMDS_MeshTriangle.hxx
//  Module : SMESH

#ifndef _SMDS_MeshTriangle_HeaderFile
#define _SMDS_MeshTriangle_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_MeshTriangle_HeaderFile
#include "Handle_SMDS_MeshTriangle.hxx"
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _SMDS_MeshFace_HeaderFile
#include "SMDS_MeshFace.hxx"
#endif
#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif


class SMDS_MeshTriangle : public SMDS_MeshFace {

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
Standard_EXPORT SMDS_MeshTriangle(const Standard_Integer ID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3);
Standard_EXPORT inline   void ComputeKey() ;
Standard_EXPORT inline   void GetEdgeDefinedByNodes(const Standard_Integer rank,Standard_Integer& idnode1,Standard_Integer& idnode2) const;
Standard_EXPORT inline   Standard_Address GetConnections() const;
Standard_EXPORT inline   Standard_Integer GetConnection(const Standard_Integer rank) const;
Standard_EXPORT ~SMDS_MeshTriangle();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_MeshTriangle_Type_();
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
Standard_EXPORT   void SetConnections(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) ;


 // Fields PRIVATE
 //
Standard_Integer myNodes[3];


};


#include "SMDS_MeshTriangle.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
