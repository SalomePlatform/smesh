// Copyright (C) 2007-2015  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : SMESH_FreeBorders.cxx
// Created   : Tue Sep  8 17:08:39 2015
// Author    : Edward AGAPOV (eap)

//================================================================================
// Implementation of SMESH_MeshAlgos::FindCoincidentFreeBorders()
//================================================================================

#include "SMESH_MeshAlgos.hxx"

#include "SMDS_LinearEdge.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_SetIterator.hxx"

#include <algorithm>
#include <limits>
#include <set>
#include <vector>

#include <NCollection_DataMap.hxx>
#include <gp_Pnt.hxx>

using namespace SMESH_MeshAlgos;

namespace
{
  struct BEdge;

  /*!
   * \brief Node on a free border
   */
  struct BNode
  {
    const SMDS_MeshNode *         myNode;
    mutable std::vector< BEdge* > myLinkedEdges;
    mutable std::vector< BEdge* > myCloseEdges;

    BNode(const SMDS_MeshNode * node): myNode( node ) {}
    void AddLinked( BEdge* e ) const;
    void AddClose ( const BEdge* e ) const;
    BEdge* FindCloseEdgeOfBorder( int borderID ) const;
    bool operator<(const BNode& other) const { return myNode->GetID() < other.myNode->GetID(); }
  };
  /*!
   * \brief Edge of a free border
   */
  struct BEdge : public SMDS_LinearEdge
  {
    const BNode*            myBNode1;
    const BNode*            myBNode2;
    int                     myBorderID;
    int                     myID; // within a border
    BEdge*                  myPrev;
    BEdge*                  myNext;
    const SMDS_MeshElement* myFace;
    std::set< int >         myCloseBorders;
    bool                    myInGroup;

    BEdge():SMDS_LinearEdge( 0, 0 ), myBorderID(-1), myID(-1), myPrev(0), myNext(0), myInGroup(0) {}

    void Set( const BNode *           node1,
              const BNode *           node2,
              const SMDS_MeshElement* face,
              const int               ID)
    {
      myBNode1   = node1;
      myBNode2   = node2;
      myNodes[0] = node1->myNode;
      myNodes[1] = node2->myNode;
      myFace     = face;
      setId( ID ); // mesh element ID
    }
    bool Contains( const BNode* n ) const
    {
      return ( n == myBNode1 || n == myBNode2 );
    }
    void AddLinked( BEdge* e )
    {
      if ( e->Contains( myBNode1 )) myPrev = e;
      else                          myNext = e;
    }
    void RemoveLinked( BEdge* e )
    {
      if ( myPrev == e ) myPrev = 0;
      if ( myNext == e ) myNext = 0;
    }
    void Reverse()
    {
      std::swap( myBNode1, myBNode2 );
      myNodes[0] = myBNode1->myNode;
      myNodes[1] = myBNode2->myNode;
    }
    void Orient()
    {
      if (( myPrev && !myPrev->Contains( myBNode1 )) ||
          ( myNext && !myNext->Contains( myBNode2 )))
        std::swap( myPrev, myNext );
      if ( myPrev && myPrev->myBNode2 != myBNode1 ) myPrev->Reverse();
      if ( myNext && myNext->myBNode1 != myBNode2 ) myNext->Reverse();
    }
    void SetID( int id )
    {
      if ( myID < 0 )
      {
        myID = id;
        if ( myNext )
          myNext->SetID( id + 1 );
      }
    }
    void FindRangeOfSameCloseBorders(BEdge* eRange[2])
    {
      eRange[0] = this;
      while ( eRange[0]->myPrev && eRange[0]->myPrev->myCloseBorders == this->myCloseBorders )
      {
        if ( eRange[0]->myPrev == this /*|| eRange[0]->myPrev->myInGroup*/ )
          break;
        eRange[0] = eRange[0]->myPrev;
      }
      eRange[1] = this;
      if ( eRange[0]->myPrev != this ) // not closed range
        while ( eRange[1]->myNext && eRange[1]->myNext->myCloseBorders == this->myCloseBorders )
        {
          if ( eRange[1]->myNext == this /*|| eRange[1]->myNext->myInGroup*/ )
            break;
          eRange[1] = eRange[1]->myNext;
        }
    }
  };

