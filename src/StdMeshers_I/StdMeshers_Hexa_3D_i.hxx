//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshers_Hexa_3D_i.hxx
//           Moved here from SMESH_Hexa_3D_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HEXA_3D_I_HXX_
#define _SMESH_HEXA_3D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_3D_Algo_i.hxx"
#include "StdMeshers_Hexa_3D.hxx"

class SMESH_Gen;

// ======================================================
// Hexaedron 3d algorithm
// ======================================================
class StdMeshers_Hexa_3D_i:
  public virtual POA_StdMeshers::StdMeshers_Hexa_3D,
  public virtual SMESH_3D_Algo_i
{
public:
  // Constructor
  StdMeshers_Hexa_3D_i( PortableServer::POA_ptr thePOA,
                   int                     theStudyId,
		   ::SMESH_Gen*            theGenImpl );

  // Destructor
  virtual ~StdMeshers_Hexa_3D_i();

  // Get implementation
  ::StdMeshers_Hexa_3D* GetImpl();
};

#endif
