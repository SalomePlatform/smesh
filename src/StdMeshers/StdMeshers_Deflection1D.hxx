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
//  File   : StdMeshers_Deflection1D.hxx
//  Module : SMESH
//  $Header$

#ifndef _StdMeshers_Deflection1D_HXX_
#define _StdMeshers_Deflection1D_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class StdMeshers_Deflection1D:public SMESH_Hypothesis
{
 public:
  StdMeshers_Deflection1D(int hypId, int studyId, SMESH_Gen * gen);
  virtual ~ StdMeshers_Deflection1D();

  void SetDeflection(double value) throw(SALOME_Exception);

  double GetDeflection() const;
  
  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream & operator <<(ostream & save, StdMeshers_Deflection1D & hyp);
  friend istream & operator >>(istream & load, StdMeshers_Deflection1D & hyp);

 protected:
  double _value;
};

#endif
