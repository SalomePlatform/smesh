//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH : implementaion of SMESH idl descriptions
// File      : StdMeshers_QuadToTriaAdaptor.cxx
// Module    : SMESH
// Created   : Wen May 07 16:37:07 2008
// Author    : Sergey KUUL (skl)
//
#include "StdMeshers_QuadToTriaAdaptor.hxx"

#include <SMESH_Algo.hxx>
#include <SMESH_MesherHelper.hxx>

#include <IntAna_IntConicQuad.hxx>
#include <IntAna_Quadric.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_HArray1OfVec.hxx>
#include <TColgp_HSequenceOfPnt.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>

#include <numeric>

using namespace std;

enum EQuadNature { NOT_QUAD, QUAD, DEGEN_QUAD };

  // sdt-like iterator used to get coordinates of nodes of mesh element
typedef SMDS_StdIterator< SMESH_MeshEditor::TNodeXYZ, SMDS_ElemIteratorPtr > TXyzIterator;

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshers_QuadToTriaAdaptor::~StdMeshers_QuadToTriaAdaptor()
{
  // delete temporary faces
  TQuad2Trias::iterator f_f = myResMap.begin(), ffEnd = myResMap.end();
  for ( ; f_f != ffEnd; ++f_f )
  {
    TTriaList& fList = f_f->second;
    TTriaList::iterator f = fList.begin(), fEnd = fList.end();
    for ( ; f != fEnd; ++f )
      delete *f;
  }
  myResMap.clear();

//   TF2PyramMap::iterator itp = myPyram2Trias.begin();
//   for(; itp!=myPyram2Trias.end(); itp++)
//     cout << itp->second << endl;
}


//=======================================================================
//function : FindBestPoint
//purpose  : Return a point P laying on the line (PC,V) so that triangle
//           (P, P1, P2) to be equilateral as much as possible
//           V - normal to (P1,P2,PC)
//=======================================================================
static gp_Pnt FindBestPoint(const gp_Pnt& P1, const gp_Pnt& P2,
                            const gp_Pnt& PC, const gp_Vec& V)
{
  double a = P1.Distance(P2);
  double b = P1.Distance(PC);
  double c = P2.Distance(PC);
  if( a < (b+c)/2 )
    return PC;
  else {
    // find shift along V in order a to became equal to (b+c)/2
    double shift = sqrt( a*a + (b*b-c*c)*(b*b-c*c)/16/a/a - (b*b+c*c)/2 );
    gp_Dir aDir(V);
    gp_Pnt Pbest = PC.XYZ() + aDir.XYZ() * shift;
    return Pbest;
  }
}


//=======================================================================
//function : HasIntersection3
//purpose  : Auxilare for HasIntersection()
//           find intersection point between triangle (P1,P2,P3)
//           and segment [PC,P]
//=======================================================================
static bool HasIntersection3(const gp_Pnt& P, const gp_Pnt& PC, gp_Pnt& Pint,
                             const gp_Pnt& P1, const gp_Pnt& P2, const gp_Pnt& P3)
{
  //cout<<"HasIntersection3"<<endl;
  //cout<<"  PC("<<PC.X()<<","<<PC.Y()<<","<<PC.Z()<<")"<<endl;
  //cout<<"  P("<<P.X()<<","<<P.Y()<<","<<P.Z()<<")"<<endl;
  //cout<<"  P1("<<P1.X()<<","<<P1.Y()<<","<<P1.Z()<<")"<<endl;
  //cout<<"  P2("<<P2.X()<<","<<P2.Y()<<","<<P2.Z()<<")"<<endl;
  //cout<<"  P3("<<P3.X()<<","<<P3.Y()<<","<<P3.Z()<<")"<<endl;
  gp_Vec VP1(P1,P2);
  gp_Vec VP2(P1,P3);
  IntAna_Quadric IAQ(gp_Pln(P1,VP1.Crossed(VP2)));
  IntAna_IntConicQuad IAICQ(gp_Lin(PC,gp_Dir(gp_Vec(PC,P))),IAQ);
  if(IAICQ.IsDone()) {
    if( IAICQ.IsInQuadric() )
      return false;
    if( IAICQ.NbPoints() == 1 ) {
      gp_Pnt PIn = IAICQ.Point(1);
      double preci = 1.e-6;
      // check if this point is internal for segment [PC,P]
      bool IsExternal =
        ( (PC.X()-PIn.X())*(P.X()-PIn.X()) > preci ) ||
        ( (PC.Y()-PIn.Y())*(P.Y()-PIn.Y()) > preci ) ||
        ( (PC.Z()-PIn.Z())*(P.Z()-PIn.Z()) > preci );
      if(IsExternal) {
        return false;
      }
      // check if this point is internal for triangle (P1,P2,P3)
      gp_Vec V1(PIn,P1);
      gp_Vec V2(PIn,P2);
      gp_Vec V3(PIn,P3);
      if( V1.Magnitude()<preci || V2.Magnitude()<preci ||
          V3.Magnitude()<preci ) {
        Pint = PIn;
        return true;
      }
      gp_Vec VC1 = V1.Crossed(V2);
      gp_Vec VC2 = V2.Crossed(V3);
      gp_Vec VC3 = V3.Crossed(V1);
      if(VC1.Magnitude()<preci) {
        if(VC2.IsOpposite(VC3,preci)) {
          return false;
        }
      }
      else if(VC2.Magnitude()<preci) {
        if(VC1.IsOpposite(VC3,preci)) {
          return false;
        }
      }
      else if(VC3.Magnitude()<preci) {
        if(VC1.IsOpposite(VC2,preci)) {
          return false;
        }
      }
      else {
        if( VC1.IsOpposite(VC2,preci) || VC1.IsOpposite(VC3,preci) ||
            VC2.IsOpposite(VC3,preci) ) {
          return false;
        }
      }
      Pint = PIn;
      return true;
    }
  }

  return false;
}


