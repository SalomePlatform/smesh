//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : SMESH_HypothesisFactory.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HYPOTHESISFACTORY_HXX_
#define _SMESH_HYPOTHESISFACTORY_HXX_

#include "SMESH_Hypothesis.hxx"

#include "Utils_SALOME_Exception.hxx"

#include <map>

class SMESH_Gen;

class GenericHypothesisCreator
{
public:
//   virtual SMESH_Hypothesis* GetInstance(int hypId) = 0;
  virtual SMESH_Hypothesis* Create(int hypId, int studyId, SMESH_Gen* gen) = 0;
};

class SMESH_HypothesisFactory
{
public:
  SMESH_HypothesisFactory();
  virtual ~SMESH_HypothesisFactory();

  void SetGen(SMESH_Gen* gen);

  SMESH_Hypothesis* Create(const char* anHypName, int studyId)
    throw (SALOME_Exception);

  GenericHypothesisCreator* GetCreator(const char* anHypName)
    throw (SALOME_Exception);

  int GetANewId();

private:
  map<string, GenericHypothesisCreator*> _creatorMap;
  int _hypId;
  SMESH_Gen* _gen;
};

#endif
