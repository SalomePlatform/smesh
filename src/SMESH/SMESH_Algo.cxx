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
//  File   : SMESH_Algo.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "SMESH_Algo.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

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

SMESH_Algo::SMESH_Algo(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_Hypothesis(hypId, studyId, gen)
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

const vector < string > &SMESH_Algo::GetCompatibleHypothesis()
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

ostream & operator <<(ostream & save, SMESH_Algo & hyp)
{
	return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, SMESH_Algo & hyp)
{
	return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESH_Algo::CheckHypothesis(SMESH_Mesh & aMesh,
	const TopoDS_Shape & aShape)
{
	MESSAGE("SMESH_Algo::CheckHypothesis");
	ASSERT(0);					// use method from derived classes
	return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESH_Algo::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
	MESSAGE("SMESH_Algo::Compute");
	ASSERT(0);					// use method from derived classes
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

const list <const SMESHDS_Hypothesis *> & SMESH_Algo::GetUsedHypothesis(
	SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
	_usedHypList.clear();
	_usedHypList = GetAppliedHypothesis(aMesh, aShape);	// copy
	int nbHyp = _usedHypList.size();
	if (nbHyp == 0)
	{
		TopoDS_Shape mainShape = aMesh.GetMeshDS()->ShapeToMesh();
		if (!mainShape.IsSame(aShape))
		{
			_usedHypList = GetAppliedHypothesis(aMesh, mainShape);	// copy
			nbHyp = _usedHypList.size();
		}
	}
	if (nbHyp > 1)
		_usedHypList.clear();	//only one compatible hypothesis allowed
	return _usedHypList;
}

//=============================================================================
/*!
 *  List the relevant hypothesis associated to the shape. Relevant hypothesis
 *  have a name (type) listed in the algorithm. Hypothesis associated to
 *  father shape -are not- taken into account (see GetUsedHypothesis)
 */
//=============================================================================

const list<const SMESHDS_Hypothesis *> & SMESH_Algo::GetAppliedHypothesis(
	SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
	const SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
	const list<const SMESHDS_Hypothesis*> & listHyp = meshDS->GetHypothesis(aShape);
	list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();

	int hypType;
	string hypName;

	_appliedHypList.clear();
	while (it!=listHyp.end())
	{
		const SMESHDS_Hypothesis *anHyp = *it;
		hypType = anHyp->GetType();
		//SCRUTE(hypType);
		if (hypType == SMESHDS_Hypothesis::PARAM_ALGO)
		{
			hypName = anHyp->GetName();
			vector < string >::iterator ith =
				find(_compatibleHypothesis.begin(), _compatibleHypothesis.end(),
				hypName);
			if (ith != _compatibleHypothesis.end())	// count only relevant 
			{
				_appliedHypList.push_back(anHyp);
				//SCRUTE(hypName);
			}
		}
		it++;
	}
	return _appliedHypList;
}

//=============================================================================
/*!
 *  Compute length of an edge
 */
//=============================================================================

double SMESH_Algo::EdgeLength(const TopoDS_Edge & E)
{
	double UMin = 0, UMax = 0;
	TopLoc_Location L;
	if (BRep_Tool::Degenerated(E))
		return 0;
	Handle(Geom_Curve) C = BRep_Tool::Curve(E, L, UMin, UMax);
	GeomAdaptor_Curve AdaptCurve(C);
	GCPnts_AbscissaPoint gabs;
	double length = gabs.Length(AdaptCurve, UMin, UMax);
	return length;
}
