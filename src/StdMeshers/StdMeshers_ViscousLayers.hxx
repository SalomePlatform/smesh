// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File      : StdMeshers_ViscousLayers.hxx
// Created   : Wed Dec  1 15:15:34 2010
// Author    : Edward AGAPOV (eap)

#ifndef __StdMeshers_ViscousLayers_HXX__
#define __StdMeshers_ViscousLayers_HXX__

#include "SMESH_StdMeshers.hxx"

#include "SMESH_Hypothesis.hxx"
#include "SMESH_ProxyMesh.hxx"

#include <vector>

/*!
 * \brief Hypothesis defining parameters of viscous layers
 */
class STDMESHERS_EXPORT StdMeshers_ViscousLayers : public SMESH_Hypothesis
{
public:
  StdMeshers_ViscousLayers(int hypId, int studyId, SMESH_Gen* gen);

  // Set boundary shapes to exclude from treatment, faces in 3D, edges in 2D
  void SetBndShapesToIgnore(const std::vector<int>& shapeIds);
  std::vector<int> GetBndShapesToIgnore() const { return _ignoreBndShapeIds; }
  bool IsIgnoredShape(const int shapeID) const;

  // Set total thickness of layers of prisms
  void SetTotalThickness(double thickness);
  double GetTotalThickness() const { return _thickness; }

  // Set number of layers of prisms
  void SetNumberLayers(int nb);
  int GetNumberLayers() const { return _nbLayers; }

  // Set factor (>1.0) of growth of layer thickness towards inside of mesh
  void SetStretchFactor(double factor);
  double GetStretchFactor() const { return _stretchFactor; }

  // Computes temporary 2D mesh to be used by 3D algorithm.
  // Return SMESH_ProxyMesh for each SOLID in theShape
  SMESH_ProxyMesh::Ptr Compute(SMESH_Mesh&         theMesh,
                               const TopoDS_Shape& theShape,
                               const bool          toMakeN2NMap=false) const;

  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);

  /*!
   * \brief Initialize my parameter values by the mesh built on the geometry
    * \param theMesh - the built mesh
    * \param theShape - the geometry of interest
    * \retval bool - true if parameter values have been successfully defined
    *
    * Just return false as this hypothesis does not have parameters values
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

  /*!
   * \brief Initialize my parameter values by default parameters.
   *  \retval bool - true if parameter values have been successfully defined
   */
  virtual bool SetParametersByDefaults(const TDefaults& dflts, const SMESH_Mesh* theMesh=0)
  { return false; }

  static const char* GetHypType() { return "ViscousLayers"; }

 private:

  std::vector<int> _ignoreBndShapeIds;
  int              _nbLayers;
  double           _thickness;
  double           _stretchFactor;
};

class SMESH_subMesh;
namespace VISCOUS_3D
{
  // sets a sub-mesh event listener to clear sub-meshes of sub-shapes of
  // the main shape when sub-mesh of the main shape is cleared,
  // for example to clear sub-meshes of FACEs when sub-mesh of a SOLID
  // is cleared
  void ToClearSubWithMain( SMESH_subMesh* sub, const TopoDS_Shape& main);
}

#endif