//=======================================================================
//function : HasIntersection
//purpose  : Auxilare for CheckIntersection()
//=======================================================================

static bool HasIntersection(const gp_Pnt& P, const gp_Pnt& PC, gp_Pnt& Pint,
                            Handle(TColgp_HSequenceOfPnt)& aContour)
{
  if(aContour->Length()==3) {
    return HasIntersection3( P, PC, Pint, aContour->Value(1),
                             aContour->Value(2), aContour->Value(3) );
  }
  else {
    bool check = false;
    if( (aContour->Value(1).Distance(aContour->Value(2)) > 1.e-6) &&
        (aContour->Value(1).Distance(aContour->Value(3)) > 1.e-6) &&
        (aContour->Value(2).Distance(aContour->Value(3)) > 1.e-6) ) {
      check = HasIntersection3( P, PC, Pint, aContour->Value(1),
                                aContour->Value(2), aContour->Value(3) );
    }
    if(check) return true;
    if( (aContour->Value(1).Distance(aContour->Value(4)) > 1.e-6) &&
        (aContour->Value(1).Distance(aContour->Value(3)) > 1.e-6) &&
        (aContour->Value(4).Distance(aContour->Value(3)) > 1.e-6) ) {
      check = HasIntersection3( P, PC, Pint, aContour->Value(1),
                                aContour->Value(3), aContour->Value(4) );
    }
    if(check) return true;
  }

  return false;
}


//=======================================================================
//function : CheckIntersection
//purpose  : Auxilare for Compute()
//           NotCheckedFace - for optimization
//=======================================================================
bool StdMeshers_QuadToTriaAdaptor::CheckIntersection
                       (const gp_Pnt& P, const gp_Pnt& PC,
                        gp_Pnt& Pint, SMESH_Mesh& aMesh,
                        const TopoDS_Shape& aShape,
                        const TopoDS_Shape& NotCheckedFace)
{
  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  //cout<<"    CheckIntersection: meshDS->NbFaces() = "<<meshDS->NbFaces()<<endl;
  bool res = false;
  double dist = RealLast();
  gp_Pnt Pres;
  for (TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next()) {
    const TopoDS_Shape& aShapeFace = exp.Current();
    if(aShapeFace==NotCheckedFace)
      continue;
    const SMESHDS_SubMesh * aSubMeshDSFace = meshDS->MeshElements(aShapeFace);
    if ( aSubMeshDSFace ) {
      SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
      while ( iteratorElem->more() ) { // loop on elements on a face
        const SMDS_MeshElement* face = iteratorElem->next();
        Handle(TColgp_HSequenceOfPnt) aContour = new TColgp_HSequenceOfPnt;
        SMDS_ElemIteratorPtr nodeIt = face->nodesIterator();
        int nbN = face->NbNodes();
        if( face->IsQuadratic() )
          nbN /= 2;
        for ( int i = 0; i < nbN; ++i ) {
          const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nodeIt->next() );
          aContour->Append(gp_Pnt(node->X(), node->Y(), node->Z()));
        }
        if( HasIntersection(P, PC, Pres, aContour) ) {
          res = true;
          double tmp = PC.Distance(Pres);
          if(tmp<dist) {
            Pint = Pres;
            dist = tmp;
          }
        }
      }
    }
  }
  return res;
}


//=======================================================================
//function : EqualTriangles
//purpose  : Auxilare for Compute()
//=======================================================================
static bool EqualTriangles(const SMDS_MeshElement* F1,const SMDS_MeshElement* F2)
{
  return
    ( F1->GetNode(1)==F2->GetNode(2) && F1->GetNode(2)==F2->GetNode(1) ) ||
    ( F1->GetNode(1)==F2->GetNode(1) && F1->GetNode(2)==F2->GetNode(2) );
}

