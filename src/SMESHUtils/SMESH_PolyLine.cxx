// Copyright (C) 2018-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File      : SMESH_PolyLine.cxx
// Created   : Thu Dec  6 17:33:26 2018
// Author    : Edward AGAPOV (eap)

#include "SMESH_MeshAlgos.hxx"

#include "SMDS_MeshGroup.hxx"
#include "SMDS_LinearEdge.hxx"
#include "SMDS_Mesh.hxx"
#include "SMESH_TryCatch.hxx"

#include <OSD_Parallel.hxx>
#include <Precision.hxx>

namespace
{
  //================================================================================
  /*!
   * \brief Sequence of found points and a current point data
   */
  struct Path
  {
    std::vector< gp_XYZ >   myPoints;
    double                  myLength;

    const SMDS_MeshElement* myFace;
    SMESH_NodeXYZ           myNode1; // nodes of the edge the path entered myFace
    SMESH_NodeXYZ           myNode2;
    int                     myNodeInd1;
    int                     myNodeInd2;
    double                  myDot1;
    double                  myDot2;

    int                     mySrcPntInd; //!< start point index
    TIDSortedElemSet        myElemSet, myAvoidSet;

    Path(const SMDS_MeshElement* face=0, int srcInd=-1):
      myLength(0.0), myFace(face), mySrcPntInd( srcInd ) {}

    void CopyNoPoints( const Path& other );

    bool Extend( const gp_XYZ& plnNorm, const gp_XYZ& plnOrig, std::vector< Path > * paths = 0 );

    bool SetCutAtCorner( const SMESH_NodeXYZ&    cornerNode,
                         const gp_XYZ&           plnNorm,
                         const gp_XYZ&           plnOrig,
                         std::vector< Path >*    paths);

    bool SetCutAtCorner( const SMESH_NodeXYZ&    cornerNode,
                         const SMDS_MeshElement* face,
                         const gp_XYZ&           plnNorm,
                         const gp_XYZ&           plnOrig );

    void AddPoint( const gp_XYZ& p );

    bool ReachSamePoint( const Path& other );

    static void Remove( std::vector< Path > & paths, size_t& i );
  };

  //================================================================================
  /*!
   * \brief Return true if this Path meats another
   */
  //================================================================================

  bool Path::ReachSamePoint( const Path& other )
  {
    return ( mySrcPntInd != other.mySrcPntInd &&
             myFace == other.myFace );
  }

  //================================================================================
  /*!
   * \brief Copy data except points
   */
  //================================================================================

  void Path::CopyNoPoints( const Path& other )
  {
    myLength    = other.myLength;
    mySrcPntInd = other.mySrcPntInd;
    myFace      = other.myFace;
    myNode1     = other.myNode1;
    myNode2     = other.myNode2;
    myNodeInd1  = other.myNodeInd1;
    myNodeInd2  = other.myNodeInd2;
    myDot1      = other.myDot1;
    myDot2      = other.myDot2;
  }

  //================================================================================
  /*!
   * \brief Remove a path from a vector
   */
  //================================================================================

  void Path::Remove( std::vector< Path > & paths, size_t& i )
  {
    if ( paths.size() > 1 )
    {
      size_t j = paths.size() - 1; // last item to be removed
      if ( i < j )
      {
        paths[ i ].CopyNoPoints ( paths[ j ]);
        paths[ i ].myPoints.swap( paths[ j ].myPoints );
      }
    }
    paths.pop_back();
    if ( i > 0 )
      --i;
  }

  //================================================================================
  /*!
   * \brief Try to extend self by a point located at a node.
   *        Return a success flag.
   */
  //================================================================================

