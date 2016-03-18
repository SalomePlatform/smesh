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
//  File   : StdMeshers_Projection_1D_2D_3D_i.hxx
//  Module : SMESH
//
#ifndef _SMESH_Projection_1D2D3D_I_HXX_
#define _SMESH_Projection_1D2D3D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_1D_Algo_i.hxx"
#include "SMESH_2D_Algo_i.hxx"
#include "SMESH_3D_Algo_i.hxx"
#include "StdMeshers_Projection_1D.hxx"
#include "StdMeshers_Projection_1D2D.hxx"
#include "StdMeshers_Projection_2D.hxx"
#include "StdMeshers_Projection_3D.hxx"

class SMESH_Gen;
class TopoDS_Shape;

// ======================================================
// Projection 3D algorithm
// ======================================================

class StdMeshers_Projection_3D_i:
  public virtual POA_StdMeshers::StdMeshers_Projection_3D,
  public virtual SMESH_3D_Algo_i
{
public:
  // Constructor
  StdMeshers_Projection_3D_i( PortableServer::POA_ptr thePOA,
                              int                     theStudyId,
                              ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_Projection_3D_i();

  // Get implementation
  ::StdMeshers_Projection_3D* GetImpl();

  // Return true if the algorithm is applicable to a shape
  static CORBA::Boolean IsApplicable(const TopoDS_Shape &S, CORBA::Boolean toCheckAll);
};

// ======================================================
// Projection 2D algorithm
// ======================================================

class StdMeshers_Projection_2D_i:
  public virtual POA_StdMeshers::StdMeshers_Projection_2D,
  public virtual SMESH_2D_Algo_i
{
public:
  // Constructor
  StdMeshers_Projection_2D_i( PortableServer::POA_ptr thePOA,
                              int                     theStudyId,
                              ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_Projection_2D_i();

  // Get implementation
  ::StdMeshers_Projection_2D* GetImpl();
};

// ======================================================
// Projection 1D-2D algorithm
// ======================================================

class StdMeshers_Projection_1D2D_i:
  public virtual POA_StdMeshers::StdMeshers_Projection_1D2D,
  public virtual SMESH_2D_Algo_i
{
public:
  // Constructor
  StdMeshers_Projection_1D2D_i( PortableServer::POA_ptr thePOA,
                                int                     theStudyId,
                                ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_Projection_1D2D_i();

  // Get implementation
  ::StdMeshers_Projection_1D2D* GetImpl();
};

// ======================================================
// Projection 1D algorithm
// ======================================================

class StdMeshers_Projection_1D_i:
  public virtual POA_StdMeshers::StdMeshers_Projection_1D,
  public virtual SMESH_1D_Algo_i
{
public:
  // Constructor
  StdMeshers_Projection_1D_i( PortableServer::POA_ptr thePOA,
                              int                     theStudyId,
                              ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_Projection_1D_i();

  // Get implementation
  ::StdMeshers_Projection_1D* GetImpl();
};


#endif
