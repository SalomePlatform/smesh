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

//  File   : StdMeshers_BlockRenumber_i.hxx
//  Author : Edward AGAPOV
//  Module : SMESH
//
#ifndef _SMESH_BlockRenumber_I_HXX_
#define _SMESH_BlockRenumber_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_BlockRenumber.hxx"

class SMESH_Gen;

// =========================================================
/*!
 * \class 3D Hypothesis used by Hexahedron(ijk) algorithm
 *        to renumber mesh of a block to be structured-like
 */
// =========================================================

class STDMESHERS_I_EXPORT StdMeshers_BlockRenumber_i:
  public virtual POA_StdMeshers::StdMeshers_BlockRenumber,
  public virtual SMESH_Hypothesis_i
{
public:

  StdMeshers_BlockRenumber_i( PortableServer::POA_ptr thePOA,
                              ::SMESH_Gen*            theGenImpl );

  // Set orientation of blocks
  void SetBlocksOrientation( const StdMeshers::blockcs_array& blockCS ) override;

  // Return orientation of blocks
  StdMeshers::blockcs_array* GetBlocksOrientation() override;


  // Get implementation
  ::StdMeshers_BlockRenumber* GetImpl();

  // Verify whether hypothesis supports given entity type
  CORBA::Boolean IsDimSupported( SMESH::Dimension type ) override;

  // Methods for copying mesh definition to other geometry
  bool getObjectsDependOn( std::vector< std::string > & entryArray,
                           std::vector< int >         & subIDArray ) const override;
  bool setObjectsDependOn( std::vector< std::string > & entryArray,
                           std::vector< int >         & subIDArray ) override;
};

#endif
