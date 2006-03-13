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
//  File   : StdMeshers_MEFISTO_2D.cxx
//           Moved here from SMESH_MEFISTO_2D.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_MEFISTO_2D.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh.hxx"

#include "StdMeshers_MaxElementArea.hxx"
#include "StdMeshers_LengthFromEdges.hxx"

#include "Rn.h"
#include "aptrte.h"

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
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_MEFISTO_2D::StdMeshers_MEFISTO_2D(int hypId, int studyId,
                                             SMESH_Gen * gen):SMESH_2D_Algo(hypId, studyId, gen)
{
  MESSAGE("StdMeshers_MEFISTO_2D::StdMeshers_MEFISTO_2D");
  _name = "MEFISTO_2D";
  _shapeType = (1 << TopAbs_FACE);
  _compatibleHypothesis.push_back("MaxElementArea");
  _compatibleHypothesis.push_back("LengthFromEdges");

  _edgeLength = 0;
  _maxElementArea = 0;
  _hypMaxElementArea = NULL;
  _hypLengthFromEdges = NULL;
  myTool = 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_MEFISTO_2D::~StdMeshers_MEFISTO_2D()
{
  MESSAGE("StdMeshers_MEFISTO_2D::~StdMeshers_MEFISTO_2D");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_MEFISTO_2D::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
	//MESSAGE("StdMeshers_MEFISTO_2D::CheckHypothesis");

	_hypMaxElementArea = NULL;
	_hypLengthFromEdges = NULL;

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
	//int hypId = theHyp->GetID();
	//SCRUTE(hypName);

	bool isOk = false;

	if (hypName == "MaxElementArea")
	{
		_hypMaxElementArea = static_cast<const StdMeshers_MaxElementArea *>(theHyp);
		ASSERT(_hypMaxElementArea);
		_maxElementArea = _hypMaxElementArea->GetMaxArea();
		_edgeLength = 0;
		isOk = true;
                aStatus = SMESH_Hypothesis::HYP_OK;
	}

	else if (hypName == "LengthFromEdges")
	{
		_hypLengthFromEdges = static_cast<const StdMeshers_LengthFromEdges *>(theHyp);
		ASSERT(_hypLengthFromEdges);
		_edgeLength = 0;
		_maxElementArea = 0;
		isOk = true;
                aStatus = SMESH_Hypothesis::HYP_OK;
	}
        else
          aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

	if (isOk)
	{
		isOk = false;
		if (_maxElementArea > 0)
		{
// 			_edgeLength = 2 * sqrt(_maxElementArea);	// triangles : minorant
			_edgeLength = 2 * sqrt(_maxElementArea/sqrt(3.0));
			isOk = true;
		}
		else
			isOk = (_hypLengthFromEdges != NULL);	// **** check mode
                if (!isOk)
                  aStatus = SMESH_Hypothesis::HYP_BAD_PARAMETER;
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

bool StdMeshers_MEFISTO_2D::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
  MESSAGE("StdMeshers_MEFISTO_2D::Compute");

  if (_hypLengthFromEdges)
    _edgeLength = ComputeEdgeElementLength(aMesh, aShape);
  
  bool isOk = false;
  //const SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  //SMESH_subMesh *theSubMesh = aMesh.GetSubMesh(aShape);

  const TopoDS_Face & FF = TopoDS::Face(aShape);
  bool faceIsForward = (FF.Orientation() == TopAbs_FORWARD);
  TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));

  Z nblf;						//nombre de lignes fermees (enveloppe en tete)
  Z *nudslf = NULL;			//numero du dernier sommet de chaque ligne fermee
  R2 *uvslf = NULL;
  Z nbpti = 0;				//nombre points internes futurs sommets de la triangulation
  R2 *uvpti = NULL;
  
  Z nbst;
  R2 *uvst = NULL;
  Z nbt;
  Z *nust = NULL;
  Z ierr = 0;

  Z nutysu = 1;				// 1: il existe un fonction areteideale_()
  // Z  nutysu=0;              // 0: on utilise aretmx
  R aretmx = _edgeLength;		// longueur max aretes future triangulation
  
  nblf = NumberOfWires(F);
  
  nudslf = new Z[1 + nblf];
  nudslf[0] = 0;
  int iw = 1;
  int nbpnt = 0;

  myTool = new StdMeshers_Helper(aMesh);
  _quadraticMesh = myTool->IsQuadraticSubMesh(aShape);

  myOuterWire = BRepTools::OuterWire(F);
  nbpnt += NumberOfPoints(aMesh, myOuterWire);
  if ( nbpnt < 3 ) { // ex: a circle with 2 segments
    delete myTool; myTool = 0;
    return false;
  }
  nudslf[iw++] = nbpnt;

  for (TopExp_Explorer exp(F, TopAbs_WIRE); exp.More(); exp.Next()) {
    const TopoDS_Wire & W = TopoDS::Wire(exp.Current());
    if (!myOuterWire.IsSame(W)) {
      nbpnt += NumberOfPoints(aMesh, W);
      nudslf[iw++] = nbpnt;
    }
  }

  // avoid passing same uv points for a vertex common to 2 wires
  TopTools_IndexedDataMapOfShapeListOfShape VWMap;
  if ( iw - 1 > 1 ) // nbofWires > 1
    TopExp::MapShapesAndAncestors( F , TopAbs_VERTEX, TopAbs_WIRE, VWMap );

  uvslf = new R2[nudslf[nblf]];
  int m = 0;

  double scalex, scaley;
  ComputeScaleOnFace(aMesh, F, scalex, scaley);

  map<int, const SMDS_MeshNode*> mefistoToDS;	// correspondence mefisto index--> points IDNodes
  if ( !LoadPoints(aMesh, F, myOuterWire, uvslf, m,
                   mefistoToDS, scalex, scaley, VWMap) ) {
    delete myTool; myTool = 0;
    return false;
  }

  for (TopExp_Explorer exp(F, TopAbs_WIRE); exp.More(); exp.Next())
  {
    const TopoDS_Wire & W = TopoDS::Wire(exp.Current());
    if (!myOuterWire.IsSame(W))
    {
      if (! LoadPoints(aMesh, F, W, uvslf, m,
                       mefistoToDS, scalex, scaley, VWMap )) {
        delete myTool; myTool = 0;
        return false;
      }
    }
  }

  uvst = NULL;
  nust = NULL;
  aptrte(nutysu, aretmx,
         nblf, nudslf, uvslf, nbpti, uvpti, nbst, uvst, nbt, nust, ierr);

  if (ierr == 0)
  {
    MESSAGE("... End Triangulation Generated Triangle Number " << nbt);
    MESSAGE("                                    Node Number " << nbst);
    StoreResult(aMesh, nbst, uvst, nbt, nust, F,
                faceIsForward, mefistoToDS, scalex, scaley);
    isOk = true;
  }
  else
  {
    MESSAGE("Error in Triangulation");
    isOk = false;
  }
  if (nudslf != NULL)
    delete[]nudslf;
  if (uvslf != NULL)
    delete[]uvslf;
  if (uvst != NULL)
    delete[]uvst;
  if (nust != NULL)
    delete[]nust;
  delete myTool; myTool = 0;

  return isOk;
}

