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
//  File   : SMESH_Algo.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_ALGO_HXX_
#define _SMESH_ALGO_HXX_

#include "SMESH_Hypothesis.hxx"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>

#include <string>
#include <vector>
#include <list>

class SMESH_gen;
class SMESH_Mesh;

class SMESH_Algo:
  public SMESH_Hypothesis
{
public:
  SMESH_Algo(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Algo();

  const vector<string> & GetCompatibleHypothesis();
  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape);

  virtual bool Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  virtual const list<SMESHDS_Hypothesis*>&
  GetUsedHypothesis(SMESH_Mesh& aMesh,
		    const TopoDS_Shape& aShape);

  const list<SMESHDS_Hypothesis*>&
  GetAppliedHypothesis(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape);

  static double EdgeLength(const TopoDS_Edge& E);

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, SMESH_Algo & hyp);
  friend istream& operator >> (istream & load, SMESH_Algo & hyp);

protected:
  vector<string> _compatibleHypothesis;
  list<SMESHDS_Hypothesis*> _appliedHypList;
  list<SMESHDS_Hypothesis*> _usedHypList;
};

#endif