//================================================================================
/*!
 * \brief Prepare data for the given face
 *  \param PN - coordinates of face nodes
 *  \param VN - cross products of vectors (PC-PN(i)) ^ (PC-PN(i+1))
 *  \param FNodes - face nodes
 *  \param PC - gravity center of nodes
 *  \param VNorm - face normal (sum of VN)
 *  \param volumes - two volumes sharing the given face, the first is in VNorm direction
 *  \retval int - 0 if given face is not quad,
 *                1 if given face is quad,
 *                2 if given face is degenerate quad (two nodes are coincided)
 */
//================================================================================

int StdMeshers_QuadToTriaAdaptor::Preparation(const SMDS_MeshElement*       face,
                                              Handle(TColgp_HArray1OfPnt)&  PN,
                                              Handle(TColgp_HArray1OfVec)&  VN,
                                              vector<const SMDS_MeshNode*>& FNodes,
                                              gp_Pnt&                       PC,
                                              gp_Vec&                       VNorm,
                                              const SMDS_MeshElement**      volumes)
{
  if( face->NbNodes() != ( face->IsQuadratic() ? 8 : 4 ))
    if( face->NbNodes() != 4 )
      return NOT_QUAD;

  int i = 0;
  gp_XYZ xyzC(0., 0., 0.);
  for ( i = 0; i < 4; ++i )
  {
    gp_XYZ p = SMESH_MeshEditor::TNodeXYZ( FNodes[i] = face->GetNode(i) );
    PN->SetValue( i+1, p );
    xyzC += p;
  }
  PC = xyzC/4;
  //cout<<"  PC("<<PC.X()<<","<<PC.Y()<<","<<PC.Z()<<")"<<endl;

  int nbp = 4;

  int j = 0;
  for(i=1; i<4; i++) {
    j = i+1;
    for(; j<=4; j++) {
      if( PN->Value(i).Distance(PN->Value(j)) < 1.e-6 )
        break;
    }
    if(j<=4) break;
  }
  //int deg_num = IsDegenarate(PN);
  //if(deg_num>0) {
  bool hasdeg = false;
  if(i<4) {
    //cout<<"find degeneration"<<endl;
    hasdeg = true;
    gp_Pnt Pdeg = PN->Value(i);

    list< const SMDS_MeshNode* >::iterator itdg = myDegNodes.begin();
    const SMDS_MeshNode* DegNode = 0;
    for(; itdg!=myDegNodes.end(); itdg++) {
      const SMDS_MeshNode* N = (*itdg);
      gp_Pnt Ptmp(N->X(),N->Y(),N->Z());
      if(Pdeg.Distance(Ptmp)<1.e-6) {
        DegNode = N;
        //DegNode = const_cast<SMDS_MeshNode*>(N);
        break;
      }
    }
    if(!DegNode) {
      DegNode = FNodes[i-1];
      myDegNodes.push_back(DegNode);
    }
    else {
      FNodes[i-1] = DegNode;
    }
    for(i=j; i<4; i++) {
      PN->SetValue(i,PN->Value(i+1));
      FNodes[i-1] = FNodes[i];
    }
    nbp = 3;
  }

  PN->SetValue(nbp+1,PN->Value(1));
  FNodes[nbp] = FNodes[0];
  // find normal direction
  gp_Vec V1(PC,PN->Value(nbp));
  gp_Vec V2(PC,PN->Value(1));
  VNorm = V1.Crossed(V2);
  VN->SetValue(nbp,VNorm);
  for(i=1; i<nbp; i++) {
    V1 = gp_Vec(PC,PN->Value(i));
    V2 = gp_Vec(PC,PN->Value(i+1));
    gp_Vec Vtmp = V1.Crossed(V2);
    VN->SetValue(i,Vtmp);
    VNorm += Vtmp;
  }

  // find volumes sharing the face
  if ( volumes )
  {
    volumes[0] = volumes[1] = 0;
    SMDS_ElemIteratorPtr vIt = FNodes[0]->GetInverseElementIterator( SMDSAbs_Volume );
    while ( vIt->more() )
    {
      const SMDS_MeshElement* vol = vIt->next();
      bool volSharesAllNodes = true;
      for ( int i = 1; i < face->NbNodes() && volSharesAllNodes; ++i )
        volSharesAllNodes = ( vol->GetNodeIndex( FNodes[i] ) >= 0 );
      if ( volSharesAllNodes )
        volumes[ volumes[0] ? 1 : 0 ] = vol;
      // we could additionally check that vol has all FNodes in its one face using SMDS_VolumeTool
    }
    // define volume position relating to the face normal
    if ( volumes[0] )
    {
      // get volume gc
      SMDS_ElemIteratorPtr nodeIt = volumes[0]->nodesIterator();
      gp_XYZ volGC(0,0,0);
      volGC = accumulate( TXyzIterator(nodeIt), TXyzIterator(), volGC ) / volumes[0]->NbNodes();

      if ( VNorm * gp_Vec( PC, volGC ) < 0 )
        swap( volumes[0], volumes[1] );
    }
  }

  //cout<<"  VNorm("<<VNorm.X()<<","<<VNorm.Y()<<","<<VNorm.Z()<<")"<<endl;
  return hasdeg ? DEGEN_QUAD : QUAD;
}


