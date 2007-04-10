//  SMESH SMESH : idl implementation based on 'SMESH' unit's calsses
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
// File      : StdMeshers_ProjectionUtils.cxx
// Created   : Fri Oct 27 10:24:28 2006
// Author    : Edward AGAPOV (eap)


#include "StdMeshers_ProjectionUtils.hxx"

#include "StdMeshers_ProjectionSource1D.hxx"
#include "StdMeshers_ProjectionSource2D.hxx"
#include "StdMeshers_ProjectionSource3D.hxx"

#include "SMESH_Algo.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_IndexedDataMapOfShapeIndexedMapOfShape.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMDS_EdgePosition.hxx"

#include "utilities.h"

#include <BRepAdaptor_Curve.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <TopAbs.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>


#define RETURN_BAD_RESULT(msg) { MESSAGE(msg); return false; }
#define SHOW_VERTEX(v,msg) // { \
//  if ( v.IsNull() ) cout << msg << " NULL SHAPE" << endl; \
// else if (v.ShapeType() == TopAbs_VERTEX) {\
//   gp_Pnt p = BRep_Tool::Pnt( TopoDS::Vertex( v ));\
//   cout << msg << (v).TShape().operator->()<<" ( " <<p.X()<<", "<<p.Y()<<", "<<p.Z()<<" )"<<endl;}\
// else {\
// cout << msg << " "; TopAbs::Print(v.ShapeType(),cout) <<" "<<(v).TShape().operator->()<<endl;}\
// }
#define SHOW_LIST(msg,l) \
// { \
//     cout << msg << " ";\
//     list< TopoDS_Edge >::const_iterator e = l.begin();\
//     for ( int i = 0; e != l.end(); ++e, ++i ) {\
//       cout << i << "V (" << TopExp::FirstVertex( *e, true ).TShape().operator->() << ") "\
//            << i << "E (" << e->TShape().operator->() << "); "; }\
//     cout << endl;\
//   }

namespace {
  //================================================================================
  /*!
   * \brief Reverse order of edges in a list and their orientation
    * \param edges - list of edges to reverse
    * \param nbEdges - number of edges to reverse
   */
  //================================================================================

  void Reverse( list< TopoDS_Edge > & edges, const int nbEdges )
  {
    SHOW_LIST("BEFORE REVERSE", edges);

    list< TopoDS_Edge >::iterator eIt = edges.begin();
    if ( edges.size() == nbEdges )
    {
      edges.reverse();
    }
    else  // reverse only the given nb of edges
    {
      // look for the last edge to be reversed
      list< TopoDS_Edge >::iterator eBackIt = edges.begin();
      for ( int i = 1; i < nbEdges; ++i )
        ++eBackIt;
      // reverse
      while ( eIt != eBackIt ) {
        std::swap( *eIt, *eBackIt );
        SHOW_LIST("# AFTER SWAP", edges)
        if ( (++eIt) != eBackIt )
          --eBackIt;
      }
    }
    for ( eIt = edges.begin(); eIt != edges.end(); ++eIt )
      eIt->Reverse();
    SHOW_LIST("ATFER REVERSE", edges)
  }

  //================================================================================
  /*!
   * \brief Check if propagation is possible
    * \param theMesh1 - source mesh
    * \param theMesh2 - target mesh
    * \retval bool - true if possible
   */
  //================================================================================

  bool IsPropagationPossible( SMESH_Mesh* theMesh1, SMESH_Mesh* theMesh2 )
  {
    if ( theMesh1 != theMesh2 ) {
      TopoDS_Shape mainShape1 = theMesh1->GetMeshDS()->ShapeToMesh();
      TopoDS_Shape mainShape2 = theMesh2->GetMeshDS()->ShapeToMesh();
      return mainShape1.IsSame( mainShape2 );
    }
    return true;
  }

  //================================================================================
  /*!
   * \brief Fix up association of edges in faces by possible propagation
    * \param nbEdges - nb of edges in an outer wire
    * \param edges1 - edges of one face
    * \param edges2 - matching edges of another face
    * \param theMesh1 - mesh 1
    * \param theMesh2 - mesh 2
   */
  //================================================================================

  void FixAssocByPropagation( const int             nbEdges,
                              list< TopoDS_Edge > & edges1,
                              list< TopoDS_Edge > & edges2,
                              SMESH_Mesh*           theMesh1,
                              SMESH_Mesh*           theMesh2)
  {
    if ( nbEdges == 2 && IsPropagationPossible( theMesh1, theMesh2 ) )
    {
      list< TopoDS_Edge >::iterator eIt2 = ++edges2.begin(); // 2nd edge of the 2nd face
      TopoDS_Edge edge2 =
        StdMeshers_ProjectionUtils::GetPropagationEdge( theMesh1, *eIt2, edges1.front() );
      if ( !edge2.IsNull() ) // propagation found for the second edge
        Reverse( edges2, nbEdges );
    }
  }
}

//=======================================================================
/*!
 * \brief Looks for association of all subshapes of two shapes
 * \param theShape1 - shape 1
 * \param theMesh1 - mesh built on shape 1
 * \param theShape2 - shape 2
 * \param theMesh2 - mesh built on shape 2
 * \param theAssociation - association map to be filled that may
 *                         contain association of one or two pairs of vertices
 * \retval bool - true if association found
 */
//=======================================================================

bool StdMeshers_ProjectionUtils::FindSubShapeAssociation(const TopoDS_Shape& theShape1,
                                                         SMESH_Mesh*         theMesh1,
                                                         const TopoDS_Shape& theShape2,
                                                         SMESH_Mesh*         theMesh2,
                                                         TShapeShapeMap &    theMap)
{
  if ( theShape1.ShapeType() != theShape2.ShapeType() )
    RETURN_BAD_RESULT("Different shape types");

  bool bidirect = ( !theShape1.IsSame( theShape2 ));
  if ( !theMap.IsEmpty())
  {
    switch ( theShape1.ShapeType() ) {

    case TopAbs_EDGE: {
      // ----------------------------------------------------------------------
      if ( theMap.Extent() != 2 )
        RETURN_BAD_RESULT("Wrong map extent " << theMap.Extent() );
      TopoDS_Edge edge1 = TopoDS::Edge( theShape1 );
      TopoDS_Edge edge2 = TopoDS::Edge( theShape2 );
      TopoDS_Vertex VV1[2], VV2[2];
      TopExp::Vertices( edge1, VV1[0], VV1[1] );
      TopExp::Vertices( edge2, VV2[0], VV2[1] );
      int i1 = 0, i2 = 0;
      if ( theMap.IsBound( VV1[ i1 ] )) i1 = 1;
      if ( theMap.IsBound( VV2[ i2 ] )) i2 = 1;
      InsertAssociation( VV1[ i1 ], VV2[ i2 ], theMap, bidirect);
      return true;
    }

    case TopAbs_FACE: {
      // ----------------------------------------------------------------------
      TopoDS_Face face1 = TopoDS::Face( theShape1 );
      TopoDS_Face face2 = TopoDS::Face( theShape2 );

      TopoDS_Vertex VV1[2], VV2[2];
      // find a not closed edge of face1 both vertices of which are associated
      int nbEdges = 0;
      TopExp_Explorer exp ( face1, TopAbs_EDGE );
      for ( ; VV2[ 1 ].IsNull() && exp.More(); exp.Next(), ++nbEdges ) {
        TopExp::Vertices( TopoDS::Edge( exp.Current() ), VV1[0], VV1[1] );
        if ( theMap.IsBound( VV1[0] ) ) {
          VV2[ 0 ] = TopoDS::Vertex( theMap( VV1[0] ));
          if ( theMap.IsBound( VV1[1] ) && !VV1[0].IsSame( VV1[1] ))
            VV2[ 1 ] = TopoDS::Vertex( theMap( VV1[1] ));
        }
      }
      if ( VV2[ 1 ].IsNull() ) { // 2 bound vertices not found
        if ( nbEdges > 1 ) {
          RETURN_BAD_RESULT("2 bound vertices not found" );
        } else {
          VV2[ 1 ] = VV2[ 0 ];
        }
      }
      list< TopoDS_Edge > edges1, edges2;
      int nbE = FindFaceAssociation( face1, VV1, face2, VV2, edges1, edges2 );
      if ( !nbE ) RETURN_BAD_RESULT("FindFaceAssociation() failed");
      FixAssocByPropagation( nbE, edges1, edges2, theMesh1, theMesh2 );

      list< TopoDS_Edge >::iterator eIt1 = edges1.begin();
      list< TopoDS_Edge >::iterator eIt2 = edges2.begin();
      for ( ; eIt1 != edges1.end(); ++eIt1, ++eIt2 )
      {
        InsertAssociation( *eIt1, *eIt2, theMap, bidirect);
        VV1[0] = TopExp::FirstVertex( *eIt1, true );
        VV2[0] = TopExp::FirstVertex( *eIt2, true );
        InsertAssociation( VV1[0], VV2[0], theMap, bidirect);
      }
      return true;
    }

    case TopAbs_SHELL:
    case TopAbs_SOLID: {
      // ----------------------------------------------------------------------
      TopoDS_Vertex VV1[2], VV2[2];
      // find a not closed edge of shape1 both vertices of which are associated
      TopoDS_Edge edge1;
      TopExp_Explorer exp ( theShape1, TopAbs_EDGE );
      for ( ; VV2[ 1 ].IsNull() && exp.More(); exp.Next() ) {
        edge1 = TopoDS::Edge( exp.Current() );
        TopExp::Vertices( edge1 , VV1[0], VV1[1] );
        if ( theMap.IsBound( VV1[0] )) {
          VV2[ 0 ] = TopoDS::Vertex( theMap( VV1[0] ));
          if ( theMap.IsBound( VV1[1] ) && !VV1[0].IsSame( VV1[1] ))
            VV2[ 1 ] = TopoDS::Vertex( theMap( VV1[1] ));
        }
      }
      if ( VV2[ 1 ].IsNull() ) // 2 bound vertices not found
        RETURN_BAD_RESULT("2 bound vertices not found" );
      TopoDS_Edge edge2 = GetEdgeByVertices( theMesh2, VV2[ 0 ], VV2[ 1 ]);
      if ( edge2.IsNull() )
        RETURN_BAD_RESULT("GetEdgeByVertices() failed");

      // get a face sharing edge1
      TopoDS_Shape F1, F2, FF2[2];
      TopTools_ListIteratorOfListOfShape ancestIt = theMesh1->GetAncestors( edge1 );
      for ( ; F1.IsNull() && ancestIt.More(); ancestIt.Next() )
        if ( ancestIt.Value().ShapeType() == TopAbs_FACE )
          F1 = ancestIt.Value().Oriented( TopAbs_FORWARD );
      if ( F1.IsNull() )
        RETURN_BAD_RESULT(" Face1 not found");

      // get 2 faces sharing edge2
      ancestIt = theMesh2->GetAncestors( edge2 );
      for ( int i = 0; FF2[1].IsNull() && ancestIt.More(); ancestIt.Next() )
        if ( ancestIt.Value().ShapeType() == TopAbs_FACE )
          FF2[ i++ ] = ancestIt.Value().Oriented( TopAbs_FORWARD );
      if ( FF2[1].IsNull() )
        RETURN_BAD_RESULT("2 faces not found");

      // get oriented edge1 and edge2 from F1 and FF2[0]
      for ( exp.Init( F1, TopAbs_EDGE ); exp.More(); exp.Next() )
        if ( edge1.IsSame( exp.Current() )) {
          edge1 = TopoDS::Edge( exp.Current() );
          break;
        }
      
      for ( exp.Init( FF2[ 0 ], TopAbs_EDGE ); exp.More(); exp.Next() )
        if ( edge2.IsSame( exp.Current() )) {
          edge2 = TopoDS::Edge( exp.Current() );
          break;
        }

      // compare first vertices of edge1 and edge2
      TopExp::Vertices( edge1, VV1[0], VV1[1], true );
      TopExp::Vertices( edge2, VV2[0], VV2[1], true );
      F2 = FF2[ 0 ];
      if ( !VV1[ 0 ].IsSame( theMap( VV2[ 0 ]))) {
        F2 = FF2[ 1 ];
        edge2.Reverse();
      }

      TopTools_MapOfShape boundEdges; 

      // association of face subshapes and neighbour faces
      list< pair < TopoDS_Face, TopoDS_Edge > > FE1, FE2;
      list< pair < TopoDS_Face, TopoDS_Edge > >::iterator fe1, fe2;
      FE1.push_back( make_pair( TopoDS::Face( F1 ), edge1 ));
      FE2.push_back( make_pair( TopoDS::Face( F2 ), edge2 ));
      for ( fe1 = FE1.begin(), fe2 = FE2.begin(); fe1 != FE1.end(); ++fe1, ++fe2 )
      {
        const TopoDS_Face& face1 = fe1->first;
        if ( theMap.IsBound( face1 ) ) continue;
        const TopoDS_Face& face2 = fe2->first;
        edge1 = fe1->second;
        edge2 = fe2->second;
        TopExp::Vertices( edge1, VV1[0], VV1[1], true );
        TopExp::Vertices( edge2, VV2[0], VV2[1], true );
        list< TopoDS_Edge > edges1, edges2;
        int nbE = FindFaceAssociation( face1, VV1, face2, VV2, edges1, edges2 );
        if ( !nbE ) RETURN_BAD_RESULT("FindFaceAssociation() failed");
        FixAssocByPropagation( nbE, edges1, edges2, theMesh1, theMesh2 );
        
        InsertAssociation( face1, face2, theMap, bidirect); // assoc faces
        list< TopoDS_Edge >::iterator eIt1 = edges1.begin();
        list< TopoDS_Edge >::iterator eIt2 = edges2.begin();
        for ( ; eIt1 != edges1.end(); ++eIt1, ++eIt2 )
        {
          if ( !boundEdges.Add( *eIt1 )) continue; // already associated
          InsertAssociation( *eIt1, *eIt2, theMap, bidirect);  // assoc edges
          VV1[0] = TopExp::FirstVertex( *eIt1, true );
          VV2[0] = TopExp::FirstVertex( *eIt2, true );
          InsertAssociation( VV1[0], VV2[0], theMap, bidirect); // assoc vertices

          // add adjacent faces to process
          TopoDS_Face nextFace1 = GetNextFace( theMesh1, *eIt1, face1 );
          TopoDS_Face nextFace2 = GetNextFace( theMesh2, *eIt2, face2 );
          if ( !nextFace1.IsNull() && !nextFace2.IsNull() ) {
            FE1.push_back( make_pair( nextFace1, *eIt1 ));
            FE2.push_back( make_pair( nextFace2, *eIt2 ));
          }
        }
      }
      return true;
    }
    default:
      RETURN_BAD_RESULT("Unexpected shape type");

    } // end switch by shape type
  } // end case of available initial vertex association

  //----------------------------------------------------------------------
  // NO INITIAL VERTEX ASSOCIATION
  //----------------------------------------------------------------------

  switch ( theShape1.ShapeType() ) {

  case TopAbs_EDGE: {
    // ----------------------------------------------------------------------
    TopoDS_Edge edge1 = TopoDS::Edge( theShape1 );
    TopoDS_Edge edge2 = TopoDS::Edge( theShape2 );
    if ( IsPropagationPossible( theMesh1, theMesh2 ))
    {
      TopoDS_Edge prpEdge = GetPropagationEdge( theMesh1, edge2, edge1 );
      if ( !prpEdge.IsNull() )
      {
        TopoDS_Vertex VV1[2], VV2[2];
        TopExp::Vertices( edge1,   VV1[0], VV1[1], true );
        TopExp::Vertices( prpEdge, VV2[0], VV2[1], true );
        InsertAssociation( VV1[ 0 ], VV2[ 0 ], theMap, bidirect);
        InsertAssociation( VV1[ 1 ], VV2[ 1 ], theMap, bidirect);
        if ( VV1[0].IsSame( VV1[1] ) || // one of edges is closed
             VV2[0].IsSame( VV2[1] ) )
        {
          InsertAssociation( edge1, prpEdge, theMap, bidirect); // insert with a proper orientation
        }
        return true; // done
      }
    }
    if ( IsClosedEdge( edge1 ) && IsClosedEdge( edge2 ))
    {
      // TODO: find out a proper orientation (is it possible?)
      InsertAssociation( edge1, edge2, theMap, bidirect); // insert with a proper orientation
      InsertAssociation( TopExp::FirstVertex(edge1), TopExp::FirstVertex(edge2),
                         theMap, bidirect);
      return true; // done
    }
    break; // try by vertex closeness
  }

  case TopAbs_FACE: {
    // ----------------------------------------------------------------------
    if ( IsPropagationPossible( theMesh1, theMesh2 )) // try by propagation in one mesh
    {
      TopoDS_Face face1 = TopoDS::Face(theShape1);
      TopoDS_Face face2 = TopoDS::Face(theShape2);
      // get outer edge of theShape1
      TopoDS_Edge edge1 = TopoDS::Edge( OuterShape( face1, TopAbs_EDGE ));
      // find out if any edge of face2 is a propagation edge of outer edge1
      for ( TopExp_Explorer exp( face2, TopAbs_EDGE ); exp.More(); exp.Next() ) {
        TopoDS_Edge edge2 = TopoDS::Edge( exp.Current() );
        edge2 = GetPropagationEdge( theMesh1, edge2, edge1 );
        if ( !edge2.IsNull() ) // propagation found
        {
          TopoDS_Vertex VV1[2], VV2[2];
          TopExp::Vertices( edge1, VV1[0], VV1[1], true );
          TopExp::Vertices( edge2, VV2[0], VV2[1], true );
          list< TopoDS_Edge > edges1, edges2;
          int nbE = FindFaceAssociation( face1, VV1, face2, VV2, edges1, edges2 );
          if ( !nbE ) RETURN_BAD_RESULT("FindFaceAssociation() failed");
          if ( nbE == 2 ) // only 2 edges
          {
            // take care of proper association of propagated edges
            bool same1 = edge1.IsSame( edges1.front() );
            bool same2 = edge2.IsSame( edges2.front() );
            if ( same1 != same2 )
              Reverse(edges2, nbE);
          }
          // store association
          list< TopoDS_Edge >::iterator eIt1 = edges1.begin();
          list< TopoDS_Edge >::iterator eIt2 = edges2.begin();
          for ( ; eIt1 != edges1.end(); ++eIt1, ++eIt2 )
          {
            InsertAssociation( *eIt1, *eIt2, theMap, bidirect);
            VV1[0] = TopExp::FirstVertex( *eIt1, true );
            VV2[0] = TopExp::FirstVertex( *eIt2, true );
            InsertAssociation( VV1[0], VV2[0], theMap, bidirect);
          }
          return true;
        }
      }
    }
    break; // try by vertex closeness
  }
  default:;
  }

  // Find association by closeness of vertices
  // ------------------------------------------

  TopTools_IndexedMapOfShape vMap1, vMap2;
  TopExp::MapShapes( theShape1, TopAbs_VERTEX, vMap1 );
  TopExp::MapShapes( theShape2, TopAbs_VERTEX, vMap2 );

  if ( vMap1.Extent() != vMap2.Extent() )
    RETURN_BAD_RESULT("Different nb of vertices");

  if ( vMap1.Extent() == 1 ) {
    InsertAssociation( vMap1(1), vMap2(1), theMap, bidirect);
    if ( theShape1.ShapeType() == TopAbs_EDGE )
      return true;
    return FindSubShapeAssociation( theShape1, theMesh1, theShape2, theMesh2, theMap);
  }

  // Find transformation to make the shapes be of similar size at same location

  Bnd_Box box[2];
  for ( int i = 1; i <= vMap1.Extent(); ++i ) {
    box[ 0 ].Add( BRep_Tool::Pnt ( TopoDS::Vertex( vMap1( i ))));
    box[ 1 ].Add( BRep_Tool::Pnt ( TopoDS::Vertex( vMap2( i ))));
  }

  gp_Pnt gc[2]; // box center
  double x0,y0,z0, x1,y1,z1;
  box[0].Get( x0,y0,z0, x1,y1,z1 );
  gc[0] = 0.5 * ( gp_XYZ( x0,y0,z0 ) + gp_XYZ( x1,y1,z1 ));
  box[1].Get( x0,y0,z0, x1,y1,z1 );
  gc[1] = 0.5 * ( gp_XYZ( x0,y0,z0 ) + gp_XYZ( x1,y1,z1 ));

  // 1 -> 2
  gp_Vec vec01( gc[0], gc[1] );
  double scale = sqrt( box[1].SquareExtent() / box[0].SquareExtent() );

  // Find 2 closest vertices

  TopoDS_Vertex VV1[2], VV2[2];
  // get 2 linked vertices of shape 1 not belonging to an inner wire of a face
  TopoDS_Shape edge = theShape1;
  TopExp_Explorer expF( theShape1, TopAbs_FACE ), expE;
  for ( ; expF.More(); expF.Next() ) {
    edge.Nullify();
    TopoDS_Shape wire = OuterShape( TopoDS::Face( expF.Current() ), TopAbs_WIRE );
    for ( expE.Init( wire, TopAbs_EDGE ); edge.IsNull() && expE.More(); expE.Next() )
      if ( !IsClosedEdge( TopoDS::Edge( expE.Current() )))
        edge = expE.Current();
    if ( !edge.IsNull() )
      break;
  }
  if ( edge.IsNull() || edge.ShapeType() != TopAbs_EDGE )
    RETURN_BAD_RESULT("Edge not found");

  TopExp::Vertices( TopoDS::Edge( edge ), VV1[0], VV1[1]);
  if ( VV1[0].IsSame( VV1[1] ))
    RETURN_BAD_RESULT("Only closed edges");

  // find vertices closest to 2 linked vertices of shape 1
  for ( int i1 = 0; i1 < 2; ++i1 )
  {
    double dist2 = DBL_MAX;
    gp_Pnt p1 = BRep_Tool::Pnt( VV1[ i1 ]);
    p1.Translate( vec01 );
    p1.Scale( gc[1], scale );
    for ( int i2 = 1; i2 <= vMap2.Extent(); ++i2 )
    {
      TopoDS_Vertex V2 = TopoDS::Vertex( vMap2( i2 ));
      gp_Pnt p2 = BRep_Tool::Pnt ( V2 );
      double d2 = p1.SquareDistance( p2 );
      if ( d2 < dist2 && !V2.IsSame( VV2[ 0 ])) {
        VV2[ i1 ] = V2; dist2 = d2;
      }
    }
  }

  InsertAssociation( VV1[ 0 ], VV2 [ 0 ], theMap, bidirect);
  InsertAssociation( VV1[ 1 ], VV2 [ 1 ], theMap, bidirect);
  if ( theShape1.ShapeType() == TopAbs_EDGE )
    return true;

  return FindSubShapeAssociation( theShape1, theMesh1, theShape2, theMesh2, theMap );
}

