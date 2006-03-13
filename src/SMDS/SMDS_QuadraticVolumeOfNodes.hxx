//  SMESH SMDS : implementaion of Salome mesh data structure
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
//  File   : SMDS_QuadraticVolumeOfNodes.hxx
//  Module : SMESH

#ifndef _SMDS_QuadraticVolumeOfNodes_HeaderFile
#define _SMDS_QuadraticVolumeOfNodes_HeaderFile

#include "SMDS_MeshVolume.hxx"

class SMDS_WNT_EXPORT SMDS_QuadraticVolumeOfNodes: public SMDS_MeshVolume
{
public:
  // tetrahedron of 10 nodes
  SMDS_QuadraticVolumeOfNodes (const SMDS_MeshNode * n1,
                               const SMDS_MeshNode * n2,
                               const SMDS_MeshNode * n3,
                               const SMDS_MeshNode * n4,
                               const SMDS_MeshNode * n12,
                               const SMDS_MeshNode * n23,
                               const SMDS_MeshNode * n31,
                               const SMDS_MeshNode * n14, 
                               const SMDS_MeshNode * n24,
                               const SMDS_MeshNode * n34);

  // pyramid of 13 nodes
  SMDS_QuadraticVolumeOfNodes(const SMDS_MeshNode * n1,
                              const SMDS_MeshNode * n2,
                              const SMDS_MeshNode * n3,
                              const SMDS_MeshNode * n4,
                              const SMDS_MeshNode * n5,
                              const SMDS_MeshNode * n12,
                              const SMDS_MeshNode * n23,
                              const SMDS_MeshNode * n34,
                              const SMDS_MeshNode * n41, 
                              const SMDS_MeshNode * n15,
                              const SMDS_MeshNode * n25,
                              const SMDS_MeshNode * n35,
                              const SMDS_MeshNode * n45);

  //  Pentahedron with 15 nodes
  SMDS_QuadraticVolumeOfNodes(const SMDS_MeshNode * n1,
                              const SMDS_MeshNode * n2,
                              const SMDS_MeshNode * n3,
                              const SMDS_MeshNode * n4,
                              const SMDS_MeshNode * n5,
                              const SMDS_MeshNode * n6, 
                              const SMDS_MeshNode * n12,
                              const SMDS_MeshNode * n23,
                              const SMDS_MeshNode * n31, 
                              const SMDS_MeshNode * n45,
                              const SMDS_MeshNode * n56,
                              const SMDS_MeshNode * n64, 
                              const SMDS_MeshNode * n14,
                              const SMDS_MeshNode * n25,
                              const SMDS_MeshNode * n36);

  // Hexahedrons with 20 nodes
  SMDS_QuadraticVolumeOfNodes(const SMDS_MeshNode * n1,
                              const SMDS_MeshNode * n2,
                              const SMDS_MeshNode * n3,
                              const SMDS_MeshNode * n4,
                              const SMDS_MeshNode * n5,
                              const SMDS_MeshNode * n6,
                              const SMDS_MeshNode * n7,
                              const SMDS_MeshNode * n8, 
                              const SMDS_MeshNode * n12,
                              const SMDS_MeshNode * n23,
                              const SMDS_MeshNode * n34,
                              const SMDS_MeshNode * n41, 
                              const SMDS_MeshNode * n56,
                              const SMDS_MeshNode * n67,
                              const SMDS_MeshNode * n78,
                              const SMDS_MeshNode * n85, 
                              const SMDS_MeshNode * n15,
                              const SMDS_MeshNode * n26,
                              const SMDS_MeshNode * n37,
                              const SMDS_MeshNode * n48);

  virtual bool IsQuadratic() const { return true; }

  virtual bool IsMediumNode(const SMDS_MeshNode* node) const;

  bool ChangeNodes(const SMDS_MeshNode* nodes[],
                   const int            nbNodes);

  virtual int NbNodes() const;
  virtual int NbEdges() const;
  virtual int NbFaces() const;

  virtual void Print (std::ostream & OS) const;

  /*!
   * \brief Return node by its index
    * \param ind - node index
    * \retval const SMDS_MeshNode* - the node
   * 
   * Index is wrapped if it is out of a valid range
   */
  virtual const SMDS_MeshNode* GetNode(const int ind) const;

 protected:
  virtual SMDS_ElemIteratorPtr elementsIterator (SMDSAbs_ElementType type) const;

 private:
  std::vector<const SMDS_MeshNode *> myNodes;
};

#endif
