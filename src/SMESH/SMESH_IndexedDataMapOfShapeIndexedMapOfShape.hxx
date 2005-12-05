// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
// File:      SMESH_IndexedDataMapOfShapeIndexedMapOfShape.hxx
// Created:   20.09.05 09:51:12
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2005


#ifndef SMESH_IndexedMapOfShape_HeaderFile
#define SMESH_IndexedMapOfShape_HeaderFile

#include <NCollection_DefineIndexedMap.hxx>

#include <TopoDS_Shape.hxx>

inline Standard_Boolean IsEqual(const TopoDS_Shape& S1,
                                const TopoDS_Shape& S2)
{
  return S1.IsSame(S2);
}

///  Class SMESH_IndexedMapOfShape

DEFINE_BASECOLLECTION (SMESH_BaseCollectionShape, TopoDS_Shape)
DEFINE_INDEXEDMAP (SMESH_IndexedMapOfShape, SMESH_BaseCollectionShape, TopoDS_Shape)

#endif 

#ifndef SMESH_IndexedDataMapOfShapeIndexedMapOfShape_HeaderFile
#define SMESH_IndexedDataMapOfShapeIndexedMapOfShape_HeaderFile

#include <NCollection_DefineIndexedDataMap.hxx>

///  Class SMESH_IndexedDataMapOfShapeIndexedMapOfShape

DEFINE_BASECOLLECTION (SMESH_BaseCollectionIndexedMapOfShape, SMESH_IndexedMapOfShape)
DEFINE_INDEXEDDATAMAP (SMESH_IndexedDataMapOfShapeIndexedMapOfShape,
                       SMESH_BaseCollectionIndexedMapOfShape, TopoDS_Shape,
                       SMESH_IndexedMapOfShape)
#endif 
