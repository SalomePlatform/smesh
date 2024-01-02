// Copyright (C) 2007-2023  CEA, EDF, OPEN CASCADE
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
//  File   : SMESH_ParallelMesh_i.hxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH

#ifndef _SMESH_PARALLELMESH_I_HXX_
#define _SMESH_PARALLELMESH_I_HXX_

#include "SMESH.hxx"

#include "SMESH_Hypothesis.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_ParallelMesh.hxx"

#include <SALOME_GenericObj_i.hh>
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <map>

class SMESH_I_EXPORT SMESH_ParallelMesh_i:
  public virtual POA_SMESH::SMESH_ParallelMesh,
  public virtual SMESH_Mesh_i
{
  SMESH_ParallelMesh_i();
  SMESH_ParallelMesh_i(const SMESH_ParallelMesh_i&);
  public:
  SMESH_ParallelMesh_i( PortableServer::POA_ptr thePOA,
                        SMESH_Gen_i*            myGen_i ):SMESH_Mesh_i(thePOA, myGen_i){};

  virtual ~SMESH_ParallelMesh_i(){};

  CORBA::Long GetParallelismMethod();
  void SetParallelismMethod(CORBA::Long aMethod);

  CORBA::Long GetParallelismDimension();
  void SetParallelismDimension(CORBA::Long aDim);

  CORBA::Long GetNbThreads();
  void SetNbThreads(CORBA::Long nbThreads);

  char* GetResource();
  void SetResource(const char* aResource);

  CORBA::Long GetNbProc();
  void SetNbProc(CORBA::Long nbProcs);

  CORBA::Long GetNbProcPerNode();
  void SetNbProcPerNode(CORBA::Long nbProcPerNodes);

  CORBA::Long GetNbNode();
  void SetNbNode(CORBA::Long nbNodes);

  char* GetWcKey();
  void SetWcKey(const char* wcKey);

  char* GetWalltime();
  void SetWalltime(const char* walltime);

  private:
  ::SMESH_ParallelMesh* DownCast();
};

#endif
