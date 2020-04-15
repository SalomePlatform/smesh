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
// File      : SMESH_Offset.cxx
// Created   : Mon Dec 25 15:52:38 2017
// Author    : Edward AGAPOV (eap)

#include "SMESH_MeshAlgos.hxx"

#include <SMDS_PolygonalFaceOfNodes.hxx>
#include "SMDS_Mesh.hxx"

#include <Utils_SALOME_Exception.hxx>

#include <Bnd_B3d.hxx>
#include <NCollection_Map.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>

#include <boost/container/flat_set.hpp>
#include <boost/dynamic_bitset.hpp>

namespace
{
  const int theMaxNbFaces = 256; // max number of faces sharing a node

  typedef NCollection_DataMap< const SMDS_MeshNode*, const SMDS_MeshNode*, SMESH_Hasher > TNNMap;
  typedef NCollection_Map< SMESH_Link, SMESH_Link >                                       TLinkMap;

  //--------------------------------------------------------------------------------
  /*!
   * \brief Intersected face side storing a node created at this intersection
   *        and an intersected face
   */
  struct CutLink
  {
    bool                     myReverse;
    const SMDS_MeshNode*     myNode[2]; // side nodes. WARNING: don't set them directly, use Set()
    mutable SMESH_NodeXYZ    myIntNode; // intersection node
    const SMDS_MeshElement*  myFace;    // cutter face
    int                      myIndex;   // index of a node on the same link

    CutLink(const SMDS_MeshNode*    node1=0,
            const SMDS_MeshNode*    node2=0,
            const SMDS_MeshElement* face=0,
            const int               index=0) { Set ( node1, node2, face, index ); }

    void Set( const SMDS_MeshNode*    node1,
              const SMDS_MeshNode*    node2,
              const SMDS_MeshElement* face,
              const int               index=0)
    {
      myNode[0] = node1; myNode[1] = node2; myFace = face; myIndex = index; myReverse = false;
      if ( myNode[0] && ( myReverse = ( myNode[0]->GetID() > myNode[1]->GetID() )))
        std::swap( myNode[0], myNode[1] );
    }
    const SMDS_MeshNode* IntNode() const { return myIntNode.Node(); }
    const SMDS_MeshNode* Node1() const { return myNode[ myReverse ]; }
    const SMDS_MeshNode* Node2() const { return myNode[ !myReverse ]; }

    static Standard_Integer HashCode(const CutLink&         link,
                                     const Standard_Integer upper)
    {
      Standard_Integer n = ( link.myNode[0]->GetID() +
                             link.myNode[1]->GetID() +
                             link.myIndex );
      return ::HashCode( n, upper );
    }
    static Standard_Boolean IsEqual(const CutLink& link1, const CutLink& link2 )
    {
      return ( link1.myNode[0] == link2.myNode[0] &&
               link1.myNode[1] == link2.myNode[1] &&
               link1.myIndex == link2.myIndex );
    }
  };

  typedef NCollection_Map< CutLink, CutLink > TCutLinkMap;

  //--------------------------------------------------------------------------------
  /*!
   * \brief Part of a divided face edge
   */
  struct EdgePart
  {
    const SMDS_MeshNode*    myNode1;
    const SMDS_MeshNode*    myNode2;
    int                     myIndex; // positive -> side index, negative -> State
    const SMDS_MeshElement* myFace;

    enum State { _INTERNAL = -1, _COPLANAR = -2, _PENDING = -3 };

    void Set( const SMDS_MeshNode*    Node1,
              const SMDS_MeshNode*    Node2,
              const SMDS_MeshElement* Face = 0,
              int                     EdgeIndex = _INTERNAL )
    { myNode1 = Node1; myNode2 = Node2; myIndex = EdgeIndex; myFace = Face; }

    // bool HasSameNode( const EdgePart& other ) { return ( myNode1 == other.myNode1 ||
    //                                                      myNode1 == other.myNode2 ||
    //                                                      myNode2 == other.myNode1 ||
    //                                                      myNode2 == other.myNode2 );
    // }
    bool IsInternal() const { return myIndex < 0; }
    bool IsTwin( const EdgePart& e ) const { return myNode1 == e.myNode2 && myNode2 == e.myNode1; }
    bool IsSame( const EdgePart& e ) const {
      return (( myNode1 == e.myNode2 && myNode2 == e.myNode1 ) ||
              ( myNode1 == e.myNode1 && myNode2 == e.myNode2 )); }
    bool ReplaceCoplanar( const EdgePart& e );
    operator SMESH_Link() const { return SMESH_Link( myNode1, myNode2 ); }
    operator gp_Vec() const { return SMESH_NodeXYZ( myNode2 ) - SMESH_NodeXYZ( myNode1 ); }
  };

  //--------------------------------------------------------------------------------
  /*!
   * \brief Loop of EdgePart's forming a new face which is a part of CutFace
   */
  struct EdgeLoop : public SMDS_PolygonalFaceOfNodes
  {
    std::vector< const EdgePart* > myLinks;
    bool                           myIsBndConnected; //!< is there a path to CutFace side edges
    bool                           myHasPending;     //!< an edge encounters twice

    EdgeLoop() : SMDS_PolygonalFaceOfNodes( std::vector<const SMDS_MeshNode *>() ) {}
    void Clear() { myLinks.clear(); myIsBndConnected = false; myHasPending = false; }
    bool SetConnected() { bool was = myIsBndConnected; myIsBndConnected = true; return !was; }
    size_t Contains( const SMDS_MeshNode* n ) const
    {
      for ( size_t i = 0; i < myLinks.size(); ++i )
        if ( myLinks[i]->myNode1 == n ) return i + 1;
      return 0;
    }
    virtual int NbNodes() const { return myLinks.size(); }
    virtual SMDS_ElemIteratorPtr nodesIterator() const
    {
      return setNodes(), SMDS_PolygonalFaceOfNodes::nodesIterator();
    }
    virtual SMDS_NodeIteratorPtr nodeIterator() const
    {
      return setNodes(), SMDS_PolygonalFaceOfNodes::nodeIterator();
    }
    void setNodes() const //!< set nodes to SMDS_PolygonalFaceOfNodes
    {
      EdgeLoop* me = const_cast<EdgeLoop*>( this );
      me->myNodes.resize( NbNodes() );
      size_t iMin = 0;
      for ( size_t i = 1; i < myNodes.size(); ++i ) {
        if ( myLinks[ i ]->myNode1->GetID() < myLinks[ iMin ]->myNode1->GetID() )
          iMin = i;
      }
      for ( size_t i = 0; i < myNodes.size(); ++i )
        me->myNodes[ i ] = myLinks[ ( iMin + i ) % myNodes.size() ]->myNode1;
    }
  };

  //--------------------------------------------------------------------------------
  /*!
   * \brief Set of EdgeLoop's constructed from a CutFace
   */
  struct EdgeLoopSet
  {
    std::vector< EdgeLoop >  myLoops;       //!< buffer of EdgeLoop's
    size_t                   myNbLoops;     //!< number of constructed loops

    const EdgePart*          myEdge0;       //!< & CutFace.myLinks[0]
    size_t                   myNbUsedEdges; //!< nb of EdgePart's added to myLoops
    boost::dynamic_bitset<>  myIsUsedEdge;  //!< is i-th EdgePart of CutFace is in any EdgeLoop
    std::vector< EdgeLoop* > myLoopOfEdge;  //!< EdgeLoop of CutFace.myLinks[i]
    std::vector< EdgePart* > myCandidates;  //!< EdgePart's starting at the same node

    EdgeLoopSet(): myLoops(100) {}

    void Init( const std::vector< EdgePart >& edges )
    {
      size_t nb = edges.size();
      myEdge0 = & edges[0];
      myNbLoops = 0;
      myNbUsedEdges = 0;
      myIsUsedEdge.reset();
      myIsUsedEdge.resize( nb, false );
      myLoopOfEdge.clear();
      myLoopOfEdge.resize( nb, (EdgeLoop*) 0 );
    }
    EdgeLoop& AddNewLoop()
    {
      if ( ++myNbLoops >= myLoops.size() )
        myLoops.resize( myNbLoops + 10 );
      myLoops[ myNbLoops-1 ].Clear();
      return myLoops[ myNbLoops-1 ];
    }
    bool AllEdgesUsed() const { return myNbUsedEdges == myLoopOfEdge.size(); }

    bool AddEdge( EdgePart& edge )
    {
      size_t i = Index( edge );
      if ( myIsUsedEdge[ i ])
        return false;
      myLoops[ myNbLoops-1 ].myLinks.push_back( &edge );
      myLoopOfEdge[ i ] = & myLoops[ myNbLoops-1 ];
      myIsUsedEdge[ i ] = true;
      ++myNbUsedEdges;
      return true;
    }
    void Erase( EdgeLoop* loop )
    {
      for ( size_t iE = 0; iE < loop->myLinks.size(); ++iE )
        myLoopOfEdge[ Index( *loop->myLinks[ iE ] )] = 0;
      loop->Clear();
    }
    void Join( EdgeLoop& loop1, size_t iAfterConcact,
               EdgeLoop& loop2, size_t iFromEdge2 )
    {
      std::vector< const EdgePart* > linksAfterContact( loop1.myLinks.begin() + iAfterConcact,
                                                        loop1.myLinks.end() );
      loop1.myLinks.reserve( loop2.myLinks.size() + loop1.myLinks.size() );
      loop1.myLinks.resize( iAfterConcact );
      loop1.myLinks.insert( loop1.myLinks.end(),
                            loop2.myLinks.begin() + iFromEdge2, loop2.myLinks.end() );
      loop1.myLinks.insert( loop1.myLinks.end(),
                            loop2.myLinks.begin(), loop2.myLinks.begin() + iFromEdge2 );
      loop1.myLinks.insert( loop1.myLinks.end(),
                            linksAfterContact.begin(), linksAfterContact.end() );
      loop1.myIsBndConnected = loop2.myIsBndConnected;
      loop2.Clear();
      for ( size_t iE = 0; iE < loop1.myLinks.size(); ++iE )
        myLoopOfEdge[ Index( *loop1.myLinks[ iE ] )] = & loop1;
    }
    size_t    Index( const EdgePart& edge ) const { return &edge - myEdge0; }
    EdgeLoop* GetLoopOf( const EdgePart* edge ) { return myLoopOfEdge[ Index( *edge )]; }
  };

  //--------------------------------------------------------------------------------
  /*!
   * \brief Intersections of a face
   */
  struct CutFace
  {
    mutable std::vector< EdgePart > myLinks;
    const SMDS_MeshElement*         myInitFace;

    CutFace( const SMDS_MeshElement* face ): myInitFace( face ) {}
    void AddEdge( const CutLink&          p1,
                  const CutLink&          p2,
                  const SMDS_MeshElement* cutter,
                  const int               nbOnPlane,
                  const int               iNotOnPlane = -1) const;
    void AddPoint( const CutLink& p1, const CutLink& p2, double tol ) const;
    bool ReplaceNodes( const TNNMap& theRm2KeepMap ) const;
    bool IsCut() const;
    int  NbInternalEdges() const;
    void MakeLoops( EdgeLoopSet& loops, const gp_XYZ& theFaceNorm ) const;
    bool RemoveInternalLoops( EdgeLoopSet& theLoops ) const;
    void CutOffLoops( EdgeLoopSet&                 theLoops,
                      const double                 theSign,
                      const std::vector< gp_XYZ >& theNormals,
                      std::vector< EdgePart >&     theCutOffLinks,
                      TLinkMap&                    theCutOffCoplanarLinks) const;
    void InitLinks() const;
    bool IsCoplanar( const EdgePart* edge ) const;

    static Standard_Integer HashCode(const CutFace& f, const Standard_Integer upper)
    {
      return ::HashCode( f.myInitFace->GetID(), upper );
    }
    static Standard_Boolean IsEqual(const CutFace& f1, const CutFace& f2 )
    {
      return f1.myInitFace == f2.myInitFace;
    }
    void Dump() const;

  private:

    EdgePart* getTwin( const EdgePart* edge ) const;
  };

  typedef NCollection_Map< CutFace, CutFace > TCutFaceMap;

  //--------------------------------------------------------------------------------
  /*!
   * \brief Intersection point of two edges of co-planar triangles
   */
  struct IntPoint2D
  {
    size_t        myEdgeInd[2]; //!< edge indices of triangles
    double        myU      [2]; //!< parameter [0,1] on edges of triangles
    SMESH_NodeXYZ myNode;       //!< intersection node
    bool          myIsCollinear;//!< edges are collinear

    IntPoint2D() : myIsCollinear( false ) {}

    void InitLink( CutLink& link, int iFace, const std::vector< SMESH_NodeXYZ >& nodes ) const
    {
      link.Set( nodes[  myEdgeInd[ iFace ]                      ].Node(),
                nodes[( myEdgeInd[ iFace ] + 1 ) % nodes.size() ].Node(),
                link.myFace );
      link.myIntNode = myNode;
    }
    const SMDS_MeshNode* Node() const { return myNode.Node(); }
  };
  struct IntPoint2DCompare
  {
    int myI;
    IntPoint2DCompare( int iFace=0 ): myI( iFace ) {}
    bool operator() ( const IntPoint2D* ip1, const IntPoint2D* ip2 ) const
    {
      return ip1->myU[ myI ] < ip2->myU[ myI ];
    }
    bool operator() ( const IntPoint2D& ip1, const IntPoint2D& ip2 ) const
    {
      return ip1.myU[ myI ] < ip2.myU[ myI ];
    }
  };
  typedef boost::container::flat_set< IntPoint2D, IntPoint2DCompare >  TIntPointSet;
  typedef boost::container::flat_set< IntPoint2D*, IntPoint2DCompare > TIntPointPtrSet;

  //--------------------------------------------------------------------------------
  /*!
   * \brief Face used to find translated position of the node
   */
  struct Face
  {
    const SMDS_MeshElement* myFace;
    SMESH_TNodeXYZ          myNode1; //!< nodes neighboring another node of myFace
    SMESH_TNodeXYZ          myNode2;
    const gp_XYZ*           myNorm;
    bool                    myNodeRightOrder;
    void operator=(const SMDS_MeshElement* f) { myFace = f; }
    const SMDS_MeshElement* operator->() { return myFace; }
    void SetNodes( int i0, int i1 ) //!< set myNode's
    {
      myNode1.Set( myFace->GetNode( i1 ));
      int i2 = ( i0 - 1 + myFace->NbCornerNodes() ) % myFace->NbCornerNodes();
      if ( i2 == i1 )
        i2 = ( i0 + 1 ) % myFace->NbCornerNodes();
      myNode2.Set( myFace->GetNode( i2 ));
      myNodeRightOrder = ( Abs( i2-i1 ) == 1 ) ?  i2 > i1  :  i2 < i1;
    }
    void SetOldNodes( const SMDS_Mesh& theSrcMesh )
    {
      myNode1.Set( theSrcMesh.FindNode( myNode1->GetID() ));
      myNode2.Set( theSrcMesh.FindNode( myNode2->GetID() ));
    }
    bool SetNormal( const std::vector< gp_XYZ >& faceNormals )
    {
      myNorm = & faceNormals[ myFace->GetID() ];
      return ( myNorm->SquareModulus() > gp::Resolution() * gp::Resolution() );
    }
    const gp_XYZ& Norm() const { return *myNorm; }
  };

  //--------------------------------------------------------------------------------
  /*!
   * \brief Offset plane used to find translated position of the node
   */
  struct OffsetPlane
  {
    gp_XYZ myNode;
    Face*  myFace;
    gp_Pln myPln;
    gp_Lin myLines[2]; //!< line of intersection with neighbor OffsetPlane's
    bool   myIsLineOk[2];
    double myWeight[2];

    void   Init( const gp_XYZ& node, Face& tria, double offset )
    {
      myNode = node;
      myFace = & tria;
      myPln  = gp_Pln( node + tria.Norm() * offset, tria.Norm() );
      myIsLineOk[0] = myIsLineOk[1] = false;
      myWeight[0] = myWeight[1] = 0;
    }
    bool   ComputeIntersectionLine( OffsetPlane& pln );
    void   SetSkewLine( const gp_Lin& line );
    gp_XYZ GetCommonPoint( int & nbOkPoints, double& sumWeight );
    gp_XYZ ProjectNodeOnLine( int & nbOkPoints );
    double Weight() const { return myWeight[0] + myWeight[1]; }
  };

  //================================================================================
  /*!
   * \brief Set the second line
   */
  //================================================================================

