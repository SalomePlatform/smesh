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
//  File   : SMDS_Mesh.hxx
//  Module : SMESH
//
#ifndef _SMDS_Mesh_HeaderFile
#define _SMDS_Mesh_HeaderFile

#include "SMESH_SMDS.hxx"

#include "SMDS_BallElement.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_Mesh0DElement.hxx"
#include "SMDS_MeshCell.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshInfo.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMDS_UnstructuredGrid.hxx"

#include <set>
#include <list>
#include <vector>
#include <smIdType.hxx>

class SMDS_ElementHolder;
class SMDS_ElementFactory;
class SMDS_NodeFactory;

class SMDS_EXPORT SMDS_Mesh : public SMDS_MeshObject
{
public:

  SMDS_Mesh();
  
  //! actual nodes coordinates, cells definition and reverse connectivity are stored in a vtkUnstructuredGrid
  inline SMDS_UnstructuredGrid* GetGrid() { return myGrid; }

  virtual SMDS_NodeIteratorPtr   nodesIterator  () const;
  virtual SMDS_EdgeIteratorPtr   edgesIterator  () const;
  virtual SMDS_FaceIteratorPtr   facesIterator  () const;
  virtual SMDS_VolumeIteratorPtr volumesIterator() const;

  virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type=SMDSAbs_All) const;
  virtual SMDS_ElemIteratorPtr elementGeomIterator(SMDSAbs_GeometryType type) const;
  virtual SMDS_ElemIteratorPtr elementEntityIterator(SMDSAbs_EntityType type) const;

  virtual SMDS_NodeIteratorPtr shapeNodesIterator   (int                  shapeID,
                                                     size_t               nbElemsToReturn=-1,
                                                     const SMDS_MeshNode* sm1stNode=0) const;
  virtual SMDS_ElemIteratorPtr shapeElementsIterator(int                     shapeID,
                                                     size_t                  nbElemsToReturn=-1,
                                                     const SMDS_MeshElement* sm1stElem=0) const;

  SMDSAbs_ElementType GetElementType( const smIdType id, const bool iselem ) const;

  SMDS_Mesh *AddSubMesh();

  virtual SMDS_MeshNode* AddNodeWithID(double x, double y, double z, smIdType ID);
  virtual SMDS_MeshNode* AddNode      (double x, double y, double z);

  virtual SMDS_Mesh0DElement* Add0DElementWithID(smIdType n,                   smIdType ID);
  virtual SMDS_Mesh0DElement* Add0DElementWithID(const SMDS_MeshNode * n, smIdType ID);
  virtual SMDS_Mesh0DElement* Add0DElement      (const SMDS_MeshNode * n);

  virtual SMDS_BallElement* AddBallWithID(smIdType n,                   double diameter, smIdType ID);
  virtual SMDS_BallElement* AddBallWithID(const SMDS_MeshNode * n, double diameter, smIdType ID);
  virtual SMDS_BallElement* AddBall      (const SMDS_MeshNode * n, double diameter);

  virtual SMDS_MeshEdge* AddEdgeWithID(smIdType n1, smIdType n2, smIdType ID);
  virtual SMDS_MeshEdge* AddEdgeWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       smIdType ID);
  virtual SMDS_MeshEdge* AddEdge(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2);

  // 2d order edge with 3 nodes: n12 - node between n1 and n2
  virtual SMDS_MeshEdge* AddEdgeWithID(smIdType n1, smIdType n2, smIdType n12, smIdType ID);
  virtual SMDS_MeshEdge* AddEdgeWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n12,
                                       smIdType ID);
  virtual SMDS_MeshEdge* AddEdge(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n12);

  virtual SMDS_MeshFace* AddFaceWithID(smIdType n1, smIdType n2, smIdType n3, smIdType ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3,
                                       smIdType ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n3);

  virtual SMDS_MeshFace* AddFaceWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4, smIdType ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3,
                                       const SMDS_MeshNode * n4,
                                       smIdType ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n3,
                                 const SMDS_MeshNode * n4);


  // 2d order triangle of 6 nodes
  virtual SMDS_MeshFace* AddFaceWithID(smIdType n1, smIdType n2, smIdType n3,
                                       smIdType n12,smIdType n23,smIdType n31, smIdType ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3,
                                       const SMDS_MeshNode * n12,
                                       const SMDS_MeshNode * n23,
                                       const SMDS_MeshNode * n31,
                                       smIdType ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n3,
                                 const SMDS_MeshNode * n12,
                                 const SMDS_MeshNode * n23,
                                 const SMDS_MeshNode * n31);

  // 2d order triangle of 7 nodes
  virtual SMDS_MeshFace* AddFaceWithID(smIdType n1, smIdType n2, smIdType n3,
                                       smIdType n12,smIdType n23,smIdType n31, smIdType nCenter, smIdType ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3,
                                       const SMDS_MeshNode * n12,
                                       const SMDS_MeshNode * n23,
                                       const SMDS_MeshNode * n31,
                                       const SMDS_MeshNode * nCenter,
                                       smIdType ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n3,
                                 const SMDS_MeshNode * n12,
                                 const SMDS_MeshNode * n23,
                                 const SMDS_MeshNode * n31,
                                 const SMDS_MeshNode * nCenter);

  // 2d order quadrangle
  virtual SMDS_MeshFace* AddFaceWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                       smIdType n12,smIdType n23,smIdType n34,smIdType n41, smIdType ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3,
                                       const SMDS_MeshNode * n4,
                                       const SMDS_MeshNode * n12,
                                       const SMDS_MeshNode * n23,
                                       const SMDS_MeshNode * n34,
                                       const SMDS_MeshNode * n41,
                                       smIdType ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n3,
                                 const SMDS_MeshNode * n4,
                                 const SMDS_MeshNode * n12,
                                 const SMDS_MeshNode * n23,
                                 const SMDS_MeshNode * n34,
                                 const SMDS_MeshNode * n41);

  virtual SMDS_MeshFace* AddFaceWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                       smIdType n12,smIdType n23,smIdType n34,smIdType n41, smIdType nCenter, smIdType ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3,
                                       const SMDS_MeshNode * n4,
                                       const SMDS_MeshNode * n12,
                                       const SMDS_MeshNode * n23,
                                       const SMDS_MeshNode * n34,
                                       const SMDS_MeshNode * n41,
                                       const SMDS_MeshNode * nCenter,
                                       smIdType ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
                                 const SMDS_MeshNode * n2,
                                 const SMDS_MeshNode * n3,
                                 const SMDS_MeshNode * n4,
                                 const SMDS_MeshNode * n12,
                                 const SMDS_MeshNode * n23,
                                 const SMDS_MeshNode * n34,
                                 const SMDS_MeshNode * n41,
                                 const SMDS_MeshNode * nCenter);

  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4, smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
                                           const SMDS_MeshNode * n2,
                                           const SMDS_MeshNode * n3,
                                           const SMDS_MeshNode * n4,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
                                     const SMDS_MeshNode * n2,
                                     const SMDS_MeshNode * n3,
                                     const SMDS_MeshNode * n4);

  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                           smIdType n5, smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
                                           const SMDS_MeshNode * n2,
                                           const SMDS_MeshNode * n3,
                                           const SMDS_MeshNode * n4,
                                           const SMDS_MeshNode * n5,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
                                     const SMDS_MeshNode * n2,
                                     const SMDS_MeshNode * n3,
                                     const SMDS_MeshNode * n4,
                                     const SMDS_MeshNode * n5);

  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                           smIdType n5, smIdType n6, smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
                                           const SMDS_MeshNode * n2,
                                           const SMDS_MeshNode * n3,
                                           const SMDS_MeshNode * n4,
                                           const SMDS_MeshNode * n5,
                                           const SMDS_MeshNode * n6,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
                                     const SMDS_MeshNode * n2,
                                     const SMDS_MeshNode * n3,
                                     const SMDS_MeshNode * n4,
                                     const SMDS_MeshNode * n5,
                                     const SMDS_MeshNode * n6);

  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                           smIdType n5, smIdType n6, smIdType n7, smIdType n8, smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
                                           const SMDS_MeshNode * n2,
                                           const SMDS_MeshNode * n3,
                                           const SMDS_MeshNode * n4,
                                           const SMDS_MeshNode * n5,
                                           const SMDS_MeshNode * n6,
                                           const SMDS_MeshNode * n7,
                                           const SMDS_MeshNode * n8,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
                                     const SMDS_MeshNode * n2,
                                     const SMDS_MeshNode * n3,
                                     const SMDS_MeshNode * n4,
                                     const SMDS_MeshNode * n5,
                                     const SMDS_MeshNode * n6,
                                     const SMDS_MeshNode * n7,
                                     const SMDS_MeshNode * n8);


  // hexagonal prism
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4, smIdType n5, smIdType n6,
                                           smIdType n7, smIdType n8, smIdType n9, smIdType n10, smIdType n11, smIdType n12,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
                                           const SMDS_MeshNode * n2,
                                           const SMDS_MeshNode * n3,
                                           const SMDS_MeshNode * n4,
                                           const SMDS_MeshNode * n5,
                                           const SMDS_MeshNode * n6,
                                           const SMDS_MeshNode * n7,
                                           const SMDS_MeshNode * n8,
                                           const SMDS_MeshNode * n9,
                                           const SMDS_MeshNode * n10,
                                           const SMDS_MeshNode * n11,
                                           const SMDS_MeshNode * n12,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
                                     const SMDS_MeshNode * n2,
                                     const SMDS_MeshNode * n3,
                                     const SMDS_MeshNode * n4,
                                     const SMDS_MeshNode * n5,
                                     const SMDS_MeshNode * n6,
                                     const SMDS_MeshNode * n7,
                                     const SMDS_MeshNode * n8,
                                     const SMDS_MeshNode * n9,
                                     const SMDS_MeshNode * n10,
                                     const SMDS_MeshNode * n11,
                                     const SMDS_MeshNode * n12);

  // 2d order tetrahedron of 10 nodes
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                           smIdType n12,smIdType n23,smIdType n31,
                                           smIdType n14,smIdType n24,smIdType n34, smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
                                           const SMDS_MeshNode * n2,
                                           const SMDS_MeshNode * n3,
                                           const SMDS_MeshNode * n4,
                                           const SMDS_MeshNode * n12,
                                           const SMDS_MeshNode * n23,
                                           const SMDS_MeshNode * n31,
                                           const SMDS_MeshNode * n14,
                                           const SMDS_MeshNode * n24,
                                           const SMDS_MeshNode * n34,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
                                     const SMDS_MeshNode * n2,
                                     const SMDS_MeshNode * n3,
                                     const SMDS_MeshNode * n4,
                                     const SMDS_MeshNode * n12,
                                     const SMDS_MeshNode * n23,
                                     const SMDS_MeshNode * n31,
                                     const SMDS_MeshNode * n14,
                                     const SMDS_MeshNode * n24,
                                     const SMDS_MeshNode * n34);

  // 2d order pyramid of 13 nodes
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4, smIdType n5,
                                           smIdType n12,smIdType n23,smIdType n34,smIdType n41,
                                           smIdType n15,smIdType n25,smIdType n35,smIdType n45,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
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
                                           const SMDS_MeshNode * n45,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
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

  // 2d order Pentahedron with 15 nodes
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3,
                                           smIdType n4, smIdType n5, smIdType n6,
                                           smIdType n12,smIdType n23,smIdType n31,
                                           smIdType n45,smIdType n56,smIdType n64,
                                           smIdType n14,smIdType n25,smIdType n36,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
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
                                           const SMDS_MeshNode * n36,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
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

  // 2d order Pentahedron with 18 nodes
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3,
                                           smIdType n4, smIdType n5, smIdType n6,
                                           smIdType n12,smIdType n23,smIdType n31,
                                           smIdType n45,smIdType n56,smIdType n64,
                                           smIdType n14,smIdType n25,smIdType n36,
                                           smIdType n1245, smIdType n2356, smIdType n1346,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
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
                                           const SMDS_MeshNode * n36,
                                           const SMDS_MeshNode * n1245,
                                           const SMDS_MeshNode * n2356,
                                           const SMDS_MeshNode * n1346,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
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
                                     const SMDS_MeshNode * n36,
                                     const SMDS_MeshNode * n1245,
                                     const SMDS_MeshNode * n2356,
                                     const SMDS_MeshNode * n1346);


  // 2d oreder Hexahedrons with 20 nodes
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                           smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                                           smIdType n12,smIdType n23,smIdType n34,smIdType n41,
                                           smIdType n56,smIdType n67,smIdType n78,smIdType n85,
                                           smIdType n15,smIdType n26,smIdType n37,smIdType n48,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
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
                                           const SMDS_MeshNode * n48,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
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

  // 2d oreder Hexahedrons with 27 nodes
  virtual SMDS_MeshVolume* AddVolumeWithID(smIdType n1, smIdType n2, smIdType n3, smIdType n4,
                                           smIdType n5, smIdType n6, smIdType n7, smIdType n8,
                                           smIdType n12,smIdType n23,smIdType n34,smIdType n41,
                                           smIdType n56,smIdType n67,smIdType n78,smIdType n85,
                                           smIdType n15,smIdType n26,smIdType n37,smIdType n48,
                                           smIdType n1234,smIdType n1256,smIdType n2367,smIdType n3478,
                                           smIdType n1458,smIdType n5678,smIdType nCenter,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
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
                                           const SMDS_MeshNode * n48,
                                           const SMDS_MeshNode * n1234,
                                           const SMDS_MeshNode * n1256,
                                           const SMDS_MeshNode * n2367,
                                           const SMDS_MeshNode * n3478,
                                           const SMDS_MeshNode * n1458,
                                           const SMDS_MeshNode * n5678,
                                           const SMDS_MeshNode * nCenter,
                                           smIdType ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
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
                                     const SMDS_MeshNode * n48,
                                     const SMDS_MeshNode * n1234,
                                     const SMDS_MeshNode * n1256,
                                     const SMDS_MeshNode * n2367,
                                     const SMDS_MeshNode * n3478,
                                     const SMDS_MeshNode * n1458,
                                     const SMDS_MeshNode * n5678,
                                     const SMDS_MeshNode * nCenter);

  virtual SMDS_MeshFace* AddPolygonalFaceWithID (const std::vector<smIdType> & nodes_ids,
                                                 const smIdType                ID);

  virtual SMDS_MeshFace* AddPolygonalFaceWithID (const std::vector<const SMDS_MeshNode*> & nodes,
                                                 const smIdType                                 ID);

  virtual SMDS_MeshFace* AddPolygonalFace (const std::vector<const SMDS_MeshNode*> & nodes);

  virtual SMDS_MeshFace* AddQuadPolygonalFaceWithID(const std::vector<smIdType> & nodes_ids,
                                                    const smIdType                ID);

  virtual SMDS_MeshFace* AddQuadPolygonalFaceWithID(const std::vector<const SMDS_MeshNode*> & nodes,
                                                    const smIdType                                 ID);

  virtual SMDS_MeshFace* AddQuadPolygonalFace(const std::vector<const SMDS_MeshNode*> & nodes);

  virtual SMDS_MeshVolume* AddPolyhedralVolumeWithID
    (const std::vector<smIdType> & nodes_ids,
     const std::vector<int>      & quantities,
     const smIdType                ID);

  virtual SMDS_MeshVolume* AddPolyhedralVolumeWithID
    (const std::vector<const SMDS_MeshNode*> & nodes,
     const std::vector<int>                  & quantities,
     const smIdType                            ID);

  virtual SMDS_MeshVolume* AddPolyhedralVolume
    (const std::vector<const SMDS_MeshNode*> & nodes,
     const std::vector<int>                  & quantities);

  virtual SMDS_MeshVolume* AddVolumeFromVtkIds(const std::vector<vtkIdType>& vtkNodeIds);

  virtual SMDS_MeshFace* AddFaceFromVtkIds(const std::vector<vtkIdType>& vtkNodeIds);

  virtual void MoveNode(const SMDS_MeshNode *n, double x, double y, double z);

  virtual void RemoveElement(const SMDS_MeshElement *               elem,
                             std::vector<const SMDS_MeshElement *>& removedElems,
                             std::vector<const SMDS_MeshElement *>& removedNodes,
                             const bool                             removenodes = false);
  virtual void RemoveElement(const SMDS_MeshElement * elem, bool removenodes = false);
  virtual void RemoveNode(const SMDS_MeshNode * node);

  /*! Remove only the given element and only if it is free.
   *  Method does not work for meshes with descendants.
   *  Implemented for fast cleaning of meshes.
   */
  virtual void RemoveFreeElement(const SMDS_MeshElement * elem);

  virtual void Clear();

  virtual bool RemoveFromParent();
  virtual bool RemoveSubMesh(const SMDS_Mesh * aMesh);

  bool ChangeElementNodes(const SMDS_MeshElement * elem,
                          const SMDS_MeshNode    * nodes[],
                          const int                nbnodes);
  bool ChangePolyhedronNodes(const SMDS_MeshElement *                 elem,
                             const std::vector<const SMDS_MeshNode*>& nodes,
                             const std::vector<int>&                  quantities);

  //virtual void Renumber (const bool isNodes, const int startID = 1, const int deltaID = 1);
  // Renumber all nodes or elements.

  virtual void CompactMesh();
  virtual bool IsCompacted();
  virtual bool HasNumerationHoles();

  template<class ELEMTYPE>
    static const ELEMTYPE* DownCast( const SMDS_MeshElement* e )
  {
    return (( e && !e->IsNull() && ELEMTYPE::Type() == e->GetType() ) ?
            static_cast<const ELEMTYPE*>(e) : 0 );
  }

  const SMDS_MeshNode *FindNode(smIdType idnode) const;
  const SMDS_MeshNode *FindNodeVtk(vtkIdType idnode) const;
  const SMDS_MeshElement *FindElementVtk(vtkIdType IDelem) const;
  virtual const SMDS_MeshElement * FindElement(smIdType IDelem) const;
  static const SMDS_Mesh0DElement* Find0DElement(const SMDS_MeshNode * n);
  static const SMDS_BallElement* FindBall(const SMDS_MeshNode * n);
  static const SMDS_MeshEdge* FindEdge(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2);
  static const SMDS_MeshEdge* FindEdge(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2,
                                       const SMDS_MeshNode * n3);
  static const SMDS_MeshFace* FindFace(const SMDS_MeshNode *n1,
                                       const SMDS_MeshNode *n2,
                                       const SMDS_MeshNode *n3);
  static const SMDS_MeshFace* FindFace(const SMDS_MeshNode *n1,
                                       const SMDS_MeshNode *n2,
                                       const SMDS_MeshNode *n3,
                                       const SMDS_MeshNode *n4);
  static const SMDS_MeshFace* FindFace(const SMDS_MeshNode *n1,
                                       const SMDS_MeshNode *n2,
                                       const SMDS_MeshNode *n3,
                                       const SMDS_MeshNode *n4,
                                       const SMDS_MeshNode *n5,
                                       const SMDS_MeshNode *n6);
  static const SMDS_MeshFace* FindFace(const SMDS_MeshNode *n1,
                                       const SMDS_MeshNode *n2,
                                       const SMDS_MeshNode *n3,
                                       const SMDS_MeshNode *n4,
                                       const SMDS_MeshNode *n5,
                                       const SMDS_MeshNode *n6,
                                       const SMDS_MeshNode *n7,
                                       const SMDS_MeshNode *n8);

  static const SMDS_MeshFace*    FindFace   (const std::vector<const SMDS_MeshNode *>& nodes);
  static const SMDS_MeshElement* FindElement(const std::vector<const SMDS_MeshNode *>& nodes,
                                             const SMDSAbs_ElementType                 type=SMDSAbs_All,
                                             const bool                                noMedium=true);
  static int GetElementsByNodes(const std::vector<const SMDS_MeshNode *>& nodes,
                                std::vector<const SMDS_MeshElement *>&    foundElems,
                                const SMDSAbs_ElementType                 type=SMDSAbs_All);

  virtual bool Contains( const SMDS_MeshElement* elem ) const;

  /*!
   * \brief Raise an exception if free memory (ram+swap) too low
    * \param doNotRaise - if true, suppress exception, just return free memory size
    * \retval int - amount of available memory in MB or negative number in failure case
   */
  static int CheckMemory(const bool doNotRaise=false);

  virtual smIdType MaxNodeID() const;
  virtual smIdType MinNodeID() const;
  virtual smIdType MaxElementID() const;
  virtual smIdType MinElementID() const;

  const SMDS_MeshInfo& GetMeshInfo() const { return myInfo; }

  virtual smIdType NbNodes() const;
  virtual smIdType NbElements() const;
  virtual smIdType Nb0DElements() const;
  virtual smIdType NbBalls() const;
  virtual smIdType NbEdges() const;
  virtual smIdType NbFaces() const;
  virtual smIdType NbVolumes() const;
  virtual smIdType NbSubMesh() const;

  virtual ~SMDS_Mesh();

  double getMaxDim();
  smIdType FromVtkToSmds(vtkIdType  vtkid) const;

  void dumpGrid(std::string ficdump="dumpGrid");
  static int chunkSize;

  //! low level modification: add, change or remove node or element
  inline void setMyModified() { this->myModified = true; }

  void Modified();
  vtkMTimeType GetMTime() const;

 protected:
  SMDS_Mesh(SMDS_Mesh * parent);

  void addChildrenWithNodes(std::set<const SMDS_MeshElement*>& setOfChildren,
                            const SMDS_MeshElement *           element,
                            std::set<const SMDS_MeshElement*>& nodes);

  inline void adjustBoundingBox(double x, double y, double z)
  {
    if (x > xmax) xmax = x;
    else if (x < xmin) xmin = x;
    if (y > ymax) ymax = y;
    else if (y < ymin) ymin = y;
    if (z > zmax) zmax = z;
    else if (z < zmin) zmin = z;
  }

  void updateInverseElements( const SMDS_MeshElement *        element,
                              const SMDS_MeshNode* const*     nodes,
                              const int                       nbnodes,
                              std::set<const SMDS_MeshNode*>& oldNodes );

  void setNbShapes( size_t nbShapes );


  // Fields PRIVATE

  //! actual nodes coordinates, cells definition and reverse connectivity are stored in a vtkUnstructuredGrid
  SMDS_UnstructuredGrid* myGrid;

  //! Small objects like SMDS_MeshNode are allocated by chunks to limit memory costs of new
  SMDS_NodeFactory*      myNodeFactory;
  SMDS_ElementFactory*   myCellFactory;

  SMDS_Mesh *            myParent;
  std::list<SMDS_Mesh *> myChildren;
  SMDS_MeshInfo          myInfo;

  //! any add, remove or change of node or cell
  bool                   myModified;
  //! use a counter to keep track of modifications
  unsigned long          myModifTime, myCompactTime;

  friend class SMDS_ElementHolder;
  std::set< SMDS_ElementHolder* > myElemHolders;

  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double zmin;
  double zmax;
};


#endif
