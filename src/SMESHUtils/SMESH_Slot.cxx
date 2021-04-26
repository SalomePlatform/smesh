// Copyright (C) 2018-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : SMESH_Slot.cxx
// Created   : Fri Nov 30 15:58:37 2018
// Author    : Edward AGAPOV (eap)

#include "SMESH_MeshAlgos.hxx"

#include "ObjectPool.hxx"
#include "SMDS_LinearEdge.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_MeshGroup.hxx"

#include <IntAna_IntConicQuad.hxx>
#include <IntAna_Quadric.hxx>
#include <NCollection_DataMap.hxx>
#include <NCollection_Map.hxx>
#include <Precision.hxx>
#include <gp_Ax1.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <Utils_SALOME_Exception.hxx>

namespace
{
  typedef SMESH_MeshAlgos::Edge TEdge;
  
  //================================================================================
  //! point of intersection of a face edge with the cylinder
  struct IntPoint
  {
    SMESH_NodeXYZ myNode;        // point and a node
    int           myEdgeIndex;   // face edge index
    bool          myIsOutPln[2]; // isOut of two planes

    double SquareDistance( const IntPoint& p ) const { return ( myNode-p.myNode ).SquareModulus(); }
  };

  //================================================================================
  //! cut of a face
  struct Cut
  {
    IntPoint myIntPnt1, myIntPnt2;
    const SMDS_MeshElement* myFace;

    const IntPoint& operator[]( size_t i ) const { return i ? myIntPnt2 : myIntPnt1; }

    double SquareDistance( const gp_Pnt& p, gp_XYZ & pClosest ) const
    {
      gp_Vec edge( myIntPnt1.myNode, myIntPnt2.myNode );
      gp_Vec n1p ( myIntPnt1.myNode, p  );
      double u = ( edge * n1p ) / edge.SquareMagnitude(); // param [0,1] on the edge
      if ( u <= 0. )
      {
        pClosest = myIntPnt1.myNode;
        return n1p.SquareMagnitude();
      }
      if ( u >= 1. )
      {
        pClosest = myIntPnt2.myNode;
        return p.SquareDistance( myIntPnt2.myNode );
      }
      pClosest = myIntPnt1.myNode + u * edge.XYZ(); // projection of the point on the edge
      return p.SquareDistance( pClosest );
    }
  };

  //================================================================================
  //! poly-line segment
  struct Segment
  {
    typedef std::vector< Cut > TCutList;

    const SMDS_MeshElement*        myEdge;
    TCutList                       myCuts;
    std::vector< const IntPoint* > myFreeEnds; // ends of cut edges

    Segment( const SMDS_MeshElement* e = 0 ): myEdge(e) { myCuts.reserve( 4 ); }

    // return its axis
    gp_Ax1 Ax1( bool reversed = false ) const
    {
      SMESH_NodeXYZ n1 = myEdge->GetNode(  reversed );
      SMESH_NodeXYZ n2 = myEdge->GetNode( !reversed );
      return gp_Ax1( n1, gp_Dir( n2 - n1 ));
    }

    // return a node
    const SMDS_MeshNode* Node(int i) const
    {
      return myEdge->GetNode( i % 2 );
    }

    // store an intersection edge forming the slot border
    void AddCutEdge( const IntPoint& p1,
                     const IntPoint& p2,
                     const SMDS_MeshElement* myFace )
    {
      myCuts.push_back( Cut({ p1, p2, myFace }));
    }

    // return number of not shared IntPoint's
    int NbFreeEnds( double tol )
    {
      if ( myCuts.empty() )
        return 0;
      if ( myFreeEnds.empty() )
      {
        // remove degenerated cuts
        // for ( size_t iC1 = 0; iC1 < myCuts.size(); ++iC1 )
        //   if ( myCuts[ iC1 ][ 0 ].myNode == myCuts[ iC1 ][ 1 ].myNode )
        //   {
        //     if ( iC1 < myCuts.size() - 1 )
        //       myCuts[ iC1 ] = myCuts.back();
        //     myCuts.pop_back();
        //   }

        int nbShared = 0;
        std::vector< bool > isSharedPnt( myCuts.size() * 2, false );
        for ( size_t iC1 = 0; iC1 < myCuts.size() - 1; ++iC1 )
          for ( size_t iP1 = 0; iP1 < 2; ++iP1 )
          {
            size_t i1 = iC1 * 2 + iP1;
            if ( isSharedPnt[ i1 ])
              continue;
            for ( size_t iC2 = iC1 + 1; iC2 < myCuts.size(); ++iC2 )
              for ( size_t iP2 = 0; iP2 < 2; ++iP2 )
              {
                size_t i2 = iC2 * 2 + iP2;
                if ( isSharedPnt[ i2 ])
                  continue;
                if ( myCuts[ iC1 ][ iP1 ].SquareDistance( myCuts[ iC2 ][ iP2 ]) < tol * tol )
                {
                  nbShared += 2;
                  if ( myCuts[ iC1 ][ 0 ].SquareDistance( myCuts[ iC1 ][ 1 ]) < tol * tol )
                    isSharedPnt[ iC1 * 2 ] = isSharedPnt[ iC1 * 2 + 1 ] = true;
                  else if ( myCuts[ iC2 ][ 0 ].SquareDistance( myCuts[ iC2 ][ 1 ]) < tol * tol )
                    isSharedPnt[ iC2 * 2 ] = isSharedPnt[ iC2 * 2 + 1 ] = true;
                  else
                    isSharedPnt[ i1 ] = isSharedPnt[ i2 ] = true;
                }
              }
          }
        myFreeEnds.reserve( isSharedPnt.size() - nbShared );
        for ( size_t i = 0; i < isSharedPnt.size(); ++i )
          if ( !isSharedPnt[ i ] )
          {
            int iP = i % 2;
            int iC = i / 2;
            myFreeEnds.push_back( & myCuts[ iC ][ iP ]);
          }
      }
      return myFreeEnds.size();
    }
  };
  typedef ObjectPoolIterator<Segment> TSegmentIterator;


