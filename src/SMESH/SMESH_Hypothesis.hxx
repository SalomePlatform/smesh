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
//  File   : SMESH_Hypothesis.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HYPOTHESIS_HXX_
#define _SMESH_HYPOTHESIS_HXX_

#include "SMESHDS_Hypothesis.hxx"

class SMESH_Gen;

class SMESH_Hypothesis: public SMESHDS_Hypothesis
{
public:
  SMESH_Hypothesis(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Hypothesis();
  int GetDim();
  int GetStudyId();
  void NotifySubMeshesHypothesisModification();
  int GetShapeType();

protected:
  SMESH_Gen* _gen;
  int _studyId;
  int _shapeType;
};

#endif
