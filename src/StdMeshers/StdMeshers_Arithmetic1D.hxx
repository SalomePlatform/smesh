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
//  File   : StdMeshers_Arithmetic1D.hxx
//  Author : Damien COQUERET, OCC
//  Module : SMESH
//  $Header$

#ifndef _SMESH_ARITHMETIC1D_HXX_
#define _SMESH_ARITHMETIC1D_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class StdMeshers_Arithmetic1D:
  public SMESH_Hypothesis
{
public:
  StdMeshers_Arithmetic1D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~StdMeshers_Arithmetic1D();

  void SetLength(double length, bool isStartLength) throw(SALOME_Exception);

  double GetLength(bool isStartLength) const;

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, StdMeshers_Arithmetic1D & hyp);
  friend istream& operator >> (istream & load, StdMeshers_Arithmetic1D & hyp);

protected:
  double _begLength, _endLength;
};

#endif
