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
// File      : SMESH_Delaunay.hxx
// Created   : Wed Apr 19 15:42:54 2017
// Author    : Edward AGAPOV (eap)


#ifndef __SMESH_Delaunay_HXX__
#define __SMESH_Delaunay_HXX__

#include "SMESH_TypeDefs.hxx"

#include <TopoDS_Face.hxx>
#include <BRepMesh_DataStructureOfDelaun.hxx>

/*!
 * \brief Create a Delaunay triangulation of nodes on a face boundary
 *        and provide exploration of nodes shared by elements lying on
 *        the face. For a returned node, also return a Delaunay triangle
 *        the node lies in and its Barycentric Coordinates within the triangle.
 *        Only non-marked nodes are visited. Boundary nodes given at the construction
 *        are not returned.
 *
 *        For usage, this class needs to be subclassed to implement getNodeUV();
 */
class SMESHUtils_EXPORT SMESH_Delaunay
{
 public:

  // construct a Delaunay triangulation of given boundary nodes
  SMESH_Delaunay(const std::vector< const UVPtStructVec* > & boundaryNodes,
                 const TopoDS_Face&                          face,
                 const int                                   faceID);

  virtual ~SMESH_Delaunay() {}

  // prepare to the exploration of nodes
  void InitTraversal(const int nbNodesToVisit = -1);

  // return a node with its Barycentric Coordinates within the triangle
  // defined by its node indices (zero based)
  const SMDS_MeshNode* NextNode( double bc[3], int triaNodes[3] );

  // return nb of nodes returned by NextNode()
  size_t NbVisitedNodes() const { return _nbVisitedNodes; }


  // find a triangle containing an UV, starting from a given triangle;
  // return barycentric coordinates of the UV and the found triangle (indices are zero based).
  const BRepMesh_Triangle* FindTriangle( const gp_XY&             uv,
                                         const BRepMesh_Triangle* bmTria,
                                         double                   bc[3],
                                         int                      triaNodes[3]);

  // return any Delaunay triangle neighboring a given boundary node (zero based)
  const BRepMesh_Triangle* GetTriangleNear( int iBndNode );

  // return source boundary nodes
  const std::vector< const SMDS_MeshNode* >& GetBndNodes() const { return _bndNodes; }

  // return UV of the i-th source boundary node (zero based)
  gp_XY GetBndUV(const int iNode) const;

  // return scale factor to convert real UV to/from UV used for Delaunay meshing:
  // delaunay_UV = real_UV * scale
  const gp_XY& GetScale() const { return _scale; }

  void ToPython() const;

  Handle(BRepMesh_DataStructureOfDelaun) GetDS() { return _triaDS; }

 protected:

  // container of a node and a triangle serving as a start while searching a
  // triangle including the node UV
  typedef std::list< std::pair< const SMDS_MeshNode*, const BRepMesh_Triangle* > > TNodeTriaList;

  // return UV of a node on the face
  virtual gp_XY getNodeUV( const TopoDS_Face& face, const SMDS_MeshNode* node ) const = 0;

  // add non-marked nodes surrounding a given one to a queue
  static void addCloseNodes( const SMDS_MeshNode*     node,
                             const BRepMesh_Triangle* bmTria,
                             const int                faceID,
                             TNodeTriaList &          noTriQueue );

  const TopoDS_Face&                     _face;
  int                                    _faceID;
  std::vector< const SMDS_MeshNode* >    _bndNodes;
  gp_XY                                  _scale;
  Handle(BRepMesh_DataStructureOfDelaun) _triaDS;
  size_t                                 _nbNodesToVisit, _nbVisitedNodes, _iBndNode;
  TNodeTriaList                          _noTriQueue;

};

#endif
