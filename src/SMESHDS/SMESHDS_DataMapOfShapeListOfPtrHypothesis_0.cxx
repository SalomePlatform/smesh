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
//  File   : SMESHDS_DataMapOfShapeListOfPtrHypothesis_0.cxx
//  Module : SMESH

using namespace std;
#include "SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx"

#ifndef _Standard_DomainError_HeaderFile
#include <Standard_DomainError.hxx>
#endif
#ifndef _Standard_NoSuchObject_HeaderFile
#include <Standard_NoSuchObject.hxx>
#endif
#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _SMESHDS_ListOfPtrHypothesis_HeaderFile
#include "SMESHDS_ListOfPtrHypothesis.hxx"
#endif
#ifndef _TopTools_ShapeMapHasher_HeaderFile
#include <TopTools_ShapeMapHasher.hxx>
#endif
#ifndef _SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx"
#endif
#ifndef _SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis.hxx"
#endif
 

#define TheKey TopoDS_Shape
#define TheKey_hxx <TopoDS_Shape.hxx>
#define TheItem SMESHDS_ListOfPtrHypothesis
#define TheItem_hxx <SMESHDS_ListOfPtrHypothesis.hxx>
#define Hasher TopTools_ShapeMapHasher
#define Hasher_hxx <TopTools_ShapeMapHasher.hxx>
#define TCollection_DataMapNode SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapNode_hxx <SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis.hxx>
#define TCollection_DataMapIterator SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapIterator_hxx <SMESHDS_DataMapIteratorOfDataMapOfShapeListOfPtrHypothesis.hxx>
#define Handle_TCollection_DataMapNode Handle_SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMapNode_Type_() SMESHDS_DataMapNodeOfDataMapOfShapeListOfPtrHypothesis_Type_()
#define TCollection_DataMap SMESHDS_DataMapOfShapeListOfPtrHypothesis
#define TCollection_DataMap_hxx <SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx>
#include <TCollection_DataMap.gxx>