//=======================================================================
//function : fixOverlappedLinkUV
//purpose  : prevent failure due to overlapped adjacent links
//=======================================================================

static bool fixOverlappedLinkUV( R2& uv0, const R2& uv1, const R2& uv2 )
{
  gp_XY v1( uv0.x - uv1.x, uv0.y - uv1.y );
  gp_XY v2( uv2.x - uv1.x, uv2.y - uv1.y );

  double tol2 = DBL_MIN * DBL_MIN;
  double sqMod1 = v1.SquareModulus();
  if ( sqMod1 <= tol2 ) return false;
  double sqMod2 = v2.SquareModulus();
  if ( sqMod2 <= tol2 ) return false;

  double dot = v1*v2;

  // check sinus >= 1.e-3
  const double minSin = 1.e-3;
  if ( dot > 0 && 1 - dot * dot / ( sqMod1 * sqMod2 ) < minSin * minSin ) {
    MESSAGE(" ___ FIX UV ____" << uv0.x << " " << uv0.y);
    v1.SetCoord( -v1.Y(), v1.X() );
    double delta = sqrt( sqMod1 ) * minSin;
    if ( v1.X() < 0 )
      uv0.x -= delta;
    else
      uv0.x += delta;
    if ( v1.Y() < 0 )
      uv0.y -= delta;
    else
      uv0.y += delta;
// #ifdef _DEBUG_
//     MESSAGE(" -> " << uv0.x << " " << uv0.y << " ");
//     MESSAGE("v1( " << v1.X() << " " << v1.Y() << " ) " <<
//       "v2( " << v2.X() << " " << v2.Y() << " ) ");
//    MESSAGE("SIN: " << sqrt(1 - dot * dot / (sqMod1 * sqMod2)));
//     v1.SetCoord( uv0.x - uv1.x, uv0.y - uv1.y );
//     v2.SetCoord( uv2.x - uv1.x, uv2.y - uv1.y );
//     gp_XY v3( uv2.x - uv0.x, uv2.y - uv0.y );
//     sqMod1 = v1.SquareModulus();
//     sqMod2 = v2.SquareModulus();
//     dot = v1*v2;
//     double sin = sqrt(1 - dot * dot / (sqMod1 * sqMod2));
//     MESSAGE("NEW SIN: " << sin);
// #endif
    return true;
  }
  return false;
}

