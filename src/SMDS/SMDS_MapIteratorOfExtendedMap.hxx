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
//  File   : SMDS_MapIteratorOfExtendedMap.hxx
//  Module : SMESH

#ifndef _SMDS_MapIteratorOfExtendedMap_HeaderFile
#define _SMDS_MapIteratorOfExtendedMap_HeaderFile

#ifndef _TCollection_BasicMapIterator_HeaderFile
#include <TCollection_BasicMapIterator.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Handle_SMDS_StdMapNodeOfExtendedMap_HeaderFile
#include "Handle_SMDS_StdMapNodeOfExtendedMap.hxx"
#endif
class Standard_NoSuchObject;
class SMDS_MeshElement;
class SMDS_MeshElementMapHasher;
class SMDS_ExtendedMap;
class SMDS_StdMapNodeOfExtendedMap;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDS_MapIteratorOfExtendedMap  : public TCollection_BasicMapIterator {

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
Standard_EXPORT SMDS_MapIteratorOfExtendedMap();
Standard_EXPORT SMDS_MapIteratorOfExtendedMap(const SMDS_ExtendedMap& aMap);
Standard_EXPORT   void Initialize(const SMDS_ExtendedMap& aMap) ;
Standard_EXPORT  const Handle_SMDS_MeshElement& Key() const;





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


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
