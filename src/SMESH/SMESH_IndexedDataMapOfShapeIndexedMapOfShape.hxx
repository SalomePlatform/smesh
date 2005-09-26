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