  bool Path::SetCutAtCorner( const SMESH_NodeXYZ&  cornerNode,
                             const gp_XYZ&         plnNorm,
                             const gp_XYZ&         plnOrig,
                             std::vector< Path > * paths )
  {
    bool ok = false;
    const bool isContinuation = myFace; // extend this path or find all possible paths?
    const SMDS_MeshElement* lastFace = myFace;

    myAvoidSet.clear();

    SMDS_ElemIteratorPtr fIt = cornerNode->GetInverseElementIterator(SMDSAbs_Face);
    while ( fIt->more() )
    {
      Path path( lastFace, mySrcPntInd );
      if ( !path.SetCutAtCorner( cornerNode, fIt->next(), plnNorm, plnOrig ))
        continue;

      if ( path.myDot1 == 0 &&
           !myAvoidSet.insert( path.myNode1.Node() ).second )
        continue;
      if ( path.myDot2 == 0 &&
           !myAvoidSet.insert( path.myNode2.Node() ).second )
        continue;

      if ( isContinuation )
      {
        if ( ok ) // non-manifold continuation
        {
          path.myPoints = myPoints;
          path.myLength = myLength;
          path.AddPoint( cornerNode );
          paths->push_back( path );
        }
        else
        {
          double len = myLength;
          this->CopyNoPoints( path );
          this->myLength = len;
          this->AddPoint( path.myPoints.back() );
        }
      }
      else
      {
        paths->push_back( path );
      }
      ok = true;
    }

    return ok;
  }

  //================================================================================
  /*!
   * \brief Store a point that is at a node of a face if the face is intersected by plane.
   *        Return false if the node is a sole intersection point of the face and the plane
   */
  //================================================================================

  bool Path::SetCutAtCorner( const SMESH_NodeXYZ&    cornerNode,
                             const SMDS_MeshElement* face,
                             const gp_XYZ&           plnNorm,
                             const gp_XYZ&           plnOrig )
  {
    if ( face == myFace )
      return false;
    myNodeInd1 = face->GetNodeIndex( cornerNode._node );
    myNodeInd2 = ( myNodeInd1 + 1 ) % face->NbCornerNodes();
    int   ind3 = ( myNodeInd1 + 2 ) % face->NbCornerNodes();
    myNode1.Set( face->GetNode( ind3 ));
    myNode2.Set( face->GetNode( myNodeInd2 ));

    myDot1 = plnNorm * ( myNode1 - plnOrig );
    myDot2 = plnNorm * ( myNode2 - plnOrig );

    bool ok = ( myDot1 * myDot2 < 0 );
    if ( !ok && myDot1 * myDot2 == 0 )
    {
      ok = ( myDot1 != myDot2 );
      if ( ok && myFace )
        ok = ( myFace->GetNodeIndex(( myDot1 == 0 ? myNode1 : myNode2 )._node ) < 0 );
    }
    if ( ok )
    {
      myFace = face;
      myDot1 = 0;
      AddPoint( cornerNode );
    }
    return ok;
  }

  //================================================================================
  /*!
   * \brief Store a point and update myLength
   */
  //================================================================================

  void Path::AddPoint( const gp_XYZ& p )
  {
    if ( !myPoints.empty() )
      myLength += ( p - myPoints.back() ).Modulus();
    else
      myLength = 0;
    myPoints.push_back( p );
  }

  //================================================================================
  /*!
   * \brief Try to find the next point
   *  \param [in] plnNorm - cutting plane normal
   *  \param [in] plnOrig - cutting plane origin
   *  \param [in] paths - all paths
   */
  //================================================================================

  bool Path::Extend( const gp_XYZ& plnNorm, const gp_XYZ& plnOrig, std::vector< Path > * paths )
  {
    bool ok = false;

    int nodeInd3 = ( myNodeInd1 + 1 ) % myFace->NbCornerNodes();
    if ( myNodeInd2 == nodeInd3 )
      nodeInd3 = ( myNodeInd1 + 2 ) % myFace->NbCornerNodes();

    SMESH_NodeXYZ node3 = myFace->GetNode( nodeInd3 );
    double         dot3 = plnNorm * ( node3 - plnOrig );

    if ( dot3 * myDot1 < 0. )
    {
      myNode2    = node3;
      myNodeInd2 = nodeInd3;
      myDot2     = dot3;
    }
    else if ( dot3 * myDot2 < 0. )
    {
      myNode1    = node3;
      myNodeInd1 = nodeInd3;
      myDot1     = dot3;
    }
    else if ( dot3 == 0. )
    {
      ok = SetCutAtCorner( node3, plnNorm, plnOrig, paths );
      return ok;
    }
    else if ( myDot2 == 0. )
    {
      ok = SetCutAtCorner( myNode2, plnNorm, plnOrig, paths );
      return ok;
    }

    double r = Abs( myDot1 / ( myDot2 - myDot1 ));
    AddPoint( myNode1 * ( 1 - r ) + myNode2 * r );

    myAvoidSet.clear();
    myAvoidSet.insert( myFace );
    const SMDS_MeshElement* nextFace;
    int ind1, ind2;
    while (( nextFace = SMESH_MeshAlgos::FindFaceInSet( myNode1._node, myNode2._node,
                                                        myElemSet, myAvoidSet,
                                                        &ind1, &ind2 )))
    {
      if ( ok ) // non-manifold continuation
      {
        paths->push_back( *this );
        paths->back().myFace = nextFace;
        paths->back().myNodeInd1 = ind1;
        paths->back().myNodeInd2 = ind2;
      }
      else
      {
        myFace = nextFace;
        myNodeInd1 = ind1;
        myNodeInd2 = ind2;
      }
      ok = true;
      if ( !paths )
        break;
      myAvoidSet.insert( nextFace );
    }

    return ok;
  }

