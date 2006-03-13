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
//  File   : StdMeshers_MEFISTO_2D.hxx
//           Moved here from SMESH_MEFISTO_2D.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _StdMeshers_MEFISTO_2D_HXX_
#define _StdMeshers_MEFISTO_2D_HXX_

#include "SMESH_2D_Algo.hxx"
#include <TopoDS_Wire.hxx>

#include "StdMeshers_Helper.hxx"

class SMDS_MeshNode;
class TopTools_IndexedDataMapOfShapeListOfShape;
class TopoDS_Face;
class TopoDS_WIre;
class StdMeshers_MaxElementArea;
class StdMeshers_LengthFromEdges;
class SMDS_MeshNode;

#include <list>
#include <map>
#include "Rn.h"

class StdMeshers_MEFISTO_2D:
  public SMESH_2D_Algo
{
public:
  StdMeshers_MEFISTO_2D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~StdMeshers_MEFISTO_2D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  double ComputeEdgeElementLength(SMESH_Mesh& aMesh,
				  const TopoDS_Shape& aShape);

  bool LoadPoints(SMESH_Mesh& aMesh,
		  const TopoDS_Face& F, 
		  const TopoDS_Wire& W,
		  R2* uvslf, 
		  int& m,
		  map<int,const SMDS_MeshNode*>& mefistoToDS,
                  double scalex, double scaley,
                  const TopTools_IndexedDataMapOfShapeListOfShape& VWMap);

  void ComputeScaleOnFace(SMESH_Mesh& aMesh,
			  const TopoDS_Face& aFace,
			  double& scalex,
			  double& scaley);

  void StoreResult (SMESH_Mesh& aMesh,
		    Z nbst, R2* uvst, Z nbt, Z* nust, 
		    const TopoDS_Face& F, bool faceIsForward,
		    map<int,const SMDS_MeshNode*>& mefistoToDS,
                    double scalex, double scaley);
					  
  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, StdMeshers_MEFISTO_2D & hyp);
  friend istream & operator >> (istream & load, StdMeshers_MEFISTO_2D & hyp);

protected:
  double                            _edgeLength;
  double                            _maxElementArea;
  const StdMeshers_MaxElementArea*  _hypMaxElementArea;
  const StdMeshers_LengthFromEdges* _hypLengthFromEdges;

  TopoDS_Wire myOuterWire;
  std::list<const SMDS_MeshNode*> myNodesOnCommonV;

  StdMeshers_Helper* myTool; // toll for working with quadratic elements
};

#endif
