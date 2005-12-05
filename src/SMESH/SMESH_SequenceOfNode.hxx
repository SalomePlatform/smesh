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
