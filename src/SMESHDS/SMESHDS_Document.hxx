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
//  File   : SMESHDS_Document.hxx
//  Module : SMESH

#ifndef _SMESHDS_Document_HeaderFile
#define _SMESHDS_Document_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_Document_HeaderFile
#include "Handle_SMESHDS_Document.hxx"
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _SMESHDS_DataMapOfIntegerMesh_HeaderFile
#include "SMESHDS_DataMapOfIntegerMesh.hxx"
#endif
#ifndef _SMESHDS_DataMapOfIntegerPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapOfIntegerPtrHypothesis.hxx"
#endif
#ifndef _SMESHDS_DataMapIteratorOfDataMapOfIntegerMesh_HeaderFile
#include "SMESHDS_DataMapIteratorOfDataMapOfIntegerMesh.hxx"
#endif
#ifndef _SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis.hxx"
#endif
#ifndef _MMgt_TShared_HeaderFile
#include <MMgt_TShared.hxx>
#endif
#ifndef _Handle_SMESHDS_Mesh_HeaderFile
#include "Handle_SMESHDS_Mesh.hxx"
#endif
#ifndef _SMESHDS_PtrHypothesis_HeaderFile
#include "SMESHDS_PtrHypothesis.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class SMESHDS_Mesh;


class SMESHDS_Document : public MMgt_TShared {

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
Standard_EXPORT SMESHDS_Document(const Standard_Integer UserID);
Standard_EXPORT   Standard_Integer NewMesh() ;
Standard_EXPORT   void RemoveMesh(const Standard_Integer MeshID) ;
Standard_EXPORT   Handle_SMESHDS_Mesh GetMesh(const Standard_Integer MeshID) ;
Standard_EXPORT   void AddHypothesis(const SMESHDS_PtrHypothesis& H) ;
Standard_EXPORT   void RemoveHypothesis(const Standard_Integer HypID) ;
Standard_EXPORT   SMESHDS_PtrHypothesis GetHypothesis(const Standard_Integer HypID) ;
Standard_EXPORT   Standard_Integer NbMeshes() ;
Standard_EXPORT   Standard_Integer NbHypothesis() ;
Standard_EXPORT   void InitMeshesIterator() ;
Standard_EXPORT   void NextMesh() ;
Standard_EXPORT   Standard_Boolean MoreMesh() ;
Standard_EXPORT   Handle_SMESHDS_Mesh CurrentMesh() ;
Standard_EXPORT   void InitHypothesisIterator() ;
Standard_EXPORT   void NextHypothesis() ;
Standard_EXPORT   Standard_Boolean MoreHypothesis() ;
Standard_EXPORT   SMESHDS_PtrHypothesis CurrentHypothesis() ;
Standard_EXPORT ~SMESHDS_Document();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_Document_Type_();
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
Standard_Integer myUserID;
SMESHDS_DataMapOfIntegerMesh myMeshes;
SMESHDS_DataMapOfIntegerPtrHypothesis myHypothesis;
SMESHDS_DataMapIteratorOfDataMapOfIntegerMesh myMeshesIt;
SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis myHypothesisIt;


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