  void OffsetPlane::SetSkewLine( const gp_Lin& line )
  {
    myLines[1] = line;
    gp_XYZ n = myLines[0].Direction().XYZ() ^ myLines[1].Direction().XYZ();
    if (( myIsLineOk[1] = n.SquareModulus() > gp::Resolution() ))
      myPln = gp_Pln( myPln.Location(), n );
  }

  //================================================================================
  /*!
   * \brief Project myNode on myLine[0]
   */
  //================================================================================

  gp_XYZ OffsetPlane::ProjectNodeOnLine( int & nbOkPoints )
  {
    gp_XYZ p = gp::Origin().XYZ();
    if ( myIsLineOk[0] )
    {
      gp_Vec l2n( myLines[0].Location(), myNode );
      double u = l2n * myLines[0].Direction();
      p = myLines[0].Location().XYZ() + u * myLines[0].Direction().XYZ();
      ++nbOkPoints;
    }
    return p;
  }

  //================================================================================
  /*!
   * \brief Computes intersection point of myLines
   */
  //================================================================================

  gp_XYZ OffsetPlane::GetCommonPoint( int & nbOkPoints, double& sumWeight )
  {
    if ( !myIsLineOk[0] || !myIsLineOk[1] )
    {
      // sumWeight += myWeight[0];
      // return ProjectNodeOnLine( nbOkPoints ) * myWeight[0];
      return gp::Origin().XYZ();
    }

    gp_XYZ p;

    gp_Vec lPerp0 = myLines[0].Direction().XYZ() ^ myPln.Axis().Direction().XYZ();
    double  dot01 = lPerp0 * myLines[1].Direction().XYZ();
    if ( Abs( dot01 ) > 0.05 )
    {
      gp_Vec l0l1 = myLines[1].Location().XYZ() - myLines[0].Location().XYZ();
      double   u1 = - ( lPerp0 * l0l1 ) / dot01;
      p = ( myLines[1].Location().XYZ() + myLines[1].Direction().XYZ() * u1 );
    }
    else
    {
      gp_Vec  lv0( myLines[0].Location(), myNode),  lv1(myLines[1].Location(), myNode );
      double dot0( lv0 * myLines[0].Direction() ), dot1( lv1 * myLines[1].Direction() );
      p  = 0.5 * ( myLines[0].Location().XYZ() + myLines[0].Direction().XYZ() * dot0 );
      p += 0.5 * ( myLines[1].Location().XYZ() + myLines[1].Direction().XYZ() * dot1 );
    }

    sumWeight += Weight();
    ++nbOkPoints;

    return p * Weight();
  }

  //================================================================================
  /*!
   * \brief Compute line of intersection of 2 planes
   */
  //================================================================================

  bool OffsetPlane::ComputeIntersectionLine( OffsetPlane& theNextPln )
  {
    const gp_XYZ& n1 = myFace->Norm();
    const gp_XYZ& n2 = theNextPln.myFace->Norm();

    gp_XYZ lineDir = n1 ^ n2;
    gp_Pnt linePos;

    double x = Abs( lineDir.X() );
    double y = Abs( lineDir.Y() );
    double z = Abs( lineDir.Z() );

    int cooMax; // max coordinate
    if (x > y) {
      if (x > z) cooMax = 1;
      else       cooMax = 3;
    }
    else {
      if (y > z) cooMax = 2;
      else       cooMax = 3;
    }

    bool ok = true;
    if ( Abs( lineDir.Coord( cooMax )) < 0.05 )
    {
      // parallel planes - intersection is an offset of the common edge
      linePos  = 0.5 * ( myPln.Location().XYZ() + theNextPln.myPln.Location().XYZ() );
      lineDir  = myNode - myFace->myNode2;
      ok       = false;
      myWeight[0] = 0;
    }
    else
    {
      // the constants in the 2 plane equations
      double d1 = - ( n1 * myPln.Location().XYZ() );
      double d2 = - ( n2 * theNextPln.myPln.Location().XYZ() );

      switch ( cooMax ) {
      case 1:
        linePos.SetX(  0 );
        linePos.SetY(( d2*n1.Z() - d1*n2.Z()) / lineDir.X() );
        linePos.SetZ(( d1*n2.Y() - d2*n1.Y()) / lineDir.X() );
        break;
      case 2:
        linePos.SetX(( d1*n2.Z() - d2*n1.Z()) / lineDir.Y() );
        linePos.SetY(  0 );
        linePos.SetZ(( d2*n1.X() - d1*n2.X()) / lineDir.Y() );
        break;
      case 3:
        linePos.SetX(( d2*n1.Y() - d1*n2.Y()) / lineDir.Z() );
        linePos.SetY(( d1*n2.X() - d2*n1.X()) / lineDir.Z() );
        linePos.SetZ(  0 );
      }
      myWeight[0] = lineDir.SquareModulus();
      if ( n1 * n2 < 0 )
        myWeight[0] = 2. - myWeight[0];
    }
    myLines   [ 0 ].SetDirection( lineDir );
    myLines   [ 0 ].SetLocation ( linePos );
    myIsLineOk[ 0 ] = ok;

    theNextPln.myLines   [ 1 ] = myLines[ 0 ];
    theNextPln.myIsLineOk[ 1 ] = ok;
    theNextPln.myWeight  [ 1 ] = myWeight[ 0 ];

    return ok;
  }

  //================================================================================
  /*!
   * \brief Return a translated position of a node
   *  \param [in] new2OldNodes - new and old nodes
   *  \param [in] faceNormals - normals to input faces
   *  \param [in] theSrcMesh - initial mesh
   *  \param [in] theNewPos - a computed normal
   *  \return bool - true if theNewPos is computed
   */
  //================================================================================

  bool getTranslatedPosition( const SMDS_MeshNode*         theNewNode,
                              const double                 theOffset,
                              const double                 theTol,
                              const double                 theSign,
                              const std::vector< gp_XYZ >& theFaceNormals,
                              SMDS_Mesh&                   theSrcMesh,
                              gp_XYZ&                      theNewPos)
  {
    bool useOneNormal = true;

    // check if theNewNode needs an average position, i.e. theNewNode is convex
    // SMDS_ElemIteratorPtr faceIt = theNewNode->GetInverseElementIterator();
    // const SMDS_MeshElement*  f0 = faceIt->next();
    // const gp_XYZ&         norm0 = theFaceNormals[ f0->GetID() ];
    // const SMESH_NodeXYZ nodePos = theNewNode;
    // while ( faceIt->more() )
    // {
    //   const SMDS_MeshElement* f = faceIt->next();
    //   const int         nodeInd = f->GetNodeIndex( theNewNode );
    //   SMESH_NodeXYZ    nodePos2 = f->GetWrapNode( nodeInd + 1 );
    //   try {
    //     const gp_XYZ      nnDir = ( nodePos2 - nodePos ).Normalized();
    //   }
    //   catch {
    //     continue;
    //   }
    //   const double dot = norm0 * nnDir;
    //   bool isConvex = 



    // get faces surrounding theNewNode and sort them
    Face faces[ theMaxNbFaces ];
    SMDS_ElemIteratorPtr faceIt = theNewNode->GetInverseElementIterator();
    faces[0] = faceIt->next();
    while ( !faces[0].SetNormal( theFaceNormals ) && faceIt->more() )
      faces[0] = faceIt->next();
    int i0 = faces[0]->GetNodeIndex( theNewNode );
    int i1 = ( i0 + 1 ) % faces[0]->NbCornerNodes();
    faces[0].SetNodes( i0, i1 );
    TIDSortedElemSet elemSet, avoidSet;
    int iFace = 0;
    const SMDS_MeshElement* f;
    for ( ; faceIt->more() && iFace < theMaxNbFaces; faceIt->next() )
    {
      avoidSet.insert( faces[ iFace ].myFace );
      f = SMESH_MeshAlgos::FindFaceInSet( theNewNode, faces[ iFace ].myNode2.Node(),
                                          elemSet, avoidSet, &i0, &i1 );
      if ( !f )
      {
        std::reverse( &faces[0], &faces[0] + iFace + 1 );
        for ( int i = 0; i <= iFace; ++i )
        {
          std::swap( faces[i].myNode1, faces[i].myNode2 );
          faces[i].myNodeRightOrder = !faces[i].myNodeRightOrder;
        }
        f = SMESH_MeshAlgos::FindFaceInSet( theNewNode, faces[ iFace ].myNode2.Node(),
                                            elemSet, avoidSet, &i0, &i1 );
        if ( !f )
          break;
      }
      faces[ ++iFace ] = f;
      faces[ iFace ].SetNodes( i0, i1 );
      faces[ iFace ].SetNormal( theFaceNormals );
    }
    int nbFaces = iFace + 1;

    theNewPos.SetCoord( 0, 0, 0 );
    gp_XYZ oldXYZ = SMESH_NodeXYZ( theNewNode );

    // check if all faces are co-planar
    bool isPlanar = true;
    const double tol = 1e-2;
    for ( int i = 1; i < nbFaces &&  isPlanar;  ++i )
      isPlanar = ( faces[i].Norm() - faces[i-1].Norm() ).SquareModulus() < tol*tol;

    if ( isPlanar )
    {
      theNewPos = oldXYZ + faces[0].Norm() * theOffset;
      return useOneNormal;
    }

    // prepare OffsetPlane's
    OffsetPlane pln[ theMaxNbFaces ];
    for ( int i = 0; i < nbFaces; ++i )
    {
      faces[i].SetOldNodes( theSrcMesh );
      pln[i].Init( oldXYZ, faces[i], theOffset );
    }
    // intersect neighboring OffsetPlane's
    int nbOkPoints = 0;
    for ( int i = 1; i < nbFaces; ++i )
      nbOkPoints += pln[ i-1 ].ComputeIntersectionLine( pln[ i ]);
    nbOkPoints += pln[ nbFaces-1 ].ComputeIntersectionLine( pln[ 0 ]);

    // move intersection lines to over parallel planes
    if ( nbOkPoints > 1 )
      for ( int i = 0; i < nbFaces; ++i )
        if ( pln[i].myIsLineOk[0] && !pln[i].myIsLineOk[1] )
          for ( int j = 1; j < nbFaces &&  !pln[i].myIsLineOk[1]; ++j )
          {
            int i2 = ( i + j ) % nbFaces;
            if ( pln[i2].myIsLineOk[0] )
              pln[i].SetSkewLine( pln[i2].myLines[0] );
          }

    // get the translated position
    nbOkPoints = 0;
    double sumWegith = 0;
    const double minWeight = Sin( 30 * M_PI / 180. ) * Sin( 30 * M_PI / 180. );
    for ( int i = 0; i < nbFaces; ++i )
      if ( pln[ i ].Weight() > minWeight )
        theNewPos += pln[ i ].GetCommonPoint( nbOkPoints, sumWegith );

    if ( nbOkPoints == 0 )
    {
      // there is only one feature edge;
      // find the theNewPos by projecting oldXYZ to any intersection line
      for ( int i = 0; i < nbFaces; ++i )
        theNewPos += pln[ i ].ProjectNodeOnLine( nbOkPoints );

      if ( nbOkPoints == 0 )
      {
        theNewPos = oldXYZ + faces[0].Norm() * theOffset;
        return useOneNormal;
      }
      sumWegith = nbOkPoints;
    }
    theNewPos /= sumWegith;


    // mark theNewNode if it is concave
    useOneNormal = false;
    gp_Vec moveVec( oldXYZ, theNewPos );
    for ( int i = 0, iPrev = nbFaces-1; i < nbFaces; iPrev = i++ )
    {
      gp_Vec nodeVec( oldXYZ, faces[ i ].myNode1 );
      double u = ( moveVec * nodeVec ) / nodeVec.SquareMagnitude();
      if ( u > 0.5 ) // param [0,1] on nodeVec
      {
        theNewNode->setIsMarked( true );
      }
      if ( !useOneNormal )
      {
        gp_XYZ inFaceVec = faces[ i ].Norm() ^ nodeVec.XYZ();
        double       dot = inFaceVec * faces[ iPrev ].Norm();
        if ( !faces[ i ].myNodeRightOrder )
          dot *= -1;
        if ( dot * theSign < 0 )
        {
          gp_XYZ p1 = oldXYZ + faces[ i ].Norm()     * theOffset;
          gp_XYZ p2 = oldXYZ + faces[ iPrev ].Norm() * theOffset;
          useOneNormal = ( p1 - p2 ).SquareModulus() > 1e-12;
        }
      }
      if ( useOneNormal && theNewNode->isMarked() )
        break;
    }

    return useOneNormal;
  }

  //================================================================================
  /*!
   * \brief Remove small faces
   */
  //================================================================================

  void removeSmallFaces( SMDS_Mesh*                        theMesh,
                         SMESH_MeshAlgos::TElemIntPairVec& theNew2OldFaces,
                         const double                      theTol2 )
  {
    std::vector< SMESH_NodeXYZ > points(3);
    std::vector< const SMDS_MeshNode* > nodes(3);
    for ( SMDS_ElemIteratorPtr faceIt = theMesh->elementsIterator(); faceIt->more(); )
    {
      const SMDS_MeshElement* face = faceIt->next();
      points.assign( face->begin_nodes(), face->end_nodes() );

      SMESH_NodeXYZ* prevN = & points.back();
      for ( size_t i = 0; i < points.size(); ++i )
      {
        double dist2 = ( *prevN - points[ i ]).SquareModulus();
        if ( dist2 < theTol2 )
        {
          const SMDS_MeshNode* nToRemove =
            (*prevN)->GetID() > points[ i ]->GetID() ? prevN->Node() : points[ i ].Node();
          const SMDS_MeshNode* nToKeep =
            nToRemove == points[ i ].Node() ? prevN->Node() : points[ i ].Node();
          for ( SMDS_ElemIteratorPtr fIt = nToRemove->GetInverseElementIterator(); fIt->more(); )
          {
            const SMDS_MeshElement* f = fIt->next();
            if ( f == face )
              continue;
            nodes.assign( f->begin_nodes(), f->end_nodes() );
            nodes[ f->GetNodeIndex( nToRemove )] = nToKeep;
            theMesh->ChangeElementNodes( f, &nodes[0], nodes.size() );
          }
          theNew2OldFaces[ face->GetID() ].first = 0;
          theMesh->RemoveFreeElement( face );
          break;
        }
        prevN = & points[ i ];
      }
      continue;
    }
    return;
  }

} // namespace

namespace SMESH_MeshAlgos
{
  //--------------------------------------------------------------------------------
  /*!
   * \brief Intersect faces of a mesh
   */
  struct Intersector::Algo
  {
    SMDS_Mesh*                   myMesh;
    double                       myTol, myEps;
    const std::vector< gp_XYZ >& myNormals;
    TCutLinkMap                  myCutLinks; //!< assure sharing of new nodes
    TCutFaceMap                  myCutFaces;
    TNNMap                       myRemove2KeepNodes; //!< node merge map

    // data to intersect 2 faces
    const SMDS_MeshElement*      myFace1;
    const SMDS_MeshElement*      myFace2;
    std::vector< SMESH_NodeXYZ > myNodes1, myNodes2;
    std::vector< double >        myDist1,  myDist2;
    int                          myInd1, myInd2; // coordinate indices on an axis-aligned plane
    int                          myNbOnPlane1, myNbOnPlane2;
    TIntPointSet                 myIntPointSet;

    Algo( SMDS_Mesh* mesh, double tol, const std::vector< gp_XYZ >& normals )
      : myMesh( mesh ),
        myTol( tol ),
        myEps( 1e-100 ),
        //myEps( Sqrt( std::numeric_limits<double>::min() )),
        //myEps( gp::Resolution() ),
        myNormals( normals )
    {}
    void Cut( const SMDS_MeshElement* face1,
              const SMDS_MeshElement* face2,
              const int               nbCommonNodes );
    void Cut( const SMDS_MeshElement* face,
              SMESH_NodeXYZ&          lineEnd1,
              int                     edgeIndex1,
              SMESH_NodeXYZ&          lineEnd2,
              int                     edgeIndex2 );
    void MakeNewFaces( TElemIntPairVec& theNew2OldFaces,
                       TNodeIntPairVec& theNew2OldNodes,
                       const double     theSign,
                       const bool       theOptimize );

    void IntersectNewEdges( const CutFace& theCFace );

  private:

