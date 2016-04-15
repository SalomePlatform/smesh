// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : StdMeshers_ProjectionSource1D_i.hxx
//  Author : Edward AGAPOV
//  Module : SMESH
//
#ifndef _SMESH_ProjectionSource1D_I_HXX_
#define _SMESH_ProjectionSource1D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_ProjectionSource1D.hxx"

class SMESH_Gen;

// =========================================================
/*!
 * This hypothesis specifies a meshed edge to take a mesh pattern from
 * and optionally association of vertices between the source edge and a
 * target one (where a hipothesis is assigned to)
 */
// =========================================================

class StdMeshers_ProjectionSource1D_i:
  public virtual POA_StdMeshers::StdMeshers_ProjectionSource1D,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_ProjectionSource1D_i( PortableServer::POA_ptr thePOA,
                                   int                     theStudyId,
                                   ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_ProjectionSource1D_i();

  /*!
   * Sets source <edge> or a group containing edges to take a mesh pattern from
   */
  void SetSourceEdge(GEOM::GEOM_Object_ptr edge)
    throw ( SALOME::SALOME_Exception );

  /*!
   * Returns the source edge or a group containing edges
   */
  GEOM::GEOM_Object_ptr GetSourceEdge();

  /*!
   * Sets source <mesh> to take a mesh pattern from
   */
  void SetSourceMesh(SMESH::SMESH_Mesh_ptr mesh)
    throw ( SALOME::SALOME_Exception );

  /*!
   * Return source mesh
   */
  SMESH::SMESH_Mesh_ptr GetSourceMesh();

  /*!
   * Sets vertex association between the source edge and the target one.
   * This parameter is optional
   */
  void SetVertexAssociation(GEOM::GEOM_Object_ptr sourceVertex,
                            GEOM::GEOM_Object_ptr targetVertex)
    throw ( SALOME::SALOME_Exception );

  /*!
   * Returns the vertex associated with the target vertex.
   * Result may be nil if association not set
   */
  GEOM::GEOM_Object_ptr GetSourceVertex();

  /*!
   * Returns the vertex associated with the source vertex.
   * Result may be nil if association not set
   */
  GEOM::GEOM_Object_ptr GetTargetVertex();

  // Get implementation
  ::StdMeshers_ProjectionSource1D* GetImpl();

  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );


  // Redefined Persistence
  virtual char* SaveTo();
  virtual void  LoadFrom( const char* theStream );

private:
  // keep entries because the same shape can be published several times with
  // different names and in this case a correct name can't be restored by a TopoDS_Shape
  // kept by ::StdMeshers_ProjectionSource1D
  enum { SRC_EDGE=0, SRC_VERTEX, TGT_VERTEX, NB_SHAPES };
  std::string           myShapeEntries[NB_SHAPES];
  SMESH::SMESH_Mesh_var myCorbaMesh;
};

#endif
