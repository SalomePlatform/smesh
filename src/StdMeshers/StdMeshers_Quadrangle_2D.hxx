// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : StdMeshers_Quadrangle_2D.hxx
//           Moved here from SMESH_Quadrangle_2D.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

#ifndef _SMESH_QUADRANGLE_2D_HXX_
#define _SMESH_QUADRANGLE_2D_HXX_

#include "SMESH_Algo.hxx"
#include "SMESH_ProxyMesh.hxx"
#include "SMESH_StdMeshers.hxx"
#include "StdMeshers_QuadrangleParams.hxx"

#include <TopoDS_Face.hxx>

class SMDS_MeshNode;
class SMESH_Mesh;
class SMESH_MesherHelper;
class SMESH_ProxyMesh;
class StdMeshers_FaceSide;
struct uvPtStruct;


enum TSideID { QUAD_BOTTOM_SIDE=0, QUAD_RIGHT_SIDE, QUAD_TOP_SIDE, QUAD_LEFT_SIDE, NB_QUAD_SIDES };

typedef uvPtStruct UVPtStruct;
typedef struct faceQuadStruct
{
  std::vector< StdMeshers_FaceSide*> side;
  bool isEdgeOut[4]; // true, if an EDGE has more nodes, than an opposite one
  UVPtStruct* uv_grid;
  TopoDS_Face face;
  ~faceQuadStruct();
  void shift( size_t nb, bool keepUnitOri );
  typedef boost::shared_ptr<faceQuadStruct> Ptr;
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
                       const TopoDS_Shape& aShape);

  virtual bool Evaluate(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape,
                        MapShapeNbElems& aResMap);

  FaceQuadStruct::Ptr CheckAnd2Dcompute(SMESH_Mesh& aMesh,
                                        const TopoDS_Shape& aShape,
                                        const bool CreateQuadratic);

  FaceQuadStruct::Ptr CheckNbEdges(SMESH_Mesh& aMesh,
                                   const TopoDS_Shape& aShape);

protected:

  bool CheckNbEdgesForEvaluate(SMESH_Mesh& aMesh,
                               const TopoDS_Shape & aShape,
                               MapShapeNbElems& aResMap,
                               std::vector<int>& aNbNodes,
                               bool& IsQuadratic);

  bool SetNormalizedGrid(SMESH_Mesh&          aMesh,
                         const TopoDS_Shape&  aShape,
                         FaceQuadStruct::Ptr& quad);
  
  void SplitQuad(SMESHDS_Mesh *theMeshDS,
                 const int theFaceID,
                 const SMDS_MeshNode* theNode1,
                 const SMDS_MeshNode* theNode2,
                 const SMDS_MeshNode* theNode3,
                 const SMDS_MeshNode* theNode4);

  bool ComputeQuadPref(SMESH_Mesh&         aMesh,
                       const TopoDS_Shape& aShape,
                       FaceQuadStruct::Ptr quad);

  bool EvaluateQuadPref(SMESH_Mesh&         aMesh,
                        const TopoDS_Shape& aShape,
                        std::vector<int>&   aNbNodes,
                        MapShapeNbElems&    aResMap,
                        bool                isQuadratic);

  bool ComputeReduced (SMESH_Mesh&         aMesh,
                       const TopoDS_Shape& aShape,
                       FaceQuadStruct::Ptr quad);

  void UpdateDegenUV(FaceQuadStruct::Ptr quad);

  void Smooth (FaceQuadStruct::Ptr quad);


  // true if QuadranglePreference hypothesis is assigned that forces
  // construction of quadrangles if the number of nodes on opposite edges
  // is not the same in the case where the global number of nodes on edges
  // is even
  bool myQuadranglePreference;

  bool myTrianglePreference;

  int  myTriaVertexID;

  bool myNeedSmooth;

  StdMeshers_QuadType  myQuadType;

  SMESH_MesherHelper*  myHelper; // tool for working with quadratic elements

  SMESH_ProxyMesh::Ptr myProxyMesh;

  FaceQuadStruct::Ptr  myQuadStruct;
};

#endif
