//  SMESH SMESH_I :
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
//  File   : SMESH.hxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef _SMESH_SMESH_HXX_
#define _SMESH_SMESH_HXX_

// Tags definition
enum {
  // Top level
  Tag_HypothesisRoot         = 1, // hypotheses root
  Tag_AlgorithmsRoot         = 2, // algorithms root
  // Mesh/Submesh
  Tag_RefOnShape             = 1, // references to shape
  Tag_RefOnAppliedHypothesis = 2, // applied hypotheses root
  Tag_RefOnAppliedAlgorithms = 3, // applied algorithms root
  // Mesh only
  Tag_SubMeshOnVertex        = 4, // sub-meshes roots by type
  Tag_SubMeshOnEdge          = 5, // ...
  Tag_SubMeshOnWire          = 6, // ...
  Tag_SubMeshOnFace          = 7, // ...
  Tag_SubMeshOnShell         = 8, // ...
  Tag_SubMeshOnSolid         = 9, // ...
  Tag_SubMeshOnCompound      = 10, // ...
  Tag_NodeGroups             = 11, // Group roots by type
  Tag_EdgeGroups             = 12, // ...
  Tag_FaceGroups             = 13, // ...
  Tag_VolumeGroups           = 14 // ... 
};

#endif