    bool isPlaneIntersected( const gp_XYZ&                       n2,
                             const double                        d2,
                             const std::vector< SMESH_NodeXYZ >& nodes1,
                             std::vector< double > &             dist1,
                             int &                               nbOnPlane1,
                             int &                               iNotOnPlane1);
    void computeIntervals( const std::vector< SMESH_NodeXYZ >& nodes,
                           const std::vector< double >&        dist,
                           const int                           nbOnPln,
                           const int                           iMaxCoo,
                           double *                            u,
                           int*                                iE);
    void cutCoplanar();
    void addLink ( CutLink& link );
    bool findLink( CutLink& link );
    bool coincide( const gp_XYZ& p1, const gp_XYZ& p2, const double tol ) const
    {
      return ( p1 - p2 ).SquareModulus() < tol * tol;
    }
    gp_XY p2D( const gp_XYZ& p ) const { return gp_XY( p.Coord( myInd1 ), p.Coord( myInd2 )); }

    void intersectLink( const std::vector< SMESH_NodeXYZ >& nodes1,
                        const std::vector< double > &       dist1,
                        const int                           iEdge1,
                        const SMDS_MeshElement*             face2,
                        CutLink&                            link1);
    void findIntPointOnPlane( const std::vector< SMESH_NodeXYZ >& nodes,
                              const std::vector< double > &       dist,
                              CutLink&                            link );
    void replaceIntNode( const SMDS_MeshNode* nToKeep, const SMDS_MeshNode* nToRemove );
    void computeIntPoint( const double           u1,
                          const double           u2,
                          const int              iE1,
                          const int              iE2,
                          CutLink &              link,
                          const SMDS_MeshNode* & node1,
                          const SMDS_MeshNode* & node2);
    void cutCollinearLink( const int                           iNotOnPlane1,
                           const std::vector< SMESH_NodeXYZ >& nodes1,
                           const SMDS_MeshElement*             face2,
                           const CutLink&                      link1,
                           const CutLink&                      link2);
    void setPlaneIndices( const gp_XYZ& planeNorm );
    bool intersectEdgeEdge( const gp_XY s1p0, const gp_XY s1p1,
                            const gp_XY s2p0, const gp_XY s2p1,
                            double &    t1,   double &    t2,
                            bool &      isCollinear  );
    bool intersectEdgeEdge( int iE1, int iE2, IntPoint2D& intPoint );
    bool isPointInTriangle( const gp_XYZ& p, const std::vector< SMESH_NodeXYZ >& nodes );
    const SMDS_MeshNode* createNode( const gp_XYZ& p );
  };

  //================================================================================
  /*!
   * \brief Return coordinate index with maximal abs value
   */
  //================================================================================

  int MaxIndex( const gp_XYZ& x )
  {
    int iMaxCoo = ( Abs( x.X()) < Abs( x.Y() )) + 1;
    if ( Abs( x.Coord( iMaxCoo )) < Abs( x.Z() ))
      iMaxCoo = 3;
    return iMaxCoo;
  }
  //================================================================================
  /*!
   * \brief Store a CutLink
   */
  //================================================================================

  const SMDS_MeshNode* Intersector::Algo::createNode( const gp_XYZ& p )
  {
    const SMDS_MeshNode* n = myMesh->AddNode( p.X(), p.Y(), p.Z() );
    n->setIsMarked( true ); // cut nodes are marked
    return n;
  }

  //================================================================================
  /*!
   * \brief Store a CutLink
   */
  //================================================================================

  void Intersector::Algo::addLink( CutLink& link )
  {
    link.myIndex = 0;
    const CutLink* added = & myCutLinks.Added( link );
    while ( added->myIntNode.Node() != link.myIntNode.Node() )
    {
      if ( !added->myIntNode )
      {
        added->myIntNode = link.myIntNode;
        break;
      }
      else
      {
        link.myIndex++;
        added = & myCutLinks.Added( link );
      }
    }
    link.myIndex = 0;
  }

  //================================================================================
  /*!
   * \brief Find a CutLink with an intersection point coincident with that of a given link
   */
  //================================================================================

  bool Intersector::Algo::findLink( CutLink& link )
  {
    link.myIndex = 0;
    while ( myCutLinks.Contains( link ))
    {
      const CutLink* added = & myCutLinks.Added( link );
      if ( !!added->myIntNode && coincide( added->myIntNode, link.myIntNode, myTol ))
      {
        link.myIntNode = added->myIntNode;
        return true;
      }
      link.myIndex++;
    }
    return false;
  }

  //================================================================================
  /*!
   * \brief Check if a triangle intersects the plane of another triangle
   *  \param [in] nodes1 - nodes of triangle 1
   *  \param [in] n2 - normal of triangle 2
   *  \param [in] d2 - a constant of the plane equation 2
   *  \param [out] dist1 - distance of nodes1 from the plane 2
   *  \param [out] nbOnPlane - number of nodes1 lying on the plane 2
   *  \return bool - true if the triangle intersects the plane 2
   */
  //================================================================================

  bool Intersector::Algo::isPlaneIntersected( const gp_XYZ&                       n2,
                                              const double                        d2,
                                              const std::vector< SMESH_NodeXYZ >& nodes1,
                                              std::vector< double > &             dist1,
                                              int &                               nbOnPlane1,
                                              int &                               iNotOnPlane1)
  {
    iNotOnPlane1 = nbOnPlane1 = 0;
    dist1.resize( nodes1.size() );
    for ( size_t i = 0; i < nodes1.size(); ++i )
    {
      dist1[i] = n2 * nodes1[i] + d2;
      if ( Abs( dist1[i] ) < myTol )
      {
        ++nbOnPlane1;
        dist1[i] = 0.;
      }
      else
      {
        iNotOnPlane1 = i;
      }
    }
    if ( nbOnPlane1 == 0 )
      for ( size_t i = 0; i < nodes1.size(); ++i )
        if ( dist1[iNotOnPlane1] * dist1[i] < 0 )
          return true;

    return nbOnPlane1;
  }

  //================================================================================
  /*!
   * \brief Compute parameters on the plane intersection line of intersections
   *        of edges of a triangle
   *  \param [in] nodes - triangle nodes
   *  \param [in] dist - distance of triangle nodes from the plane of another triangle
   *  \param [in] nbOnPln - number of nodes lying on the plane of another triangle
   *  \param [in] iMaxCoo - index of coordinate of max component of the plane intersection line
   *  \param [out] u - two computed parameters on the plane intersection line
   *  \param [out] iE - indices of intersected edges
   */
  //================================================================================

  void Intersector::Algo::computeIntervals( const std::vector< SMESH_NodeXYZ >& nodes,
                                            const std::vector< double >&        dist,
                                            const int                           nbOnPln,
                                            const int                           iMaxCoo,
                                            double *                            u,
                                            int*                                iE)
  {
    if ( nbOnPln == 3 )
    {
      u[0] = u[1] = 1e+100;
      return;
    }
    int nb = 0;
    int i1 = 2, i2 = 0;
    if ( nbOnPln == 1 && ( dist[i1] == 0. || dist[i2] == 0 ))
    {
      int i = dist[i1] == 0 ? i1 : i2;
      u [ 1 ] = nodes[ i ].Coord( iMaxCoo );
      iE[ 1 ] = i;
      i1 = i2++;
    }
    for ( ; i2 < 3 && nb < 2;  i1 = i2++ )
    {
      double dd = dist[i1] - dist[i2];
      if ( dd != 0. && dist[i2] * dist[i1] <= 0. )
      {
        double x1 = nodes[i1].Coord( iMaxCoo );
        double x2 = nodes[i2].Coord( iMaxCoo );
        u [ nb ] = x1 + ( x2 - x1 ) * dist[i1] / dd;
        iE[ nb ] = i1;
        ++nb;
      }
    }
    if ( u[0] > u[1] )
    {
      std::swap( u [0], u [1] );
      std::swap( iE[0], iE[1] );
    }
  }

  //================================================================================
  /*!
   * \brief Try to find an intersection node on a link collinear with the plane intersection line
   */
  //================================================================================

  void Intersector::Algo::findIntPointOnPlane( const std::vector< SMESH_NodeXYZ >& nodes,
                                               const std::vector< double > &       dist,
                                               CutLink&                            link )
  {
    int i1 = ( dist[0] == 0 ? 0 : 1 ), i2 = ( dist[2] == 0 ? 2 : 1 );
    CutLink link2 = link;
    link2.Set( nodes[i1].Node(), nodes[i2].Node(), 0 );
    if ( findLink( link2 ))
      link.myIntNode = link2.myIntNode;
  }

  //================================================================================
  /*!
   * \brief Compute intersection point of a link1 with a face2
   */
  //================================================================================

  void Intersector::Algo::intersectLink( const std::vector< SMESH_NodeXYZ >& nodes1,
                                         const std::vector< double > &       dist1,
                                         const int                           iEdge1,
                                         const SMDS_MeshElement*             face2,
                                         CutLink&                            link1)
  {
    const int iEdge2 = ( iEdge1 + 1 ) % nodes1.size();
    const SMESH_NodeXYZ& p1 = nodes1[ iEdge1 ];
    const SMESH_NodeXYZ& p2 = nodes1[ iEdge2 ];

    link1.Set( p1.Node(), p2.Node(), face2 );
    const CutLink* link = & myCutLinks.Added( link1 );
    if ( !link->IntNode() )
    {
      if      ( dist1[ iEdge1 ] == 0. ) link1.myIntNode = p1;
      else if ( dist1[ iEdge2 ] == 0. ) link1.myIntNode = p2;
      else
      {
        gp_XYZ p = p1 + ( p2 - p1 ) * dist1[ iEdge1 ] / ( dist1[ iEdge1 ] - dist1[ iEdge2 ]);
        (gp_XYZ&)link1.myIntNode = p;
      }
    }
    else
    {
      gp_XYZ p = p1 + ( p2 - p1 ) * dist1[ iEdge1 ] / ( dist1[ iEdge1 ] - dist1[ iEdge2 ]);
      while ( link->IntNode() )
      {
        if ( coincide( p, link->myIntNode, myTol ))
        {
          link1.myIntNode = link->myIntNode;
          break;
        }
        link1.myIndex++;
        link = & myCutLinks.Added( link1 );
      }
      if ( !link1.IntNode() )
      {
        if      ( dist1[ iEdge1 ] == 0. ) link1.myIntNode = p1;
        else if ( dist1[ iEdge2 ] == 0. ) link1.myIntNode = p2;
        else                     (gp_XYZ&)link1.myIntNode = p;
      }
    }
  }

  //================================================================================
  /*!
   * \brief Store node replacement in myCutFaces
   */
  //================================================================================

  void Intersector::Algo::replaceIntNode( const SMDS_MeshNode* nToKeep,
                                          const SMDS_MeshNode* nToRemove )
  {
    if ( nToKeep == nToRemove )
      return;
    if ( nToRemove->GetID() < nToKeep->GetID() ) // keep node with lower ID
      myRemove2KeepNodes.Bind( nToKeep, nToRemove );
    else
      myRemove2KeepNodes.Bind( nToRemove, nToKeep );
  }

  //================================================================================
  /*!
   * \brief Compute intersection point on a link of either of faces by choosing
   *        a link whose parameter on the intersection line in maximal
   *  \param [in] u1 - parameter on the intersection line of link iE1 of myFace1
   *  \param [in] u2 - parameter on the intersection line of link iE2 of myFace2
   *  \param [in] iE1 - index of a link myFace1
   *  \param [in] iE2 - index of a link myFace2
   *  \param [out] link - CutLink storing the intersection point
   *  \param [out] node1 - a node of the 2nd link if two links intersect
   *  \param [out] node2 - a node of the 2nd link if two links intersect
   */
  //================================================================================

  void Intersector::Algo::computeIntPoint( const double           u1,
                                           const double           u2,
                                           const int              iE1,
                                           const int              iE2,
                                           CutLink &              link,
                                           const SMDS_MeshNode* & node1,
                                           const SMDS_MeshNode* & node2)
  {
    if      ( u1 > u2 + myTol )
    {
      intersectLink( myNodes1, myDist1, iE1, myFace2, link );
      node1 = node2 = 0;
      if ( myNbOnPlane2 == 2 )
        findIntPointOnPlane( myNodes2, myDist2, link );
    }
    else if ( u2 > u1 + myTol )
    {
      intersectLink( myNodes2, myDist2, iE2, myFace1, link );
      node1 = node2 = 0;
      if ( myNbOnPlane1 == 2 )
        findIntPointOnPlane( myNodes1, myDist1, link );
    }
    else // edges of two faces intersect the line at the same point
    {
      CutLink link2;
      intersectLink( myNodes1, myDist1, iE1, myFace2, link );
      intersectLink( myNodes2, myDist2, iE2, myFace1, link2 );
      node1 = link2.Node1();
      node2 = link2.Node2();

      if      ( !link.IntNode() && link2.IntNode() )
        link.myIntNode = link2.myIntNode;

      else if ( !link.IntNode() && !link2.IntNode() )
        (gp_XYZ&)link.myIntNode = 0.5 * ( link.myIntNode + link2.myIntNode );

      else if ( link.IntNode() && link2.IntNode() )
        replaceIntNode( link.IntNode(), link2.IntNode() );
    }
  }

  //================================================================================
  /*!
   * \brief Add intersections to a link collinear with the intersection line
   */
  //================================================================================

  void Intersector::Algo::cutCollinearLink( const int                           iNotOnPlane1,
                                            const std::vector< SMESH_NodeXYZ >& nodes1,
                                            const SMDS_MeshElement*             face2,
                                            const CutLink&                      link1,
                                            const CutLink&                      link2)

  {
    int iN1 = ( iNotOnPlane1 + 1 ) % 3;
    int iN2 = ( iNotOnPlane1 + 2 ) % 3;
    CutLink link( nodes1[ iN1 ].Node(), nodes1[ iN2 ].Node(), face2 );
    if ( link1.myFace != face2 )
    {
      link.myIntNode = link1.myIntNode;
      addLink( link );
    }
    if ( link2.myFace != face2 )
    {
      link.myIntNode = link2.myIntNode;
      addLink( link );
    }
  }

  //================================================================================
  /*!
   * \brief Choose indices on an axis-aligned plane
   */
  //================================================================================

  void Intersector::Algo::setPlaneIndices( const gp_XYZ& planeNorm )
  {
    switch ( MaxIndex( planeNorm )) {
    case 1: myInd1 = 2; myInd2 = 3; break;
    case 2: myInd1 = 3; myInd2 = 1; break;
    case 3: myInd1 = 1; myInd2 = 2; break;
    }
  }

  //================================================================================
  /*!
   * \brief Intersect two faces
   */
  //================================================================================

