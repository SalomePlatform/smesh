//=============================================================================
// File      : SMESH_3D_Algo.hxx
// Created   : sam mai 18 09:24:47 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_3D_ALGO_HXX_
#define _SMESH_3D_ALGO_HXX_

#include "SMESH_Algo.hxx"

class SMESH_3D_Algo:
  public SMESH_Algo
{
public:
  SMESH_3D_Algo(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_3D_Algo();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, SMESH_3D_Algo & hyp);
  friend istream& operator >> (istream & load, SMESH_3D_Algo & hyp);

protected:
};

#endif
