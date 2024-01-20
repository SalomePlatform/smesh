// Copyright (C) 2007-2024  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : StdMeshers_ViscousLayerBuilder_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_ViscousLayersBuilder_I_HXX_
#define _SMESH_ViscousLayersBuilder_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_2D_Algo_i.hxx"
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "StdMeshers_ViscousLayerBuilder.hxx"
#include "StdMeshers_ObjRefUlils.hxx"

class SMESH_Gen;

class STDMESHERS_I_EXPORT StdMeshers_ViscousLayerBuilder_i: 
  public virtual POA_StdMeshers::StdMeshers_ViscousLayerBuilder,
  public virtual SMESH_2D_Algo_i
{
 public:
  // Constructor
  StdMeshers_ViscousLayerBuilder_i( PortableServer::POA_ptr thePOA,
                                      ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_ViscousLayerBuilder_i();

  // Verify whether algorithm supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

  // Get implementation
  ::StdMeshers_ViscousLayerBuilder* GetImpl();  

  void SetIgnoreFaces(const ::SMESH::long_array& faceIDs);
  void SetFaces(const SMESH::long_array& faceIDs,
                CORBA::Boolean           toIgnore);
  void SetTotalThickness(::CORBA::Double thickness);
  void SetNumberLayers(::CORBA::Short nb);
  void SetStretchFactor(::CORBA::Double factor);
  void SetMethod( ::StdMeshers::VLExtrusionMethod how );
  void SetGroupName(const char* name);

  // Compute and return the shrink geometry 
  GEOM::GEOM_Object_ptr GetShrinkGeometry( SMESH::SMESH_Mesh_ptr finalMesh, GEOM::GEOM_Object_ptr theShapeObject );

  // Build the viscous layer on the specified faces/edges available in the sourceMesh and consolidate the result in the finalMesh
  CORBA::Boolean AddLayers( SMESH::SMESH_Mesh_ptr sourceMesh, SMESH::SMESH_Mesh_ptr finalMesh, GEOM::GEOM_Object_ptr theShapeObject );
  
};

#endif
