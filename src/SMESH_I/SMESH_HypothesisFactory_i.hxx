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
//  File   : SMESH_HypothesisFactory_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HYPOTHESISFACTORY_I_HXX_
#define _SMESH_HYPOTHESISFACTORY_I_HXX_

#include "SMESH_Hypothesis_i.hxx"
#include <map>
#include <string>

class GenericHypothesisCreator_i
{
public:
  virtual SMESH_Hypothesis_i* Create(const char* anHyp,
				     int studyId,
				     ::SMESH_Gen* genImpl) = 0;
};

class SMESH_HypothesisFactory_i
{
public:
  SMESH_HypothesisFactory_i();
  virtual ~SMESH_HypothesisFactory_i();

  SMESH_Hypothesis_i* Create(const char* anHyp,
			     CORBA::Long studyId,
			     ::SMESH_Gen* genImpl)
    throw (SALOME::SALOME_Exception);

private:
  map<string, GenericHypothesisCreator_i*> _creatorMap;
};

#endif