//================================================================================
/*!
 * \brief Find association of edges of faces
 * \param face1 - face 1
 * \param VV1 - vertices of face 1
 * \param face2 - face 2
 * \param VV2 - vertices of face 2 associated with oned of face 1
 * \param edges1 - out list of edges of face 1
 * \param edges2 - out list of edges of face 2
 * \retval int - nb of edges in an outer wire in a success case, else zero
 */
//================================================================================

int StdMeshers_ProjectionUtils::FindFaceAssociation(const TopoDS_Face& face1,
                                                    TopoDS_Vertex      VV1[2],
                                                    const TopoDS_Face& face2,
                                                    TopoDS_Vertex      VV2[2],
                                                    list< TopoDS_Edge > & edges1,
                                                    list< TopoDS_Edge > & edges2)
{
  edges1.clear();
  edges2.clear();

  list< int > nbVInW1, nbVInW2;
  if ( SMESH_Block::GetOrderedEdges( face1, VV1[0], edges1, nbVInW1) !=
       SMESH_Block::GetOrderedEdges( face2, VV2[0], edges2, nbVInW2) )
    RETURN_BAD_RESULT("Different number of wires in faces ");

  if ( nbVInW1.front() != nbVInW2.front() )
    RETURN_BAD_RESULT("Different number of edges in faces: " <<
                      nbVInW1.front() << " != " << nbVInW2.front());

  // Define if we need to reverse one of wires to make edges in lists match each other

  bool reverse = false;

  list< TopoDS_Edge >::iterator eBackIt;
  if ( !VV1[1].IsSame( TopExp::LastVertex( edges1.front(), true ))) {
    eBackIt = --edges1.end();
    // check if the second vertex belongs to the first or last edge in the wire
    if ( !VV1[1].IsSame( TopExp::FirstVertex( *eBackIt, true ))) {
      bool KO = true; // belongs to none
      if ( nbVInW1.size() > 1 ) { // several wires
        eBackIt = edges1.begin();
        for ( int i = 1; i < nbVInW1.front(); ++i ) ++eBackIt;
        KO = !VV1[1].IsSame( TopExp::FirstVertex( *eBackIt, true ));
      }
      if ( KO )
        RETURN_BAD_RESULT("GetOrderedEdges() failed");
    }
    reverse = true;
  }
  eBackIt = --edges2.end();
  if ( !VV2[1].IsSame( TopExp::LastVertex( edges2.front(), true ))) {
    // check if the second vertex belongs to the first or last edge in the wire
    if ( !VV2[1].IsSame( TopExp::FirstVertex( *eBackIt, true ))) {
      bool KO = true; // belongs to none
      if ( nbVInW2.size() > 1 ) { // several wires
        eBackIt = edges2.begin();
        for ( int i = 1; i < nbVInW2.front(); ++i ) ++eBackIt;
        KO = !VV2[1].IsSame( TopExp::FirstVertex( *eBackIt, true ));
      }
      if ( KO )
        RETURN_BAD_RESULT("GetOrderedEdges() failed");
    }
    reverse = !reverse;
  }
  if ( reverse )
  {
    Reverse( edges2 , nbVInW2.front());
    if (( VV1[1].IsSame( TopExp::LastVertex( edges1.front(), true ))) !=
        ( VV2[1].IsSame( TopExp::LastVertex( edges2.front(), true ))))
      RETURN_BAD_RESULT("GetOrderedEdges() failed");
  }
  return nbVInW2.front();
}

