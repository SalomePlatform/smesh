//=============================================================================
// File      : SMESH_Algo.hxx
// Created   : sam mai 18 09:20:46 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_ALGO_HXX_
#define _SMESH_ALGO_HXX_

#include "SMESH_Hypothesis.hxx"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>

#include <string>
#include <vector>
#include <list>

class SMESH_gen;
class SMESH_Mesh;

class SMESH_Algo:
  public SMESH_Hypothesis
{
public:
  SMESH_Algo(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Algo();

  const vector<string> & GetCompatibleHypothesis();
  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  virtual const list<SMESHDS_Hypothesis*>&
  GetUsedHypothesis(SMESH_Mesh& aMesh,
		    const TopoDS_Shape& aShape);

  const list<SMESHDS_Hypothesis*>&
  GetAppliedHypothesis(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  static double EdgeLength(const TopoDS_Edge& E);

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, SMESH_Algo & hyp);
  friend istream& operator >> (istream & load, SMESH_Algo & hyp);

protected:
  vector<string> _compatibleHypothesis;
  list<SMESHDS_Hypothesis*> _appliedHypList;
  list<SMESHDS_Hypothesis*> _usedHypList;
};

#endif