//=======================================================================
//function : fixCommonVertexUV
//purpose  : 
//=======================================================================

static bool fixCommonVertexUV (gp_Pnt2d &           theUV,
                               const TopoDS_Vertex& theV,
                               const TopoDS_Wire&   theW,
                               const TopoDS_Wire&   theOW,
                               const TopoDS_Face&   theF,
                               const TopTools_IndexedDataMapOfShapeListOfShape & theVWMap,
                               SMESH_Mesh &         theMesh,
                               bool CreateQuadratic)
{
  if( theW.IsSame( theOW ) ||
      !theVWMap.Contains( theV )) return false;

  // check if there is another wire sharing theV
  const TopTools_ListOfShape& WList = theVWMap.FindFromKey( theV );
  TopTools_ListIteratorOfListOfShape aWIt;
  for ( aWIt.Initialize( WList ); aWIt.More(); aWIt.Next() )
    if ( !theW.IsSame( aWIt.Value() ))
      break;
  if ( !aWIt.More() ) return false;

  TopTools_ListOfShape EList;
  list< double >       UList;

  // find edges of theW sharing theV
  // and find 2d normal to them at theV
  gp_Vec2d N(0.,0.);
  TopoDS_Iterator itE( theW );
  for (  ; itE.More(); itE.Next() )
  {
    const TopoDS_Edge& E = TopoDS::Edge( itE.Value() );
    TopoDS_Iterator itV( E );
    for ( ; itV.More(); itV.Next() )
    {
      const TopoDS_Vertex & V = TopoDS::Vertex( itV.Value() );
      if ( !V.IsSame( theV ))
        continue;
      EList.Append( E );
      Standard_Real u = BRep_Tool::Parameter( V, E );
      UList.push_back( u );
      double f, l;
      Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, theF, f, l);
      gp_Vec2d d1;
      gp_Pnt2d p;
      C2d->D1( u, p, d1 );
      gp_Vec2d n( d1.Y(), -d1.X() );
      if ( E.Orientation() == TopAbs_REVERSED )
        n.Reverse();
      N += n.Normalized();
    }
  }

  // define step size by which to move theUV

  gp_Pnt2d nextUV; // uv of next node on edge, most distant of the four
  double maxDist = -DBL_MAX;
  TopTools_ListIteratorOfListOfShape aEIt (EList);
  list< double >::iterator aUIt = UList.begin();
  for ( ; aEIt.More(); aEIt.Next(), aUIt++ )
  {
    const TopoDS_Edge& E = TopoDS::Edge( aEIt.Value() );
    double f, l;
    Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, theF, f, l);

    double umin = DBL_MAX, umax = -DBL_MAX;
    SMDS_NodeIteratorPtr nIt = theMesh.GetSubMesh(E)->GetSubMeshDS()->GetNodes();
    if ( !nIt->more() ) // no nodes on edge, only on vertices
    {
      umin = l;
      umax = f;
    }
    else {
      while ( nIt->more() ) {
        const SMDS_MeshNode* node = nIt->next();
        // check if node is medium
        if ( CreateQuadratic && StdMeshers_Helper::IsMedium( node, SMDSAbs_Edge ))
          continue;
        const SMDS_EdgePosition* epos =
          static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
        double u = epos->GetUParameter();
        if ( u < umin )
          umin = u;
        if ( u > umax )
          umax = u;
      }
    }
    bool isFirstCommon = ( *aUIt == f );
    gp_Pnt2d uv = C2d->Value( isFirstCommon ? umin : umax );
    double dist = theUV.SquareDistance( uv );
    if ( dist > maxDist ) {
      maxDist = dist;
      nextUV  = uv;
    }
  }
  R2 uv0, uv1, uv2;
  uv0.x = theUV.X();    uv0.y = theUV.Y(); 
  uv1.x = nextUV.X();   uv1.y = nextUV.Y(); 
  uv2.x = uv0.x;        uv2.y = uv0.y;
  if ( fixOverlappedLinkUV( uv0, uv1, uv2 ))
  {
    double step = theUV.Distance( gp_Pnt2d( uv0.x, uv0.y ));

    // move theUV along the normal by the step

    N *= step;

    MESSAGE("--fixCommonVertexUV move(" << theUV.X() << " " << theUV.Y()
            << ") by (" << N.X() << " " << N.Y() << ")" 
            << endl << "--- MAX DIST " << maxDist);

    theUV.SetXY( theUV.XY() + N.XY() );

    return true;
  }
  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_MEFISTO_2D::LoadPoints(SMESH_Mesh &        aMesh,
                                       const TopoDS_Face & FF,
                                       const TopoDS_Wire & WW,
                                       R2 *                uvslf,
                                       int &               m,
                                       map<int, const SMDS_MeshNode*>&mefistoToDS,
                                       double              scalex,
                                       double              scaley,
                                       const TopTools_IndexedDataMapOfShapeListOfShape& VWMap)
{
  //  MESSAGE("StdMeshers_MEFISTO_2D::LoadPoints");

  TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));

  list< int > mOnVertex;

  gp_XY scale( scalex, scaley );

  TopoDS_Wire W = TopoDS::Wire(WW.Oriented(TopAbs_FORWARD));
  BRepTools_WireExplorer wexp(W, F);
  for ( wexp.Init(W, F); wexp.More(); wexp.Next() )
  {
    const TopoDS_Edge & E = wexp.Current();
    bool isForward = (E.Orientation() == TopAbs_FORWARD);

    // --- IDNodes of first and last Vertex

    TopoDS_Vertex VFirst, VLast, V;
    TopExp::Vertices(E, VFirst, VLast); // corresponds to f and l
    V = isForward ? VFirst : VLast;

    ASSERT(!V.IsNull());
    SMDS_NodeIteratorPtr lid= aMesh.GetSubMesh(V)->GetSubMeshDS()->GetNodes();
    if ( !lid->more() ) {
      MESSAGE (" NO NODE BUILT ON VERTEX ");
      return false;
    }
    const SMDS_MeshNode* idFirst = lid->next();

    // --- edge internal IDNodes (relies on good order storage, not checked)

    map<double, const SMDS_MeshNode*> params;
    const SMDS_MeshNode * node;

    SMDS_NodeIteratorPtr nodeIt= aMesh.GetSubMesh(E)->GetSubMeshDS()->GetNodes();
    while ( nodeIt->more() )
    {
      node = nodeIt->next();
      if ( _quadraticMesh && StdMeshers_Helper::IsMedium( node, SMDSAbs_Edge ))
        continue;
      const SMDS_EdgePosition* epos =
        static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
      double param = epos->GetUParameter();
      if ( !isForward ) param = -param;
      if ( !params.insert( make_pair( param, node )).second )
      {
        MESSAGE( "BAD NODE ON EDGE POSITIONS" );
        return false;
      }
    }

    // --- load 2D values into MEFISTO structure,
    //     add IDNodes in mefistoToDS map

    double f, l, uFirst, u;
    Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);
    uFirst = isForward ? f : l;

    // vertex node
    gp_Pnt2d p = C2d->Value( uFirst ).XY().Multiplied( scale );
    if ( fixCommonVertexUV( p, V, W, myOuterWire, F, VWMap, aMesh, _quadraticMesh ))
      myNodesOnCommonV.push_back( idFirst );
    mOnVertex.push_back( m );
    uvslf[m].x = p.X();
    uvslf[m].y = p.Y();
    mefistoToDS[m + 1] = idFirst;
    //MESSAGE(" "<<m<<" "<<mefistoToDS[m+1]);
    //MESSAGE("__ f "<<uFirst<<" "<<uvslf[m].x <<" "<<uvslf[m].y);
    m++;

    // internal nodes
    map<double, const SMDS_MeshNode*>::iterator u_n = params.begin();
    for ( int i = 0; u_n != params.end(); ++u_n, ++i )
    {
      u = isForward ? u_n->first : - u_n->first;
      gp_Pnt2d p = C2d->Value( u ).XY().Multiplied( scale );
      uvslf[m].x = p.X();
      uvslf[m].y = p.Y();
      mefistoToDS[m + 1] = u_n->second;
      //MESSAGE(" "<<m<<" "<<mefistoToDS[m+1]);
      //MESSAGE("__ "<<i<<" "<<param<<" "<<uvslf[m].x <<" "<<uvslf[m].y);
      m++;
    }
  } // for  wexp

  // prevent failure on overlapped adjacent links,
  // check only links ending in vertex nodes
  int mFirst = mOnVertex.front(), mLast = m - 1;
  list< int >::iterator mIt = mOnVertex.begin();
  for ( ; mIt != mOnVertex.end(); ++mIt ) {
    int i = *mIt;
    int iB = i - 1, iA = i + 1; // indices Before and After
    if ( iB < mFirst ) iB = mLast;
    if ( iA > mLast )  iA = mFirst;
    fixOverlappedLinkUV (uvslf[ iB ], uvslf[ i ], uvslf[ iA ]);
  }

  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_MEFISTO_2D::ComputeScaleOnFace(SMESH_Mesh & aMesh,
	const TopoDS_Face & aFace, double &scalex, double &scaley)
{
  //MESSAGE("StdMeshers_MEFISTO_2D::ComputeScaleOnFace");
  TopoDS_Face F = TopoDS::Face(aFace.Oriented(TopAbs_FORWARD));
  TopoDS_Wire W = BRepTools::OuterWire(F);

  double xmin = 1.e300;         // min & max of face 2D parametric coord.
  double xmax = -1.e300;
  double ymin = 1.e300;
  double ymax = -1.e300;
  int nbp = 23;
  scalex = 1;
  scaley = 1;

  TopExp_Explorer wexp(W, TopAbs_EDGE);
  for ( ; wexp.More(); wexp.Next())
  {
    const TopoDS_Edge & E = TopoDS::Edge( wexp.Current() );
    double f, l;
    Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);
    if ( C2d.IsNull() ) continue;
    double du = (l - f) / double (nbp);
    for (int i = 0; i <= nbp; i++)
    {
      double param = f + double (i) * du;
      gp_Pnt2d p = C2d->Value(param);
      if (p.X() < xmin)
        xmin = p.X();
      if (p.X() > xmax)
        xmax = p.X();
      if (p.Y() < ymin)
        ymin = p.Y();
      if (p.Y() > ymax)
        ymax = p.Y();
      //    MESSAGE(" "<< f<<" "<<l<<" "<<param<<" "<<xmin<<" "<<xmax<<" "<<ymin<<" "<<ymax);
    }
  }
  //   SCRUTE(xmin);
  //   SCRUTE(xmax);
  //   SCRUTE(ymin);
  //   SCRUTE(ymax);
  double xmoy = (xmax + xmin) / 2.;
  double ymoy = (ymax + ymin) / 2.;
  double xsize = xmax - xmin;
  double ysize = ymax - ymin;

  Handle(Geom_Surface) S = BRep_Tool::Surface(F);       // 3D surface

  double length_x = 0;
  double length_y = 0;
  gp_Pnt PX0 = S->Value(xmin, ymoy);
  gp_Pnt PY0 = S->Value(xmoy, ymin);
  double dx = xsize / double (nbp);
  double dy = ysize / double (nbp);
  for (int i = 1; i <= nbp; i++)
  {
    double x = xmin + double (i) * dx;
    gp_Pnt PX = S->Value(x, ymoy);
    double y = ymin + double (i) * dy;
    gp_Pnt PY = S->Value(xmoy, y);
    length_x += PX.Distance(PX0);
    length_y += PY.Distance(PY0);
    PX0 = PX;
    PY0 = PY;
  }
  scalex = length_x / xsize;
  scaley = length_y / ysize;