//=======================================================================
//function : InitVertexAssociation
//purpose  : 
//=======================================================================

void StdMeshers_ProjectionUtils::InitVertexAssociation( const SMESH_Hypothesis* theHyp,
                                                        TShapeShapeMap &        theAssociationMap)
{
  string hypName = theHyp->GetName();
  if ( hypName == "ProjectionSource1D" ) {
    const StdMeshers_ProjectionSource1D * hyp =
      static_cast<const StdMeshers_ProjectionSource1D*>( theHyp );
    if ( hyp->HasVertexAssociation() ) {
      InsertAssociation( hyp->GetSourceVertex(),hyp->GetTargetVertex(),theAssociationMap);
    }
  }
  else if ( hypName == "ProjectionSource2D" ) {
    const StdMeshers_ProjectionSource2D * hyp =
      static_cast<const StdMeshers_ProjectionSource2D*>( theHyp );
    if ( hyp->HasVertexAssociation() ) {
      InsertAssociation( hyp->GetSourceVertex(1),hyp->GetTargetVertex(1),theAssociationMap);
      InsertAssociation( hyp->GetSourceVertex(2),hyp->GetTargetVertex(2),theAssociationMap);
    }
  }
  else if ( hypName == "ProjectionSource3D" ) {
    const StdMeshers_ProjectionSource3D * hyp =
      static_cast<const StdMeshers_ProjectionSource3D*>( theHyp );
    if ( hyp->HasVertexAssociation() ) {
      InsertAssociation( hyp->GetSourceVertex(1),hyp->GetTargetVertex(1),theAssociationMap);
      InsertAssociation( hyp->GetSourceVertex(2),hyp->GetTargetVertex(2),theAssociationMap);
    }
  }
}

