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
//  File   : SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _TColStd_MapIntegerHasher_HeaderFile
#include <TColStd_MapIntegerHasher.hxx>
#endif
#ifndef _SMESHDS_DataMapOfIntegerPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapOfIntegerPtrHypothesis.hxx"
#endif
#ifndef _SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis.hxx"
#endif
 

#define TheKey Standard_Integer
#define TheKey_hxx <Standard_Integer.hxx>
#define TheItem SMESHDS_PtrHypothesis
#define TheItem_hxx <SMESHDS_PtrHypothesis.hxx>
#define Hasher TColStd_MapIntegerHasher
#define Hasher_hxx <TColStd_MapIntegerHasher.hxx>
#define TCollection_DataMapNode SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis
#define TCollection_DataMapNode_hxx <SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis.hxx>
#define TCollection_DataMapIterator SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis
#define TCollection_DataMapIterator_hxx <SMESHDS_DataMapIteratorOfDataMapOfIntegerPtrHypothesis.hxx>
#define Handle_TCollection_DataMapNode Handle_SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis
#define TCollection_DataMapNode_Type_() SMESHDS_DataMapNodeOfDataMapOfIntegerPtrHypothesis_Type_()
#define TCollection_DataMap SMESHDS_DataMapOfIntegerPtrHypothesis
#define TCollection_DataMap_hxx <SMESHDS_DataMapOfIntegerPtrHypothesis.hxx>
#include <TCollection_DataMapIterator.gxx>

