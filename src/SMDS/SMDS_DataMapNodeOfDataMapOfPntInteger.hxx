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
//  File   : SMDS_DataMapNodeOfDataMapOfPntInteger.hxx
//  Module : SMESH

#ifndef _SMDS_DataMapNodeOfDataMapOfPntInteger_HeaderFile
#define _SMDS_DataMapNodeOfDataMapOfPntInteger_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMDS_DataMapNodeOfDataMapOfPntInteger_HeaderFile
#include "Handle_SMDS_DataMapNodeOfDataMapOfPntInteger.hxx"
#endif

#ifndef _gp_Pnt_HeaderFile
#include <gp_Pnt.hxx>
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class gp_Pnt;
class SMDS_PntHasher;
class SMDS_DataMapOfPntInteger;
class SMDS_DataMapIteratorOfDataMapOfPntInteger;


class SMDS_DataMapNodeOfDataMapOfPntInteger : public TCollection_MapNode {

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
Standard_EXPORT inline SMDS_DataMapNodeOfDataMapOfPntInteger(const gp_Pnt& K,const Standard_Integer& I,const TCollection_MapNodePtr& n);
Standard_EXPORT inline   gp_Pnt& Key() const;
Standard_EXPORT inline   Standard_Integer& Value() const;
Standard_EXPORT ~SMDS_DataMapNodeOfDataMapOfPntInteger();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMDS_DataMapNodeOfDataMapOfPntInteger_Type_();
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
gp_Pnt myKey;
Standard_Integer myValue;


};

#define TheKey gp_Pnt
#define TheKey_hxx <gp_Pnt.hxx>
#define TheItem Standard_Integer
#define TheItem_hxx <Standard_Integer.hxx>
#define Hasher SMDS_PntHasher
#define Hasher_hxx <SMDS_PntHasher.hxx>
#define TCollection_DataMapNode SMDS_DataMapNodeOfDataMapOfPntInteger
#define TCollection_DataMapNode_hxx <SMDS_DataMapNodeOfDataMapOfPntInteger.hxx>
#define TCollection_DataMapIterator SMDS_DataMapIteratorOfDataMapOfPntInteger
#define TCollection_DataMapIterator_hxx <SMDS_DataMapIteratorOfDataMapOfPntInteger.hxx>
#define Handle_TCollection_DataMapNode Handle_SMDS_DataMapNodeOfDataMapOfPntInteger
#define TCollection_DataMapNode_Type_() SMDS_DataMapNodeOfDataMapOfPntInteger_Type_()
#define TCollection_DataMap SMDS_DataMapOfPntInteger
#define TCollection_DataMap_hxx <SMDS_DataMapOfPntInteger.hxx>

#include <TCollection_DataMapNode.lxx>

#undef TheKey
#undef TheKey_hxx
#undef TheItem
#undef TheItem_hxx
#undef Hasher
#undef Hasher_hxx
#undef TCollection_DataMapNode
#undef TCollection_DataMapNode_hxx
#undef TCollection_DataMapIterator
#undef TCollection_DataMapIterator_hxx
#undef Handle_TCollection_DataMapNode
#undef TCollection_DataMapNode_Type_
#undef TCollection_DataMap
#undef TCollection_DataMap_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
