//=============================================================================
// File      : SMESH_NETGEN_3D.hxx
// Created   : lundi 27 Janvier 2003
// Author    : Nadir BOUHAMOU (CEA)
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================

#ifndef _SMESH_NETGEN_3D_HXX_
#define _SMESH_NETGEN_3D_HXX_

#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MaxElementVolume.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_NETGEN_3D: public SMESH_3D_Algo
{
public:
  SMESH_NETGEN_3D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_NETGEN_3D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_NETGEN_3D & hyp);
  friend istream & operator >> (istream & load, SMESH_NETGEN_3D & hyp);

protected:
  double _maxElementVolume;

  const SMESH_MaxElementVolume* _hypMaxElementVolume;
};

#endif
