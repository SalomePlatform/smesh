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
//  File   : StdMeshers_LocalLength.hxx
//           Moved here from SMESH_LocalLength.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_LOCALLENGTH_HXX_
#define _SMESH_LOCALLENGTH_HXX_

#include "SMESH_StdMeshers.hxx"

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class STDMESHERS_EXPORT StdMeshers_LocalLength: public SMESH_Hypothesis
{
 public:
  StdMeshers_LocalLength(int hypId, int studyId, SMESH_Gen * gen);
  virtual ~ StdMeshers_LocalLength();

  void SetLength(double length) throw(SALOME_Exception);
  void SetPrecision(double precision) throw(SALOME_Exception);

  double GetLength() const;
  double GetPrecision() const;

  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);
  friend std::ostream & operator <<(std::ostream & save, StdMeshers_LocalLength & hyp);
  friend std::istream & operator >>(std::istream & load, StdMeshers_LocalLength & hyp);

  /*!
   * \brief Initialize segment length by the mesh built on the geometry
   * \param theMesh - the built mesh
   * \param theShape - the geometry of interest
   * \retval bool - true if parameter values have been successfully defined
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

 protected:
  double _length;
  double _precision;
};

#endif