  void Intersector::Algo::Cut( const SMDS_MeshElement* face1,
                               const SMDS_MeshElement* face2,
                               const int               nbCommonNodes)
  {
    myFace1 = face1;
    myFace2 = face2;
    myNodes1.assign( face1->begin_nodes(), face1->end_nodes() );
    myNodes2.assign( face2->begin_nodes(), face2->end_nodes() );

    const gp_XYZ& n1 = myNormals[ face1->GetID() ];
    const gp_XYZ& n2 = myNormals[ face2->GetID() ];

    // check if triangles intersect
    int iNotOnPlane1, iNotOnPlane2;
    const double d2 = -( n2 * myNodes2[0]);
    if ( !isPlaneIntersected( n2, d2, myNodes1, myDist1, myNbOnPlane1, iNotOnPlane1 ))
      return;
    const double d1 = -( n1 * myNodes1[0]);
    if ( !isPlaneIntersected( n1, d1, myNodes2, myDist2, myNbOnPlane2, iNotOnPlane2 ))
      return;

    if ( myNbOnPlane1 == 3 || myNbOnPlane2 == 3 )// triangles are co-planar
    {
      setPlaneIndices( myNbOnPlane1 == 3 ? n2 : n1 ); // choose indices on an axis-aligned plane
      cutCoplanar();
    }
    else if ( nbCommonNodes < 2 ) // triangle planes intersect
    {
      gp_XYZ lineDir = n1 ^ n2; // intersection line

      // check if intervals of intersections of triangles with lineDir overlap

      double u1[2], u2 [2]; // parameters on lineDir of edge intersection points { minU, maxU }
      int   iE1[2], iE2[2]; // indices of edges
      int iMaxCoo = MaxIndex( lineDir );
      computeIntervals( myNodes1, myDist1, myNbOnPlane1, iMaxCoo, u1, iE1 );
      computeIntervals( myNodes2, myDist2, myNbOnPlane2, iMaxCoo, u2, iE2 );
      if ( u1[1] < u2[0] - myTol || u2[1] < u1[0] - myTol )
        return; // intervals do not overlap

      // make intersection nodes

      const SMDS_MeshNode *l1n1, *l1n2, *l2n1, *l2n2;
      CutLink link1; // intersection with smaller u on lineDir
      computeIntPoint( u1[0], u2[0], iE1[0], iE2[0], link1, l1n1, l1n2 );
      CutLink link2; // intersection with larger u on lineDir
      computeIntPoint( -u1[1], -u2[1], iE1[1], iE2[1], link2, l2n1, l2n2 );

      const CutFace& cf1 = myCutFaces.Added( CutFace( face1 ));
      const CutFace& cf2 = myCutFaces.Added( CutFace( face2 ));

      if ( coincide( link1.myIntNode, link2.myIntNode, myTol ))
      {
        // intersection is a point
        if ( link1.IntNode() && link2.IntNode() )
          replaceIntNode( link1.IntNode(), link2.IntNode() );

        CutLink* link = link2.IntNode() ? &link2 : &link1;
        if ( !link->IntNode() )
        {
          gp_XYZ p = 0.5 * ( link1.myIntNode + link2.myIntNode );
          link->myIntNode.Set( createNode( p ));
        }
        if ( !link1.IntNode() ) link1.myIntNode = link2.myIntNode;
        if ( !link2.IntNode() ) link2.myIntNode = link1.myIntNode;

        cf1.AddPoint( link1, link2, myTol );
        if ( l1n1 ) link1.Set( l1n1, l1n2, face2 );
        if ( l2n1 ) link2.Set( l2n1, l2n2, face2 );
        cf2.AddPoint( link1, link2, myTol );
      }
      else
      {
        // intersection is a line segment
        if ( !link1.IntNode() )
          link1.myIntNode.Set( createNode( link1.myIntNode ));
        if ( !link2.IntNode() )
          link2.myIntNode.Set( createNode( link2.myIntNode ));

        cf1.AddEdge( link1, link2, face2, myNbOnPlane1, iNotOnPlane1 );
        if ( l1n1 ) link1.Set( l1n1, l1n2, face2 );
        if ( l2n1 ) link2.Set( l2n1, l2n2, face2 );
        cf2.AddEdge( link1, link2, face1, myNbOnPlane2, iNotOnPlane2 );

        // add intersections to a link collinear with the intersection line
        if ( myNbOnPlane1 == 2 && ( link1.myFace != face2 || link2.myFace != face2 ))
          cutCollinearLink( iNotOnPlane1, myNodes1, face2, link1, link2 );

        if ( myNbOnPlane2 == 2 && ( link1.myFace != face1 || link2.myFace != face1 ))
          cutCollinearLink( iNotOnPlane2, myNodes2, face1, link1, link2 );
      }

      addLink( link1 );
      addLink( link2 );

    } // non co-planar case

    return;
  }

  //================================================================================
  /*!
   * \brief Store a face cut by a line given by its ends
   *        accompanied by indices of intersected face edges.
   *        Edge index is <0 if a line end is inside the face.
   *  \param [in] face - a face to cut
   *  \param [inout] lineEnd1 - line end coordinates + optional node existing at this point
   *  \param [in] edgeIndex1 - index of face edge cut by lineEnd1
   *  \param [inout] lineEnd2 - line end coordinates + optional node existing at this point
   *  \param [in] edgeIndex2 - index of face edge cut by lineEnd2
   */
  //================================================================================

  void Intersector::Algo::Cut( const SMDS_MeshElement* face,
                               SMESH_NodeXYZ&          lineEnd1,
                               int                     edgeIndex1,
                               SMESH_NodeXYZ&          lineEnd2,
                               int                     edgeIndex2 )
  {
    if ( lineEnd1.Node() && lineEnd2.Node() &&
         face->GetNodeIndex( lineEnd1.Node() ) >= 0 &&
         face->GetNodeIndex( lineEnd2.Node() ) >= 0 )
      return; // intersection at a face node or edge

    if ((int) myNormals.size() <= face->GetID() )
      const_cast< std::vector< gp_XYZ >& >( myNormals ).resize( face->GetID() + 1 );

    const CutFace& cf = myCutFaces.Added( CutFace( face ));
    cf.InitLinks();

    // look for intersection nodes coincident with line ends
    CutLink links[2];
    for ( int is2nd = 0; is2nd < 2; ++is2nd )
    {
      SMESH_NodeXYZ& lineEnd = is2nd ? lineEnd2 : lineEnd1;
      int          edgeIndex = is2nd ? edgeIndex2 : edgeIndex1;
      CutLink &         link = links[ is2nd ];

      link.myIntNode = lineEnd;

      for ( size_t i = ( edgeIndex < 0 ? 3 : 0  ); i < cf.myLinks.size(); ++i )
        if ( coincide( lineEnd, SMESH_NodeXYZ( cf.myLinks[i].myNode1 ), myTol ))
        {
          link.myIntNode = cf.myLinks[i].myNode1;
          break;
        }

      if ( edgeIndex >= 0 )
      {
        link.Set( face->GetNode    ( edgeIndex ),
                  face->GetNodeWrap( edgeIndex + 1 ),
                  /*cuttingFace=*/0);
        findLink( link );
      }

      if ( !link.myIntNode )
        link.myIntNode.Set( createNode( lineEnd ));

      lineEnd._node = link.IntNode();

      if ( link.myNode[0] )
        addLink( link );
    }

    cf.AddEdge( links[0], links[1], /*face=*/0, /*nbOnPlane=*/0, /*iNotOnPlane=*/-1 );
  }

  //================================================================================
  /*!
   * \brief Intersect two 2D line segments
   */
  //================================================================================

  bool Intersector::Algo::intersectEdgeEdge( const gp_XY s1p0, const gp_XY s1p1,
                                             const gp_XY s2p0, const gp_XY s2p1,
                                             double &    t1,   double &    t2,
                                             bool &      isCollinear )
  {
    gp_XY u = s1p1 - s1p0;
    gp_XY v = s2p1 - s2p0;
    gp_XY w = s1p0 - s2p0;
    double perpDotUV = u * gp_XY( -v.Y(), v.X() );
    double perpDotVW = v * gp_XY( -w.Y(), w.X() );
    double perpDotUW = u * gp_XY( -w.Y(), w.X() );
    double        u2 = u.SquareModulus();
    double        v2 = v.SquareModulus();
    if ( u2 < myEps * myEps || v2 < myEps * myEps )
      return false;
    if ( perpDotUV * perpDotUV / u2 / v2 < 1e-6 ) // cos ^ 2
    {
      if ( !isCollinear )
        return false; // no need in collinear solution
      if ( perpDotUW * perpDotUW / u2 > myTol * myTol )
        return false; // parallel

      // collinear
      gp_XY w2 = s1p1 - s2p0;
      if ( Abs( v.X()) + Abs( u.X()) > Abs( v.Y()) + Abs( u.Y())) {
        t1 = w.X() / v.X();  // params on segment 2
        t2 = w2.X() / v.X();
      }
      else {
        t1 = w.Y() / v.Y();
        t2 = w2.Y() / v.Y();
      }
      if ( Max( t1,t2 ) <= 0 || Min( t1,t2 ) >= 1 )
        return false; // no overlap
      return true;
    }
    isCollinear = false;

    t1 = perpDotVW / perpDotUV; // param on segment 1
    if ( t1 < 0. || t1 > 1. )
      return false; // intersection not within the segment

    t2 = perpDotUW / perpDotUV; // param on segment 2
    if ( t2 < 0. || t2 > 1. )
      return false; // intersection not within the segment

    return true;
  }

  //================================================================================
  /*!
   * \brief Intersect two edges of co-planar triangles
   *  \param [inout] iE1 - edge index of triangle 1
   *  \param [inout] iE2 - edge index of triangle 2
   *  \param [inout] intPoints - intersection points
   *  \param [inout] nbIntPoints - nb of found intersection points
   */
  //================================================================================

  bool Intersector::Algo::intersectEdgeEdge( int iE1, int iE2, IntPoint2D& intPoint )
  {
    int i01 = iE1, i11 = ( iE1 + 1 ) % 3;
    int i02 = iE2, i12 = ( iE2 + 1 ) % 3;
    if (( !intPoint.myIsCollinear ) &&
        ( myNodes1[ i01 ] == myNodes2[ i02 ] ||
          myNodes1[ i01 ] == myNodes2[ i12 ] ||
          myNodes1[ i11 ] == myNodes2[ i02 ] ||
          myNodes1[ i11 ] == myNodes2[ i12 ] ))
      return false;

    // segment 1
    gp_XY s1p0 = p2D( myNodes1[ i01 ]);
    gp_XY s1p1 = p2D( myNodes1[ i11 ]);

    // segment 2
    gp_XY s2p0 = p2D( myNodes2[ i02 ]);
    gp_XY s2p1 = p2D( myNodes2[ i12 ]);

    double t1, t2;
    if ( !intersectEdgeEdge( s1p0,s1p1, s2p0,s2p1, t1, t2, intPoint.myIsCollinear ))
      return false;

    intPoint.myEdgeInd[0] = iE1;
    intPoint.myEdgeInd[1] = iE2;
    intPoint.myU[0] = t1;
    intPoint.myU[1] = t2;
    (gp_XYZ&)intPoint.myNode = myNodes1[i01] * ( 1 - t1 ) + myNodes1[i11] * t1;

    if ( intPoint.myIsCollinear )
      return true;

    // try to find existing node at intPoint.myNode

    if ( myNodes1[ i01 ] == myNodes2[ i02 ] ||
         myNodes1[ i01 ] == myNodes2[ i12 ] ||
         myNodes1[ i11 ] == myNodes2[ i02 ] ||
         myNodes1[ i11 ] == myNodes2[ i12 ] )
      return false;

    const double coincTol = myTol * 1e-3;

    CutLink link1( myNodes1[i01].Node(), myNodes1[i11].Node(), myFace2 );
    CutLink link2( myNodes2[i02].Node(), myNodes2[i12].Node(), myFace1 );

    SMESH_NodeXYZ& n1 = myNodes1[ t1 < 0.5 ? i01 : i11 ];
    bool same1 = coincide( n1, intPoint.myNode, coincTol );
    if ( same1 )
    {
      link2.myIntNode = intPoint.myNode = n1;
      addLink( link2 );
    }
    SMESH_NodeXYZ& n2 = myNodes2[ t2 < 0.5 ? i02 : i12 ];
    bool same2 = coincide( n2, intPoint.myNode, coincTol );
    if ( same2 )
    {
      link1.myIntNode = intPoint.myNode = n2;
      addLink( link1 );
      if ( same1 )
      {
        replaceIntNode( n1.Node(), n2.Node() );
        return false;
      }
      return true;
    }
    if ( same1 )
      return true;

    link1.myIntNode = intPoint.myNode;
    if ( findLink( link1 ))
    {
      intPoint.myNode = link2.myIntNode = link1.myIntNode;
      addLink( link2 );
      return true;
    }

    link2.myIntNode = intPoint.myNode;
    if ( findLink( link2 ))
    {
      intPoint.myNode = link1.myIntNode = link2.myIntNode;
      addLink( link1 );
      return true;
    }

    for ( int is2nd = 0; is2nd < 2; ++is2nd )
    {
      const SMDS_MeshElement* f = is2nd ? myFace1 : myFace2;
      if ( !f ) continue;
      const CutFace& cf = myCutFaces.Added( CutFace( is2nd ? myFace2 : myFace1 ));
      for ( size_t i = 0; i < cf.myLinks.size(); ++i )
        if ( cf.myLinks[i].myFace == f &&
             //cf.myLinks[i].myIndex != EdgePart::_COPLANAR &&
             coincide( intPoint.myNode, SMESH_NodeXYZ( cf.myLinks[i].myNode1 ), coincTol ))
        {
          intPoint.myNode.Set( cf.myLinks[i].myNode1 );
          return true;
        }
    }

    // make a new node

    intPoint.myNode._node = createNode( intPoint.myNode );
    link1.myIntNode = link2.myIntNode = intPoint.myNode;
    addLink( link1 );
    addLink( link2 );

    return true;
  }


  //================================================================================
  /*!
   * \brief Check if a point is contained in a triangle
   */
  //================================================================================

  bool Intersector::Algo::isPointInTriangle( const gp_XYZ& p, const std::vector< SMESH_NodeXYZ >& nodes )
  {
    double bc1, bc2;
    SMESH_MeshAlgos::GetBarycentricCoords( p2D( p ),
                                           p2D( nodes[0] ), p2D( nodes[1] ), p2D( nodes[2] ),
                                           bc1, bc2 );
    //return ( 0. < bc1 && 0. < bc2 && bc1 + bc2 < 1. );
    return ( myTol < bc1 && myTol < bc2 && bc1 + bc2 + myTol < 1. );
  }

  //================================================================================
  /*!
   * \brief Intersect two co-planar faces
   */
  //================================================================================

  void Intersector::Algo::cutCoplanar()
  {
    // find intersections of edges

    IntPoint2D intPoints[ 6 ];
    int      nbIntPoints = 0;
    for ( int iE1 = 0; iE1 < 3; ++iE1 )
    {
      int maxNbIntPoints = nbIntPoints + 2;
      for ( int iE2 = 0; iE2 < 3 &&  nbIntPoints < maxNbIntPoints; ++iE2 )
        nbIntPoints += intersectEdgeEdge( iE1, iE2, intPoints[ nbIntPoints ]);
    }
    const int minNbOnPlane = Min( myNbOnPlane1, myNbOnPlane2 );

    if ( nbIntPoints == 0 ) // no intersections of edges
    {
      bool is1in2;
      if      ( isPointInTriangle( myNodes1[0], myNodes2 )) // face2 includes face1
        is1in2 = true;
      else if ( isPointInTriangle( myNodes2[0], myNodes1 )) // face1 includes face2
        is1in2 = false;
      else
        return;

      // add edges of an inner triangle to an outer one

      const std::vector< SMESH_NodeXYZ >& nodesIn = is1in2 ? myNodes1 : myNodes2;
      const SMDS_MeshElement*             faceOut = is1in2 ? myFace2  : myFace1;
      const SMDS_MeshElement*              faceIn = is1in2 ? myFace1  : myFace2;

      const CutFace& outFace = myCutFaces.Added( CutFace( faceOut ));
      CutLink link1( nodesIn.back().Node(), nodesIn.back().Node(), faceOut );
      CutLink link2( nodesIn.back().Node(), nodesIn.back().Node(), faceOut );

      link1.myIntNode = nodesIn.back();
      for ( size_t i = 0; i < nodesIn.size(); ++i )
      {
        link2.myIntNode = nodesIn[ i ];
        outFace.AddEdge( link1, link2, faceIn, minNbOnPlane );
        link1.myIntNode = link2.myIntNode;
      }
    }
    else
    {
      // add parts of edges to a triangle including them

      CutLink link1, link2;
      IntPoint2D ip0, ip1;
      ip0.myU[0] = ip0.myU[1] = 0.;
      ip1.myU[0] = ip1.myU[1] = 1.;
      ip0.myEdgeInd[0] = ip0.myEdgeInd[1] = ip1.myEdgeInd[0] = ip1.myEdgeInd[1] = 0;

      for ( int isFromFace1 = 0; isFromFace1 < 2; ++isFromFace1 )
      {
        const SMDS_MeshElement*                faceTo = isFromFace1 ? myFace2  : myFace1;
        const SMDS_MeshElement*              faceFrom = isFromFace1 ? myFace1  : myFace2;
        const std::vector< SMESH_NodeXYZ >&   nodesTo = isFromFace1 ? myNodes2 : myNodes1;
        const std::vector< SMESH_NodeXYZ >& nodesFrom = isFromFace1 ? myNodes1 : myNodes2;
        const int                                 iTo = isFromFace1 ? 1 : 0;
        const int                               iFrom = isFromFace1 ? 0 : 1;
        //const int                       nbOnPlaneFrom = isFromFace1 ? myNbOnPlane1 : myNbOnPlane2;

        const CutFace* cutFaceTo   = & myCutFaces.Added( CutFace( faceTo ));
        // const CutFace* cutFaceFrom = 0;
        // if ( nbOnPlaneFrom > minNbOnPlane )
        //   cutFaceFrom = & myCutFaces.Added( CutFace( faceTo ));

        link1.myFace = link2.myFace = faceTo;

        IntPoint2DCompare ipCompare( iFrom );
        TIntPointPtrSet pointsOnEdge( ipCompare ); // IntPoint2D sorted by parameter on edge

        for ( size_t iE = 0; iE < nodesFrom.size(); ++iE )
        {
          // get parts of an edge iE

          ip0.myEdgeInd[ iTo ] = iE;
          ip1.myEdgeInd[ iTo ] = ( iE + 1 ) % nodesFrom.size();
          ip0.myNode = nodesFrom[ ip0.myEdgeInd[ iTo ]];
          ip1.myNode = nodesFrom[ ip1.myEdgeInd[ iTo ]];

          pointsOnEdge.clear();

          for ( int iP = 0; iP < nbIntPoints; ++iP )
            if ( intPoints[ iP ].myEdgeInd[ iFrom ] == iE )
              pointsOnEdge.insert( & intPoints[ iP ] );

          pointsOnEdge.insert( pointsOnEdge.begin(), & ip0 );
          pointsOnEdge.insert( pointsOnEdge.end(),   & ip1 );

          // add edge parts to faceTo

          TIntPointPtrSet::iterator ipIt = pointsOnEdge.begin() + 1;
          for ( ; ipIt != pointsOnEdge.end(); ++ipIt )
          {
            const IntPoint2D* p1 = *(ipIt-1);
            const IntPoint2D* p2 = *ipIt;
            gp_XYZ middle = 0.5 * ( p1->myNode + p2->myNode );
            if ( isPointInTriangle( middle, nodesTo ))
            {
              p1->InitLink( link1, iTo, ( p1 != & ip0 ) ? nodesTo : nodesFrom );
              p2->InitLink( link2, iTo, ( p2 != & ip1 ) ? nodesTo : nodesFrom );
              cutFaceTo->AddEdge( link1, link2, faceFrom, minNbOnPlane );

              // if ( cutFaceFrom )
              // {
              //   p1->InitLink( link1, iFrom, nodesFrom );
              //   p2->InitLink( link2, iFrom, nodesFrom );
              //   cutFaceTo->AddEdge( link1, link2, faceTo, minNbOnPlane );
              // }
            }
          }
        }
      }
    }
    return;

  } // Intersector::Algo::cutCoplanar()

