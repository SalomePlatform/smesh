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
//  File   : StdMeshers_UseExisting_1D2D.hxx
//  Module : SMESH
//  $Header$
//
#ifndef _SMESH_UseExisting_1D2D_I_HXX_
#define _SMESH_UseExisting_1D2D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_1D_Algo_i.hxx"
#include "SMESH_2D_Algo_i.hxx"
#include "StdMeshers_UseExisting_1D2D.hxx"

class SMESH_Gen;

/*!
 * \brief 1D and 2D algorithms doing nothing to allow mesh generation
 * by mesh edition functions in TUI mode
 */

class StdMeshers_UseExisting_1D_i:
  public virtual POA_StdMeshers::StdMeshers_UseExisting_1D,
  public virtual SMESH_1D_Algo_i
{
public:
  // Constructor
  StdMeshers_UseExisting_1D_i( PortableServer::POA_ptr thePOA,
                               int                     theStudyId,
                               ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_UseExisting_1D_i();

  // Get implementation
  ::StdMeshers_UseExisting_1D* GetImpl();
};

class StdMeshers_UseExisting_2D_i:
  public virtual POA_StdMeshers::StdMeshers_UseExisting_2D,
  public virtual SMESH_2D_Algo_i
{
public:
  // Constructor
  StdMeshers_UseExisting_2D_i( PortableServer::POA_ptr thePOA,
                               int                     theStudyId,
                               ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_UseExisting_2D_i();

  // Get implementation
  ::StdMeshers_UseExisting_2D* GetImpl();
};


#endif
