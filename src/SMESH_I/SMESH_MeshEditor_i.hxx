//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_MeshEditor_i.hxx
//  Author : Nicolas REJNERI
//  Module : SMESH
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

