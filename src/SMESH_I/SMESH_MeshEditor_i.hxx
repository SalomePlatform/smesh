//  File      : SMESH_MeshEditor_i.hxx
//  Created   : Wed Jun 19 18:43:16 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef _SMESH_MESHEDITOR_I_HXX_
#define _SMESH_MESHEDIOTR_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include "SMESHDS_Mesh.hxx"


class SMESH_MeshEditor_i:
  public POA_SMESH::SMESH_MeshEditor
{
public:
  SMESH_MeshEditor_i(const Handle(SMESHDS_Mesh)& theMesh);

  virtual ~SMESH_MeshEditor_i() {};

  // --- CORBA
  CORBA::Boolean RemoveElements(const SMESH::long_array& IDsOfElements);
  CORBA::Boolean RemoveNodes(const SMESH::long_array& IDsOfNodes);

  CORBA::Boolean AddNode(CORBA::Double x,
			 CORBA::Double y,
			 CORBA::Double z);
  CORBA::Boolean AddEdge(const SMESH::long_array& IDsOfNodes);
  CORBA::Boolean AddFace(const SMESH::long_array& IDsOfNodes);
  CORBA::Boolean AddVolume(const SMESH::long_array& IDsOfNodes);

private:
  SMESH::SMESH_subMesh_var _myMesh;
  Handle (SMESHDS_Mesh) _myMeshDS;
};

#endif