//=======================================================================
/*!
 * \brief Inserts association theShape1 <-> theShape2 to TShapeShapeMap
 * \param theShape1 - shape 1
 * \param theShape2 - shape 2
 * \param theAssociationMap - association map 
 * \retval bool - true if there was no association for these shapes before
 */
//=======================================================================

bool StdMeshers_ProjectionUtils::InsertAssociation( const TopoDS_Shape& theShape1,
                                                    const TopoDS_Shape& theShape2,
                                                    TShapeShapeMap &    theAssociationMap,
                                                    const bool          theBidirectional)
{
  if ( !theShape1.IsNull() && !theShape2.IsNull() ) {
    SHOW_VERTEX(theShape1,"Assoc ");
    SHOW_VERTEX(theShape2," to ");
    bool isNew = ( theAssociationMap.Bind( theShape1, theShape2 ));
    if ( theBidirectional )
      theAssociationMap.Bind( theShape2, theShape1 );
    return isNew;
  }
  return false;
}

//=======================================================================
//function : IsSubShape
//purpose  : 
//=======================================================================

bool StdMeshers_ProjectionUtils::IsSubShape( const TopoDS_Shape& shape,
                                             SMESH_Mesh*         aMesh )
{
  if ( shape.IsNull() || !aMesh )
    return false;
  return aMesh->GetMeshDS()->ShapeToIndex( shape );
}

//=======================================================================
//function : IsSubShape
//purpose  : 
//=======================================================================

bool StdMeshers_ProjectionUtils::IsSubShape( const TopoDS_Shape& shape,
                                             const TopoDS_Shape& mainShape )
{
  if ( !shape.IsNull() && !mainShape.IsNull() )
  {
    for ( TopExp_Explorer exp( mainShape, shape.ShapeType());
          exp.More();
          exp.Next() )
      if ( shape.IsSame( exp.Current() ))
        return true;
  }
  SCRUTE((shape.IsNull()));
  SCRUTE((mainShape.IsNull()));
  return false;
}


//=======================================================================
/*!
 * \brief Finds an edge by its vertices in a main shape of the mesh
 * \param aMesh - the mesh
 * \param V1 - vertex 1
 * \param V2 - vertex 2
 * \retval TopoDS_Edge - found edge
 */
//=======================================================================

TopoDS_Edge StdMeshers_ProjectionUtils::GetEdgeByVertices( SMESH_Mesh*          theMesh,
                                                           const TopoDS_Vertex& theV1,
                                                           const TopoDS_Vertex& theV2)
{
  if ( theMesh && !theV1.IsNull() && !theV2.IsNull() )
  {
    TopTools_ListIteratorOfListOfShape ancestorIt( theMesh->GetAncestors( theV1 ));
    for ( ; ancestorIt.More(); ancestorIt.Next() )
      if ( ancestorIt.Value().ShapeType() == TopAbs_EDGE )
        for ( TopExp_Explorer expV ( ancestorIt.Value(), TopAbs_VERTEX );
              expV.More();
              expV.Next() )
          if ( theV2.IsSame( expV.Current() ))
            return TopoDS::Edge( ancestorIt.Value() );
  }
  return TopoDS_Edge();
}

//================================================================================
/*!
 * \brief Return another face sharing an edge
 * \param aMesh - mesh
 * \param edge - edge
 * \param face - face
 * \retval TopoDS_Face - found face
 */
//================================================================================

TopoDS_Face StdMeshers_ProjectionUtils::GetNextFace( SMESH_Mesh*        mesh,
                                                     const TopoDS_Edge& edge,
                                                     const TopoDS_Face& face)
{
  if ( mesh && !edge.IsNull() && !face.IsNull() )
  {
    TopTools_ListIteratorOfListOfShape ancestorIt( mesh->GetAncestors( edge ));
    for ( ; ancestorIt.More(); ancestorIt.Next() )
      if ( ancestorIt.Value().ShapeType() == TopAbs_FACE &&
           !face.IsSame( ancestorIt.Value() ))
        return TopoDS::Face( ancestorIt.Value() );
  }
  return TopoDS_Face();
  
}

//================================================================================
/*!
 * \brief Return a propagation edge
 * \param aMesh - mesh
 * \param theEdge - edge to find by propagation
 * \param fromEdge - start edge for propagation
 * \retval TopoDS_Edge - found edge
 */
//================================================================================

TopoDS_Edge StdMeshers_ProjectionUtils::GetPropagationEdge( SMESH_Mesh*        aMesh,
                                                            const TopoDS_Edge& theEdge,
                                                            const TopoDS_Edge& fromEdge)
{
  SMESH_IndexedMapOfShape aChain;
  //aChain.Add(fromEdge);

  // List of edges, added to chain on the previous cycle pass
  TopTools_ListOfShape listPrevEdges;
  listPrevEdges.Append(fromEdge/*.Oriented( TopAbs_FORWARD )*/);

  // Collect all edges pass by pass
  while (listPrevEdges.Extent() > 0) {
    // List of edges, added to chain on this cycle pass
    TopTools_ListOfShape listCurEdges;

    // Find the next portion of edges
    TopTools_ListIteratorOfListOfShape itE (listPrevEdges);
    for (; itE.More(); itE.Next()) {
      TopoDS_Shape anE = itE.Value();

      // Iterate on faces, having edge <anE>
      TopTools_ListIteratorOfListOfShape itA (aMesh->GetAncestors(anE));
      for (; itA.More(); itA.Next()) {
        TopoDS_Shape aW = itA.Value();

        // There are objects of different type among the ancestors of edge
        if (aW.ShapeType() == TopAbs_WIRE) {
          TopoDS_Shape anOppE;

          BRepTools_WireExplorer aWE (TopoDS::Wire(aW));
          Standard_Integer nb = 1, found = 0;
          TopTools_Array1OfShape anEdges (1,4);
          for (; aWE.More(); aWE.Next(), nb++) {
            if (nb > 4) {
              found = 0;
              break;
            }
            anEdges(nb) = aWE.Current();
            if (anEdges(nb).IsSame(anE)) found = nb;
          }

          if (nb == 5 && found > 0) {
            // Quadrangle face found, get an opposite edge
            Standard_Integer opp = found + 2;
            if (opp > 4) opp -= 4;
            anOppE = anEdges(opp);

            // add anOppE to aChain if ...
            if (!aChain.Contains(anOppE)) { // ... anOppE is not in aChain
              // Add found edge to the chain oriented so that to
              // have it co-directed with a forward MainEdge
              TopAbs_Orientation ori = anE.Orientation();
              if ( anEdges(opp).Orientation() == anEdges(found).Orientation() )
                ori = TopAbs::Reverse( ori );
              anOppE.Orientation( ori );
              if ( anOppE.IsSame( theEdge ))
                return TopoDS::Edge( anOppE );
              aChain.Add(anOppE);
              listCurEdges.Append(anOppE);
            }
          } // if (nb == 5 && found > 0)
        } // if (aF.ShapeType() == TopAbs_WIRE)
      } // for (; itF.More(); itF.Next())
    } // for (; itE.More(); itE.Next())

    listPrevEdges = listCurEdges;
  } // while (listPrevEdges.Extent() > 0)

  return TopoDS_Edge();
}

