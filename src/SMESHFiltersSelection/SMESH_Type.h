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
//  File      : SMESH_Type.h
//  Created   : Mon Jun 03 15:14:15 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef SMESH_TYPE_HEADER
#define SMESH_TYPE_HEADER

enum MeshObjectType {
  HYPOTHESIS,
  ALGORITHM,
  MESH,
  SUBMESH,
  MESHorSUBMESH,
  SUBMESH_VERTEX,
  SUBMESH_EDGE,
  SUBMESH_FACE,
  SUBMESH_SOLID,
  SUBMESH_COMPOUND,
  GROUP,
  COMPONENT
};

#endif
