//=============================================================================
// File      : SMESH_2D_Algo.hxx
// Created   : sam mai 18 09:23:37 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_2D_ALGO_HXX_
#define _SMESH_2D_ALGO_HXX_

#include "SMESH_Algo.hxx"
#include <TopoDS_Wire.hxx>

class SMESH_2D_Algo:
  public SMESH_Algo
{
public:
  SMESH_2D_Algo(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_2D_Algo();

  int NumberOfWires(const TopoDS_Shape& S);
  int NumberOfPoints(SMESH_Mesh& aMesh,const TopoDS_Wire& W);

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, SMESH_2D_Algo & hyp);
  friend istream& operator >> (istream & load, SMESH_2D_Algo & hyp);

protected:
};

#endif
