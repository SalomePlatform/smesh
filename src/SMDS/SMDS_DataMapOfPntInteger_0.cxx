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
//  File   : SMDS_DataMapOfPntInteger_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_DataMapOfPntInteger.hxx"

#ifndef _Standard_DomainError_HeaderFile
#include <Standard_DomainError.hxx>
#endif
#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _gp_Pnt_HeaderFile
#include <gp_Pnt.hxx>
#endif
#ifndef _SMDS_PntHasher_HeaderFile
#include "SMDS_PntHasher.hxx"
#endif
#ifndef _SMDS_DataMapNodeOfDataMapOfPntInteger_HeaderFile
#include "SMDS_DataMapNodeOfDataMapOfPntInteger.hxx"
#endif
#ifndef _SMDS_DataMapIteratorOfDataMapOfPntInteger_HeaderFile
#include "SMDS_DataMapIteratorOfDataMapOfPntInteger.hxx"
#endif
 

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
#include <TCollection_DataMap.gxx>

