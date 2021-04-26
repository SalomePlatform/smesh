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
//  File   : StdMeshers_FixedPoints1D.hxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//
#ifndef _SMESH_FIXEDPOINTS1D_HXX_
#define _SMESH_FIXEDPOINTS1D_HXX_

#include "SMESH_StdMeshers.hxx"

#include "StdMeshers_Reversible1D.hxx"
#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"
#include <smIdType.hxx>

#include <vector>

class STDMESHERS_EXPORT StdMeshers_FixedPoints1D: public StdMeshers_Reversible1D
{
public:
  StdMeshers_FixedPoints1D(int hypId, SMESH_Gen* gen);
  virtual ~StdMeshers_FixedPoints1D();

  void SetPoints(const std::vector<double>& listParams);

  void SetNbSegments(const std::vector<smIdType>& listNbSeg) ;

  const std::vector<double>& GetPoints() const { return _params; }

  const std::vector<smIdType>& GetNbSegments() const { return _nbsegs; }

  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);

  /*!
   * \brief Initialize start and end length by the mesh built on the geometry
   *  \param theMesh - the built mesh
   *  \param theShape - the geometry of interest
   *  \retval bool - true if parameter values have been successfully defined
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

  /*!
   * \brief Initialize my parameter values by default parameters.
   *  \retval bool - true if parameter values have been successfully defined
   */
  virtual bool SetParametersByDefaults(const TDefaults& dflts, const SMESH_Mesh* theMesh=0);

 protected:
  std::vector<double>   _params;
  std::vector<smIdType> _nbsegs;
};

#endif
