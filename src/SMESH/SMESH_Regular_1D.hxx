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
//  File   : SMESH_Regular_1D.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_REGULAR_1D_HXX_
#define _SMESH_REGULAR_1D_HXX_

#include "SMESH_1D_Algo.hxx"

class SMESH_LocalLength;
class SMESH_NumberOfSegments;

class SMESH_Regular_1D:
  public SMESH_1D_Algo
{
public:
  SMESH_Regular_1D(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Regular_1D();

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, SMESH_Regular_1D & hyp);
  friend istream & operator >> (istream & load, SMESH_Regular_1D & hyp);

protected:
  double _localLength;
  int _numberOfSegments;
  double _scaleFactor;
  const SMESH_LocalLength* _hypLocalLength;
  const SMESH_NumberOfSegments* _hypNumberOfSegments;
};

#endif