  //================================================================================
  //! Segments and plane separating domains of segments, at common node
  struct NodeData
  {
    std::vector< Segment* > mySegments;
    gp_Ax1                  myPlane; // oriented OK for mySegments[0]

    void AddSegment( Segment* seg, const SMDS_MeshNode* n )
    {
      mySegments.reserve(2);
      mySegments.push_back( seg );
      if ( mySegments.size() == 1 )
      {
        myPlane = mySegments[0]->Ax1( mySegments[0]->myEdge->GetNodeIndex( n ));
      }
      else
      {
        gp_Ax1 axis2 = mySegments[1]->Ax1( mySegments[1]->myEdge->GetNodeIndex( n ));
        myPlane.SetDirection( myPlane.Direction().XYZ() - axis2.Direction().XYZ() );
      }
    }
    gp_Ax1 Plane( const Segment* seg )
    {
      return ( seg == mySegments[0] ) ? myPlane : myPlane.Reversed();
    }
  };
  typedef NCollection_DataMap< const SMDS_MeshNode*, NodeData, SMESH_Hasher > TSegmentsOfNode;


  //================================================================================
  /*!
   * \brief Intersect a face edge given by its nodes with a cylinder.
   */
  //================================================================================

  bool intersectEdge( const gp_Cylinder&      cyl,
                      const SMESH_NodeXYZ&    n1,
                      const SMESH_NodeXYZ&    n2,
                      const double            tol,
                      std::vector< IntPoint >& intPoints )
  {
    gp_Lin line( gp_Ax1( n1, gp_Dir( n2 - n1 )));
    IntAna_IntConicQuad intersection( line, IntAna_Quadric( cyl ));

    if ( !intersection.IsDone()     ||
         intersection.IsParallel()  ||
         intersection.IsInQuadric() ||
         intersection.NbPoints() == 0 )
      return false;

    gp_Vec edge( n1, n2 );

    size_t oldNbPnts = intPoints.size();
    for ( int iP = 1; iP <= intersection.NbPoints(); ++iP )
    {
      const gp_Pnt& p = intersection.Point( iP );

      gp_Vec n1p ( n1, p );
      const SMDS_MeshNode* n = 0;

      double u = ( edge * n1p ) / edge.SquareMagnitude(); // param [0,1] on the edge
      if ( u <= 0. ) {
        if ( p.SquareDistance( n1 ) < tol * tol )
          n = n1.Node();
        else
          continue;
      }
      else if ( u >= 1. ) {
        if ( p.SquareDistance( n2 ) < tol * tol )
          n = n2.Node();
        else
          continue;
      }
      else {
        if      ( p.SquareDistance( n1 ) < tol * tol )
          n = n1.Node();
        else if ( p.SquareDistance( n2 ) < tol * tol )
          n = n2.Node();
      }

      intPoints.push_back( IntPoint() );
      if ( n )
        intPoints.back().myNode.Set( n );
      else
        intPoints.back().myNode.SetCoord( p.X(),p.Y(),p.Z() );
    }

    // set points order along an edge
    if ( intPoints.size() - oldNbPnts == 2 &&
         intersection.ParamOnConic( 1 ) > intersection.ParamOnConic( 2 ))
    {
      int i = intPoints.size() - 1;
      std::swap( intPoints[ i ], intPoints[ i - 1 ]);
    }

    return intPoints.size() - oldNbPnts > 0;
  }

  //================================================================================
  /*!
   * \brief Return signed distance between a point and a plane
   */
  //================================================================================

  double signedDist( const gp_Pnt& p, const gp_Ax1& planeNormal )
  {
    const gp_Pnt& O = planeNormal.Location();
    gp_Vec Op( O, p );
    return Op * planeNormal.Direction();
  }

  //================================================================================
  /*!
   * \brief Check if a point is outside a segment domain bound by two planes
   */
  //================================================================================

  bool isOut( const gp_Pnt& p, const gp_Ax1* planeNormal, bool* isOutPtr, int nbPln = 2 )
  {
    isOutPtr[0] = isOutPtr[1] = false;

    for ( int i = 0; i < nbPln; ++i )
    {
      isOutPtr[i] = ( signedDist( p, planeNormal[i] ) <= 0. );
    }
    return ( isOutPtr[0] && isOutPtr[1] );
  }