  //================================================================================
  /*!
   * \brief Intersect edges added to myCutFaces
   */
  //================================================================================

  void Intersector::Algo::IntersectNewEdges( const CutFace& cf )
  {
    IntPoint2D intPoint;

    if ( cf.NbInternalEdges() < 2 )
      return;

    if ( myNodes1.empty() )
    {
      myNodes1.resize(2);
      myNodes2.resize(2);
    }

    const gp_XYZ& faceNorm = myNormals[ cf.myInitFace->GetID() ];
    setPlaneIndices( faceNorm ); // choose indices on an axis-aligned plane

    size_t limit = cf.myLinks.size() * cf.myLinks.size() * 2;

    size_t i1 = 3;
    while ( cf.myLinks[i1-1].IsInternal() && i1 > 0 )
      --i1;

    for ( ; i1 < cf.myLinks.size(); ++i1 )
    {
      if ( !cf.myLinks[i1].IsInternal() )
        continue;

      myIntPointSet.clear();
      for ( size_t i2 = i1 + 2; i2 < cf.myLinks.size(); ++i2 )
      {
        if ( !cf.myLinks[i2].IsInternal() )
          continue;

        // prepare to intersection
        myFace1     = cf.myLinks[i1].myFace;
        myNodes1[0] = cf.myLinks[i1].myNode1;
        myNodes1[1] = cf.myLinks[i1].myNode2;
        myFace2     = cf.myLinks[i2].myFace;
        myNodes2[0] = cf.myLinks[i2].myNode1;
        myNodes2[1] = cf.myLinks[i2].myNode2;

        // intersect
        intPoint.myIsCollinear = true; // to find collinear solutions
        if ( intersectEdgeEdge( 0, 0, intPoint ))
        {
          if ( cf.myLinks[i1].IsSame( cf.myLinks[i2] )) // remove i2
          {
            cf.myLinks[i1].ReplaceCoplanar( cf.myLinks[i2] );
            cf.myLinks.erase( cf.myLinks.begin() + i2, cf.myLinks.begin() + i2 + 2 );
            --i2;
            continue;
          }
          if ( !intPoint.myIsCollinear )
          {
            intPoint.myEdgeInd[1] = i2;
            myIntPointSet.insert( intPoint );
          }
          else // if ( intPoint.myIsCollinear ) // overlapping edges
          {
            myIntPointSet.clear(); // to recompute

            if ( intPoint.myU[0] > intPoint.myU[1] ) // orient in same direction
            {
              std::swap( intPoint.myU[0], intPoint.myU[1] );
              std::swap( myNodes1[0], myNodes1[1] );
            }
            // replace _COPLANAR by _INTERNAL
            cf.myLinks[i1].ReplaceCoplanar( cf.myLinks[i1+1] );
            cf.myLinks[i2].ReplaceCoplanar( cf.myLinks[i2+1] );

            if ( coincide( myNodes1[0], myNodes2[0], myTol ) &&
                 coincide( myNodes1[1], myNodes2[1], myTol ))
            {
              cf.myLinks.erase( cf.myLinks.begin() + i2, cf.myLinks.begin() + i2 + 2 );
              --i2;
              continue;
            }

            EdgePart common = cf.myLinks[i1];
            common.ReplaceCoplanar( cf.myLinks[i2] );

            const SMDS_MeshNode* n1 = myNodes1[0].Node(); // end nodes of an overlapping part
            const SMDS_MeshNode* n2 = myNodes1[1].Node();
            size_t i3 = cf.myLinks.size();

            if ( myNodes1[0] != myNodes2[0] ) // a part before the overlapping one
            {
              if ( intPoint.myU[0] < 0 )
                cf.myLinks[i1].Set( myNodes1[0].Node(), myNodes2[0].Node(),
                                    cf.myLinks[i1].myFace, cf.myLinks[i1].myIndex );
              else
                cf.myLinks[i1].Set( myNodes2[0].Node(), myNodes1[0].Node(),
                                    cf.myLinks[i2].myFace, cf.myLinks[i2].myIndex );

              cf.myLinks[i1+1].Set( cf.myLinks[i1].myNode2,
                                    cf.myLinks[i1].myNode1,
                                    cf.myLinks[i1].myFace,
                                    cf.myLinks[i1].myIndex);
              n1 = cf.myLinks[i1].myNode2;
            }
            else
              i3 = i1;

            if ( myNodes1[1] != myNodes2[1] ) // a part after the overlapping one
            {
              if ( intPoint.myU[1] < 1 )
                cf.myLinks[i2].Set( myNodes1[1].Node(), myNodes2[1].Node(),
                                    cf.myLinks[i2].myFace, cf.myLinks[i2].myIndex );
              else
                cf.myLinks[i2].Set( myNodes2[1].Node(), myNodes1[1].Node(),
                                    cf.myLinks[i1].myFace, cf.myLinks[i1].myIndex );

              cf.myLinks[i2+1].Set( cf.myLinks[i2].myNode2,
                                    cf.myLinks[i2].myNode1,
                                    cf.myLinks[i2].myFace,
                                    cf.myLinks[i2].myIndex);
              n2 = cf.myLinks[i2].myNode1;
            }
            else
              i3 = i2;

            if ( i3 == cf.myLinks.size() )
              cf.myLinks.resize( i3 + 2 );

            cf.myLinks[i3].Set  ( n1, n2, common.myFace, common.myIndex );
            cf.myLinks[i3+1].Set( n2, n1, common.myFace, common.myIndex );

            i2 = i1 + 1; // recheck modified i1
            continue;
          }
          //else
          // {
          //   // remember a new node
          //   CutLink link1( myNodes1[0].Node(), myNodes1[1].Node(), cf.myInitFace );
          //   CutLink link2( myNodes2[0].Node(), myNodes2[1].Node(), cf.myInitFace );
          //   link2.myIntNode = link1.myIntNode = intPoint.myNode;
          //   addLink( link1 );
          //   addLink( link2 );

          //   // split edges
          //   size_t i = cf.myLinks.size();
          //   if ( intPoint.myNode != cf.myLinks[ i1 ].myNode1 &&
          //        intPoint.myNode != cf.myLinks[ i1 ].myNode2 )
          //   {
          //     cf.myLinks.push_back( cf.myLinks[ i1 ]);
          //     cf.myLinks.push_back( cf.myLinks[ i1 + 1 ]);
          //     cf.myLinks[ i1 ].myNode2 = cf.myLinks[ i1 + 1 ].myNode1 = intPoint.Node();
          //     cf.myLinks[ i  ].myNode1 = cf.myLinks[ i  + 1 ].myNode2 = intPoint.Node();
          //   }
          //   if ( intPoint.myNode != cf.myLinks[ i2 ].myNode1 &&
          //        intPoint.myNode != cf.myLinks[ i2 ].myNode2 )
          //   {
          //     i = cf.myLinks.size();
          //     cf.myLinks.push_back( cf.myLinks[ i2 ]);
          //     cf.myLinks.push_back( cf.myLinks[ i2 + 1 ]);
          //     cf.myLinks[ i2 ].myNode2 = cf.myLinks[ i2 + 1 ].myNode1 = intPoint.Node();
          //     cf.myLinks[ i  ].myNode1 = cf.myLinks[ i  + 1 ].myNode2 = intPoint.Node();
          //   }
          // }

        } // if ( intersectEdgeEdge( 0, 0, intPoint ))

        ++i2;
        --limit;
      }

      // split i1 edge and all edges it intersects
      // don't do it inside intersection loop in order not to loose direction of i1 edge
      if ( !myIntPointSet.empty() )
      {
        cf.myLinks.reserve( cf.myLinks.size() + myIntPointSet.size() * 2 + 2 );

        EdgePart* edge1 = &cf.myLinks[ i1 ];
        EdgePart* twin1 = &cf.myLinks[ i1 + 1 ];

        TIntPointSet::iterator ipIt = myIntPointSet.begin();
        for ( ; ipIt != myIntPointSet.end(); ++ipIt ) // int points sorted on i1 edge
        {
          size_t i = cf.myLinks.size();
          if ( ipIt->myNode != edge1->myNode1 &&
               ipIt->myNode != edge1->myNode2 )
          {
            cf.myLinks.push_back( *edge1 );
            cf.myLinks.push_back( *twin1 );
            edge1->myNode2          = twin1->myNode1              = ipIt->Node();
            cf.myLinks[ i ].myNode1 = cf.myLinks[ i + 1 ].myNode2 = ipIt->Node();
            edge1 = & cf.myLinks[ i ];
            twin1 = & cf.myLinks[ i + 1 ];
          }
          size_t i2 = ipIt->myEdgeInd[1];
          if ( ipIt->myNode != cf.myLinks[ i2 ].myNode1 &&
               ipIt->myNode != cf.myLinks[ i2 ].myNode2 )
          {
            i = cf.myLinks.size();
            cf.myLinks.push_back( cf.myLinks[ i2 ]);
            cf.myLinks.push_back( cf.myLinks[ i2 + 1 ]);
            cf.myLinks[ i2 ].myNode2 = cf.myLinks[ i2 + 1 ].myNode1 = ipIt->Node();
            cf.myLinks[ i  ].myNode1 = cf.myLinks[ i  + 1 ].myNode2 = ipIt->Node();
          }
        }
        if ( cf.myLinks.size() >= limit )
          throw SALOME_Exception( "Infinite loop in Intersector::Algo::IntersectNewEdges()" );
      }
      ++i1; // each internal edge encounters twice
    }
    return;
  }

  //================================================================================
  /*!
   * \brief Split intersected faces
   */
  //================================================================================

  void Intersector::Algo::MakeNewFaces( SMESH_MeshAlgos::TElemIntPairVec& theNew2OldFaces,
                                        SMESH_MeshAlgos::TNodeIntPairVec& theNew2OldNodes,
                                        const double                      theSign,
                                        const bool                        theOptimize)
  {
    // fill theNew2OldFaces if empty
    TCutFaceMap::const_iterator cutFacesIt = myCutFaces.cbegin();
    if ( theNew2OldFaces.empty() )
      for ( ; cutFacesIt != myCutFaces.cend(); ++cutFacesIt )
      {
        const CutFace& cf = *cutFacesIt;
        int index = cf.myInitFace->GetID(); // index in theNew2OldFaces
        if ((int) theNew2OldFaces.size() <= index )
          theNew2OldFaces.resize( index + 1 );
        theNew2OldFaces[ index ] = std::make_pair( cf.myInitFace, index );
      }

    // unmark all nodes except intersection ones

    for ( SMDS_NodeIteratorPtr nIt = myMesh->nodesIterator(); nIt->more(); )
    {
      const SMDS_MeshNode* n = nIt->next();
      if ( n->isMarked() && n->GetID()-1 < (int) theNew2OldNodes.size() )
        n->setIsMarked( false );
    }
    // SMESH_MeshAlgos::MarkElems( myMesh->nodesIterator(), false );

    TCutLinkMap::const_iterator cutLinksIt = myCutLinks.cbegin();
    // for ( ; cutLinksIt != myCutLinks.cend(); ++cutLinksIt )
    // {
    //   const CutLink& link = *cutLinksIt;
    //   if ( link.IntNode() && link.IntNode()->GetID()-1 < (int) theNew2OldNodes.size() )
    //     link.IntNode()->setIsMarked( true );
    // }

    // intersect edges added to myCutFaces

    for ( cutFacesIt = myCutFaces.cbegin(); cutFacesIt != myCutFaces.cend(); ++cutFacesIt )
    {
      const CutFace& cf = *cutFacesIt;
      cf.ReplaceNodes( myRemove2KeepNodes );
      IntersectNewEdges( cf );
    }

    // make new faces

    EdgeLoopSet                            loopSet;
    SMESH_MeshAlgos::Triangulate           triangulator( theOptimize );
    std::vector< EdgePart >                cutOffLinks;
    TLinkMap                               cutOffCoplanarLinks;
    std::vector< const CutFace* >          touchedFaces;
    SMESH_MeshAlgos::TElemIntPairVec::value_type new2OldTria;
    CutFace                                cutFace(0);
    std::vector< const SMDS_MeshNode* >    nodes;
    std::vector<const SMDS_MeshElement *>  faces;

    cutOffLinks.reserve( myCutFaces.Extent() * 2 );

    for ( cutFacesIt = myCutFaces.cbegin(); cutFacesIt != myCutFaces.cend(); ++cutFacesIt )
    {
      const CutFace& cf = *cutFacesIt;
      if ( !cf.IsCut() )
      {
        touchedFaces.push_back( & cf );
        continue;
      }

      const gp_XYZ& normal = myNormals[ cf.myInitFace->GetID() ];

      // form loops of new faces
      cf.ReplaceNodes( myRemove2KeepNodes );
      cf.MakeLoops( loopSet, normal );

      // avoid loops that are not connected to boundary edges of cf.myInitFace
      if ( cf.RemoveInternalLoops( loopSet ))
      {
        IntersectNewEdges( cf );
        cf.MakeLoops( loopSet, normal );
      }
      // erase loops that are cut off by face intersections
      cf.CutOffLoops( loopSet, theSign, myNormals, cutOffLinks, cutOffCoplanarLinks );

      int index = cf.myInitFace->GetID(); // index in theNew2OldFaces

      const SMDS_MeshElement* tria;
      for ( size_t iL = 0; iL < loopSet.myNbLoops; ++iL )
      {
        EdgeLoop& loop = loopSet.myLoops[ iL ];
        if ( loop.myLinks.size() == 0 )
          continue;

        int nbTria  = triangulator.GetTriangles( &loop, nodes );
        int nbNodes = 3 * nbTria;
        for ( int i = 0; i < nbNodes; i += 3 )
        {
          if ( nodes[i] == nodes[i+1] || nodes[i] == nodes[i+2] || nodes[i+1] == nodes[i+2] )
          {
#ifdef _DEBUG_
            std::cerr << "BAD tria" << std::endl;
            cf.Dump();
#endif
            continue;
          }
          if (!( tria = myMesh->FindFace( nodes[i], nodes[i+1], nodes[i+2] )))
            tria = myMesh->AddFace( nodes[i], nodes[i+1], nodes[i+2] );
          tria->setIsMarked( true ); // not to remove it

          new2OldTria = std::make_pair( tria, theNew2OldFaces[ index ].second );
          if ( tria->GetID() < (int)theNew2OldFaces.size() )
            theNew2OldFaces[ tria->GetID() ] = new2OldTria;
          else
            theNew2OldFaces.push_back( new2OldTria );

          if ( index == tria->GetID() )
            index = 0; // do not remove tria
        }
      }
      theNew2OldFaces[ index ].first = 0;
    }

    // remove split faces
    for ( size_t id = 1; id < theNew2OldFaces.size(); ++id )
    {
      if ( theNew2OldFaces[id].first ||
           theNew2OldFaces[id].second == 0 )
        continue;
      if ( const SMDS_MeshElement* f = myMesh->FindElement( id ))
        myMesh->RemoveFreeElement( f );
    }

    // remove faces that are merged off
    for ( cutFacesIt = myCutFaces.cbegin(); cutFacesIt != myCutFaces.cend(); ++cutFacesIt )
    {
      const CutFace& cf = *cutFacesIt;
      if ( !cf.myLinks.empty() || cf.myInitFace->IsNull() )
        continue;

      nodes.assign( cf.myInitFace->begin_nodes(), cf.myInitFace->end_nodes() );
      for ( size_t i = 0; i < nodes.size(); ++i )
      {
        const SMDS_MeshNode* n = nodes[ i ];
        while ( myRemove2KeepNodes.IsBound( n ))
          n = myRemove2KeepNodes( n );
        if ( n != nodes[ i ] && cf.myInitFace->GetNodeIndex( n ) >= 0 )
        {
          theNew2OldFaces[ cf.myInitFace->GetID() ].first = 0;
          myMesh->RemoveFreeElement( cf.myInitFace );
          break;
        }
      }
    }

    // remove faces connected to cut off parts of cf.myInitFace

    nodes.resize(2);
    for ( size_t i = 0; i < cutOffLinks.size(); ++i )
    {
      //break;
      nodes[0] = cutOffLinks[i].myNode1;
      nodes[1] = cutOffLinks[i].myNode2;

      if ( nodes[0] != nodes[1] &&
           myMesh->GetElementsByNodes( nodes, faces ))
      {
        if ( // cutOffLinks[i].myFace &&
            cutOffLinks[i].myIndex != EdgePart::_COPLANAR &&
            faces.size() != 1 )
          continue;
        for ( size_t iF = 0; iF < faces.size(); ++iF )
        {
          int index = faces[iF]->GetID();
          // if ( //faces[iF]->isMarked()         ||  // kept part of cutFace
          //      !theNew2OldFaces[ index ].first ) // already removed
          //   continue;
          cutFace.myInitFace = faces[iF];
          // if ( myCutFaces.Contains( cutFace )) // keep cutting faces needed in CutOffLoops()
          // {
          //   if ( !myCutFaces.Added( cutFace ).IsCut() )
          //     theNew2OldFaces[ index ].first = 0;
          //   continue;
          // }
          cutFace.myLinks.clear();
          cutFace.InitLinks();
          for ( size_t iL = 0; iL < cutFace.myLinks.size(); ++iL )
            if ( !cutOffLinks[i].IsSame( cutFace.myLinks[ iL ]))
              cutOffLinks.push_back( cutFace.myLinks[ iL ]);

          theNew2OldFaces[ index ].first = 0;
          myMesh->RemoveFreeElement( faces[iF] );
        }
      }
    }

    // replace nodes in touched faces

    // treat touched faces
    for ( size_t i = 0; i < touchedFaces.size(); ++i )
    {
      const CutFace& cf = *touchedFaces[i];
      if ( cf.myInitFace->IsNull() )
        continue;

      int index = cf.myInitFace->GetID(); // index in theNew2OldFaces
      if ( !theNew2OldFaces[ index ].first )
        continue; // already cut off

      cf.InitLinks();
      if ( !cf.ReplaceNodes( myRemove2KeepNodes ))
      {
        if ( cf.myLinks.size() == 3 &&
             cf.myInitFace->GetNodeIndex( cf.myLinks[0].myNode1 ) >= 0 &&
             cf.myInitFace->GetNodeIndex( cf.myLinks[1].myNode1 ) >= 0 &&
             cf.myInitFace->GetNodeIndex( cf.myLinks[2].myNode1 ) >= 0 )
          continue; // just keep as is
      }

      if ( cf.myLinks.size() == 3 )
      {
        const SMDS_MeshElement* tria = myMesh->AddFace( cf.myLinks[0].myNode1,
                                                        cf.myLinks[1].myNode1,
                                                        cf.myLinks[2].myNode1 );
        new2OldTria = std::make_pair( tria, theNew2OldFaces[ index ].second );
        if ( tria->GetID() < (int)theNew2OldFaces.size() )
          theNew2OldFaces[ tria->GetID() ] = new2OldTria;
        else
          theNew2OldFaces.push_back( new2OldTria );
      }
      theNew2OldFaces[ index ].first = 0;
    }


    // add used new nodes to theNew2OldNodes
    SMESH_MeshAlgos::TNodeIntPairVec::value_type new2OldNode;
    new2OldNode.second = 0;
    for ( cutLinksIt = myCutLinks.cbegin(); cutLinksIt != myCutLinks.cend(); ++cutLinksIt )
    {
      const CutLink& link = *cutLinksIt;
      if ( link.IntNode() ) // && link.IntNode()->NbInverseElements() > 0 )
      {
        new2OldNode.first = link.IntNode();
        theNew2OldNodes.push_back( new2OldNode );
      }
    }

    return;
  }