  void BNode::AddLinked( BEdge* e ) const
  {
    myLinkedEdges.reserve(2);
    myLinkedEdges.push_back( e );
    if ( myLinkedEdges.size() < 2 ) return;

    if ( myLinkedEdges.size() == 2 )
    {
      myLinkedEdges[0]->AddLinked( myLinkedEdges[1] );
      myLinkedEdges[1]->AddLinked( myLinkedEdges[0] );
    }
    else
    {
      for ( size_t i = 0; i < myLinkedEdges.size(); ++i )
        for ( size_t j = 0; j < myLinkedEdges.size(); ++j )
          if ( i != j )
            myLinkedEdges[i]->RemoveLinked( myLinkedEdges[j] );
    }
  }
  void BNode::AddClose ( const BEdge* e ) const
  {
    if ( ! e->Contains( this ))
      myCloseEdges.push_back( const_cast< BEdge* >( e ));
  }
  BEdge* BNode::FindCloseEdgeOfBorder( int borderID ) const
  {
    for ( size_t i = 0; i < myCloseEdges.size(); ++i )
      if ( borderID == myCloseEdges[ i ]->myBorderID )
        return myCloseEdges[ i ];
    return 0;
  }

  /// Accessor to SMDS_MeshElement* inherited by BEdge
  struct ElemAcess
  {
    static const SMDS_MeshElement* value( std::vector< BEdge >::const_iterator it)
    {
      return & (*it);
    }
  };
  /// Iterator over a vector of BEdge's
  static SMDS_ElemIteratorPtr getElemIterator( const std::vector< BEdge > & bedges )
  {
    typedef SMDS_SetIterator
      < const SMDS_MeshElement*, std::vector< BEdge >::const_iterator, ElemAcess > BEIter;
    return SMDS_ElemIteratorPtr( new BEIter( bedges.begin(), bedges.end() ));
  }

} // namespace

// struct needed for NCollection_Map
struct TLinkHasher
{
  static int HashCode(const SMESH_TLink& link, int aLimit)
  {
    return ::HashCode( link.node1()->GetID() + link.node2()->GetID(), aLimit );
  }
  static Standard_Boolean IsEqual(const SMESH_TLink& l1, const SMESH_TLink& l2)
  {
    return ( l1.node1() == l2.node1() && l1.node2() == l2.node2() );
  }
};

//================================================================================
/*
 * Returns groups of TFreeBorder's coincident within the given tolerance.
 * If the tolerance <= 0.0 then one tenth of an average size of elements adjacent
 * to free borders being compared is used.
 */
//================================================================================

