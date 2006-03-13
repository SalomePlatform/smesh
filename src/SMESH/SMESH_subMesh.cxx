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
#include "SMESH_HypoFilter.hxx"

#include "utilities.h"
#include "OpUtil.hxx"

#include <BRep_Builder.hxx>

#include <TopExp.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#ifdef _DEBUG_
#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#endif

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================

SMESH_subMesh::SMESH_subMesh(int Id, SMESH_Mesh * father, SMESHDS_Mesh * meshDS,
	const TopoDS_Shape & aSubShape)
{
	_subShape = aSubShape;
	_meshDS = meshDS;
	_subMeshDS = meshDS->MeshElements(_subShape);	// may be null ...
	_father = father;
	_Id = Id;
	_dependenceAnalysed = false;

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

int SMESH_subMesh::GetId() const
{
  //MESSAGE("SMESH_subMesh::GetId");
  return _Id;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESHDS_SubMesh * SMESH_subMesh::GetSubMeshDS()
{
  // submesh appears in DS only when a mesher set nodes and elements on it
  if (_subMeshDS==NULL)
  {
    _subMeshDS = _meshDS->MeshElements(_subShape);	// may be null ...
//     if (_subMeshDS==NULL)
//     {
//       MESSAGE("problem... subMesh still empty");
//     }
  }
  return _subMeshDS;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESHDS_SubMesh* SMESH_subMesh::CreateSubMeshDS()
{
  if ( !GetSubMeshDS() )
    _meshDS->NewSubMesh( _meshDS->ShapeToIndex( _subShape ) );

  return GetSubMeshDS();
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_subMesh *SMESH_subMesh::GetFirstToCompute()
{
  const map < int, SMESH_subMesh * >&subMeshes = DependsOn();
  SMESH_subMesh *firstToCompute = 0;

  map < int, SMESH_subMesh * >::const_iterator itsub;
  for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
  {
    SMESH_subMesh *sm = (*itsub).second;
    bool readyToCompute = (sm->GetComputeState() == READY_TO_COMPUTE);
    if (readyToCompute)
    {
      firstToCompute = sm;
      break;
    }
  }
  if (firstToCompute)
  {
    return firstToCompute;	// a subMesh of this
  }
  if (_computeState == READY_TO_COMPUTE)
  {
    return this;		// this
  }
  return 0;                     // nothing to compute
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool SMESH_subMesh::SubMeshesComputed()
{
  //MESSAGE("SMESH_subMesh::SubMeshesComputed");
  const map < int, SMESH_subMesh * >&subMeshes = DependsOn();

  bool subMeshesComputed = true;
  map < int, SMESH_subMesh * >::const_iterator itsub;
  for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
  {
    SMESH_subMesh *sm = (*itsub).second;
    SMESHDS_SubMesh * ds = sm->GetSubMeshDS();
    // PAL10974.
    // There are some tricks with compute states, e.g. Penta_3D leaves
    // one face with READY_TO_COMPUTE state in order to be able to
    // recompute 3D when a locale triangle hypo changes (see PAL7428).
    // So we check if mesh is really present
    //bool computeOk = (sm->GetComputeState() == COMPUTE_OK);
    bool computeOk = ( ds && ( ds->GetNodes()->more() || ds->GetElements()->more() ));
    if (!computeOk)
    {
      const TopoDS_Shape & ss = sm->GetSubShape();
      int type = ss.ShapeType();

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
      for (TopExp_Explorer exp(_subShape, TopAbs_SOLID); exp.More();
           exp.Next())
      {
        InsertDependence(exp.Current());
      }
      for (TopExp_Explorer exp(_subShape, TopAbs_SHELL, TopAbs_SOLID); exp.More();
           exp.Next())
      {
          InsertDependence(exp.Current());      //only shell not in solid
      }
      for (TopExp_Explorer exp(_subShape, TopAbs_FACE, TopAbs_SHELL); exp.More();
           exp.Next())
      {
        InsertDependence(exp.Current());
      }
      for (TopExp_Explorer exp(_subShape, TopAbs_EDGE, TopAbs_FACE); exp.More();
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
      break;
    }
  case TopAbs_SOLID:
    {
      //MESSAGE("solid");
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
  SMESH_subMesh *aSubMesh = _father->GetSubMesh(aSubShape);
  int type = aSubShape.ShapeType();
  int ordType = 9 - type;               // 2 = Vertex, 8 = CompSolid
  int cle = aSubMesh->GetId();
  cle += 10000000 * ordType;    // sort map by ordType then index
  if ( _mapDepend.find( cle ) == _mapDepend.end())
  {
    _mapDepend[cle] = aSubMesh;
    const map < int, SMESH_subMesh * > & subMap = aSubMesh->DependsOn();
    _mapDepend.insert( subMap.begin(), subMap.end() );
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================

const TopoDS_Shape & SMESH_subMesh::GetSubShape() const
{
	//MESSAGE("SMESH_subMesh::GetSubShape");
	return _subShape;
}


//=======================================================================
//function : CanAddHypothesis
//purpose  : return true if theHypothesis can be attached to me:
//           its dimention is checked
//=======================================================================

bool SMESH_subMesh::CanAddHypothesis(const SMESH_Hypothesis* theHypothesis) const
{
  int aHypDim   = theHypothesis->GetDim();
  int aShapeDim = SMESH_Gen::GetShapeDim(_subShape);
  if ( aHypDim <= aShapeDim )
    return true;
//   if ( aHypDim < aShapeDim )
//     return ( _father->IsMainShape( _subShape ));

  return false;
}

//=======================================================================
//function : IsApplicableHypotesis
//purpose  : 
//=======================================================================

bool SMESH_subMesh::IsApplicableHypotesis(const SMESH_Hypothesis* theHypothesis,
                                          const TopAbs_ShapeEnum  theShapeType)
{
  if ( theHypothesis->GetType() > SMESHDS_Hypothesis::PARAM_ALGO)
    // algorithm
    return ( theHypothesis->GetShapeType() & (1<< theShapeType));

  // hypothesis
  switch ( theShapeType ) {
  case TopAbs_EDGE: 
  case TopAbs_FACE: 
  case TopAbs_SHELL:
  case TopAbs_SOLID:
    return SMESH_Gen::GetShapeDim( theShapeType ) == theHypothesis->GetDim();
//   case TopAbs_WIRE:
//   case TopAbs_COMPSOLID:
//   case TopAbs_COMPOUND:
  default:;
  }
  return false;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_subMesh::AlgoStateEngine(int event, SMESH_Hypothesis * anHyp)
{
  //  MESSAGE("SMESH_subMesh::AlgoStateEngine");
  //SCRUTE(_algoState);
  //SCRUTE(event);

  // **** les retour des evenement shape sont significatifs
  // (add ou remove fait ou non)
  // le retour des evenement father n'indiquent pas que add ou remove fait

  SMESH_Hypothesis::Hypothesis_Status aux_ret, ret = SMESH_Hypothesis::HYP_OK;

  int dim = SMESH_Gen::GetShapeDim(_subShape);

  if (dim < 1)
  {
    _algoState = HYP_OK;
    if (event == ADD_HYP || event == ADD_ALGO)
      return SMESH_Hypothesis::HYP_BAD_DIM; // do not allow to assign any hyp
    else
      return SMESH_Hypothesis::HYP_OK;
  }

  SMESH_Gen* gen =_father->GetGen();
//  bool ret = false;
  int oldAlgoState = _algoState;
  bool modifiedHyp = false;  // if set to true, force event MODIF_ALGO_STATE
                             // in ComputeStateEngine

  // ----------------------
  // check mesh conformity
  // ----------------------
  if (event == ADD_ALGO)
  {
    if (IsApplicableHypotesis( anHyp ) &&
        !_father->IsNotConformAllowed() &&
        !IsConform( static_cast< SMESH_Algo* >( anHyp )))
      return SMESH_Hypothesis::HYP_NOTCONFORM;
  }

  // ----------------------------------
  // add a hypothesis to DS if possible
  // ----------------------------------
  if (event == ADD_HYP || event == ADD_ALGO)
  {
    if ( ! CanAddHypothesis( anHyp ))
      return SMESH_Hypothesis::HYP_BAD_DIM;

    if ( /*!anHyp->IsAuxiliary() &&*/ GetSimilarAttached( _subShape, anHyp ) )
      return SMESH_Hypothesis::HYP_ALREADY_EXIST;

    if ( !_meshDS->AddHypothesis(_subShape, anHyp))
      return SMESH_Hypothesis::HYP_ALREADY_EXIST;

    // Serve Propagation of 1D hypothesis
    if (event == ADD_HYP) {
      bool isPropagationOk = true;
      bool isPropagationHyp = ( strcmp( "Propagation", anHyp->GetName() ) == 0 );

      if ( isPropagationHyp ) {
        TopExp_Explorer exp (_subShape, TopAbs_EDGE);
        TopTools_MapOfShape aMap;
        for (; exp.More(); exp.Next()) {
          if (aMap.Add(exp.Current())) {
            if (!_father->BuildPropagationChain(exp.Current())) {
              isPropagationOk = false;
            }
          }
        }
      }
      else if (anHyp->GetDim() == 1) { // Only 1D hypothesis can be propagated
        TopExp_Explorer exp (_subShape, TopAbs_EDGE);
        TopTools_MapOfShape aMap;
        for (; exp.More(); exp.Next()) {
          if (aMap.Add(exp.Current())) {
            TopoDS_Shape aMainEdge;
            if (_father->IsPropagatedHypothesis(exp.Current(), aMainEdge)) {
              isPropagationOk = _father->RebuildPropagationChains();
            } else if (_father->IsPropagationHypothesis(exp.Current())) {
              isPropagationOk = _father->BuildPropagationChain(exp.Current());
            } else {
            }
          }
        }
      } else {
      }

      if ( isPropagationOk ) {
        if ( isPropagationHyp )
          return ret; // nothing more to do for "Propagation" hypothesis
      }
      else if ( ret < SMESH_Hypothesis::HYP_CONCURENT) {
        ret = SMESH_Hypothesis::HYP_CONCURENT;
      }
    } // Serve Propagation of 1D hypothesis
  }

  // --------------------------
  // remove a hypothesis from DS
  // --------------------------
  if (event == REMOVE_HYP || event == REMOVE_ALGO)
  {
    if (!_meshDS->RemoveHypothesis(_subShape, anHyp))
      return SMESH_Hypothesis::HYP_OK; // nothing changes

    // Serve Propagation of 1D hypothesis
    if (event == REMOVE_HYP)
    {
      bool isPropagationOk = true;
      SMESH_HypoFilter propagFilter( SMESH_HypoFilter::HasName( "Propagation" ));
      bool isPropagationHyp = propagFilter.IsOk( anHyp, _subShape );

      if ( isPropagationHyp )
      {
        TopExp_Explorer exp (_subShape, TopAbs_EDGE);
        TopTools_MapOfShape aMap;
        for (; exp.More(); exp.Next()) {
          if (aMap.Add(exp.Current()) &&
              !_father->GetHypothesis( exp.Current(), propagFilter, true )) {
            // no more Propagation on the current edge
            if (!_father->RemovePropagationChain(exp.Current())) {
              return SMESH_Hypothesis::HYP_UNKNOWN_FATAL;
            }
          }
        }
        // rebuild propagation chains, because removing one
        // chain can resolve concurention, existing before
        isPropagationOk = _father->RebuildPropagationChains();
      }
      else if (anHyp->GetDim() == 1) // Only 1D hypothesis can be propagated
      {
        isPropagationOk = _father->RebuildPropagationChains();
      }

      if ( isPropagationOk ) {
        if ( isPropagationHyp )
          return ret; // nothing more to do for "Propagation" hypothesis
      }
      else if ( ret < SMESH_Hypothesis::HYP_CONCURENT) {
        ret = SMESH_Hypothesis::HYP_CONCURENT;
      }
    } // Serve Propagation of 1D hypothesis
  }

  // ------------------
  // analyse algo state
  // ------------------
  if (!IsApplicableHypotesis( anHyp ))
    return ret; // not applicable hypotheses do not change algo state

  switch (_algoState)
  {

    // ----------------------------------------------------------------------

  case NO_ALGO:
    switch (event) {
    case ADD_HYP:
      break;
    case ADD_ALGO: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if (algo->CheckHypothesis((*_father),_subShape, aux_ret))
        SetAlgoState(HYP_OK);
      else
        SetAlgoState(MISSING_HYP);
      break;
    }
    case REMOVE_HYP:
      break;
    case REMOVE_ALGO:
      break;
    case ADD_FATHER_HYP:
      break;
    case ADD_FATHER_ALGO: {    // Algo just added in father
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo == anHyp ) {
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret))
          SetAlgoState(HYP_OK);
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    case REMOVE_FATHER_HYP:
      break;
    case REMOVE_FATHER_ALGO: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if (algo)
      {
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))
            SetAlgoState(HYP_OK);
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    default:
      ASSERT(0);
      break;
    }
    break;

    // ----------------------------------------------------------------------

  case MISSING_HYP:
    switch (event)
    {
    case ADD_HYP: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo->CheckHypothesis((*_father),_subShape, ret ))
        SetAlgoState(HYP_OK);
      if (SMESH_Hypothesis::IsStatusFatal( ret ))
        _meshDS->RemoveHypothesis(_subShape, anHyp);
      else if (!_father->IsUsedHypothesis( anHyp, this ))
      {
        _meshDS->RemoveHypothesis(_subShape, anHyp);
        ret = SMESH_Hypothesis::HYP_INCOMPATIBLE;
      }
      break;
    }
    case ADD_ALGO: {           //already existing algo : on father ?
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))// ignore hyp status
        SetAlgoState(HYP_OK);
      else
        SetAlgoState(MISSING_HYP);
      break;
    }
    case REMOVE_HYP:
      break;
    case REMOVE_ALGO: {        // perhaps a father algo applies ?
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if (algo == NULL)  // no more algo applying on subShape...
      {
        SetAlgoState(NO_ALGO);
      }
      else
      {
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))
          SetAlgoState(HYP_OK);
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    case ADD_FATHER_HYP: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))
        SetAlgoState(HYP_OK);
      else
        SetAlgoState(MISSING_HYP);
    }
      break;
    case ADD_FATHER_ALGO: { // new father algo
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT( algo );
      if ( algo == anHyp ) {
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))
          SetAlgoState(HYP_OK);
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    case REMOVE_FATHER_HYP:    // nothing to do
      break;
    case REMOVE_FATHER_ALGO: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if (algo == NULL)  // no more applying algo on father
      {
        SetAlgoState(NO_ALGO);
      }
      else
      {
        if ( algo->CheckHypothesis((*_father),_subShape , aux_ret ))
          SetAlgoState(HYP_OK);
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    default:
      ASSERT(0);
      break;
    }
    break;

    // ----------------------------------------------------------------------

  case HYP_OK:
    switch (event)
    {
    case ADD_HYP: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if (!algo->CheckHypothesis((*_father),_subShape, ret ))
      {
        if ( !SMESH_Hypothesis::IsStatusFatal( ret ))
          // ret should be fatal: anHyp was not added
          ret = SMESH_Hypothesis::HYP_INCOMPATIBLE;
      }
      else if (!_father->IsUsedHypothesis(  anHyp, this ))
        ret = SMESH_Hypothesis::HYP_INCOMPATIBLE;

      if (SMESH_Hypothesis::IsStatusFatal( ret ))
      {
        MESSAGE("do not add extra hypothesis");
        _meshDS->RemoveHypothesis(_subShape, anHyp);
      }
      else
      {
        modifiedHyp = true;
      }
      break;
    }
    case ADD_ALGO: {           //already existing algo : on father ?
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if ( algo->CheckHypothesis((*_father),_subShape, aux_ret )) {
        // check if algo changes
        SMESH_HypoFilter f;
        f.Init(   SMESH_HypoFilter::IsAlgo() );
        f.And(    SMESH_HypoFilter::IsApplicableTo( _subShape ));
        f.AndNot( SMESH_HypoFilter::Is( algo ));
        const SMESH_Hypothesis * prevAlgo = _father->GetHypothesis( _subShape, f, true );
        if (prevAlgo && 
            string(algo->GetName()) != string(prevAlgo->GetName()) )
          modifiedHyp = true;
      }
      else
        SetAlgoState(MISSING_HYP);
      break;
    }
    case REMOVE_HYP: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))
        SetAlgoState(HYP_OK);
      else
        SetAlgoState(MISSING_HYP);
      modifiedHyp = true;
      break;
    }
    case REMOVE_ALGO: {         // perhaps a father algo applies ?
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if (algo == NULL)   // no more algo applying on subShape...
      {
        SetAlgoState(NO_ALGO);
      }
      else
      {
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret )) {
          // check if algo remains
          if ( anHyp != algo && strcmp( anHyp->GetName(), algo->GetName()) )
            modifiedHyp = true;
        }
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    case ADD_FATHER_HYP: {  // new father hypothesis ?
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo->CheckHypothesis((*_father),_subShape, aux_ret ))
      {
        if (_father->IsUsedHypothesis( anHyp, this )) // new Hyp
          modifiedHyp = true;
      }
      else
        SetAlgoState(MISSING_HYP);
      break;
    }
    case ADD_FATHER_ALGO: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if ( algo == anHyp ) { // a new algo on father
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret )) {
          // check if algo changes
          SMESH_HypoFilter f;
          f.Init(   SMESH_HypoFilter::IsAlgo() );
          f.And(    SMESH_HypoFilter::IsApplicableTo( _subShape ));
          f.AndNot( SMESH_HypoFilter::Is( algo ));
          const SMESH_Hypothesis* prevAlgo = _father->GetHypothesis( _subShape, f, true );
          if (prevAlgo && 
              string(algo->GetName()) != string(prevAlgo->GetName()) )
            modifiedHyp = true;
        }
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
    case REMOVE_FATHER_HYP: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      ASSERT(algo);
      if ( algo->CheckHypothesis((*_father),_subShape, aux_ret )) {
        // is there the same local hyp or maybe a new father algo applied?
        if ( !GetSimilarAttached( _subShape, anHyp ) )
          modifiedHyp = true;
      }
      else
        SetAlgoState(MISSING_HYP);
      break;
    }
    case REMOVE_FATHER_ALGO: {
      SMESH_Algo* algo = gen->GetAlgo((*_father), _subShape);
      if (algo == NULL)  // no more applying algo on father
      {
        SetAlgoState(NO_ALGO);
      }
      else
      {
        if ( algo->CheckHypothesis((*_father),_subShape, aux_ret )) {
          // check if algo changes
          if ( string(algo->GetName()) != string( anHyp->GetName()) )
            modifiedHyp = true;
        }
        else
          SetAlgoState(MISSING_HYP);
      }
      break;
    }
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

  if ((_algoState != oldAlgoState) || modifiedHyp)
    ComputeStateEngine(MODIF_ALGO_STATE);

  return ret;
}


