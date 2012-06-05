// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
//  File   : StdMeshers_QuadraticMesh_i.hxx
//           Moved here from SMESH_LocalLength_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$
//
#ifndef _SMESH_QuadraticMesh_I_HXX_
#define _SMESH_QuadraticMesh_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_QuadraticMesh.hxx"

class SMESH_Gen;

// ======================================================
// Local Length hypothesis
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_QuadraticMesh_i:
  public virtual POA_StdMeshers::StdMeshers_QuadraticMesh,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_QuadraticMesh_i( PortableServer::POA_ptr thePOA,
                              int                     theStudyId,
                              ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_QuadraticMesh_i();

  // Get implementation
  ::StdMeshers_QuadraticMesh* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif

