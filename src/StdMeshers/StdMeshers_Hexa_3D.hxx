//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshers_Hexa_3D.hxx
//           Moved here from SMESH_Hexa_3D.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HEXA_3D_HXX_
#define _SMESH_HEXA_3D_HXX_

#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "StdMeshers_Quadrangle_2D.hxx"
#include "Utils_SALOME_Exception.hxx"

typedef struct point3Dstruct
{
	const SMDS_MeshNode * node;
} Point3DStruct;

typedef double Pt3[3];

typedef struct conv2dstruct
{
  double a1; // X = a1*x + b1*y + c1 
  double b1; // Y = a2*x + b2*y + c2
  double c1; // a1, b1 a2, b2 in {-1,0,1}
  double a2; // c1, c2 in {0,1}
  double b2;
  double c2;
  int ia;    // I = ia*i + ib*j + ic
  int ib;
  int ic;
  int ja;    // J = ja*i + jb*j + jc
  int jb;
  int jc;
} Conv2DStruct;

typedef struct cubeStruct
{
  TopoDS_Vertex V000;
  TopoDS_Vertex V001;
  TopoDS_Vertex V010;
  TopoDS_Vertex V011;
  TopoDS_Vertex V100;
  TopoDS_Vertex V101;
  TopoDS_Vertex V110;
  TopoDS_Vertex V111;
  faceQuadStruct* quad_X0;
  faceQuadStruct* quad_X1;
  faceQuadStruct* quad_Y0;
  faceQuadStruct* quad_Y1;
  faceQuadStruct* quad_Z0;
  faceQuadStruct* quad_Z1;
  Point3DStruct* np; // normalised 3D coordinates
} CubeStruct;

class StdMeshers_Hexa_3D:
  public SMESH_3D_Algo
{
public:
  StdMeshers_Hexa_3D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~StdMeshers_Hexa_3D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, StdMeshers_Hexa_3D & hyp);
  friend istream & operator >> (istream & load, StdMeshers_Hexa_3D & hyp);

protected:
  TopoDS_Edge
  EdgeNotInFace(SMESH_Mesh& aMesh,
		const TopoDS_Shape& aShape,
		const TopoDS_Face& aFace,
		const TopoDS_Vertex& aVertex,
		const TopTools_IndexedDataMapOfShapeListOfShape& MS);

  int GetFaceIndex(SMESH_Mesh& aMesh,
		   const TopoDS_Shape& aShape,
		   const vector<SMESH_subMesh*>& meshFaces,
		   const TopoDS_Vertex& V0,
		   const TopoDS_Vertex& V1,
		   const TopoDS_Vertex& V2,
		   const TopoDS_Vertex& V3);

  void GetConv2DCoefs(const faceQuadStruct& quad,
		      const TopoDS_Shape& aShape,
		      const TopoDS_Vertex& V0,
		      const TopoDS_Vertex& V1,
		      const TopoDS_Vertex& V2,
		      const TopoDS_Vertex& V3,
		      Conv2DStruct& conv);

  void GetPoint(Pt3 p,
		int i, int j, int k,
		int nbx, int nby, int nbz,
		Point3DStruct *np,
		const SMESHDS_Mesh* meshDS);

  CubeStruct _cube;
  FaceQuadStruct* _quads[6];
  int _indX0;
  int _indX1;
  int _indY0;
  int _indY1;
  int _indZ0;
  int _indZ1;
};

#endif
