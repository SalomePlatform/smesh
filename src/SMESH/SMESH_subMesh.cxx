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
//  File   : SMESH_subMesh.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_subMesh.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Algo.hxx"
#include "utilities.h"
#include "OpUtil.hxx"

#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================

SMESH_subMesh::SMESH_subMesh(int Id, SMESH_Mesh * father, SMESHDS_Mesh * meshDS,
	const TopoDS_Shape & aSubShape)
{
	//MESSAGE("SMESH_subMesh::SMESH_subMesh");
	_subShape = aSubShape;
	_meshDS = meshDS;
	_subMeshDS = meshDS->MeshElements(_subShape);	// may be null ...
	_father = father;
	_Id = Id;
	_vertexSet = false;			// only for Vertex subMesh
	_dependenceAnalysed = false;
	_dependantsFound = false;

	if (_subShape.ShapeType() == TopAbs_VERTEX)
	{
		_algoState = HYP_OK;
		_computeState = READY_TO_COMPUTE;
	}
	else
	{
		_algoState = NO_ALGO;
		_computeState = NOT_READY;
	}
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_subMesh::~SMESH_subMesh()
{
	MESSAGE("SMESH_subMesh::~SMESH_subMesh");
	// ****
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESH_subMesh::GetId()
{
	//MESSAGE("SMESH_subMesh::GetId");
	return _Id;
}

//=============================================================================
/*!
 * Given a subShape, find the subMesh is associated to this subShape or
 * to a collection of shapes containing this subShape. Collection = compsolid,
 * shell, wire
 */
//=============================================================================

// bool SMESH_subMesh::Contains(const TopoDS_Shape & aSubShape)
//   throw (SALOME_Exception)
// {
//   //MESSAGE("SMESH_subMesh::Contains");
//   bool contains = false;
//   int type = _subShape.ShapeType();
//   int typesub = aSubShape.ShapeType();
//   //SCRUTE(type)
//   //SCRUTE(typesub)
//   switch (type)
//     {
// //     case TopAbs_COMPOUND:
// //       {
// //   //MESSAGE("---");
// //   throw SALOME_Exception(LOCALIZED("Compound not yet treated"));
// //   break;
// //       }
//     case TopAbs_COMPSOLID:
//       {
//  //MESSAGE("---");
//  for (TopExp_Explorer exp(aSubShape,TopAbs_SOLID);exp.More();exp.Next())
//    {
//      contains = _subShape.IsSame(exp.Current());
//      if (contains) break;
//    }
//  break;
//       }
//     case TopAbs_SHELL:
//       {
//  //MESSAGE("---");
//  for (TopExp_Explorer exp(aSubShape,TopAbs_FACE);exp.More();exp.Next())
//    {
//      contains = _subShape.IsSame(exp.Current());
//      if (contains) break;
//    }
//  break;
//       }
//     case TopAbs_WIRE:
//       {
//  //MESSAGE("---");
//  for (TopExp_Explorer exp(aSubShape,TopAbs_EDGE);exp.More();exp.Next())
//    {
//      contains = _subShape.IsSame(exp.Current());
//      if (contains) break;
//    }
//  break;
//       }
//     case TopAbs_COMPOUND:
//     case TopAbs_SOLID:
//     case TopAbs_FACE:
//     case TopAbs_EDGE:
//     case TopAbs_VERTEX:
//       {
//  //MESSAGE("---");
//  contains = _subShape.IsSame(aSubShape);
//  break;
//       }
//     default:
//       {
//  break;
//       }
//     }
//   //SCRUTE(contains);
//   return contains;
// }

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESHDS_SubMesh * SMESH_subMesh::GetSubMeshDS() throw(SALOME_Exception)
{
	//MESSAGE("SMESH_subMesh::GetSubMeshDS");
	if (_subMeshDS==NULL)
	{
		//MESSAGE("subMesh pointer still null, trying to get it...");
		_subMeshDS = _meshDS->MeshElements(_subShape);	// may be null ...
		if (_subMeshDS==NULL)
		{
			MESSAGE("problem... subMesh still empty");
			//NRI   ASSERT(0);
			//NRI   throw SALOME_Exception(LOCALIZED(subMesh still empty));
		}
	}
	return _subMeshDS;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_subMesh *SMESH_subMesh::GetFirstToCompute() throw(SALOME_Exception)
{
	//MESSAGE("SMESH_subMesh::GetFirstToCompute");
	const map < int, SMESH_subMesh * >&subMeshes = DependsOn();
	SMESH_subMesh *firstToCompute = 0;

	map < int, SMESH_subMesh * >::const_iterator itsub;
	for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
	{
		SMESH_subMesh *sm = (*itsub).second;
//       SCRUTE(sm->GetId());
//       SCRUTE(sm->GetComputeState());
		bool readyToCompute = (sm->GetComputeState() == READY_TO_COMPUTE);
		if (readyToCompute)
		{
			firstToCompute = sm;
			//SCRUTE(sm->GetId());
			break;
		}
	}
	if (firstToCompute)
	{
		//MESSAGE("--- submesh to compute");
		return firstToCompute;	// a subMesh of this
	}
	if (_computeState == READY_TO_COMPUTE)
	{
		//MESSAGE("--- this to compute");
		return this;			// this
	}
	//MESSAGE("--- nothing to compute");
	return 0;					// nothing to compute
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

bool SMESH_subMesh::SubMeshesComputed() throw(SALOME_Exception)
{
	//MESSAGE("SMESH_subMesh::SubMeshesComputed");
	const map < int, SMESH_subMesh * >&subMeshes = DependsOn();

	bool subMeshesComputed = true;
	map < int, SMESH_subMesh * >::const_iterator itsub;
	for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
	{
		SMESH_subMesh *sm = (*itsub).second;

		const TopoDS_Shape & ss = sm->GetSubShape();
		int type = ss.ShapeType();
//       SCRUTE(sm->GetId());
//       SCRUTE(sm->GetComputeState());
		bool computeOk = (sm->GetComputeState() == COMPUTE_OK);
		if (!computeOk)
		{
			subMeshesComputed = false;

			switch (type)
			  {
			  case TopAbs_COMPOUND:
			    {
			      MESSAGE("The not computed sub mesh is a COMPOUND");
			      break;
			    }
			  case TopAbs_COMPSOLID:
			    {
			      MESSAGE("The not computed sub mesh is a COMPSOLID");
			      break;
			    }
			  case TopAbs_SHELL:
			    {
			      MESSAGE("The not computed sub mesh is a SHEL");
			      break;
			    }
			  case TopAbs_WIRE:
			    {
			      MESSAGE("The not computed sub mesh is a WIRE");
			      break;
			    }
			  case TopAbs_SOLID:
			    {
			      MESSAGE("The not computed sub mesh is a SOLID");
			      break;
			    }
			  case TopAbs_FACE:
			    {
			      MESSAGE("The not computed sub mesh is a FACE");
			      break;
			    }
			  case TopAbs_EDGE:
			    {
			      MESSAGE("The not computed sub mesh is a EDGE");
			      break;
			    }
			  default:
			    {
			      MESSAGE("The not computed sub mesh is of unknown type");
			      break;
			    }
			  }

			SCRUTE(sm->GetId());
			break;
		}
	}
	return subMeshesComputed;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

bool SMESH_subMesh::SubMeshesReady()
{
	MESSAGE("SMESH_subMesh::SubMeshesReady");
	const map < int, SMESH_subMesh * >&subMeshes = DependsOn();

	bool subMeshesReady = true;
	map < int, SMESH_subMesh * >::const_iterator itsub;
	for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
	{
		SMESH_subMesh *sm = (*itsub).second;
//       SCRUTE(sm->GetId());
//       SCRUTE(sm->GetComputeState());
		bool computeOk = ((sm->GetComputeState() == COMPUTE_OK)
			|| (sm->GetComputeState() == READY_TO_COMPUTE));
		if (!computeOk)
		{
			subMeshesReady = false;
			SCRUTE(sm->GetId());
			break;
		}
	}
	return subMeshesReady;
}

//=============================================================================
/*!
 * Construct dependence on first level subMeshes. complex shapes (compsolid, 
 * shell, wire) are not analysed the same way as simple shapes (solid, face,
 * edge). 
 * For collection shapes (compsolid, shell, wire) prepare a list of submeshes
 * with possible multiples occurences. Multiples occurences corresponds to
 * internal frontiers within shapes of the collection and must not be keeped.
 * See FinalizeDependence.
 */
//=============================================================================

const map < int, SMESH_subMesh * >&SMESH_subMesh::DependsOn()
{
	if (_dependenceAnalysed)
		return _mapDepend;

	//MESSAGE("SMESH_subMesh::DependsOn");

	int type = _subShape.ShapeType();
	//SCRUTE(type);
	switch (type)
	{
	case TopAbs_COMPOUND:
	{
		//MESSAGE("compound");
		list < TopoDS_Shape > shellInSolid;
		for (TopExp_Explorer exp(_subShape, TopAbs_SOLID); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
			for (TopExp_Explorer
				exp2(exp.Current(), TopAbs_SHELL); exp2.More(); exp2.Next())
			{
				shellInSolid.push_back(exp2.Current());
			}
		}
		for (TopExp_Explorer exp(_subShape, TopAbs_SHELL); exp.More();
			exp.Next())
		{
			list < TopoDS_Shape >::iterator it1;
			bool isInSolid = false;
			for (it1 = shellInSolid.begin(); it1 != shellInSolid.end(); it1++)
			{
				TopoDS_Shape aShape = (*it1);
				if (aShape.IsSame(exp.Current()))
				{
					isInSolid = true;
					break;
				}
			}
			if (!isInSolid)
				InsertDependence(exp.Current());	//only shell not in solid
		}
		for (TopExp_Explorer exp(_subShape, TopAbs_FACE); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
		for (TopExp_Explorer exp(_subShape, TopAbs_EDGE); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
		break;
	}
	case TopAbs_COMPSOLID:
	{
		//MESSAGE("compsolid");
		for (TopExp_Explorer exp(_subShape, TopAbs_SOLID); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
//  list<TopoDS_Shape> shapeList;
//  for (TopExp_Explorer exp(_subShape,TopAbs_SOLID);exp.More();exp.Next())
//    {
//      for (TopExp_Explorer 
//         exp2(exp.Current(),TopAbs_FACE);exp2.More();exp2.Next())
//        {
//      shapeList.push_back(exp2.Current()); 
//        }
//    }
//  FinalizeDependence(shapeList);
		break;
	}
	case TopAbs_SHELL:
	{
		//MESSAGE("shell");
		for (TopExp_Explorer exp(_subShape, TopAbs_FACE); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
//  list<TopoDS_Shape> shapeList;
//  for (TopExp_Explorer exp(_subShape,TopAbs_FACE);exp.More();exp.Next())
//    {
//      for (TopExp_Explorer 
//         exp2(exp.Current(),TopAbs_EDGE);exp2.More();exp2.Next())
//        {
//      shapeList.push_back(exp2.Current()); 
//        }
//    }
//  FinalizeDependence(shapeList);
		break;
	}
	case TopAbs_WIRE:
	{
		//MESSAGE("wire");
		for (TopExp_Explorer exp(_subShape, TopAbs_EDGE); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
//  list<TopoDS_Shape> shapeList;
//  for (TopExp_Explorer exp(_subShape,TopAbs_EDGE);exp.More();exp.Next())
//    {
//      for (TopExp_Explorer 
//         exp2(exp.Current(),TopAbs_VERTEX);exp2.More();exp2.Next())
//        {
//      shapeList.push_back(exp2.Current()); 
//        }
//    }
//  FinalizeDependence(shapeList);
		break;
	}
	case TopAbs_SOLID:
	{
		//MESSAGE("solid");
//  for (TopExp_Explorer exp(_subShape,TopAbs_SHELL);exp.More();exp.Next())
//    {
//      InsertDependence(exp.Current());
//    }
		for (TopExp_Explorer exp(_subShape, TopAbs_FACE); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
		break;
	}
	case TopAbs_FACE:
	{
		//MESSAGE("face");
//  for (TopExp_Explorer exp(_subShape,TopAbs_WIRE);exp.More();exp.Next())
//    {
//      InsertDependence(exp.Current());
//    }
		for (TopExp_Explorer exp(_subShape, TopAbs_EDGE); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
		break;
	}
	case TopAbs_EDGE:
	{
		//MESSAGE("edge");
		for (TopExp_Explorer exp(_subShape, TopAbs_VERTEX); exp.More();
			exp.Next())
		{
			InsertDependence(exp.Current());
		}
		break;
	}
	case TopAbs_VERTEX:
	{
		break;
	}
	default:
	{
		break;
	}
	}
	_dependenceAnalysed = true;
	return _mapDepend;
}

//=============================================================================
/*!
 * For simple Shapes (solid, face, edge): add subMesh into dependence list.
 */
//=============================================================================

void SMESH_subMesh::InsertDependence(const TopoDS_Shape aSubShape)
{
	//MESSAGE("SMESH_subMesh::InsertDependence");
	//SMESH_subMesh* aSubMesh = _father->GetSubMeshContaining(aSubShape);
	//SCRUTE(aSubMesh);
	//if (! aSubMesh) aSubMesh = _father->GetSubMesh(aSubShape);

	SMESH_subMesh *aSubMesh = _father->GetSubMesh(aSubShape);
	int type = aSubShape.ShapeType();
	int ordType = 9 - type;		// 2 = Vertex, 8 = CompSolid
	int cle = aSubMesh->GetId();
	cle += 10000000 * ordType;	// sort map by ordType then index
	if (_mapDepend.find(cle) == _mapDepend.end())
	{
		_mapDepend[cle] = aSubMesh;
		const map < int, SMESH_subMesh * >&subMap = aSubMesh->DependsOn();
		map < int, SMESH_subMesh * >::const_iterator im;
		for (im = subMap.begin(); im != subMap.end(); im++)
		{
			int clesub = (*im).first;
			SMESH_subMesh *sm = (*im).second;
			if (_mapDepend.find(clesub) == _mapDepend.end())
				_mapDepend[clesub] = sm;
		}
	}

}

//=============================================================================
/*!
 * For collection shapes (compsolid, shell, wire).
 * Add only subMesh figuring only once in multiset to dependence list 
 */
//=============================================================================

// void SMESH_subMesh::FinalizeDependence(list<TopoDS_Shape>& shapeList)
// {
//   //MESSAGE("SMESH_subMesh::FinalizeDependence");
//   list<TopoDS_Shape>::iterator it1, it2;
//   for(it1 = shapeList.begin(); it1 != shapeList.end(); it1++)
//     {
//       TopoDS_Shape aSubShape = (*it1);
//       int count = 0;
//       for(it2 = shapeList.begin(); it2 != shapeList.end(); it2++)
//  {
//    TopoDS_Shape other = (*it2);
//    if (other.IsSame(aSubShape)) count++;
//  }
//       if (count == 1) InsertDependence(aSubShape);
//       SCRUTE(count);
//     }
// }

//=============================================================================
/*!
 * 
 */
//=============================================================================

const TopoDS_Shape & SMESH_subMesh::GetSubShape()
{
	//MESSAGE("SMESH_subMesh::GetSubShape");
	return _subShape;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

bool SMESH_subMesh::AlgoStateEngine(int event, SMESH_Hypothesis * anHyp)
throw(SALOME_Exception)
{
	//  MESSAGE("SMESH_subMesh::AlgoStateEngine");
	//SCRUTE(_algoState);
	//SCRUTE(event);

	// **** les retour des evenement shape sont significatifs
	// (add ou remove fait ou non)
	// le retour des evenement father n'indiquent pas que add ou remove fait
	int dim = SMESH_Gen::GetShapeDim(_subShape);

	if (dim < 1)
	{
		_algoState = HYP_OK;
		//SCRUTE(_algoState);
		return true;
	}

	SMESH_Gen *gen = _father->GetGen();
	bool ret;
	_oldAlgoState = _algoState;
	bool modifiedHyp = false;	// if set to true, force event MODIF_ALGO_STATE
	// in ComputeStateEngine

	switch (_algoState)
	{

		// ----------------------------------------------------------------------

	case NO_ALGO:
		switch (event)
		{
		case ADD_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->AddHypothesis(_subShape, anHyp);
			break;
		case ADD_ALGO:
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
			if (anHyp->GetDim() <= SMESH_Gen::GetShapeDim(_subShape))
			{
				ret = _meshDS->AddHypothesis(_subShape, anHyp);
//        if (ret &&(anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape)))
//        if (ret &&(anHyp->GetShapeType() == _subShape.ShapeType()))
				if (ret &&
					(anHyp->GetShapeType() & (1 << _subShape.ShapeType())))
				{
					SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
					ASSERT(algo);
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		case REMOVE_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
			break;
		case REMOVE_ALGO:
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
			break;
		case ADD_FATHER_HYP:	// nothing to do
			break;
		case ADD_FATHER_ALGO:	// Algo just added in father
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
//    if (anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape))
//    if (anHyp->GetShapeType() == _subShape.ShapeType())
			if (anHyp->GetShapeType() & (1 << _subShape.ShapeType()))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				ASSERT(algo);
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					SetAlgoState(HYP_OK);
				else
					SetAlgoState(MISSING_HYP);
			}
			break;
		case REMOVE_FATHER_HYP:	// nothing to do
			break;
		case REMOVE_FATHER_ALGO:	// nothing to do
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------

	case MISSING_HYP:
		switch (event)
		{
		case ADD_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->AddHypothesis(_subShape, anHyp);
			if (ret)
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				ASSERT(algo);
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					SetAlgoState(HYP_OK);
				else
					SetAlgoState(MISSING_HYP);
			}
			break;
		case ADD_ALGO:			//already existing algo : on father ?
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
			if (anHyp->GetDim() <= SMESH_Gen::GetShapeDim(_subShape))
			{
				ret = _meshDS->AddHypothesis(_subShape, anHyp);
//        if (ret &&(anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape)))
//        if (ret &&(anHyp->GetShapeType() == _subShape.ShapeType()))
				if (ret &&
					(anHyp->GetShapeType() & (1 << _subShape.ShapeType())))
				{
					SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
					if (algo == NULL)	// two algo on the same subShape...
					{
						MESSAGE("two algo on the same subshape not allowed");
						ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
						ret = false;
					}
					else
					{
						ret = algo->CheckHypothesis((*_father), _subShape);
						if (ret)
							SetAlgoState(HYP_OK);
						else
							SetAlgoState(MISSING_HYP);
					}
				}
			}
			break;
		case REMOVE_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
			break;
		case REMOVE_ALGO:		// perhaps a father algo applies ?
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
//    if (ret &&(anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape)))
//    if (ret &&(anHyp->GetShapeType() == _subShape.ShapeType()))
			if (ret && (anHyp->GetShapeType() & (1 << _subShape.ShapeType())))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				if (algo == NULL)	// no more algo applying on subShape...
				{
					SetAlgoState(NO_ALGO);
				}
				else
				{
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		case ADD_FATHER_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				ASSERT(algo);
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					SetAlgoState(HYP_OK);
				else
					SetAlgoState(MISSING_HYP);
			}
			break;
		case ADD_FATHER_ALGO:	// detect if two algo of same dim on father
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
//    if (anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape))
//    if (anHyp->GetShapeType() == _subShape.ShapeType())
			if (anHyp->GetShapeType() & (1 << _subShape.ShapeType()))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				if (algo == NULL)	// two applying algo on father
				{
					MESSAGE("two applying algo on fatherShape...");
					SetAlgoState(NO_ALGO);
				}
				else
				{
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		case REMOVE_FATHER_HYP:	// nothing to do
			break;
		case REMOVE_FATHER_ALGO:
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
//    if (anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape))
//    if (anHyp->GetShapeType() == _subShape.ShapeType())
			if (anHyp->GetShapeType() & (1 << _subShape.ShapeType()))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				if (algo == NULL)	// no more applying algo on father
				{
					SetAlgoState(NO_ALGO);
				}
				else
				{
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------

	case HYP_OK:
		switch (event)
		{
		case ADD_HYP:
		{
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
			ASSERT(algo);
			list<const SMESHDS_Hypothesis *> originalUsedHyps = algo->GetUsedHypothesis((*_father), _subShape);	// copy

			ret = _meshDS->AddHypothesis(_subShape, anHyp);
			if (ret)
			{
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (!ret)
				{
					INFOS("two applying algo on the same shape not allowed");
					ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
					ret = false;
				}
				else			// compare SMESHDS_Hypothesis* lists (order important)
				{
					MESSAGE("---");
					const list <const SMESHDS_Hypothesis *> & newUsedHyps
						= algo->GetUsedHypothesis((*_father), _subShape);
					modifiedHyp = (originalUsedHyps != newUsedHyps);
				}
			}
		}
			break;
		case ADD_ALGO:			//already existing algo : on father ?
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
			if (anHyp->GetDim() <= SMESH_Gen::GetShapeDim(_subShape))
			{
				ret = _meshDS->AddHypothesis(_subShape, anHyp);
//        if (ret &&(anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape)))
//        if (ret &&(anHyp->GetShapeType() == _subShape.ShapeType()))
				if (ret &&
					(anHyp->GetShapeType() & (1 << _subShape.ShapeType())))
				{
					SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
					if (algo == NULL)	// two algo on the same subShape...
					{
						INFOS("two algo on the same subshape not allowed");
						ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
						ret = false;
					}
					else
					{
						ret = algo->CheckHypothesis((*_father), _subShape);
						if (ret)
							SetAlgoState(HYP_OK);
						else
							SetAlgoState(MISSING_HYP);
					}
				}
			}
			break;
		case REMOVE_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
			if (ret)
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				ASSERT(algo);
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					SetAlgoState(HYP_OK);
				else
					SetAlgoState(MISSING_HYP);
				modifiedHyp = true;
			}
			break;
		case REMOVE_ALGO:		// perhaps a father algo applies ?
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
			ret = _meshDS->RemoveHypothesis(_subShape, anHyp);
//    if (ret &&(anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape)))
//    if (ret &&(anHyp->GetShapeType() == _subShape.ShapeType()))
			if (ret && (anHyp->GetShapeType() & (1 << _subShape.ShapeType())))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				if (algo == NULL)	// no more algo applying on subShape...
				{
					SetAlgoState(NO_ALGO);
				}
				else
				{
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		case ADD_FATHER_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				ASSERT(algo);
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					SetAlgoState(HYP_OK);
				else
					SetAlgoState(MISSING_HYP);
			}
			break;
		case ADD_FATHER_ALGO:	// detect if two algo of same dim on father
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
//    if (anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape))
//    if (anHyp->GetShapeType() == _subShape.ShapeType())
			if (anHyp->GetShapeType() & (1 << _subShape.ShapeType()))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				if (algo == NULL)	// two applying algo on father
				{
					MESSAGE("two applying algo on fatherShape...");
					SetAlgoState(NO_ALGO);
				}
				else
				{
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		case REMOVE_FATHER_HYP:
			ASSERT(anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO);
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				ASSERT(algo);
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					SetAlgoState(HYP_OK);
				else
					SetAlgoState(MISSING_HYP);
			}
			break;
		case REMOVE_FATHER_ALGO:
			ASSERT(anHyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO);
//    if (anHyp->GetDim() == SMESH_Gen::GetShapeDim(_subShape))
//    if (anHyp->GetShapeType() == _subShape.ShapeType())
			if (anHyp->GetShapeType() & (1 << _subShape.ShapeType()))
			{
				SMESH_Algo *algo = gen->GetAlgo((*_father), _subShape);
				if (algo == NULL)	// no more applying algo on father
				{
					SetAlgoState(NO_ALGO);
				}
				else
				{
					ret = algo->CheckHypothesis((*_father), _subShape);
					if (ret)
						SetAlgoState(HYP_OK);
					else
						SetAlgoState(MISSING_HYP);
				}
			}
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------

	default:
		ASSERT(0);
		break;
	}
	//SCRUTE(_algoState);
	if ((_algoState != _oldAlgoState) || modifiedHyp)
		int retc = ComputeStateEngine(MODIF_ALGO_STATE);
	return ret;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::SetAlgoState(int state)
{
	if (state != _oldAlgoState)
//     int retc = ComputeStateEngine(MODIF_ALGO_STATE);
		_algoState = state;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::SubMeshesAlgoStateEngine(int event,
	SMESH_Hypothesis * anHyp) throw(SALOME_Exception)
{
	//MESSAGE("SMESH_subMesh::SubMeshesAlgoStateEngine");
	int dim = SMESH_Gen::GetShapeDim(_subShape);
	if (dim > 1)
	{
		const map < int, SMESH_subMesh * >&subMeshes = DependsOn();

		map < int, SMESH_subMesh * >::const_iterator itsub;
		for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
		{
			SMESH_subMesh *sm = (*itsub).second;
			sm->AlgoStateEngine(event, anHyp);
		}
	}
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::DumpAlgoState(bool isMain)
{
	int dim = SMESH_Gen::GetShapeDim(_subShape);
//   if (dim < 1) return;
	if (isMain)
	{
		const map < int, SMESH_subMesh * >&subMeshes = DependsOn();

		map < int, SMESH_subMesh * >::const_iterator itsub;
		for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
		{
			SMESH_subMesh *sm = (*itsub).second;
			sm->DumpAlgoState(false);
		}
	}
	int type = _subShape.ShapeType();
	MESSAGE("dim = " << dim << " type of shape " << type);
	switch (_algoState)
	{
	case NO_ALGO:
		MESSAGE(" AlgoState = NO_ALGO");
		break;
	case MISSING_HYP:
		MESSAGE(" AlgoState = MISSING_HYP");
		break;
	case HYP_OK:
		MESSAGE(" AlgoState = HYP_OK");
		break;
	}
	switch (_computeState)
	{
	case NOT_READY:
		MESSAGE(" ComputeState = NOT_READY");
		break;
	case READY_TO_COMPUTE:
		MESSAGE(" ComputeState = READY_TO_COMPUTE");
		break;
	case COMPUTE_OK:
		MESSAGE(" ComputeState = COMPUTE_OK");
		break;
	case FAILED_TO_COMPUTE:
		MESSAGE(" ComputeState = FAILED_TO_COMPUTE");
		break;
	}
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

bool SMESH_subMesh::ComputeStateEngine(int event) throw(SALOME_Exception)
{
	//MESSAGE("SMESH_subMesh::ComputeStateEngine");
	//SCRUTE(_computeState);
	//SCRUTE(event);

	int dim = SMESH_Gen::GetShapeDim(_subShape);

	if (dim < 1)
	{
		if (_vertexSet)
			_computeState = COMPUTE_OK;
		else
			_computeState = READY_TO_COMPUTE;
		//SCRUTE(_computeState);
		return true;
	}
	SMESH_Gen *gen = _father->GetGen();
	SMESH_Algo *algo = 0;
	bool ret;

	switch (_computeState)
	{

		// ----------------------------------------------------------------------

	case NOT_READY:
		switch (event)
		{
		case MODIF_HYP:		// nothing to do
			break;
		case MODIF_ALGO_STATE:
			if (_algoState == HYP_OK)
				_computeState = READY_TO_COMPUTE;
			break;
		case COMPUTE:			// nothing to do
			break;
		case CLEAN:			// nothing to do
			break;
		case CLEANDEP:			// nothing to do
			RemoveSubMeshElementsAndNodes();	// recursive call...
			break;
		case SUBMESH_COMPUTED:	// nothing to do
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------

	case READY_TO_COMPUTE:
		switch (event)
		{
		case MODIF_HYP:		// nothing to do
			break;
		case MODIF_ALGO_STATE:
			_computeState = NOT_READY;
			algo = gen->GetAlgo((*_father), _subShape);
			if (algo)
			{
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					_computeState = READY_TO_COMPUTE;
			}
			break;
		case COMPUTE:
		{
			algo = gen->GetAlgo((*_father), _subShape);
			ASSERT(algo);
			ret = algo->CheckHypothesis((*_father), _subShape);
			if (!ret)
			{
				MESSAGE("***** verify compute state *****");
				_computeState = NOT_READY;
				break;
			}
			ret = SubMeshesComputed();
			if (!ret)
			{
				MESSAGE("Some SubMeshes not computed");
				_computeState = FAILED_TO_COMPUTE;
				break;
			}
			ret = algo->Compute((*_father), _subShape);
			if (!ret)
			{
				MESSAGE("problem in algo execution: failed to compute");
				_computeState = FAILED_TO_COMPUTE;
				break;
			}
			else
			{
				_computeState = COMPUTE_OK;
				UpdateDependantsState();	// send event SUBMESH_COMPUTED
			}
		}
			break;
		case CLEAN:
			_computeState = NOT_READY;
			algo = gen->GetAlgo((*_father), _subShape);
			if (algo)
			{
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					_computeState = READY_TO_COMPUTE;
			}
			break;
		case CLEANDEP:
			RemoveSubMeshElementsAndNodes();
			_computeState = NOT_READY;
			algo = gen->GetAlgo((*_father), _subShape);
			if (algo)
			{
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					_computeState = READY_TO_COMPUTE;
			}
			break;
		case SUBMESH_COMPUTED:	// nothing to do
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------

	case COMPUTE_OK:
		switch (event)
		{
		case MODIF_HYP:
			CleanDependants();	// recursive recall with event CLEANDEP
			break;
		case MODIF_ALGO_STATE:
			CleanDependants();	// recursive recall with event CLEANDEP
			break;
		case COMPUTE:			// nothing to do
			break;
		case CLEAN:
			CleanDependants();	// recursive recall with event CLEANDEP
			break;
		case CLEANDEP:
			RemoveSubMeshElementsAndNodes();
			_computeState = NOT_READY;
			algo = gen->GetAlgo((*_father), _subShape);
			if (algo)
			{
				ret = algo->CheckHypothesis((*_father), _subShape);
				if (ret)
					_computeState = READY_TO_COMPUTE;
			}
			break;
		case SUBMESH_COMPUTED:	// nothing to do
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------

	case FAILED_TO_COMPUTE:
		switch (event)
		{
		case MODIF_HYP:
			if (_algoState == HYP_OK)
				_computeState = READY_TO_COMPUTE;
			else
				_computeState = NOT_READY;
			break;
		case MODIF_ALGO_STATE:
			if (_algoState == HYP_OK)
				_computeState = READY_TO_COMPUTE;
			else
				_computeState = NOT_READY;
			break;
		case COMPUTE:			// nothing to do
			break;
		case CLEAN:
			break;
		case CLEANDEP:
			RemoveSubMeshElementsAndNodes();
			if (_algoState == HYP_OK)
				_computeState = READY_TO_COMPUTE;
			else
				_computeState = NOT_READY;
			break;
		case SUBMESH_COMPUTED:	// allow retry compute
			if (_algoState == HYP_OK)
				_computeState = READY_TO_COMPUTE;
			else
				_computeState = NOT_READY;
			break;
		default:
			ASSERT(0);
			break;
		}
		break;

		// ----------------------------------------------------------------------
	default:
		ASSERT(0);
		break;
	}

	//SCRUTE(_computeState);
	return ret;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::UpdateDependantsState()
{
	//MESSAGE("SMESH_subMesh::UpdateDependantsState");

	const map < int, SMESH_subMesh * >&dependants = Dependants();
	map < int, SMESH_subMesh * >::const_iterator its;
	for (its = dependants.begin(); its != dependants.end(); its++)
	{
		SMESH_subMesh *sm = (*its).second;
		//SCRUTE((*its).first);
		sm->ComputeStateEngine(SUBMESH_COMPUTED);
	}
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::CleanDependants()
{
	MESSAGE("SMESH_subMesh::CleanDependants");
	// **** parcourir les ancetres dans l'ordre de dépendance

	const map < int, SMESH_subMesh * >&dependants = Dependants();
	map < int, SMESH_subMesh * >::const_iterator its;
	for (its = dependants.begin(); its != dependants.end(); its++)
	{
		SMESH_subMesh *sm = (*its).second;
		SCRUTE((*its).first);
		sm->ComputeStateEngine(CLEANDEP);
	}
	ComputeStateEngine(CLEANDEP);
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::RemoveSubMeshElementsAndNodes()
{
	MESSAGE("SMESH_subMesh::RemoveSubMeshElementsAndNodes");
	SCRUTE(_subShape.ShapeType());
	SCRUTE(_Id);

	_subMeshDS = _meshDS->MeshElements(_subShape);
	if (_subMeshDS!=NULL)
	{
		SMDS_Iterator<const SMDS_MeshElement *> * ite=_subMeshDS->GetElements();
		while(ite->more())
		{
			const SMDS_MeshElement * elt = ite->next();
			_subMeshDS->RemoveElement(elt);
			_meshDS->RemoveElement(elt);
		}
		delete ite;
		
		SMDS_Iterator<const SMDS_MeshNode *> * itn=_subMeshDS->GetNodes();
		while(itn->more())
		{
			const SMDS_MeshNode * node = itn->next();
			_subMeshDS->RemoveNode(node);
			_meshDS->RemoveNode(node);
		}
		delete itn;
	}
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const map < int, SMESH_subMesh * >&SMESH_subMesh::Dependants()
{
	if (_dependantsFound)
		return _mapDependants;

	//MESSAGE("SMESH_subMesh::Dependants");

	int shapeType = _subShape.ShapeType();
	//SCRUTE(shapeType);
	TopTools_IndexedDataMapOfShapeListOfShape M;
	TopoDS_Shape mainShape = _meshDS->ShapeToMesh();

	switch (shapeType)
	{
	case TopAbs_VERTEX:
		break;
	case TopAbs_EDGE:
	case TopAbs_WIRE:
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_EDGE, TopAbs_WIRE, M);
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_EDGE, TopAbs_FACE, M);
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_EDGE, TopAbs_SHELL, M);
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_EDGE, TopAbs_SOLID, M);
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_EDGE, TopAbs_COMPSOLID,
			M);
		ExtractDependants(M, TopAbs_EDGE);
		break;
	case TopAbs_FACE:
	case TopAbs_SHELL:
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_FACE, TopAbs_SHELL, M);
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_FACE, TopAbs_SOLID, M);
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_FACE, TopAbs_COMPSOLID,
			M);
		ExtractDependants(M, TopAbs_FACE);
		break;
	case TopAbs_SOLID:
	case TopAbs_COMPSOLID:
		TopExp::MapShapesAndAncestors(mainShape, TopAbs_SOLID, TopAbs_COMPSOLID,
			M);
		ExtractDependants(M, TopAbs_SOLID);
		break;
	case TopAbs_COMPOUND:
		break;
	}

	_dependantsFound = true;
	return _mapDependants;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_subMesh::
ExtractDependants(const TopTools_IndexedDataMapOfShapeListOfShape & M,
	const TopAbs_ShapeEnum etype)
{
	//MESSAGE("SMESH_subMesh::ExtractDependants");

	TopoDS_Shape mainShape = _meshDS->ShapeToMesh();
	int lg = M.Extent();
	//SCRUTE(lg);

	int shapeType = _subShape.ShapeType();
	switch (shapeType)
	{
	case TopAbs_VERTEX:
		break;
	case TopAbs_EDGE:
	case TopAbs_FACE:
	case TopAbs_SOLID:
	{
		const TopTools_ListOfShape & ancestors = M.FindFromKey(_subShape);
		TopTools_ListIteratorOfListOfShape it(ancestors);
		for (; it.More(); it.Next())
		{
			TopoDS_Shape ancestor = it.Value();
			SMESH_subMesh *aSubMesh = _father->GetSubMeshContaining(ancestor);
			//      if (! aSubMesh) aSubMesh = _father->GetSubMesh(ancestor);
			if (aSubMesh)
			{
				int type = aSubMesh->_subShape.ShapeType();
				int cle = aSubMesh->GetId();
				cle += 10000000 * type;	// sort map by ordType then index
				if (_mapDependants.find(cle) == _mapDependants.end())
				{
					_mapDependants[cle] = aSubMesh;
					//SCRUTE(cle);
				}
			}
		}
	}
		break;
	case TopAbs_WIRE:
	case TopAbs_SHELL:
	case TopAbs_COMPSOLID:
		for (TopExp_Explorer expE(_subShape, etype); expE.More(); expE.Next())
		{
			TopoDS_Shape aShape = expE.Current();
			const TopTools_ListOfShape & ancestors = M.FindFromKey(aShape);
			TopTools_ListIteratorOfListOfShape it(ancestors);
			for (; it.More(); it.Next())
			{
				MESSAGE("---");
				TopoDS_Shape ancestor = it.Value();
				SMESH_subMesh *aSubMesh =
					_father->GetSubMeshContaining(ancestor);
				if (!aSubMesh)
					aSubMesh = _father->GetSubMesh(ancestor);
				int type = aSubMesh->_subShape.ShapeType();
				int cle = aSubMesh->GetId();
				cle += 10000000 * type;	// sort map by ordType then index
				if (_mapDependants.find(cle) == _mapDependants.end())
				{
					_mapDependants[cle] = aSubMesh;
					SCRUTE(cle);
				}
			}
		}
		break;
	case TopAbs_COMPOUND:
		break;
	}
}