//   SCRUTE(xsize);
//   SCRUTE(ysize);
  double xyratio = xsize*scalex/(ysize*scaley);
  const double maxratio = 1.e2;
  //SCRUTE(xyratio);
  if (xyratio > maxratio) {
    SCRUTE( scaley );
    scaley *= xyratio / maxratio;
    SCRUTE( scaley );
  }
  else if (xyratio < 1./maxratio) {
    SCRUTE( scalex );
    scalex *= 1 / xyratio / maxratio;
    SCRUTE( scalex );
  }
  ASSERT(scalex);
  ASSERT(scaley);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_MEFISTO_2D::StoreResult(SMESH_Mesh & aMesh,
                                        Z nbst, R2 * uvst, Z nbt, Z * nust,
                                        const TopoDS_Face & F, bool faceIsForward,
                                        map<int, const SMDS_MeshNode*>&mefistoToDS,
                                        double scalex, double scaley)
{
  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  int faceID = meshDS->ShapeToIndex( F );

  Z n, m;
  Handle(Geom_Surface) S = BRep_Tool::Surface(F);

  for (n = 0; n < nbst; n++)
  {
    if (mefistoToDS.find(n + 1) == mefistoToDS.end())
    {
      double u = uvst[n][0] / scalex;
      double v = uvst[n][1] / scaley;
      gp_Pnt P = S->Value(u, v);

      SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnFace(node, faceID, u, v);

      //MESSAGE(P.X()<<" "<<P.Y()<<" "<<P.Z());
      mefistoToDS[n + 1] = node;
      //MESSAGE("NEW: "<<n<<" "<<mefistoToDS[n+1]);
    }
  }

  m = 0;

  // triangle points must be in trigonometric order if face is Forward
  // else they must be put clockwise

  bool triangleIsWellOriented = faceIsForward;

  for (n = 1; n <= nbt; n++)
  {
    const SMDS_MeshNode * n1 = mefistoToDS[ nust[m++] ];
    const SMDS_MeshNode * n2 = mefistoToDS[ nust[m++] ];
    const SMDS_MeshNode * n3 = mefistoToDS[ nust[m++] ];

    SMDS_MeshElement * elt;
    if (triangleIsWellOriented)
      //elt = meshDS->AddFace(n1, n2, n3);
      elt = myTool->AddFace(n1, n2, n3);
    else
      //elt = meshDS->AddFace(n1, n3, n2);
      elt = myTool->AddFace(n1, n3, n2);

    meshDS->SetMeshElementOnShape(elt, faceID);
    m++;
  }

  // remove bad elements built on vertices shared by wires

  list<const SMDS_MeshNode*>::iterator itN = myNodesOnCommonV.begin();
  for ( ; itN != myNodesOnCommonV.end(); itN++ )
  {
    const SMDS_MeshNode* node = *itN;
    SMDS_ElemIteratorPtr invElemIt = node->GetInverseElementIterator();
    while ( invElemIt->more() )
    {
      const SMDS_MeshElement* elem = invElemIt->next();
      SMDS_ElemIteratorPtr itN = elem->nodesIterator();
      int nbSame = 0;
      while ( itN->more() )
        if ( itN->next() == node)
          nbSame++;
      if (nbSame > 1) {
        MESSAGE( "RM bad element " << elem->GetID());
        meshDS->RemoveElement( elem );
      }
    }
  }

}

