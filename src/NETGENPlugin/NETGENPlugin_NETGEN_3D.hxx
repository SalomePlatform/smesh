//=============================================================================
// File      : NETGENPlugin_NETGEN_3D.hxx
//             Moved here from SMESH_NETGEN_3D.hxx
// Created   : lundi 27 Janvier 2003
// Author    : Nadir BOUHAMOU (CEA)
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================

#ifndef _NETGENPlugin_NETGEN_3D_HXX_
#define _NETGENPlugin_NETGEN_3D_HXX_

#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "StdMeshers_MaxElementVolume.hxx"
#include "Utils_SALOME_Exception.hxx"

class NETGENPlugin_NETGEN_3D: public SMESH_3D_Algo
{
public:
  NETGENPlugin_NETGEN_3D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~NETGENPlugin_NETGEN_3D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, NETGENPlugin_NETGEN_3D & hyp);
  friend istream & operator >> (istream & load, NETGENPlugin_NETGEN_3D & hyp);

protected:
  double _maxElementVolume;

  const StdMeshers_MaxElementVolume* _hypMaxElementVolume;
};

#endif
