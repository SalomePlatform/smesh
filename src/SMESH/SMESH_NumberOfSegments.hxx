//=============================================================================
// File      : SMESH_NumberOfSegments.hxx
// Created   : sam mai 18 08:11:20 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_NUMBEROFSEGMENTS_HXX_
#define _SMESH_NUMBEROFSEGMENTS_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_NumberOfSegments:
  public SMESH_Hypothesis
{
public:
  SMESH_NumberOfSegments(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_NumberOfSegments();

  void SetNumberOfSegments(int segmentsNumber)
    throw (SALOME_Exception);

  int GetNumberOfSegments();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, SMESH_NumberOfSegments & hyp);
  friend istream& operator >> (istream & load, SMESH_NumberOfSegments & hyp);

protected:
  int _numberOfSegments;
};

#endif
