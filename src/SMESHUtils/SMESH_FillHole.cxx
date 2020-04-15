// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : SMESH_FillHole.cxx
// Created   : Tue Sep 26 15:11:17 2017
// Author    : Edward AGAPOV (eap)
//

#include "SMESH_MeshAlgos.hxx"

#include "SMESH_Comment.hxx"

#include "ObjectPool.hxx"
#include "SMDS_Mesh.hxx"
#include "SMESH_TypeDefs.hxx"

#include <Utils_SALOME_Exception.hxx>

#include <boost/intrusive/circular_list_algorithms.hpp>
//#include <boost/container/flat_map.hpp>

#include <Bnd_B3d.hxx>

namespace
{
  bool isSmallAngle( double cos2 )
  {
    // cosine of min angle at which adjacent faces are considered overlapping
    const double theMinCos2 = 0.996 * 0.996; // ~5 degrees
    return ( cos2 > theMinCos2 );
  }

  struct BEdge;
  typedef std::multimap< double, BEdge* >          TAngleMap;
  typedef std::map< const SMDS_MeshElement*, int > TFaceIndMap;

  //--------------------------------------------------------------------------------
  /*!
   * \brief Edge of a free border
   */
  struct BEdge
  {
    const SMDS_MeshNode*    myNode1;
    const SMDS_MeshNode*    myNode2;
    const SMDS_MeshElement* myFace;   // face adjacent to the border

    gp_XYZ                  myFaceNorm;
    gp_XYZ                  myDir;     // myNode1 -> myNode2
    double                  myDirCoef; // 1. or -1, to make myDir oriented as myNodes in myFace
    double                  myLength;  // between nodes
    double                  myAngleWithPrev; // between myDir and -myPrev->myDir
    double                  myMinMaxRatio; // of a possible triangle sides
    TAngleMap::iterator     myAngleMapPos;
    double                  myOverlapAngle;  // angle delta due to overlapping
    const SMDS_MeshNode*    myNode1Shift;    // nodes created to avoid overlapping of faces
    const SMDS_MeshNode*    myNode2Shift;

    BEdge*                  myPrev; // neighbors in the border
    BEdge*                  myNext;

    BEdge(): myNode1Shift(0), myNode2Shift(0) {}
    void   Init( const SMDS_MeshNode* n1, const SMDS_MeshNode* n2,
                 const SMDS_MeshElement* f=0,
                 const SMDS_MeshNode* nf1=0, const SMDS_MeshNode* nf2=0 );
    void   ComputeAngle( bool reverseAngle = false );
    void   ShiftOverlapped( const SMDS_MeshNode*                  oppNode,
                            const TFaceIndMap&                    capFaceWithBordInd,
                            SMDS_Mesh&                            mesh,
                            std::vector<const SMDS_MeshElement*>& newFaces);
    void   MakeShiftfFaces( SMDS_Mesh&                            mesh,
                            std::vector<const SMDS_MeshElement*>& newFaces,
                            const bool                            isReverse );
    gp_XYZ GetInFaceDir() const { return myFaceNorm ^ myDir * myDirCoef; }
    double ShapeFactor()  const { return 0.5 * ( 1. - myMinMaxRatio ); }
    void   InsertSelf(TAngleMap& edgesByAngle, bool isReverseFaces, bool reBind, bool useOverlap )
    {
      if ( reBind ) edgesByAngle.erase( myAngleMapPos );
      double key = (( isReverseFaces ? 2 * M_PI - myAngleWithPrev : myAngleWithPrev )
                    + myOverlapAngle * useOverlap
                    + ShapeFactor() );
      myAngleMapPos = edgesByAngle.insert( std::make_pair( key, this ));
    }

    // traits used by boost::intrusive::circular_list_algorithms
    typedef BEdge         node;
    typedef BEdge *       node_ptr;
    typedef const BEdge * const_node_ptr;
    static node_ptr get_next(const_node_ptr n)             {  return n->myNext;  }
    static void     set_next(node_ptr n, node_ptr next)    {  n->myNext = next;  }
    static node_ptr get_previous(const_node_ptr n)         {  return n->myPrev;  }
    static void     set_previous(node_ptr n, node_ptr prev){  n->myPrev = prev;  }
  };

  //================================================================================
  /*!
   * \brief Initialize a border edge data
   */
  //================================================================================

