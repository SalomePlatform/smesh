//  SMESH SMESH : idl implementation based on 'SMESH' unit's calsses
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
//  File   : StdMeshers_NumberOfLayers.hxx
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header$

#ifndef _SMESH_NumberOfLayers_HXX_
#define _SMESH_NumberOfLayers_HXX_

#include "SMESH_StdMeshers.hxx"

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_Gen;

// =========================================================
// =========================================================
/*!
 * This hypothesis is used by "Radial prism" algorithm.
 * It specifies number of segments between the internal 
 * and the external surfaces.
 */
// =========================================================
// =========================================================

class STDMESHERS_EXPORT StdMeshers_NumberOfLayers:  public SMESH_Hypothesis
{
public:
  // Constructor
  StdMeshers_NumberOfLayers( int hypId, int studyId, SMESH_Gen * gen );
  // Destructor
  virtual ~StdMeshers_NumberOfLayers();

  // Sets <number of segments> parameter value
  void SetNumberOfLayers(int numberOfLayers) throw ( SALOME_Exception );

  // Returns <number of layers> parameter value
  int GetNumberOfLayers() const;

  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);
  friend std::ostream & operator <<(std::ostream & save, StdMeshers_NumberOfLayers & hyp);
  friend std::istream & operator >>(std::istream & load, StdMeshers_NumberOfLayers & hyp);

  /*!
   * \brief Initialize parameters by the mesh built on the geometry
    * \param theMesh - the built mesh
    * \param theShape - the geometry of interest
    * \retval bool - true if parameter values have been successfully defined
    *
    * Implementation does noting
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

protected:
  int _nbLayers;
};

#endif