  //================================================================================
  /*!
   * \brief Compute a path between two points of PolySegment
   */
  struct PolyPathCompute
  {
    SMESH_MeshAlgos::TListOfPolySegments& mySegments; //!< inout PolySegment's
    std::vector< Path >&                  myPaths;    //!< path of each of segments to compute
    SMDS_Mesh*                            myMesh;
    mutable std::vector< std::string >    myErrors;

    PolyPathCompute( SMESH_MeshAlgos::TListOfPolySegments& theSegments,
                     std::vector< Path >&                  thePaths,
                     SMDS_Mesh*                            theMesh):
      mySegments( theSegments ),
      myPaths( thePaths ),
      myMesh( theMesh ),
      myErrors( theSegments.size() )
    {
    }

#undef SMESH_CAUGHT
#define SMESH_CAUGHT myErrors[i] =
    void operator() ( const int i ) const
    {
      SMESH_TRY;
      const_cast< PolyPathCompute* >( this )->Compute( i );
      SMESH_CATCH( SMESH::returnError );
    }
#undef SMESH_CAUGHT

    //================================================================================
    /*!
     * \brief Compute a path of a given segment
     */
    //================================================================================

    void Compute( const int iSeg )
    {
      SMESH_MeshAlgos::PolySegment& polySeg = mySegments[ iSeg ];

      if ( ( polySeg.myXYZ[0] - polySeg.myXYZ[1] ).SquareModulus() == 0 )
      {
        myPaths[ iSeg ].AddPoint( polySeg.myXYZ[0] );
        myPaths[ iSeg ].AddPoint( polySeg.myXYZ[1] );
        return;
      }

      // the cutting plane
      gp_XYZ plnNorm = ( polySeg.myXYZ[0] - polySeg.myXYZ[1] ) ^ polySeg.myVector.XYZ();
      gp_XYZ plnOrig = polySeg.myXYZ[1];

      // Find paths connecting the 2 end points of polySeg

      std::vector< Path > paths; paths.reserve(10);

      // 1) initialize paths; two paths starts at each end point

      for ( int iP = 0; iP < 2; ++iP ) // loop on the polySeg end points
      {
        Path path( 0, iP );
        size_t nbPaths = paths.size();

        if ( polySeg.myFace[ iP ]) // the end point lies on polySeg.myFace[ iP ]
        {
          // check coincidence of polySeg.myXYZ[ iP ] with nodes
          const double tol = 1e-17;
          SMESH_NodeXYZ nodes[4];
          for ( int i = 0; i < 3 &&  !polySeg.myNode1[ iP ]; ++i )
          {
            nodes[ i ] = polySeg.myFace[ iP ]->GetNode( i );
            if (( nodes[ i ] - polySeg.myXYZ[ iP ]).SquareModulus() < tol*tol )
              polySeg.myNode1[ iP ] = nodes[ i ].Node();
          }
          nodes[ 3 ] = nodes[ 0 ];

          double dot[ 4 ];
          for ( int i = 0; i < 3; ++i )
            dot[ i ] = plnNorm * ( nodes[ i ] - plnOrig );
          dot[ 3 ] = dot[ 0 ];

          // check coincidence of polySeg.myXYZ[ iP ] with edges
          for ( int i = 0; i < 3 &&  !polySeg.myNode1[ iP ]; ++i )
          {
            SMDS_LinearEdge edge( nodes[i].Node(), nodes[i+1].Node() );
            if ( SMESH_MeshAlgos::GetDistance( &edge, polySeg.myXYZ[ iP ]) < tol )
            {
              polySeg.myNode1[ iP ] = nodes[ i     ].Node();
              polySeg.myNode2[ iP ] = nodes[ i + 1 ].Node();

              int i3 = ( i + 2 ) % 3;
              if ( dot[ i   ] * dot [ i3 ] > 0 &&
                   dot[ i+1 ] * dot [ i3 ] > 0 ) // point iP is inside a neighbor triangle
              {
                path.myAvoidSet.insert( polySeg.myFace[ iP ]);
                const SMDS_MeshElement* face2 = 
                  SMESH_MeshAlgos::FindFaceInSet( polySeg.myNode1[ iP ],
                                                  polySeg.myNode2[ iP ],
                                                  path.myElemSet,
                                                  path.myAvoidSet );
                if ( face2 )
                  polySeg.myFace[ iP ] = face2;
                else
                  ;// ??
                for ( int i = 0; i < 3; ++i )
                {
                  nodes[ i ] = polySeg.myFace[ iP ]->GetNode( i );
                  dot[ i ] = plnNorm * ( nodes[ i ] - plnOrig );
                }
                dot[ 3 ] = dot[ 0 ];
                polySeg.myNode1[ iP ] = polySeg.myNode2[ iP ] = 0;
                break;
              }
            }
          }

          if ( !polySeg.myNode1[ iP ] ) // polySeg.myXYZ[ iP ] is within polySeg.myFace[ iP ]
          {
            int iCut = 0; // index of a cut edge
            if      ( dot[ 1 ] * dot[ 2 ] < 0. ) iCut = 1;
            else if ( dot[ 2 ] * dot[ 3 ] < 0. ) iCut = 2;

            // initialize path so as if it entered the face via iCut-th edge
            path.myFace = polySeg.myFace[ iP ];
            path.myNodeInd1 = iCut;
            path.myNodeInd2 = iCut + 1;
            path.myNode1.Set( nodes[ iCut     ].Node() );
            path.myNode2.Set( nodes[ iCut + 1 ].Node() );
            path.myDot1 = dot[ iCut ];
            path.myDot2 = dot[ iCut + 1 ];
            path.myPoints.clear();
            path.AddPoint( polySeg.myXYZ[ iP ]);
            paths.push_back( path );

            path.Extend( plnNorm, plnOrig ); // to get another edge cut
            path.myFace = polySeg.myFace[ iP ];
            if ( path.myDot1 == 0. ) // cut at a node
            {
              path.myNodeInd1 = ( iCut + 2 ) % 3;
              path.myNodeInd2 = ( iCut + 3 ) % 3;
              path.myNode2.Set( path.myFace->GetNode( path.myNodeInd2 ));
              path.myDot2 = dot[ path.myNodeInd2 ];
            }
            else
            {
              path.myNodeInd1 = path.myFace->GetNodeIndex( path.myNode1.Node() );
              path.myNodeInd2 = path.myFace->GetNodeIndex( path.myNode2.Node() );
            }
            path.myPoints.clear();
            path.AddPoint( polySeg.myXYZ[ iP ]);
            paths.push_back( path );
          }
        }

        if ( polySeg.myNode2[ iP ] && polySeg.myNode2[ iP ] != polySeg.myNode1[ iP ] )
        {
          // the end point is on an edge
          while (( path.myFace = SMESH_MeshAlgos::FindFaceInSet( polySeg.myNode1[ iP ],
                                                                 polySeg.myNode2[ iP ],
                                                                 path.myElemSet,
                                                                 path.myAvoidSet,
                                                                 &path.myNodeInd1,
                                                                 &path.myNodeInd2 )))
          {
            path.myNode1.Set( polySeg.myNode1[ iP ]);
            path.myNode2.Set( polySeg.myNode2[ iP ]);
            path.myDot1 = plnNorm * ( path.myNode1 - plnOrig );
            path.myDot2 = plnNorm * ( path.myNode2 - plnOrig );
            path.myPoints.clear();
            path.AddPoint( polySeg.myXYZ[ iP ]);
            path.myAvoidSet.insert( path.myFace );
            paths.push_back( path );
            std::swap( polySeg.myNode1[ iP ], polySeg.myNode2[ iP ]);
          }
          if ( nbPaths == paths.size() )
            throw SALOME_Exception ( SMESH_Comment("No face edge found by point ") << iP+1
                                     << " in a PolySegment " << iSeg );

          if ( path.myDot1 == 0. &&
               path.myDot2 == 0. )
          {
            if ( paths.size() - nbPaths >= 2 ) // use a face non-parallel to the plane
            {
              const SMDS_MeshElement* goodFace = 0;
              for ( size_t j = nbPaths; j < paths.size(); ++j )
              {
                path = paths[j];
                if ( path.Extend( plnNorm, plnOrig ))
                  goodFace = paths[j].myFace;
                else
                  paths[j].myFace = 0;
              }
              if ( !goodFace )
                throw SALOME_Exception ( SMESH_Comment("Cant move from point ") << iP+1
                                         << " of a PolySegment " << iSeg );
              for ( size_t j = nbPaths; j < paths.size(); ++j )
                if ( !paths[j].myFace )
                {
                  paths[j].myFace = goodFace;
                  paths[j].myNodeInd1 = goodFace->GetNodeIndex( paths[j].myNode1.Node() );
                  paths[j].myNodeInd2 = goodFace->GetNodeIndex( paths[j].myNode2.Node() );
                }
            }
            else // use the sole found face
            {
              path = paths.back();
              std::swap( path.myNode1,    path.myNode2 );
              std::swap( path.myNodeInd1, path.myNodeInd2 );
              paths.push_back( path );
            }
          }
        }

        else if ( polySeg.myNode1[ iP ] ) // the end point is at a node
        {
          path.myFace = 0;
          path.SetCutAtCorner( polySeg.myNode1[ iP ], plnNorm, plnOrig, &paths );
        }


        // look for a one-segment path
        for ( size_t i = 0; i < nbPaths; ++i )
          for ( size_t j = nbPaths; j < paths.size(); ++j )
            if ( paths[i].myFace == paths[j].myFace )
            {
              myPaths[ iSeg ].myPoints.push_back( paths[i].myPoints[0] );
              myPaths[ iSeg ].myPoints.push_back( paths[j].myPoints[0] );
              paths.clear();
            }

      }  // loop on the polySeg end points to initialize all possible paths


      // 2) extend paths and compose the shortest one connecting the two points

      myPaths[ iSeg ].myLength = 1e100;

      while ( paths.size() >= 2 )
      {
        for ( size_t i = 0; i < paths.size(); ++i )
        {
          Path& path = paths[ i ];
          if ( !path.Extend( plnNorm, plnOrig, &paths ) || // path reached a mesh boundary
               path.myLength > myPaths[ iSeg ].myLength )  // path is longer than others
          {
            Path::Remove( paths, i );
            continue;
          }

          // join paths that reach same point
          for ( size_t j = 0; j < paths.size(); ++j )
          {
            if ( i != j && paths[i].ReachSamePoint( paths[j] ))
            {
              double   distLast = ( paths[i].myPoints.back() - paths[j].myPoints.back() ).Modulus();
              double fullLength = ( paths[i].myLength + paths[j].myLength + distLast );
              if ( fullLength < myPaths[ iSeg ].myLength )
              {
                myPaths[ iSeg ].myLength = fullLength;
                std::vector< gp_XYZ > & allPoints = myPaths[ iSeg ].myPoints;
                allPoints.swap( paths[i].myPoints );
                allPoints.insert( allPoints.end(),
                                  paths[j].myPoints.rbegin(),
                                  paths[j].myPoints.rend() );
              }
              if ( i < j ) std::swap( i, j );
              Path::Remove( paths, i );
              Path::Remove( paths, j );
              break;
            }
          }
        }
        if ( !paths.empty() && (int) paths[0].myPoints.size() > myMesh->NbFaces() )
          throw SALOME_Exception(LOCALIZED( "Infinite loop in MakePolyLine()"));
      }

      if ( myPaths[ iSeg ].myPoints.empty() )
        throw SALOME_Exception( SMESH_Comment("Can't find a full path for PolySegment #") << iSeg );

      // reverse the path
      double d00 = ( polySeg.myXYZ[0] - myPaths[ iSeg ].myPoints.front() ).SquareModulus();
      double d01 = ( polySeg.myXYZ[0] - myPaths[ iSeg ].myPoints.back()  ).SquareModulus();
      if ( d00 > d01 )
        std::reverse( myPaths[ iSeg ].myPoints.begin(), myPaths[ iSeg ].myPoints.end() );

    } // PolyPathCompute::Compute()

  }; // struct PolyPathCompute

} // namespace

