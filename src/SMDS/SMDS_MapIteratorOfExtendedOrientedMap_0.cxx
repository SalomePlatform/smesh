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
//  File   : SMDS_MapIteratorOfExtendedOrientedMap_0.cxx
//  Module : SMESH

using namespace std;
#include "SMDS_MapIteratorOfExtendedOrientedMap.hxx"

#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _SMDS_MeshElement_HeaderFile
#include "SMDS_MeshElement.hxx"
#endif
#ifndef _SMDS_MeshOrientedElementMapHasher_HeaderFile
#include "SMDS_MeshOrientedElementMapHasher.hxx"
#endif
#ifndef _SMDS_ExtendedOrientedMap_HeaderFile
#include "SMDS_ExtendedOrientedMap.hxx"
#endif
#ifndef _SMDS_StdMapNodeOfExtendedOrientedMap_HeaderFile
#include "SMDS_StdMapNodeOfExtendedOrientedMap.hxx"
#endif
 

#define TheKey Handle_SMDS_MeshElement
#define TheKey_hxx <SMDS_MeshElement.hxx>
#define Hasher SMDS_MeshOrientedElementMapHasher
#define Hasher_hxx <SMDS_MeshOrientedElementMapHasher.hxx>
#define TCollection_StdMapNode SMDS_StdMapNodeOfExtendedOrientedMap
#define TCollection_StdMapNode_hxx <SMDS_StdMapNodeOfExtendedOrientedMap.hxx>
#define TCollection_MapIterator SMDS_MapIteratorOfExtendedOrientedMap
#define TCollection_MapIterator_hxx <SMDS_MapIteratorOfExtendedOrientedMap.hxx>
#define Handle_TCollection_StdMapNode Handle_SMDS_StdMapNodeOfExtendedOrientedMap
#define TCollection_StdMapNode_Type_() SMDS_StdMapNodeOfExtendedOrientedMap_Type_()
#define TCollection_Map SMDS_ExtendedOrientedMap
#define TCollection_Map_hxx <SMDS_ExtendedOrientedMap.hxx>
#include <TCollection_MapIterator.gxx>

