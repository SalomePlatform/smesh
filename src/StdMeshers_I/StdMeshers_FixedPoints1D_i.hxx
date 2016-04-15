// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_FixedPoints1D_i.hxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//
#ifndef _SMESH_FIXEDPOINTS1D_I_HXX_
#define _SMESH_FIXEDPOINTS1D_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_FixedPoints1D.hxx"

// ======================================================
// Fixed points 1D hypothesis
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_FixedPoints1D_i:
  public virtual POA_StdMeshers::StdMeshers_FixedPoints1D,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_FixedPoints1D_i( PortableServer::POA_ptr thePOA,
                              int                     theStudyId,
                              ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_FixedPoints1D_i();

  // Sets some points on edge using parameter on curve from 0 to 1
  // (additionally it is neecessary to check orientation of edges and
  // create list of reversed edges if it is needed) and sets numbers
  // of segments between given points (default values are equals 1)
  void SetPoints(const SMESH::double_array& listParams) 
    throw ( SALOME::SALOME_Exception );
  void SetNbSegments(const SMESH::long_array& listNbSeg) 
    throw ( SALOME::SALOME_Exception );

  // Returns list of point's parameters
  SMESH::double_array* GetPoints();
  
  // Returns list of numbers of segments
  SMESH::long_array* GetNbSegments();
    
  //Set Reversed Edges
  void SetReversedEdges( const SMESH::long_array& theIDs);

  //Get Reversed Edges
  SMESH::long_array*  GetReversedEdges();
  
  //Set the Entry of the Object
  void SetObjectEntry( const char* theEntry);

  //Get Object Entry
  char* GetObjectEntry();

  // Get implementation
  ::StdMeshers_FixedPoints1D* GetImpl();

  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
