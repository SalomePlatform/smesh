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
//  File   : SMESHDS_ListIteratorOfListOfAsciiString.hxx
//  Module : SMESH

#ifndef _SMESHDS_ListIteratorOfListOfAsciiString_HeaderFile
#define _SMESHDS_ListIteratorOfListOfAsciiString_HeaderFile

#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Handle_SMESHDS_ListNodeOfListOfAsciiString_HeaderFile
#include "Handle_SMESHDS_ListNodeOfListOfAsciiString.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoMoreObject;
class Standard_NoSuchObject;
class SMESHDS_ListOfAsciiString;
class TCollection_AsciiString;
class SMESHDS_ListNodeOfListOfAsciiString;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMESHDS_ListIteratorOfListOfAsciiString  {

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
Standard_EXPORT SMESHDS_ListIteratorOfListOfAsciiString();
Standard_EXPORT SMESHDS_ListIteratorOfListOfAsciiString(const SMESHDS_ListOfAsciiString& L);
Standard_EXPORT   void Initialize(const SMESHDS_ListOfAsciiString& L) ;
Standard_EXPORT inline   Standard_Boolean More() const;
Standard_EXPORT   void Next() ;
Standard_EXPORT   TCollection_AsciiString& Value() const;


friend class SMESHDS_ListOfAsciiString;



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
Standard_Address current;
Standard_Address previous;


};

#define Item TCollection_AsciiString
#define Item_hxx <TCollection_AsciiString.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfAsciiString
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfAsciiString.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfAsciiString
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfAsciiString.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfAsciiString
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfAsciiString_Type_()
#define TCollection_List SMESHDS_ListOfAsciiString
#define TCollection_List_hxx <SMESHDS_ListOfAsciiString.hxx>

#include <TCollection_ListIterator.lxx>

#undef Item
#undef Item_hxx
#undef TCollection_ListNode
#undef TCollection_ListNode_hxx
#undef TCollection_ListIterator
#undef TCollection_ListIterator_hxx
#undef Handle_TCollection_ListNode
#undef TCollection_ListNode_Type_
#undef TCollection_List
#undef TCollection_List_hxx


// other inline functions and methods (like "C++: function call" methods)
//


#endif
