// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMDS : implementation of Salome mesh data structure
//
#ifndef _SMDS_FaceOfNodes_HeaderFile
#define _SMDS_FaceOfNodes_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_CellOfNodes.hxx"

class SMDS_EXPORT SMDS_FaceOfNodes: public SMDS_CellOfNodes
{
 public:
  void Print(std::ostream & OS) const;
  SMDS_FaceOfNodes(const SMDS_MeshNode* node1,
                   const SMDS_MeshNode* node2,
                   const SMDS_MeshNode* node3);
  SMDS_FaceOfNodes(const SMDS_MeshNode* node1,
                   const SMDS_MeshNode* node2,
                   const SMDS_MeshNode* node3,
                   const SMDS_MeshNode* node4);
  virtual bool ChangeNodes(const SMDS_MeshNode* nodes[],
                           const int            nbNodes);
  virtual int  NbEdges() const;
  virtual int  NbFaces() const;
  virtual int  NbNodes() const;

  virtual int  NbCornerNodes() const { return NbNodes(); }
  virtual int  GetNodeIndex( const SMDS_MeshNode* node ) const;

  virtual bool IsPoly() const { return false; }
  virtual bool IsQuadratic() const  { return false; }

  virtual SMDS_ElemIteratorPtr nodesIterator() const;
  virtual SMDS_NodeIteratorPtr nodeIterator() const;

  virtual const SMDS_MeshNode* GetNode(const int ind) const;

  virtual SMDSAbs_ElementType  GetType() const { return SMDSAbs_Face; }
  virtual SMDSAbs_EntityType   GetEntityType() const;
  virtual SMDSAbs_GeometryType GetGeomType() const;

 private:
  const SMDS_MeshNode* myNodes[4];
  int                  myNbNodes;

};

#endif