  void BEdge::Init( const SMDS_MeshNode*    n1,
                    const SMDS_MeshNode*    n2,
                    const SMDS_MeshElement* newFace, // new cap face
                    const SMDS_MeshNode*    nf1,
                    const SMDS_MeshNode*    nf2 )
  {
    myNode1  = n1;
    myNode2  = n2;
    myDir    = SMESH_NodeXYZ( n2 ) - SMESH_NodeXYZ( n1 );
    myLength = myDir.Modulus();
    if ( myLength > std::numeric_limits<double>::min() )
      myDir /= myLength;

    myFace = newFace;
    if ( !myFace )
    {
      TIDSortedElemSet elemSet, avoidSet;
      int ind1, ind2;
      myFace = SMESH_MeshAlgos::FindFaceInSet( n1, n2, elemSet, avoidSet, &ind1, &ind2 );
      if ( !myFace )
        throw SALOME_Exception( SMESH_Comment("No face sharing nodes #")
                                << myNode1->GetID() << " and #" << myNode2->GetID());
      avoidSet.insert( myFace );
      if ( SMESH_MeshAlgos::FindFaceInSet( n1, n2, elemSet, avoidSet ))
        throw SALOME_Exception( SMESH_Comment("No free border between nodes #")
                                << myNode1->GetID() << " and #" << myNode2->GetID());

      myDirCoef = SMESH_MeshAlgos::IsRightOrder( myFace, myNode1, myNode2 ) ? 1. : -1.;
    }

    if (! SMESH_MeshAlgos::FaceNormal( myFace, myFaceNorm, /*normalized=*/false ))
    {
      SMDS_ElemIteratorPtr fIt = myNode1->GetInverseElementIterator( SMDSAbs_Face );
      while ( fIt->more() )
        if ( SMESH_MeshAlgos::FaceNormal( fIt->next(), myFaceNorm, /*normalized=*/false ))
          break;
    }

    if ( newFace )
    {
      myFace    = 0;
      myDirCoef = SMESH_MeshAlgos::IsRightOrder( newFace, nf1, nf2 ) ? 1. : -1.;
      if ( myPrev->myNode2 == n1 )
        myNode1Shift = myPrev->myNode2Shift;
      if ( myNext->myNode1 == n2 )
        myNode2Shift = myNext->myNode1Shift;
    }
    else if ( myDirCoef * myPrev->myDirCoef < 0 ) // different orientation of faces
    {
      myFaceNorm *= -1;
      myDirCoef  *= -1;
    }
  }

  //================================================================================
  /*!
   * \brief Compute myAngleWithPrev
   */
  //================================================================================

  void BEdge::ComputeAngle( bool theReverseAngle )
  {
    double dot = myDir.Dot( myPrev->myDir.Reversed() );
    if      ( dot >=  1 ) myAngleWithPrev = 0;
    else if ( dot <= -1 ) myAngleWithPrev = M_PI;
    else                  myAngleWithPrev = acos( dot );

    bool isObtuse;
    gp_XYZ inFaceDirNew = myDir - myPrev->myDir;
    gp_XYZ   inFaceDir1 = myPrev->GetInFaceDir();
    gp_XYZ   inFaceDir2 = this->GetInFaceDir();
    double         dot1 = inFaceDirNew * inFaceDir1;
    double         dot2 = inFaceDirNew * inFaceDir2;
    bool     isOverlap1 = ( dot1 > 0 );
    bool     isOverlap2 = ( dot2 > 0 );
    if ( !myPrev->myFace )
      isObtuse = isOverlap1;
    else if  ( !myFace )
      isObtuse = isOverlap2;
    else
    {
      double dt1 = myDir.Dot( myPrev->myFaceNorm );
      double dt2 = myPrev->myDir.Dot( myFaceNorm );
      isObtuse = ( dt1 > 0 || dt2 < 0 ); // suppose face normals point outside the border
      if ( theReverseAngle )
        isObtuse = !isObtuse;
    }
    if ( isObtuse )
    {
      myAngleWithPrev = 2 * M_PI - myAngleWithPrev;
    }

    // if ( ! isObtuse )
    //   isObtuse =
    //     isSmallAngle( 1 - myDir.CrossSquareMagnitude( myPrev->myDir )); // edges co-directed

    myOverlapAngle = 0;
    //if ( !isObtuse )
    {
      // check if myFace and a triangle built on this and prev edges overlap
      if ( isOverlap1 )
      {
        double cos2 = dot1 * dot1 / inFaceDirNew.SquareModulus() / inFaceDir1.SquareModulus();
        myOverlapAngle += 1. * M_PI * cos2;
      }
      if ( isOverlap2 )
      {
        double cos2 = dot2 * dot2 / inFaceDirNew.SquareModulus() / inFaceDir2.SquareModulus();
        myOverlapAngle += 1. * M_PI * cos2;
      }
    }

    {
      double len3 = SMESH_NodeXYZ( myPrev->myNode1 ).Distance( myNode2 );
      double minLen = Min( myLength, Min( myPrev->myLength, len3 ));
      double maxLen = Max( myLength, Max( myPrev->myLength, len3 ));
      myMinMaxRatio = minLen / maxLen;
    }
  }

