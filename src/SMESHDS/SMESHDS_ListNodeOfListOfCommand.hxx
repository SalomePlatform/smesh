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
//  File   : SMESHDS_ListNodeOfListOfCommand.hxx
//  Module : SMESH

#ifndef _SMESHDS_ListNodeOfListOfCommand_HeaderFile
#define _SMESHDS_ListNodeOfListOfCommand_HeaderFile

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_ListNodeOfListOfCommand_HeaderFile
#include "Handle_SMESHDS_ListNodeOfListOfCommand.hxx"
#endif

#ifndef _Handle_SMESHDS_Command_HeaderFile
#include "Handle_SMESHDS_Command.hxx"
#endif
#ifndef _TCollection_MapNode_HeaderFile
#include <TCollection_MapNode.hxx>
#endif
#ifndef _TCollection_MapNodePtr_HeaderFile
#include <TCollection_MapNodePtr.hxx>
#endif
class SMESHDS_Command;
class SMESHDS_ListOfCommand;
class SMESHDS_ListIteratorOfListOfCommand;


class SMESHDS_ListNodeOfListOfCommand : public TCollection_MapNode {

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
Standard_EXPORT inline SMESHDS_ListNodeOfListOfCommand(const Handle(SMESHDS_Command)& I,const TCollection_MapNodePtr& n);
Standard_EXPORT inline   Handle_SMESHDS_Command& Value() const;
Standard_EXPORT ~SMESHDS_ListNodeOfListOfCommand();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_ListNodeOfListOfCommand_Type_();
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
Handle_SMESHDS_Command myValue;


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

#include <TCollection_ListNode.lxx>

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
