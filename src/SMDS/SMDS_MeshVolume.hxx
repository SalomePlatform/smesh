// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMDS_MeshVolume.hxx
//  Module : SMESH
//
#ifndef _SMDS_MeshVolume_HeaderFile
#define _SMDS_MeshVolume_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshCell.hxx"

/*!
 * \brief Mesh volume. This type is not allocated.
 *        It is only used as function argument type to provide more clear semantic
 *        and to provide API specific to polyherdal volume
 */
class SMDS_EXPORT SMDS_MeshVolume : public SMDS_MeshCell
{
  void init( const std::vector<const SMDS_MeshNode*>& nodes,
             const std::vector<int>&                  nbNodesPerFace ); // init a polyherdon

  void init( const std::vector<vtkIdType>& vtkNodeIds );

  friend class SMDS_Mesh;

 public:
  virtual SMDSAbs_ElementType  GetType() const { return SMDSAbs_Volume; }
  virtual const SMDS_MeshNode* GetNode(const int ind) const;
  virtual int  NbNodes() const;
  virtual int  NbFaces() const;
  virtual int  NbEdges() const;
  virtual int  GetNodeIndex( const SMDS_MeshNode* node ) const;
  virtual bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes);
  virtual bool IsMediumNode(const SMDS_MeshNode* node) const;
  virtual int  NbCornerNodes() const;

  virtual SMDS_ElemIteratorPtr nodesIterator() const = 0;
  virtual SMDS_NodeIteratorPtr nodeIterator() const = 0;

  bool ChangeNodes(const std::vector<const SMDS_MeshNode*>& nodes,
                   const std::vector<int>&                  quantities) const;

  // 1 <= face_ind <= NbFaces()
  int NbFaceNodes (const int face_ind) const;

  // 1 <= face_ind <= NbFaces()
  // 1 <= node_ind <= NbFaceNodes()
  const SMDS_MeshNode* GetFaceNode (const int face_ind, const int node_ind) const;

  std::vector<int> GetQuantities() const;

  static SMDSAbs_ElementType Type() { return SMDSAbs_Volume; }
};
#endif