//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

bool StdMeshers_QuadToTriaAdaptor::Compute(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape)
{
  myResMap.clear();
  myPyram2Trias.clear();

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  SMESH_MesherHelper helper(aMesh);
  helper.IsQuadraticSubMesh(aShape);
  helper.SetElementsOnShape( true );

  for (TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next()) {
    const TopoDS_Shape& aShapeFace = exp.Current();
    const SMESHDS_SubMesh * aSubMeshDSFace = meshDS->MeshElements( aShapeFace );
    if ( aSubMeshDSFace ) {
      bool isRev = SMESH_Algo::IsReversedSubMesh( TopoDS::Face(aShapeFace), meshDS );

      SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
      while ( iteratorElem->more() ) { // loop on elements on a face
        const SMDS_MeshElement* face = iteratorElem->next();
        //cout<<endl<<"================= face->GetID() = "<<face->GetID()<<endl;
        // preparation step using face info
        Handle(TColgp_HArray1OfPnt) PN = new TColgp_HArray1OfPnt(1,5);
        Handle(TColgp_HArray1OfVec) VN = new TColgp_HArray1OfVec(1,4);
        vector<const SMDS_MeshNode*> FNodes(5);
        gp_Pnt PC;
        gp_Vec VNorm;
        int stat =  Preparation(face, PN, VN, FNodes, PC, VNorm);
        if(stat==0)
          continue;

        if(stat==2) {
          // degenerate face
          // add triangles to result map
          SMDS_FaceOfNodes* NewFace;
          if(!isRev)
            NewFace = new SMDS_FaceOfNodes( FNodes[0], FNodes[1], FNodes[2] );
          else
            NewFace = new SMDS_FaceOfNodes( FNodes[0], FNodes[2], FNodes[1] );
          TTriaList aList( 1, NewFace );
          myResMap.insert(make_pair(face,aList));
          continue;
        }

        if(!isRev) VNorm.Reverse();
        double xc = 0., yc = 0., zc = 0.;
        int i = 1;
        for(; i<=4; i++) {
          gp_Pnt Pbest;
          if(!isRev)
            Pbest = FindBestPoint(PN->Value(i), PN->Value(i+1), PC, VN->Value(i).Reversed());
          else
            Pbest = FindBestPoint(PN->Value(i), PN->Value(i+1), PC, VN->Value(i));
          xc += Pbest.X();
          yc += Pbest.Y();
          zc += Pbest.Z();
        }
        gp_Pnt PCbest(xc/4., yc/4., zc/4.);

        // check PCbest
        double height = PCbest.Distance(PC);
        if(height<1.e-6) {
          // create new PCbest using a bit shift along VNorm
          PCbest = PC.XYZ() + VNorm.XYZ() * 0.001;
        }
        else {
          // check possible intersection with other faces
          gp_Pnt Pint;
          bool check = CheckIntersection(PCbest, PC, Pint, aMesh, aShape, aShapeFace);
          if(check) {
            //cout<<"--PC("<<PC.X()<<","<<PC.Y()<<","<<PC.Z()<<")"<<endl;
            //cout<<"  PCbest("<<PCbest.X()<<","<<PCbest.Y()<<","<<PCbest.Z()<<")"<<endl;
            double dist = PC.Distance(Pint)/3.;
            gp_Dir aDir(gp_Vec(PC,PCbest));
            PCbest = PC.XYZ() + aDir.XYZ() * dist;
          }
          else {
            gp_Vec VB(PC,PCbest);
            gp_Pnt PCbestTmp = PC.XYZ() + VB.XYZ() * 3.0;
            bool check = CheckIntersection(PCbestTmp, PC, Pint, aMesh, aShape, aShapeFace);
            if(check) {
              double dist = PC.Distance(Pint)/3.;
              if(dist<height) {
                gp_Dir aDir(gp_Vec(PC,PCbest));
                PCbest = PC.XYZ() + aDir.XYZ() * dist;
              }
            }
          }
        }
        // create node for PCbest
        SMDS_MeshNode* NewNode = helper.AddNode( PCbest.X(), PCbest.Y(), PCbest.Z() );

        // add triangles to result map
        TTriaList& triaList = myResMap.insert( make_pair( face, TTriaList() ))->second;
        for(i=0; i<4; i++)
          triaList.push_back( new SMDS_FaceOfNodes( NewNode, FNodes[i], FNodes[i+1] ));

        // create pyramid
        if ( isRev ) swap( FNodes[1], FNodes[3]);
        SMDS_MeshVolume* aPyram =
          helper.AddVolume( FNodes[0], FNodes[1], FNodes[2], FNodes[3], NewNode );
        myPyram2Trias.insert(make_pair(aPyram, & triaList));
      } // end loop on elements on a face submesh
    }
  } // end for(TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next()) {

  return Compute2ndPart(aMesh);
}


//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

