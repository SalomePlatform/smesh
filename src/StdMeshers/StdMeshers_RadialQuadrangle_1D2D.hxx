// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH : implementation of SMESH idl descriptions
//  File   : StdMeshers_RadialQuadrangle_1D2D.hxx
//  Module : SMESH
//
#ifndef _SMESH_RadialQuadrangle_1D2D_HXX_
#define _SMESH_RadialQuadrangle_1D2D_HXX_

#include "StdMeshers_Quadrangle_2D.hxx"

#include <vector>

class StdMeshers_NumberOfLayers;
class StdMeshers_LayerDistribution;

/*!
 * \brief Algorithm generating quadrangles on a full or a part of an elliptic face.
 *        Elements around an ellipse center are triangles.
 */

class STDMESHERS_EXPORT StdMeshers_RadialQuadrangle_1D2D: public StdMeshers_Quadrangle_2D
{
public:
  StdMeshers_RadialQuadrangle_1D2D(int hypId, SMESH_Gen* gen);
  virtual ~StdMeshers_RadialQuadrangle_1D2D();

  virtual bool CheckHypothesis(SMESH_Mesh&                          aMesh,
                               const TopoDS_Shape&                  aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape);

  virtual bool Evaluate(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape,
                        MapShapeNbElems& aResMap);

  // Allow algo to do something after persistent restoration
  virtual void SubmeshRestored(SMESH_subMesh* subMesh);

  // Sets event listener to a submesh
  virtual void SetEventListener(SMESH_subMesh* subMesh);

  virtual bool IsApplicableToShape(const TopoDS_Shape & shape, bool toCheckAll) const
  {
    return IsApplicable( shape, toCheckAll );
  }
  static bool IsApplicable(const TopoDS_Shape & aShape, bool toCheckAll);

 protected:

  int computeLayerPositions(StdMeshers_FaceSidePtr linSide,
                            std::vector< double >& positions,
                            int*                   nbEdgesComputed = 0,
                            bool                   useHalf = false);


  const StdMeshers_NumberOfLayers*    myNbLayerHypo;
  const StdMeshers_LayerDistribution* myDistributionHypo;
};

#endif
