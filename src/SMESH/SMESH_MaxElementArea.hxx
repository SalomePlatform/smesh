//=============================================================================
// File      : SMESH_MaxElementArea.hxx
// Created   : sam mai 18 23:14:04 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_MAXELEMENTAREA_HXX_
#define _SMESH_MAXELEMENTAREA_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_MaxElementArea:
  public SMESH_Hypothesis
{
public:
  SMESH_MaxElementArea(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_MaxElementArea();

  void SetMaxArea(double maxArea)
    throw (SALOME_Exception);

  double GetMaxArea();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_MaxElementArea & hyp);
  friend istream & operator >> (istream & load, SMESH_MaxElementArea & hyp);

protected:
  double _maxArea;
};

#endif
