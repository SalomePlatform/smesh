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
//  File   : SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis.hxx
//  Module : SMESH

#ifndef _SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis_HeaderFile
#define _SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis_HeaderFile

#ifndef _TCollection_BasicMapIterator_HeaderFile
#include <TCollection_BasicMapIterator.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _SMESHDS_PtrHypothesis_HeaderFile
#include "SMESHDS_PtrHypothesis.hxx"
#endif
#ifndef _Handle_SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis_HeaderFile
#include "Handle_SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis.hxx"
#endif
class Standard_NoSuchObject;
class TColStd_MapIntegerHasher;
class SMESHDS_DataMapOfIntegerPtrHypothesis;
class SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis  : public TCollection_BasicMapIterator {

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
Standard_EXPORT SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis();
Standard_EXPORT SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis(const SMESHDS_DataMapOfIntegerPtrHypothesis& aMap);
Standard_EXPORT   void Initialize(const SMESHDS_DataMapOfIntegerPtrHypothesis& aMap) ;
Standard_EXPORT  const Standard_Integer& Key() const;
Standard_EXPORT  const SMESHDS_PtrHypothesis& Value() const;





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
