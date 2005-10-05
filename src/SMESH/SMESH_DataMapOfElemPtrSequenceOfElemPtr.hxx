// File:      SMESH_DataMapOfElemPtrSequenceOfElemPtr.hxx
// Created:   26.09.05 17:41:10
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2005


#ifndef SMESH_DataMapOfElemPtrSequenceOfElemPtr_HeaderFile
#define SMESH_DataMapOfElemPtrSequenceOfElemPtr_HeaderFile

#include <SMESH_SequenceOfElemPtr.hxx>

#include <NCollection_DefineDataMap.hxx>

inline Standard_Integer HashCode(SMDS_MeshElementPtr theElem,
                                 const Standard_Integer theUpper)
{
  void* anElem = (void*) theElem;
  return HashCode(anElem,theUpper);
}

inline Standard_Boolean IsEqual(SMDS_MeshElementPtr theOne,
                                SMDS_MeshElementPtr theTwo)
{
  return theOne == theTwo;
}

DEFINE_BASECOLLECTION (SMESH_BaseCollectionSequenceOfElemPtr, SMESH_SequenceOfElemPtr)
DEFINE_DATAMAP (SMESH_DataMapOfElemPtrSequenceOfElemPtr,
                SMESH_BaseCollectionSequenceOfElemPtr,
                SMDS_MeshElementPtr, SMESH_SequenceOfElemPtr)
#endif 
