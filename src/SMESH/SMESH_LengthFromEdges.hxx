//=============================================================================
// File      : SMESH_LengthFromEdges.hxx
// Created   : mar jun 11 22:42:20 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_LENGTHFROMEDGES_HXX_
#define _SMESH_LENGTHFROMEDGES_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_LengthFromEdges:
  public SMESH_Hypothesis
{
public:
  SMESH_LengthFromEdges(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_LengthFromEdges();

  void SetMode(int mode)
    throw (SALOME_Exception);

  int GetMode();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_LengthFromEdges & hyp);
  friend istream & operator >> (istream & load, SMESH_LengthFromEdges & hyp);

protected:
  int _mode;
};

#endif
