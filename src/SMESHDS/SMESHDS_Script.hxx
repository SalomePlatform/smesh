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
//  File   : SMESHDS_Script.hxx
//  Module : SMESH

#ifndef _SMESHDS_Script_HeaderFile
#define _SMESHDS_Script_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_Script_HeaderFile
#include "Handle_SMESHDS_Script.hxx"
#endif

#ifndef _SMESHDS_ListOfCommand_HeaderFile
#include "SMESHDS_ListOfCommand.hxx"
#endif
#ifndef _MMgt_TShared_HeaderFile
#include <MMgt_TShared.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
class SMESHDS_ListOfCommand;


class SMESHDS_Script : public MMgt_TShared {

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
Standard_EXPORT   void AddNode(const Standard_Integer NewNodeID,const Standard_Real x,const Standard_Real y,const Standard_Real z) ;
Standard_EXPORT   void AddEdge(const Standard_Integer NewEdgeID,const Standard_Integer idnode1,const Standard_Integer idnode2) ;
Standard_EXPORT   void AddFace(const Standard_Integer NewFaceID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) ;
Standard_EXPORT   void AddFace(const Standard_Integer NewFaceID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT   void AddVolume(const Standard_Integer NewVolID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT   void AddVolume(const Standard_Integer NewVolID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5) ;
Standard_EXPORT   void AddVolume(const Standard_Integer NewVolID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6) ;
Standard_EXPORT   void AddVolume(const Standard_Integer NewVolID,const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6,const Standard_Integer idnode7,const Standard_Integer idnode8) ;
Standard_EXPORT   void MoveNode(const Standard_Integer NewNodeID,const Standard_Real x,const Standard_Real y,const Standard_Real z) ;
Standard_EXPORT   void RemoveNode(const Standard_Integer NodeID) ;
Standard_EXPORT   void RemoveElement(const Standard_Integer ElementID) ;
Standard_EXPORT   void Clear() ;
Standard_EXPORT  const SMESHDS_ListOfCommand& GetCommands() ;
Standard_EXPORT ~SMESHDS_Script();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_Script_Type_();
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
SMESHDS_ListOfCommand myCommands;


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