//================================================================================
  /*!
   * \brief Find corresponding nodes on two faces
    * \param face1 - the first face
    * \param mesh1 - mesh containing elements on the first face
    * \param face2 - the second face
    * \param mesh2 - mesh containing elements on the second face
    * \param assocMap - map associating subshapes of the faces
    * \param node1To2Map - map containing found matching nodes
    * \retval bool - is a success
   */
//================================================================================

bool StdMeshers_ProjectionUtils::
FindMatchingNodesOnFaces( const TopoDS_Face&     face1,
                          SMESH_Mesh*            mesh1,
                          const TopoDS_Face&     face2,
                          SMESH_Mesh*            mesh2,
                          const TShapeShapeMap & assocMap,
                          TNodeNodeMap &         node1To2Map)
{
  SMESHDS_Mesh* meshDS1 = mesh1->GetMeshDS();
  SMESHDS_Mesh* meshDS2 = mesh2->GetMeshDS();
  
  SMESH_MesherHelper helper1( *mesh1 );
  SMESH_MesherHelper helper2( *mesh2 );

  // Get corresponding submeshes and roughly check match of meshes

  SMESHDS_SubMesh * SM2 = meshDS2->MeshElements( face2 );
  SMESHDS_SubMesh * SM1 = meshDS1->MeshElements( face1 );
  if ( !SM2 || !SM1 )
    RETURN_BAD_RESULT("Empty submeshes");
  if ( SM2->NbNodes()    != SM1->NbNodes() ||
       SM2->NbElements() != SM1->NbElements() )
    RETURN_BAD_RESULT("Different meshes on corresponding faces "
                      << meshDS1->ShapeToIndex( face1 ) << " and "
                      << meshDS2->ShapeToIndex( face2 ));
  if ( SM2->NbElements() == 0 )
    RETURN_BAD_RESULT("Empty submeshes");

  helper1.SetSubShape( face1 );
  helper2.SetSubShape( face2 );
  if ( helper1.HasSeam() != helper2.HasSeam() )
    RETURN_BAD_RESULT("Different faces' geometry");

  // Data to call SMESH_MeshEditor::FindMatchingNodes():

  // 1. Nodes of corresponding links:

  // get 2 matching edges, not seam ones
  TopoDS_Edge edge1, edge2;
  TopExp_Explorer eE( OuterShape( face2, TopAbs_WIRE ), TopAbs_EDGE );
  do {
    edge2 = TopoDS::Edge( eE.Current() );
    eE.Next();
  } while ( BRep_Tool::IsClosed( edge2, face2 ) && eE.More());
  if ( !assocMap.IsBound( edge2 ))
    RETURN_BAD_RESULT("Association not found for edge " << meshDS2->ShapeToIndex( edge2 ));
  edge1 = TopoDS::Edge( assocMap( edge2 ));
  if ( !IsSubShape( edge1, face1 ))
    RETURN_BAD_RESULT("Wrong association, edge " << meshDS1->ShapeToIndex( edge1 ) <<
                      " isn't a subshape of face " << meshDS1->ShapeToIndex( face1 ));

  // get 2 matching vertices
  TopoDS_Shape V2 = TopExp::FirstVertex( TopoDS::Edge( edge2 ));
  if ( !assocMap.IsBound( V2 ))
    RETURN_BAD_RESULT("Association not found for vertex " << meshDS2->ShapeToIndex( V2 ));
  TopoDS_Shape V1 = assocMap( V2 );

  // nodes on vertices
  SMESHDS_SubMesh * vSM1 = meshDS1->MeshElements( V1 );
  SMESHDS_SubMesh * vSM2 = meshDS2->MeshElements( V2 );
  if ( !vSM1 || !vSM2 || vSM1->NbNodes() != 1 || vSM2->NbNodes() != 1 )
    RETURN_BAD_RESULT("Bad node submesh");
  const SMDS_MeshNode* vNode1 = vSM1->GetNodes()->next();
  const SMDS_MeshNode* vNode2 = vSM2->GetNodes()->next();

  // nodes on edges linked with nodes on vertices
  const SMDS_MeshNode* nullNode = 0;
  vector< const SMDS_MeshNode*> eNode1( 2, nullNode );
  vector< const SMDS_MeshNode*> eNode2( 2, nullNode );
  int nbNodeToGet = 1;
  if ( IsClosedEdge( edge1 ) || IsClosedEdge( edge2 ) )
    nbNodeToGet = 2;
  for ( int is2 = 0; is2 < 2; ++is2 )
  {
    TopoDS_Edge &     edge  = is2 ? edge2 : edge1;
    SMESHDS_Mesh *    smDS  = is2 ? meshDS2 : meshDS1;
    SMESHDS_SubMesh* edgeSM = smDS->MeshElements( edge );
    // nodes linked with ones on vertices
    const SMDS_MeshNode*           vNode = is2 ? vNode2 : vNode1;
    vector< const SMDS_MeshNode*>& eNode = is2 ? eNode2 : eNode1;
    int nbGotNode = 0;
    SMDS_ElemIteratorPtr vElem = vNode->GetInverseElementIterator();
    while ( vElem->more() && nbGotNode != nbNodeToGet ) {
      const SMDS_MeshElement* elem = vElem->next();
      if ( elem->GetType() == SMDSAbs_Edge && edgeSM->Contains( elem ))
        eNode[ nbGotNode++ ] = 
          ( elem->GetNode(0) == vNode ) ? elem->GetNode(1) : elem->GetNode(0);
    }
    if ( nbGotNode > 1 ) // sort found nodes by param on edge
    {
      SMESH_MesherHelper* helper = is2 ? &helper2 : &helper1;
      double u0 = helper->GetNodeU( edge, eNode[ 0 ]);
      double u1 = helper->GetNodeU( edge, eNode[ 1 ]);
      if ( u0 > u1 ) std::swap( eNode[ 0 ], eNode[ 1 ]);
    }
    if ( nbGotNode == 0 )
      RETURN_BAD_RESULT("Found no nodes on edge " << smDS->ShapeToIndex( edge ) <<
                        " linked to " << vNode );
  }

  // 2. face sets

  set<const SMDS_MeshElement*> Elems1, Elems2;
  for ( int is2 = 0; is2 < 2; ++is2 )
  {
    set<const SMDS_MeshElement*> & elems = is2 ? Elems2 : Elems1;
    SMESHDS_SubMesh*                  sm = is2 ? SM2 : SM1;
    SMESH_MesherHelper*           helper = is2 ? &helper2 : &helper1;
    const TopoDS_Face &             face = is2 ? face2 : face1;
    SMDS_ElemIteratorPtr eIt = sm->GetElements();

    if ( !helper->IsSeamShape( is2 ? edge2 : edge1 ))
    {
      while ( eIt->more() ) elems.insert( eIt->next() );
    }
    else
    {
      // there is only seam edge in a face, i.e. it is a sphere.
      // FindMatchingNodes() will not know which way to go from any edge.
      // So we ignore all faces having nodes on edges or vertices except
      // one of faces sharing current start nodes

      // find a face to keep
      const SMDS_MeshElement* faceToKeep = 0;
      const SMDS_MeshNode* vNode = is2 ? vNode2 : vNode1;
      const SMDS_MeshNode* eNode = is2 ? eNode2[0] : eNode1[0];
      TIDSortedElemSet inSet, notInSet;

      const SMDS_MeshElement* f1 =
        SMESH_MeshEditor::FindFaceInSet( vNode, eNode, inSet, notInSet );
      if ( !f1 ) RETURN_BAD_RESULT("The first face on seam not found");
      notInSet.insert( f1 );

      const SMDS_MeshElement* f2 =
        SMESH_MeshEditor::FindFaceInSet( vNode, eNode, inSet, notInSet );
      if ( !f2 ) RETURN_BAD_RESULT("The second face on seam not found");

      // select a face with less UV of vNode
      const SMDS_MeshNode* notSeamNode[2] = {0, 0};
      for ( int iF = 0; iF < 2; ++iF ) {
        const SMDS_MeshElement* f = ( iF ? f2 : f1 );
        for ( int i = 0; !notSeamNode[ iF ] && i < f->NbNodes(); ++i ) {
          const SMDS_MeshNode* node = f->GetNode( i );
          if ( !helper->IsSeamShape( node->GetPosition()->GetShapeId() ))
            notSeamNode[ iF ] = node;
        }
      }
      gp_Pnt2d uv1 = helper->GetNodeUV( face, vNode, notSeamNode[0] );
      gp_Pnt2d uv2 = helper->GetNodeUV( face, vNode, notSeamNode[1] );
      if ( uv1.X() + uv1.Y() > uv2.X() + uv2.Y() )
        faceToKeep = f2;
      else
        faceToKeep = f1;

      // fill elem set
      elems.insert( faceToKeep );
      while ( eIt->more() ) {
        const SMDS_MeshElement* f = eIt->next();
        int nbNodes = f->NbNodes();
        if ( f->IsQuadratic() )
          nbNodes /= 2;
        bool onBnd = false;
        for ( int i = 0; !onBnd && i < nbNodes; ++i ) {
          const SMDS_MeshNode* node = f->GetNode( i );
          onBnd = ( node->GetPosition()->GetTypeOfPosition() != SMDS_TOP_FACE);
        }
        if ( !onBnd )
          elems.insert( f );
      }
    } // case on a sphere
  } // loop on 2 faces

  node1To2Map.clear();
  int res = SMESH_MeshEditor::FindMatchingNodes( Elems1, Elems2,
                                                 vNode1, vNode2,
                                                 eNode1[0], eNode2[0],
                                                 node1To2Map);
  if ( res != SMESH_MeshEditor::SEW_OK )
    RETURN_BAD_RESULT("FindMatchingNodes() result " << res );


  // On a sphere, add matching nodes on the edge

  if ( helper1.IsSeamShape( edge1 ))
  {
    // sort nodes on edges by param on edge
    map< double, const SMDS_MeshNode* > u2nodesMaps[2];
    for ( int is2 = 0; is2 < 2; ++is2 )
    {
      TopoDS_Edge &     edge  = is2 ? edge2 : edge1;
      SMESHDS_Mesh *    smDS  = is2 ? meshDS2 : meshDS1;
      SMESHDS_SubMesh* edgeSM = smDS->MeshElements( edge );
      map< double, const SMDS_MeshNode* > & pos2nodes = u2nodesMaps[ is2 ];

      SMDS_NodeIteratorPtr nIt = edgeSM->GetNodes();
      while ( nIt->more() ) {
        const SMDS_MeshNode* node = nIt->next();
        const SMDS_EdgePosition* pos =
          static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
        pos2nodes.insert( make_pair( pos->GetUParameter(), node ));
      }
      if ( pos2nodes.size() != edgeSM->NbNodes() )
        RETURN_BAD_RESULT("Equal params of nodes on edge "
                          << smDS->ShapeToIndex( edge ) << " of face " << is2 );
    }
    if ( u2nodesMaps[0].size() != u2nodesMaps[1].size() )
      RETURN_BAD_RESULT("Different nb of new nodes on edges or wrong params");

    // compare edge orientation
    double u1 = helper1.GetNodeU( edge1, vNode1 );
    double u2 = helper2.GetNodeU( edge2, vNode2 );
    bool isFirst1 = ( u1 < u2nodesMaps[0].begin()->first );
    bool isFirst2 = ( u2 < u2nodesMaps[1].begin()->first );
    bool reverse ( isFirst1 != isFirst2 );

    // associate matching nodes
    map< double, const SMDS_MeshNode* >::iterator u_Node1, u_Node2, end1;
    map< double, const SMDS_MeshNode* >::reverse_iterator uR_Node2;
    u_Node1 = u2nodesMaps[0].begin();
    u_Node2 = u2nodesMaps[1].begin();
    uR_Node2 = u2nodesMaps[1].rbegin();
    end1 = u2nodesMaps[0].end();
    for ( ; u_Node1 != end1; ++u_Node1 ) {
      const SMDS_MeshNode* n1 = u_Node1->second;
      const SMDS_MeshNode* n2 = ( reverse ? (uR_Node2++)->second : (u_Node2++)->second );
      node1To2Map.insert( make_pair( n1, n2 ));
    }

    // associate matching nodes on the last vertices
    V2 = TopExp::LastVertex( TopoDS::Edge( edge2 ));
    if ( !assocMap.IsBound( V2 ))
      RETURN_BAD_RESULT("Association not found for vertex " << meshDS2->ShapeToIndex( V2 ));
    V1 = assocMap( V2 );
    vSM1 = meshDS1->MeshElements( V1 );
    vSM2 = meshDS2->MeshElements( V2 );
    if ( !vSM1 || !vSM2 || vSM1->NbNodes() != 1 || vSM2->NbNodes() != 1 )
      RETURN_BAD_RESULT("Bad node submesh");
    vNode1 = vSM1->GetNodes()->next();
    vNode2 = vSM2->GetNodes()->next();
    node1To2Map.insert( make_pair( vNode1, vNode2 ));
  }
  
  return true;
}

