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

#include "SMDS_SetIterator.hxx"

#include "SMESH_Algo.hxx"
#include "SMESH_MesherHelper.hxx"

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

// std-like iterator used to get coordinates of nodes of mesh element
typedef SMDS_StdIterator< SMESH_MeshEditor::TNodeXYZ, SMDS_ElemIteratorPtr > TXyzIterator;

namespace
{
  const int Q2TAbs_TmpTriangle = SMDSEntity_Last + 1;

  //================================================================================
  /*!
   * \brief Temporary face. It's key feature is that it adds itself to an apex node
   * as inverse element, so that tmp triangles of a piramid can be easily found
   */
  //================================================================================

  class STDMESHERS_EXPORT Q2TAdaptor_Triangle : public SMDS_MeshFace
  {
    const SMDS_MeshNode* _nodes[3];
  public:
    Q2TAdaptor_Triangle(const SMDS_MeshNode* apexNode,
                        const SMDS_MeshNode* node2,
                        const SMDS_MeshNode* node3)
    {
      _nodes[0]=0; ChangeApex(apexNode);
      _nodes[1]=node2;
      _nodes[2]=node3;
    }
    ~Q2TAdaptor_Triangle() { MarkAsRemoved(); }
    void ChangeApex(const SMDS_MeshNode* node)
    {
      MarkAsRemoved();
      _nodes[0]=node;
      const_cast<SMDS_MeshNode*>(node)->AddInverseElement(this);
    }
    void MarkAsRemoved()
    {
      if ( _nodes[0] )
        const_cast<SMDS_MeshNode*>(_nodes[0])->RemoveInverseElement(this), _nodes[0] = 0;
    }
    bool IsRemoved() const { return !_nodes[0]; }
    virtual int NbNodes() const { return 3; }
    virtual const SMDS_MeshNode* GetNode(const int ind) const { return _nodes[ind]; }
    virtual SMDSAbs_EntityType   GetEntityType() const
    {
      return SMDSAbs_EntityType( Q2TAbs_TmpTriangle );
    }
    virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type) const
    {
      if ( type == SMDSAbs_Node )
        return SMDS_ElemIteratorPtr( new SMDS_NodeArrayElemIterator( _nodes, _nodes+3 ));
      throw SALOME_Exception(LOCALIZED("Not implemented"));
    }
  };

  //================================================================================
  /*!
   * \brief Return true if two nodes of triangles are equal
   */
  //================================================================================

  bool EqualTriangles(const SMDS_MeshElement* F1,const SMDS_MeshElement* F2)
  {
    return
      ( F1->GetNode(1)==F2->GetNode(2) && F1->GetNode(2)==F2->GetNode(1) ) ||
      ( F1->GetNode(1)==F2->GetNode(1) && F1->GetNode(2)==F2->GetNode(2) );
  }

  //================================================================================
  /*!
   * \brief Merge the two pyramids (i.e. fuse their apex) and others already merged with them
   */
  //================================================================================

  void MergePiramids( const SMDS_MeshElement*     PrmI,
                      const SMDS_MeshElement*     PrmJ,
                      SMESHDS_Mesh*               meshDS,
                      set<const SMDS_MeshNode*> & nodesToMove)
  {
    const SMDS_MeshNode* Nrem = PrmJ->GetNode(4); // node to remove
    int nbJ = Nrem->NbInverseElements( SMDSAbs_Volume );
    SMESH_MeshEditor::TNodeXYZ Pj( Nrem );

    // an apex node to make common to all merged pyramids
    SMDS_MeshNode* CommonNode = const_cast<SMDS_MeshNode*>(PrmI->GetNode(4));
    if ( CommonNode == Nrem ) return; // already merged
    int nbI = CommonNode->NbInverseElements( SMDSAbs_Volume );
    SMESH_MeshEditor::TNodeXYZ Pi( CommonNode );
    gp_XYZ Pnew = ( nbI*Pi + nbJ*Pj ) / (nbI+nbJ);
    CommonNode->setXYZ( Pnew.X(), Pnew.Y(), Pnew.Z() );

    nodesToMove.insert( CommonNode );
    nodesToMove.erase ( Nrem );

    // find and remove coincided faces of merged pyramids
    SMDS_ElemIteratorPtr triItI = CommonNode->GetInverseElementIterator(SMDSAbs_Face);
    while ( triItI->more() )
    {
      const SMDS_MeshElement* FI = triItI->next();
      const SMDS_MeshElement* FJEqual = 0;
      SMDS_ElemIteratorPtr triItJ = Nrem->GetInverseElementIterator(SMDSAbs_Face);
      while ( !FJEqual && triItJ->more() )
      {
        const SMDS_MeshElement* FJ = triItJ->next();
        if ( EqualTriangles( FJ, FI ))
          FJEqual = FJ;
      }
      if ( FJEqual )
      {
        ((Q2TAdaptor_Triangle*) FI)->MarkAsRemoved();
        ((Q2TAdaptor_Triangle*) FJEqual)->MarkAsRemoved();
      }
    }

    // set the common apex node to pyramids and triangles merged with J
    SMDS_ElemIteratorPtr itJ = Nrem->GetInverseElementIterator();
    while ( itJ->more() )
    {
      const SMDS_MeshElement* elem = itJ->next();
      if ( elem->GetType() == SMDSAbs_Volume ) // pyramid
      {
        vector< const SMDS_MeshNode* > nodes( elem->begin_nodes(), elem->end_nodes() );
        nodes[4] = CommonNode;
        meshDS->ChangeElementNodes( elem, &nodes[0], nodes.size());
      }
      else if ( elem->GetEntityType() == Q2TAbs_TmpTriangle ) // tmp triangle
      {
        ((Q2TAdaptor_Triangle*) elem )->ChangeApex( CommonNode );
      }
    }
    ASSERT( Nrem->NbInverseElements() == 0 );
    meshDS->RemoveFreeNode( Nrem,
                            meshDS->MeshElements( Nrem->GetPosition()->GetShapeId()),
                            /*fromGroups=*/false);
  }

  //================================================================================
  /*!
   * \brief Return true if two adjacent pyramids are too close one to another
   * so that a tetrahedron to built between them would have too poor quality
   */
  //================================================================================

  bool TooCloseAdjacent( const SMDS_MeshElement* PrmI,
                         const SMDS_MeshElement* PrmJ,
                         const bool              hasShape)
  {
    const SMDS_MeshNode* nApexI = PrmI->GetNode(4);
    const SMDS_MeshNode* nApexJ = PrmJ->GetNode(4);
    if ( nApexI == nApexJ ||
         nApexI->GetPosition()->GetShapeId() != nApexJ->GetPosition()->GetShapeId() )
      return false;

    // Find two common base nodes and their indices within PrmI and PrmJ
    const SMDS_MeshNode* baseNodes[2] = { 0,0 };
    int baseNodesIndI[2], baseNodesIndJ[2];
    for ( int i = 0; i < 4 ; ++i )
    {
      int j = PrmJ->GetNodeIndex( PrmI->GetNode(i));
      if ( j >= 0 )
      {
        int ind = baseNodes[0] ? 1:0;
        if ( baseNodes[ ind ])
          return false; // pyramids with a common base face
        baseNodes   [ ind ] = PrmI->GetNode(i);
        baseNodesIndI[ ind ] = i;
        baseNodesIndJ[ ind ] = j;
      }
    }
    if ( !baseNodes[1] ) return false; // not adjacent

    // Get normals of triangles sharing baseNodes
    gp_XYZ apexI = SMESH_MeshEditor::TNodeXYZ( nApexI );
    gp_XYZ apexJ = SMESH_MeshEditor::TNodeXYZ( nApexJ );
    gp_XYZ base1 = SMESH_MeshEditor::TNodeXYZ( baseNodes[0]);
    gp_XYZ base2 = SMESH_MeshEditor::TNodeXYZ( baseNodes[1]);
    gp_Vec baseVec( base1, base2 );
    gp_Vec baI( base1, apexI );
    gp_Vec baJ( base1, apexJ );
    gp_Vec nI = baseVec.Crossed( baI );
    gp_Vec nJ = baseVec.Crossed( baJ );

    // Check angle between normals
    double angle = nI.Angle( nJ );
    bool tooClose = ( angle < 15 * PI180 );

    // Check if pyramids collide
    bool isOutI, isOutJ;
    if ( !tooClose && baI * baJ > 0 )
    {
      // find out if nI points outside of PrmI or inside
      int dInd = baseNodesIndI[1] - baseNodesIndI[0];
      isOutI = ( abs(dInd)==1 ) ? dInd < 0 : dInd > 0;

      // find out sign of projection of nJ to baI
      double proj = baI * nJ;

      tooClose = isOutI ? proj > 0 : proj < 0;
    }

    // Check if PrmI and PrmJ are in same domain
    if ( tooClose && !hasShape )
    {
      // check order of baseNodes within pyramids, it must be opposite
      int dInd = baseNodesIndJ[1] - baseNodesIndJ[0];
      isOutJ = ( abs(dInd)==1 ) ? dInd < 0 : dInd > 0;
      if ( isOutJ == isOutI )
        return false; // other domain

      // check absence of a face separating domains between pyramids
      TIDSortedElemSet emptySet, avoidSet;
      int i1, i2;
      while ( const SMDS_MeshElement* f =
              SMESH_MeshEditor::FindFaceInSet( baseNodes[0], baseNodes[1],
                                               emptySet, avoidSet, &i1, &i2 ))
      {
        avoidSet.insert( f );

        // face node other than baseNodes
        int otherNodeInd = 0;
        while ( otherNodeInd == i1 || otherNodeInd == i2 ) otherNodeInd++;
        const SMDS_MeshNode* otherFaceNode = f->GetNode( otherNodeInd );

        // check if f is a base face of either of pyramids
        if ( f->NbCornerNodes() == 4 &&
             ( PrmI->GetNodeIndex( otherFaceNode ) >= 0 ||
               PrmJ->GetNodeIndex( otherFaceNode ) >= 0 ))
          continue; // f is a base quadrangle

        // check projections of face direction (baOFN) to triange normals (nI and nJ)
        gp_Vec baOFN( base1, SMESH_MeshEditor::TNodeXYZ( otherFaceNode ));
        ( isOutI ? nJ : nI ).Reverse();
        if ( nI * baOFN > 0 && nJ * baOFN > 0 )
        {
          tooClose = false; // f is between pyramids
          break;
        }
      }
    }

    return tooClose;
  }

  //================================================================================
  /*!
   * \brief Merges adjacent pyramids
   */
  //================================================================================

  void MergeAdjacent(const SMDS_MeshElement*    PrmI,
                     SMESH_Mesh&                mesh,
                     set<const SMDS_MeshNode*>& nodesToMove)
  {
    TIDSortedElemSet adjacentPyrams, mergedPyrams;
    for(int k=0; k<4; k++) // loop on 4 base nodes of PrmI
    {
      const SMDS_MeshNode* n = PrmI->GetNode(k);
      SMDS_ElemIteratorPtr vIt = n->GetInverseElementIterator( SMDSAbs_Volume );
      while ( vIt->more() )
      {
        const SMDS_MeshElement* PrmJ = vIt->next();
        if ( PrmJ->NbCornerNodes() != 5 || !adjacentPyrams.insert( PrmJ ).second  )
          continue;
        if ( PrmI != PrmJ && TooCloseAdjacent( PrmI, PrmJ, mesh.HasShapeToMesh() ))
        {
          MergePiramids( PrmI, PrmJ, mesh.GetMeshDS(), nodesToMove );
          mergedPyrams.insert( PrmJ );
        }
      }
    }
    if ( !mergedPyrams.empty() )
    {
      TIDSortedElemSet::iterator prm;
//       for (prm = mergedPyrams.begin(); prm != mergedPyrams.end(); ++prm)
//         MergeAdjacent( *prm, mesh, nodesToMove );

      for (prm = adjacentPyrams.begin(); prm != adjacentPyrams.end(); ++prm)
        MergeAdjacent( *prm, mesh, nodesToMove );
    }
  }
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

