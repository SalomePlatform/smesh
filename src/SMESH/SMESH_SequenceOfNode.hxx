// File:      SMESH_SequenceOfNode.hxx
// Created:   11.11.05 10:00:04
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2005


#ifndef SMESH_SequenceOfNode_HeaderFile
#define SMESH_SequenceOfNode_HeaderFile

#include <NCollection_DefineSequence.hxx>

typedef const SMDS_MeshNode* SMDS_MeshNodePtr;

DEFINE_BASECOLLECTION (SMESH_BaseCollectionNodePtr, SMDS_MeshNodePtr)
DEFINE_SEQUENCE(SMESH_SequenceOfNode,
                SMESH_BaseCollectionNodePtr, SMDS_MeshNodePtr)


#endif