//================================================================================
/*!
 * \brief Check if the first and last vertices of an edge are the same
 * \param anEdge - the edge to check
 * \retval bool - true if same
 */
//================================================================================

bool StdMeshers_ProjectionUtils::IsClosedEdge( const TopoDS_Edge& anEdge )
{
  return TopExp::FirstVertex( anEdge ).IsSame( TopExp::LastVertex( anEdge ));
}

//================================================================================
  /*!
   * \brief Return any subshape of a face belonging to the outer wire
    * \param face - the face
    * \param type - type of subshape to return
    * \retval TopoDS_Shape - the found subshape
   */
//================================================================================

TopoDS_Shape StdMeshers_ProjectionUtils::OuterShape( const TopoDS_Face& face,
                                                     TopAbs_ShapeEnum   type)
{
  TopExp_Explorer exp( BRepTools::OuterWire( face ), type );
  if ( exp.More() )
    return exp.Current();
  return TopoDS_Shape();
}

//================================================================================
  /*!
   * \brief Check that submesh is computed and try to compute it if is not
    * \param sm - submesh to compute
    * \param iterationNb - int used to stop infinite recursive call
    * \retval bool - true if computed
   */
//================================================================================

bool StdMeshers_ProjectionUtils::MakeComputed(SMESH_subMesh * sm, const int iterationNb)
{
  if ( iterationNb > 10 )
    RETURN_BAD_RESULT("Infinite recursive projection");
  if ( !sm )
    RETURN_BAD_RESULT("NULL submesh");
  if ( sm->IsMeshComputed() )
    return true;

  SMESH_Mesh* mesh = sm->GetFather();
  SMESH_Gen* gen   = mesh->GetGen();
  SMESH_Algo* algo = gen->GetAlgo( *mesh, sm->GetSubShape() );
  if ( !algo )
    RETURN_BAD_RESULT("No algo assigned to submesh " << sm->GetId());

  string algoType = algo->GetName();
  if ( algoType.substr(0, 11) != "Projection_")
    return gen->Compute( *mesh, sm->GetSubShape() );

  // try to compute source mesh

  const list <const SMESHDS_Hypothesis *> & hyps =
    algo->GetUsedHypothesis( *mesh, sm->GetSubShape() );

  TopoDS_Shape srcShape;
  SMESH_Mesh* srcMesh = 0;
  list <const SMESHDS_Hypothesis*>::const_iterator hIt = hyps.begin();
  for ( ; srcShape.IsNull() && hIt != hyps.end(); ++hIt ) {
    string hypName = (*hIt)->GetName();
    if ( hypName == "ProjectionSource1D" ) {
      const StdMeshers_ProjectionSource1D * hyp =
        static_cast<const StdMeshers_ProjectionSource1D*>( *hIt );
      srcShape = hyp->GetSourceEdge();
      srcMesh = hyp->GetSourceMesh();
    }
    else if ( hypName == "ProjectionSource2D" ) {
      const StdMeshers_ProjectionSource2D * hyp =
        static_cast<const StdMeshers_ProjectionSource2D*>( *hIt );
      srcShape = hyp->GetSourceFace();
      srcMesh = hyp->GetSourceMesh();
    }
    else if ( hypName == "ProjectionSource3D" ) {
      const StdMeshers_ProjectionSource3D * hyp =
        static_cast<const StdMeshers_ProjectionSource3D*>( *hIt );
      srcShape = hyp->GetSource3DShape();
      srcMesh = hyp->GetSourceMesh();
    }
  }
  if ( srcShape.IsNull() ) // no projection source defined
    return gen->Compute( *mesh, sm->GetSubShape() );

  if ( srcShape.IsSame( sm->GetSubShape() ))
    RETURN_BAD_RESULT("Projection from self");
    
  if ( !srcMesh )
    srcMesh = mesh;

  return MakeComputed( srcMesh->GetSubMesh( srcShape ), iterationNb + 1 );
}

