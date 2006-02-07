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
#include "SMESH_subMesh.hxx"

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
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>

#include <Precision.hxx>
#include <gp_Pnt2d.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>
#include <TColStd_SequenceOfReal.hxx>
#include <TColgp_SequenceOfXY.hxx>

#include "utilities.h"
#include "Utils_ExceptHandlers.hxx"

#ifndef StdMeshers_Array2OfNode_HeaderFile
#define StdMeshers_Array2OfNode_HeaderFile
typedef const SMDS_MeshNode* SMDS_MeshNodePtr;
#include <NCollection_DefineArray2.hxx>
DEFINE_BASECOLLECTION (StdMeshers_BaseCollectionNodePtr, SMDS_MeshNodePtr)
DEFINE_ARRAY2(StdMeshers_Array2OfNode,
              StdMeshers_BaseCollectionNodePtr, SMDS_MeshNodePtr)
#endif


//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Quadrangle_2D::StdMeshers_Quadrangle_2D (int hypId, int studyId, SMESH_Gen* gen)
     : SMESH_2D_Algo(hypId, studyId, gen)
{
  MESSAGE("StdMeshers_Quadrangle_2D::StdMeshers_Quadrangle_2D");
  _name = "Quadrangle_2D";
  _shapeType = (1 << TopAbs_FACE);
  _compatibleHypothesis.push_back("QuadranglePreference");
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
                         (SMESH_Mesh&                          aMesh,
                          const TopoDS_Shape&                  aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  bool isOk = true;
  aStatus = SMESH_Hypothesis::HYP_OK;

  // there is only one compatible Hypothesis so far
  const list <const SMESHDS_Hypothesis * >&hyps = GetUsedHypothesis(aMesh, aShape);
  myQuadranglePreference = hyps.size() > 0;

  return isOk;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Quadrangle_2D::Compute (SMESH_Mesh& aMesh,
                                        const TopoDS_Shape& aShape) throw (SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  //MESSAGE("StdMeshers_Quadrangle_2D::Compute");
  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  aMesh.GetSubMesh(aShape);

  //FaceQuadStruct *quad = CheckAnd2Dcompute(aMesh, aShape);
  FaceQuadStruct* quad = CheckNbEdges(aMesh, aShape);

  if (!quad)
    return false;

  if(myQuadranglePreference) {
    int n1 = quad->nbPts[0];
    int n2 = quad->nbPts[1];
    int n3 = quad->nbPts[2];
    int n4 = quad->nbPts[3];
    int nfull = n1+n2+n3+n4;
    int ntmp = nfull/2;
    ntmp = ntmp*2;
    if( nfull==ntmp && ( (n1!=n3) || (n2!=n4) ) ) {
      // special path for using only quandrangle faces
      return ComputeQuadPref(aMesh, aShape, quad);
    }
  }

  // set normalized grid on unit square in parametric domain
  SetNormalizedGrid(aMesh, aShape, quad);
  if (!quad)
    return false;

  // --- compute 3D values on points, store points & quadrangles

  int nbdown  = quad->nbPts[0];
  int nbup    = quad->nbPts[2];

  int nbright = quad->nbPts[1];
  int nbleft  = quad->nbPts[3];

  int nbhoriz  = Min(nbdown, nbup);
  int nbvertic = Min(nbright, nbleft);

  const TopoDS_Face& F = TopoDS::Face(aShape);
  Handle(Geom_Surface) S = BRep_Tool::Surface(F);

  // internal mesh nodes
  int i, j, geomFaceID = meshDS->ShapeToIndex( F );
  for (i = 1; i < nbhoriz - 1; i++) {
    for (j = 1; j < nbvertic - 1; j++) {
      int ij = j * nbhoriz + i;
      double u = quad->uv_grid[ij].u;
      double v = quad->uv_grid[ij].v;
      gp_Pnt P = S->Value(u, v);
      SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnFace(node, geomFaceID, u, v);
      quad->uv_grid[ij].node = node;
    }
  }

  // mesh faces

  //             [2]
  //      --.--.--.--.--.--  nbvertic
  //     |                 | ^
  //     |                 | ^
  // [3] |                 | ^ j  [1]
  //     |                 | ^
  //     |                 | ^
  //      ---.----.----.---  0
  //     0 > > > > > > > > nbhoriz
  //              i
  //             [0]

  i = 0;
  int ilow = 0;
  int iup = nbhoriz - 1;
  if (quad->isEdgeOut[3]) { ilow++; } else { if (quad->isEdgeOut[1]) iup--; }

  int jlow = 0;
  int jup = nbvertic - 1;
  if (quad->isEdgeOut[0]) { jlow++; } else { if (quad->isEdgeOut[2]) jup--; }

  // regular quadrangles
  for (i = ilow; i < iup; i++) {
    for (j = jlow; j < jup; j++) {
      const SMDS_MeshNode *a, *b, *c, *d;
      a = quad->uv_grid[j * nbhoriz + i].node;
      b = quad->uv_grid[j * nbhoriz + i + 1].node;
      c = quad->uv_grid[(j + 1) * nbhoriz + i + 1].node;
      d = quad->uv_grid[(j + 1) * nbhoriz + i].node;
      SMDS_MeshFace * face = meshDS->AddFace(a, b, c, d);
      meshDS->SetMeshElementOnShape(face, geomFaceID);
    }
  }

  UVPtStruct *uv_e0 = quad->uv_edges[0];
  UVPtStruct *uv_e1 = quad->uv_edges[1];
  UVPtStruct *uv_e2 = quad->uv_edges[2];
  UVPtStruct *uv_e3 = quad->uv_edges[3];

  double eps = Precision::Confusion();

  // Boundary quadrangles

  if (quad->isEdgeOut[0]) {
    // Down edge is out
    // 
    // |___|___|___|___|___|___|
    // |   |   |   |   |   |   |
    // |___|___|___|___|___|___|
    // |   |   |   |   |   |   |
    // |___|___|___|___|___|___| __ first row of the regular grid
    // .  .  .  .  .  .  .  .  . __ down edge nodes
    // 
    // >->->->->->->->->->->->-> -- direction of processing

    int g = 0; // number of last processed node in the regular grid

    // number of last node of the down edge to be processed
    int stop = nbdown - 1;
    // if right edge is out, we will stop at a node, previous to the last one
    if (quad->isEdgeOut[1]) stop--;

    // for each node of the down edge find nearest node
    // in the first row of the regular grid and link them
    for (i = 0; i < stop; i++) {
      const SMDS_MeshNode *a, *b, *c, *d;
      a = uv_e0[i].node;
      b = uv_e0[i + 1].node;
      gp_Pnt pb (b->X(), b->Y(), b->Z());

      // find node c in the regular grid, which will be linked with node b
      int near = g;
      if (i == stop - 1) {
        // right bound reached, link with the rightmost node
        near = iup;
        c = quad->uv_grid[nbhoriz + iup].node;
      } else {
        // find in the grid node c, nearest to the b
        double mind = RealLast();
        for (int k = g; k <= iup; k++) {

          const SMDS_MeshNode *nk;
          if (k < ilow) // this can be, if left edge is out
            nk = uv_e3[1].node; // get node from the left edge
          else
            nk = quad->uv_grid[nbhoriz + k].node; // get one of middle nodes

          gp_Pnt pnk (nk->X(), nk->Y(), nk->Z());
          double dist = pb.Distance(pnk);
          if (dist < mind - eps) {
            c = nk;
            near = k;
            mind = dist;
          } else {
            break;
          }
        }
      }

      if (near == g) { // make triangle
        SMDS_MeshFace* face = meshDS->AddFace(a, b, c);
        meshDS->SetMeshElementOnShape(face, geomFaceID);
      } else { // make quadrangle
        if (near - 1 < ilow)
          d = uv_e3[1].node;
        else
          d = quad->uv_grid[nbhoriz + near - 1].node;
        SMDS_MeshFace* face = meshDS->AddFace(a, b, c, d);
        meshDS->SetMeshElementOnShape(face, geomFaceID);

        // if node d is not at position g - make additional triangles
        if (near - 1 > g) {
          for (int k = near - 1; k > g; k--) {
            c = quad->uv_grid[nbhoriz + k].node;
            if (k - 1 < ilow)
              d = uv_e3[1].node;
            else
              d = quad->uv_grid[nbhoriz + k - 1].node;
            SMDS_MeshFace* face = meshDS->AddFace(a, c, d);
            meshDS->SetMeshElementOnShape(face, geomFaceID);
          }
        }
        g = near;
      }
    }
  } else {
    if (quad->isEdgeOut[2]) {
      // Up edge is out
      // 
      // <-<-<-<-<-<-<-<-<-<-<-<-< -- direction of processing
      // 
      // .  .  .  .  .  .  .  .  . __ up edge nodes
      //  ___ ___ ___ ___ ___ ___  __ first row of the regular grid
      // |   |   |   |   |   |   |
      // |___|___|___|___|___|___|
      // |   |   |   |   |   |   |
      // |___|___|___|___|___|___|
      // |   |   |   |   |   |   |

      int g = nbhoriz - 1; // last processed node in the regular grid

      int stop = 0;
      // if left edge is out, we will stop at a second node
      if (quad->isEdgeOut[3]) stop++;

      // for each node of the up edge find nearest node
      // in the first row of the regular grid and link them
      for (i = nbup - 1; i > stop; i--) {
        const SMDS_MeshNode *a, *b, *c, *d;
        a = uv_e2[i].node;
        b = uv_e2[i - 1].node;
        gp_Pnt pb (b->X(), b->Y(), b->Z());

        // find node c in the grid, which will be linked with node b
        int near = g;
        if (i == stop + 1) { // left bound reached, link with the leftmost node
          c = quad->uv_grid[nbhoriz*(nbvertic - 2) + ilow].node;
          near = ilow;
        } else {
          // find node c in the grid, nearest to the b
          double mind = RealLast();
          for (int k = g; k >= ilow; k--) {
            const SMDS_MeshNode *nk;
            if (k > iup)
              nk = uv_e1[nbright - 2].node;
            else
              nk = quad->uv_grid[nbhoriz*(nbvertic - 2) + k].node;
            gp_Pnt pnk (nk->X(), nk->Y(), nk->Z());
            double dist = pb.Distance(pnk);
            if (dist < mind - eps) {
              c = nk;
              near = k;
              mind = dist;
            } else {
              break;
            }
          }
        }

        if (near == g) { // make triangle
          SMDS_MeshFace* face = meshDS->AddFace(a, b, c);
          meshDS->SetMeshElementOnShape(face, geomFaceID);
        } else { // make quadrangle
          if (near + 1 > iup)
            d = uv_e1[nbright - 2].node;
          else
            d = quad->uv_grid[nbhoriz*(nbvertic - 2) + near + 1].node;
          SMDS_MeshFace* face = meshDS->AddFace(a, b, c, d);
          meshDS->SetMeshElementOnShape(face, geomFaceID);

          if (near + 1 < g) { // if d not is at g - make additional triangles
            for (int k = near + 1; k < g; k++) {
              c = quad->uv_grid[nbhoriz*(nbvertic - 2) + k].node;
              if (k + 1 > iup)
                d = uv_e1[nbright - 2].node;
              else
                d = quad->uv_grid[nbhoriz*(nbvertic - 2) + k + 1].node;
              SMDS_MeshFace* face = meshDS->AddFace(a, c, d);
              meshDS->SetMeshElementOnShape(face, geomFaceID);
            }
          }
          g = near;
        }
      }
    }
  }

  // right or left boundary quadrangles
  if (quad->isEdgeOut[1]) {
//    MESSAGE("right edge is out");
    int g = 0; // last processed node in the grid
    int stop = nbright - 1;
    if (quad->isEdgeOut[2]) stop--;
    for (i = 0; i < stop; i++) {
      const SMDS_MeshNode *a, *b, *c, *d;
      a = uv_e1[i].node;
      b = uv_e1[i + 1].node;
      gp_Pnt pb (b->X(), b->Y(), b->Z());

      // find node c in the grid, nearest to the b
      int near = g;
      if (i == stop - 1) { // up bondary reached
        c = quad->uv_grid[nbhoriz*(jup + 1) - 2].node;
        near = jup;
      } else {
        double mind = RealLast();
        for (int k = g; k <= jup; k++) {
          const SMDS_MeshNode *nk;
          if (k < jlow)
            nk = uv_e0[nbdown - 2].node;
          else
            nk = quad->uv_grid[nbhoriz*(k + 1) - 2].node;
          gp_Pnt pnk (nk->X(), nk->Y(), nk->Z());
          double dist = pb.Distance(pnk);
          if (dist < mind - eps) {
            c = nk;
            near = k;
            mind = dist;
          } else {
            break;
          }
        }
      }

      if (near == g) { // make triangle
        SMDS_MeshFace* face = meshDS->AddFace(a, b, c);
        meshDS->SetMeshElementOnShape(face, geomFaceID);
      } else { // make quadrangle
        if (near - 1 < jlow)
          d = uv_e0[nbdown - 2].node;
        else
          d = quad->uv_grid[nbhoriz*near - 2].node;
        SMDS_MeshFace* face = meshDS->AddFace(a, b, c, d);
        meshDS->SetMeshElementOnShape(face, geomFaceID);

        if (near - 1 > g) { // if d not is at g - make additional triangles
          for (int k = near - 1; k > g; k--) {
            c = quad->uv_grid[nbhoriz*(k + 1) - 2].node;
            if (k - 1 < jlow)
              d = uv_e0[nbdown - 2].node;
            else
              d = quad->uv_grid[nbhoriz*k - 2].node;
            SMDS_MeshFace* face = meshDS->AddFace(a, c, d);
            meshDS->SetMeshElementOnShape(face, geomFaceID);
          }
        }
        g = near;
      }
    }
  } else {
    if (quad->isEdgeOut[3]) {
//      MESSAGE("left edge is out");
      int g = nbvertic - 1; // last processed node in the grid
      int stop = 0;
      if (quad->isEdgeOut[0]) stop++;
      for (i = nbleft - 1; i > stop; i--) {
        const SMDS_MeshNode *a, *b, *c, *d;
        a = uv_e3[i].node;
        b = uv_e3[i - 1].node;
        gp_Pnt pb (b->X(), b->Y(), b->Z());

        // find node c in the grid, nearest to the b
        int near = g;
        if (i == stop + 1) { // down bondary reached
          c = quad->uv_grid[nbhoriz*jlow + 1].node;
          near = jlow;
        } else {
          double mind = RealLast();
          for (int k = g; k >= jlow; k--) {
            const SMDS_MeshNode *nk;
            if (k > jup)
              nk = uv_e2[1].node;
            else
              nk = quad->uv_grid[nbhoriz*k + 1].node;
            gp_Pnt pnk (nk->X(), nk->Y(), nk->Z());
            double dist = pb.Distance(pnk);
            if (dist < mind - eps) {
              c = nk;
              near = k;
              mind = dist;
            } else {
              break;
            }
          }
        }

        if (near == g) { // make triangle
          SMDS_MeshFace* face = meshDS->AddFace(a, b, c);
          meshDS->SetMeshElementOnShape(face, geomFaceID);
        } else { // make quadrangle
          if (near + 1 > jup)
            d = uv_e2[1].node;
          else
            d = quad->uv_grid[nbhoriz*(near + 1) + 1].node;
          SMDS_MeshFace* face = meshDS->AddFace(a, b, c, d);
          meshDS->SetMeshElementOnShape(face, geomFaceID);

          if (near + 1 < g) { // if d not is at g - make additional triangles
            for (int k = near + 1; k < g; k++) {
              c = quad->uv_grid[nbhoriz*k + 1].node;
              if (k + 1 > jup)
                d = uv_e2[1].node;
              else
                d = quad->uv_grid[nbhoriz*(k + 1) + 1].node;
              SMDS_MeshFace* face = meshDS->AddFace(a, c, d);
              meshDS->SetMeshElementOnShape(face, geomFaceID);
            }
          }
          g = near;
        }
      }
    }
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

FaceQuadStruct* StdMeshers_Quadrangle_2D::CheckNbEdges(SMESH_Mesh & aMesh,
                                                       const TopoDS_Shape & aShape)
     throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);

  const TopoDS_Face & F = TopoDS::Face(aShape);

  // verify 1 wire only, with 4 edges

  if (NumberOfWires(F) != 1) {
    INFOS("only 1 wire by face (quadrangles)");
    return 0;
  }
  const TopoDS_Wire& W = BRepTools::OuterWire(F);
  BRepTools_WireExplorer wexp (W, F);

  FaceQuadStruct* quad = new FaceQuadStruct;
  for (int i = 0; i < 4; i++)
    quad->uv_edges[i] = 0;
  quad->uv_grid = 0;

  int nbEdges = 0;
  for (wexp.Init(W, F); wexp.More(); wexp.Next()) {
    const TopoDS_Edge& E = wexp.Current();
    int nb = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
    if (nbEdges < 4) {
      quad->edge[nbEdges] = E;
      quad->nbPts[nbEdges] = nb + 2; // internal points + 2 extrema
    }
    nbEdges++;
  }

  if (nbEdges != 4) {
    INFOS("face must have 4 edges /quadrangles");
    QuadDelete(quad);
    return 0;
  }

  return quad;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

FaceQuadStruct *StdMeshers_Quadrangle_2D::CheckAnd2Dcompute
  (SMESH_Mesh & aMesh, const TopoDS_Shape & aShape) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);

  FaceQuadStruct *quad = CheckNbEdges(aMesh, aShape);

  if(!quad) return 0;

  // set normalized grid on unit square in parametric domain
  SetNormalizedGrid(aMesh, aShape, quad);

  return quad;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_Quadrangle_2D::QuadDelete (FaceQuadStruct * quad)
{
  //MESSAGE("StdMeshers_Quadrangle_2D::QuadDelete");
  if (quad)
  {
    for (int i = 0; i < 4; i++)
    {
      if (quad->uv_edges[i])
        delete [] quad->uv_edges[i];
      quad->edge[i].Nullify();
    }
    if (quad->uv_grid)
      delete [] quad->uv_grid;
    delete quad;
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_Quadrangle_2D::SetNormalizedGrid (SMESH_Mesh & aMesh,
                                                  const TopoDS_Shape& aShape,
                                                  FaceQuadStruct* quad) throw (SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  // Algorithme décrit dans "Génération automatique de maillages"
  // P.L. GEORGE, MASSON, § 6.4.1 p. 84-85
  // traitement dans le domaine paramétrique 2d u,v
  // transport - projection sur le carré unité

//  MESSAGE("StdMeshers_Quadrangle_2D::SetNormalizedGrid");
  const TopoDS_Face& F = TopoDS::Face(aShape);

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
    c2d[i] = BRep_Tool::CurveOnSurface(quad->edge[i], F,
                                       quad->first[i], quad->last[i]);
    pf[i] = c2d[i]->Value(quad->first[i]);
    pl[i] = c2d[i]->Value(quad->last[i]);
    quad->isEdgeForward[i] = false;
  }

  double l0f1 = pl[0].SquareDistance(pf[1]);
  double l0l1 = pl[0].SquareDistance(pl[1]);
  double f0f1 = pf[0].SquareDistance(pf[1]);
  double f0l1 = pf[0].SquareDistance(pl[1]);
  if ( Min( l0f1, l0l1 ) < Min ( f0f1, f0l1 ))
  {
    quad->isEdgeForward[0] = true;
  } else {
    double tmp = quad->first[0];
    quad->first[0] = quad->last[0];
    quad->last[0] = tmp;
    pf[0] = c2d[0]->Value(quad->first[0]);
    pl[0] = c2d[0]->Value(quad->last[0]);
  }
  for (int i = 1; i < 4; i++)
  {
    l0l1 = pl[i - 1].SquareDistance(pl[i]);
    l0f1 = pl[i - 1].SquareDistance(pf[i]);
    quad->isEdgeForward[i] = ( l0f1 < l0l1 );
    if (!quad->isEdgeForward[i])
    {
      double tmp = quad->first[i];
      quad->first[i] = quad->last[i];
      quad->last[i] = tmp;
      pf[i] = c2d[i]->Value(quad->first[i]);
      pl[i] = c2d[i]->Value(quad->last[i]);
    }
  }

  // 2 --- load 2d edge points (u,v) with orientation and value on unit square

  bool loadOk = true;
  for (int i = 0; i < 2; i++)
  {
    quad->uv_edges[i] = LoadEdgePoints(aMesh, F, quad->edge[i],
                                       quad->first[i], quad->last[i]);
    if (!quad->uv_edges[i]) loadOk = false;
  }

  for (int i = 2; i < 4; i++)
  {
    quad->uv_edges[i] = LoadEdgePoints(aMesh, F, quad->edge[i],
                                       quad->last[i], quad->first[i]);
    if (!quad->uv_edges[i]) loadOk = false;
  }

  if (!loadOk)
  {
    INFOS("StdMeshers_Quadrangle_2D::SetNormalizedGrid - LoadEdgePoints failed");
    QuadDelete( quad );
    quad = 0;
    return;
  }
  // 3 --- 2D normalized values on unit square [0..1][0..1]

  int nbhoriz  = Min(quad->nbPts[0], quad->nbPts[2]);
  int nbvertic = Min(quad->nbPts[1], quad->nbPts[3]);

  quad->isEdgeOut[0] = (quad->nbPts[0] > quad->nbPts[2]);
  quad->isEdgeOut[1] = (quad->nbPts[1] > quad->nbPts[3]);
  quad->isEdgeOut[2] = (quad->nbPts[2] > quad->nbPts[0]);
  quad->isEdgeOut[3] = (quad->nbPts[3] > quad->nbPts[1]);

  quad->uv_grid = new UVPtStruct[nbvertic * nbhoriz];

  UVPtStruct *uv_grid = quad->uv_grid;
  UVPtStruct *uv_e0 = quad->uv_edges[0];
  UVPtStruct *uv_e1 = quad->uv_edges[1];
  UVPtStruct *uv_e2 = quad->uv_edges[2];
  UVPtStruct *uv_e3 = quad->uv_edges[3];

  // nodes Id on "in" edges
  if (! quad->isEdgeOut[0]) {
    int j = 0;
    for (int i = 0; i < nbhoriz; i++) { // down
      int ij = j * nbhoriz + i;
      uv_grid[ij].node = uv_e0[i].node;
    }
  }
  if (! quad->isEdgeOut[1]) {
    int i = nbhoriz - 1;
    for (int j = 0; j < nbvertic; j++) { // right
      int ij = j * nbhoriz + i;
      uv_grid[ij].node = uv_e1[j].node;
    }
  }
  if (! quad->isEdgeOut[2]) {
    int j = nbvertic - 1;
    for (int i = 0; i < nbhoriz; i++) { // up
      int ij = j * nbhoriz + i;
      uv_grid[ij].node = uv_e2[i].node;
    }
  }
  if (! quad->isEdgeOut[3]) {
    int i = 0;
    for (int j = 0; j < nbvertic; j++) { // left
      int ij = j * nbhoriz + i;
      uv_grid[ij].node = uv_e3[j].node;
    }
  }

  // falsificate "out" edges
  if (quad->isEdgeOut[0]) // down
    uv_e0 = MakeEdgePoints
      (aMesh, F, quad->edge[0], quad->first[0], quad->last[0], nbhoriz - 1);
  else if (quad->isEdgeOut[2]) // up
    uv_e2 = MakeEdgePoints
      (aMesh, F, quad->edge[2], quad->last[2], quad->first[2], nbhoriz - 1);

  if (quad->isEdgeOut[1]) // right
    uv_e1 = MakeEdgePoints
      (aMesh, F, quad->edge[1], quad->first[1], quad->last[1], nbvertic - 1);
  else if (quad->isEdgeOut[3]) // left
    uv_e3 = MakeEdgePoints
      (aMesh, F, quad->edge[3], quad->last[3], quad->first[3], nbvertic - 1);

  // normalized 2d values on grid
  for (int i = 0; i < nbhoriz; i++)
  {
    for (int j = 0; j < nbvertic; j++)
    {
      int ij = j * nbhoriz + i;
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
  }

  // 4 --- projection on 2d domain (u,v)
  gp_Pnt2d a0 = pf[0];
  gp_Pnt2d a1 = pf[1];
  gp_Pnt2d a2 = pf[2];
  gp_Pnt2d a3 = pf[3];

  for (int i = 0; i < nbhoriz; i++)
  {
    for (int j = 0; j < nbvertic; j++)
    {
      int ij = j * nbhoriz + i;
      double x = uv_grid[ij].x;
      double y = uv_grid[ij].y;
      double param_0 = uv_e0[0].param + x * (uv_e0[nbhoriz - 1].param - uv_e0[0].param); // sud
      double param_2 = uv_e2[0].param + x * (uv_e2[nbhoriz - 1].param - uv_e2[0].param); // nord
      double param_1 = uv_e1[0].param + y * (uv_e1[nbvertic - 1].param - uv_e1[0].param); // est
      double param_3 = uv_e3[0].param + y * (uv_e3[nbvertic - 1].param - uv_e3[0].param); // ouest

      //MESSAGE("params "<<param_0<<" "<<param_1<<" "<<param_2<<" "<<param_3);
      gp_Pnt2d p0 = c2d[0]->Value(param_0);
      gp_Pnt2d p1 = c2d[1]->Value(param_1);
      gp_Pnt2d p2 = c2d[2]->Value(param_2);
      gp_Pnt2d p3 = c2d[3]->Value(param_3);

      double u = (1 - y) * p0.X() + x * p1.X() + y * p2.X() + (1 - x) * p3.X();
      double v = (1 - y) * p0.Y() + x * p1.Y() + y * p2.Y() + (1 - x) * p3.Y();

      u -= (1 - x) * (1 - y) * a0.X() + x * (1 - y) * a1.X() +
        x * y * a2.X() + (1 - x) * y * a3.X();
      v -= (1 - x) * (1 - y) * a0.Y() + x * (1 - y) * a1.Y() +
        x * y * a2.Y() + (1 - x) * y * a3.Y();

      uv_grid[ij].u = u;
      uv_grid[ij].v = v;
    }
  }
}


//=======================================================================
//function : ShiftQuad
//purpose  : auxilary function for ComputeQuadPref
//=======================================================================
static void ShiftQuad(FaceQuadStruct* quad, const int num, bool WisF)
{
  if(num>3) return;
  int i;
  for(i=1; i<=num; i++) {
    int nbPts3 = quad->nbPts[0];
    quad->nbPts[0] = quad->nbPts[1];
    quad->nbPts[1] = quad->nbPts[2];
    quad->nbPts[2] = quad->nbPts[3];
    quad->nbPts[3] = nbPts3;
    TopoDS_Edge edge3 = quad->edge[0];
    quad->edge[0] = quad->edge[1];
    quad->edge[1] = quad->edge[2];
    quad->edge[2] = quad->edge[3];
    quad->edge[3] = edge3;
    double first3 = quad->first[0];
    quad->first[0] = quad->first[1];
    quad->first[1] = quad->first[2];
    quad->first[2] = quad->first[3];
    quad->first[3] = first3;
    double last3 = quad->last[0];
    quad->last[0] = quad->last[1];
    quad->last[1] = quad->last[2];
    quad->last[2] = quad->last[3];
    quad->last[3] = last3;
    bool isEdgeForward3 = quad->isEdgeForward[0];
    quad->isEdgeForward[0] = quad->isEdgeForward[1];
    quad->isEdgeForward[1] = quad->isEdgeForward[2];
    quad->isEdgeForward[2] = quad->isEdgeForward[3];
    quad->isEdgeForward[3] = isEdgeForward3;
    bool isEdgeOut3 = quad->isEdgeOut[0];
    quad->isEdgeOut[0] = quad->isEdgeOut[1];
    quad->isEdgeOut[1] = quad->isEdgeOut[2];
    quad->isEdgeOut[2] = quad->isEdgeOut[3];
    quad->isEdgeOut[3] = isEdgeOut3;
    UVPtStruct* uv_edges3 = quad->uv_edges[0];
    quad->uv_edges[0] = quad->uv_edges[1];
    quad->uv_edges[1] = quad->uv_edges[2];
    quad->uv_edges[2] = quad->uv_edges[3];
    quad->uv_edges[3] = uv_edges3;
  }
  if(!WisF) {
    // replacement left and right edges
    int nbPts3 = quad->nbPts[1];
    quad->nbPts[1] = quad->nbPts[3];
    quad->nbPts[3] = nbPts3;
    TopoDS_Edge edge3 = quad->edge[1];
    quad->edge[1] = quad->edge[3];
    quad->edge[3] = edge3;
    double first3 = quad->first[1];
    quad->first[1] = quad->first[3];
    quad->first[3] = first3;
    double last3 = quad->last[1];
    quad->last[1] = quad->last[2];
    quad->last[3] = last3;
    bool isEdgeForward3 = quad->isEdgeForward[1];
    quad->isEdgeForward[1] = quad->isEdgeForward[3];
    quad->isEdgeForward[3] = isEdgeForward3;
    bool isEdgeOut3 = quad->isEdgeOut[1];
    quad->isEdgeOut[1] = quad->isEdgeOut[3];
    quad->isEdgeOut[3] = isEdgeOut3;
    UVPtStruct* uv_edges3 = quad->uv_edges[1];
    quad->uv_edges[1] = quad->uv_edges[3];
    quad->uv_edges[3] = uv_edges3;
  }
}


//=======================================================================
//function : CalcUV
//purpose  : auxilary function for ComputeQuadPref
//=======================================================================
static gp_XY CalcUV(double x0, double x1, double y0, double y1,
                    FaceQuadStruct* quad,
                    const gp_Pnt2d& a0, const gp_Pnt2d& a1,
                    const gp_Pnt2d& a2, const gp_Pnt2d& a3,
                    const Handle(Geom2d_Curve)& c2db,
                    const Handle(Geom2d_Curve)& c2dr,
                    const Handle(Geom2d_Curve)& c2dt,
                    const Handle(Geom2d_Curve)& c2dl)
{
  int nb = quad->nbPts[0];
  int nr = quad->nbPts[1];
  int nt = quad->nbPts[2];
  int nl = quad->nbPts[3];

  UVPtStruct* uv_eb = quad->uv_edges[0];
  UVPtStruct* uv_er = quad->uv_edges[1];
  UVPtStruct* uv_et = quad->uv_edges[2];
  UVPtStruct* uv_el = quad->uv_edges[3];

  double x = (x0 + y0 * (x1 - x0)) / (1 - (y1 - y0) * (x1 - x0));
  double y = y0 + x * (y1 - y0);

  double param_b = uv_eb[0].param + x * (uv_eb[nb-1].param - uv_eb[0].param);
  double param_t = uv_et[0].param + x * (uv_et[nt-1].param - uv_et[0].param);
  double param_r = uv_er[0].param + y * (uv_er[nr-1].param - uv_er[0].param);
  double param_l = uv_el[0].param + y * (uv_el[nl-1].param - uv_el[0].param);

  gp_Pnt2d p0 = c2db->Value(param_b);
  gp_Pnt2d p1 = c2dr->Value(param_r);
  gp_Pnt2d p2 = c2dt->Value(param_t);
  gp_Pnt2d p3 = c2dl->Value(param_l);

  double u = (1 - y) * p0.X() + x * p1.X() + y * p2.X() + (1 - x) * p3.X();
  double v = (1 - y) * p0.Y() + x * p1.Y() + y * p2.Y() + (1 - x) * p3.Y();

  u -= (1 - x) * (1 - y) * a0.X() + x * (1 - y) * a1.X() +
    x * y * a2.X() + (1 - x) * y * a3.X();
  v -= (1 - x) * (1 - y) * a0.Y() + x * (1 - y) * a1.Y() +
    x * y * a2.Y() + (1 - x) * y * a3.Y();

  //cout<<"x0="<<x0<<" x1="<<x1<<" y0="<<y0<<" y1="<<y1<<endl;
  //cout<<"x="<<x<<" y="<<y<<endl;
  //cout<<"param_b="<<param_b<<" param_t="<<param_t<<" param_r="<<param_r<<" param_l="<<param_l<<endl;
  //cout<<"u="<<u<<" v="<<v<<endl;

  return gp_XY(u,v);
}


//=======================================================================
//function : ComputeQuadPref
//purpose  : 
//=======================================================================
/*!
 * Special function for creation only quandrangle faces
 */
bool StdMeshers_Quadrangle_2D::ComputeQuadPref
                          (SMESH_Mesh & aMesh,
                           const TopoDS_Shape& aShape,
                           FaceQuadStruct* quad) throw (SALOME_Exception)
{
  Unexpect aCatch(SalomeException);

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  const TopoDS_Face& F = TopoDS::Face(aShape);
  Handle(Geom_Surface) S = BRep_Tool::Surface(F);
  const TopoDS_Wire& W = BRepTools::OuterWire(F);
  bool WisF = false;
  if(W.Orientation()==TopAbs_FORWARD) 
    WisF = true;
  //if(WisF) cout<<"W is FORWARD"<<endl;
  //else cout<<"W is REVERSED"<<endl;
  bool FisF = (F.Orientation()==TopAbs_FORWARD);
  if(!FisF) WisF = !WisF;
  int i,j,geomFaceID = meshDS->ShapeToIndex( F );

  int nb = quad->nbPts[0];
  int nr = quad->nbPts[1];
  int nt = quad->nbPts[2];
  int nl = quad->nbPts[3];
  int dh = abs(nb-nt);
  int dv = abs(nr-nl);

  if( dh>=dv ) {
    if( nt>nb ) {
      // it is a base case => not shift quad but me be replacement is need
      ShiftQuad(quad,0,WisF);
    }
    else {
      // we have to shift quad on 2
      ShiftQuad(quad,2,WisF);
    }
  }
  else {
    if( nr>nl ) {
      // we have to shift quad on 3
      ShiftQuad(quad,3,WisF);
    }
    else {
      // we have to shift quad on 1
      ShiftQuad(quad,1,WisF);
    }
  }

  nb = quad->nbPts[0];
  nr = quad->nbPts[1];
  nt = quad->nbPts[2];
  nl = quad->nbPts[3];
  dh = abs(nb-nt);
  dv = abs(nr-nl);
  int nbh  = Max(nb,nt);
  int nbv = Max(nr,nl);
  int addh = 0;
  int addv = 0;

  // orientation of face and 3 main domain for future faces
  //       0   top    1
  //      1------------1
  //       |   |  |   |
  //       |   |  |   |
  //       | L |  | R |
  //  left |   |  |   | rigth
  //       |  /    \  |
  //       | /  C   \ |
  //       |/        \|
  //      0------------0
  //       0  bottom  1

  if(dh>dv) {
    addv = (dh-dv)/2;
    nbv = nbv + addv;
  }
  else { // dv>=dh
    addh = (dv-dh)/2;
    nbh = nbh + addh;
  }

  Handle(Geom2d_Curve) c2d[4];
  for(i=0; i<4; i++) {
    c2d[i] = BRep_Tool::CurveOnSurface(quad->edge[i], F,
                                       quad->first[i], quad->last[i]);
  }

  bool loadOk = true;
  for(i=0; i<2; i++) {
    quad->uv_edges[i] = LoadEdgePoints2(aMesh, F, quad->edge[i], false);
    if(!quad->uv_edges[i]) loadOk = false;
  }
  for(i=2; i<4; i++) {
    quad->uv_edges[i] = LoadEdgePoints2(aMesh, F, quad->edge[i], true);
    if (!quad->uv_edges[i]) loadOk = false;
  }
  if (!loadOk) {
    INFOS("StdMeshers_Quadrangle_2D::ComputeQuadPref - LoadEdgePoints failed");
    QuadDelete( quad );
    quad = 0;
    return false;
  }

  UVPtStruct* uv_eb = quad->uv_edges[0];
  UVPtStruct* uv_er = quad->uv_edges[1];
  UVPtStruct* uv_et = quad->uv_edges[2];
  UVPtStruct* uv_el = quad->uv_edges[3];

  // arrays for normalized params
  //cout<<"Dump B:"<<endl;
  TColStd_SequenceOfReal npb, npr, npt, npl;
  for(i=0; i<nb; i++) {
    npb.Append(uv_eb[i].normParam);
    //cout<<"i="<<i<<" par="<<uv_eb[i].param<<" npar="<<uv_eb[i].normParam;
    //const SMDS_MeshNode* N = uv_eb[i].node;
    //cout<<" node("<<N->X()<<","<<N->Y()<<","<<N->Z()<<")"<<endl;
  }
  for(i=0; i<nr; i++) {
    npr.Append(uv_er[i].normParam);
  }
  for(i=0; i<nt; i++) {
    npt.Append(uv_et[i].normParam);
  }
  for(i=0; i<nl; i++) {
    npl.Append(uv_el[i].normParam);
  }

  // we have to add few values of params to right and left
  // insert them after first param
  // insert to right
  int dr = nbv - nr;
  double dpr = (npr.Value(2) - npr.Value(1))/(dr+1);
  for(i=1; i<=dr; i++) {
    npr.InsertAfter(1,npr.Value(2)-dpr);
  }
  // insert to left
  int dl = nbv - nl;
  dpr = (npl.Value(2) - npl.Value(1))/(dl+1);
  for(i=1; i<=dl; i++) {
    npl.InsertAfter(1,npl.Value(2)-dpr);
  }
  //cout<<"npb:";
  //for(i=1; i<=npb.Length(); i++) {
  //  cout<<" "<<npb.Value(i);
  //}
  //cout<<endl;
  
  gp_Pnt2d a[4];
  c2d[0]->D0(uv_eb[0].param,a[0]);
  c2d[0]->D0(uv_eb[nb-1].param,a[1]);
  c2d[2]->D0(uv_et[nt-1].param,a[2]);
  c2d[2]->D0(uv_et[0].param,a[3]);
  //cout<<" a[0]("<<a[0].X()<<","<<a[0].Y()<<")"<<" a[1]("<<a[1].X()<<","<<a[1].Y()<<")"
  //    <<" a[2]("<<a[2].X()<<","<<a[2].Y()<<")"<<" a[3]("<<a[3].X()<<","<<a[3].Y()<<")"<<endl;

  int nnn = Min(nr,nl);
  // auxilary sequence of XY for creation nodes
  // in the bottom part of central domain
  // it's length must be == nbv-nnn-1
  TColgp_SequenceOfXY UVL;
  TColgp_SequenceOfXY UVR;

  // step1: create faces for left domain
  StdMeshers_Array2OfNode NodesL(1,dl+1,1,nl);
  // add left nodes
  for(j=1; j<=nl; j++) 
    NodesL.SetValue(1,j,uv_el[j-1].node);
  if(dl>0) {
    // add top nodes
    for(i=1; i<=dl; i++) 
      NodesL.SetValue(i+1,nl,uv_et[i].node);
    // create and add needed nodes
    TColgp_SequenceOfXY UVtmp;
    for(i=1; i<=dl; i++) {
      double x0 = npt.Value(i+1);
      double x1 = x0;
      // diagonal node
      double y0 = npl.Value(i+1);
      double y1 = npr.Value(i+1);
      gp_XY UV = CalcUV(x0, x1, y0, y1, quad, a[0], a[1], a[2], a[3],
                        c2d[0], c2d[1], c2d[2], c2d[3]);
      gp_Pnt P = S->Value(UV.X(),UV.Y());
      SMDS_MeshNode * N = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnFace(N, geomFaceID, UV.X(), UV.Y());
      NodesL.SetValue(i+1,1,N);
      if(UVL.Length()<nbv-nnn-1) UVL.Append(UV);
      // internal nodes
      for(j=2; j<nl; j++) {
        double y0 = npl.Value(dl+j);
        double y1 = npr.Value(dl+j);
        gp_XY UV = CalcUV(x0, x1, y0, y1, quad, a[0], a[1], a[2], a[3],
                          c2d[0], c2d[1], c2d[2], c2d[3]);
        gp_Pnt P = S->Value(UV.X(),UV.Y());
        SMDS_MeshNode* N = meshDS->AddNode(P.X(), P.Y(), P.Z());
        meshDS->SetNodeOnFace(N, geomFaceID, UV.X(), UV.Y());
        NodesL.SetValue(i+1,j,N);
        if( i==dl ) UVtmp.Append(UV);
      }
    }
    for(i=1; i<=UVtmp.Length() && UVL.Length()<nbv-nnn-1; i++) {
      UVL.Append(UVtmp.Value(i));
    }
    //cout<<"Dump NodesL:"<<endl;
    //for(i=1; i<=dl+1; i++) {
    //  cout<<"i="<<i;
    //  for(j=1; j<=nl; j++) {
    //    cout<<" ("<<NodesL.Value(i,j)->X()<<","<<NodesL.Value(i,j)->Y()<<","<<NodesL.Value(i,j)->Z()<<")";
    //  }
    //  cout<<endl;
    //}
    // create faces
    for(i=1; i<=dl; i++) {
      for(j=1; j<nl; j++) {
        if(WisF) {
          SMDS_MeshFace* F =
            meshDS->AddFace(NodesL.Value(i,j), NodesL.Value(i+1,j),
                            NodesL.Value(i+1,j+1), NodesL.Value(i,j+1));
          meshDS->SetMeshElementOnShape(F, geomFaceID);
        }
        else {
          SMDS_MeshFace* F =
            meshDS->AddFace(NodesL.Value(i,j), NodesL.Value(i,j+1),
                            NodesL.Value(i+1,j+1), NodesL.Value(i+1,j));
          meshDS->SetMeshElementOnShape(F, geomFaceID);
        }
      }
    }
  }
  else {
    // fill UVL using c2d
    for(i=1; i<npl.Length() && UVL.Length()<nbv-nnn-1; i++) {
      gp_Pnt2d p2d;
      c2d[3]->D0(uv_el[i].param,p2d);
      UVL.Append(p2d.XY());
    }
  }

  // step2: create faces for right domain
  StdMeshers_Array2OfNode NodesR(1,dr+1,1,nr);
  // add right nodes
  for(j=1; j<=nr; j++) 
    NodesR.SetValue(1,j,uv_er[nr-j].node);
  if(dr>0) {
    // add top nodes
    for(i=1; i<=dr; i++) 
      NodesR.SetValue(i+1,1,uv_et[nt-1-i].node);
    // create and add needed nodes
    TColgp_SequenceOfXY UVtmp;
    for(i=1; i<=dr; i++) {
      double x0 = npt.Value(nt-i);
      double x1 = x0;
      // diagonal node
      double y0 = npl.Value(i+1);
      double y1 = npr.Value(i+1);
      gp_XY UV = CalcUV(x0, x1, y0, y1, quad, a[0], a[1], a[2], a[3],
                        c2d[0], c2d[1], c2d[2], c2d[3]);
      gp_Pnt P = S->Value(UV.X(),UV.Y());
      SMDS_MeshNode * N = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnFace(N, geomFaceID, UV.X(), UV.Y());
      NodesR.SetValue(i+1,nr,N);
      if(UVR.Length()<nbv-nnn-1) UVR.Append(UV);
      // internal nodes
      for(j=2; j<nr; j++) {
        double y0 = npl.Value(nbv-j+1);
        double y1 = npr.Value(nbv-j+1);
        gp_XY UV = CalcUV(x0, x1, y0, y1, quad, a[0], a[1], a[2], a[3],
                          c2d[0], c2d[1], c2d[2], c2d[3]);
        gp_Pnt P = S->Value(UV.X(),UV.Y());
        SMDS_MeshNode* N = meshDS->AddNode(P.X(), P.Y(), P.Z());
        meshDS->SetNodeOnFace(N, geomFaceID, UV.X(), UV.Y());
        NodesR.SetValue(i+1,j,N);
        if( i==dr ) UVtmp.Prepend(UV);
      }
    }
    for(i=1; i<=UVtmp.Length() && UVR.Length()<nbv-nnn-1; i++) {
      UVR.Append(UVtmp.Value(i));
    }
    // create faces
    for(i=1; i<=dr; i++) {
      for(j=1; j<nr; j++) {
        if(WisF) {
          SMDS_MeshFace* F =
            meshDS->AddFace(NodesR.Value(i,j), NodesR.Value(i+1,j),
                            NodesR.Value(i+1,j+1), NodesR.Value(i,j+1));
          meshDS->SetMeshElementOnShape(F, geomFaceID);
        }
        else {
          SMDS_MeshFace* F =
            meshDS->AddFace(NodesR.Value(i,j), NodesR.Value(i,j+1),
                            NodesR.Value(i+1,j+1), NodesR.Value(i+1,j));
          meshDS->SetMeshElementOnShape(F, geomFaceID);
        }
      }
    }
  }
  else {
    // fill UVR using c2d
    for(i=1; i<npr.Length() && UVR.Length()<nbv-nnn-1; i++) {
      gp_Pnt2d p2d;
      c2d[1]->D0(uv_er[i].param,p2d);
      UVR.Append(p2d.XY());
    }
  }

  // step3: create faces for central domain
  StdMeshers_Array2OfNode NodesC(1,nb,1,nbv);
  // add first string using NodesL
  for(i=1; i<=dl+1; i++)
    NodesC.SetValue(1,i,NodesL(i,1));
  for(i=2; i<=nl; i++)
    NodesC.SetValue(1,dl+i,NodesL(dl+1,i));
  // add last string using NodesR
  for(i=1; i<=dr+1; i++)
    NodesC.SetValue(nb,i,NodesR(i,nr));
  for(i=1; i<nr; i++)
    NodesC.SetValue(nb,dr+i+1,NodesR(dr+1,nr-i));
  // add top nodes (last columns)
  for(i=dl+2; i<nbh-dr; i++) 
    NodesC.SetValue(i-dl,nbv,uv_et[i-1].node);
  // add bottom nodes (first columns)
  for(i=2; i<nb; i++) {
    NodesC.SetValue(i,1,uv_eb[i-1].node);
    gp_Pnt2d p2d;
    c2d[0]->D0(uv_eb[i-1].param,p2d);
  }
  // create and add needed nodes
  // add linear layers
  for(i=2; i<nb; i++) {
    double x0 = npt.Value(dl+i);
    double x1 = x0;
    for(j=1; j<nnn; j++) {
      double y0 = npl.Value(nbv-nnn+j);
      double y1 = npr.Value(nbv-nnn+j);
      gp_XY UV = CalcUV(x0, x1, y0, y1, quad, a[0], a[1], a[2], a[3],
                        c2d[0], c2d[1], c2d[2], c2d[3]);
      gp_Pnt P = S->Value(UV.X(),UV.Y());
      SMDS_MeshNode* N = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnFace(N, geomFaceID, UV.X(), UV.Y());
      NodesC.SetValue(i,nbv-nnn+j,N);
    }
  }
  // add diagonal layers
  //cout<<"UVL.Length()="<<UVL.Length()<<" UVR.Length()="<<UVR.Length()<<endl;
  //cout<<"Dump UVL:"<<endl;
  //for(i=1; i<=UVL.Length(); i++) {
  //  cout<<" ("<<UVL.Value(i).X()<<","<<UVL.Value(i).Y()<<")";
  //}
  //cout<<endl;
  for(i=1; i<nbv-nnn; i++) {
    double du = UVR.Value(i).X() - UVL.Value(i).X();
    double dv = UVR.Value(i).Y() - UVL.Value(i).Y();
    for(j=2; j<nb; j++) {
      double u = UVL.Value(i).X() + du*npb.Value(j);
      double v = UVL.Value(i).Y() + dv*npb.Value(j);
      gp_Pnt P = S->Value(u,v);
      SMDS_MeshNode* N = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnFace(N, geomFaceID, u, v);
      NodesC.SetValue(j,i+1,N);
    }
  }
  // create faces
  for(i=1; i<nb; i++) {
    for(j=1; j<nbv; j++) {
      if(WisF) {
        SMDS_MeshFace* F =
          meshDS->AddFace(NodesC.Value(i,j), NodesC.Value(i+1,j),
                          NodesC.Value(i+1,j+1), NodesC.Value(i,j+1));
        meshDS->SetMeshElementOnShape(F, geomFaceID);
      }
      else {
        SMDS_MeshFace* F =
          meshDS->AddFace(NodesC.Value(i,j), NodesC.Value(i,j+1),
                          NodesC.Value(i+1,j+1), NodesC.Value(i+1,j));
        meshDS->SetMeshElementOnShape(F, geomFaceID);
      }
    }
  }

  QuadDelete(quad);
  bool isOk = true;
  return isOk;
}


//=============================================================================
/*!
 *  LoadEdgePoints2
 */
//=============================================================================
UVPtStruct* StdMeshers_Quadrangle_2D::LoadEdgePoints2 (SMESH_Mesh & aMesh,
                                                       const TopoDS_Face& F,
                                                       const TopoDS_Edge& E,
                                                       bool IsReverse)
{
  //MESSAGE("StdMeshers_Quadrangle_2D::LoadEdgePoints");
  // --- IDNodes of first and last Vertex
  TopoDS_Vertex VFirst, VLast;
  TopExp::Vertices(E, VFirst, VLast); // corresponds to f and l

  ASSERT(!VFirst.IsNull());
  SMDS_NodeIteratorPtr lid = aMesh.GetSubMesh(VFirst)->GetSubMeshDS()->GetNodes();
  if (!lid->more()) {
    MESSAGE ( "NO NODE BUILT ON VERTEX" );
    return 0;
  }
  const SMDS_MeshNode* idFirst = lid->next();

  ASSERT(!VLast.IsNull());
  lid = aMesh.GetSubMesh(VLast)->GetSubMeshDS()->GetNodes();
  if (!lid->more()) {
    MESSAGE ( "NO NODE BUILT ON VERTEX" );
    return 0;
  }
  const SMDS_MeshNode* idLast = lid->next();

  // --- edge internal IDNodes (relies on good order storage, not checked)

  map<double, const SMDS_MeshNode *> params;
  SMDS_NodeIteratorPtr ite = aMesh.GetSubMesh(E)->GetSubMeshDS()->GetNodes();

  while(ite->more()) {
    const SMDS_MeshNode* node = ite->next();
    const SMDS_EdgePosition* epos =
      static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
    double param = epos->GetUParameter();
    params[param] = node;
  }

  int nbPoints = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
  if (nbPoints != params.size()) {
    MESSAGE( "BAD NODE ON EDGE POSITIONS" );
    return 0;
  }
  UVPtStruct* uvslf = new UVPtStruct[nbPoints + 2];

  double f, l;
  Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);

  const TopoDS_Wire& W = BRepTools::OuterWire(F);
  bool FisF = (F.Orientation()==TopAbs_FORWARD);
  bool WisF = (W.Orientation()==TopAbs_FORWARD);
  bool isForward = (E.Orientation()==TopAbs_FORWARD);
  //if(isForward) cout<<"E is FORWARD"<<endl;
  //else cout<<"E is REVERSED"<<endl;
  if(!WisF) isForward = !isForward;
  if(!FisF) isForward = !isForward;
  //bool isForward = !(E.Orientation()==TopAbs_FORWARD);
  if(IsReverse) isForward = !isForward;
  double paramin = 0;
  double paramax = 0;
  if (isForward) {
    paramin = f;
    paramax = l;
    gp_Pnt2d p = C2d->Value(f);	// first point = Vertex Forward
    uvslf[0].x = p.X();
    uvslf[0].y = p.Y();
    uvslf[0].param = f;
    uvslf[0].node = idFirst;
    //MESSAGE("__ f "<<f<<" "<<uvslf[0].x <<" "<<uvslf[0].y);
    map < double, const SMDS_MeshNode* >::iterator itp = params.begin();
    for (int i = 1; i <= nbPoints; i++)	{ // nbPoints internal
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
  else {
    paramin = l;
    paramax = f;
    gp_Pnt2d p = C2d->Value(l);	// first point = Vertex Reversed
    uvslf[0].x = p.X();
    uvslf[0].y = p.Y();
    uvslf[0].param = l;
    uvslf[0].node = idLast;
    //MESSAGE("__ l "<<l<<" "<<uvslf[0].x <<" "<<uvslf[0].y);
    map < double, const SMDS_MeshNode* >::reverse_iterator itp = params.rbegin();
    for (int j = nbPoints; j >= 1; j--)	{ // nbPoints internal
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
  for (int i = 0; i < nbPoints + 2; i++) {
    uvslf[i].normParam = (uvslf[i].param - paramin) / (paramax - paramin);
  }

  return uvslf;
}


//=============================================================================
/*!
 *  LoadEdgePoints
 */
//=============================================================================
UVPtStruct* StdMeshers_Quadrangle_2D::LoadEdgePoints (SMESH_Mesh & aMesh,
                                                      const TopoDS_Face& F,
                                                      const TopoDS_Edge& E,
                                                      double first, double last)
//                        bool isForward)
{
  //MESSAGE("StdMeshers_Quadrangle_2D::LoadEdgePoints");

  // --- IDNodes of first and last Vertex

  TopoDS_Vertex VFirst, VLast;
  TopExp::Vertices(E, VFirst, VLast); // corresponds to f and l

  ASSERT(!VFirst.IsNull());
  SMDS_NodeIteratorPtr lid = aMesh.GetSubMesh(VFirst)->GetSubMeshDS()->GetNodes();
  if (!lid->more())
  {
    MESSAGE ( "NO NODE BUILT ON VERTEX" );
    return 0;
  }
  const SMDS_MeshNode* idFirst = lid->next();

  ASSERT(!VLast.IsNull());
  lid = aMesh.GetSubMesh(VLast)->GetSubMeshDS()->GetNodes();
  if (!lid->more())
  {
    MESSAGE ( "NO NODE BUILT ON VERTEX" );
    return 0;
  }
  const SMDS_MeshNode* idLast = lid->next();

  // --- edge internal IDNodes (relies on good order storage, not checked)

  map<double, const SMDS_MeshNode *> params;
  SMDS_NodeIteratorPtr ite = aMesh.GetSubMesh(E)->GetSubMeshDS()->GetNodes();

  while(ite->more())
  {
    const SMDS_MeshNode* node = ite->next();
    const SMDS_EdgePosition* epos =
      static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
    double param = epos->GetUParameter();
    params[param] = node;
  }

  int nbPoints = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
  if (nbPoints != params.size())
  {
    MESSAGE( "BAD NODE ON EDGE POSITIONS" );
    return 0;
  }
  UVPtStruct* uvslf = new UVPtStruct[nbPoints + 2];

  double f, l;
  Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);

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
  } else
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
  }

  return uvslf;
}

//=============================================================================
/*!
 *  MakeEdgePoints
 */
//=============================================================================
UVPtStruct* StdMeshers_Quadrangle_2D::MakeEdgePoints (SMESH_Mesh & aMesh,
                                                      const TopoDS_Face& F,
                                                      const TopoDS_Edge& E,
                                                      double first, double last,
                                                      int nb_segm)
{
//  MESSAGE("StdMeshers_Quadrangle_2D::MakeEdgePoints");

  UVPtStruct* uvslf = new UVPtStruct[nb_segm + 1];
  list<double> params;

  // --- edge internal points
  double fi, li;
  Handle(Geom_Curve) Curve = BRep_Tool::Curve(E, fi, li);
  if (!Curve.IsNull()) {
    try {
      GeomAdaptor_Curve C3d (Curve);
      double length = EdgeLength(E);
      double eltSize = length / nb_segm;
      GCPnts_UniformAbscissa Discret (C3d, eltSize, fi, li);
      if (!Discret.IsDone()) return false;
      int NbPoints = Discret.NbPoints();
      for (int i = 1; i <= NbPoints; i++) {
        double param = Discret.Parameter(i);
        params.push_back(param);
      }
    }
    catch (Standard_Failure) {
      return 0;
    }
  }
  else
  {
    // Edge is a degenerated Edge
    BRep_Tool::Range(E, fi, li);
    double du = (li - fi) / nb_segm;
    for (int i = 1; i <= nb_segm + 1; i++)
    {
      double param = fi + (i - 1) * du;
      params.push_back(param);
    }
  }

  double f, l;
  Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);
  ASSERT(f != l);

  bool isForward = (((l - f) * (last - first)) > 0);
  if (isForward) {
    list<double>::iterator itU = params.begin();
    for (int i = 0; i <= nb_segm; i++) // nbPoints internal
    {
      double param = *itU;
      gp_Pnt2d p = C2d->Value(param);
      uvslf[i].x = p.X();
      uvslf[i].y = p.Y();
      uvslf[i].param = param;
      uvslf[i].normParam = (param - f) / (l - f);
      itU++;
    }
  } else {
    list<double>::reverse_iterator itU = params.rbegin();
    for (int j = nb_segm; j >= 0; j--) // nbPoints internal
    {
      double param = *itU;
      int i = nb_segm - j;
      gp_Pnt2d p = C2d->Value(param);
      uvslf[i].x = p.X();
      uvslf[i].y = p.Y();
      uvslf[i].param = param;
      uvslf[i].normParam = (param - l) / (f - l);
      itU++;
    }
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
