// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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

//  File   : StdMeshers_PolyhedronPerSolid_3D.hxx
//  Module : SMESH
//
#ifndef _SMESH_PolyhedronPerSolid_3D_HXX_
#define _SMESH_PolyhedronPerSolid_3D_HXX_

#include "SMESH_StdMeshers.hxx"
#include "SMESH_Algo.hxx"

class StdMeshers_Regular_1D;
class StdMeshers_PolygonPerFace_2D;
class StdMeshers_ViscousLayers;

class STDMESHERS_EXPORT StdMeshers_PolyhedronPerSolid_3D: public SMESH_3D_Algo
{
 public:
  StdMeshers_PolyhedronPerSolid_3D(int hypId, SMESH_Gen* gen);
  ~StdMeshers_PolyhedronPerSolid_3D();

  virtual bool CheckHypothesis(SMESH_Mesh&                          aMesh,
                               const TopoDS_Shape&                  aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape);

  virtual bool Evaluate(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape,
                        MapShapeNbElems& aResMap);

 private:

  StdMeshers_Regular_1D*          myEdgeMesher;
  StdMeshers_PolygonPerFace_2D*   myFaceMesher;
  const StdMeshers_ViscousLayers* myViscousLayersHyp;
};

#endif
