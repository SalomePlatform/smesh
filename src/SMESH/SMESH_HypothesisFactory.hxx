//=============================================================================
// File      : SMESH_HypothesisFactory.hxx
// Created   : mer mai 15 13:45:47 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

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
