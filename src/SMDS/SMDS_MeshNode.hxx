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
//  File   : SMDS_MeshNode.hxx
//  Module : SMESH

#ifndef _SMDS_MeshNode_HeaderFile
#define _SMDS_MeshNode_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_MeshNode_HeaderFile
#include "Handle_SMDS_MeshNode.hxx"
#endif

#ifndef _gp_Pnt_HeaderFile
#include <gp_Pnt.hxx>
#endif
#ifndef _SMDS_ListOfMeshElement_HeaderFile
#include "SMDS_ListOfMeshElement.hxx"
#endif
#ifndef _Handle_SMDS_Position_HeaderFile
#include "Handle_SMDS_Position.hxx"
#endif
#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _Standard_OStream_HeaderFile
#include <Standard_OStream.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
class SMDS_Position;
class gp_Pnt;
class SMDS_MeshElement;
class SMDS_ListOfMeshElement;


class SMDS_MeshNode : public SMDS_MeshElement {

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
Standard_EXPORT SMDS_MeshNode(const Standard_Integer ID,const Standard_Real x,const Standard_Real y,const Standard_Real z);
Standard_EXPORT   void Print(Standard_OStream& OS) const;
Standard_EXPORT inline   Standard_Integer GetKey() const;
Standard_EXPORT inline   Standard_Real X() const;
Standard_EXPORT inline   Standard_Real Y() const;
Standard_EXPORT inline   Standard_Real Z() const;
Standard_EXPORT inline   gp_Pnt Pnt() const;
Standard_EXPORT inline   void SetPnt(const gp_Pnt& P) ;
Standard_EXPORT inline   void AddInverseElement(const Handle(SMDS_MeshElement)& ME) ;
Standard_EXPORT   void RemoveInverseElement(const Handle(SMDS_MeshElement)& parent) ;
Standard_EXPORT inline  const SMDS_ListOfMeshElement& InverseElements() const;
Standard_EXPORT inline   void ClearInverseElements() ;
Standard_EXPORT   void SetPosition(const Handle(SMDS_Position)& aPos) ;
Standard_EXPORT   Handle_SMDS_Position GetPosition() const;
Standard_EXPORT ~SMDS_MeshNode();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_MeshNode_Type_();
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
gp_Pnt myPnt;
SMDS_ListOfMeshElement myInverseElements;
Handle_SMDS_Position myPosition;


};


#include "SMDS_MeshNode.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
