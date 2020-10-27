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

//  SMESH SMESH : implementation of SMESH idl descriptions
//  File   : StdMeshers_BlockRenumber.hxx
//  Author : Edward AGAPOV, OCC
//  Module : SMESH
//
#ifndef _SMESH_BlockRenumber_HXX_
#define _SMESH_BlockRenumber_HXX_

#include "SMESH_StdMeshers.hxx"

#include "SMESH_ComputeError.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_MeshEditor.hxx"

#include <NCollection_DataMap.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS_Vertex.hxx>

#include <boost/serialization/vector.hpp>
#include <map>

class SMESH_Mesh;
class TopoDS_Shape;
class TopoDS_TShape;
class TopoDS_Vertex;

// =========================================================
struct StdMeshers_BlockCS // Local coordinate system of a block
{
  std::string _solid;
  std::string _vertex000;
  std::string _vertex001;

  bool operator==( const StdMeshers_BlockCS& other ) const
  {
    return ( _solid     == other._solid &&
             _vertex000 == other._vertex000 &&
             _vertex001 == other._vertex001 );
  }
};

// =========================================================
/*!
 * \class 3D Hypothesis used by Hexahedron(ijk) algorithm
 *        to renumber mesh of a block to be structured-like 
 */
// =========================================================

class STDMESHERS_EXPORT StdMeshers_BlockRenumber : public SMESH_Hypothesis
{
public:
  StdMeshers_BlockRenumber(int hypId, SMESH_Gen * gen);

  void SetBlocksOrientation( std::vector< StdMeshers_BlockCS > & blockCS );

  const std::vector< StdMeshers_BlockCS > &  GetBlocksOrientation() const { return _blockCS; }

  virtual std::ostream & SaveTo(std::ostream & save) override;
  virtual std::istream & LoadFrom(std::istream & load) override;

  /*!
   * \brief Initialize Fineness by the mesh built on the geometry
   *  \param theMesh - the built mesh
   *  \param theShape - the geometry of interest
   *  \retval bool - true if parameter values have been successfully defined
   */
  bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape) override
  { return false; }

  /*!
   * \brief Initialize my parameter values by default parameters.
   *  \retval bool - true if parameter values have been successfully defined
   */
  bool SetParametersByDefaults(const TDefaults&  dflts, const SMESH_Mesh* theMesh=0) override
  { return false; }

 public:

  /*!
   * \brief Check validity of parameter
   */
  SMESH_ComputeErrorPtr CheckHypothesis(SMESH_Mesh& theMesh, const TopoDS_Shape& theShape) const;

  /*!
   * \brief Return true and vertices if block orientation is defined for a given solid
   */
  bool IsSolidIncluded( SMESH_Mesh&         mesh,
                        const TopoDS_Shape& solid,
                        TopoDS_Vertex&      vertex000,
                        TopoDS_Vertex&      vertex001 ) const;

 private:

  // Persistence: define both input and output at once
  friend class boost::serialization::access;
  template<class Archive> void serialize( Archive & ar, const unsigned int version )
  {
    ar & _blockCS;
  }

 protected:

  std::vector< StdMeshers_BlockCS > _blockCS;

  typedef NCollection_DataMap< TopoDS_Shape, std::pair< TopoDS_Vertex, TopoDS_Vertex > > TSolid2VV;
  TSolid2VV                         _solids2vertices; // shapes defined by _blockCS, non-persistent
};

// =========================================================
/*!
 * \brief Help in using StdMeshers_BlockRenumber
 */
class StdMeshers_RenumberHelper
{
public:

  StdMeshers_RenumberHelper( SMESH_Mesh&                     mesh,
                             const StdMeshers_BlockRenumber* hyp);
  /*!
   * \brief Find default vertex at (0,0,0) local position
   */
  static TopoDS_Vertex GetVertex000( const TopTools_MapOfShape& cornerVertices );

  /*!
   * \brief Find a vertex of a solid located at the given point
   */
  static TopoDS_Vertex GetVertexAtPoint( const TopoDS_Shape& solid, const TopoDS_Shape& point );

  /*!
   * \brief Create a copy of an old node and remember this couple of nodes for replacement
   */
  void AddReplacingNode( const SMDS_MeshNode* & oldNode );

  /*!
   * \brief Replace old nodes by new ones
   */
  void DoReplaceNodes();

private:

  SMESH_Mesh*                     _mesh;
  const StdMeshers_BlockRenumber* _hyp;

  SMESH_MeshEditor::TListOfListOfNodes _nodesToMerge;
  std::list< const SMDS_MeshNode* >    _newOldNodes;
};

#endif
