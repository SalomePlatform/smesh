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
//  File   : StdMeshers_LengthFromEdges_i.hxx
//           Moved here from SMESH_LengthFromEdges_i.hxx
//  Author : Nadir BOUHAMOU CEA/DEN, Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_LENGTHFROMEDGES_I_HXX_
#define _SMESH_LENGTHFROMEDGES_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"
#include "StdMeshers_LengthFromEdges.hxx"

// ======================================================
// Length from edges hypothesis
// ======================================================
class StdMeshers_LengthFromEdges_i:
  public virtual POA_StdMeshers::StdMeshers_LengthFromEdges,
  public virtual SMESH_Hypothesis_i
{
public:
  // Constructor
  StdMeshers_LengthFromEdges_i( PortableServer::POA_ptr thePOA,
                           int                     theStudyId,
                           ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~StdMeshers_LengthFromEdges_i();

  // Set mode
  void SetMode( CORBA::Long theMode )
    throw ( SALOME::SALOME_Exception );
  // Get mode
  CORBA::Long GetMode();

  // Get implementation
  ::StdMeshers_LengthFromEdges* GetImpl();
};

#endif