bool StdMeshers_QuadToTriaAdaptor::Compute(SMESH_Mesh& aMesh)
{
  myResMap.clear();
  myPyram2Trias.clear();
  SMESH_MesherHelper helper(aMesh);
  helper.IsQuadraticSubMesh(aMesh.GetShapeToMesh());
  helper.SetElementsOnShape( true );

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();

  SMDS_FaceIteratorPtr fIt = meshDS->facesIterator();
  TIDSortedElemSet sortedFaces; //  0020279: control the "random" use when using mesh algorithms
  while( fIt->more()) sortedFaces.insert( fIt->next() );

  TIDSortedElemSet::iterator itFace = sortedFaces.begin(), fEnd = sortedFaces.end();
  for ( ; itFace != fEnd; ++itFace )
  {
    const SMDS_MeshElement* face = *itFace;
    if ( !face ) continue;
    //cout<<endl<<"================= face->GetID() = "<<face->GetID()<<endl;
    // retrieve needed information about a face
    Handle(TColgp_HArray1OfPnt) PN = new TColgp_HArray1OfPnt(1,5);
    Handle(TColgp_HArray1OfVec) VN = new TColgp_HArray1OfVec(1,4);
    vector<const SMDS_MeshNode*> FNodes(5);
    gp_Pnt PC;
    gp_Vec VNorm;
    const SMDS_MeshElement* volumes[2];
    int what = Preparation(face, PN, VN, FNodes, PC, VNorm, volumes);
    if ( what == NOT_QUAD )
      continue;
    if ( volumes[0] && volumes[1] )
      continue; // face is shared by two volumes - no space for a pyramid

    if ( what == DEGEN_QUAD )
    {
      // degenerate face
      // add triangles to result map
      TTriaList aList;
      SMDS_FaceOfNodes* NewFace;
      // check orientation

      double tmp = PN->Value(1).Distance(PN->Value(2)) + PN->Value(2).Distance(PN->Value(3));
      // far points in VNorm direction
      gp_Pnt Ptmp1 = PC.XYZ() + VNorm.XYZ() * tmp * 1.e6;
      gp_Pnt Ptmp2 = PC.XYZ() - VNorm.XYZ() * tmp * 1.e6;
      // check intersection for Ptmp1 and Ptmp2
      bool IsRev = false;
      bool IsOK1 = false;
      bool IsOK2 = false;
      double dist1 = RealLast();
      double dist2 = RealLast();
      gp_Pnt Pres1,Pres2;
      for (TIDSortedElemSet::iterator itF = sortedFaces.begin(); itF != fEnd; ++itF ) {
        const SMDS_MeshElement* F = *itF;
        if(F==face) continue;
        Handle(TColgp_HSequenceOfPnt) aContour = new TColgp_HSequenceOfPnt;
        for ( int i = 0; i < 4; ++i )
          aContour->Append( SMESH_MeshEditor::TNodeXYZ( F->GetNode(i) ));
        gp_Pnt PPP;
        if( !volumes[0] && HasIntersection(Ptmp1, PC, PPP, aContour) ) {
          IsOK1 = true;
          double tmp = PC.Distance(PPP);
          if(tmp<dist1) {
            Pres1 = PPP;
            dist1 = tmp;
          }
        }
        if( !volumes[1] && HasIntersection(Ptmp2, PC, PPP, aContour) ) {
          IsOK2 = true;
          double tmp = PC.Distance(PPP);
          if(tmp<dist2) {
            Pres2 = PPP;
            dist2 = tmp;
          }
        }
      }

      if( IsOK1 && !IsOK2 ) {
        // using existed direction
      }
      else if( !IsOK1 && IsOK2 ) {
        // using opposite direction
        IsRev = true;
      }
      else { // IsOK1 && IsOK2
        double tmp1 = PC.Distance(Pres1);
        double tmp2 = PC.Distance(Pres2);
        if(tmp1<tmp2) {
          // using existed direction
        }
        else {
          // using opposite direction
          IsRev = true;
        }
      }
      if(!IsRev)
        NewFace = new SMDS_FaceOfNodes( FNodes[0], FNodes[1], FNodes[2] );
      else
        NewFace = new SMDS_FaceOfNodes( FNodes[0], FNodes[2], FNodes[1] );
      aList.push_back(NewFace);
      myResMap.insert(make_pair(face,aList));
      continue;
    }

    // Find pyramid peak

    gp_XYZ PCbest(0., 0., 0.); // pyramid peak
    int i = 1;
    for(; i<=4; i++) {
      gp_Pnt Pbest = FindBestPoint(PN->Value(i), PN->Value(i+1), PC, VN->Value(i));
      PCbest += Pbest.XYZ();
    }
    PCbest /= 4;

    double height = PC.Distance(PCbest); // pyramid height to precise
    if(height<1.e-6) {
      // create new PCbest using a bit shift along VNorm
      PCbest = PC.XYZ() + VNorm.XYZ() * 0.001;
      height = PC.Distance(PCbest);
    }
    //cout<<"  PCbest("<<PCbest.X()<<","<<PCbest.Y()<<","<<PCbest.Z()<<")"<<endl;

    // Restrict pyramid height by intersection with other faces
    gp_Vec tmpDir(PC,PCbest); tmpDir.Normalize();
    double tmp = PN->Value(1).Distance(PN->Value(3)) + PN->Value(2).Distance(PN->Value(4));
    // far points: in (PC, PCbest) direction and vice-versa
    gp_Pnt farPnt[2] = { PC.XYZ() + tmpDir.XYZ() * tmp * 1.e6,
                         PC.XYZ() - tmpDir.XYZ() * tmp * 1.e6 };
    // check intersection for farPnt1 and farPnt2
    bool   intersected[2] = { false, false };
    double dist       [2] = { RealLast(), RealLast() };
    gp_Pnt intPnt[2];
    for (TIDSortedElemSet::iterator itF = sortedFaces.begin(); itF != fEnd; ++itF )
    {
      const SMDS_MeshElement* F = *itF;
      if(F==face) continue;
      Handle(TColgp_HSequenceOfPnt) aContour = new TColgp_HSequenceOfPnt;
      int nbN = F->NbNodes() / ( F->IsQuadratic() ? 2 : 1 );
      for ( i = 0; i < nbN; ++i )
        aContour->Append( SMESH_MeshEditor::TNodeXYZ( F->GetNode(i) ));
      gp_Pnt intP;
      for ( int isRev = 0; isRev < 2; ++isRev )
      {
        if( !volumes[isRev] && HasIntersection(farPnt[isRev], PC, intP, aContour) ) {
          intersected[isRev] = true;
          double d = PC.Distance( intP );
          if( d < dist[isRev] )
          {
            intPnt[isRev] = intP;
            dist  [isRev] = d;
          }
        }
      }
    }

    // Create one or two pyramids

    for ( int isRev = 0; isRev < 2; ++isRev )
    {
      if( !intersected[isRev] ) continue;
      double pyramidH = Min( height, PC.Distance(intPnt[isRev])/3.);
      PCbest = PC.XYZ() + tmpDir.XYZ() * (isRev ? -pyramidH : pyramidH);

      // create node for PCbest
      SMDS_MeshNode* NewNode = helper.AddNode( PCbest.X(), PCbest.Y(), PCbest.Z() );

      // add triangles to result map
      TTriaList& aList = myResMap.insert( make_pair( face, TTriaList()))->second;
      for(i=0; i<4; i++) {
        SMDS_FaceOfNodes* NewFace;
        if(isRev)
          NewFace = new SMDS_FaceOfNodes( NewNode, FNodes[i], FNodes[i+1] );
        else
          NewFace = new SMDS_FaceOfNodes( NewNode, FNodes[i+1], FNodes[i] );
        aList.push_back(NewFace);
      }
      // create a pyramid
      SMDS_MeshVolume* aPyram;
      if(isRev)
        aPyram = helper.AddVolume( FNodes[0], FNodes[1], FNodes[2], FNodes[3], NewNode );
      else
        aPyram = helper.AddVolume( FNodes[0], FNodes[3], FNodes[2], FNodes[1], NewNode );
      myPyram2Trias.insert(make_pair(aPyram, & aList));
    }
  } // end loop on all faces

  return Compute2ndPart(aMesh);
}