//================================================================================
  /*!
   * \brief Count nb of subshapes
    * \param shape - the shape
    * \param type - the type of subshapes to count
    * \retval int - the calculated number
   */
//================================================================================

int StdMeshers_ProjectionUtils::Count(const TopoDS_Shape&    shape,
                                      const TopAbs_ShapeEnum type,
                                      const bool             ignoreSame)
{
  if ( ignoreSame ) {
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes( shape, type, map );
    return map.Extent();
  }
  else {
    int nb = 0;
    for ( TopExp_Explorer exp( shape, type ); exp.More(); exp.Next() )
      ++nb;
    return nb;
  }
}

namespace {

  SMESH_subMeshEventListener* GetSrcSubMeshListener();

  //================================================================================
  /*!
   * \brief Listener that resets an event listener on source submesh when 
   * "ProjectionSource*D" hypothesis is modified
   */
  //================================================================================

  struct HypModifWaiter: SMESH_subMeshEventListener
  {
    HypModifWaiter():SMESH_subMeshEventListener(0){} // won't be deleted by submesh

    void ProcessEvent(const int event, const int eventType, SMESH_subMesh* subMesh,
                      EventListenerData*, const SMESH_Hypothesis*)
    {
      if ( event     == SMESH_subMesh::MODIF_HYP &&
           eventType == SMESH_subMesh::ALGO_EVENT)
      {
        // delete current source listener
        subMesh->DeleteEventListener( GetSrcSubMeshListener() );
        // let algo set a new one
        SMESH_Gen* gen = subMesh->GetFather()->GetGen();
        if ( SMESH_Algo* algo = gen->GetAlgo( *subMesh->GetFather(),
                                              subMesh->GetSubShape() ))
          algo->SetEventListener( subMesh );
      }
    }
  };
  //================================================================================
  /*!
   * \brief return static HypModifWaiter
   */
  //================================================================================

  SMESH_subMeshEventListener* GetHypModifWaiter() {
    static HypModifWaiter aHypModifWaiter;
    return &aHypModifWaiter;
  }
  //================================================================================
  /*!
   * \brief return static listener for source shape submeshes
   */
  //================================================================================

  SMESH_subMeshEventListener* GetSrcSubMeshListener() {
    static SMESH_subMeshEventListener srcListener(0); // won't be deleted by submesh
    return &srcListener;
  }
}

//================================================================================
/*!
 * \brief Set event listeners to submesh with projection algo
 * \param subMesh - submesh with projection algo
 * \param srcShape - source shape
 * \param srcMesh - source mesh
 */
//================================================================================

void StdMeshers_ProjectionUtils::SetEventListener(SMESH_subMesh* subMesh,
                                                  TopoDS_Shape   srcShape,
                                                  SMESH_Mesh*    srcMesh)
{
  // Set listener that resets an event listener on source submesh when
  // "ProjectionSource*D" hypothesis is modified
  subMesh->SetEventListener( GetHypModifWaiter(),0,subMesh);

  // Set an event listener to submesh of the source shape
  if ( !srcShape.IsNull() )
  {
    if ( !srcMesh )
      srcMesh = subMesh->GetFather();

    SMESH_subMesh* srcShapeSM = srcMesh->GetSubMesh( srcShape );

    if ( srcShapeSM != subMesh )
      subMesh->SetEventListener( GetSrcSubMeshListener(),
                                 SMESH_subMeshEventListenerData::MakeData( subMesh ),
                                 srcShapeSM );
  }
}
