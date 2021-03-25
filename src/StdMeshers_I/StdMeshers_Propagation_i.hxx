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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//  File   : StdMeshers_Propagation_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_PROPAGATION_I_HXX_
#define _SMESH_PROPAGATION_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_Propagation.hxx"

class SMESH_Gen;

// ======================================================
// Propagation hypothesis
// ======================================================

class STDMESHERS_I_EXPORT StdMeshers_Propagation_i:
  public virtual POA_StdMeshers::StdMeshers_Propagation,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_Propagation_i (PortableServer::POA_ptr thePOA,
                            ::SMESH_Gen*            theGenImpl);
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

  // Methods for copying mesh definition to other geometry
  virtual bool getObjectsDependOn( std::vector< std::string > & /*entryArray*/,
                                   std::vector< int >         & /*subIDArray*/ ) const { return false; }
  virtual bool setObjectsDependOn( std::vector< std::string > & /*entryArray*/,
                                   std::vector< int >         & /*subIDArray*/ ) { return true; }
};

// ======================================================
// Propagation of Distribution hypothesis
// ======================================================

class STDMESHERS_I_EXPORT StdMeshers_PropagOfDistribution_i:
  public virtual POA_StdMeshers::StdMeshers_PropagOfDistribution,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_PropagOfDistribution_i (PortableServer::POA_ptr thePOA,
                                     ::SMESH_Gen*            theGenImpl);
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );

  // Methods for copying mesh definition to other geometry
  virtual bool getObjectsDependOn( std::vector< std::string > & /*entryArray*/,
                                   std::vector< int >         & /*subIDArray*/ ) const { return false; }
  virtual bool setObjectsDependOn( std::vector< std::string > & /*entryArray*/,
                                   std::vector< int >         & /*subIDArray*/ ) { return true; }
};

#endif
