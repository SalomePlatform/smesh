//  File      : SMESH_Type.h
//  Created   : Mon Jun 03 15:14:15 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

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
  GROUP
};
