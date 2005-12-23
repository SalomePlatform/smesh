//  SMESH StdMeshers : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_QuadranglePreference.hxx
//  Module : SMESH
//  $Header$

#ifndef _StdMeshers_QuadranglePreference_HXX_
#define _StdMeshers_QuadranglePreference_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

/*!
 * \brief Hypothesis for StdMeshers_Quadrangle_2D, forcing construction
 *        of quadrangles if the number of nodes on opposite edges is not the same.
 *        GIBI can do it if the global number of nodes is even (DALL operator).
 *        See PAL10467
 */
class StdMeshers_QuadranglePreference:public SMESH_Hypothesis
{
 public:
  StdMeshers_QuadranglePreference(int hypId, int studyId, SMESH_Gen * gen);
  virtual ~ StdMeshers_QuadranglePreference();
  
  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);
  friend std::ostream & operator <<(std::ostream & save, StdMeshers_QuadranglePreference & hyp);
  friend std::istream & operator >>(std::istream & load, StdMeshers_QuadranglePreference & hyp);
};

#endif
