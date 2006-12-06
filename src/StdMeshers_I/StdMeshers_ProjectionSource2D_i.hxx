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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : StdMeshers_ProjectionSource2D_i.hxx
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header$

#ifndef _SMESH_ProjectionSource2D_I_HXX_
#define _SMESH_ProjectionSource2D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_ProjectionSource2D.hxx"

class SMESH_Gen;

// =========================================================
/*!
 * This hypothesis specifies a meshed face to take a mesh pattern from
 * and optionally association of vertices between the source face and a
 * target one (where a hipothesis is assigned to)
 */
// =========================================================

class StdMeshers_ProjectionSource2D_i:
  public virtual POA_StdMeshers::StdMeshers_ProjectionSource2D,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_ProjectionSource2D_i( PortableServer::POA_ptr thePOA,
                                   int                     theStudyId,
                                   ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_ProjectionSource2D_i();

  /*!
   * Sets a source <face> to take a mesh pattern from
   */
  void SetSourceFace(GEOM::GEOM_Object_ptr face)
    throw ( SALOME::SALOME_Exception );

  /*!
   * Returns the source face
   */
  GEOM::GEOM_Object_ptr GetSourceFace();

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
   * Sets vertex association between the source face and the target one.
   * This parameter is optional.
   * Two vertices must belong to one edge of a face
   */
  void SetVertexAssociation(GEOM::GEOM_Object_ptr sourceVertex1,
                            GEOM::GEOM_Object_ptr sourceVertex2,
                            GEOM::GEOM_Object_ptr targetVertex1,
                            GEOM::GEOM_Object_ptr targetVertex2)
    throw ( SALOME::SALOME_Exception );

  /*!
   * Returns the <i>-th source vertex associated with the <i>-th target vertex.
   * Result may be nil if association not set.
   */
  GEOM::GEOM_Object_ptr GetSourceVertex(CORBA::Long i);

  /*!
   * Returns the <i>-th target vertex associated with the <i>-th source vertex.
   * Result may be nil if association not set.
   */
  GEOM::GEOM_Object_ptr GetTargetVertex(CORBA::Long i);

  // Get implementation
  ::StdMeshers_ProjectionSource2D* GetImpl();

  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );


  // Redefined Persistence
  virtual char* SaveTo();
  virtual void  LoadFrom( const char* theStream );

private:
  SMESH::SMESH_Mesh_var myCorbaMesh;
};

#endif

