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
