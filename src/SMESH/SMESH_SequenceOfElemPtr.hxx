// File:      SMESH_SequenceOfElemPtr.hxx
// Created:   26.09.05 17:41:10
// Author:    Sergey KUUL
// Copyright: Airbus Industries 2004


#ifndef SMESH_SequenceOfElemPtr_HeaderFile
#define SMESH_SequenceOfElemPtr_HeaderFile

#include <NCollection_DefineSequence.hxx>

//#include <Handle_SMDS_MeshElement.hxx>
#include <SMDS_MeshElement.hxx>

typedef const SMDS_MeshElement* SMDS_MeshElementPtr;

DEFINE_BASECOLLECTION (SMESH_BaseCollectionElemPtr, SMDS_MeshElementPtr)
DEFINE_SEQUENCE (SMESH_SequenceOfElemPtr, SMESH_BaseCollectionElemPtr, SMDS_MeshElementPtr)

#endif 
