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
//  File   : StdMeshers_Cartesian_3D_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_Cartesian_3D_I_HXX_
#define _SMESH_Cartesian_3D_I_HXX_

#include "SMESH_StdMeshers_I.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_3D_Algo_i.hxx"

class SMESH_Gen;
class StdMeshers_Cartesian_3D;

// ======================================================
// Cartesianedron 3d algorithm
// ======================================================
class STDMESHERS_I_EXPORT StdMeshers_Cartesian_3D_i:
  public virtual POA_StdMeshers::StdMeshers_Cartesian_3D,
  public virtual SMESH_3D_Algo_i
{
public:
  // Constructor
  StdMeshers_Cartesian_3D_i( PortableServer::POA_ptr thePOA,
                             int                     theStudyId,
                             ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_Cartesian_3D_i();

  // Get implementation
  ::StdMeshers_Cartesian_3D* GetImpl();
};

#endif
