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
//  File   : StdMeshers_Regular_1D.cxx
//           Moved here from SMESH_Regular_1D.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;

#include "StdMeshers_Regular_1D.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

#include "StdMeshers_LocalLength.hxx"
#include "StdMeshers_NumberOfSegments.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_EdgePosition.hxx"

#include "utilities.h"

#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>

#include <string>
#include <algorithm>

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Regular_1D::StdMeshers_Regular_1D(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_1D_Algo(hypId, studyId, gen)
{
	MESSAGE("StdMeshers_Regular_1D::StdMeshers_Regular_1D");
	_name = "Regular_1D";
	//  _shapeType = TopAbs_EDGE;
	_shapeType = (1 << TopAbs_EDGE);
	_compatibleHypothesis.push_back("LocalLength");
	_compatibleHypothesis.push_back("NumberOfSegments");

	_localLength = 0;
	_numberOfSegments = 0;
	_hypLocalLength = NULL;
	_hypNumberOfSegments = NULL;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Regular_1D::~StdMeshers_Regular_1D()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Regular_1D::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
	//MESSAGE("StdMeshers_Regular_1D::CheckHypothesis");

	list <const SMESHDS_Hypothesis * >::const_iterator itl;
	const SMESHDS_Hypothesis *theHyp;

	const list <const SMESHDS_Hypothesis * >&hyps = GetUsedHypothesis(aMesh, aShape);
	int nbHyp = hyps.size();
        if (!nbHyp)
        {
          aStatus = SMESH_Hypothesis::HYP_MISSING;
          return false;  // can't work with no hypothesis
        }

	itl = hyps.begin();
	theHyp = (*itl); // use only the first hypothesis

	string hypName = theHyp->GetName();
	int hypId = theHyp->GetID();
	//SCRUTE(hypName);

	bool isOk = false;

	if (hypName == "LocalLength")
	{
		_hypLocalLength = dynamic_cast <const StdMeshers_LocalLength * >(theHyp);
		ASSERT(_hypLocalLength);
		_localLength = _hypLocalLength->GetLength();
		_numberOfSegments = 0;
		isOk = true;
                aStatus = SMESH_Hypothesis::HYP_OK;
	}

	else if (hypName == "NumberOfSegments")
	{
		_hypNumberOfSegments =
			dynamic_cast <const StdMeshers_NumberOfSegments * >(theHyp);
		ASSERT(_hypNumberOfSegments);
		_numberOfSegments = _hypNumberOfSegments->GetNumberOfSegments();
		_scaleFactor = _hypNumberOfSegments->GetScaleFactor();
		_localLength = 0;
		isOk = true;
                aStatus = SMESH_Hypothesis::HYP_OK;
	}
        else
          aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

	//SCRUTE(_localLength);
	//SCRUTE(_numberOfSegments);

	return isOk;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Regular_1D::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
	MESSAGE("StdMeshers_Regular_1D::Compute");

	SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
	SMESH_subMesh *theSubMesh = aMesh.GetSubMesh(aShape);

	const TopoDS_Edge & EE = TopoDS::Edge(aShape);
	TopoDS_Edge E = TopoDS::Edge(EE.Oriented(TopAbs_FORWARD));

	double f, l;
	Handle(Geom_Curve) Curve = BRep_Tool::Curve(E, f, l);

	TopoDS_Vertex VFirst, VLast;
	TopExp::Vertices(E, VFirst, VLast);	// Vfirst corresponds to f and Vlast to l

	double length = EdgeLength(E);
	//SCRUTE(length);

	double eltSize = 1;
//   if (_localLength > 0) eltSize = _localLength;
	if (_localLength > 0)
	{
		double nbseg = ceil(length / _localLength);	// integer sup
		if (nbseg <= 0)
			nbseg = 1;			// degenerated edge
		eltSize = length / nbseg;
	}
	else
	{
		ASSERT(_numberOfSegments > 0);
		eltSize = length / _numberOfSegments;
	}

	ASSERT(!VFirst.IsNull());
	SMDS_NodeIteratorPtr lid= aMesh.GetSubMesh(VFirst)->GetSubMeshDS()->GetNodes();
	const SMDS_MeshNode * idFirst = lid->next();

	ASSERT(!VLast.IsNull());
	lid=aMesh.GetSubMesh(VLast)->GetSubMeshDS()->GetNodes();
	const SMDS_MeshNode * idLast = lid->next();

	if (!Curve.IsNull())
	{
		GeomAdaptor_Curve C3d(Curve);
		GCPnts_UniformAbscissa Discret(C3d, eltSize, f, l);
		int NbPoints = Discret.NbPoints();
		//MESSAGE("nb points on edge : "<<NbPoints);

		// edge extrema (indexes : 1 & NbPoints) already in SMDS (TopoDS_Vertex)
		// only internal nodes receive an edge position with param on curve

		const SMDS_MeshNode * idPrev = idFirst;
		for (int i = 2; i < NbPoints; i++)
		{
			double param = Discret.Parameter(i);

			if (_numberOfSegments > 1)
			{
				double epsilon = 0.001;
				if (fabs(_scaleFactor - 1.0) > epsilon)
				{
					double alpha =
						pow(_scaleFactor, 1.0 / (_numberOfSegments - 1));
					double d =
						length * (1 - pow(alpha, i - 1)) / (1 - pow(alpha,
							_numberOfSegments));
					param = d;
				}
			}

			gp_Pnt P = Curve->Value(param);

			//Add the Node in the DataStructure
			//MESSAGE("point "<<nodeId<<" "<<P.X()<<" "<<P.Y()<<" "<<P.Z()<<" - "<<i<<" "<<param);
			SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
			meshDS->SetNodeOnEdge(node, E);

			// **** edgePosition associe au point = param. 
			SMDS_EdgePosition* epos =
                          dynamic_cast<SMDS_EdgePosition *>(node->GetPosition().get());
			epos->SetUParameter(param);

			SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node);
			meshDS->SetMeshElementOnShape(edge, E);
			idPrev = node;
		}
		SMDS_MeshEdge* edge = meshDS->AddEdge(idPrev, idLast);
		meshDS->SetMeshElementOnShape(edge, E);
	}
	else
	{
//       MESSAGE ("Edge Degeneree non traitee --- arret");
//       ASSERT(0);
		if (BRep_Tool::Degenerated(E))
		{
			// Edge is a degenerated Edge : We put n = 5 points on the edge.
			int NbPoints = 5;
			BRep_Tool::Range(E, f, l);
			double du = (l - f) / (NbPoints - 1);
			MESSAGE("************* Degenerated edge! *****************");

			TopoDS_Vertex V1, V2;
			TopExp::Vertices(E, V1, V2);
			gp_Pnt P = BRep_Tool::Pnt(V1);

			const SMDS_MeshNode * idPrev = idFirst;
			for (int i = 2; i < NbPoints; i++)
			{
				double param = f + (i - 1) * du;
				SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
				meshDS->SetNodeOnEdge(node, E);

//        Handle (SMDS_EdgePosition) epos
//      = new SMDS_EdgePosition(theSubMesh->GetId(),param);
//        node->SetPosition(epos);
				SMDS_EdgePosition* epos =
                                  dynamic_cast<SMDS_EdgePosition*>(node->GetPosition().get());
				epos->SetUParameter(param);

				SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node);
				meshDS->SetMeshElementOnShape(edge, E);
				idPrev = node;
			}
			SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, idLast);
			meshDS->SetMeshElementOnShape(edge, E);
		}
		else
			ASSERT(0);
	}
	return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_Regular_1D::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_Regular_1D::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_Regular_1D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_Regular_1D & hyp)
{
  return hyp.LoadFrom( load );
}
