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
//  File   : SMDS_DataMapOfIntegerMeshElement_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_DataMapOfIntegerMeshElement.hxx"

#ifndef _Standard_DomainError_HeaderFile
#include <Standard_DomainError.hxx>
#endif
#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _TColStd_MapIntegerHasher_HeaderFile
#include <TColStd_MapIntegerHasher.hxx>
#endif
#ifndef _SMDS_DataMapNodeOfDataMapOfIntegerMeshElement_HeaderFile
#include "SMDS_DataMapNodeOfDataMapOfIntegerMeshElement.hxx"
#endif
#ifndef _SMDS_DataMapIteratorOfDataMapOfIntegerMeshElement_HeaderFile
#include "SMDS_DataMapIteratorOfDataMapOfIntegerMeshElement.hxx"
#endif
 

#define TheKey Standard_Integer
#define TheKey_hxx <Standard_Integer.hxx>
#define TheItem Handle_SMDS_MeshElement
#define TheItem_hxx <SMDS_MeshElement.hxx>
#define Hasher TColStd_MapIntegerHasher
#define Hasher_hxx <TColStd_MapIntegerHasher.hxx>
#define TCollection_DataMapNode SMDS_DataMapNodeOfDataMapOfIntegerMeshElement
#define TCollection_DataMapNode_hxx <SMDS_DataMapNodeOfDataMapOfIntegerMeshElement.hxx>
#define TCollection_DataMapIterator SMDS_DataMapIteratorOfDataMapOfIntegerMeshElement
#define TCollection_DataMapIterator_hxx <SMDS_DataMapIteratorOfDataMapOfIntegerMeshElement.hxx>
#define Handle_TCollection_DataMapNode Handle_SMDS_DataMapNodeOfDataMapOfIntegerMeshElement
#define TCollection_DataMapNode_Type_() SMDS_DataMapNodeOfDataMapOfIntegerMeshElement_Type_()
#define TCollection_DataMap SMDS_DataMapOfIntegerMeshElement
#define TCollection_DataMap_hxx <SMDS_DataMapOfIntegerMeshElement.hxx>
#include <TCollection_DataMap.gxx>