  //================================================================================
  /*!
   * \brief Check if myFace is overlapped by a triangle formed by myNode's and a
   *        given node. If so, create shifted nodes to avoid overlapping
   */
  //================================================================================

  void BEdge::ShiftOverlapped( const SMDS_MeshNode*                  theOppNode,
                               const TFaceIndMap&                    theCapFaceWithBordInd,
                               SMDS_Mesh&                            theMesh,
                               std::vector<const SMDS_MeshElement*>& theNewFaces )
  {
    if ( myNode1Shift && myNode2Shift )
      return;

    gp_XYZ inNewFaceDir = SMESH_NodeXYZ( theOppNode ) - SMESH_NodeXYZ( myNode1 );
    double          dot = inNewFaceDir.Dot( myFaceNorm );
    double         cos2 = dot * dot / myFaceNorm.SquareModulus() / inNewFaceDir.SquareModulus();
    bool      isOverlap = ( isSmallAngle( 1 - cos2 ) && GetInFaceDir() * inNewFaceDir > 0 );

    if ( isOverlap )
    {
      gp_XYZ shift = myFaceNorm / myLength / 4;
      if ( myFace )
        shift.Reverse();
      if ( !myNode1Shift )
      {
        gp_XYZ p = SMESH_NodeXYZ( myNode1 ) + shift;
        myNode1Shift = theMesh.AddNode( p.X(), p.Y(), p.Z() );
        myPrev->myNode2Shift = myNode1Shift;
      }
      if ( !myNode2Shift )
      {
        gp_XYZ p = SMESH_NodeXYZ( myNode2 ) + shift;
        myNode2Shift = theMesh.AddNode( p.X(), p.Y(), p.Z() );
        myNext->myNode1Shift = myNode2Shift;
      }

      // MakeShiftfFaces() for already created cap faces
      for ( int is2nd = 0; is2nd < 2; ++is2nd )
      {
        const SMDS_MeshNode* ns = is2nd ? myNode2Shift : myNode1Shift;
        const SMDS_MeshNode* n  = is2nd ? myNode2 : myNode1;
        if ( !ns ) continue;

        SMDS_ElemIteratorPtr fIt = n->GetInverseElementIterator( SMDSAbs_Face );
        while ( fIt->more() )
        {
          const SMDS_MeshElement* f = fIt->next();
          if ( !f->isMarked() ) continue;

          TFaceIndMap::const_iterator f2i = theCapFaceWithBordInd.find( f );
          if ( f2i == theCapFaceWithBordInd.end() )
            continue;
          const SMDS_MeshNode* nf1 = f->GetNode(  f2i->second );
          const SMDS_MeshNode* nf2 = f->GetNode(( f2i->second+1 ) % f->NbNodes() );
          if ( nf1 == n || nf2 == n )
          {
            BEdge tmpE;
            tmpE.myPrev = tmpE.myNext = this;
            tmpE.Init( nf1, nf2, f, nf1, nf2 );
            if ( !tmpE.myNode1Shift && !tmpE.myNode2Shift )
              tmpE.Init( nf2, nf1, f, nf2, nf1 );
            tmpE.myFace = f;
            tmpE.MakeShiftfFaces( theMesh, theNewFaces, tmpE.myDirCoef < 0 );
          }
          std::vector< const SMDS_MeshNode* > nodes( f->begin_nodes(), f->end_nodes() );
          nodes[ f->GetNodeIndex( n ) ] = ns;
          theMesh.ChangeElementNodes( f, &nodes[0], nodes.size() );
        }
      }
    }
  }

  //================================================================================
  /*!
   * \brief Create a triangle
   */
  //================================================================================

  const SMDS_MeshElement* MakeTria( SMDS_Mesh&           mesh,
                                    const SMDS_MeshNode* n1,
                                    const SMDS_MeshNode* n2,
                                    const SMDS_MeshNode* n3,
                                    const bool           isReverse )
  {
    if ( isReverse )
      return mesh.AddFace( n1, n3, n2 );
    return mesh.AddFace( n1, n2, n3 );
  }

  //================================================================================
  /*!
   * \brief Create a quadrangle
   */
  //================================================================================

