using namespace std;
//=============================================================================
// File      : SMESH_MEFISTO_2D.cxx
// Created   : sam mai 18 08:10:55 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_MEFISTO_2D.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

#include "SMESH_MaxElementArea.hxx"
#include "SMESH_LengthFromEdges.hxx"

#include "Rn.h"
#include "aptrte.h"

#include "SMESHDS_ListOfPtrHypothesis.hxx"
#include "SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"

#include "utilities.h"

#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Pnt2d.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

#include <string>
#include <algorithm>

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MEFISTO_2D::SMESH_MEFISTO_2D(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_2D_Algo(hypId, studyId, gen)
{
  MESSAGE("SMESH_MEFISTO_2D::SMESH_MEFISTO_2D");
  _name = "MEFISTO_2D";
//   _shapeType = TopAbs_FACE;
  _shapeType = (1<<TopAbs_FACE);
  _compatibleHypothesis.push_back("MaxElementArea");
  _compatibleHypothesis.push_back("LengthFromEdges");

  _edgeLength = 0;
  _maxElementArea = 0;
  _hypMaxElementArea = NULL;
  _hypLengthFromEdges = NULL;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_MEFISTO_2D::~SMESH_MEFISTO_2D()
{
  MESSAGE("SMESH_MEFISTO_2D::~SMESH_MEFISTO_2D");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESH_MEFISTO_2D::CheckHypothesis(SMESH_Mesh& aMesh,
			       const TopoDS_Shape& aShape)
{
  //MESSAGE("SMESH_MEFISTO_2D::CheckHypothesis");

  _hypMaxElementArea = NULL;
  _hypLengthFromEdges = NULL;

  list<SMESHDS_Hypothesis*>::const_iterator itl;
  SMESHDS_Hypothesis* theHyp;

  const list<SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  int nbHyp = hyps.size();
  if (nbHyp != 1) return false;  // only one compatible hypothesis allowed

  itl = hyps.begin();
  theHyp = (*itl);

  string hypName = theHyp->GetName();
  int hypId = theHyp->GetID();
  //SCRUTE(hypName);

  bool isOk = false;

  if (hypName == "MaxElementArea")
    {
      _hypMaxElementArea = dynamic_cast<SMESH_MaxElementArea*> (theHyp);
      ASSERT(_hypMaxElementArea);
      _maxElementArea = _hypMaxElementArea->GetMaxArea();
      _edgeLength = 0;
      isOk =true;
    }

  if (hypName == "LengthFromEdges")
    {
      _hypLengthFromEdges = dynamic_cast<SMESH_LengthFromEdges*> (theHyp);
      ASSERT(_hypLengthFromEdges);
      _edgeLength = 0;
      _maxElementArea = 0;
      isOk =true;
    }


  if (isOk)
    {
      isOk = false;
      if (_maxElementArea > 0)
	{
	  _edgeLength = 2*sqrt(_maxElementArea);  // triangles : minorant
	  isOk = true;
	}
      else isOk = (_hypLengthFromEdges != NULL); // **** check mode
    }

  //SCRUTE(_edgeLength);
  //SCRUTE(_maxElementArea);
  return isOk;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

bool SMESH_MEFISTO_2D::Compute(SMESH_Mesh& aMesh,
		       const TopoDS_Shape& aShape)
{
  MESSAGE("SMESH_MEFISTO_2D::Compute");

  if (_hypLengthFromEdges)
    _edgeLength = ComputeEdgeElementLength(aMesh, aShape);

  bool isOk = false;
  const Handle(SMESHDS_Mesh)& meshDS = aMesh.GetMeshDS();
  SMESH_subMesh* theSubMesh = aMesh.GetSubMesh(aShape);

  const TopoDS_Face& FF = TopoDS::Face(aShape);
  bool faceIsForward = (FF.Orientation() == TopAbs_FORWARD);
  TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));

  Z   nblf;        //nombre de lignes fermees (enveloppe en tete)
  Z  *nudslf=NULL; //numero du dernier sommet de chaque ligne fermee
  R2 *uvslf=NULL;
  Z   nbpti=0;     //nombre points internes futurs sommets de la triangulation
  R2 *uvpti=NULL;
  
  Z   nbst;
  R2 *uvst=NULL;
  Z   nbt;
  Z  *nust=NULL;
  Z  ierr=0;
  
  Z  nutysu=1;                 // 1: il existe un fonction areteideale_()
  // Z  nutysu=0;              // 0: on utilise aretmx
  R  aretmx=_edgeLength;       // longueur max aretes future triangulation
  //SCRUTE(aretmx);

  nblf =  NumberOfWires(F);
  //SCRUTE(nblf);

  nudslf = new Z[1+nblf];
  nudslf[0] = 0;
  int iw = 1;
  int nbpnt = 0;

  const TopoDS_Wire OW1 = BRepTools::OuterWire(F);
  nbpnt += NumberOfPoints (aMesh, OW1);
  nudslf [iw++] = nbpnt;
  //SCRUTE(nbpnt);

  for (TopExp_Explorer exp (F, TopAbs_WIRE); exp.More(); exp.Next())
    {
      const TopoDS_Wire& W = TopoDS::Wire(exp.Current());
      if (!OW1.IsSame(W))
	{
	  nbpnt += NumberOfPoints (aMesh, W);
	  nudslf [iw++] = nbpnt;
	  //SCRUTE(nbpnt);
	}
    }

  uvslf = new R2[nudslf[nblf]];
  //SCRUTE(nudslf[nblf]);
  int m = 0;

  map<int,int> mefistoToDS;  // correspondence mefisto index--> points IDNodes
  TopoDS_Wire OW = BRepTools::OuterWire(F);
  LoadPoints (aMesh, F, OW, uvslf, m, mefistoToDS);
  //SCRUTE(m);
 
  for (TopExp_Explorer exp (F, TopAbs_WIRE); exp.More(); exp.Next())
    {
      const TopoDS_Wire& W = TopoDS::Wire(exp.Current());
      if (!OW.IsSame(W))
	{
	  LoadPoints (aMesh, F, W, uvslf, m, mefistoToDS);
	  //SCRUTE(m);
	}
    }
//   SCRUTE(nudslf[nblf]);
//   for (int i=0; i<=nblf; i++)
//     {
//       MESSAGE(" -+- " <<i<< " "<< nudslf[i]);
//     }
//   for (int i=0; i<nudslf[nblf]; i++)
//     {
//       MESSAGE(" -+- " <<i<< " "<< uvslf[i]);
//     }
//   SCRUTE(nutysu);
//   SCRUTE(aretmx);
//   SCRUTE(nblf);

  MESSAGE("MEFISTO triangulation ..."); 
  uvst = NULL;
  nust = NULL;
  aptrte( nutysu, aretmx,
	  nblf, nudslf, uvslf,
	  nbpti, uvpti,
	  nbst, uvst, nbt, nust,
	  ierr );

  if( ierr == 0 )
    {
      MESSAGE("... End Triangulation");
      //SCRUTE(nbst);
      //SCRUTE(nbt);
      StoreResult (aMesh, nbst, uvst, nbt, nust, F,
		   faceIsForward, mefistoToDS);
      isOk = true;
    }
  else
    {
      MESSAGE("Error in Triangulation");
      isOk = false;
    }
  if (nudslf != NULL) delete [] nudslf;
  if (uvslf  != NULL) delete [] uvslf;
  if (uvst   != NULL) delete [] uvst;
  if (nust   != NULL) delete [] nust;
  return isOk;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MEFISTO_2D::LoadPoints(SMESH_Mesh& aMesh,
				  const TopoDS_Face& FF, 
				  const TopoDS_Wire& WW,
				  R2* uvslf, 
				  int& m,
				  map<int,int>& mefistoToDS)
{
  MESSAGE("SMESH_MEFISTO_2D::LoadPoints");

  Handle (SMDS_Mesh) meshDS = aMesh.GetMeshDS();

  double scalex;
  double scaley;
  TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));
  ComputeScaleOnFace(aMesh, F, scalex, scaley);

  TopoDS_Wire W = TopoDS::Wire(WW.Oriented(TopAbs_FORWARD));
  BRepTools_WireExplorer wexp(W,F);    
  for (wexp.Init(W,F);wexp.More(); wexp.Next())
    {
      const TopoDS_Edge& E = wexp.Current();

      // --- IDNodes of first and last Vertex

      TopoDS_Vertex VFirst, VLast;
      TopExp::Vertices(E, VFirst, VLast); // corresponds to f and l

      ASSERT(!VFirst.IsNull());
      SMESH_subMesh* firstSubMesh = aMesh.GetSubMesh(VFirst);
      const TColStd_ListOfInteger& lidf
	= firstSubMesh->GetSubMeshDS()->GetIDNodes();
      int idFirst= lidf.First();
//       SCRUTE(idFirst);

      ASSERT(!VLast.IsNull());
      SMESH_subMesh* lastSubMesh = aMesh.GetSubMesh(VLast);
      const TColStd_ListOfInteger& lidl
	= lastSubMesh->GetSubMeshDS()->GetIDNodes();
      int idLast= lidl.First();
//       SCRUTE(idLast);

      // --- edge internal IDNodes (relies on good order storage, not checked)

      int nbPoints = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
      //SCRUTE(nbPoints);
      
      Standard_Real f,l;
      Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E,F,f,l);
    
      const TColStd_ListOfInteger& indElt
	= aMesh.GetSubMesh(E)->GetSubMeshDS()->GetIDNodes();
      TColStd_ListIteratorOfListOfInteger ite(indElt);
      //SCRUTE(nbPoints);
      //SCRUTE(indElt.Extent());
      ASSERT(nbPoints == indElt.Extent());
      bool isForward = (E.Orientation() == TopAbs_FORWARD);
      map<double,int> params;
      for (; ite.More(); ite.Next())
	{
	  int nodeId = ite.Value();
 	  Handle (SMDS_MeshElement) elt = meshDS->FindNode(nodeId);
  	  Handle (SMDS_MeshNode) node = meshDS->GetNode(1, elt);
	  Handle (SMDS_EdgePosition) epos
	    = Handle (SMDS_EdgePosition)::DownCast(node->GetPosition());
	  double param = epos->GetUParameter();
	  params[param] = nodeId;
// 	  MESSAGE(" " << param << " " << params[param]);
	}

      // --- load 2D values into MEFISTO structure,
      //     add IDNodes in mefistoToDS map

      if (E.Orientation() == TopAbs_FORWARD)
	{
	  gp_Pnt2d p = C2d->Value(f); // first point = Vertex Forward
	  uvslf [m].x = scalex * p.X();
	  uvslf [m].y = scaley * p.Y();
	  mefistoToDS[m+1] = idFirst;
	  //MESSAGE(" "<<m<<" "<<mefistoToDS[m+1]);
	  //MESSAGE("__ f "<<f<<" "<<uvslf[m].x <<" "<<uvslf[m].y);
	  m++;
	  map<double,int>::iterator itp = params.begin();
	  for (Standard_Integer i = 1; i<=nbPoints; i++) // nbPoints internal
	    {
	      double param = (*itp).first;
	      gp_Pnt2d p = C2d->Value(param);
	      uvslf [m].x = scalex * p.X();
	      uvslf [m].y = scaley * p.Y();
	      mefistoToDS[m+1] = (*itp).second;
// 	      MESSAGE(" "<<m<<" "<<mefistoToDS[m+1]);
// 	      MESSAGE("__ "<<i<<" "<<param<<" "<<uvslf[m].x <<" "<<uvslf[m].y);
	      m++;
	      itp++;
	    }
	}
      else 
	{
	  gp_Pnt2d p = C2d->Value(l); // last point = Vertex Reversed
	  uvslf [m].x = scalex * p.X();
	  uvslf [m].y = scaley * p.Y();
	  mefistoToDS[m+1] = idLast;
// 	  MESSAGE(" "<<m<<" "<<mefistoToDS[m+1]);
// 	  MESSAGE("__ l "<<l<<" "<<uvslf[m].x <<" "<<uvslf[m].y);
	  m++;
	  map<double,int>::reverse_iterator itp = params.rbegin();
 	  for (Standard_Integer i = nbPoints ; i >= 1; i--)
	    {
	      double param = (*itp).first;
	      gp_Pnt2d p = C2d->Value(param);
	      uvslf [m].x = scalex * p.X();
	      uvslf [m].y = scaley * p.Y();
	      mefistoToDS[m+1] = (*itp).second;
// 	      MESSAGE(" "<<m<<" "<<mefistoToDS[m+1]);
//  	      MESSAGE("__ "<<i<<" "<<param<<" "<<uvslf[m].x <<" "<<uvslf[m].y);
	      m++;
	      itp++;
	    }
	}
    }	
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

// **** a mettre dans SMESH_Algo ou SMESH_2D_Algo

void SMESH_MEFISTO_2D::ComputeScaleOnFace(SMESH_Mesh& aMesh,
					  const TopoDS_Face& aFace,
					  double& scalex,
					  double& scaley)
{
  //MESSAGE("SMESH_MEFISTO_2D::ComputeScaleOnFace");
  TopoDS_Face F = TopoDS::Face(aFace.Oriented(TopAbs_FORWARD));  
  TopoDS_Wire W = BRepTools::OuterWire(F);

  BRepTools_WireExplorer wexp(W,F);    

  double xmin =  1.e300; // min & max of face 2D parametric coord.
  double xmax = -1.e300;
  double ymin =  1.e300;
  double ymax = -1.e300;
  int nbp = 50;
  scalex = 1;
  scaley = 1;
  for (wexp.Init(W,F);wexp.More(); wexp.Next())
    {
      const TopoDS_Edge& E = wexp.Current();
      double f,l;
      Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E,F,f,l);
      for (int i = 0; i<= nbp; i++)
	{
	  double param = f + (double(i)/double(nbp))*(l-f);
	  gp_Pnt2d p = C2d->Value(param);
	  if (p.X() < xmin) xmin = p.X();
	  if (p.X() > xmax) xmax = p.X();
	  if (p.Y() < ymin) ymin = p.Y();
	  if (p.Y() > ymax) ymax = p.Y();
// 	  MESSAGE(" "<< f<<" "<<l<<" "<<param<<" "<<xmin<<" "<<xmax<<" "<<ymin<<" "<<ymax);
	}
    }
//   SCRUTE(xmin);
//   SCRUTE(xmax);
//   SCRUTE(ymin);
//   SCRUTE(ymax);
  double xmoy = (xmax + xmin)/2.; 
  double ymoy = (ymax + ymin)/2.;

  Handle(Geom_Surface) S = BRep_Tool::Surface(F); // 3D surface

  double length_x = 0;
  double length_y = 0;
  gp_Pnt PX0 = S->Value(xmin, ymoy);
  gp_Pnt PY0 = S->Value(xmoy, ymin);
  for (Standard_Integer i = 1; i<= nbp; i++)
    {
      double x = xmin + (double(i)/double(nbp))*(xmax-xmin);
      gp_Pnt PX = S->Value(x,ymoy);
      double y = ymin + (double(i)/double(nbp))*(ymax-ymin);
      gp_Pnt PY = S->Value(xmoy,y);
      length_x += PX.Distance(PX0);
      length_y += PY.Distance(PY0);
      PX0.SetCoord(PX.X(),PX.Y(),PX.Z());
      PY0.SetCoord(PY.X(),PY.Y(),PY.Z());
    }
//   SCRUTE(length_x);
//   SCRUTE(length_y);
  scalex = length_x/(xmax - xmin);
  scaley = length_y/(ymax - ymin);
//   SCRUTE(scalex);
//   SCRUTE(scaley);
  ASSERT(scalex);
  ASSERT(scaley);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_MEFISTO_2D::StoreResult (SMESH_Mesh& aMesh,
				    Z nbst, R2* uvst, Z nbt, Z* nust, 
				    const TopoDS_Face& F, bool faceIsForward,
				    map<int,int>& mefistoToDS)
{
  double scalex;
  double scaley;
  ComputeScaleOnFace(aMesh, F, scalex, scaley);

  Handle (SMESHDS_Mesh) meshDS = aMesh.GetMeshDS();

  Z n,m;
  Handle(Geom_Surface) S = BRep_Tool::Surface(F);
  
  for ( n=0; n<nbst; n++ )
    {
      double u = uvst[n][0]/scalex;
      double v = uvst[n][1]/scaley;
      gp_Pnt P = S->Value(u,v);
    
      if (mefistoToDS.find(n+1) == mefistoToDS.end())
	{
	  int nodeId = meshDS->AddNode(P.X(), P.Y(), P.Z());
 	  Handle (SMDS_MeshElement) elt = meshDS->FindNode(nodeId);
 	  Handle (SMDS_MeshNode) node = meshDS->GetNode(1, elt);
 	  meshDS->SetNodeOnFace(node, F);
	  
 	  //MESSAGE(nodeId<<" "<<P.X()<<" "<<P.Y()<<" "<<P.Z());
	  mefistoToDS[n+1] = nodeId;
	  //MESSAGE(" "<<n<<" "<<mefistoToDS[n+1]);
	  Handle (SMDS_FacePosition) fpos
	    = Handle (SMDS_FacePosition)::DownCast(node->GetPosition());
	  fpos->SetUParameter(u);
	  fpos->SetVParameter(v);
 	}
    }
  
  m=0;
  int mt=0;

  //SCRUTE(faceIsForward);
  for ( n=1; n<=nbt; n++ )
    {
      int inode1 = nust[m++];
      int inode2 = nust[m++];
      int inode3 = nust[m++];

      int nodeId1 = mefistoToDS[inode1];
      int nodeId2 = mefistoToDS[inode2];
      int nodeId3 = mefistoToDS[inode3];
      //MESSAGE("-- "<<inode1<<" "<<inode2<<" "<<inode3<<" ++ "<<nodeId1<<" "<<nodeId2<<" "<<nodeId3);

      // triangle points must be in trigonometric order if face is Forward
      // else they must be put clockwise

      bool triangleIsWellOriented = faceIsForward;
      int faceId;
      if (triangleIsWellOriented)
	{
	  faceId = meshDS->AddFace(nodeId1, nodeId2, nodeId3);
	}
      else
	{
	  faceId = meshDS->AddFace(nodeId1, nodeId3, nodeId2);
	}
      Handle (SMDS_MeshElement) elt = meshDS->FindElement(faceId);
      meshDS->SetMeshElementOnShape(elt, F);
      m++;
    }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double SMESH_MEFISTO_2D::ComputeEdgeElementLength(SMESH_Mesh& aMesh,
						  const TopoDS_Shape& aShape)
{
  MESSAGE("SMESH_MEFISTO_2D::ComputeEdgeElementLength");
  // **** a mettre dans SMESH_2D_Algo ?

  const TopoDS_Face& FF = TopoDS::Face(aShape);
  bool faceIsForward = (FF.Orientation() == TopAbs_FORWARD);
  TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));

  double meanElementLength = 100;
  double wireLength =0;
  int wireElementsNumber =0;
  for (TopExp_Explorer exp (F, TopAbs_WIRE); exp.More(); exp.Next())
    {
      const TopoDS_Wire& W = TopoDS::Wire(exp.Current());
      for (TopExp_Explorer expe(W,TopAbs_EDGE); expe.More(); expe.Next())
	{
	  const TopoDS_Edge& E = TopoDS::Edge(expe.Current());
	  int nb = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
	  double length = EdgeLength(E);
	  wireLength += length;
	  wireElementsNumber += nb;
	}
    }
  if (wireElementsNumber)
    meanElementLength = wireLength/wireElementsNumber;
  //SCRUTE(meanElementLength);
  return meanElementLength;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_MEFISTO_2D::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_MEFISTO_2D::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, SMESH_MEFISTO_2D & hyp)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, SMESH_MEFISTO_2D & hyp)
{
  return load;
}