//=======================================================================
//function : Compute2ndPart
//purpose  : Update created pyramids and faces to avoid their intersection
//=======================================================================

bool StdMeshers_QuadToTriaAdaptor::Compute2ndPart(SMESH_Mesh& aMesh)
{
  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();

  // check intersections between created pyramids

  if(myPyram2Trias.empty())
    return true;

  int k = 0;

  // for each pyramid store list of merged pyramids with their faces
  typedef map< const SMDS_MeshElement*, list< TPyram2Trias::iterator > > TPyram2Merged;
  TPyram2Merged MergesInfo;

  // iterate on all pyramids
  TPyram2Trias::iterator itPi = myPyram2Trias.begin(), itPEnd = myPyram2Trias.end();
  for ( ; itPi != itPEnd; ++itPi )
  {
    const SMDS_MeshElement* PrmI = itPi->first;
    TPyram2Merged::iterator pMergesI = MergesInfo.find( PrmI );

    TXyzIterator xyzIt( PrmI->nodesIterator() );
    vector<gp_Pnt> PsI( xyzIt, TXyzIterator() );

    // compare PrmI with all the rest pyramids
    bool NeedMove = false;
    TPyram2Trias::iterator itPj = itPi;
    for ( ++itPj; itPj != itPEnd; ++itPj )
    {
      const SMDS_MeshElement* PrmJ = itPj->first;
      TPyram2Merged::iterator pMergesJ = MergesInfo.find( PrmJ );

      // check if two pyramids already merged
      if ( pMergesJ != MergesInfo.end() &&
           find(pMergesJ->second.begin(),pMergesJ->second.end(), itPi )!=pMergesJ->second.end())
        continue; // already merged

      xyzIt = TXyzIterator( PrmJ->nodesIterator() );
      vector<gp_Pnt> PsJ( xyzIt, TXyzIterator() );

      bool hasInt = false;
      gp_Pnt Pint;
      for(k=0; k<4 && !hasInt; k++) {
        gp_Vec Vtmp(PsI[k],PsI[4]);
        gp_Pnt Pshift = PsI[k].XYZ() + Vtmp.XYZ() * 0.01;
        hasInt = 
          ( HasIntersection3( Pshift, PsI[4], Pint, PsJ[0], PsJ[1], PsJ[4]) ||
            HasIntersection3( Pshift, PsI[4], Pint, PsJ[1], PsJ[2], PsJ[4]) ||
            HasIntersection3( Pshift, PsI[4], Pint, PsJ[2], PsJ[3], PsJ[4]) ||
            HasIntersection3( Pshift, PsI[4], Pint, PsJ[3], PsJ[0], PsJ[4]) );
      }
      for(k=0; k<4 && !hasInt; k++) {
        gp_Vec Vtmp(PsJ[k],PsJ[4]);
        gp_Pnt Pshift = PsJ[k].XYZ() + Vtmp.XYZ() * 0.01;
        hasInt = 
          ( HasIntersection3( Pshift, PsJ[4], Pint, PsI[0], PsI[1], PsI[4]) ||
            HasIntersection3( Pshift, PsJ[4], Pint, PsI[1], PsI[2], PsI[4]) ||
            HasIntersection3( Pshift, PsJ[4], Pint, PsI[2], PsI[3], PsI[4]) ||
            HasIntersection3( Pshift, PsJ[4], Pint, PsI[3], PsI[0], PsI[4]) );
      }
      if(hasInt) {
        // count common nodes of base faces of two pyramids
        int nbc = 0;
        for(k=0; k<4; k++)
          nbc += int ( PrmI->GetNodeIndex( PrmJ->GetNode(k) ) >= 0 );
        //cout<<"      nbc = "<<nbc<<endl;

        if ( nbc == 4 )
          continue; // pyrams have a common base face

        if(nbc>0)
        {
          // Merge the two pyramids and others already merged with them

          // initialize merge info of pyramids
          if ( pMergesI == MergesInfo.end() ) // first merge of PrmI
          {
            pMergesI = MergesInfo.insert( make_pair( PrmI, list<TPyram2Trias::iterator >())).first;
            pMergesI->second.push_back( itPi );
          }
          if ( pMergesJ == MergesInfo.end() ) // first merge of PrmJ
          {
            pMergesJ = MergesInfo.insert( make_pair( PrmJ, list<TPyram2Trias::iterator >())).first;
            pMergesJ->second.push_back( itPj );
          }
          int nbI = pMergesI->second.size(), nbJ = pMergesJ->second.size();

          // an apex node to make common to all merged pyramids
          SMDS_MeshNode* CommonNode = const_cast<SMDS_MeshNode*>(PrmI->GetNode(4));
          CommonNode->setXYZ( ( nbI*PsI[4].X() + nbJ*PsJ[4].X() ) / (nbI+nbJ),
                              ( nbI*PsI[4].Y() + nbJ*PsJ[4].Y() ) / (nbI+nbJ),
                              ( nbI*PsI[4].Z() + nbJ*PsJ[4].Z() ) / (nbI+nbJ) );
          NeedMove = true;
          const SMDS_MeshNode* Nrem = PrmJ->GetNode(4); // node to remove

          list< TPyram2Trias::iterator >& aMergesI = pMergesI->second;
          list< TPyram2Trias::iterator >& aMergesJ = pMergesJ->second;

            // find and remove coincided faces of merged pyramids
          list< TPyram2Trias::iterator >::iterator itPttI, itPttJ;
          TTriaList::iterator trI, trJ;
          for ( itPttI = aMergesI.begin(); itPttI != aMergesI.end(); ++itPttI )
          {
            TTriaList* triaListI = (*itPttI)->second;
            for ( trI = triaListI->begin(); trI != triaListI->end(); )
            {
              const SMDS_FaceOfNodes* FI = *trI;

              for ( itPttJ = aMergesJ.begin(); itPttJ != aMergesJ.end() && FI; ++itPttJ )
              {
                TTriaList* triaListJ = (*itPttJ)->second;
                for ( trJ = triaListJ->begin(); trJ != triaListJ->end();  )
                {
                  const SMDS_FaceOfNodes* FJ = *trJ;

                  if( EqualTriangles(FI,FJ) )
                  {
                    delete FI;
                    delete FJ;
                    FI = FJ = 0;
                    trI = triaListI->erase( trI );
                    trJ = triaListJ->erase( trJ ); 
                    break; // only one triangle of a pyramid can coincide with another pyramid
                  }
                  ++trJ;
                }
              }
              if ( FI ) ++trI; // increament if triangle not deleted
            }
          }

          // set the common apex node to pyramids and triangles merged with J
          for ( itPttJ = aMergesJ.begin(); itPttJ != aMergesJ.end(); ++itPttJ )
          {
            const SMDS_MeshElement* Prm = (*itPttJ)->first;
            TTriaList*         triaList = (*itPttJ)->second;

            vector< const SMDS_MeshNode* > nodes( Prm->begin_nodes(), Prm->end_nodes() );
            nodes[4] = CommonNode;
            meshDS->ChangeElementNodes( Prm, &nodes[0], nodes.size());

            for ( TTriaList::iterator trIt = triaList->begin(); trIt != triaList->end(); ++trIt )
            {
              SMDS_FaceOfNodes* Ftria = const_cast< SMDS_FaceOfNodes*>( *trIt );
              const SMDS_MeshNode* NF[3] = { CommonNode, Ftria->GetNode(1), Ftria->GetNode(2)};
              Ftria->ChangeNodes(NF, 3);
            }
          }

          // join MergesInfo of merged pyramids
          for ( k = 0, itPttI = aMergesI.begin(); k < nbI; ++itPttI, ++k )
          {
            const SMDS_MeshElement* PrmI = (*itPttI)->first;
            list< TPyram2Trias::iterator >& merges = MergesInfo[ PrmI ];
            merges.insert( merges.end(), aMergesJ.begin(), aMergesJ.end() );
          }
          for ( k = 0, itPttJ = aMergesJ.begin(); k < nbJ; ++itPttJ, ++k )
          {
            const SMDS_MeshElement* PrmJ = (*itPttJ)->first;
            list< TPyram2Trias::iterator >& merges = MergesInfo[ PrmJ ];
            merges.insert( merges.end(), aMergesI.begin(), aMergesI.end() );
          }

          // removing node
          meshDS->RemoveNode(Nrem);
        }
        else { // nbc==0

          // decrease height of pyramids
          gp_XYZ PC1(0,0,0), PC2(0,0,0);
          for(k=0; k<4; k++) {
            PC1 += PsI[k].XYZ();
            PC2 += PsJ[k].XYZ();
          }
          PC1 /= 4; PC2 /= 4; 
          gp_Vec VN1(PC1,PsI[4]);
          gp_Vec VI1(PC1,Pint);
          gp_Vec VN2(PC2,PsJ[4]);
          gp_Vec VI2(PC2,Pint);
          double ang1 = fabs(VN1.Angle(VI1));
          double ang2 = fabs(VN2.Angle(VI2));
          double h1,h2;
          if(ang1>PI/3.)
            h1 = VI1.Magnitude()/2;
          else
            h1 = VI1.Magnitude()*cos(ang1);
          if(ang2>PI/3.)
            h2 = VI2.Magnitude()/2;
          else
            h2 = VI2.Magnitude()*cos(ang2);
          double coef1 = 0.5;
          if(ang1<PI/3)
            coef1 -= cos(ang1)*0.25;
          double coef2 = 0.5;
          if(ang2<PI/3)
            coef2 -= cos(ang1)*0.25;

          VN1.Scale(coef1);
          VN2.Scale(coef2);
          SMDS_MeshNode* aNode1 = const_cast<SMDS_MeshNode*>(PrmI->GetNode(4));
          aNode1->setXYZ( PC1.X()+VN1.X(), PC1.Y()+VN1.Y(), PC1.Z()+VN1.Z() );
          SMDS_MeshNode* aNode2 = const_cast<SMDS_MeshNode*>(PrmJ->GetNode(4));
          aNode2->setXYZ( PC2.X()+VN2.X(), PC2.Y()+VN2.Y(), PC2.Z()+VN2.Z() );
          NeedMove = true;
        }
      } // end if(hasInt)
    }
    if( NeedMove && !meshDS->IsEmbeddedMode() )
    {
      const SMDS_MeshNode* apex = PrmI->GetNode( 4 );
      meshDS->MoveNode( apex, apex->X(), apex->Y(), apex->Z() );
    }
  }

  // rebind triangles of pyramids sharing the same base quadrangle to the first
  // entrance of the base quadrangle
  TQuad2Trias::iterator q2t = myResMap.begin(), q2tPrev = q2t;
  for ( ++q2t; q2t != myResMap.end(); ++q2t, ++q2tPrev )
  {
    if ( q2t->first == q2tPrev->first )
      q2tPrev->second.splice( q2tPrev->second.end(), q2t->second );
  }

  myPyram2Trias.clear(); // no more needed
  myDegNodes.clear();

  return true;
}

//================================================================================
/*!
 * \brief Return list of created triangles for given face
 */
//================================================================================

const list<const SMDS_FaceOfNodes* >* StdMeshers_QuadToTriaAdaptor::GetTriangles (const SMDS_MeshElement* aQuad)
{
  TQuad2Trias::iterator it = myResMap.find(aQuad);
  if( it != myResMap.end() ) {
    return & it->second;
  }
  return 0;
}
