//  SMESH StdMeshers_I : idl implementation based on 'SMESH' unit's classes
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
//  File   : StdMeshers_NotConformAllowed_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _StdMeshers_NotConformAllowed_I_HXX_
#define _StdMeshers_NotConformAllowed_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

#include "StdMeshers_NotConformAllowed.hxx"

class StdMeshers_NotConformAllowed_i:
  public POA_StdMeshers::StdMeshers_NotConformAllowed,
  public SMESH_Hypothesis_i
{
public:
  StdMeshers_NotConformAllowed_i(PortableServer::POA_ptr thePOA,
                                 int studyId,
                                 ::SMESH_Gen* genImpl);
  virtual ~StdMeshers_NotConformAllowed_i();

protected:
  ::StdMeshers_NotConformAllowed* _impl;
};

#endif

