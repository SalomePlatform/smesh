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
//  File   : SMESHDS_ListOfCommand.hxx
//  Module : SMESH

#ifndef _SMESHDS_ListOfCommand_HeaderFile
#define _SMESHDS_ListOfCommand_HeaderFile

#ifndef _Standard_Address_HeaderFile
#include <Standard_Address.hxx>
#endif
#ifndef _Handle_SMESHDS_Command_HeaderFile
#include "Handle_SMESHDS_Command.hxx"
#endif
#ifndef _Handle_SMESHDS_ListNodeOfListOfCommand_HeaderFile
#include "Handle_SMESHDS_ListNodeOfListOfCommand.hxx"
#endif
#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
class Standard_NoSuchObject;
class SMESHDS_ListIteratorOfListOfCommand;
class SMESHDS_Command;
class SMESHDS_ListNodeOfListOfCommand;


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

class SMESHDS_ListOfCommand  {

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
Standard_EXPORT SMESHDS_ListOfCommand();
Standard_EXPORT   void Assign(const SMESHDS_ListOfCommand& Other) ;
  void operator=(const SMESHDS_ListOfCommand& Other) 
{
  Assign(Other);
}

Standard_EXPORT   Standard_Integer Extent() const;
Standard_EXPORT   void Clear() ;
~SMESHDS_ListOfCommand()
{
  Clear();
}

Standard_EXPORT inline   Standard_Boolean IsEmpty() const;
Standard_EXPORT   void Prepend(const Handle(SMESHDS_Command)& I) ;
Standard_EXPORT   void Prepend(SMESHDS_ListOfCommand& Other) ;
Standard_EXPORT   void Append(const Handle(SMESHDS_Command)& I) ;
Standard_EXPORT   void Append(SMESHDS_ListOfCommand& Other) ;
Standard_EXPORT   Handle_SMESHDS_Command& First() const;
Standard_EXPORT   Handle_SMESHDS_Command& Last() const;
Standard_EXPORT   void RemoveFirst() ;
Standard_EXPORT   void Remove(SMESHDS_ListIteratorOfListOfCommand& It) ;
Standard_EXPORT   void InsertBefore(const Handle(SMESHDS_Command)& I,SMESHDS_ListIteratorOfListOfCommand& It) ;
Standard_EXPORT   void InsertBefore(SMESHDS_ListOfCommand& Other,SMESHDS_ListIteratorOfListOfCommand& It) ;
Standard_EXPORT   void InsertAfter(const Handle(SMESHDS_Command)& I,SMESHDS_ListIteratorOfListOfCommand& It) ;
Standard_EXPORT   void InsertAfter(SMESHDS_ListOfCommand& Other,SMESHDS_ListIteratorOfListOfCommand& It) ;


friend class SMESHDS_ListIteratorOfListOfCommand;



protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 
Standard_EXPORT SMESHDS_ListOfCommand(const SMESHDS_ListOfCommand& Other);


 // Fields PRIVATE
 //
Standard_Address myFirst;
Standard_Address myLast;


};

#define Item Handle_SMESHDS_Command
#define Item_hxx <SMESHDS_Command.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfCommand
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfCommand.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfCommand
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfCommand.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfCommand
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfCommand_Type_()
#define TCollection_List SMESHDS_ListOfCommand
#define TCollection_List_hxx <SMESHDS_ListOfCommand.hxx>

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
