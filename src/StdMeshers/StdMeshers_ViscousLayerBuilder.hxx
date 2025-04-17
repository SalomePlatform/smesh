// Copyright (C) 2007-2025  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : StdMeshers_ViscousLayersBuilder.hxx
//  Module : SMESH
//
#ifndef _SMESH_ViscourLayerBuilder_HXX_
#define _SMESH_ViscourLayerBuilder_HXX_

#include <BRepOffset_MakeOffset.hxx>
#include <BRepOffset_Offset.hxx>

#include "SMESH_StdMeshers.hxx"
#include "SMESH_Hypothesis.hxx"
#include "StdMeshers_Cartesian_VL.hxx"
#include "StdMeshers_ViscousLayers.hxx"
#include "StdMeshers_ViscousLayers2D.hxx"


class STDMESHERS_EXPORT StdMeshers_ViscousLayerBuilder: public SMESH_2D_Algo
{
 public:
  StdMeshers_ViscousLayerBuilder(int hypId, SMESH_Gen* gen);
  ~StdMeshers_ViscousLayerBuilder();

  virtual bool CheckHypothesis(SMESH_Mesh&                          aMesh,
                               const TopoDS_Shape&                  aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& /*aMesh*/, const TopoDS_Shape& /*aShape*/ );

  virtual bool Evaluate(SMESH_Mesh & /*aMesh*/, const TopoDS_Shape & /*aShape*/,
                        MapShapeNbElems& /*aResMap*/ );

  /*!
   * \brief Check if the algo is applicable to the geometry and dimension
  */
  virtual bool IsApplicable( const TopoDS_Shape &S, bool /*toCheckAll*/, int algoDim );  

  void   SetBndShapes(const std::vector<int>& shapeIds, bool toIgnore);
  // std::vector<int> GetBndShapes() const { return _shapeIds; }
  // bool   IsToIgnoreShapes() const { return _isToIgnoreShapes; }

  void   SetTotalThickness(double thickness);
  // double GetTotalThickness() const { return _thickness; }

  void   SetNumberLayers(int nb);
  // int    GetNumberLayers() const { return _nbLayers; }

  void   SetStretchFactor(double factor);
  // double GetStretchFactor() const { return _stretchFactor; }

  void   SetMethod( StdMeshers_ViscousLayers::ExtrusionMethod how );
  // StdMeshers_ViscousLayers::ExtrusionMethod GetMethod() const { return _method; }

  // name of a group to create
  void SetGroupName(const std::string& name);
  // const std::string& GetGroupName() const { return _groupName; }

   /*!
   * \brief Compute a shrink version of the geometry. 
   *        Use the BRepOffset_MakeOffset to perform the operations for Solids. 
   *        Use BRepBuilderAPI_MakeFace to perform the operation for planar faces.
   * \remark For positive offsets, planar faces are shrink in all directions BRepBuilderAPI_MakeFace does not support coarse grained edge selection.
    * \param theMesh - the built mesh
    * \param theShape - the geometry to be shrink
    * \retval TopoDS_Shape - a new shape of the shrink geometry
   */
  TopoDS_Shape GetShrinkGeometry( SMESH_Mesh & theMesh, const TopoDS_Shape & theShape );

    /*!
   * \brief Build the elements of the viscous layer based on the shrinkMesh and copied to theMesh 
    * \param shrinkMesh - the mesh defined on the shrink geometry
    * \param theMesh - the final mesh with the combination of the shrink mesh and the viscous layer
    * \param theShape - the original geometry
    * \retval bool - Ok if success in the operation
   */
  bool AddLayers( SMESH_Mesh & shrinkMesh, SMESH_Mesh & theMesh, const TopoDS_Shape & theShape );

  private:
    
    
    StdMeshers_ViscousLayers2D*                     _hyp;
    StdMeshers_Cartesian_VL::ViscousBuilder*  _vlBuilder;

    BRepOffset_MakeOffset                     _makeOffset;
    BRepOffset_Offset                     _makeFaceOffset;
    TopoDS_Shape                             _offsetShape;
};

#endif
