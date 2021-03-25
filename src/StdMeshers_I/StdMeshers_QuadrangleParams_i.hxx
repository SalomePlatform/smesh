// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_QuadrangleParams_i.hxx
//  Author : Sergey KUUL, OCC
//  Module : SMESH

#ifndef _SMESH_QUADRANGLEPARAMS_I_HXX_
#define _SMESH_QUADRANGLEPARAMS_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_QuadrangleParams.hxx"

// ======================================================
// QuadrangleParams hypothesis
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_QuadrangleParams_i:
  public virtual POA_StdMeshers::StdMeshers_QuadrangleParams,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_QuadrangleParams_i (PortableServer::POA_ptr thePOA,
                                 ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~StdMeshers_QuadrangleParams_i();

  // Set base vertex for triangles
  void SetTriaVertex (CORBA::Long vertID);

  // Get base vertex for triangles
  CORBA::Long GetTriaVertex();
  
  // Set the Entry of the Object
  void SetObjectEntry (const char* theEntry);

  // Get Object Entry
  char* GetObjectEntry();

  // Set the type of quadrangulation
  void SetQuadType (StdMeshers::QuadType type);

  // Get the type of quadrangulation
  StdMeshers::QuadType GetQuadType();

  // Set positions of enforced nodes
  void SetEnforcedNodes(const GEOM::ListOfGO&     vertices,
                        const SMESH::nodes_array& points);
  
  // Returns positions of enforced nodes
  void GetEnforcedNodes(GEOM::ListOfGO_out vertices, SMESH::nodes_array_out points);

  // Returns entries of shapes defining enforced nodes
  SMESH::string_array* GetEnfVertices();

  // Set corner vertices
  void SetCorners( const SMESH::long_array& vertexIDs );

  // Return IDs of corner vertices
  SMESH::long_array* GetCorners();

  // Get implementation
  ::StdMeshers_QuadrangleParams* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

  // Redefined Persistence
  virtual char* SaveTo();
  virtual void  LoadFrom( const char* theStream );


  // Methods for copying mesh definition to other geometry

  // Return geometry this hypothesis depends on. Return false if there is no geometry parameter
  virtual bool getObjectsDependOn( std::vector< std::string > & entryArray,
                                   std::vector< int >         & subIDArray ) const;

  // Set new geometry instead of that returned by getObjectsDependOn()
  virtual bool setObjectsDependOn( std::vector< std::string > & entryArray,
                                   std::vector< int >         & subIDArray );
 protected:

  std::vector<std::string> myShapeEntries;
  
};

#endif