  //================================================================================
  Intersector::Intersector( SMDS_Mesh* mesh, double tol, const std::vector< gp_XYZ >& normals )
  {
    myAlgo = new Algo( mesh, tol, normals );
  }
  //================================================================================
  Intersector::~Intersector()
  {
    delete myAlgo;
  }
  //================================================================================
  //! compute cut of two faces of the mesh
  void Intersector::Cut( const SMDS_MeshElement* face1,
                         const SMDS_MeshElement* face2,
                         const int               nbCommonNodes )
  {
    myAlgo->Cut( face1, face2, nbCommonNodes );
  }
  //================================================================================
  //! store a face cut by a line given by its ends
  //  accompanied by indices of intersected face edges.
  //  Edge index is <0 if a line end is inside the face.
  void Intersector::Cut( const SMDS_MeshElement* face,
                         SMESH_NodeXYZ&          lineEnd1,
                         int                     edgeIndex1,
                         SMESH_NodeXYZ&          lineEnd2,
                         int                     edgeIndex2 )
  {
    myAlgo->Cut( face, lineEnd1, edgeIndex1, lineEnd2, edgeIndex2 );
  }
  //================================================================================
  //! split all face intersected by Cut() methods
  void Intersector::MakeNewFaces( SMESH_MeshAlgos::TElemIntPairVec& theNew2OldFaces,
                                  SMESH_MeshAlgos::TNodeIntPairVec& theNew2OldNodes,
                                  const double                      theSign,
                                  const bool                        theOptimize )
  {
    myAlgo->MakeNewFaces( theNew2OldFaces, theNew2OldNodes, theSign, theOptimize );
  }
  //================================================================================
  //! Cut a face by planes, whose normals point to parts to keep
  bool Intersector::CutByPlanes(const SMDS_MeshElement*        theFace,
                                const std::vector< gp_Ax1 > &  thePlanes,
                                const double                   theTol,
                                std::vector< TFace > &         theNewFaceConnectivity )
  {
    theNewFaceConnectivity.clear();

    // check if theFace is wholly cut off
    std::vector< SMESH_NodeXYZ > facePoints( theFace->begin_nodes(), theFace->end_nodes() );
    facePoints.resize( theFace->NbCornerNodes() );
    for ( size_t iP = 0; iP < thePlanes.size(); ++iP )
    {
      size_t nbOut = 0;
      const gp_Pnt& O = thePlanes[iP].Location();
      for ( size_t i = 0; i < facePoints.size(); ++i )
      {
        gp_Vec Op( O, facePoints[i] );
        nbOut += ( Op * thePlanes[iP].Direction() <= 0 );
      }
      if ( nbOut == facePoints.size() )
        return true;
    }

    // copy theFace into a temporary mesh
    SMDS_Mesh mesh;
    Bnd_B3d faceBox;
    std::vector< const SMDS_MeshNode* > faceNodes;
    faceNodes.resize( facePoints.size() );
    for ( size_t i = 0; i < facePoints.size(); ++i )
    {
      const SMESH_NodeXYZ& n = facePoints[i];
      faceNodes[i] = mesh.AddNode( n.X(), n.Y(), n.Z() );
      faceBox.Add( n );
    }
    const SMDS_MeshElement* faceToCut = 0;
    switch ( theFace->NbCornerNodes() )
    {
    case 3:
      faceToCut = mesh.AddFace( faceNodes[0], faceNodes[1], faceNodes[2] );
      break;
    case 4:
      faceToCut = mesh.AddFace( faceNodes[0], faceNodes[1], faceNodes[2], faceNodes[3] );
      break;
    default:
      faceToCut = mesh.AddPolygonalFace( faceNodes );
    }

    std::vector< gp_XYZ > normals( 2 + thePlanes.size() );
    SMESH_MeshAlgos::FaceNormal( faceToCut, normals[ faceToCut->GetID() ]);

    // add faces corresponding to thePlanes
    std::vector< const SMDS_MeshElement* > planeFaces;
    double faceSize = Sqrt( faceBox.SquareExtent() );
    gp_XYZ   center = 0.5 * ( faceBox.CornerMin() + faceBox.CornerMax() );
    for ( size_t i = 0; i < thePlanes.size(); ++i )
    {
      gp_Ax2 plnAx( thePlanes[i].Location(), thePlanes[i].Direction() );
      gp_XYZ O = plnAx.Location().XYZ();
      gp_XYZ X = plnAx.XDirection().XYZ();
      gp_XYZ Y = plnAx.YDirection().XYZ();
      gp_XYZ Z = plnAx.Direction().XYZ();

      double dot = ( O - center ) * Z;
      gp_XYZ o = center + Z * dot; // center projected to a plane

      gp_XYZ p1 = o + X * faceSize * 2;
      gp_XYZ p2 = o + Y * faceSize * 2;
      gp_XYZ p3 = o - (X + Y ) * faceSize * 2;

      const SMDS_MeshNode* n1 = mesh.AddNode( p1.X(), p1.Y(), p1.Z() );
      const SMDS_MeshNode* n2 = mesh.AddNode( p2.X(), p2.Y(), p2.Z() );
      const SMDS_MeshNode* n3 = mesh.AddNode( p3.X(), p3.Y(), p3.Z() );
      planeFaces.push_back( mesh.AddFace( n1, n2, n3 ));

      normals[ planeFaces.back()->GetID() ] = thePlanes[i].Direction().XYZ();
    }

    // cut theFace
    Algo algo ( &mesh, theTol, normals );
    for ( size_t i = 0; i < planeFaces.size(); ++i )
    {
      algo.Cut( faceToCut, planeFaces[i], 0 );
    }

    // retrieve a result
    SMESH_MeshAlgos::TElemIntPairVec new2OldFaces;
    SMESH_MeshAlgos::TNodeIntPairVec new2OldNodes;
    TCutFaceMap::const_iterator cutFacesIt= algo.myCutFaces.cbegin();
    for ( ; cutFacesIt != algo.myCutFaces.cend(); ++cutFacesIt )
    {
      const CutFace& cf = *cutFacesIt;
      if ( cf.myInitFace != faceToCut )
        continue;

      if ( !cf.IsCut() )
      {
        theNewFaceConnectivity.push_back( facePoints );
        break;
      }

      // intersect cut lines
      algo.IntersectNewEdges( cf );

      // form loops of new faces
      EdgeLoopSet loopSet;
      cf.MakeLoops( loopSet, normals[ faceToCut->GetID() ]);

      // erase loops that are cut off by thePlanes
      const double sign = 1;
      std::vector< EdgePart > cutOffLinks;
      TLinkMap                cutOffCoplanarLinks;
      cf.CutOffLoops( loopSet, sign, normals, cutOffLinks, cutOffCoplanarLinks );

      for ( size_t iL = 0; iL < loopSet.myNbLoops; ++iL )
      {
        EdgeLoop& loop = loopSet.myLoops[ iL ];
        if ( loop.myLinks.size() > 0 )
        {
          facePoints.clear();
          for ( SMDS_NodeIteratorPtr nIt = loop.nodeIterator(); nIt->more(); )
          {
            const SMDS_MeshNode* n = nIt->next();
            facePoints.push_back( n );
            int iN = faceToCut->GetNodeIndex( n );
            if ( iN < 0 )
              facePoints.back()._node = 0; // an intersection point
            else
              facePoints.back()._node = theFace->GetNode( iN );
          }
          theNewFaceConnectivity.push_back( facePoints );
        }
      }
      break;
    }

    return theNewFaceConnectivity.empty();
  }

} // namespace SMESH_MeshAlgos

namespace
{
  //================================================================================
  /*!
   * \brief Debug
   */
  //================================================================================

  void CutFace::Dump() const
  {
    std::cout << std::endl << "INI F " << myInitFace->GetID() << std::endl;
    for ( size_t i = 0; i < myLinks.size(); ++i )
      std::cout << "[" << i << "] ("
                << char(( myLinks[i].IsInternal() ? 'j' : '0' ) + myLinks[i].myIndex ) << ") "
                << myLinks[i].myNode1->GetID() << " - " << myLinks[i].myNode2->GetID()
                << " " << ( myLinks[i].myFace ? 'F' : 'C' )
                << ( myLinks[i].myFace ? myLinks[i].myFace->GetID() : 0 ) << " " << std::endl;
  }

  //================================================================================
  /*!
   * \brief Add an edge cutting this face
   *  \param [in] p1 - start point of the edge
   *  \param [in] p2 - end point of the edge
   *  \param [in] cutter - a face producing the added cut edge.
   *  \param [in] nbOnPlane - nb of triangle nodes lying on the plane of the cutter face
   */
  //================================================================================

  void CutFace::AddEdge( const CutLink&          p1,
                         const CutLink&          p2,
                         const SMDS_MeshElement* cutterFace,
                         const int               nbOnPlane,
                         const int               iNotOnPlane) const
  {
    int iN[2] = { myInitFace->GetNodeIndex( p1.IntNode() ),
                  myInitFace->GetNodeIndex( p2.IntNode() ) };
    if ( iN[0] >= 0 && iN[1] >= 0 )
    {
      // the cutting edge is a whole side
      if ((  cutterFace && nbOnPlane < 3 ) &&
          !( cutterFace->GetNodeIndex( p1.IntNode() ) >= 0 &&
             cutterFace->GetNodeIndex( p2.IntNode() ) >= 0 ))
      {
        InitLinks();
        myLinks[ Abs( iN[0] - iN[1] ) == 1 ? Min( iN[0], iN[1] ) : 2 ].myFace = cutterFace;
      }
      return;
    }

    if ( p1.IntNode() == p2.IntNode() )
    {
      AddPoint( p1, p2, 1e-10 );
      return;
    }

    InitLinks();

    // cut side edges by a new one

    int iEOnPlane = ( nbOnPlane == 2 ) ? ( iNotOnPlane + 1 ) % 3  :  -1;

    double dist[2];
    for ( int is2nd = 0; is2nd < 2; ++is2nd )
    {
      const CutLink& p = is2nd ? p2 : p1;
      dist[ is2nd ] = 0;
      if ( iN[ is2nd ] >= 0 )
        continue;

      int iE = Max( iEOnPlane, myInitFace->GetNodeIndex( p.Node1() ));
      if ( iE < 0 )
        continue; // link of other face

      SMESH_NodeXYZ n0 = myLinks[iE].myNode1;
      dist[ is2nd ]    = ( n0 - p.myIntNode ).SquareModulus();

      for ( size_t i = 0; i < myLinks.size(); ++i )
        if ( myLinks[i].myIndex == iE )
        {
          double d1 = n0.SquareDistance( myLinks[i].myNode1 );
          if ( d1 < dist[ is2nd ] )
          {
            double d2 = n0.SquareDistance( myLinks[i].myNode2 );
            if ( dist[ is2nd ] < d2 )
            {
              myLinks.push_back( myLinks[i] );
              myLinks.back().myNode1 = myLinks[i].myNode2 = p.IntNode();
              break;
            }
          }
        }
    }

    int state = nbOnPlane == 3 ? EdgePart::_COPLANAR : EdgePart::_INTERNAL;

    // look for an existing equal edge
    if ( nbOnPlane == 2 )
    {
      SMESH_NodeXYZ n0 = myLinks[ iEOnPlane ].myNode1;
      if ( iN[0] >= 0 ) dist[0] = ( n0 - p1.myIntNode ).SquareModulus();
      if ( iN[1] >= 0 ) dist[1] = ( n0 - p2.myIntNode ).SquareModulus();
      if ( dist[0] > dist[1] )
        std::swap( dist[0], dist[1] );
      for ( size_t i = 0; i < myLinks.size(); ++i )
      {
        if ( myLinks[i].myIndex != iEOnPlane )
          continue;
        gp_XYZ mid = 0.5 * ( SMESH_NodeXYZ( myLinks[i].myNode1 ) +
                             SMESH_NodeXYZ( myLinks[i].myNode2 ));
        double d = ( n0 - mid ).SquareModulus();
        if ( dist[0] < d && d < dist[1] )
          myLinks[i].myFace = cutterFace;
      }
      return;
    }
    else
    {
      EdgePart newEdge; newEdge.Set( p1.IntNode(), p2.IntNode(), cutterFace, state );
      for ( size_t i = 0; i < myLinks.size(); ++i )
      {
        if ( myLinks[i].IsSame( newEdge ))
        {
          // if ( !myLinks[i].IsInternal() )
          //   myLinks[ i ].myFace = cutterFace;
          // else
          myLinks[ i ].ReplaceCoplanar( newEdge );
          if ( myLinks[i].IsInternal() && i+1 < myLinks.size() )
            myLinks[ i+1 ].ReplaceCoplanar( newEdge );
          return;
        }
        i += myLinks[i].IsInternal();
      }
    }

    size_t  i = myLinks.size();
    myLinks.resize( i + 2 );
    myLinks[ i   ].Set( p1.IntNode(), p2.IntNode(), cutterFace, state );
    myLinks[ i+1 ].Set( p2.IntNode(), p1.IntNode(), cutterFace, state );
  }

