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
//  File   : SMDS_ListOfMeshElement.hxx
//  Module : SMESH

#ifndef _SMDS_ListOfMeshElement_HeaderFile
#define _SMDS_ListOfMeshElement_HeaderFile

#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Handle_SMDS_ListNodeOfListOfMeshElement_HeaderFile
#include "Handle_SMDS_ListNodeOfListOfMeshElement.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoSuchObject;
class SMDS_ListIteratorOfListOfMeshElement;
class SMDS_MeshElement;
class SMDS_ListNodeOfListOfMeshElement;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMDS_ListOfMeshElement  {

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
Standard_EXPORT SMDS_ListOfMeshElement();
Standard_EXPORT   void Assign(const SMDS_ListOfMeshElement& Other) ;
  void operator=(const SMDS_ListOfMeshElement& Other) 
{
  Assign(Other);
}

Standard_EXPORT   Standard_Integer Extent() const;
Standard_EXPORT   void Clear() ;
~SMDS_ListOfMeshElement()
{
  Clear();
}

Standard_EXPORT inline   Standard_Boolean IsEmpty() const;
Standard_EXPORT   void Prepend(const Handle(SMDS_MeshElement)& I) ;
Standard_EXPORT   void Prepend(SMDS_ListOfMeshElement& Other) ;
Standard_EXPORT   void Append(const Handle(SMDS_MeshElement)& I) ;
Standard_EXPORT   void Append(SMDS_ListOfMeshElement& Other) ;
Standard_EXPORT   Handle_SMDS_MeshElement& First() const;
Standard_EXPORT   Handle_SMDS_MeshElement& Last() const;
Standard_EXPORT   void RemoveFirst() ;
Standard_EXPORT   void Remove(SMDS_ListIteratorOfListOfMeshElement& It) ;
Standard_EXPORT   void InsertBefore(const Handle(SMDS_MeshElement)& I,SMDS_ListIteratorOfListOfMeshElement& It) ;
Standard_EXPORT   void InsertBefore(SMDS_ListOfMeshElement& Other,SMDS_ListIteratorOfListOfMeshElement& It) ;
Standard_EXPORT   void InsertAfter(const Handle(SMDS_MeshElement)& I,SMDS_ListIteratorOfListOfMeshElement& It) ;
Standard_EXPORT   void InsertAfter(SMDS_ListOfMeshElement& Other,SMDS_ListIteratorOfListOfMeshElement& It) ;


friend class SMDS_ListIteratorOfListOfMeshElement;



protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMDS_ListOfMeshElement(const SMDS_ListOfMeshElement& Other);


 // Fields PRIVATE
 //
Standard_Address myFirst;
Standard_Address myLast;


};

#define Item Handle_SMDS_MeshElement
#define Item_hxx <SMDS_MeshElement.hxx>
#define TCollection_ListNode SMDS_ListNodeOfListOfMeshElement
#define TCollection_ListNode_hxx <SMDS_ListNodeOfListOfMeshElement.hxx>
#define TCollection_ListIterator SMDS_ListIteratorOfListOfMeshElement
#define TCollection_ListIterator_hxx <SMDS_ListIteratorOfListOfMeshElement.hxx>
#define Handle_TCollection_ListNode Handle_SMDS_ListNodeOfListOfMeshElement
#define TCollection_ListNode_Type_() SMDS_ListNodeOfListOfMeshElement_Type_()
#define TCollection_List SMDS_ListOfMeshElement
#define TCollection_List_hxx <SMDS_ListOfMeshElement.hxx>

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
