//=============================================================================
// File      : SMESH_1D_Algo.hxx
// Created   : sam mai 18 09:23:02 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_1D_ALGO_HXX_
#define _SMESH_1D_ALGO_HXX_

#include "SMESH_Algo.hxx"

class SMESH_1D_Algo:
  public SMESH_Algo
{
public:
  SMESH_1D_Algo(int hypId, int studyId,  SMESH_Gen* gen);
  virtual ~SMESH_1D_Algo();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, SMESH_1D_Algo & hyp);
  friend istream& operator >> (istream & load, SMESH_1D_Algo & hyp);

protected:
};

#endif