  //================================================================================
  /*!
   * \brief Check if a segment between two points is outside a segment domain bound by two planes
   */
  //================================================================================

  bool isSegmentOut( bool* isOutPtr1, bool* isOutPtr2 )
  {
    return (( isOutPtr1[0] && isOutPtr2[0] ) ||
            ( isOutPtr1[1] && isOutPtr2[1] ));
  }

  //================================================================================
  /*!
   * \brief cut off ip1 from edge (ip1 - ip2) by a plane
   */
  //================================================================================

  void cutOff( IntPoint & ip1, const IntPoint & ip2, const gp_Ax1& planeNormal, double tol )
  {
    gp_Lin lin( ip1.myNode, ( ip2.myNode - ip1.myNode ));
    gp_Pln pln( planeNormal.Location(), planeNormal.Direction() );

    IntAna_IntConicQuad intersection( lin, pln, Precision::Angular/*Tolerance*/() );
    if ( intersection.IsDone()      &&
         !intersection.IsParallel()  &&
         !intersection.IsInQuadric() &&
         intersection.NbPoints() == 1 )
    {
      if ( intersection.Point( 1 ).SquareDistance( ip1.myNode ) > tol * tol )
      {
        static_cast< gp_XYZ& >( ip1.myNode ) = intersection.Point( 1 ).XYZ();
        ip1.myNode._node = 0;
        ip1.myEdgeIndex = -1;
      }
    }
  }

  //================================================================================
  /*!
   * \brief Assure that face normal is computed in faceNormals vector 
   */
  //================================================================================

  const gp_XYZ& computeNormal( const SMDS_MeshElement* face,
                               std::vector< gp_XYZ >&  faceNormals )
  {
    bool toCompute;
    if ((int) faceNormals.size() <= face->GetID() )
    {
      toCompute = true;
      faceNormals.resize( face->GetID() + 1 );
    }
    else
    {
      toCompute = faceNormals[ face->GetID() ].SquareModulus() == 0.;
    }
    if ( toCompute )
      SMESH_MeshAlgos::FaceNormal( face, faceNormals[ face->GetID() ], /*normalized=*/false );

    return faceNormals[ face->GetID() ];
  }

  typedef std::vector< SMDS_MeshGroup* > TGroupVec;

  //================================================================================
  /*!
   * \brief Fill theFaceID2Groups map for a given face
   *  \param [in] theFace - the face
   *  \param [in] theGroupsToUpdate - list of groups to treat
   *  \param [out] theFaceID2Groups - the map to fill in
   *  \param [out] theWorkGroups - a working buffer of groups
   */
  //================================================================================

  void findGroups( const SMDS_MeshElement *                                 theFace,
                   TGroupVec &                                              theGroupsToUpdate,
                   NCollection_DataMap< smIdType, TGroupVec, smIdHasher > & theFaceID2Groups,
                   TGroupVec &                                              theWorkGroups )
  {
    theWorkGroups.clear();
    for ( size_t i = 0; i < theGroupsToUpdate.size(); ++i )
      if ( theGroupsToUpdate[i]->Contains( theFace ))
        theWorkGroups.push_back( theGroupsToUpdate[i] );

    if ( !theWorkGroups.empty() )
      theFaceID2Groups.Bind( theFace->GetID(), theWorkGroups );
  }

  //================================================================================
  /*!
   * \brief Check distance between a point and an edge defined by a couple of nodes
   */
  //================================================================================

  bool isOnEdge( const SMDS_MeshNode* n1,
                 const SMDS_MeshNode* n2,
                 const gp_Pnt&        p,
                 const double         tol )
  {
    SMDS_LinearEdge edge( n1, n2 );
    return ( SMESH_MeshAlgos::GetDistance( &edge, p ) < tol );
  }

  //================================================================================
  /*!
   * \return Index of intersection point detected on a triangle cut by planes
   *  \param [in] i - index of a cut triangle side
   *  \param [in] n1 - 1st point of a cut triangle side
   *  \param [in] n2 - 2nd point of a cut triangle side
   *  \param [in] face - a not cut triangle
   *  \param [in] intPoint - the intersection point
   *  \param [in] faceNodes - nodes of not cut triangle
   *  \param [in] tol - tolerance
   */
  //================================================================================

  int edgeIndex( const int                                  i,
                 const SMESH_NodeXYZ&                       n1,
                 const SMESH_NodeXYZ&                       n2,
                 const SMDS_MeshElement*                    face,
                 const IntPoint&                            intPoint,
                 const std::vector< const SMDS_MeshNode* >& faceNodes,
                 const double                               tol )
  {
    if ( n1.Node() && n2.Node() )
      return face->GetNodeIndex( n1.Node() );

    // project intPoint to sides of face
    for ( size_t i = 1; i < faceNodes.size(); ++i )
      if ( isOnEdge( faceNodes[ i-1 ], faceNodes[ i ], intPoint.myNode, tol ))
        return i - 1;

    return -(i+1);
  }

  //================================================================================
  /*!
   * \brief Find a neighboring segment and its next node
   *  \param [in] curSegment - a current segment
   *  \param [in,out] curNode - a current node to update
   *  \param [in] segmentsOfNode - map of segments of nodes
   *  \return Segment* - the found segment
   */
  //================================================================================

