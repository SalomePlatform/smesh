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
//  File   : SMESHDS_ListOfAsciiString.hxx
//  Module : SMESH

#ifndef _SMESHDS_ListOfAsciiString_HeaderFile
#define _SMESHDS_ListOfAsciiString_HeaderFile

#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Handle_SMESHDS_ListNodeOfListOfAsciiString_HeaderFile
#include "Handle_SMESHDS_ListNodeOfListOfAsciiString.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoSuchObject;
class SMESHDS_ListIteratorOfListOfAsciiString;
class TCollection_AsciiString;
class SMESHDS_ListNodeOfListOfAsciiString;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMESHDS_ListOfAsciiString  {

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
Standard_EXPORT SMESHDS_ListOfAsciiString();
Standard_EXPORT   void Assign(const SMESHDS_ListOfAsciiString& Other) ;
  void operator=(const SMESHDS_ListOfAsciiString& Other) 
{
  Assign(Other);
}

Standard_EXPORT   Standard_Integer Extent() const;
Standard_EXPORT   void Clear() ;
~SMESHDS_ListOfAsciiString()
{
  Clear();
}

Standard_EXPORT inline   Standard_Boolean IsEmpty() const;
Standard_EXPORT   void Prepend(const TCollection_AsciiString& I) ;
Standard_EXPORT   void Prepend(SMESHDS_ListOfAsciiString& Other) ;
Standard_EXPORT   void Append(const TCollection_AsciiString& I) ;
Standard_EXPORT   void Append(SMESHDS_ListOfAsciiString& Other) ;
Standard_EXPORT   TCollection_AsciiString& First() const;
Standard_EXPORT   TCollection_AsciiString& Last() const;
Standard_EXPORT   void RemoveFirst() ;
Standard_EXPORT   void Remove(SMESHDS_ListIteratorOfListOfAsciiString& It) ;
Standard_EXPORT   void InsertBefore(const TCollection_AsciiString& I,SMESHDS_ListIteratorOfListOfAsciiString& It) ;
Standard_EXPORT   void InsertBefore(SMESHDS_ListOfAsciiString& Other,SMESHDS_ListIteratorOfListOfAsciiString& It) ;
Standard_EXPORT   void InsertAfter(const TCollection_AsciiString& I,SMESHDS_ListIteratorOfListOfAsciiString& It) ;
Standard_EXPORT   void InsertAfter(SMESHDS_ListOfAsciiString& Other,SMESHDS_ListIteratorOfListOfAsciiString& It) ;


friend class SMESHDS_ListIteratorOfListOfAsciiString;



protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMESHDS_ListOfAsciiString(const SMESHDS_ListOfAsciiString& Other);


 // Fields PRIVATE
 //
Standard_Address myFirst;
Standard_Address myLast;


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

#include <TCollection_List.lxx>

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
