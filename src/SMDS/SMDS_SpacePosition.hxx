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
//  File   : SMDS_SpacePosition.hxx
//  Module : SMESH

#ifndef _SMDS_SpacePosition_HeaderFile
#define _SMDS_SpacePosition_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_SpacePosition_HeaderFile
#include "Handle_SMDS_SpacePosition.hxx"
#endif

#ifndef _gp_Pnt_HeaderFile
#include <gp_Pnt.hxx>
#endif
#ifndef _SMDS_Position_HeaderFile
#include "SMDS_Position.hxx"
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
class gp_Pnt;


class SMDS_SpacePosition : public SMDS_Position {

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
Standard_EXPORT SMDS_SpacePosition();
Standard_EXPORT SMDS_SpacePosition(const Standard_Real x,const Standard_Real y,const Standard_Real z);
Standard_EXPORT SMDS_SpacePosition(const gp_Pnt& aCoords);
Standard_EXPORT virtual  gp_Pnt Coords() const;
Standard_EXPORT inline   void SetCoords(const Standard_Real x,const Standard_Real y,const Standard_Real z) ;
Standard_EXPORT inline   void SetCoords(const gp_Pnt& aCoords) ;
Standard_EXPORT ~SMDS_SpacePosition();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_SpacePosition_Type_();
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
gp_Pnt myCoords;


};


#include "SMDS_SpacePosition.lxx"



// other inline functions and methods (like "C++: function call" methods)
//


#endif
