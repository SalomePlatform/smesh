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
//  File   : SMESHDS_ListIteratorOfListOfPtrHypothesis_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx"

#ifndef _Standard_NoMoreObject_HeaderFile
#include <Standard_NoMoreObject.hxx>
#endif
#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMESHDS_ListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListOfPtrHypothesis.hxx"
#endif
#ifndef _SMESHDS_ListNodeOfListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListNodeOfListOfPtrHypothesis.hxx"
#endif
 

#define Item SMESHDS_PtrHypothesis
#define Item_hxx <SMESHDS_PtrHypothesis.hxx>
#define TCollection_ListNode SMESHDS_ListNodeOfListOfPtrHypothesis
#define TCollection_ListNode_hxx <SMESHDS_ListNodeOfListOfPtrHypothesis.hxx>
#define TCollection_ListIterator SMESHDS_ListIteratorOfListOfPtrHypothesis
#define TCollection_ListIterator_hxx <SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx>
#define Handle_TCollection_ListNode Handle_SMESHDS_ListNodeOfListOfPtrHypothesis
#define TCollection_ListNode_Type_() SMESHDS_ListNodeOfListOfPtrHypothesis_Type_()
#define TCollection_List SMESHDS_ListOfPtrHypothesis
#define TCollection_List_hxx <SMESHDS_ListOfPtrHypothesis.hxx>
#include <TCollection_ListIterator.gxx>