  Segment* nextSegment( const Segment*         curSegment,
                        const SMDS_MeshNode* & curNode,
                        const TSegmentsOfNode& segmentsOfNode )
  {
    Segment* neighborSeg = 0;
    const NodeData& noData = segmentsOfNode( curNode );
    for ( size_t iS = 0; iS < noData.mySegments.size()  && !neighborSeg; ++iS )
      if ( noData.mySegments[ iS ] != curSegment )
        neighborSeg = noData.mySegments[ iS ];

    if ( neighborSeg )
    {
      int iN = ( neighborSeg->Node(0) == curNode );
      curNode = neighborSeg->Node( iN );
    }
    return neighborSeg;
  }

  //================================================================================
  /*!
   * \brief Tries to find a segment to which a given point is too close
   *  \param [in] p - the point
   *  \param [in] minDist - minimal allowed distance from segment
   *  \param [in] curSegment - start segment
   *  \param [in] curNode - start node
   *  \param [in] segmentsOfNode - map of segments of nodes
   *  \return bool - true if a too close segment found
   */
  //================================================================================

  const Segment* findTooCloseSegment( const IntPoint&        p,
                                      const double           minDist,
                                      const double           /*tol*/,
                                      const Segment*         curSegment,
                                      const SMDS_MeshNode*   curNode,
                                      const TSegmentsOfNode& segmentsOfNode )
  {
    double prevDist = Precision::Infinite();
    while ( curSegment )
    {
      double dist = SMESH_MeshAlgos::GetDistance( curSegment->myEdge, p.myNode );
      if ( dist < minDist )
      {
        // check if dist is less than distance of curSegment to its cuts
        // double minCutDist = prevDist;
        // bool     coincide = false;
        // for ( size_t iC = 0; iC < curSegment->myCuts.size(); ++iC )
        // {
        //   if (( coincide = ( curSegment->myCuts[iC].SquareDistance( p.myNode ) < tol * tol )))
        //     break;
        //   for ( size_t iP = 0; iP < 2; ++iP )
        //   {
        //     double cutDist = SMESH_MeshAlgos::GetDistance( curSegment->myEdge,
        //                                                    curSegment->myCuts[iC][iP].myNode );
        //     minCutDist = std::min( minCutDist, cutDist );
        //   }
        // }
        // if ( !coincide && minCutDist > dist )
        return curSegment;
      }
      if ( dist > prevDist )
        break;
      prevDist   = dist;
      curSegment = nextSegment( curSegment, curNode, segmentsOfNode );
    }
    return 0;
  }
}

//================================================================================
/*!
 * \brief Create a slot of given width around given 1D elements lying on a triangle mesh.
 * The slot is constructed by cutting faces by cylindrical surfaces made around each segment.
 * \return Edges located at the slot boundary
 */
//================================================================================

