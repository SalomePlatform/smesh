//=============================================================================
// File      : SMESH_LocalLength.hxx
// Created   : sam mai 18 08:10:19 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_LOCALLENGTH_HXX_
#define _SMESH_LOCALLENGTH_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_LocalLength:
  public SMESH_Hypothesis
{
public:
  SMESH_LocalLength(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_LocalLength();

  void SetLength(double length)
    throw (SALOME_Exception);

  double GetLength();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_LocalLength & hyp);
  friend istream & operator >> (istream & load, SMESH_LocalLength & hyp);

protected:
  double _length;
};

#endif
