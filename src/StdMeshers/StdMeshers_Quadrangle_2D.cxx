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
//  File   : StdMeshers_Quadrangle_2D.cxx
//           Moved here from SMESH_Quadrangle_2D.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_Quadrangle_2D.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <Handle_Geom2d_Curve.hxx>
#include <Handle_Geom_Curve.hxx>
#include <gp_Pnt2d.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

#include "utilities.h"
#include "Utils_ExceptHandlers.hxx"


//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Quadrangle_2D::StdMeshers_Quadrangle_2D(int hypId,
	int studyId, SMESH_Gen * gen):SMESH_2D_Algo(hypId, studyId, gen)
{
	MESSAGE("StdMeshers_Quadrangle_2D::StdMeshers_Quadrangle_2D");
	_name = "Quadrangle_2D";
	//  _shapeType = TopAbs_FACE;
	_shapeType = (1 << TopAbs_FACE);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Quadrangle_2D::~StdMeshers_Quadrangle_2D()
{
	MESSAGE("StdMeshers_Quadrangle_2D::~StdMeshers_Quadrangle_2D");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Quadrangle_2D::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
	//MESSAGE("StdMeshers_Quadrangle_2D::CheckHypothesis");

	bool isOk = true;
        aStatus = SMESH_Hypothesis::HYP_OK;

	// nothing to check

	return isOk;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Quadrangle_2D::Compute(SMESH_Mesh & aMesh,
	const TopoDS_Shape & aShape)throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	//MESSAGE("StdMeshers_Quadrangle_2D::Compute");
	SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
	SMESH_subMesh *theSubMesh = aMesh.GetSubMesh(aShape);

	FaceQuadStruct *quad = CheckAnd2Dcompute(aMesh, aShape);
	if (!quad)
		return false;

	// --- compute 3D values on points, store points & quadrangles

	int nbdown = quad->nbPts[0];
	int nbright = quad->nbPts[1];
	int nbVertices = nbdown * nbright;
	int nbQuad = (nbdown - 1) * (nbright - 1);
	//SCRUTE(nbVertices);
	//SCRUTE(nbQuad);

	//   const TopoDS_Face& FF = TopoDS::Face(aShape);
	//   bool faceIsForward = (FF.Orientation() == TopAbs_FORWARD);
	//   TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));
	const TopoDS_Face & F = TopoDS::Face(aShape);
	bool faceIsForward = (F.Orientation() == TopAbs_FORWARD);
	Handle(Geom_Surface) S = BRep_Tool::Surface(F);

	for (int i = 1; i < nbdown - 1; i++)
		for (int j = 1; j < nbright - 1; j++)	// internal points
		{
			int ij = j * nbdown + i;
			double u = quad->uv_grid[ij].u;
			double v = quad->uv_grid[ij].v;
			gp_Pnt P = S->Value(u, v);
			SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
			meshDS->SetNodeOnFace(node, F);
			quad->uv_grid[ij].node = node;
//  Handle (SMDS_FacePosition) fpos
//    = new SMDS_FacePosition(theSubMesh->GetId(),i,j); // easier than u,v
//  node->SetPosition(fpos);
			SMDS_FacePosition* fpos =
                          dynamic_cast<SMDS_FacePosition*>(node->GetPosition().get());
			fpos->SetUParameter(i);
			fpos->SetVParameter(j);
		}

	//   bool isQuadForward = ( faceIsForward == quad->isEdgeForward[0]);
	for (int i = 0; i < nbdown - 1; i++)
		for (int j = 0; j < nbright - 1; j++)	// faces
		{
			const SMDS_MeshNode *a, *b, *c, *d;
			a = quad->uv_grid[j * nbdown + i].node;
			b = quad->uv_grid[j * nbdown + i + 1].node;
			c = quad->uv_grid[(j + 1) * nbdown + i + 1].node;
			d = quad->uv_grid[(j + 1) * nbdown + i].node;
			//  if (isQuadForward) faceId = meshDS->AddFace(a,b,c,d);
			//  else faceId = meshDS->AddFace(a,d,c,b);
			SMDS_MeshFace * face = meshDS->AddFace(a, b, c, d);
			meshDS->SetMeshElementOnShape(face, F);
		}

	QuadDelete(quad);
	bool isOk = true;
	return isOk;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

FaceQuadStruct *StdMeshers_Quadrangle_2D::CheckAnd2Dcompute(SMESH_Mesh & aMesh,
	const TopoDS_Shape & aShape)throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	//MESSAGE("StdMeshers_Quadrangle_2D::ComputeWithoutStore");

	SMESH_subMesh *theSubMesh = aMesh.GetSubMesh(aShape);

	//   const TopoDS_Face& FF = TopoDS::Face(aShape);
	//   bool faceIsForward = (FF.Orientation() == TopAbs_FORWARD);
	//   TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));
	const TopoDS_Face & F = TopoDS::Face(aShape);
	bool faceIsForward = (F.Orientation() == TopAbs_FORWARD);

	// verify 1 wire only, with 4 edges, same number of points on opposite edges

	if (NumberOfWires(F) != 1)
	{
		MESSAGE("only 1 wire by face (quadrangles)");
		return 0;
		//throw SALOME_Exception(LOCALIZED("only 1 wire by face (quadrangles)"));
	}
	//   const TopoDS_Wire WW = BRepTools::OuterWire(F);
	//   TopoDS_Wire W = TopoDS::Wire(WW.Oriented(TopAbs_FORWARD));
	const TopoDS_Wire & W = BRepTools::OuterWire(F);
	BRepTools_WireExplorer wexp(W, F);

	FaceQuadStruct *quad = new FaceQuadStruct;
	for (int i = 0; i < 4; i++)
		quad->uv_edges[i] = 0;
	quad->uv_grid = 0;

	int nbEdges = 0;
	for (wexp.Init(W, F); wexp.More(); wexp.Next())
	{
		//       const TopoDS_Edge& EE = wexp.Current();
		//       TopoDS_Edge E = TopoDS::Edge(EE.Oriented(TopAbs_FORWARD));
		const TopoDS_Edge & E = wexp.Current();
		int nb = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
		if (nbEdges < 4)
		{
			quad->edge[nbEdges] = E;
			quad->nbPts[nbEdges] = nb + 2;	// internal points + 2 extrema
		}
		nbEdges++;
	}

	if (nbEdges != 4)
	{
		MESSAGE("face must have 4 edges /quadrangles");
		QuadDelete(quad);
		return 0;
		//throw SALOME_Exception(LOCALIZED("face must have 4 edges /quadrangles"));
	}

	if (quad->nbPts[0] != quad->nbPts[2])
	{
		MESSAGE("different point number-opposed edge");
		QuadDelete(quad);
		return 0;
		//throw SALOME_Exception(LOCALIZED("different point number-opposed edge"));
	}

	if (quad->nbPts[1] != quad->nbPts[3])
	{
		MESSAGE("different point number-opposed edge");
		QuadDelete(quad);
		return 0;
		//throw SALOME_Exception(LOCALIZED("different point number-opposed edge"));
	}

	// set normalized grid on unit square in parametric domain

	SetNormalizedGrid(aMesh, F, quad);

	return quad;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_Quadrangle_2D::QuadDelete(FaceQuadStruct * quad)
{
	//MESSAGE("StdMeshers_Quadrangle_2D::QuadDelete");
	if (quad)
	{
		for (int i = 0; i < 4; i++)
		{
			if (quad->uv_edges[i])
				delete[]quad->uv_edges[i];
			quad->edge[i].Nullify();
		}
		if (quad->uv_grid)
			delete[]quad->uv_grid;
		delete quad;
	}
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_Quadrangle_2D::SetNormalizedGrid(SMESH_Mesh & aMesh,
	const TopoDS_Shape & aShape, FaceQuadStruct * quad) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	// Algorithme décrit dans "Génération automatique de maillages"
	// P.L. GEORGE, MASSON, § 6.4.1 p. 84-85
	// traitement dans le domaine paramétrique 2d u,v
	// transport - projection sur le carré unité

	const TopoDS_Face & F = TopoDS::Face(aShape);

	// 1 --- find orientation of the 4 edges, by test on extrema

	//      max             min                    0     x1     1
	//     |<----north-2-------^                a3 -------------> a2
	//     |                   |                   ^1          1^
	//    west-3            east-1 =right          |            |
	//     |                   |         ==>       |            |
	//  y0 |                   | y1                |            |
	//     |                   |                   |0          0|
	//     v----south-0-------->                a0 -------------> a1
	//      min             max                    0     x0     1
	//             =down
	//

	Handle(Geom2d_Curve) c2d[4];
	gp_Pnt2d pf[4];
	gp_Pnt2d pl[4];
	for (int i = 0; i < 4; i++)
	{
		c2d[i] = BRep_Tool::CurveOnSurface(quad->edge[i],
			F, quad->first[i], quad->last[i]);
		pf[i] = c2d[i]->Value(quad->first[i]);
		pl[i] = c2d[i]->Value(quad->last[i]);
		quad->isEdgeForward[i] = false;
	}

	double eps2d = 1.e-3;		// *** utiliser plutot TopExp::CommonVertex, puis
	// distances si piece fausse
	int i = 0;
	if ((pf[1].Distance(pl[0]) < eps2d) || (pl[1].Distance(pl[0]) < eps2d))
	{
		quad->isEdgeForward[0] = true;
	}
	else
	{
		double tmp = quad->first[0];
		quad->first[0] = quad->last[0];
		quad->last[0] = tmp;
		pf[0] = c2d[0]->Value(quad->first[0]);
		pl[0] = c2d[0]->Value(quad->last[0]);
	}
	for (int i = 1; i < 4; i++)
	{
		quad->isEdgeForward[i] = (pf[i].Distance(pl[i - 1]) < eps2d);
		if (!quad->isEdgeForward[i])
		{
			double tmp = quad->first[i];
			quad->first[i] = quad->last[i];
			quad->last[i] = tmp;
			pf[i] = c2d[i]->Value(quad->first[i]);
			pl[i] = c2d[i]->Value(quad->last[i]);
			//SCRUTE(pf[i].Distance(pl[i-1]));
			ASSERT(pf[i].Distance(pl[i - 1]) < eps2d);
		}
	}
	//SCRUTE(pf[0].Distance(pl[3]));
	ASSERT(pf[0].Distance(pl[3]) < eps2d);

//   for (int i=0; i<4; i++)
//     {
//       SCRUTE(quad->isEdgeForward[i]);
//       MESSAGE(" -first "<<i<<" "<<pf[i].X()<<" "<<pf[i].Y());
//       MESSAGE(" -last  "<<i<<" "<<pl[i].X()<<" "<<pl[i].Y());
//     }

	// 2 --- load 2d edge points (u,v) with orientation and value on unit square

	for (int i = 0; i < 2; i++)
	{
		quad->uv_edges[i] = LoadEdgePoints(aMesh, F,
			quad->edge[i], quad->first[i], quad->last[i]);

		//                         quad->isEdgeForward[i]);
	}
	for (int i = 2; i < 4; i++)
	{
		quad->uv_edges[i] = LoadEdgePoints(aMesh, F,
			quad->edge[i], quad->last[i], quad->first[i]);

		//                         !quad->isEdgeForward[i]);
	}

	// 3 --- 2D normalized values on unit square [0..1][0..1]

	int nbdown = quad->nbPts[0];
	int nbright = quad->nbPts[1];
	quad->uv_grid = new UVPtStruct[nbright * nbdown];

	UVPtStruct *uv_grid = quad->uv_grid;
	UVPtStruct *uv_e0 = quad->uv_edges[0];
	UVPtStruct *uv_e1 = quad->uv_edges[1];
	UVPtStruct *uv_e2 = quad->uv_edges[2];
	UVPtStruct *uv_e3 = quad->uv_edges[3];
	gp_Pnt2d a0 = pf[0];
	gp_Pnt2d a1 = pf[1];
	gp_Pnt2d a2 = pf[2];
	gp_Pnt2d a3 = pf[3];

	// nodes Id on edges

	int j = 0;
	for (int i = 0; i < nbdown; i++)
	{
		int ij = j * nbdown + i;
		uv_grid[ij].node = uv_e0[i].node;
	}
	i = nbdown - 1;
	for (int j = 0; j < nbright; j++)
	{
		int ij = j * nbdown + i;
		uv_grid[ij].node = uv_e1[j].node;
	}
	j = nbright - 1;
	for (int i = 0; i < nbdown; i++)
	{
		int ij = j * nbdown + i;
		uv_grid[ij].node = uv_e2[i].node;
	}
	i = 0;
	for (int j = 0; j < nbright; j++)
	{
		int ij = j * nbdown + i;
		uv_grid[ij].node = uv_e3[j].node;
	}

	// normalized 2d values on grid

	for (int i = 0; i < nbdown; i++)
		for (int j = 0; j < nbright; j++)
		{
			int ij = j * nbdown + i;
			// --- droite i cste : x = x0 + y(x1-x0)
			double x0 = uv_e0[i].normParam;	// bas - sud
			double x1 = uv_e2[i].normParam;	// haut - nord
			// --- droite j cste : y = y0 + x(y1-y0)
			double y0 = uv_e3[j].normParam;	// gauche-ouest
			double y1 = uv_e1[j].normParam;	// droite - est
			// --- intersection : x=x0+(y0+x(y1-y0))(x1-x0)
			double x = (x0 + y0 * (x1 - x0)) / (1 - (y1 - y0) * (x1 - x0));
			double y = y0 + x * (y1 - y0);
			uv_grid[ij].x = x;
			uv_grid[ij].y = y;
			//MESSAGE("-xy-01 "<<x0<<" "<<x1<<" "<<y0<<" "<<y1);
			//MESSAGE("-xy-norm "<<i<<" "<<j<<" "<<x<<" "<<y);
		}

	// 4 --- projection on 2d domain (u,v)

	for (int i = 0; i < nbdown; i++)
		for (int j = 0; j < nbright; j++)
		{
			int ij = j * nbdown + i;
			double x = uv_grid[ij].x;
			double y = uv_grid[ij].y;
			double param_0 = uv_e0[0].param + x * (uv_e0[nbdown - 1].param - uv_e0[0].param);	// sud
			double param_2 = uv_e2[0].param + x * (uv_e2[nbdown - 1].param - uv_e2[0].param);	// nord
			double param_1 = uv_e1[0].param + y * (uv_e1[nbright - 1].param - uv_e1[0].param);	// est
			double param_3 = uv_e3[0].param + y * (uv_e3[nbright - 1].param - uv_e3[0].param);	// ouest

			//MESSAGE("params "<<param_0<<" "<<param_1<<" "<<param_2<<" "<<param_3);
			gp_Pnt2d p0 = c2d[0]->Value(param_0);
			gp_Pnt2d p1 = c2d[1]->Value(param_1);
			gp_Pnt2d p2 = c2d[2]->Value(param_2);
			gp_Pnt2d p3 = c2d[3]->Value(param_3);

			double u =
				(1 - y) * p0.X() + x * p1.X() + y * p2.X() + (1 - x) * p3.X();
			double v =
				(1 - y) * p0.Y() + x * p1.Y() + y * p2.Y() + (1 - x) * p3.Y();

			u -= (1 - x) * (1 - y) * a0.X() + x * (1 - y) * a1.X() +
				x * y * a2.X() + (1 - x) * y * a3.X();
			v -= (1 - x) * (1 - y) * a0.Y() + x * (1 - y) * a1.Y() +
				x * y * a2.Y() + (1 - x) * y * a3.Y();

			uv_grid[ij].u = u;
			uv_grid[ij].v = v;

			//MESSAGE("-uv- "<<i<<" "<<j<<" "<<uv_grid[ij].u<<" "<<uv_grid[ij].v);
		}
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

UVPtStruct *StdMeshers_Quadrangle_2D::LoadEdgePoints(SMESH_Mesh & aMesh,
	const TopoDS_Face & F, const TopoDS_Edge & E, double first, double last)
  //                        bool isForward)
{
	//MESSAGE("StdMeshers_Quadrangle_2D::LoadEdgePoints");

	SMDS_Mesh * meshDS = aMesh.GetMeshDS();

	// --- IDNodes of first and last Vertex

	TopoDS_Vertex VFirst, VLast;
	TopExp::Vertices(E, VFirst, VLast);	// corresponds to f and l

    ASSERT(!VFirst.IsNull());
    SMDS_NodeIteratorPtr lid= aMesh.GetSubMesh(VFirst)->GetSubMeshDS()->GetNodes();
    const SMDS_MeshNode * idFirst = lid->next();

    ASSERT(!VLast.IsNull());
    lid=aMesh.GetSubMesh(VLast)->GetSubMeshDS()->GetNodes();
    const SMDS_MeshNode * idLast = lid->next();

	// --- edge internal IDNodes (relies on good order storage, not checked)

	int nbPoints = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
	//SCRUTE(nbPoints);
	UVPtStruct *uvslf = new UVPtStruct[nbPoints + 2];

	double f, l;
	Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);

	map<double, const SMDS_MeshNode *> params;
	SMDS_NodeIteratorPtr ite= aMesh.GetSubMesh(E)->GetSubMeshDS()->GetNodes();

	while(ite->more())
	{
		const SMDS_MeshNode * node = ite->next();
		const SMDS_EdgePosition* epos =
                  static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
		double param = epos->GetUParameter();
		params[param] = node;
	}

	bool isForward = (((l - f) * (last - first)) > 0);
	double paramin = 0;
	double paramax = 0;
	if (isForward)
	{
		paramin = f;
		paramax = l;
		gp_Pnt2d p = C2d->Value(f);	// first point = Vertex Forward
		uvslf[0].x = p.X();
		uvslf[0].y = p.Y();
		uvslf[0].param = f;
		uvslf[0].node = idFirst;
		//MESSAGE("__ f "<<f<<" "<<uvslf[0].x <<" "<<uvslf[0].y);
		map < double, const SMDS_MeshNode* >::iterator itp = params.begin();
		for (int i = 1; i <= nbPoints; i++)	// nbPoints internal
		{
			double param = (*itp).first;
			gp_Pnt2d p = C2d->Value(param);
			uvslf[i].x = p.X();
			uvslf[i].y = p.Y();
			uvslf[i].param = param;
			uvslf[i].node = (*itp).second;
			//MESSAGE("__ "<<i<<" "<<param<<" "<<uvslf[i].x <<" "<<uvslf[i].y);
			itp++;
		}
		p = C2d->Value(l);		// last point = Vertex Reversed
		uvslf[nbPoints + 1].x = p.X();
		uvslf[nbPoints + 1].y = p.Y();
		uvslf[nbPoints + 1].param = l;
		uvslf[nbPoints + 1].node = idLast;
		//MESSAGE("__ l "<<l<<" "<<uvslf[nbPoints+1].x <<" "<<uvslf[nbPoints+1].y);
	}
	else
	{
		paramin = l;
		paramax = f;
		gp_Pnt2d p = C2d->Value(l);	// first point = Vertex Reversed
		uvslf[0].x = p.X();
		uvslf[0].y = p.Y();
		uvslf[0].param = l;
		uvslf[0].node = idLast;
		//MESSAGE("__ l "<<l<<" "<<uvslf[0].x <<" "<<uvslf[0].y);
		map < double, const SMDS_MeshNode* >::reverse_iterator itp = params.rbegin();
		for (int j = nbPoints; j >= 1; j--)	// nbPoints internal
		{
			double param = (*itp).first;
			int i = nbPoints + 1 - j;
			gp_Pnt2d p = C2d->Value(param);
			uvslf[i].x = p.X();
			uvslf[i].y = p.Y();
			uvslf[i].param = param;
			uvslf[i].node = (*itp).second;
			//MESSAGE("__ "<<i<<" "<<param<<" "<<uvslf[i].x <<" "<<uvslf[i].y);
			itp++;
		}
		p = C2d->Value(f);		// last point = Vertex Forward
		uvslf[nbPoints + 1].x = p.X();
		uvslf[nbPoints + 1].y = p.Y();
		uvslf[nbPoints + 1].param = f;
		uvslf[nbPoints + 1].node = idFirst;
		//MESSAGE("__ f "<<f<<" "<<uvslf[nbPoints+1].x <<" "<<uvslf[nbPoints+1].y);
	}

	ASSERT(paramin != paramax);
	for (int i = 0; i < nbPoints + 2; i++)
	{
		uvslf[i].normParam = (uvslf[i].param - paramin) / (paramax - paramin);
		//SCRUTE(uvslf[i].normParam);
	}

	return uvslf;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_Quadrangle_2D::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_Quadrangle_2D::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_Quadrangle_2D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_Quadrangle_2D & hyp)
{
  return hyp.LoadFrom( load );
}
