using namespace std;
//=============================================================================
// File      : SMESH_Algo.cxx
// Created   : sam mai 18 09:20:53 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_Algo.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

#include "SMESHDS_ListOfPtrHypothesis.hxx"
#include "SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx"

#include <GeomAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_AbscissaPoint.hxx>

#include "utilities.h"

#include <algorithm>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Algo::SMESH_Algo(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
//   _compatibleHypothesis.push_back("hypothese_bidon");
  _type = ALGO;
  gen->_mapAlgo[hypId] = this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Algo::~SMESH_Algo()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

const vector<string> &  SMESH_Algo::GetCompatibleHypothesis()
{
  return _compatibleHypothesis;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_Algo::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_Algo::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream& operator << (ostream & save, SMESH_Algo & hyp)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream& operator >> (istream & load, SMESH_Algo & hyp)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESH_Algo::CheckHypothesis(SMESH_Mesh& aMesh,
				 const TopoDS_Shape& aShape)
{
  MESSAGE("SMESH_Algo::CheckHypothesis");
  ASSERT(0); // use method from derived classes
  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESH_Algo::Compute(SMESH_Mesh& aMesh,
			 const TopoDS_Shape& aShape)
{
  MESSAGE("SMESH_Algo::Compute");
  ASSERT(0); // use method from derived classes
  return false;
}

//=============================================================================
/*!
 *  List the hypothesis used by the algorithm associated to the shape.
 *  Hypothesis associated to father shape -are- taken into account (see
 *  GetAppliedHypothesis). Relevant hypothesis have a name (type) listed in
 *  the algorithm. This method could be surcharged by specific algorithms, in 
 *  case of several hypothesis simultaneously applicable.
 */
//=============================================================================

const list<SMESHDS_Hypothesis*>&
SMESH_Algo::GetUsedHypothesis(SMESH_Mesh& aMesh,
			      const TopoDS_Shape& aShape)
{
  _usedHypList.clear();
  _usedHypList = GetAppliedHypothesis(aMesh, aShape); // copy
  int nbHyp = _usedHypList.size();
  if (nbHyp == 0)
    {
      TopoDS_Shape mainShape = aMesh.GetMeshDS()->ShapeToMesh();
      if (!mainShape.IsSame(aShape))
	{
	  _usedHypList  = GetAppliedHypothesis(aMesh, mainShape); // copy
	  nbHyp = _usedHypList.size();
	}
    }
  if (nbHyp > 1) _usedHypList.clear(); //only one compatible hypothesis allowed
  return _usedHypList;
}

//=============================================================================
/*!
 *  List the relevant hypothesis associated to the shape. Relevant hypothesis
 *  have a name (type) listed in the algorithm. Hypothesis associated to
 *  father shape -are not- taken into account (see GetUsedHypothesis)
 */
//=============================================================================

const list<SMESHDS_Hypothesis*>&
SMESH_Algo::GetAppliedHypothesis(SMESH_Mesh& aMesh,
				 const TopoDS_Shape& aShape)
{
  const Handle(SMESHDS_Mesh)& meshDS = aMesh.GetMeshDS();
  const SMESHDS_ListOfPtrHypothesis& listHyp = meshDS->GetHypothesis(aShape);
  SMESHDS_ListIteratorOfListOfPtrHypothesis it(listHyp);

  int hypType;
  string hypName;

  _appliedHypList.clear();
  while (it.More())
    {
      SMESHDS_Hypothesis* anHyp = it.Value();
      hypType = anHyp->GetType();
      //SCRUTE(hypType);
      if (hypType == SMESHDS_Hypothesis::PARAM_ALGO)
	{
	  hypName = anHyp->GetName();
	  vector<string>::iterator ith = find(_compatibleHypothesis.begin(),
					      _compatibleHypothesis.end(),
					      hypName);
	  if (ith != _compatibleHypothesis.end()) // count only relevant 
	    {
	      _appliedHypList.push_back(anHyp);
	      //SCRUTE(hypName);
	    }
	}
      it.Next();
    }
  return _appliedHypList;
}


//=============================================================================
/*!
 *  Compute length of an edge
 */
//=============================================================================

double SMESH_Algo::EdgeLength(const TopoDS_Edge& E)
{
  double UMin = 0, UMax = 0;
  TopLoc_Location L;
  if (BRep_Tool::Degenerated(E)) return 0;
  Handle (Geom_Curve) C = BRep_Tool::Curve(E, L, UMin, UMax);
  GeomAdaptor_Curve AdaptCurve(C);
  GCPnts_AbscissaPoint gabs;
  double length = gabs.Length(AdaptCurve, UMin, UMax);
  return length;
}