//=============================================================================
/*!
 *  
 */
//=============================================================================

double StdMeshers_MEFISTO_2D::ComputeEdgeElementLength(SMESH_Mesh & aMesh,
	const TopoDS_Shape & aShape)
{
	//MESSAGE("StdMeshers_MEFISTO_2D::ComputeEdgeElementLength");
	// **** a mettre dans SMESH_2D_Algo ?

	//const TopoDS_Face & FF = TopoDS::Face(aShape);
	//bool faceIsForward = (FF.Orientation() == TopAbs_FORWARD);
	//TopoDS_Face F = TopoDS::Face(FF.Oriented(TopAbs_FORWARD));

	double meanElementLength = 100;
	double wireLength = 0;
	int wireElementsNumber = 0;
		for (TopExp_Explorer expe(aShape, TopAbs_EDGE); expe.More(); expe.Next())
		{
			const TopoDS_Edge & E = TopoDS::Edge(expe.Current());
			int nb = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
			double length = EdgeLength(E);
			wireLength += length;
			wireElementsNumber += nb;
		}
	if (wireElementsNumber)
		meanElementLength = wireLength / wireElementsNumber;
	//SCRUTE(meanElementLength);
	return meanElementLength;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_MEFISTO_2D::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_MEFISTO_2D::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_MEFISTO_2D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_MEFISTO_2D & hyp)
{
  return hyp.LoadFrom( load );
}
