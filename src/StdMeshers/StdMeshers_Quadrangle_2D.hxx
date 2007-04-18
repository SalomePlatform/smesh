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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : StdMeshers_Quadrangle_2D.hxx
//           Moved here from SMESH_Quadrangle_2D.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_QUADRANGLE_2D_HXX_
#define _SMESH_QUADRANGLE_2D_HXX_

#include "SMESH_StdMeshers.hxx"

#include "SMESH_2D_Algo.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_Mesh;
class SMESH_MesherHelper;
class StdMeshers_FaceSide;
struct uvPtStruct;

//class SMDS_MeshNode;

enum TSideID { BOTTOM_SIDE=0, RIGHT_SIDE, TOP_SIDE, LEFT_SIDE, NB_SIDES };

typedef uvPtStruct UVPtStruct;
typedef struct faceQuadStruct
{
  //int nbPts[4];
  //TopoDS_Edge edge[4];
  vector< StdMeshers_FaceSide*> side;
  //double first[4];
  //double last[4];
  //bool isEdgeForward[4];
  bool isEdgeOut[4]; // true, if an edge has more nodes, than the opposite
  //UVPtStruct* uv_edges[4];
  UVPtStruct* uv_grid;
  ~faceQuadStruct();
} FaceQuadStruct;

class STDMESHERS_EXPORT StdMeshers_Quadrangle_2D: public SMESH_2D_Algo
{
public:
  StdMeshers_Quadrangle_2D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~StdMeshers_Quadrangle_2D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  FaceQuadStruct* CheckAnd2Dcompute(SMESH_Mesh& aMesh,
				    const TopoDS_Shape& aShape,
                                    const bool CreateQuadratic)
    throw (SALOME_Exception);

protected:

  FaceQuadStruct* CheckNbEdges(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape)
    throw (SALOME_Exception);

  bool SetNormalizedGrid(SMESH_Mesh& aMesh,
			 const TopoDS_Shape& aShape,
			 FaceQuadStruct*& quad)
    throw (SALOME_Exception);

  /**
   * Special function for creation only quandrangle faces
   */
  bool ComputeQuadPref(SMESH_Mesh& aMesh,
                       const TopoDS_Shape& aShape,
                       FaceQuadStruct* quad)
    throw (SALOME_Exception);

  UVPtStruct* LoadEdgePoints2(SMESH_Mesh& aMesh,
			      const TopoDS_Face& F, const TopoDS_Edge& E,
                              bool IsReverse);

  UVPtStruct* LoadEdgePoints(SMESH_Mesh& aMesh,
			     const TopoDS_Face& F, const TopoDS_Edge& E,
			     double first, double last);

  UVPtStruct* MakeEdgePoints(SMESH_Mesh& aMesh,
			     const TopoDS_Face& F, const TopoDS_Edge& E,
			     double first, double last, int nb_segm);

  // true if QuadranglePreference hypothesis is assigned that forces
  // construction of quadrangles if the number of nodes on opposite edges
  // is not the same in the case where the global number of nodes on edges is even
  bool myQuadranglePreference;

  SMESH_MesherHelper* myTool; // tool for working with quadratic elements
};

#endif
