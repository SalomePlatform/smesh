//=============================================================================
// File      : SMESH_Quadrangle_2D.hxx
// Created   : sam mai 18 08:11:36 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_QUADRANGLE_2D_HXX_
#define _SMESH_QUADRANGLE_2D_HXX_

#include "SMESH_2D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "Utils_SALOME_Exception.hxx"

typedef struct uvPtStruct
{
  double param;
  double normParam;
  double u; // original 2d parameter
  double v;
  double x; // 2d parameter, normalized [0,1]
  double y; 
  int nodeId;
} UVPtStruct;

typedef struct faceQuadStruct
{
  int nbPts[4];
  TopoDS_Edge edge[4];
  double first[4];
  double last[4];
  bool isEdgeForward[4];
  UVPtStruct* uv_edges[4];
  UVPtStruct* uv_grid;
} FaceQuadStruct;

class SMESH_Quadrangle_2D:
  public SMESH_2D_Algo
{
public:
  SMESH_Quadrangle_2D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Quadrangle_2D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  FaceQuadStruct* CheckAnd2Dcompute(SMESH_Mesh& aMesh,
				    const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  void QuadDelete(FaceQuadStruct* quad);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_Quadrangle_2D & hyp);
  friend istream & operator >> (istream & load, SMESH_Quadrangle_2D & hyp);

protected:

  void SetNormalizedGrid(SMESH_Mesh& aMesh,
			 const TopoDS_Shape& aShape,
			 FaceQuadStruct* quad)
    throw (SALOME_Exception);

  UVPtStruct* LoadEdgePoints(SMESH_Mesh& aMesh,
			     const TopoDS_Face& F,
			     const TopoDS_Edge& E,
			     double first,
			     double last);
// 			     bool isForward);

//   FaceQuadStruct _quadDesc;
};

#endif