  //================================================================================
  /*!
   * \brief Add a point cutting this face
   */
  //================================================================================

  void CutFace::AddPoint( const CutLink& p1, const CutLink& p2, double tol ) const
  {
    if ( myInitFace->GetNodeIndex( p1.IntNode() ) >= 0 ||
         myInitFace->GetNodeIndex( p2.IntNode() ) >= 0 )
      return;

    InitLinks();

    const CutLink* link = &p1;
    int iE = myInitFace->GetNodeIndex( link->Node1() );
    if ( iE < 0 )
    {
      link = &p2;
      iE = myInitFace->GetNodeIndex( link->Node1() );
    }
    if ( iE >= 0 )
    {
      // cut an existing edge by the point
      SMESH_NodeXYZ n0 = link->Node1();
      double         d = ( n0 - link->myIntNode ).SquareModulus();

      for ( size_t i = 0; i < myLinks.size(); ++i )
        if ( myLinks[i].myIndex == iE )
        {
          double d1 = n0.SquareDistance( myLinks[i].myNode1 );
          if ( d1 < d )
          {
            double d2 = n0.SquareDistance( myLinks[i].myNode2 );
            if ( d < d2 )
            {
              myLinks.push_back( myLinks[i] );
              myLinks.back().myNode1 = myLinks[i].myNode2 = link->IntNode();
              return;
            }
          }
        }
    }
    else // point is inside the triangle
    {
      // // check if a point already added
      // for ( size_t i = 3; i < myLinks.size(); ++i )
      //   if ( myLinks[i].myNode1 == p1.IntNode() )
      //     return;

      // // create a link between the point and the closest corner node
      // const SMDS_MeshNode* closeNode = myLinks[0].myNode1;
      // double minDist = p1.myIntNode.SquareDistance( closeNode );
      // for ( int i = 1; i < 3; ++i )
      // {
      //   double dist = p1.myIntNode.SquareDistance( myLinks[i].myNode1 );
      //   if ( dist < minDist )
      //   {
      //     minDist = dist;
      //     closeNode = myLinks[i].myNode1;
      //   }
      // }
      // if ( minDist > tol * tol )
      // {
      //   size_t i = myLinks.size();
      //   myLinks.resize( i + 2 );
      //   myLinks[ i   ].Set( p1.IntNode(), closeNode );
      //   myLinks[ i+1 ].Set( closeNode, p1.IntNode() );
      // }
    }
  }

  //================================================================================
  /*!
   * \brief Perform node replacement
   */
  //================================================================================

  bool CutFace::ReplaceNodes( const TNNMap& theRm2KeepMap ) const
  {
    bool replaced = false;
    for ( size_t i = 0; i < myLinks.size(); ++i )
    {
      while ( theRm2KeepMap.IsBound( myLinks[i].myNode1 ))
        replaced = ( myLinks[i].myNode1 = theRm2KeepMap( myLinks[i].myNode1 ));

      while ( theRm2KeepMap.IsBound( myLinks[i].myNode2 ))
        replaced = ( myLinks[i].myNode2 = theRm2KeepMap( myLinks[i].myNode2 ));
    }

    //if ( replaced ) // remove equal links
    {
      for ( size_t i1 = 0; i1 < myLinks.size(); ++i1 )
      {
        if ( myLinks[i1].myNode1 == myLinks[i1].myNode2 )
        {
          myLinks.erase( myLinks.begin() + i1,
                         myLinks.begin() + i1 + 1 + myLinks[i1].IsInternal() );
          --i1;
          continue;
        }
        size_t i2 = i1 + 1 + myLinks[i1].IsInternal();
        for ( ; i2 < myLinks.size(); ++i2 )
        {
          if ( !myLinks[i2].IsInternal() )
            continue;
          if ( myLinks[i1].IsSame( myLinks[i2] ))
          {
            myLinks[i1].  ReplaceCoplanar( myLinks[i2] );
            if ( myLinks[i1].IsInternal() )
              myLinks[i1+1].ReplaceCoplanar( myLinks[i2+1] );
            if ( !myLinks[i1].myFace && myLinks[i2].myFace )
            {
              myLinks[i1].  myFace = myLinks[i2].myFace;
              if ( myLinks[i1].IsInternal() )
                myLinks[i1+1].myFace = myLinks[i2+1].myFace;
            }
            myLinks.erase( myLinks.begin() + i2,
                           myLinks.begin() + i2 + 2 );
            --i2;
            continue;
          }
          ++i2;
        }
        i1 += myLinks[i1].IsInternal();
      }
    }

    return replaced;
  }

  //================================================================================
  /*!
   * \brief Initialize myLinks with edges of myInitFace
   */
  //================================================================================

  void CutFace::InitLinks() const
  {
    if ( !myLinks.empty() ) return;

    int nbNodes = myInitFace->NbNodes();
    myLinks.reserve( nbNodes * 2 );
    myLinks.resize( nbNodes );

    for ( int i = 0; i < nbNodes; ++i )
    {
      const SMDS_MeshNode* n1 = myInitFace->GetNode( i );
      const SMDS_MeshNode* n2 = myInitFace->GetNodeWrap( i + 1);
      myLinks[i].Set( n1, n2, 0, i );
    }
  }
  
  //================================================================================
  /*!
   * \brief Return number of internal edges
   */
  //================================================================================

  int CutFace::NbInternalEdges() const
  {
    int nb = 0;
    for ( size_t i = 3; i < myLinks.size(); ++i )
      nb += myLinks[i].IsInternal();

    return nb / 2; // each internal edge encounters twice
  }

  //================================================================================
  /*!
   * \brief Remove loops that are not connected to boundary edges of myFace by
   *        adding edges connecting these loops to the boundary.
   *        Such loops must be removed as they form polygons with ring topology.
   */
  //================================================================================

  bool CutFace::RemoveInternalLoops( EdgeLoopSet& theLoops ) const
  {
    size_t nbReachedLoops = 0;

    // count loops including boundary EdgeParts
    for ( size_t iL = 0; iL < theLoops.myNbLoops; ++iL )
    {
      EdgeLoop& loop = theLoops.myLoops[ iL ];

      for ( size_t iE = 0; iE < loop.myLinks.size(); ++iE )
        if ( !loop.myLinks[ iE ]->IsInternal() )
        {
          nbReachedLoops += loop.SetConnected();
          break;
        }
    }
    if ( nbReachedLoops == theLoops.myNbLoops )
      return false; // no unreachable loops


    // try to reach all loops by propagating via internal edges shared by loops
    size_t prevNbReached;
    do
    {
      prevNbReached = nbReachedLoops;

      for ( size_t iL = 0; iL < theLoops.myNbLoops; ++iL )
      {
        EdgeLoop& loop = theLoops.myLoops[ iL ];
        if ( !loop.myIsBndConnected )
          continue;

        for ( size_t iE = 0; iE < loop.myLinks.size(); ++iE )
          if ( loop.myLinks[ iE ]->IsInternal() )
          {
            const EdgePart* twinEdge = getTwin( loop.myLinks[ iE ]);
            EdgeLoop*          loop2 = theLoops.GetLoopOf( twinEdge );
            if ( loop2->SetConnected() && ++nbReachedLoops == theLoops.myNbLoops )
              return false; // no unreachable loops
          }
      }
    }
    while ( prevNbReached < nbReachedLoops );



    for ( size_t iL = 0; iL < theLoops.myNbLoops; ++iL )
    {
      EdgeLoop& loop = theLoops.myLoops[ iL ];
      if ( loop.myIsBndConnected || loop.myLinks.size() == 0 )
        continue;

      if ( loop.myHasPending )
      {
        // try to join the loop to another one, with which it contacts at a node

        // look for a node where the loop reverses
        const EdgePart* edgePrev = loop.myLinks.back();
        for ( size_t iE = 0; iE < loop.myLinks.size(); edgePrev = loop.myLinks[ iE++ ] )
        {
          if ( !edgePrev->IsTwin( *loop.myLinks[ iE ]))
            continue;
          const SMDS_MeshNode* reverseNode = edgePrev->myNode2;

          // look for a loop including reverseNode
          size_t iContactEdge2; // index(+1) of edge starting at reverseNode
          for ( size_t iL2 = 0; iL2 < theLoops.myNbLoops; ++iL2 )
          {
            if ( iL == iL2 )
              continue;
            EdgeLoop& loop2 = theLoops.myLoops[ iL2 ];
            if ( ! ( iContactEdge2 = loop2.Contains( reverseNode )))
              continue;

            // insert loop2 into the loop
            theLoops.Join( loop, iE, loop2, iContactEdge2 - 1 );
            break;
          }
          if ( loop.myIsBndConnected )
            break;
        }

        if ( loop.myIsBndConnected )
          continue;
      }

      // add links connecting internal loops with the boundary ones

      // find a pair of closest nodes
      const SMDS_MeshNode *closestNode1, *closestNode2;
      double minDist = 1e100;
      for ( size_t iE = 0; iE < loop.myLinks.size(); ++iE )
      {
        SMESH_NodeXYZ n1 = loop.myLinks[ iE ]->myNode1;

        for ( size_t i = 0; i < myLinks.size(); ++i )
        {
          if ( !loop.Contains( myLinks[i].myNode1 ))
          {
            double dist = n1.SquareDistance( myLinks[i].myNode1 );
            if ( dist < minDist )
            {
              minDist = dist;
              closestNode1 = loop.myLinks[ iE ]->myNode1;
              closestNode2 = myLinks[i].myNode1;
            }
          }
          if ( myLinks[i].IsInternal() )
            ++i;
        }
      }

      size_t i = myLinks.size();
      myLinks.resize( i + 2 );
      myLinks[ i   ].Set( closestNode1, closestNode2 );
      myLinks[ i+1 ].Set( closestNode2, closestNode1 );
    }

    return true;
  }

  //================================================================================
  /*!
   * \brief Return equal reversed edge
   */
  //================================================================================

  EdgePart* CutFace::getTwin( const EdgePart* edge ) const
  {
    size_t i = edge - & myLinks[0];

    if ( i > 2 && myLinks[ i-1 ].IsTwin( *edge ))
      return & myLinks[ i-1 ];

    if ( i+1 < myLinks.size() &&
         myLinks[ i+1 ].IsTwin( *edge ))
      return & myLinks[ i+1 ];

    return 0;
  }

  //================================================================================
  /*!
   * \brief Fill loops of edges
   */
  //================================================================================

  void CutFace::MakeLoops( EdgeLoopSet& theLoops, const gp_XYZ& theFaceNorm ) const
  {
    theLoops.Init( myLinks );

    if ( myLinks.size() == 3 )
    {
      theLoops.AddNewLoop();
      theLoops.AddEdge( myLinks[0] );
      if ( myLinks[0].myNode2 == myLinks[1].myNode1 )
      {
        theLoops.AddEdge( myLinks[1] );
        theLoops.AddEdge( myLinks[2] );
      }
      else
      {
        theLoops.AddEdge( myLinks[2] );
        theLoops.AddEdge( myLinks[1] );
      }
      return;
    }

    while ( !theLoops.AllEdgesUsed() )
    {
      EdgeLoop& loop = theLoops.AddNewLoop();

      // add 1st edge to a new loop
      size_t i1;
      for ( i1 = theLoops.myNbLoops - 1; i1 < myLinks.size(); ++i1 )
        if ( theLoops.AddEdge( myLinks[i1] ))
          break;

      EdgePart*             lastEdge = & myLinks[ i1 ];
      EdgePart*             twinEdge = getTwin( lastEdge );
      const SMDS_MeshNode* firstNode = lastEdge->myNode1;
      const SMDS_MeshNode*  lastNode = lastEdge->myNode2;

      do // add the rest edges
      {
        theLoops.myCandidates.clear(); // edges starting at lastNode
        int nbInternal = 0;

        // find candidate edges
        for ( size_t i = i1 + 1; i < myLinks.size(); ++i )
          if ( myLinks[ i ].myNode1 == lastNode &&
               &myLinks[ i ] != twinEdge &&
               !theLoops.myIsUsedEdge[ i ])
          {
            theLoops.myCandidates.push_back( & myLinks[ i ]);
            nbInternal += myLinks[ i ].IsInternal();
          }

        // choose among candidates
        if ( theLoops.myCandidates.size() == 0 )
        {
          loop.myHasPending = bool( twinEdge );
          lastEdge = twinEdge;
        }
        else if ( theLoops.myCandidates.size() == 1 )
        {
          lastEdge = theLoops.myCandidates[0];
        }
        else if ( nbInternal == 1 && !lastEdge->IsInternal() )
        {
          lastEdge = theLoops.myCandidates[ !theLoops.myCandidates[0]->IsInternal() ];
        }
        else
        {
          gp_Vec  lastVec = *lastEdge;
          double maxAngle = -2 * M_PI;
          for ( size_t i = 0; i < theLoops.myCandidates.size(); ++i )
          {
            double angle = lastVec.AngleWithRef( *theLoops.myCandidates[i], theFaceNorm );
            if ( angle > maxAngle )
            {
              maxAngle = angle;
              lastEdge = theLoops.myCandidates[i];
            }
          }
        }
        theLoops.AddEdge( *lastEdge );
        lastNode = lastEdge->myNode2;
        twinEdge = getTwin( lastEdge );
      }
      while ( lastNode != firstNode );


      if ( twinEdge == & myLinks[ i1 ])
        loop.myHasPending = true;

    } // while ( !theLoops.AllEdgesUsed() )

    return;
  }

  //================================================================================
  /*!
   * \brief Erase loops that are cut off by face intersections
   */
  //================================================================================

