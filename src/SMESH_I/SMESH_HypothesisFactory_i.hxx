//=============================================================================
// File      : SMESH_HypothesisFactory_i.hxx
// Created   : dim mai 19 22:02:45 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

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
