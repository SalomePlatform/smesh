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
//  File   : Handle_SMDS_SpacePosition.hxx
//  Module : SMESH

#ifndef _Handle_SMDS_SpacePosition_HeaderFile
#define _Handle_SMDS_SpacePosition_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_SMDS_Position_HeaderFile
#include "Handle_SMDS_Position.hxx"
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(SMDS_Position);
class SMDS_SpacePosition;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SMDS_SpacePosition);

class Handle(SMDS_SpacePosition) : public Handle(SMDS_Position) {
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
    Handle(SMDS_SpacePosition)():Handle(SMDS_Position)() {} 
    Handle(SMDS_SpacePosition)(const Handle(SMDS_SpacePosition)& aHandle) : Handle(SMDS_Position)(aHandle) 
     {
     }

    Handle(SMDS_SpacePosition)(const SMDS_SpacePosition* anItem) : Handle(SMDS_Position)((SMDS_Position *)anItem) 
     {
     }

    Handle(SMDS_SpacePosition)& operator=(const Handle(SMDS_SpacePosition)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SMDS_SpacePosition)& operator=(const SMDS_SpacePosition* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SMDS_SpacePosition* operator->() 
     {
      return (SMDS_SpacePosition *)ControlAccess();
     }

    SMDS_SpacePosition* operator->() const 
     {
      return (SMDS_SpacePosition *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SMDS_SpacePosition)();
 
   Standard_EXPORT static const Handle(SMDS_SpacePosition) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
