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
//  File   : SMESH_MaxElementArea_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_MAXELEMENTAREA_I_HXX_
#define _SMESH_MAXELEMENTAREA_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_Hypothesis_i.hxx"

#include "SMESH_MaxElementArea.hxx"

class SMESH_MaxElementArea_i:
  public POA_SMESH::SMESH_MaxElementArea,
  public SMESH_Hypothesis_i
{
public:
  SMESH_MaxElementArea_i(const char* anHyp,
			 int studyId,
			 ::SMESH_Gen* genImpl);
  virtual ~SMESH_MaxElementArea_i();

  void SetMaxElementArea(CORBA::Double area)
    throw (SALOME::SALOME_Exception);

  CORBA::Double GetMaxElementArea();

protected:
  ::SMESH_MaxElementArea* _impl;
};

#endif
