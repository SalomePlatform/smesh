//=============================================================================
// File      : SMESH_Regular_1D.hxx
// Created   : sam mai 18 08:11:54 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_REGULAR_1D_HXX_
#define _SMESH_REGULAR_1D_HXX_

#include "SMESH_1D_Algo.hxx"

class SMESH_LocalLength;
class SMESH_NumberOfSegments;

class SMESH_Regular_1D:
  public SMESH_1D_Algo
{
public:
  SMESH_Regular_1D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Regular_1D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_Regular_1D & hyp);
  friend istream & operator >> (istream & load, SMESH_Regular_1D & hyp);

protected:
  double _localLength;
  int _numberOfSegments;
  SMESH_LocalLength* _hypLocalLength;
  SMESH_NumberOfSegments* _hypNumberOfSegments;
};

#endif