  // const SMDS_MeshElement* MakeQuad( SMDS_Mesh&           mesh,
  //                                   const SMDS_MeshNode* n1,
  //                                   const SMDS_MeshNode* n2,
  //                                   const SMDS_MeshNode* n3,
  //                                   const SMDS_MeshNode* n4,
  //                                   const bool           isReverse )
  // {
  //   if ( isReverse )
  //     return mesh.AddFace( n4, n3, n2, n1 );
  //   return mesh.AddFace( n1, n2, n3, n4 );
  // }

  //================================================================================
  /*!
   * \brief Create faces on myNode* and myNode*Shift
   */
  //================================================================================

  void BEdge::MakeShiftfFaces(SMDS_Mesh&                            mesh,
                              std::vector<const SMDS_MeshElement*>& newFaces,
                              const bool                            isReverse )
  {
    if ( !myFace )
      return;
    if ( myNode1Shift && myNode2Shift )
    {
      newFaces.push_back( MakeTria( mesh, myNode1, myNode2, myNode2Shift, isReverse ));
      newFaces.push_back( MakeTria( mesh, myNode1, myNode2Shift, myNode1Shift, isReverse ));
    }
    else if ( myNode1Shift )
    {
      newFaces.push_back( MakeTria( mesh, myNode1, myNode2, myNode1Shift, isReverse ));
    }
    else if ( myNode2Shift )
    {
      newFaces.push_back( MakeTria( mesh, myNode1, myNode2, myNode2Shift, isReverse ));
    }
  }

} // namespace

//================================================================================
/*!
 * \brief Fill with 2D elements a hole defined by a TFreeBorder
 */
//================================================================================