//=======================================================================
//function : IsConform
//purpose  : check if a conform mesh will be produced by the Algo
//=======================================================================

bool SMESH_subMesh::IsConform(const SMESH_Algo* theAlgo)
{
//  MESSAGE( "SMESH_subMesh::IsConform" );

  if ( !theAlgo ) return false;

  // check only algo that doesn't NeedDescretBoundary(): because mesh made
  // on a sub-shape will be ignored by theAlgo
  if ( theAlgo->NeedDescretBoundary() )
    return true;

  SMESH_Gen* gen =_father->GetGen();

  // only local algo is to be checked
  if ( gen->IsGlobalHypothesis( theAlgo, *_father ))
    return true;

  // check algo attached to adjacent shapes

  // loop on one level down sub-meshes
  TopoDS_Iterator itsub( _subShape );
  for (; itsub.More(); itsub.Next())
  {
    // loop on adjacent subShapes
    TopTools_ListIteratorOfListOfShape it( _father->GetAncestors( itsub.Value() ));
    for (; it.More(); it.Next())
    {
      const TopoDS_Shape& adjacent = it.Value();
      if ( _subShape.IsSame( adjacent )) continue;
      if ( adjacent.ShapeType() != _subShape.ShapeType())
        break;

      // check algo attached to smAdjacent
      SMESH_Algo * algo = gen->GetAlgo((*_father), adjacent);
      if (algo &&
          //algo != theAlgo &&
          !algo->NeedDescretBoundary() /*&&
          !gen->IsGlobalHypothesis( algo, *_father )*/)
        return false; // NOT CONFORM MESH WILL BE PRODUCED
    }
  }

  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_subMesh::SetAlgoState(int state)
{
  _algoState = state;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Hypothesis::Hypothesis_Status
  SMESH_subMesh::SubMeshesAlgoStateEngine(int event,
                                          SMESH_Hypothesis * anHyp)
{
  //MESSAGE("SMESH_subMesh::SubMeshesAlgoStateEngine");
  SMESH_Hypothesis::Hypothesis_Status ret = SMESH_Hypothesis::HYP_OK;
  //EAP: a wire (dim==1) should notify edges (dim==1)
  //EAP: int dim = SMESH_Gen::GetShapeDim(_subShape);
  if (_subShape.ShapeType() < TopAbs_EDGE ) // wire,face etc
  {
    const map < int, SMESH_subMesh * >&subMeshes = DependsOn();

    map < int, SMESH_subMesh * >::const_iterator itsub;
    for (itsub = subMeshes.begin(); itsub != subMeshes.end(); itsub++)
    {
      SMESH_subMesh *sm = (*itsub).second;
      SMESH_Hypothesis::Hypothesis_Status ret2 =
        sm->AlgoStateEngine(event, anHyp);
      if ( ret2 > ret )
        ret = ret2;
    }
  }
  return ret;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_subMesh::CleanDependsOn()
{
  //MESSAGE("SMESH_subMesh::CleanDependsOn");
  // **** parcourir les ancetres dans l'ordre de dépendance

  ComputeStateEngine(CLEAN);

  const map < int, SMESH_subMesh * >&dependson = DependsOn();
  map < int, SMESH_subMesh * >::const_iterator its;
  for (its = dependson.begin(); its != dependson.end(); its++)
  {
    SMESH_subMesh *sm = (*its).second;
    sm->ComputeStateEngine(CLEAN);
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

//================================================================================
/*!
 * \brief Remove nodes and elements bound to submesh
  * \param subMesh - submesh containing nodes and elements
 */
//================================================================================

static void cleanSubMesh( SMESH_subMesh * subMesh )
{
  if (subMesh) {
    if (SMESHDS_SubMesh * subMeshDS = subMesh->GetSubMeshDS()) {
      SMESHDS_Mesh * meshDS = subMesh->GetFather()->GetMeshDS();
      SMDS_ElemIteratorPtr ite = subMeshDS->GetElements();
      while (ite->more()) {
        const SMDS_MeshElement * elt = ite->next();
        //MESSAGE( " RM elt: "<<elt->GetID()<<" ( "<<elt->NbNodes()<<" )" );
        //meshDS->RemoveElement(elt);
        meshDS->RemoveFreeElement(elt, subMeshDS);
      }

      SMDS_NodeIteratorPtr itn = subMeshDS->GetNodes();
      while (itn->more()) {
        const SMDS_MeshNode * node = itn->next();
        //MESSAGE( " RM node: "<<node->GetID());
        //meshDS->RemoveNode(node);
        meshDS->RemoveFreeNode(node, subMeshDS);
      }
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool SMESH_subMesh::ComputeStateEngine(int event)
{
  //MESSAGE("SMESH_subMesh::ComputeStateEngine");
  //SCRUTE(_computeState);
  //SCRUTE(event);

  int dim = SMESH_Gen::GetShapeDim(_subShape);

  if (dim < 1)
  {
    if ( IsMeshComputed() )
      _computeState = COMPUTE_OK;
    else
      _computeState = READY_TO_COMPUTE;
    return true;
  }
  SMESH_Gen *gen = _father->GetGen();
  SMESH_Algo *algo = 0;
  bool ret = true;
  SMESH_Hypothesis::Hypothesis_Status hyp_status;

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
      {
        _computeState = READY_TO_COMPUTE;
      }
      break;
    case COMPUTE:		// nothing to do
      break;
    case CLEAN:
      CleanDependants();
      RemoveSubMeshElementsAndNodes();
      break;
    case SUBMESH_COMPUTED:	// nothing to do
      break;
    case SUBMESH_RESTORED:
      ComputeSubMeshStateEngine( CHECK_COMPUTE_STATE );
      break;
    case MESH_ENTITY_REMOVED:
      break;
    case CHECK_COMPUTE_STATE:
      if ( IsMeshComputed() )
        _computeState = COMPUTE_OK;
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
        ret = algo->CheckHypothesis((*_father), _subShape, hyp_status);
        if (ret)
          _computeState = READY_TO_COMPUTE;
      }
      break;
    case COMPUTE:
      {
        algo = gen->GetAlgo((*_father), _subShape);
        ASSERT(algo);
        ret = algo->CheckHypothesis((*_father), _subShape, hyp_status);
        if (!ret)
        {
          MESSAGE("***** verify compute state *****");
          _computeState = NOT_READY;
          break;
        }
        // check submeshes needed
        if (algo->NeedDescretBoundary())
          ret = SubMeshesComputed();
        if (!ret)
        {
          MESSAGE("Some SubMeshes not computed");
          _computeState = FAILED_TO_COMPUTE;
          break;
        }
        // compute
        CleanDependants();
        RemoveSubMeshElementsAndNodes();
        try {
          if (!algo->NeedDescretBoundary() && !algo->OnlyUnaryInput())
            ret = ApplyToCollection( algo, GetCollection( gen, algo ) );
          else
            ret = algo->Compute((*_father), _subShape);
        }
        catch (Standard_Failure) {
          MESSAGE( "Exception in algo->Compute() ");
          ret = false;
        }
        if (!ret)
        {
          MESSAGE("problem in algo execution: failed to compute");
          _computeState = FAILED_TO_COMPUTE;
          if (!algo->NeedDescretBoundary())
            UpdateSubMeshState( FAILED_TO_COMPUTE );

#ifdef _DEBUG_
          // Show vertices location of a failed shape
          TopTools_IndexedMapOfShape vMap;
          TopExp::MapShapes( _subShape, TopAbs_VERTEX, vMap );
          for ( int iv = 1; iv <= vMap.Extent(); ++iv ) {
            gp_Pnt P( BRep_Tool::Pnt( TopoDS::Vertex( vMap( iv ) )));
            cout << P.X() << " " << P.Y() << " " << P.Z() << " " << endl;
          }
#endif
          break;
        }
        else
        {
          _computeState = COMPUTE_OK;
          UpdateDependantsState( SUBMESH_COMPUTED ); // send event SUBMESH_COMPUTED
          if (!algo->NeedDescretBoundary())
            UpdateSubMeshState( COMPUTE_OK );
        }
      }
      break;
    case CLEAN:
      CleanDependants();
      RemoveSubMeshElementsAndNodes();
      _computeState = NOT_READY;
      algo = gen->GetAlgo((*_father), _subShape);
      if (algo)
      {
        ret = algo->CheckHypothesis((*_father), _subShape, hyp_status);
        if (ret)
          _computeState = READY_TO_COMPUTE;
      }
      break;
    case SUBMESH_COMPUTED:      // nothing to do
      break;
    case SUBMESH_RESTORED:
      // check if a mesh is already computed that may
      // happen after retrieval from a file
      ComputeStateEngine( CHECK_COMPUTE_STATE );
      ComputeSubMeshStateEngine( CHECK_COMPUTE_STATE );
      break;
    case MESH_ENTITY_REMOVED:
      break;
    case CHECK_COMPUTE_STATE:
      if ( IsMeshComputed() )
        _computeState = COMPUTE_OK;
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
    case MODIF_ALGO_STATE:
      ComputeStateEngine( CLEAN );
      algo = gen->GetAlgo((*_father), _subShape);
      if (algo && !algo->NeedDescretBoundary())
        CleanDependsOn(); // clean sub-meshes with event CLEAN
      break;
    case COMPUTE:               // nothing to do
      break;
    case CLEAN:
      CleanDependants();  // clean sub-meshes, dependant on this one, with event CLEAN
      RemoveSubMeshElementsAndNodes();
      _computeState = NOT_READY;
      algo = gen->GetAlgo((*_father), _subShape);
      if (algo)
      {
        ret = algo->CheckHypothesis((*_father), _subShape, hyp_status);
        if (ret)
          _computeState = READY_TO_COMPUTE;
      }
      break;
    case SUBMESH_COMPUTED:      // nothing to do
      break;
    case SUBMESH_RESTORED:
      ComputeStateEngine( CHECK_COMPUTE_STATE );
      ComputeSubMeshStateEngine( CHECK_COMPUTE_STATE );
      break;
    case MESH_ENTITY_REMOVED:
      UpdateDependantsState( CHECK_COMPUTE_STATE );
      ComputeStateEngine( CHECK_COMPUTE_STATE );
      ComputeSubMeshStateEngine( CHECK_COMPUTE_STATE );
      break;
    case CHECK_COMPUTE_STATE:
      if ( !IsMeshComputed() )
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
    case COMPUTE:      // nothing to do
      break;
    case CLEAN:
      CleanDependants(); // submeshes dependent on me should be cleaned as well
      RemoveSubMeshElementsAndNodes();
      if (_algoState == HYP_OK)
        _computeState = READY_TO_COMPUTE;
      else
        _computeState = NOT_READY;
      break;
    case SUBMESH_COMPUTED:      // allow retry compute
      if (_algoState == HYP_OK)
        _computeState = READY_TO_COMPUTE;
      else
        _computeState = NOT_READY;
      break;
    case SUBMESH_RESTORED:
      ComputeSubMeshStateEngine( CHECK_COMPUTE_STATE );
      break;
    case MESH_ENTITY_REMOVED:
      break;
    case CHECK_COMPUTE_STATE:
      if ( IsMeshComputed() )
        _computeState = COMPUTE_OK;
      else
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

//=======================================================================
//function : ApplyToCollection
//purpose  : Apply theAlgo to all subshapes in theCollection
//=======================================================================

bool SMESH_subMesh::ApplyToCollection (SMESH_Algo*         theAlgo,
                                       const TopoDS_Shape& theCollection)
{
  MESSAGE("SMESH_subMesh::ApplyToCollection");
  ASSERT ( !theAlgo->NeedDescretBoundary() );

  bool ret = false;


  ret = theAlgo->Compute( *_father, theCollection );

  // set _computeState of subshapes
  TopExp_Explorer anExplorer( theCollection, _subShape.ShapeType() );
  for ( ; anExplorer.More(); anExplorer.Next() )
  {
    const TopoDS_Shape& aSubShape = anExplorer.Current();
    SMESH_subMesh* subMesh = _father->GetSubMeshContaining( aSubShape );
    if ( subMesh )
    {
      if (ret)
      {
        subMesh->_computeState = COMPUTE_OK;
        subMesh->UpdateDependantsState( SUBMESH_COMPUTED );
        subMesh->UpdateSubMeshState( COMPUTE_OK );
      }
      else
      {
        subMesh->_computeState = FAILED_TO_COMPUTE;
      }
    }
  }
  return ret;
}


//=======================================================================
//function : UpdateSubMeshState
//purpose  :
//=======================================================================

void SMESH_subMesh::UpdateSubMeshState(const compute_state theState)
{
  const map<int, SMESH_subMesh*>& smMap = DependsOn();
  map<int, SMESH_subMesh*>::const_iterator itsub;
  for (itsub = smMap.begin(); itsub != smMap.end(); itsub++)
  {
    SMESH_subMesh* sm = (*itsub).second;
    sm->_computeState = theState;
  }
}

//=======================================================================
//function : ComputeSubMeshStateEngine
//purpose  :
//=======================================================================

void SMESH_subMesh::ComputeSubMeshStateEngine(int event)
{
  const map<int, SMESH_subMesh*>& smMap = DependsOn();
  map<int, SMESH_subMesh*>::const_iterator itsub;
  for (itsub = smMap.begin(); itsub != smMap.end(); itsub++)
  {
    SMESH_subMesh* sm = (*itsub).second;
    sm->ComputeStateEngine(event);
  }
}

//=======================================================================
//function : UpdateDependantsState
//purpose  :
//=======================================================================

void SMESH_subMesh::UpdateDependantsState(const compute_event theEvent)
{
  //MESSAGE("SMESH_subMesh::UpdateDependantsState");
  TopTools_ListIteratorOfListOfShape it( _father->GetAncestors( _subShape ));
  for (; it.More(); it.Next())
  {
    const TopoDS_Shape& ancestor = it.Value();
    SMESH_subMesh *aSubMesh =
      _father->GetSubMeshContaining(ancestor);
    if (aSubMesh)
      aSubMesh->ComputeStateEngine( theEvent );
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_subMesh::CleanDependants()
{
  int dimToClean = SMESH_Gen::GetShapeDim( _subShape ) + 1;

  TopTools_ListIteratorOfListOfShape it( _father->GetAncestors( _subShape ));
  for (; it.More(); it.Next())
  {
    const TopoDS_Shape& ancestor = it.Value();
    if ( SMESH_Gen::GetShapeDim( ancestor ) == dimToClean ) {
      // PAL8021. do not go upper than SOLID, else ComputeStateEngine(CLEAN)
      // will erase mesh on other shapes in a compound
      if ( ancestor.ShapeType() >= TopAbs_SOLID ) {
        SMESH_subMesh *aSubMesh = _father->GetSubMeshContaining(ancestor);
        if (aSubMesh)
          aSubMesh->ComputeStateEngine(CLEAN);
      }
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_subMesh::RemoveSubMeshElementsAndNodes()
{
  //SCRUTE(_subShape.ShapeType());

  cleanSubMesh( this );

  // algo may bind a submesh not to _subShape, eg 3D algo
  // sets nodes on SHELL while _subShape may be SOLID

  int dim = SMESH_Gen::GetShapeDim( _subShape );
  int type = _subShape.ShapeType() + 1;
  for ( ; type <= TopAbs_EDGE; type++) {
    if ( dim == SMESH_Gen::GetShapeDim( (TopAbs_ShapeEnum) type ))
    {
      TopExp_Explorer exp( _subShape, (TopAbs_ShapeEnum) type );
      for ( ; exp.More(); exp.Next() )
        cleanSubMesh( _father->GetSubMeshContaining( exp.Current() ));
    }
    else
      break;
  }
}

//=======================================================================
//function : IsMeshComputed
//purpose  : check if _subMeshDS contains mesh elements
//=======================================================================

bool SMESH_subMesh::IsMeshComputed() const
{
  // algo may bind a submesh not to _subShape, eg 3D algo
  // sets nodes on SHELL while _subShape may be SOLID

  int dim = SMESH_Gen::GetShapeDim( _subShape );
  int type = _subShape.ShapeType();
  for ( ; type <= TopAbs_VERTEX; type++) {
    if ( dim == SMESH_Gen::GetShapeDim( (TopAbs_ShapeEnum) type ))
    {
      TopExp_Explorer exp( _subShape, (TopAbs_ShapeEnum) type );
      for ( ; exp.More(); exp.Next() )
      {
        SMESHDS_SubMesh * subMeshDS = _meshDS->MeshElements( exp.Current() );
        if ( subMeshDS != NULL &&
            (subMeshDS->GetElements()->more() || subMeshDS->GetNodes()->more())) {
          return true;
        }
      }
    }
    else
      break;
  }

  return false;
}


//=======================================================================
//function : GetCollection
//purpose  : return a shape containing all sub-shapes of the MainShape that can be
//           meshed at once along with _subShape
//=======================================================================

TopoDS_Shape SMESH_subMesh::GetCollection(SMESH_Gen * theGen, SMESH_Algo* theAlgo)
{
  MESSAGE("SMESH_subMesh::GetCollection");
  ASSERT (!theAlgo->NeedDescretBoundary());

  TopoDS_Shape mainShape = _father->GetMeshDS()->ShapeToMesh();

  if ( mainShape.IsSame( _subShape ))
    return _subShape;

  const bool ignoreAuxiliaryHyps = false;
  list<const SMESHDS_Hypothesis*> aUsedHyp =
    theAlgo->GetUsedHypothesis( *_father, _subShape, ignoreAuxiliaryHyps ); // copy

  // put in a compound all shapes with the same hypothesis assigned
  // and a good ComputState

  TopoDS_Compound aCompound;
  BRep_Builder aBuilder;
  aBuilder.MakeCompound( aCompound );

  TopExp_Explorer anExplorer( mainShape, _subShape.ShapeType() );
  for ( ; anExplorer.More(); anExplorer.Next() )
  {
    const TopoDS_Shape& S = anExplorer.Current();
    SMESH_subMesh* subMesh = _father->GetSubMesh( S );
    SMESH_Algo* anAlgo = theGen->GetAlgo( *_father, S );

    if (subMesh->GetComputeState() == READY_TO_COMPUTE &&
        anAlgo == theAlgo &&
        anAlgo->GetUsedHypothesis( *_father, S, ignoreAuxiliaryHyps ) == aUsedHyp)
    {
      aBuilder.Add( aCompound, S );
    }
  }

  return aCompound;
}

//=======================================================================
//function : GetSimilarAttached
//purpose  : return a hypothesis attached to theShape.
//           If theHyp is provided, similar but not same hypotheses
//           is returned; else only applicable ones having theHypType
//           is returned
//=======================================================================

const SMESH_Hypothesis* SMESH_subMesh::GetSimilarAttached(const TopoDS_Shape&      theShape,
                                                          const SMESH_Hypothesis * theHyp,
                                                          const int                theHypType)
{
  SMESH_HypoFilter hypoKind;
  hypoKind.Init( hypoKind.HasType( theHyp ? theHyp->GetType() : theHypType ));
  if ( theHyp ) {
    hypoKind.And   ( hypoKind.HasDim( theHyp->GetDim() ));
    hypoKind.AndNot( hypoKind.Is( theHyp ));
    if ( theHyp->IsAuxiliary() )
      hypoKind.And( hypoKind.HasName( theHyp->GetName() ));
    else
      hypoKind.AndNot( hypoKind.IsAuxiliary());
  }
  else {
    hypoKind.And( hypoKind.IsApplicableTo( theShape ));
  }

  return _father->GetHypothesis( theShape, hypoKind, false );
}

//=======================================================================
//function : CheckConcurentHypothesis
//purpose  : check if there are several applicable hypothesis attached to
//           ansestors
//=======================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_subMesh::CheckConcurentHypothesis (const int theHypType)
{
  MESSAGE ("SMESH_subMesh::CheckConcurentHypothesis");

  // is there local hypothesis on me?
  if ( GetSimilarAttached( _subShape, 0, theHypType ) )
    return SMESH_Hypothesis::HYP_OK;


  TopoDS_Shape aPrevWithHyp;
  const SMESH_Hypothesis* aPrevHyp = 0;
  TopTools_ListIteratorOfListOfShape it( _father->GetAncestors( _subShape ));
  for (; it.More(); it.Next())
  {
    const TopoDS_Shape& ancestor = it.Value();
    const SMESH_Hypothesis* hyp = GetSimilarAttached( ancestor, 0, theHypType );
    if ( hyp )
    {
      if ( aPrevWithHyp.IsNull() || aPrevWithHyp.IsSame( ancestor ))
      {
        aPrevWithHyp = ancestor;
        aPrevHyp     = hyp;
      }
      else if ( aPrevWithHyp.ShapeType() == ancestor.ShapeType() && aPrevHyp != hyp )
        return SMESH_Hypothesis::HYP_CONCURENT;
      else
        return SMESH_Hypothesis::HYP_OK;
    }
  }
  return SMESH_Hypothesis::HYP_OK;
}
