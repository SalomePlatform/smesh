//  SMESH SMESHDS : management of mesh data and SMESH document
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHDS_SubMesh.hxx
//  Module : SMESH

#ifndef _SMESHDS_SubMesh_HeaderFile
#define _SMESHDS_SubMesh_HeaderFile

#include "SMESH_SMESHDS.hxx"

#include "SMDS_Mesh.hxx"
#include <set>

class SMESHDS_EXPORT SMESHDS_SubMesh
{
 public:

  bool IsComplexSubmesh() const { return !mySubMeshes.empty(); }

  // if !IsComplexSubmesh()
  void AddElement(const SMDS_MeshElement * ME);
  bool RemoveElement(const SMDS_MeshElement * ME); // ret true if ME was in
  void AddNode(const SMDS_MeshNode * ME);
  bool RemoveNode(const SMDS_MeshNode * ME);       // ret true if ME was in

  // if IsComplexSubmesh()
  void AddSubMesh( const SMESHDS_SubMesh* theSubMesh );
  bool RemoveSubMesh( const SMESHDS_SubMesh* theSubMesh );
  bool ContainsSubMesh( const SMESHDS_SubMesh* theSubMesh ) const;

  // for both types
  int NbElements() const;
  SMDS_ElemIteratorPtr GetElements() const;
  int NbNodes() const;
  SMDS_NodeIteratorPtr GetNodes() const;
  bool Contains(const SMDS_MeshElement * ME) const;      // check if elem or node is in

 private:
  //const SMDS_Mesh * myMesh;
  std::set<const SMDS_MeshElement*> myElements;
  std::set<const SMDS_MeshNode*>    myNodes;
  std::set<const SMESHDS_SubMesh*>  mySubMeshes;
};
#endif
