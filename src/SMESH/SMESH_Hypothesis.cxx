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
//  File   : SMESH_Hypothesis.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_subMesh.hxx"
#include "utilities.h"

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Hypothesis::SMESH_Hypothesis(int hypId,
				   int studyId,
				   SMESH_Gen* gen) : SMESHDS_Hypothesis(hypId)
{
  //MESSAGE("SMESH_Hypothesis::SMESH_Hypothesis");
  _gen = gen;
  _studyId = studyId;
  StudyContextStruct* myStudyContext = _gen->GetStudyContext(_studyId);
  myStudyContext->mapHypothesis[_hypId] = this;
  _type = PARAM_ALGO;
  _shapeType = 0; // to be set by algo with TopAbs_Enum
  _param_algo_dim = -1; // to be set by algo parameter
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Hypothesis::~SMESH_Hypothesis()
{
  MESSAGE("SMESH_Hypothesis::~SMESH_Hypothesis");
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESH_Hypothesis::GetDim() const
{
  int dim = -1;
  switch (_type)
    {
    case ALGO_1D: dim = 1; break;
    case ALGO_2D: dim = 2; break;
    case ALGO_3D: dim = 3; break;
    case PARAM_ALGO: dim = _param_algo_dim; break;
    }
  return dim;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_Hypothesis::GetShapeType() const
{
  return _shapeType;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESH_Hypothesis::GetStudyId() const
{
  return _studyId;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_Hypothesis::NotifySubMeshesHypothesisModification()
{
  MESSAGE("SMESH_Hypothesis::NotifySubMeshesHypothesisModification");

  // for all meshes in study

  StudyContextStruct* myStudyContext = _gen->GetStudyContext(_studyId);
  map<int, SMESH_Mesh*>::iterator itm;
  for (itm = myStudyContext->mapMesh.begin();
       itm != myStudyContext->mapMesh.end();
       itm++)
    {
      SMESH_Mesh* mesh = (*itm).second;
      const list<SMESH_subMesh*>& subMeshes =
	 mesh->GetSubMeshUsingHypothesis(this);

      //for all subMeshes using hypothesis
	
      list<SMESH_subMesh*>::const_iterator its;
      for (its = subMeshes.begin(); its != subMeshes.end(); its++)
	(*its)->ComputeStateEngine(SMESH_subMesh::MODIF_HYP);
    }
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const char* SMESH_Hypothesis::GetLibName() const
{
//   MESSAGE("SMESHDS_Hypothesis::GetLibName");
//   SCRUTE(_LibName);
//   SCRUTE(&_LibName);
  return _libName.c_str();
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_Hypothesis::SetLibName(const char* theLibName)
{
//   MESSAGE("SMESHDS_Hypothesis::SetLibName");
  _libName = string(theLibName);
}