void SMESH_MeshAlgos::FillHole(const SMESH_MeshAlgos::TFreeBorder &  theFreeBorder,
                               SMDS_Mesh&                            theMesh,
                               std::vector<const SMDS_MeshElement*>& theNewFaces)
{
  if ( theFreeBorder.size() < 4 ||                // at least 3 nodes
       theFreeBorder[0] != theFreeBorder.back() ) // the hole must be closed
    return;

  // prepare data of the border

  ObjectPool< BEdge > edgeAllocator;
  boost::intrusive::circular_list_algorithms< BEdge > circularList;
  BEdge* edge;
  BEdge* edge0 = edgeAllocator.getNew();
  BEdge* edgePrev = edge0;
  circularList.init_header( edge0 );
  edge0->Init( theFreeBorder[0], theFreeBorder[1], 0 );
  Bnd_B3d box;
  box.Add( SMESH_NodeXYZ( edge0->myNode1 ));
  for ( size_t i = 2; i < theFreeBorder.size(); ++i )
  {
    edge = edgeAllocator.getNew();
    circularList.link_after( edgePrev, edge );
    edge->Init( theFreeBorder[i-1], theFreeBorder[i] );
    edge->ComputeAngle();
    edgePrev = edge;
    box.Add( SMESH_NodeXYZ( edge->myNode1 ));
  }
  edge0->ComputeAngle();

  // check if face normals point outside the border

  gp_XYZ hSize = 0.5 * ( box.CornerMax() - box.CornerMin() );
  const double hDelta = 1e-6 * hSize.Modulus();
  hSize -= gp_XYZ( hDelta, hDelta, hDelta );
  if ( hSize.X() < 0 ) hSize.SetX(hDelta);
  if ( hSize.Y() < 0 ) hSize.SetY(hDelta);
  if ( hSize.Z() < 0 ) hSize.SetZ(hDelta);
  box.SetHSize( hSize ); // decrease the box by hDelta

  size_t nbEdges = theFreeBorder.size() - 1;
  edge = edge0;
  int nbRev = 0, nbFrw = 0;
  double angTol = M_PI - ( nbEdges - 2 ) * M_PI / nbEdges, sumDirCoeff = 0;
  for ( size_t i = 0; i < nbEdges; ++i, edge = edge->myNext )
  {
    if ( box.IsOut( SMESH_NodeXYZ( edge->myNode1 )) &&
         edge->myOverlapAngle < 0.1 * M_PI )
    {
      nbRev += edge->myAngleWithPrev > M_PI + angTol;
      nbFrw += edge->myAngleWithPrev < M_PI - angTol;
    }
    sumDirCoeff += edge->myDirCoef;

    // unmark all adjacent faces, new faces will be marked
    SMDS_ElemIteratorPtr fIt = edge->myNode1->GetInverseElementIterator( SMDSAbs_Face );
    while ( fIt->more() )
      fIt->next()->setIsMarked( false );
  }
  bool isReverseAngle = ( nbRev > nbFrw ); // true == face normals point inside the border
  //std::cout << "nbRev="<< nbRev << ", nbFrw="<< nbFrw<<std::endl;

  // sort border edges by myAngleWithPrev

  TAngleMap edgesByAngle;
  bool useOverlap = true; // to add BEdge.myOverlapAngle when filling edgesByAngle
  edge = edge0;
  for ( size_t i = 0; i < nbEdges; ++i, edge = edge->myNext )
    edge->InsertSelf( edgesByAngle, isReverseAngle, /*reBind=*/false, useOverlap );

  // create triangles to fill the hole

  //compare order of nodes in the edges with their order in faces
  bool isReverse = sumDirCoeff > 0.5 * nbEdges;

  // faces filling the hole (cap faces) and indices of border edges in them
  TFaceIndMap capFaceWithBordInd;

  theNewFaces.reserve( nbEdges - 2 );
  std::vector< const SMDS_MeshNode* > nodes(3);
  while ( edgesByAngle.size() > 2 )
  {
    TAngleMap::iterator a2e = edgesByAngle.begin();
    edge = a2e->second;
    if ( useOverlap &&
         a2e->first - edge->ShapeFactor() > M_PI - angTol ) // all new triangles need shift
    {
      // re-sort the edges w/o overlap consideration
      useOverlap = false;
      nbEdges = edgesByAngle.size();
      edgesByAngle.clear();
      for ( size_t i = 0; i < nbEdges; ++i, edge = edge->myNext )
        edge->InsertSelf( edgesByAngle, isReverseAngle, /*reBind=*/false, useOverlap );
      a2e = edgesByAngle.begin();
    }
    edge     = a2e->second;
    edgePrev = edge->myPrev;

    // create shift nodes and faces
    edgePrev->ShiftOverlapped( edge->myNode2, capFaceWithBordInd, theMesh, theNewFaces );
    edge->ShiftOverlapped( edgePrev->myNode1, capFaceWithBordInd, theMesh, theNewFaces );
    edge    ->MakeShiftfFaces( theMesh, theNewFaces, isReverse );
    edgePrev->MakeShiftfFaces( theMesh, theNewFaces, isReverse );

    // make a cap face
    //nodes.resize( 3 );
    nodes[0] = edgePrev->myNode1Shift ? edgePrev->myNode1Shift : edgePrev->myNode1;
    nodes[1] = edgePrev->myNode2Shift ? edgePrev->myNode2Shift : edgePrev->myNode2;
    nodes[2] = edge->myNode2Shift     ? edge->myNode2Shift     : edge->myNode2;
    theNewFaces.push_back( MakeTria( theMesh, nodes[0], nodes[1], nodes[2], isReverse ));
    // std::cout << nodes[1]->GetID() << " "  << nodes[0]->GetID() << " "  << nodes[2]->GetID()
    //           << " " << edge->myAngleWithPrev << std::endl;

    // remember a border edge within the new cap face
    theNewFaces.back()->setIsMarked( true );
    if ( edgePrev->myFace )
      capFaceWithBordInd.insert( std::make_pair( theNewFaces.back(), isReverse ? 2 : 0 ));
    if ( edge->myFace )
      capFaceWithBordInd.insert( std::make_pair( theNewFaces.back(), 1 ));

    // remove edgePrev from the list and update <edge>
    edgesByAngle.erase( edgePrev->myAngleMapPos );
    circularList.unlink( edgePrev ); // remove edgePrev from the border

    edge->Init( edgePrev->myNode1, edge->myNode2, theNewFaces.back(), nodes[0], nodes[2] );
    edge->ComputeAngle( isReverseAngle );
    edge->InsertSelf( edgesByAngle, /*isReverse=*/false, /*reBind=*/true, useOverlap );
    edge->myNext->ComputeAngle( isReverseAngle );
    edge->myNext->InsertSelf( edgesByAngle, /*isReverse=*/false, /*reBind=*/true, useOverlap );
    // std::cout << "A " << edge->myNode1->GetID() << " " << edge->myAngleWithPrev
    //           << " " << edge->myNext->myNode1->GetID() << " " << edge->myNext->myAngleWithPrev
    //           << std::endl;
  }
  edge = edgesByAngle.begin()->second;
  edge->        MakeShiftfFaces( theMesh, theNewFaces, isReverse );
  edge->myNext->MakeShiftfFaces( theMesh, theNewFaces, isReverse );
}