std::vector< SMESH_MeshAlgos::Edge >
SMESH_MeshAlgos::MakeSlot( SMDS_ElemIteratorPtr             theSegmentIt,
                           double                           theWidth,
                           SMDS_Mesh*                       theMesh,
                           std::vector< SMDS_MeshGroup* > & theGroupsToUpdate)
{
  std::vector< Edge > bndEdges;

  if ( !theSegmentIt || !theSegmentIt->more() || !theMesh || theWidth == 0.)
    return bndEdges;

  // ----------------------------------------------------------------------------------
  // put the input segments to a data map in order to be able finding neighboring ones
  // ----------------------------------------------------------------------------------

  TSegmentsOfNode segmentsOfNode;
  ObjectPool< Segment > segmentPool;

  while( theSegmentIt->more() )
  {
    const SMDS_MeshElement* edge = theSegmentIt->next();
    if ( edge->GetType() != SMDSAbs_Edge )
      throw SALOME_Exception( "A segment is not a mesh edge");

    Segment* segment = segmentPool.getNew();
    segment->myEdge = edge;

    for ( SMDS_NodeIteratorPtr nIt = edge->nodeIterator(); nIt->more(); )
    {
      const SMDS_MeshNode* n = nIt->next();
      NodeData* noData = segmentsOfNode.ChangeSeek( n );
      if ( !noData )
        noData = segmentsOfNode.Bound( n, NodeData() );
      noData->AddSegment( segment, n );
    }
  }

  // ---------------------------------
  // Cut the mesh around the segments
  // ---------------------------------

  const double tol = Precision::Confusion();
  const double angularTol = 1e-5;
  std::vector< gp_XYZ > faceNormals;
  SMESH_MeshAlgos::Intersector meshIntersector( theMesh, tol, faceNormals );
  std::unique_ptr< SMESH_ElementSearcher> faceSearcher;

  std::vector< NLink > startEdges;
  std::vector< const SMDS_MeshNode* > faceNodes(4), edgeNodes(2);
  std::vector<const SMDS_MeshElement *> faces(2);
  NCollection_Map<const SMDS_MeshElement*, SMESH_Hasher > checkedFaces;
  std::vector< IntPoint > intPoints, p(2);
  std::vector< SMESH_NodeXYZ > facePoints(4);
  std::vector< Intersector::TFace > cutFacePoints;

  NCollection_DataMap< smIdType, TGroupVec, smIdHasher > faceID2Groups;
  TGroupVec groupVec;

  std::vector< gp_Ax1 > planeNormalVec(2);
  gp_Ax1 * planeNormal = & planeNormalVec[0];
  
  for ( TSegmentIterator segIt( segmentPool ); segIt.more(); ) // loop on all segments
  {
    Segment* segment = const_cast< Segment* >( segIt.next() );

    gp_Lin      segLine( segment->Ax1() );
    gp_Ax3      cylAxis( segLine.Location(), segLine.Direction() );
    gp_Cylinder segCylinder( cylAxis, 0.5 * theWidth );
    double      radius2( segCylinder.Radius() * segCylinder.Radius() );

    // get normals of planes separating domains of neighboring segments
    for ( int i = 0; i < 2; ++i ) // loop on 2 segment ends
    {
      const SMDS_MeshNode* n = segment->Node( i );
      planeNormal[i] = segmentsOfNode( n ).Plane( segment );
    }

    // we explore faces around a segment starting from face edges;
    // initialize a list of starting edges
    startEdges.clear();
    {
      // get a face to start searching intersected faces from
      const SMDS_MeshNode*      n0 = segment->Node( 0 );
      SMDS_ElemIteratorPtr     fIt = n0->GetInverseElementIterator( SMDSAbs_Face );
      const SMDS_MeshElement* face = ( fIt->more() ) ? fIt->next() : 0;
      if ( !theMesh->Contains( face ))
      {
        if ( !faceSearcher )
          faceSearcher.reset( SMESH_MeshAlgos::GetElementSearcher( *theMesh ));
        face = faceSearcher->FindClosestTo( SMESH_NodeXYZ( n0 ), SMDSAbs_Face );
      }
      // collect face edges
      int nbNodes = face->NbCornerNodes();
      faceNodes.assign( face->begin_nodes(), face->end_nodes() );
      faceNodes.resize( nbNodes + 1 );
      faceNodes[ nbNodes ] = faceNodes[ 0 ];
      for ( int i = 0; i < nbNodes; ++i )
        startEdges.push_back( NLink( faceNodes[i], faceNodes[i+1] ));
    }

    // intersect faces located around a segment
    checkedFaces.Clear();
    while ( !startEdges.empty() )
    {
      edgeNodes[0] = startEdges[0].first;
      edgeNodes[1] = startEdges[0].second;

      theMesh->GetElementsByNodes( edgeNodes, faces, SMDSAbs_Face );
      for ( size_t iF = 0; iF < faces.size(); ++iF ) // loop on faces sharing a start edge
      {
        const SMDS_MeshElement* face = faces[iF];
        if ( !checkedFaces.Add( face ))
          continue;

        int nbNodes = face->NbCornerNodes();
        if ( nbNodes != 3 )
          throw SALOME_Exception( "MakeSlot() accepts triangles only" );
        faceNodes.assign( face->begin_nodes(), face->end_nodes() );
        faceNodes.resize( nbNodes + 1 );
        faceNodes[ nbNodes ] = faceNodes[ 0 ];
        facePoints.assign( faceNodes.begin(), faceNodes.end() );

        // check if cylinder axis || face
        const gp_XYZ& faceNorm = computeNormal( face, faceNormals );
        bool isCylinderOnFace  = ( Abs( faceNorm * cylAxis.Direction().XYZ() ) < tol );

        if ( !isCylinderOnFace )
        {
          if ( Intersector::CutByPlanes( face, planeNormalVec, tol, cutFacePoints ))
            continue; // whole face cut off
          facePoints.swap( cutFacePoints[0] );
          facePoints.push_back( facePoints[0] );
        }

        // find intersection points on face edges
        intPoints.clear();
        int nbPoints = facePoints.size()-1;
        int nbFarPoints = 0;
        for ( int i = 0; i < nbPoints; ++i )
        {
          const SMESH_NodeXYZ& n1 = facePoints[i];
          const SMESH_NodeXYZ& n2 = facePoints[i+1];

          size_t iP = intPoints.size();
          intersectEdge( segCylinder, n1, n2, tol, intPoints );

          // save edge index
          if ( isCylinderOnFace )
            for ( ; iP < intPoints.size(); ++iP )
              intPoints[ iP ].myEdgeIndex = i;
          else
            for ( ; iP < intPoints.size(); ++iP )
              intPoints[ iP ].myEdgeIndex = edgeIndex( i, n1, n2, face,
                                                       intPoints[ iP ], faceNodes, tol );

          nbFarPoints += ( segLine.SquareDistance( n1 ) > radius2 );
        }

        // feed startEdges
        if ( nbFarPoints < nbPoints || !intPoints.empty() )
          for ( size_t i = 1; i < faceNodes.size(); ++i )
          {
            const SMESH_NodeXYZ& n1 = faceNodes[i];
            const SMESH_NodeXYZ& n2 = faceNodes[i-1];
            isOut( n1, planeNormal, p[0].myIsOutPln );
            isOut( n2, planeNormal, p[1].myIsOutPln );
            if ( !isSegmentOut( p[0].myIsOutPln, p[1].myIsOutPln ))
            {
              startEdges.push_back( NLink( n1.Node(), n2.Node() ));
            }
          }

        if ( intPoints.size() < 2 )
          continue;

        // classify intPoints by planes
        for ( size_t i = 0; i < intPoints.size(); ++i )
          isOut( intPoints[i].myNode, planeNormal, intPoints[i].myIsOutPln );

        // cut the face

        if ( intPoints.size() > 2 )
          intPoints.push_back( intPoints[0] );

        for ( size_t iE = 1; iE < intPoints.size(); ++iE ) // 2 <= intPoints.size() <= 5
        {
          if (( intPoints[iE].myIsOutPln[0] && intPoints[iE].myIsOutPln[1]   ) ||
              ( isSegmentOut( intPoints[iE].myIsOutPln, intPoints[iE-1].myIsOutPln )))
            continue; // intPoint is out of domain

          // check if a cutting edge connecting two intPoints is on cylinder surface
          if ( intPoints[iE].myEdgeIndex == intPoints[iE-1].myEdgeIndex )
            continue; // on same edge
          if ( intPoints[iE].myNode.Node() &&
               intPoints[iE].myNode == intPoints[iE-1].myNode ) // coincide
            continue;

          gp_XYZ edegDir = intPoints[iE].myNode - intPoints[iE-1].myNode;

          bool toCut; // = edegDir.SquareModulus() > tol * tol;
          if ( intPoints.size() == 2 )
            toCut = true;
          else if ( isCylinderOnFace )
            toCut = cylAxis.Direction().IsParallel( edegDir, angularTol );
          else
          {
            SMESH_NodeXYZ nBetween;
            int eInd = intPoints[iE-1].myEdgeIndex;
            if ( eInd < 0 )
              nBetween = facePoints[( 1 - (eInd-1)) % nbPoints ];
            else
              nBetween = faceNodes[( 1 + eInd ) % nbNodes ];
            toCut = ( segLine.SquareDistance( nBetween ) > radius2 );
          }
          if ( !toCut )
            continue;

          // limit the edge by planes
          if ( intPoints[iE].myIsOutPln[0] ||
               intPoints[iE].myIsOutPln[1] )
            cutOff( intPoints[iE], intPoints[iE-1],
                    planeNormal[ intPoints[iE].myIsOutPln[1] ], tol );

          if ( intPoints[iE-1].myIsOutPln[0] ||
               intPoints[iE-1].myIsOutPln[1] )
            cutOff( intPoints[iE-1], intPoints[iE],
                    planeNormal[ intPoints[iE-1].myIsOutPln[1] ], tol );

          gp_XYZ edegDirNew = intPoints[iE].myNode - intPoints[iE-1].myNode;
          if ( edegDir * edegDirNew < 0 ||
               edegDir.SquareModulus() < tol * tol )
            continue; // fully cut off

          segment->AddCutEdge( intPoints[iE], intPoints[iE-1], face );

        }
      }  // loop on faces sharing an edge

      startEdges[0] = startEdges.back();
      startEdges.pop_back();

    } // loop on startEdges
  } // loop on all input segments


  // ----------------------------------------------------------
  // If a plane fully cuts off edges of one side of a segment,
  // it also may cut edges of adjacent segments
  // ----------------------------------------------------------

  for ( TSegmentIterator segIt( segmentPool ); segIt.more(); ) // loop on all segments
  {
    Segment* segment = const_cast< Segment* >( segIt.next() );
    if ( segment->NbFreeEnds( tol ) >= 4 )
      continue;

    for ( int iE = 0; iE < 2; ++iE ) // loop on 2 segment ends
    {
      const SMDS_MeshNode* n1 = segment->Node( iE );
      const SMDS_MeshNode* n2 = segment->Node( 1 - iE );
      planeNormal[0] = segmentsOfNode( n1 ).Plane( segment );

      bool isNeighborCut;
      Segment* neighborSeg = segment;
      do // check segments connected to the segment via n2
      {
        neighborSeg = nextSegment( neighborSeg, n2, segmentsOfNode );
        if ( !neighborSeg )
          break;

        isNeighborCut = false;
        for ( size_t iC = 0; iC < neighborSeg->myCuts.size(); ++iC ) // check cut edges
        {
          IntPoint* intPnt = &( neighborSeg->myCuts[iC].myIntPnt1 );
          isOut( intPnt[0].myNode, planeNormal, intPnt[0].myIsOutPln, 1 );
          isOut( intPnt[1].myNode, planeNormal, intPnt[1].myIsOutPln, 1 );
          const Segment * closeSeg[2] = { 0, 0 };
          if ( intPnt[0].myIsOutPln[0] )
            closeSeg[0] = findTooCloseSegment( intPnt[0], 0.5 * theWidth - 1e-3*tol, tol,
                                               segment, n1, segmentsOfNode );
          if ( intPnt[1].myIsOutPln[0] )
            closeSeg[1] = findTooCloseSegment( intPnt[1], 0.5 * theWidth - 1e-3*tol, tol,
                                               segment, n1, segmentsOfNode );
          int nbCut = bool( closeSeg[0] ) + bool( closeSeg[1] );
          if ( nbCut == 0 )
            continue;
          isNeighborCut = true;
          if ( nbCut == 2 ) // remove a cut
          {
            if ( iC < neighborSeg->myCuts.size() - 1 )
              neighborSeg->myCuts[iC] = neighborSeg->myCuts.back();
            neighborSeg->myCuts.pop_back();
          }
          else // shorten cuts of 1) neighborSeg and 2) closeSeg
          {
            // 1)
            int iP = bool( closeSeg[1] );
            gp_Lin      segLine( closeSeg[iP]->Ax1() );
            gp_Ax3      cylAxis( segLine.Location(), segLine.Direction() );
            gp_Cylinder cyl( cylAxis, 0.5 * theWidth );
            intPoints.clear();
            if ( intersectEdge( cyl, intPnt[iP].myNode, intPnt[1-iP].myNode, tol, intPoints ) &&
                 intPoints[0].SquareDistance( intPnt[iP] ) > tol * tol )
              intPnt[iP].myNode = intPoints[0].myNode;

            // 2)
            double minCutDist = theWidth;
            gp_XYZ projection, closestProj;
            int    iCut = -1;
            for ( size_t iC2 = 0; iC2 < closeSeg[iP]->myCuts.size(); ++iC2 )
            {
              double cutDist = closeSeg[iP]->myCuts[iC2].SquareDistance( intPnt[iP].myNode,
                                                                        projection );
              if ( cutDist < minCutDist )
              {
                closestProj = projection;
                minCutDist  = cutDist;
                iCut        = iC2;
                if ( minCutDist < tol * tol )
                  break;
              }
            }
            if ( iCut < 0 )
              continue; // ???
            double d1 = SMESH_MeshAlgos::GetDistance( neighborSeg->myEdge,
                                                      closeSeg[iP]->myCuts[iCut][0].myNode );
            double d2 = SMESH_MeshAlgos::GetDistance( neighborSeg->myEdge,
                                                      closeSeg[iP]->myCuts[iCut][1].myNode );
            int iP2 = ( d2 < d1 );
            IntPoint& ip = const_cast< IntPoint& >( closeSeg[iP]->myCuts[iCut][iP2] );
            ip = intPnt[iP];
          }
          // update myFreeEnds
          neighborSeg->myFreeEnds.clear();
          neighborSeg->NbFreeEnds( tol );
        }
      }
      while ( isNeighborCut );
    }
  }

  // -----------------------
  // Cut faces by cut edges
  // -----------------------

  for ( TSegmentIterator segIt( segmentPool ); segIt.more(); ) // loop on all segments
  {
    Segment* segment = const_cast< Segment* >( segIt.next() );
    for ( size_t iC = 0; iC < segment->myCuts.size(); ++iC )
    {
      Cut & cut = segment->myCuts[ iC ];
      computeNormal( cut.myFace, faceNormals );
      meshIntersector.Cut( cut.myFace,
                           cut.myIntPnt1.myNode, cut.myIntPnt1.myEdgeIndex,
                           cut.myIntPnt2.myNode, cut.myIntPnt2.myEdgeIndex );

      Edge e = { cut.myIntPnt1.myNode.Node(), cut.myIntPnt2.myNode.Node(), 0 };
      bndEdges.push_back( e );

      findGroups( cut.myFace, theGroupsToUpdate, faceID2Groups, groupVec );
    }
  }

  // -----------------------------------------
  // Make cut at the end of group of segments
  // -----------------------------------------

  std::vector<const SMDS_MeshElement*> polySegments;

  for ( TSegmentsOfNode::Iterator nSegsIt( segmentsOfNode ); nSegsIt.More(); nSegsIt.Next() )
  {
    const NodeData& noData = nSegsIt.Value();
    if ( noData.mySegments.size() != 1 )
      continue;

    const Segment* segment = noData.mySegments[0];

    // find two IntPoint's of cut edges to make a cut between
    if ( segment->myFreeEnds.size() != 4 )
      throw SALOME_Exception( "MakeSlot(): too short end edge?" );
    std::multimap< double, const IntPoint* > dist2IntPntMap;
    for ( size_t iE = 0; iE < segment->myFreeEnds.size(); ++iE )
    {
      const SMESH_NodeXYZ& n = segment->myFreeEnds[ iE ]->myNode;
      double d = Abs( signedDist( n, noData.myPlane ));
      dist2IntPntMap.insert( std::make_pair( d, segment->myFreeEnds[ iE ]));
    }
    std::multimap< double, const IntPoint* >::iterator d2ip = dist2IntPntMap.begin();
    SMESH_MeshAlgos::PolySegment linkNodes;
    linkNodes.myXYZ[0] = d2ip->second->myNode;
    linkNodes.myXYZ[1] = (++d2ip)->second->myNode;
    linkNodes.myVector = noData.myPlane.Direction() ^ (linkNodes.myXYZ[0] - linkNodes.myXYZ[1]);
    linkNodes.myNode1[ 0 ] = linkNodes.myNode2[ 0 ] = 0;
    linkNodes.myNode1[ 1 ] = linkNodes.myNode2[ 1 ] = 0;

    // create segments connecting linkNodes
    std::vector<const SMDS_MeshElement*> newSegments;
    std::vector<const SMDS_MeshNode*>    newNodes;
    SMESH_MeshAlgos::TListOfPolySegments polySegs(1, linkNodes);
    SMESH_MeshAlgos::MakePolyLine( theMesh, polySegs, newSegments, newNodes,
                                   /*group=*/0, faceSearcher.get() );
    // cut faces by newSegments
    intPoints.resize(2);
    for ( size_t i = 0; i < newSegments.size(); ++i )
    {
      intPoints[0].myNode = edgeNodes[0] = newSegments[i]->GetNode(0);
      intPoints[1].myNode = edgeNodes[1] = newSegments[i]->GetNode(1);

      // find an underlying face
      gp_XYZ                middle = 0.5 * ( intPoints[0].myNode + intPoints[1].myNode );
      const SMDS_MeshElement* face = faceSearcher->FindClosestTo( middle, SMDSAbs_Face );

      // find intersected edges of the face
      int nbNodes = face->NbCornerNodes();
      faceNodes.assign( face->begin_nodes(), face->end_nodes() );
      faceNodes.resize( nbNodes + 1 );
      faceNodes[ nbNodes ] = faceNodes[ 0 ];
      for ( int iP = 0; iP < 2; ++iP )
      {
        intPoints[iP].myEdgeIndex = -1;
        for ( int iN = 0; iN < nbNodes &&  intPoints[iP].myEdgeIndex < 0; ++iN )
        {
          if ( isOnEdge( faceNodes[iN], faceNodes[iN+1], intPoints[iP].myNode, tol ))
            intPoints[iP].myEdgeIndex = iN;
        }
      }


      // face cut
      computeNormal( face, faceNormals );
      meshIntersector.Cut( face,
                           intPoints[0].myNode, intPoints[0].myEdgeIndex,
                           intPoints[1].myNode, intPoints[1].myEdgeIndex );

      Edge e = { intPoints[0].myNode.Node(), intPoints[1].myNode.Node(), 0 };
      bndEdges.push_back( e );

      findGroups( face, theGroupsToUpdate, faceID2Groups, groupVec );

      // add cut points to an adjacent face at ends of poly-line
      // if they fall onto face edges
      if (( i == 0                       && intPoints[0].myEdgeIndex >= 0 ) ||
          ( i == newSegments.size() - 1  && intPoints[1].myEdgeIndex >= 0 ))
      {
        for ( int iE = 0; iE < 2; ++iE ) // loop on ends of a new segment
        {
          if ( iE ? ( i != newSegments.size() - 1 ) : ( i != 0 ))
            continue;
          int iEdge = intPoints[ iE ].myEdgeIndex;
          edgeNodes[0] = faceNodes[ iEdge ];
          edgeNodes[1] = faceNodes[ iEdge+1 ];
          theMesh->GetElementsByNodes( edgeNodes, faces, SMDSAbs_Face );
          for ( size_t iF = 0; iF < faces.size(); ++iF )
            if ( faces[iF] != face )
            {
              int iN1 = faces[iF]->GetNodeIndex( edgeNodes[0] );
              int iN2 = faces[iF]->GetNodeIndex( edgeNodes[1] );
              intPoints[ iE ].myEdgeIndex = Abs( iN1 - iN2 ) == 1 ? Min( iN1, iN2 ) : 2;
              computeNormal( faces[iF], faceNormals );
              meshIntersector.Cut( faces[iF],
                                   intPoints[iE].myNode, intPoints[iE].myEdgeIndex,
                                   intPoints[iE].myNode, intPoints[iE].myEdgeIndex );

              findGroups( faces[iF], theGroupsToUpdate, faceID2Groups, groupVec );
            }
        }
      }

    } // loop on newSegments

    polySegments.insert( polySegments.end(), newSegments.begin(), newSegments.end() );

  } // loop on map of input segments

  // actual mesh splitting
  TElemIntPairVec new2OldFaces;
  TNodeIntPairVec new2OldNodes;
  meshIntersector.MakeNewFaces( new2OldFaces, new2OldNodes, /*sign=*/1, /*optimize=*/true );

  // add new faces to theGroupsToUpdate
  for ( size_t i = 0; i < new2OldFaces.size(); ++i )
  {
    const SMDS_MeshElement* newFace = new2OldFaces[i].first;
    const int             oldFaceID = new2OldFaces[i].second;
    if ( !newFace ) continue;

    if ( TGroupVec* groups = const_cast< TGroupVec* >( faceID2Groups.Seek( oldFaceID )))
      for ( size_t iG = 0; iG < groups->size(); ++iG )
        (*groups)[ iG ]->Add( newFace );
  }

  // remove poly-line edges
  for ( size_t i = 0; i < polySegments.size(); ++i )
  {
    edgeNodes[0] = polySegments[i]->GetNode(0);
    edgeNodes[1] = polySegments[i]->GetNode(1);

    theMesh->RemoveFreeElement( polySegments[i] );

    if ( edgeNodes[0]->NbInverseElements() == 0 )
      theMesh->RemoveNode( edgeNodes[0] );
    if ( edgeNodes[1]->NbInverseElements() == 0 )
      theMesh->RemoveNode( edgeNodes[1] );
  }

  return bndEdges;
}
