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
//  File   : SMDS_MeshElement.hxx
//  Module : SMESH

#ifndef _SMDS_MeshElement_HeaderFile
#define _SMDS_MeshElement_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _SMDSAbs_ElementType_HeaderFile                                                                           
#include "SMDSAbs_ElementType.hxx"
#endif
#ifndef _Handle_TColgp_HArray1OfDir_HeaderFile
#include "Handle_TColgp_HArray1OfDir.hxx"
#endif
#ifndef _SMDS_MeshObject_HeaderFile
#include "SMDS_MeshObject.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _Standard_OStream_HeaderFile
#include <Standard_OStream.hxx>
#endif
class TColgp_HArray1OfDir;
class Standard_OutOfRange;
class SMDS_ListOfMeshElement;
class gp_Vec;
class gp_Dir;


class SMDS_MeshElement : public SMDS_MeshObject {

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
Standard_EXPORT inline   Standard_Integer HashCode(const Standard_Integer Upper) const;
Standard_EXPORT inline   Standard_Boolean IsSame(const Handle(SMDS_MeshElement)& other) const;
Standard_EXPORT inline   Standard_Boolean IsEqual(const Handle(SMDS_MeshElement)& other) const;
Standard_EXPORT inline   Standard_Boolean IsNodeInElement(const Standard_Integer idnode) const;
Standard_EXPORT virtual  void GetEdgeDefinedByNodes(const Standard_Integer rank,Standard_Integer& idnode1,Standard_Integer& idnode2) const;
Standard_EXPORT virtual  void GetFaceDefinedByNodes(const Standard_Integer rank,const Standard_Address idnode,Standard_Integer& nb) const;
Standard_EXPORT virtual  Standard_Integer GetConnection(const Standard_Integer rank) const;
Standard_EXPORT virtual  void AddInverseElement(const Handle(SMDS_MeshElement)& ME) ;
Standard_EXPORT virtual const SMDS_ListOfMeshElement& InverseElements() const;
Standard_EXPORT virtual  void ClearInverseElements() ;
Standard_EXPORT inline   Standard_Integer NbNodes() const;
Standard_EXPORT virtual  Standard_Integer NbEdges() const;
Standard_EXPORT virtual  Standard_Integer NbFaces() const;
Standard_EXPORT inline   Standard_Integer GetID() const;
Standard_EXPORT virtual  Standard_Integer GetKey() const = 0;
Standard_EXPORT virtual  void SetNormal(const Standard_Integer rank,const Standard_Real vx,const Standard_Real vy,const Standard_Real vz) ;
Standard_EXPORT virtual  void SetNormal(const Standard_Integer rank,const gp_Vec& v) ;
Standard_EXPORT virtual  gp_Dir GetNormal(const Standard_Integer rank) ;
Standard_EXPORT virtual  void Print(Standard_OStream& OS) const;
friend Standard_EXPORT Standard_OStream& operator << (Standard_OStream& OS, const Handle(SMDS_MeshElement)& );
Standard_EXPORT inline   SMDSAbs_ElementType GetType() const;
Standard_EXPORT ~SMDS_MeshElement();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_MeshElement_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	       IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 
Standard_EXPORT SMDS_MeshElement(const Standard_Integer ID,const Standard_Integer NbConnections,const SMDSAbs_ElementType Type);
Standard_EXPORT virtual  Standard_Address GetConnections() const;


 // Fields PROTECTED
 //
Standard_Integer myID;
Standard_Integer myNbNodes;
Handle_TColgp_HArray1OfDir myNormals;


private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //
SMDSAbs_ElementType myType;


};


#include "SMDS_MeshElement.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
