//=============================================================================
// File      : SMESH_MEFISTO_2D.hxx
// Created   : sam mai 18 08:10:50 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_MEFISTO_2D_HXX_
#define _SMESH_MEFISTO_2D_HXX_

#include "SMESH_2D_Algo.hxx"
#include "SMESH_MaxElementArea.hxx"
#include "SMESH_LengthFromEdges.hxx"
#include "Rn.h"

#include <TopoDS_Face.hxx>

#include <map>

class SMESH_MEFISTO_2D:
  public SMESH_2D_Algo
{
public:
  SMESH_MEFISTO_2D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_MEFISTO_2D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  double ComputeEdgeElementLength(SMESH_Mesh& aMesh,
				  const TopoDS_Shape& aShape);

  void LoadPoints(SMESH_Mesh& aMesh,
		  const TopoDS_Face& F, 
		  const TopoDS_Wire& W,
		  R2* uvslf, 
		  int& m,
		  map<int,int>& mefistoToDS);

  void ComputeScaleOnFace(SMESH_Mesh& aMesh,
			  const TopoDS_Face& aFace,
			  double& scalex,
			  double& scaley);

  void StoreResult (SMESH_Mesh& aMesh,
		    Z nbst, R2* uvst, Z nbt, Z* nust, 
		    const TopoDS_Face& F, bool faceIsForward,
		    map<int,int>& mefistoToDS);
					  
  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_MEFISTO_2D & hyp);
  friend istream & operator >> (istream & load, SMESH_MEFISTO_2D & hyp);

protected:
  double _edgeLength;
  double _maxElementArea;
  SMESH_MaxElementArea* _hypMaxElementArea;
  SMESH_LengthFromEdges* _hypLengthFromEdges;
};

#endif
