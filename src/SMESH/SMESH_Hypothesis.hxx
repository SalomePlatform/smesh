//=============================================================================
// File      : SMESH_Hypothesis.hxx
// Created   : sam mai 18 08:07:54 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_HYPOTHESIS_HXX_
#define _SMESH_HYPOTHESIS_HXX_

#include "SMESHDS_Hypothesis.hxx"

class SMESH_Gen;

class SMESH_Hypothesis: public SMESHDS_Hypothesis
{
public:
  SMESH_Hypothesis(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Hypothesis();
  int GetDim();
  int GetStudyId();
  void NotifySubMeshesHypothesisModification();
  int GetShapeType();

protected:
  SMESH_Gen* _gen;
  int _studyId;
  int _shapeType;
};

#endif