//=======================================================================
//function : MakePolyLine
//purpose  : Create a polyline consisting of 1D mesh elements each lying on a 2D element of
//           the initial mesh
//=======================================================================

void SMESH_MeshAlgos::MakePolyLine( SMDS_Mesh*                            theMesh,
                                    TListOfPolySegments&                  theSegments,
                                    std::vector<const SMDS_MeshElement*>& theNewEdges,
                                    std::vector< const SMDS_MeshNode* >&  theNewNodes,
                                    SMDS_MeshGroup*                       theGroup,
                                    SMESH_ElementSearcher*                theSearcher)
{
  std::vector< Path > segPaths( theSegments.size() ); // path of each of segments

  SMESH_ElementSearcher* searcher = theSearcher;
  SMESHUtils::Deleter<SMESH_ElementSearcher> delSearcher;
  if ( !searcher )
  {
    searcher = SMESH_MeshAlgos::GetElementSearcher( *theMesh );
    delSearcher._obj = searcher;
  }

  // get cutting planes

  std::vector< bool > isVectorOK( theSegments.size(), true );
  const double planarCoef = 0.333; // plane height in planar case

  for ( size_t iSeg = 0; iSeg < theSegments.size(); ++iSeg )
  {
    PolySegment& polySeg = theSegments[ iSeg ];

    gp_XYZ p1 = SMESH_NodeXYZ( polySeg.myNode1[0] );
    gp_XYZ p2 = SMESH_NodeXYZ( polySeg.myNode1[1] );
    if ( polySeg.myNode2[0] ) p1 = 0.5 * ( p1 + SMESH_NodeXYZ( polySeg.myNode2[0] ));
    if ( polySeg.myNode2[1] ) p2 = 0.5 * ( p2 + SMESH_NodeXYZ( polySeg.myNode2[1] ));

    polySeg.myFace[0] = polySeg.myFace[1] = 0;
    if ( !polySeg.myNode1[0] && !polySeg.myNode2[0] )
    {
      p1 = searcher->Project( polySeg.myXYZ[0], SMDSAbs_Face, &polySeg.myFace[0] );
    }
    if ( !polySeg.myNode1[1] && !polySeg.myNode2[1] )
    {
      p2 = searcher->Project( polySeg.myXYZ[1], SMDSAbs_Face, &polySeg.myFace[1] );
    }
    polySeg.myXYZ[0] = p1;
    polySeg.myXYZ[1] = p2;

    gp_XYZ plnNorm = ( p1 - p2 ) ^ polySeg.myVector.XYZ();

    isVectorOK[ iSeg ] = ( plnNorm.Modulus() > std::numeric_limits<double>::min() );
    if ( !isVectorOK[ iSeg ] && ( p1 - p2 ).SquareModulus() > 0. )
    {
      gp_XYZ pMid = 0.5 * ( p1 + p2 );
      const SMDS_MeshElement* face;
      polySeg.myMidProjPoint = searcher->Project( pMid, SMDSAbs_Face, &face );
      polySeg.myVector       = polySeg.myMidProjPoint.XYZ() - pMid;

      gp_XYZ faceNorm;
      SMESH_MeshAlgos::FaceNormal( face, faceNorm, /*normalized=*/false );

      const double tol = Precision::Confusion();
      if ( polySeg.myVector.Magnitude() < tol || polySeg.myVector * faceNorm  < tol )
      {
        polySeg.myVector = faceNorm;
        polySeg.myMidProjPoint = pMid + faceNorm * ( p1 - p2 ).Modulus() * planarCoef;
      }
      plnNorm = ( p1 - p2 ) ^ polySeg.myVector.XYZ();
      if ( plnNorm.SquareModulus() == 0 ) // p1-p2 perpendicular to mesh
      {
        double radius = faceNorm.Modulus();
        std::vector< const SMDS_MeshElement* > foundElems;
        while ( plnNorm.SquareModulus() == 0  &&  radius < 1e200 )
        {
          foundElems.clear();
          searcher->GetElementsInSphere( p1, radius, SMDSAbs_Face, foundElems );
          searcher->GetElementsInSphere( p2, radius, SMDSAbs_Face, foundElems );
          radius *= 2;
          polySeg.myVector.SetCoord( 0,0,0 );
          for ( size_t i = 0; i < foundElems.size(); ++i )
          {
            SMESH_MeshAlgos::FaceNormal( foundElems[i], faceNorm );
            polySeg.myVector += faceNorm / foundElems.size();
          }
          plnNorm = ( p1 - p2 ) ^ polySeg.myVector.XYZ();
        }
      }
      
    }
    else
    {
      polySeg.myVector = plnNorm ^ ( p1 - p2 );
    }
  }

  // assure that inverse elements are constructed, avoid their concurrent building in threads
  theMesh->nodesIterator()->next()->NbInverseElements();

  // find paths

  PolyPathCompute algo( theSegments, segPaths, theMesh );
  OSD_Parallel::For( 0, theSegments.size(), algo, theSegments.size() == 1 );

  for ( size_t iSeg = 0; iSeg < theSegments.size(); ++iSeg )
    if ( !algo.myErrors[ iSeg ].empty() )
      throw SALOME_Exception( algo.myErrors[ iSeg ].c_str() );

  // create an 1D mesh

  const SMDS_MeshNode *n, *nPrev = 0;

  for ( size_t iSeg = 0; iSeg < theSegments.size(); ++iSeg )
  {
    const Path& path = segPaths[iSeg];
    if ( path.myPoints.size() < 2 )
      continue;

    double tol = path.myLength / path.myPoints.size() / 1000.;
    if ( !nPrev || ( SMESH_NodeXYZ( nPrev ) - path.myPoints[0] ).SquareModulus() > tol*tol )
    {
      nPrev = theMesh->AddNode( path.myPoints[0].X(), path.myPoints[0].Y(), path.myPoints[0].Z() );
      theNewNodes.push_back( nPrev );
    }
    for ( size_t iP = 1; iP < path.myPoints.size(); ++iP )
    {
      n = theMesh->AddNode( path.myPoints[iP].X(), path.myPoints[iP].Y(), path.myPoints[iP].Z() );
      theNewNodes.push_back( n );

      const SMDS_MeshElement* elem = theMesh->AddEdge( nPrev, n );
      theNewEdges.push_back( elem );
      if ( theGroup )
        theGroup->Add( elem );

      nPrev = n;
    }

    // return a vector

    gp_XYZ pMid = 0.5 * ( path.myPoints[0] + path.myPoints.back() );
    if ( isVectorOK[ iSeg ])
    {
      // find the most distant point of a path
      double maxDist = 0;
      for ( size_t iP = 1; iP < path.myPoints.size(); ++iP )
      {
        double dist = Abs( theSegments[iSeg].myVector * ( path.myPoints[iP] - path.myPoints[0] ));
        if ( dist > maxDist )
        {
          maxDist = dist;
          theSegments[iSeg].myMidProjPoint = path.myPoints[iP];
        }
      }
      if ( maxDist < Precision::Confusion() ) // planar case
        theSegments[iSeg].myMidProjPoint =
          pMid + theSegments[iSeg].myVector.XYZ().Normalized() * path.myLength * planarCoef;
    }
    theSegments[iSeg].myVector = gp_Vec( pMid, theSegments[iSeg].myMidProjPoint );
  }

  return;
}