void SMESH_MeshAlgos::FindCoincidentFreeBorders(SMDS_Mesh&              mesh,
                                                double                  tolerance,
                                                CoincidentFreeBorders & foundFreeBordes)
{
  // find free links
  typedef NCollection_DataMap<SMESH_TLink, const SMDS_MeshElement*, TLinkHasher > TLink2FaceMap;
  TLink2FaceMap linkMap;
  SMDS_FaceIteratorPtr faceIt = mesh.facesIterator();
  while ( faceIt->more() )
  {
    const SMDS_MeshElement* face = faceIt->next();
    if ( !face ) continue;

    const SMDS_MeshNode*     n0 = face->GetNode( face->NbNodes() - 1 );
    SMDS_NodeIteratorPtr nodeIt = face->interlacedNodesIterator();
    while ( nodeIt->more() )
    {
      const SMDS_MeshNode* n1 = nodeIt->next();
      SMESH_TLink link( n0, n1 );
      if ( !linkMap.Bind( link, face ))
        linkMap.UnBind( link );
      n0 = n1;
    }
  }
  if ( linkMap.IsEmpty() )
    return;

  // form free borders
  std::set   < BNode > bNodes;
  std::vector< BEdge > bEdges( linkMap.Extent() );

  TLink2FaceMap::Iterator linkIt( linkMap );
  for ( int iEdge = 0; linkIt.More(); linkIt.Next(), ++iEdge )
  {
    const SMESH_TLink & link = linkIt.Key();
    std::set< BNode >::iterator n1 = bNodes.insert( BNode( link.node1() )).first;
    std::set< BNode >::iterator n2 = bNodes.insert( BNode( link.node2() )).first;
    bEdges[ iEdge ].Set( &*n1, &*n2, linkIt.Value(), iEdge+1 );
    n1->AddLinked( & bEdges[ iEdge ] );
    n2->AddLinked( & bEdges[ iEdge ] );
  }
  linkMap.Clear();

  // assign IDs to borders
  std::vector< BEdge* > borders; // 1st of connected (via myPrev and myNext) edges
  std::set< BNode >::iterator bn = bNodes.begin();
  for ( ; bn != bNodes.end(); ++bn )
  {
    for ( size_t i = 0; i < bn->myLinkedEdges.size(); ++i )
    {
      if ( bn->myLinkedEdges[i]->myBorderID < 0 )
      {
        BEdge* be = bn->myLinkedEdges[i];
        int borderID = borders.size();
        borders.push_back( be );
        for ( ; be && be->myBorderID < 0; be = be->myNext )
        {
          be->myBorderID = borderID;
          be->Orient();
        }
        bool isClosed = ( be == bn->myLinkedEdges[i] );
        be = bn->myLinkedEdges[i]->myPrev;
        for ( ; be && be->myBorderID < 0; be = be->myPrev )
        {
          be->myBorderID = borderID;
          be->Orient();
        }
        if ( !isClosed )
          while ( borders.back()->myPrev )
            borders.back() = borders.back()->myPrev;

        borders.back()->SetID( 0 ); // set IDs to all edges of the border
      }
    }
  }

  // compute tolerance of each border
  double maxTolerance = tolerance;
  std::vector< double > bordToler( borders.size(), tolerance );
  if ( maxTolerance < std::numeric_limits< double >::min() )
  {
    // no tolerance provided by the user; compute tolerance of each border
    // as one tenth of an average size of faces adjacent to a border
    for ( size_t i = 0; i < borders.size(); ++i )
    {
      double avgFaceSize = 0;
      int    nbFaces     = 0;
      BEdge* be = borders[ i ];
      do {
        double facePerimeter = 0;
        gp_Pnt p0 = SMESH_TNodeXYZ( be->myFace->GetNode( be->myFace->NbNodes() - 1 ));
        SMDS_NodeIteratorPtr nodeIt = be->myFace->interlacedNodesIterator();
        while ( nodeIt->more() )
        {
          gp_Pnt p1 = SMESH_TNodeXYZ( nodeIt->next() );
          facePerimeter += p0.Distance( p1 );
          p0 = p1;
        }
        avgFaceSize += ( facePerimeter / be->myFace->NbCornerNodes() );
        nbFaces++;

        be = be->myNext;
      }
      while ( be && be != borders[i] );

      bordToler[ i ] = 0.1 * avgFaceSize / nbFaces;
      maxTolerance = Max( maxTolerance, bordToler[ i ]);
    }
  }

  // for every border node find close border edges
  SMESH_ElementSearcher* searcher =
    GetElementSearcher( mesh, getElemIterator( bEdges ), maxTolerance );
  SMESHUtils::Deleter< SMESH_ElementSearcher > searcherDeleter( searcher );
  std::vector< const SMDS_MeshElement* > candidateEdges;
  for ( bn = bNodes.begin(); bn != bNodes.end(); ++bn )
  {
    gp_Pnt point = SMESH_TNodeXYZ( bn->myNode );
    searcher->FindElementsByPoint( point, SMDSAbs_Edge, candidateEdges );
    if ( candidateEdges.size() <= bn->myLinkedEdges.size() )
      continue;

    double nodeTol = 0;
    for ( size_t i = 0; i < bn->myLinkedEdges.size(); ++i )
      nodeTol = Max( nodeTol, bordToler[ bn->myLinkedEdges[ i ]->myBorderID ]);

    for ( size_t i = 0; i < candidateEdges.size(); ++i )
    {
      const BEdge* be = static_cast< const BEdge* >( candidateEdges[ i ]);
      double      tol = Max( nodeTol, bordToler[ be->myBorderID ]);
      if ( maxTolerance - tol < 1e-12 ||
           !SMESH_MeshAlgos::IsOut( be, point, tol ))
        bn->AddClose( be );
    }
  }

  // for every border edge find close borders

  std::vector< BEdge* > closeEdges;
  for ( size_t i = 0; i < bEdges.size(); ++i )
  {
    BEdge& be = bEdges[i];
    if ( be.myBNode1->myCloseEdges.empty() ||
         be.myBNode2->myCloseEdges.empty() )
      continue;

    closeEdges.clear();
    for ( size_t iE1 = 0; iE1 < be.myBNode1->myCloseEdges.size(); ++iE1 )
    {
      // find edges of the same border close to both nodes of the edge
      BEdge* closeE1 = be.myBNode1->myCloseEdges[ iE1 ];
      BEdge* closeE2 = be.myBNode2->FindCloseEdgeOfBorder( closeE1->myBorderID );
      if ( !closeE2 )
        continue;
      // check that edges connecting closeE1 and closeE2 (if any) are also close to 'be'
      if ( closeE1 != closeE2 )
      {
        bool coincide;
        for ( int j = 0; j < 2; ++j ) // move closeE1 -> closeE2 or inversely
        {
          BEdge* ce = closeE1;
          do {
            coincide = ( ce->myBNode2->FindCloseEdgeOfBorder( be.myBorderID ));
            ce       = ce->myNext;
          } while ( coincide && ce && ce != closeE2 );

          if ( coincide && ce == closeE2 )
            break;
          if ( j == 0 )
            std::swap( closeE1, closeE2 );
          coincide = false;
        }
        if ( !coincide )
          continue;
        closeEdges.push_back( closeE1 );
        closeEdges.push_back( closeE2 );
      }
      else
      {
        closeEdges.push_back( closeE1 );
      }
      be.myCloseBorders.insert( closeE1->myBorderID );
    }
    if ( !closeEdges.empty() )
    {
      be.myCloseBorders.insert( be.myBorderID );
      // for ( size_t iB = 0; iB < closeEdges.size(); ++iB )
      //   closeEdges[ iB ]->myCloseBorders.insert( be.myCloseBorders.begin(),
      //                                            be.myCloseBorders.end() );
    }
  }

  // Fill in CoincidentFreeBorders

  // save nodes of free borders
  foundFreeBordes._borders.resize( borders.size() );
  for ( size_t i = 0; i < borders.size(); ++i )
  {
    BEdge* be = borders[i];
    foundFreeBordes._borders[i].push_back( be->myBNode1->myNode );
    do {
      foundFreeBordes._borders[i].push_back( be->myBNode2->myNode );
      be = be->myNext;
    }
    while ( be && be != borders[i] );
  }

  // form groups of coincident parts of free borders

  TFreeBorderPart  part;
  TCoincidentGroup group;
  for ( size_t i = 0; i < borders.size(); ++i )
  {
    BEdge* be = borders[i];

    // look for an edge close to other borders
    do {
      if ( !be->myInGroup && !be->myCloseBorders.empty() )
        break;
      be = be->myNext;
    } while ( be && be != borders[i] );

    if ( !be || be->myInGroup || be->myCloseBorders.empty() )
      continue; // all edges of a border treated or are non-coincident

    group.clear();

    // look for the 1st and last edge of a coincident group
    BEdge* beRange[2];
    be->FindRangeOfSameCloseBorders( beRange );
    BEdge* be1st = beRange[0];

    // fill in a group
    part._border   = i;
    part._node1    = beRange[0]->myID;
    part._node2    = beRange[0]->myID + 1;
    part._nodeLast = beRange[1]->myID + 1;
    group.push_back( part );

    be = beRange[0];
    be->myInGroup = true;
    while ( be != beRange[1] )
    {
      be->myInGroup = true;
      be = be->myNext;
    }
    beRange[1]->myInGroup = true;

    // add parts of other borders
    std::set<int>::iterator closeBord = be1st->myCloseBorders.begin();
    for ( ; closeBord != be1st->myCloseBorders.end(); ++closeBord )
    {
      be = be1st->myBNode2->FindCloseEdgeOfBorder( *closeBord );
      if ( !be ) continue;

      be->FindRangeOfSameCloseBorders( beRange );

      // find out mutual orientation of borders
      bool reverse = ( beRange[0]->myBNode1->FindCloseEdgeOfBorder( i ) != be1st &&
                       beRange[0]->myBNode2->FindCloseEdgeOfBorder( i ) != be1st );

      // fill in a group
      part._border   = beRange[0]->myBorderID;
      if ( reverse ) {
        part._node1    = beRange[1]->myID + 1;
        part._node2    = beRange[1]->myID;
        part._nodeLast = beRange[0]->myID;
      }
      else  {
        part._node1    = beRange[0]->myID;
        part._node2    = beRange[0]->myID + 1;
        part._nodeLast = beRange[1]->myID + 1;
      }
      group.push_back( part );

      be = beRange[0];
      be->myInGroup = true;
      while ( be != beRange[1] )
      {
        be->myInGroup = true;
        be = be->myNext;
      }
      beRange[1]->myInGroup = true;
    }

    foundFreeBordes._coincidentGroups.push_back( group );

  } // loop on free borders
}