StdMeshers_QuadToTriaAdaptor::StdMeshers_QuadToTriaAdaptor():
  myElemSearcher(0), myNbTriangles(0)
{
}

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

  if ( myElemSearcher ) delete myElemSearcher;
  myElemSearcher=0;
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
      const double preci = 1.e-10 * P.Distance(PC);
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
      if( V1.Magnitude()<preci ||
          V2.Magnitude()<preci ||
          V3.Magnitude()<preci ) {
        Pint = PIn;
        return true;
      }
      const double angularTol = 1e-6;
      gp_Vec VC1 = V1.Crossed(V2);
      gp_Vec VC2 = V2.Crossed(V3);
      gp_Vec VC3 = V3.Crossed(V1);
      if(VC1.Magnitude()<gp::Resolution()) {
        if(VC2.IsOpposite(VC3,angularTol)) {
          return false;
        }
      }
      else if(VC2.Magnitude()<gp::Resolution()) {
        if(VC1.IsOpposite(VC3,angularTol)) {
          return false;
        }
      }
      else if(VC3.Magnitude()<gp::Resolution()) {
        if(VC1.IsOpposite(VC2,angularTol)) {
          return false;
        }
      }
      else {
        if( VC1.IsOpposite(VC2,angularTol) || VC1.IsOpposite(VC3,angularTol) ||
            VC2.IsOpposite(VC3,angularTol) ) {
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

//================================================================================
/*!
 * \brief Checks if a line segment (P,PC) intersects any mesh face.
 *  \param P - first segment end
 *  \param PC - second segment end (it is a gravity center of quadrangle)
 *  \param Pint - (out) intersection point
 *  \param aMesh - mesh
 *  \param aShape - shape to check faces on
 *  \param NotCheckedFace - mesh face not to check
 *  \retval bool - true if there is an intersection
 */
//================================================================================

bool StdMeshers_QuadToTriaAdaptor::CheckIntersection (const gp_Pnt&       P,
                                                      const gp_Pnt&       PC,
                                                      gp_Pnt&             Pint,
                                                      SMESH_Mesh&         aMesh,
                                                      const TopoDS_Shape& aShape,
                                                      const SMDS_MeshElement* NotCheckedFace)
{
  if ( !myElemSearcher )
    myElemSearcher = SMESH_MeshEditor(&aMesh).GetElementSearcher();
  SMESH_ElementSearcher* searcher = const_cast<SMESH_ElementSearcher*>(myElemSearcher);

  //SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  //cout<<"    CheckIntersection: meshDS->NbFaces() = "<<meshDS->NbFaces()<<endl;
  bool res = false;
  double dist = RealLast(); // find intersection closest to the segment
  gp_Pnt Pres;

  gp_Ax1 line( P, gp_Vec(P,PC));
  vector< const SMDS_MeshElement* > suspectElems;
  searcher->GetElementsNearLine( line, SMDSAbs_Face, suspectElems);
  
//   for (TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next()) {
//     const TopoDS_Shape& aShapeFace = exp.Current();
//     if(aShapeFace==NotCheckedFace)
//       continue;
//     const SMESHDS_SubMesh * aSubMeshDSFace = meshDS->MeshElements(aShapeFace);
//     if ( aSubMeshDSFace ) {
//       SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
//       while ( iteratorElem->more() ) { // loop on elements on a face
//         const SMDS_MeshElement* face = iteratorElem->next();
  for ( int i = 0; i < suspectElems.size(); ++i )
  {
    const SMDS_MeshElement* face = suspectElems[i];
    if ( face == NotCheckedFace ) continue;
    Handle(TColgp_HSequenceOfPnt) aContour = new TColgp_HSequenceOfPnt;
    for ( int i = 0; i < face->NbCornerNodes(); ++i ) 
      aContour->Append( SMESH_MeshEditor::TNodeXYZ( face->GetNode(i) ));
    if( HasIntersection(P, PC, Pres, aContour) ) {
      res = true;
      double tmp = PC.Distance(Pres);
      if(tmp<dist) {
        Pint = Pres;
        dist = tmp;
      }
    }
  }
  return res;
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
  if( face->NbCornerNodes() != 4 )
  {
    myNbTriangles += int( face->NbCornerNodes() == 3 );
    return NOT_QUAD;
  }

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
  myPyramids.clear();
  myNbTriangles = 0;
  myShape = aShape;

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  SMESH_MesherHelper helper(aMesh);
  helper.IsQuadraticSubMesh(aShape);
  helper.SetElementsOnShape( true );

  for (TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next())
  {
    const TopoDS_Shape& aShapeFace = exp.Current();
    const SMESHDS_SubMesh * aSubMeshDSFace = meshDS->MeshElements( aShapeFace );
    if ( aSubMeshDSFace )
    {
      bool isRev = SMESH_Algo::IsReversedSubMesh( TopoDS::Face(aShapeFace), meshDS );

      SMDS_ElemIteratorPtr iteratorElem = aSubMeshDSFace->GetElements();
      while ( iteratorElem->more() ) // loop on elements on a geometrical face
      {
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

        if(stat==2)
        {
          // degenerate face
          // add triangles to result map
          SMDS_MeshFace* NewFace;
          if(!isRev)
            NewFace = new Q2TAdaptor_Triangle( FNodes[0], FNodes[1], FNodes[2] );
          else
            NewFace = new Q2TAdaptor_Triangle( FNodes[0], FNodes[2], FNodes[1] );
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
          bool check = CheckIntersection(PCbest, PC, Pint, aMesh, aShape, face);
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
            check = CheckIntersection(PCbestTmp, PC, Pint, aMesh, aShape, face);
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
          triaList.push_back( new Q2TAdaptor_Triangle( NewNode, FNodes[i], FNodes[i+1] ));

        // create a pyramid
        if ( isRev ) swap( FNodes[1], FNodes[3]);
        SMDS_MeshVolume* aPyram =
          helper.AddVolume( FNodes[0], FNodes[1], FNodes[2], FNodes[3], NewNode );
        myPyramids.push_back(aPyram);

      } // end loop on elements on a face submesh
    }
  } // end for(TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next()) {

  return Compute2ndPart(aMesh);
}

//================================================================================
/*!
 * \brief Computes pyramids in mesh with no shape
 */
//================================================================================

bool StdMeshers_QuadToTriaAdaptor::Compute(SMESH_Mesh& aMesh)
{
  myResMap.clear();
  myPyramids.clear();
  SMESH_MesherHelper helper(aMesh);
  helper.IsQuadraticSubMesh(aMesh.GetShapeToMesh());
  helper.SetElementsOnShape( true );

  if ( !myElemSearcher )
    myElemSearcher = SMESH_MeshEditor(&aMesh).GetElementSearcher();
  SMESH_ElementSearcher* searcher = const_cast<SMESH_ElementSearcher*>(myElemSearcher);

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();

  SMDS_FaceIteratorPtr fIt = meshDS->facesIterator(/*idInceasingOrder=*/true);
  while( fIt->more()) 
  {
    const SMDS_MeshElement* face = fIt->next();
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
      SMDS_MeshFace* NewFace;
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

      gp_Ax1 line( PC, VNorm );
      vector< const SMDS_MeshElement* > suspectElems;
      searcher->GetElementsNearLine( line, SMDSAbs_Face, suspectElems);

      for ( int iF = 0; iF < suspectElems.size(); ++iF ) {
        const SMDS_MeshElement* F = suspectElems[iF];
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
        NewFace = new Q2TAdaptor_Triangle( FNodes[0], FNodes[1], FNodes[2] );
      else
        NewFace = new Q2TAdaptor_Triangle( FNodes[0], FNodes[2], FNodes[1] );
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

    gp_Ax1 line( PC, tmpDir );
    vector< const SMDS_MeshElement* > suspectElems;
    searcher->GetElementsNearLine( line, SMDSAbs_Face, suspectElems);

    for ( int iF = 0; iF < suspectElems.size(); ++iF )
    {
      const SMDS_MeshElement* F = suspectElems[iF];
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
        SMDS_MeshFace* NewFace;
        if(isRev)
          NewFace = new Q2TAdaptor_Triangle( NewNode, FNodes[i], FNodes[i+1] );
        else
          NewFace = new Q2TAdaptor_Triangle( NewNode, FNodes[i+1], FNodes[i] );
        aList.push_back(NewFace);
      }
      // create a pyramid
      SMDS_MeshVolume* aPyram;
      if(isRev)
        aPyram = helper.AddVolume( FNodes[0], FNodes[1], FNodes[2], FNodes[3], NewNode );
      else
        aPyram = helper.AddVolume( FNodes[0], FNodes[3], FNodes[2], FNodes[1], NewNode );
      myPyramids.push_back(aPyram);
    }
  } // end loop on all faces

  return Compute2ndPart(aMesh);
}

//================================================================================
/*!
 * \brief Update created pyramids and faces to avoid their intersection
 */
//================================================================================

bool StdMeshers_QuadToTriaAdaptor::Compute2ndPart(SMESH_Mesh& aMesh)
{
  if(myPyramids.empty())
    return true;

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  int i, j, k, myShapeID = myPyramids[0]->GetNode(4)->GetPosition()->GetShapeId();

  if ( !myElemSearcher )
    myElemSearcher = SMESH_MeshEditor(&aMesh).GetElementSearcher();
  SMESH_ElementSearcher* searcher = const_cast<SMESH_ElementSearcher*>(myElemSearcher);

  set<const SMDS_MeshNode*> nodesToMove;

  // check adjacent pyramids

  for ( i = 0; i <  myPyramids.size(); ++i )
  {
    const SMDS_MeshElement* PrmI = myPyramids[i];
    MergeAdjacent( PrmI, aMesh, nodesToMove );
  }

  // iterate on all pyramids
  for ( i = 0; i <  myPyramids.size(); ++i )
  {
    const SMDS_MeshElement* PrmI = myPyramids[i];

    // compare PrmI with all the rest pyramids

    // collect adjacent pyramids and nodes coordinates of PrmI
    set<const SMDS_MeshElement*> checkedPyrams;
    vector<gp_Pnt> PsI(5);
    for(k=0; k<5; k++) // loop on 4 base nodes of PrmI
    {
      const SMDS_MeshNode* n = PrmI->GetNode(k);
      PsI[k] = SMESH_MeshEditor::TNodeXYZ( n );
      SMDS_ElemIteratorPtr vIt = n->GetInverseElementIterator( SMDSAbs_Volume );
      while ( vIt->more() )
        checkedPyrams.insert( vIt->next() );
    }

    // check intersection with distant pyramids
    for(k=0; k<4; k++) // loop on 4 base nodes of PrmI
    {
      gp_Vec Vtmp(PsI[k],PsI[4]);
      gp_Pnt Pshift = PsI[k].XYZ() + Vtmp.XYZ() * 0.01; // base node moved a bit to apex

      gp_Ax1 line( PsI[k], Vtmp );
      vector< const SMDS_MeshElement* > suspectPyrams;
      searcher->GetElementsNearLine( line, SMDSAbs_Volume, suspectPyrams);

      for ( j = 0; j < suspectPyrams.size(); ++j )
      {
        const SMDS_MeshElement* PrmJ = suspectPyrams[j];
        if ( PrmJ == PrmI || PrmJ->NbCornerNodes() != 5 )
          continue;
        if ( myShapeID != PrmJ->GetNode(4)->GetPosition()->GetShapeId())
          continue; // pyramid from other SOLID
        if ( PrmI->GetNode(4) == PrmJ->GetNode(4) )
          continue; // pyramids PrmI and PrmJ already merged
        if ( !checkedPyrams.insert( PrmJ ).second )
          continue; // already checked

        TXyzIterator xyzIt( PrmJ->nodesIterator() );
        vector<gp_Pnt> PsJ( xyzIt, TXyzIterator() );

        gp_Pnt Pint;
        bool hasInt = 
          ( HasIntersection3( Pshift, PsI[4], Pint, PsJ[0], PsJ[1], PsJ[4]) ||
            HasIntersection3( Pshift, PsI[4], Pint, PsJ[1], PsJ[2], PsJ[4]) ||
            HasIntersection3( Pshift, PsI[4], Pint, PsJ[2], PsJ[3], PsJ[4]) ||
            HasIntersection3( Pshift, PsI[4], Pint, PsJ[3], PsJ[0], PsJ[4]) );

        for(k=0; k<4 && !hasInt; k++) {
          gp_Vec Vtmp(PsJ[k],PsJ[4]);
          gp_Pnt Pshift = PsJ[k].XYZ() + Vtmp.XYZ() * 0.01;
          hasInt = 
            ( HasIntersection3( Pshift, PsJ[4], Pint, PsI[0], PsI[1], PsI[4]) ||
              HasIntersection3( Pshift, PsJ[4], Pint, PsI[1], PsI[2], PsI[4]) ||
              HasIntersection3( Pshift, PsJ[4], Pint, PsI[2], PsI[3], PsI[4]) ||
              HasIntersection3( Pshift, PsJ[4], Pint, PsI[3], PsI[0], PsI[4]) );
        }

        if ( hasInt )
        {
          // count common nodes of base faces of two pyramids
          int nbc = 0;
          for (k=0; k<4; k++)
            nbc += int ( PrmI->GetNodeIndex( PrmJ->GetNode(k) ) >= 0 );

          if ( nbc == 4 )
            continue; // pyrams have a common base face

          if(nbc>0)
          {
            // Merge the two pyramids and others already merged with them
            MergePiramids( PrmI, PrmJ, meshDS, nodesToMove );
          }
          else { // nbc==0

            // decrease height of pyramids
            gp_XYZ PCi(0,0,0), PCj(0,0,0);
            for(k=0; k<4; k++) {
              PCi += PsI[k].XYZ();
              PCj += PsJ[k].XYZ();
            }
            PCi /= 4; PCj /= 4; 
            gp_Vec VN1(PCi,PsI[4]);
            gp_Vec VN2(PCj,PsJ[4]);
            gp_Vec VI1(PCi,Pint);
            gp_Vec VI2(PCj,Pint);
            double ang1 = fabs(VN1.Angle(VI1));
            double ang2 = fabs(VN2.Angle(VI2));
            double coef1 = 0.5 - (( ang1<PI/3 ) ? cos(ang1)*0.25 : 0 );
            double coef2 = 0.5 - (( ang2<PI/3 ) ? cos(ang2)*0.25 : 0 ); // cos(ang2) ?
//             double coef2 = 0.5;
//             if(ang2<PI/3)
//               coef2 -= cos(ang1)*0.25;

            VN1.Scale(coef1);
            VN2.Scale(coef2);
            SMDS_MeshNode* aNode1 = const_cast<SMDS_MeshNode*>(PrmI->GetNode(4));
            aNode1->setXYZ( PCi.X()+VN1.X(), PCi.Y()+VN1.Y(), PCi.Z()+VN1.Z() );
            SMDS_MeshNode* aNode2 = const_cast<SMDS_MeshNode*>(PrmJ->GetNode(4));
            aNode2->setXYZ( PCj.X()+VN2.X(), PCj.Y()+VN2.Y(), PCj.Z()+VN2.Z() );
            nodesToMove.insert( aNode1 );
            nodesToMove.insert( aNode2 );
          }
          // fix intersections that could appear after apex movement
          MergeAdjacent( PrmI, aMesh, nodesToMove );
          MergeAdjacent( PrmJ, aMesh, nodesToMove );

        } // end if(hasInt)
      } // loop on suspectPyrams
    }  // loop on 4 base nodes of PrmI

  } // loop on all pyramids

  if( !nodesToMove.empty() && !meshDS->IsEmbeddedMode() )
  {
    set<const SMDS_MeshNode*>::iterator n = nodesToMove.begin();
    for ( ; n != nodesToMove.end(); ++n )
      meshDS->MoveNode( *n, (*n)->X(), (*n)->Y(), (*n)->Z() );
  }

  // rebind triangles of pyramids sharing the same base quadrangle to the first
  // entrance of the base quadrangle
  TQuad2Trias::iterator q2t = myResMap.begin(), q2tPrev = q2t;
  for ( ++q2t; q2t != myResMap.end(); ++q2t, ++q2tPrev )
  {
    if ( q2t->first == q2tPrev->first )
      q2tPrev->second.splice( q2tPrev->second.end(), q2t->second );
  }
  // delete removed triangles and count resulting nb of triangles
  for ( q2t = myResMap.begin(); q2t != myResMap.end(); ++q2t )
  {
    TTriaList & trias = q2t->second;
    for ( TTriaList::iterator tri = trias.begin(); tri != trias.end(); )
      if ( ((const Q2TAdaptor_Triangle*) *tri)->IsRemoved() )
        delete *tri, trias.erase( tri++ );
      else
        tri++, myNbTriangles++;
  }

  myPyramids.clear(); // no more needed
  myDegNodes.clear();

  delete myElemSearcher;
  myElemSearcher=0;

  return true;
}

//================================================================================
/*!
 * \brief Return list of created triangles for given face
 */
//================================================================================

const list<const SMDS_MeshFace* >* StdMeshers_QuadToTriaAdaptor::GetTriangles (const SMDS_MeshElement* aQuad)
{
  TQuad2Trias::iterator it = myResMap.find(aQuad);
  return ( it != myResMap.end() ?  & it->second : 0 );
}