  void CutFace::CutOffLoops( EdgeLoopSet&                 theLoops,
                             const double                 theSign,
                             const std::vector< gp_XYZ >& theNormals,
                             std::vector< EdgePart >&     theCutOffLinks,
                             TLinkMap&                    theCutOffCoplanarLinks) const
  {
    EdgePart sideEdge;
    boost::container::flat_set< const SMDS_MeshElement* > checkedCoplanar;

    for ( size_t i = 0; i < myLinks.size(); ++i )
    {
      if ( !myLinks[i].myFace )
        continue;

      EdgeLoop* loop = theLoops.GetLoopOf( & myLinks[i] );
      if ( !loop || loop->myLinks.empty() || loop->myHasPending )
        continue;

      bool toErase, isCoplanar = ( myLinks[i].myIndex == EdgePart::_COPLANAR );

      gp_Vec iniNorm = theNormals[ myInitFace->GetID() ];
      if ( isCoplanar )
      {
        toErase = ( myLinks[i].myFace->GetID() > myInitFace->GetID() );

        const EdgePart* twin = getTwin( & myLinks[i] );
        if ( !twin || twin->myFace == myLinks[i].myFace )
        {
          // only one co-planar face includes myLinks[i]
          gp_Vec inFaceDir = iniNorm ^ myLinks[i];
          gp_XYZ   edgePnt = SMESH_NodeXYZ( myLinks[i].myNode1 );
          for ( int iN = 0; iN < 3; ++iN )
          {
            gp_Vec inCutFaceDir = ( SMESH_NodeXYZ( myLinks[i].myFace->GetNode( iN )) - edgePnt );
            if ( inCutFaceDir * inFaceDir < 0 )
            {
              toErase = false;
              break;
            }
          }
        }
      }
      else
      {
        gp_Vec   cutNorm = theNormals[ myLinks[i].myFace->GetID() ];
        gp_Vec inFaceDir = iniNorm ^ myLinks[i];

        toErase = inFaceDir * cutNorm * theSign < 0;
        if ( !toErase )
        {
          // erase a neighboring loop
          loop = 0;
          if ( const EdgePart* twin = getTwin( & myLinks[i] ))
            loop = theLoops.GetLoopOf( twin );
          toErase = ( loop && !loop->myLinks.empty() );
        }

        if ( toErase ) // do not erase if cutFace is connected to a co-planar cutFace
        {
          checkedCoplanar.clear();
          for ( size_t iE = 0; iE < myLinks.size() &&  toErase; ++iE )
          {
            if ( !myLinks[iE].myFace || myLinks[iE].myIndex != EdgePart::_COPLANAR )
              continue;
            bool isAdded = checkedCoplanar.insert( myLinks[iE].myFace ).second;
            if ( !isAdded )
              continue;
            toErase = ( SMESH_MeshAlgos::NbCommonNodes( myLinks[i ].myFace,
                                                        myLinks[iE].myFace ) < 1 );
          }
        }
      }

      if ( toErase )
      {
        if ( !isCoplanar )
        {
          // remember whole sides of myInitFace that are cut off
          for ( size_t iE = 0; iE < loop->myLinks.size(); ++iE )
          {
            if ( !loop->myLinks[ iE ]->myFace              &&
                 !loop->myLinks[ iE ]->IsInternal()     )//   &&
              // !loop->myLinks[ iE ]->myNode1->isMarked() && // cut nodes are marked
              // !loop->myLinks[ iE ]->myNode2->isMarked() )
            {
              int i = loop->myLinks[ iE ]->myIndex;
              sideEdge.Set( myInitFace->GetNode    ( i   ),
                            myInitFace->GetNodeWrap( i+1 ));
              theCutOffLinks.push_back( sideEdge );

              if ( !sideEdge.IsSame( *loop->myLinks[ iE ] )) // nodes replaced
              {
                theCutOffLinks.push_back( *loop->myLinks[ iE ] );
              }
            }
            else if ( IsCoplanar( loop->myLinks[ iE ]))
            {
              // propagate erasure to a co-planar face
              theCutOffLinks.push_back( *loop->myLinks[ iE ]);
            }
            else if ( loop->myLinks[ iE ]->myFace &&
                      loop->myLinks[ iE ]->IsInternal() )
              theCutOffLinks.push_back( *loop->myLinks[ iE ]);
          }

          // clear the loop
          theLoops.Erase( loop );
        }
      }
    }
    return;
  }

  //================================================================================
  /*!
   * \brief Check if the face has cut edges
   */
  //================================================================================

  bool CutFace::IsCut() const
  {
    if ( myLinks.size() > 3 )
      return true;

    if ( myLinks.size() == 3 )
      for ( size_t i = 0; i < 3; ++i )
        if ( myLinks[i].myFace )
          return true;

    return false;
  }

  //================================================================================
  /*!
   * \brief Check if an edge is produced by a co-planar cut
   */
  //================================================================================

  bool CutFace::IsCoplanar( const EdgePart* edge ) const
  {
    if ( edge->myIndex == EdgePart::_COPLANAR )
    {
      const EdgePart* twin = getTwin( edge );
      return ( !twin || twin->myIndex == EdgePart::_COPLANAR );
    }
    return false;
  }

  //================================================================================
  /*!
   * \brief Replace _COPLANAR cut edge by _INTERNAL or vice versa
   */
  //================================================================================

  bool EdgePart::ReplaceCoplanar( const EdgePart& e )
  {
    if ( myIndex + e.myIndex == _COPLANAR + _INTERNAL )
    {
      //check if the faces are connected
      int nbCommonNodes = 0;
      if ( e.myFace && myFace )
        nbCommonNodes = SMESH_MeshAlgos::NbCommonNodes( e.myFace, myFace );
      bool toReplace = (( myIndex == _INTERNAL && nbCommonNodes > 1 ) ||
                        ( myIndex == _COPLANAR && nbCommonNodes < 2 ));
      if ( toReplace )
      {
        myIndex = e.myIndex;
        myFace  = e.myFace;
        return true;
      }
    }
    return false;
  }

} // namespace

//================================================================================
/*!
 * \brief Create an offsetMesh of given faces
 *  \param [in] faceIt - the input faces
 *  \param [out] new2OldFaces - history of faces (new face -> old face ID)
 *  \param [out] new2OldNodes - history of nodes (new node -> old node ID)
 *  \return SMDS_Mesh* - the new offset mesh, a caller should delete
 */
//================================================================================

SMDS_Mesh* SMESH_MeshAlgos::MakeOffset( SMDS_ElemIteratorPtr theFaceIt,
                                        SMDS_Mesh&           theSrcMesh,
                                        const double         theOffset,
                                        const bool           theFixIntersections,
                                        TElemIntPairVec&     theNew2OldFaces,
                                        TNodeIntPairVec&     theNew2OldNodes)
{
  if ( theSrcMesh.GetMeshInfo().NbFaces( ORDER_QUADRATIC ) > 0 )
    throw SALOME_Exception( "Offset of quadratic mesh not supported" );
  if ( theSrcMesh.GetMeshInfo().NbFaces() > theSrcMesh.GetMeshInfo().NbTriangles() )
    throw SALOME_Exception( "Offset of non-triangular mesh not supported" );

  SMDS_Mesh* newMesh = new SMDS_Mesh;
  theNew2OldFaces.clear();
  theNew2OldNodes.clear();
  theNew2OldFaces.push_back
    ( std::make_pair(( const SMDS_MeshElement*) 0, 0)); // to have index == face->GetID()

  // copy input faces to the newMesh keeping IDs of nodes

  double minNodeDist = 1e100;

  std::vector< const SMDS_MeshNode* > nodes;
  while ( theFaceIt->more() )
  {
    const SMDS_MeshElement* face = theFaceIt->next();
    if ( face->GetType() != SMDSAbs_Face ) continue;

    // copy nodes
    nodes.assign( face->begin_nodes(), face->end_nodes() );
    for ( size_t i = 0; i < nodes.size(); ++i )
    {
      const SMDS_MeshNode* newNode = newMesh->FindNode( nodes[i]->GetID() );
      if ( !newNode )
      {
        SMESH_NodeXYZ xyz( nodes[i] );
        newNode = newMesh->AddNodeWithID( xyz.X(), xyz.Y(), xyz.Z(), nodes[i]->GetID() );
        theNew2OldNodes.push_back( std::make_pair( newNode, nodes[i]->GetID() ));
        nodes[i] = newNode;
      }
    }
    const SMDS_MeshElement* newFace = 0;
    switch ( face->GetEntityType() )
    {
    case SMDSEntity_Triangle:
      newFace = newMesh->AddFace( nodes[0],nodes[1],nodes[2] );
      break;
    case SMDSEntity_Quad_Triangle:
      newFace = newMesh->AddFace( nodes[0],nodes[1],nodes[2],
                                  nodes[3],nodes[4],nodes[5] );
      break;
    case SMDSEntity_BiQuad_Triangle:
      newFace = newMesh->AddFace( nodes[0],nodes[1],nodes[2],
                                  nodes[3],nodes[4],nodes[5],nodes[6] );
      break;
    case SMDSEntity_Quadrangle:
      newFace = newMesh->AddFace( nodes[0],nodes[1],nodes[2],nodes[3] );
      break;
    case SMDSEntity_Quad_Quadrangle:
      newFace = newMesh->AddFace( nodes[0],nodes[1],nodes[2],nodes[3],
                                  nodes[4],nodes[5],nodes[6],nodes[7] );
      break;
    case SMDSEntity_BiQuad_Quadrangle:
      newFace = newMesh->AddFace( nodes[0],nodes[1],nodes[2],nodes[3],nodes[4],
                                  nodes[5],nodes[6],nodes[7],nodes[8] );
      break;
    case SMDSEntity_Polygon:
      newFace = newMesh->AddPolygonalFace( nodes );
      break;
    case SMDSEntity_Quad_Polygon:
      newFace = newMesh->AddQuadPolygonalFace( nodes );
      break;
    default:
      continue;
    }
    theNew2OldFaces.push_back( std::make_pair( newFace, face->GetID() ));

    SMESH_NodeXYZ pPrev = nodes.back(), p;
    for ( size_t i = 0; i < nodes.size(); ++i )
    {
      p.Set( nodes[i] );
      double dist = ( pPrev - p ).SquareModulus();
      if ( dist < minNodeDist && dist > std::numeric_limits<double>::min() )
        minNodeDist = dist;
      pPrev = p;
    }
  } // while ( faceIt->more() )


  // compute normals to faces
  std::vector< gp_XYZ > normals( theNew2OldFaces.size() );
  for ( size_t i = 1; i < normals.size(); ++i )
  {
    if ( !SMESH_MeshAlgos::FaceNormal( theNew2OldFaces[i].first, normals[i] ))
      normals[i].SetCoord( 0,0,0 ); // TODO find norm by neighbors
  }

  const double sign = ( theOffset < 0 ? -1 : +1 );
  const double  tol = Min( 1e-3 * Sqrt( minNodeDist ),
                           1e-2 * theOffset * sign );

  // translate new nodes by normal to input faces
  gp_XYZ newXYZ;
  std::vector< const SMDS_MeshNode* > multiNormalNodes;
  for ( size_t i = 0; i < theNew2OldNodes.size(); ++i )
  {
    const SMDS_MeshNode* newNode = theNew2OldNodes[i].first;

    if ( getTranslatedPosition( newNode, theOffset, tol*10., sign, normals, theSrcMesh, newXYZ ))
      newMesh->MoveNode( newNode, newXYZ.X(), newXYZ.Y(), newXYZ.Z() );
    else
      multiNormalNodes.push_back( newNode );
  }
  // make multi-normal translation
  std::vector< SMESH_NodeXYZ > multiPos(10);
  for ( size_t i = 0; i < multiNormalNodes.size(); ++i )
  {
    const SMDS_MeshNode* newNode = multiNormalNodes[i];
    newNode->setIsMarked( true );
    SMESH_NodeXYZ oldXYZ = newNode;
    multiPos.clear();
    for ( SMDS_ElemIteratorPtr fIt = newNode->GetInverseElementIterator(); fIt->more(); )
    {
      const SMDS_MeshElement* newFace = fIt->next();
      const int             faceIndex = newFace->GetID();
      const gp_XYZ&           oldNorm = normals[ faceIndex ];
      const gp_XYZ             newXYZ = oldXYZ + oldNorm * theOffset;
      if ( multiPos.empty() )
      {
        newMesh->MoveNode( newNode, newXYZ.X(), newXYZ.Y(), newXYZ.Z() );
        multiPos.emplace_back( newNode );
      }
      else
      {
        newNode = 0;
        for ( size_t iP = 0; iP < multiPos.size() &&  !newNode; ++iP )
          if (( multiPos[iP] - newXYZ ).SquareModulus() < tol * tol )
            newNode = multiPos[iP].Node();
        if ( !newNode )
        {
          newNode = newMesh->AddNode( newXYZ.X(), newXYZ.Y(), newXYZ.Z() );
          newNode->setIsMarked( true );
          theNew2OldNodes.push_back( std::make_pair( newNode, 0 ));
          multiPos.emplace_back( newNode );
        }
      }
      if ( newNode != oldXYZ.Node() )
      {
        nodes.assign( newFace->begin_nodes(), newFace->end_nodes() );
        nodes[ newFace->GetNodeIndex( oldXYZ.Node() )] = newNode;
        newMesh->ChangeElementNodes( newFace, & nodes[0], nodes.size() );
      }
    }
  }

  if ( !theFixIntersections )
    return newMesh;


  // remove new faces around concave nodes (they are marked) if the faces are inverted
  gp_XYZ faceNorm;
  for ( size_t i = 0; i < theNew2OldNodes.size(); ++i )
  {
    const SMDS_MeshNode* newNode = theNew2OldNodes[i].first;
    //const SMDS_MeshNode* oldNode = theNew2OldNodes[i].second;
    if ( newNode->isMarked() )
    {
      //gp_XYZ moveVec = sign * ( SMESH_NodeXYZ( newNode ) - SMESH_NodeXYZ( oldNode ));

      //bool haveInverseFace = false;
      for ( SMDS_ElemIteratorPtr fIt = newNode->GetInverseElementIterator(); fIt->more(); )
      {
        const SMDS_MeshElement* newFace = fIt->next();
        const int             faceIndex = newFace->GetID();
        const gp_XYZ&           oldNorm = normals[ faceIndex ];
        if ( !SMESH_MeshAlgos::FaceNormal( newFace, faceNorm, /*normalize=*/false ) ||
             //faceNorm * moveVec < 0 )
             faceNorm * oldNorm < 0 )
        {
          //haveInverseFace = true;
          theNew2OldFaces[ faceIndex ].first = 0;
          newMesh->RemoveFreeElement( newFace );
          //break;
        }
      }
      // if ( haveInverseFace )
      // {
      //   newMesh->MoveNode( newNode, oldNode->X(), oldNode->Y(), oldNode->Z() );

      //   for ( SMDS_ElemIteratorPtr fIt = newNode->GetInverseElementIterator(); fIt->more(); )
      //   {
      //     const SMDS_MeshElement* newFace = fIt->next();
      //     if ( !SMESH_MeshAlgos::FaceNormal( newFace, normals[ newFace->GetID() ] ))
      //       normals[i].SetCoord( 0,0,0 ); // TODO find norm by neighbors
      //   }
      // }
    }
    // mark all new nodes located closer than theOffset from theSrcMesh
  }

  removeSmallFaces( newMesh, theNew2OldFaces, tol*tol );

  // ==================================================
  // find self-intersections of new faces and fix them
  // ==================================================

  std::unique_ptr< SMESH_ElementSearcher > fSearcher
    ( SMESH_MeshAlgos::GetElementSearcher( *newMesh, tol ));

  Intersector intersector( newMesh, tol, normals );

  std::vector< const SMDS_MeshElement* > closeFaces;
  std::vector< SMESH_NodeXYZ >           faceNodes;
  Bnd_B3d faceBox;

  for ( size_t iF = 1; iF < theNew2OldFaces.size(); ++iF )
  {
    const SMDS_MeshElement* newFace = theNew2OldFaces[iF].first;
    if ( !newFace ) continue;
    faceNodes.assign( newFace->begin_nodes(), newFace->end_nodes() );

    bool isConcaveNode1 = false;
    for ( size_t iN = 0; iN < faceNodes.size() && !isConcaveNode1; ++iN )
      isConcaveNode1 = faceNodes[iN]->isMarked();

    // get faces close to a newFace
    closeFaces.clear();
    faceBox.Clear();
    for ( size_t i = 0; i < faceNodes.size(); ++i )
      faceBox.Add( faceNodes[i] );
    faceBox.Enlarge( tol );

    fSearcher->GetElementsInBox( faceBox, SMDSAbs_Face, closeFaces );

    // intersect the newFace with closeFaces

    for ( size_t i = 0; i < closeFaces.size(); ++i )
    {
      const SMDS_MeshElement* closeFace = closeFaces[i];
      if ( closeFace->GetID() <= newFace->GetID() )
        continue; // this pair already treated

      // do not intersect connected faces if they have no concave nodes
      int nbCommonNodes = 0;
      for ( size_t iN = 0; iN < faceNodes.size(); ++iN )
        nbCommonNodes += ( closeFace->GetNodeIndex( faceNodes[iN].Node() ) >= 0 );

      if ( !isConcaveNode1 )
      {
        bool isConcaveNode2 = false;
        for ( SMDS_ElemIteratorPtr nIt = closeFace->nodesIterator(); nIt->more(); )
          if (( isConcaveNode2 = nIt->next()->isMarked() ))
            break;

        if ( !isConcaveNode2 && nbCommonNodes > 0 )
        {
          if ( normals[ newFace->GetID() ] * normals[ closeFace->GetID() ] < 1.0 )
            continue; // not co-planar
        }
      }

      intersector.Cut( newFace, closeFace, nbCommonNodes );
    }
  }
  intersector.MakeNewFaces( theNew2OldFaces, theNew2OldNodes, sign, /*optimize=*/true );

  return newMesh;
}
