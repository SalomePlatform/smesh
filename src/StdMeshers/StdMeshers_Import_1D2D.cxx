// Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  SMESH SMESH : implementaion of SMESH idl descriptions
//  File   : StdMeshers_Import_1D2D.cxx
//  Module : SMESH
//
#include "StdMeshers_Import_1D2D.hxx"

#include "StdMeshers_Import_1D.hxx"
#include "StdMeshers_ImportSource.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"

#include "Utils_SALOME_Exception.hxx"
#include "utilities.h"

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <Precision.hxx>

#include <numeric>

using namespace std;

namespace
{
  double getMinElemSize2( const SMESHDS_GroupBase* srcGroup )
  {
    double minSize2 = 1e100;
    SMDS_ElemIteratorPtr srcElems = srcGroup->GetElements();
    while ( srcElems->more() ) // loop on group contents
    {
      const SMDS_MeshElement* face = srcElems->next();
      int nbN = face->NbCornerNodes();

      SMESH_TNodeXYZ prevN( face->GetNode( nbN-1 ));
      for ( int i = 0; i < nbN; ++i )
      {
        SMESH_TNodeXYZ n( face->GetNode( i ) );
        double size2 = ( n - prevN ).SquareModulus();
        minSize2 = std::min( minSize2, size2 );
        prevN = n;
      }
    }
    return minSize2;
  }
}

//=============================================================================
/*!
 * Creates StdMeshers_Import_1D2D
 */
//=============================================================================

StdMeshers_Import_1D2D::StdMeshers_Import_1D2D(int hypId, int studyId, SMESH_Gen * gen)
  :SMESH_2D_Algo(hypId, studyId, gen), _sourceHyp(0)
{
  MESSAGE("StdMeshers_Import_1D2D::StdMeshers_Import_1D2D");
  _name = "Import_1D2D";
  _shapeType = (1 << TopAbs_FACE);

  _compatibleHypothesis.push_back("ImportSource2D");
  _requireDescretBoundary = false;
}

//=============================================================================
/*!
 * Check presence of a hypothesis
 */
//=============================================================================

bool StdMeshers_Import_1D2D::CheckHypothesis
                         (SMESH_Mesh&                          aMesh,
                          const TopoDS_Shape&                  aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  _sourceHyp = 0;

  const list <const SMESHDS_Hypothesis * >&hyps = GetUsedHypothesis(aMesh, aShape);
  if ( hyps.size() == 0 )
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work with no hypothesis
  }

  if ( hyps.size() > 1 )
  {
    aStatus = SMESH_Hypothesis::HYP_ALREADY_EXIST;
    return false;
  }

  const SMESHDS_Hypothesis *theHyp = hyps.front();

  string hypName = theHyp->GetName();

  if (hypName == _compatibleHypothesis.front())
  {
    _sourceHyp = (StdMeshers_ImportSource1D *)theHyp;
    aStatus = SMESH_Hypothesis::HYP_OK;
    return true;
  }

  aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;
  return true;
}

namespace
{
  /*!
   * \brief OrientedLink additionally storing a medium node
   */
  struct TLink : public SMESH_OrientedLink
  {
    const SMDS_MeshNode* _medium;
    TLink( const SMDS_MeshNode* n1,
           const SMDS_MeshNode* n2,
           const SMDS_MeshNode* medium=0)
      : SMESH_OrientedLink( n1,n2 ), _medium( medium ) {}
  };
}

//=============================================================================
/*!
 * Import elements from the other mesh 
 */
//=============================================================================

bool StdMeshers_Import_1D2D::Compute(SMESH_Mesh & theMesh, const TopoDS_Shape & theShape)
{
  if ( !_sourceHyp ) return false;

  const vector<SMESH_Group*>& srcGroups = _sourceHyp->GetGroups();
  if ( srcGroups.empty() )
    return error("Invalid source groups");

  bool allGroupsEmpty = true;
  for ( size_t iG = 0; iG < srcGroups.size() && allGroupsEmpty; ++iG )
    allGroupsEmpty = srcGroups[iG]->GetGroupDS()->IsEmpty();
  if ( allGroupsEmpty )
    return error("No faces in source groups");

  SMESH_MesherHelper helper(theMesh);
  helper.SetSubShape(theShape);
  SMESHDS_Mesh* tgtMesh = theMesh.GetMeshDS();

  const TopoDS_Face& geomFace = TopoDS::Face( theShape );
  const double faceTol = helper.MaxTolerance( geomFace );
  const int shapeID = tgtMesh->ShapeToIndex( geomFace );
  const bool toCheckOri = (helper.NbAncestors( geomFace, theMesh, TopAbs_SOLID ) == 1 );

  Handle(Geom_Surface) surface = BRep_Tool::Surface( geomFace );
  const bool reverse = 
    ( helper.GetSubShapeOri( tgtMesh->ShapeToMesh(), geomFace) == TopAbs_REVERSED );
  gp_Pnt p; gp_Vec du, dv;

  set<int> subShapeIDs;
  subShapeIDs.insert( shapeID );

  // get nodes on vertices
  list < SMESH_TNodeXYZ > vertexNodes;
  list < SMESH_TNodeXYZ >::iterator vNIt;
  TopExp_Explorer exp( theShape, TopAbs_VERTEX );
  for ( ; exp.More(); exp.Next() )
  {
    const TopoDS_Vertex& v = TopoDS::Vertex( exp.Current() );
    if ( !subShapeIDs.insert( tgtMesh->ShapeToIndex( v )).second )
      continue;
    const SMDS_MeshNode* n = SMESH_Algo::VertexNode( v, tgtMesh );
    if ( !n )
    {
      _gen->Compute(theMesh,v,/*anUpward=*/true);
      n = SMESH_Algo::VertexNode( v, tgtMesh );
      if ( !n ) return false; // very strange
    }
    vertexNodes.push_back( SMESH_TNodeXYZ( n ));
  }

  // to count now many times a link between nodes encounters
  map<TLink, int> linkCount;
  map<TLink, int>::iterator link2Nb;
  double minGroupTol = Precision::Infinite();

  // =========================
  // Import faces from groups
  // =========================

  StdMeshers_Import_1D::TNodeNodeMap* n2n;
  StdMeshers_Import_1D::TElemElemMap* e2e;
  vector<const SMDS_MeshNode*> newNodes;
  for ( size_t iG = 0; iG < srcGroups.size(); ++iG )
  {
    const SMESHDS_GroupBase* srcGroup = srcGroups[iG]->GetGroupDS();

    const int meshID = srcGroup->GetMesh()->GetPersistentId();
    const SMESH_Mesh* srcMesh = GetMeshByPersistentID( meshID );
    if ( !srcMesh ) continue;
    StdMeshers_Import_1D::getMaps( srcMesh, &theMesh, n2n, e2e );

    const double groupTol = 0.5 * sqrt( getMinElemSize2( srcGroup ));
    minGroupTol = std::min( groupTol, minGroupTol );

    SMDS_ElemIteratorPtr srcElems = srcGroup->GetElements();
    SMDS_MeshNode *tmpNode = helper.AddNode(0,0,0);
    gp_XY uv( Precision::Infinite(), Precision::Infinite() );
    while ( srcElems->more() ) // loop on group contents
    {
      const SMDS_MeshElement* face = srcElems->next();
      // find or create nodes of a new face
      newNodes.resize( face->NbNodes() );
      newNodes.back() = 0;
      int nbCreatedNodes = 0;
      SMDS_MeshElement::iterator node = face->begin_nodes();
      for ( size_t i = 0; i < newNodes.size(); ++i, ++node )
      {
        StdMeshers_Import_1D::TNodeNodeMap::iterator n2nIt = n2n->insert( make_pair( *node, (SMDS_MeshNode*)0 )).first;
        if ( n2nIt->second )
        {
          if ( !subShapeIDs.count( n2nIt->second->getshapeId() ))
            break;
        }
        else
        {
          // find an existing vertex node
          for ( vNIt = vertexNodes.begin(); vNIt != vertexNodes.end(); ++vNIt)
            if ( vNIt->SquareDistance( *node ) < groupTol * groupTol)
            {
              (*n2nIt).second = vNIt->_node;
              vertexNodes.erase( vNIt );
              break;
            }
        }
        if ( !n2nIt->second )
        {
          // find out if node lies on theShape
          tmpNode->setXYZ( (*node)->X(), (*node)->Y(), (*node)->Z());
          uv.SetCoord( Precision::Infinite(), Precision::Infinite() );
          if ( helper.CheckNodeUV( geomFace, tmpNode, uv, groupTol, /*force=*/true ))
          {
            SMDS_MeshNode* newNode = tgtMesh->AddNode( (*node)->X(), (*node)->Y(), (*node)->Z());
            n2nIt->second = newNode;
            tgtMesh->SetNodeOnFace( newNode, shapeID, uv.X(), uv.Y() );
            nbCreatedNodes++;
          }
        }
        if ( !(newNodes[i] = n2nIt->second ))
          break;
      }
      if ( !newNodes.back() )
        continue; // not all nodes of the face lie on theShape

      // try to find already created face
      SMDS_MeshElement * newFace = 0;
      if ( nbCreatedNodes == 0 &&
           tgtMesh->FindElement(newNodes, SMDSAbs_Face, /*noMedium=*/false))
        continue; // repeated face in source groups already created 

      // check future face orientation
      if ( toCheckOri )
      {
        int iNode = -1;
        gp_Vec geomNorm;
        do
        {
          uv = helper.GetNodeUV( geomFace, newNodes[++iNode] );
          surface->D1( uv.X(),uv.Y(), p, du,dv );
          geomNorm = reverse ? dv^du : du^dv;
        }
        while ( geomNorm.SquareMagnitude() < 1e-6 && iNode+1 < face->NbCornerNodes());

        int iNext = helper.WrapIndex( iNode+1, face->NbCornerNodes() );
        int iPrev = helper.WrapIndex( iNode-1, face->NbCornerNodes() );

        SMESH_TNodeXYZ prevNode( newNodes[iPrev] );
        SMESH_TNodeXYZ curNode ( newNodes[iNode] );
        SMESH_TNodeXYZ nextNode( newNodes[iNext] );
        gp_Vec n1n0( prevNode - curNode);
        gp_Vec n1n2( nextNode - curNode );
        gp_Vec meshNorm = n1n2 ^ n1n0;

        if ( geomNorm * meshNorm < 0 )
          std::reverse( newNodes.begin(), newNodes.end() );
      }

      // make a new face
      switch ( newNodes.size() )
      {
      case 3:
        newFace = tgtMesh->AddFace( newNodes[0], newNodes[1], newNodes[2] );
        break;
      case 4:
        newFace = tgtMesh->AddFace( newNodes[0], newNodes[1], newNodes[2], newNodes[3] );
        break;
      case 6:
        newFace = tgtMesh->AddFace( newNodes[0], newNodes[1], newNodes[2],
                                    newNodes[3], newNodes[4], newNodes[5]);
        break;
      case 8:
        newFace = tgtMesh->AddFace( newNodes[0], newNodes[1], newNodes[2], newNodes[3],
                                    newNodes[4], newNodes[5], newNodes[6], newNodes[7]);
        break;
      default: continue;
      }
      tgtMesh->SetMeshElementOnShape( newFace, shapeID );
      e2e->insert( make_pair( face, newFace ));

      // collect links
      int nbNodes = face->NbCornerNodes();
      const SMDS_MeshNode* medium = 0;
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* n1 = newNodes[i];
        const SMDS_MeshNode* n2 = newNodes[ (i+1)%nbNodes ];
        if ( newFace->IsQuadratic() )
          medium = newNodes[i+nbNodes];
        link2Nb = linkCount.insert( make_pair( TLink( n1, n2, medium ), 0)).first;
        ++link2Nb->second;
        // if ( link2Nb->second == 1 )
        // {
        //   // measure link length
        //   double len2 = SMESH_TNodeXYZ( n1 ).SquareDistance( n2 );
        //   if ( len2 < minGroupTol )
        //     minGroupTol = len2;
        // }
      }
    }
    helper.GetMeshDS()->RemoveNode(tmpNode);
  }

  // ==========================================================
  // Put nodes on geom edges and create edges on them;
  // check if the whole geom face is covered by imported faces
  // ==========================================================

  vector< TopoDS_Edge > edges;
  for ( exp.Init( theShape, TopAbs_EDGE ); exp.More(); exp.Next() )
    if ( subShapeIDs.insert( tgtMesh->ShapeToIndex( exp.Current() )).second )
      edges.push_back( TopoDS::Edge( exp.Current() ));

  // use large tolerance for projection of nodes to edges because of
  // BLSURF mesher specifics (issue 0020918, Study2.hdf)
  const double projTol = minGroupTol;

  bool isFaceMeshed = false;
  SMESHDS_SubMesh* tgtFaceSM = tgtMesh->MeshElements( theShape );
  if ( tgtFaceSM )
  {
    // the imported mesh is valid if all external links (encountered once)
    // lie on geom edges
    subShapeIDs.erase( shapeID ); // to contain edges and vertices only
    double u, f, l;
    for ( link2Nb = linkCount.begin(); link2Nb != linkCount.end(); ++link2Nb)
    {
      const TLink& link = (*link2Nb).first;
      int nbFaces = link2Nb->second;
      if ( nbFaces == 1 )
      {
        // check if a not shared link lies on face boundary
        bool nodesOnBoundary = true;
        list< TopoDS_Shape > bndShapes;
        for ( int is1stN = 0; is1stN < 2 && nodesOnBoundary; ++is1stN )
        {
          const SMDS_MeshNode* n = is1stN ? link.node1() : link.node2();
          if ( !subShapeIDs.count( n->getshapeId() ))
          {
            for ( size_t iE = 0; iE < edges.size(); ++iE )
              if ( helper.CheckNodeU( edges[iE], n, u=0, projTol, /*force=*/true ))
              {
                BRep_Tool::Range(edges[iE],f,l);
                if ( Abs(u-f) < 2 * faceTol || Abs(u-l) < 2 * faceTol )
                  // duplicated node on vertex
                  return error("Source elements overlap one another");
                tgtFaceSM->RemoveNode( n, /*isNodeDeleted=*/false );
                tgtMesh->SetNodeOnEdge( (SMDS_MeshNode*)n, edges[iE], u );
                break;
              }
            nodesOnBoundary = subShapeIDs.count( n->getshapeId());
          }
          if ( nodesOnBoundary )
          {
            TopoDS_Shape s = helper.GetSubShapeByNode( n, tgtMesh );
            if ( s.ShapeType() == TopAbs_VERTEX )
              bndShapes.push_front( s ); // vertex first
            else
              bndShapes.push_back( s ); // edges last
          }
        }
        if ( !nodesOnBoundary )
          break; // error: free internal link
        if ( bndShapes.front().ShapeType() == TopAbs_EDGE &&
             bndShapes.front() != bndShapes.back() )
          break; // error: link nodes on different geom edges

        // find geom edge the link is on
        if ( bndShapes.back().ShapeType() != TopAbs_EDGE )
        {
          // find geom edge by two vertices
          TopoDS_Shape geomEdge = helper.GetCommonAncestor( bndShapes.back(),
                                                            bndShapes.front(),
                                                            theMesh, TopAbs_EDGE );
          if ( geomEdge.IsNull() )
            break; // vertices belong to different edges -> error: free internal link
          bndShapes.push_back( geomEdge );
        }

        // create an edge if not yet exists
        newNodes.resize(2);
        newNodes[0] = link.node1(), newNodes[1] = link.node2();
        const SMDS_MeshElement* edge = tgtMesh->FindElement( newNodes, SMDSAbs_Edge );
        if ( edge ) continue;

        if ( link._reversed ) std::swap( newNodes[0], newNodes[1] );
        if ( link._medium )
        {
          newNodes.push_back( link._medium );
          edge = tgtMesh->AddEdge( newNodes[0], newNodes[1], newNodes[2] );

          TopoDS_Edge geomEdge = TopoDS::Edge(bndShapes.back());
          helper.CheckNodeU( geomEdge, link._medium, u, projTol, /*force=*/true );
          tgtFaceSM->RemoveNode( link._medium, /*isNodeDeleted=*/false );
          tgtMesh->SetNodeOnEdge( (SMDS_MeshNode*)link._medium, geomEdge, u );
        }
        else
        {
          edge = tgtMesh->AddEdge( newNodes[0], newNodes[1]);
        }
        if ( !edge )
          return false;

        tgtMesh->SetMeshElementOnShape( edge, bndShapes.back() );
      }
      else if ( nbFaces > 2 )
      {
        return error( "Non-manifold source mesh");
      }
    }
    isFaceMeshed = ( link2Nb == linkCount.end() && !linkCount.empty());
    if ( isFaceMeshed )
    {
      // check that source faces do not overlap:
      // there must be only two edges sharing each vertex and bound to sub-edges of theShape
      SMESH_MeshEditor editor( &theMesh );
      set<int>::iterator subID = subShapeIDs.begin();
      for ( ; subID != subShapeIDs.end(); ++subID )
      {
        const TopoDS_Shape& s = tgtMesh->IndexToShape( *subID );
        if ( s.ShapeType() != TopAbs_VERTEX ) continue;
        const SMDS_MeshNode* n = SMESH_Algo::VertexNode( TopoDS::Vertex(s), tgtMesh );
        SMDS_ElemIteratorPtr eIt = n->GetInverseElementIterator(SMDSAbs_Edge);
        int nbEdges = 0;
        while ( eIt->more() )
        {
          const SMDS_MeshElement* edge = eIt->next();
          int sId = editor.FindShape( edge );
          nbEdges += subShapeIDs.count( sId );
        }
        if ( nbEdges < 2 )
          return false; // weird
        if ( nbEdges > 2 )
          return error( "Source elements overlap one another");
      }
    }
  }
  if ( !isFaceMeshed )
    return error( "Source elements don't cover totally the geometrical face" );

  if ( helper.HasSeam() )
  {
    // links on seam edges are shared by two faces, so no edges were created on them
    // by the previous detection of 2D mesh boundary
    for ( size_t iE = 0; iE < edges.size(); ++iE )
    {
      if ( !helper.IsRealSeam( edges[iE] )) continue;
      const TopoDS_Edge& seamEdge = edges[iE];
      // to find nodes lying on the seamEdge we check nodes of mesh faces sharing a node on one
      // of its vertices; after finding another node on seamEdge we continue the same way
      // until finding all nodes.
      TopoDS_Vertex      seamVertex = helper.IthVertex( 0, seamEdge );
      const SMDS_MeshNode* vertNode = SMESH_Algo::VertexNode( seamVertex, tgtMesh );
      set< const SMDS_MeshNode* > checkedNodes; checkedNodes.insert( vertNode );
      set< const SMDS_MeshElement* > checkedFaces;
      // as a face can have more than one node on the seamEdge, there is a difficulty in selecting
      // one of those nodes to treat next; so we simply find all nodes on the seamEdge and
      // then sort them by U on edge
      typedef list< pair< double, const SMDS_MeshNode* > > TUNodeList;
      TUNodeList nodesOnSeam;
      double u = helper.GetNodeU( seamEdge, vertNode );
      nodesOnSeam.push_back( make_pair( u, vertNode ));
      TUNodeList::iterator u2nIt = nodesOnSeam.begin();
      for ( ; u2nIt != nodesOnSeam.end(); ++u2nIt )
      {
        const SMDS_MeshNode* startNode = (*u2nIt).second;
        SMDS_ElemIteratorPtr faceIt = startNode->GetInverseElementIterator( SMDSAbs_Face );
        while ( faceIt->more() )
        {
          const SMDS_MeshElement* face = faceIt->next();
          if ( !checkedFaces.insert( face ).second ) continue;
          for ( int i = 0, nbNodes = face->NbCornerNodes(); i < nbNodes; ++i )
          {
            const SMDS_MeshNode* n = face->GetNode( i );
            if ( n == startNode || !checkedNodes.insert( n ).second ) continue;
            if ( helper.CheckNodeU( seamEdge, n, u=0, projTol, /*force=*/true ))
              nodesOnSeam.push_back( make_pair( u, n ));
          }
        }
      }
      // sort the found nodes by U on the seamEdge; most probably they are in a good order,
      // so we can use the hint to spead-up map filling
      map< double, const SMDS_MeshNode* > u2nodeMap;
      for ( u2nIt = nodesOnSeam.begin(); u2nIt != nodesOnSeam.end(); ++u2nIt )
        u2nodeMap.insert( u2nodeMap.end(), *u2nIt );

      // create edges
      {
        SMESH_MesherHelper seamHelper( theMesh );
        seamHelper.SetSubShape( edges[ iE ]);
        seamHelper.SetElementsOnShape( true );

        if ( (*checkedFaces.begin())->IsQuadratic() )
          for ( set< const SMDS_MeshElement* >::iterator fIt = checkedFaces.begin();
                fIt != checkedFaces.end(); ++fIt )
            seamHelper.AddTLinks( static_cast<const SMDS_MeshFace*>( *fIt ));

        map< double, const SMDS_MeshNode* >::iterator n1, n2, u2nEnd = u2nodeMap.end();
        for ( n2 = u2nodeMap.begin(), n1 = n2++; n2 != u2nEnd; ++n1, ++n2 )
        {
          const SMDS_MeshNode* node1 = n1->second;
          const SMDS_MeshNode* node2 = n2->second;
          seamHelper.AddEdge( node1, node2 );
          if ( node2->getshapeId() == helper.GetSubShapeID() )
          {
            tgtFaceSM->RemoveNode( node2, /*isNodeDeleted=*/false );
            tgtMesh->SetNodeOnEdge( const_cast<SMDS_MeshNode*>( node2 ), seamEdge, n2->first );
          }
        }
      }
    } // loop on edges to find seam ones
  } // if ( helper.HasSeam() )

  // notify sub-meshes of edges on computation
  for ( size_t iE = 0; iE < edges.size(); ++iE )
  {
    SMESH_subMesh * sm = theMesh.GetSubMesh( edges[iE] );
    if ( BRep_Tool::Degenerated( edges[iE] ))
      sm->SetIsAlwaysComputed( true );
    sm->ComputeStateEngine(SMESH_subMesh::CHECK_COMPUTE_STATE);
    if ( sm->GetComputeState() != SMESH_subMesh::COMPUTE_OK )
      return error(SMESH_Comment("Failed to create segments on the edge ")
                   << tgtMesh->ShapeToIndex( edges[iE ]));
  }

  // ============
  // Copy meshes
  // ============

  vector<SMESH_Mesh*> srcMeshes = _sourceHyp->GetSourceMeshes();
  for ( size_t i = 0; i < srcMeshes.size(); ++i )
    StdMeshers_Import_1D::importMesh( srcMeshes[i], theMesh, _sourceHyp, theShape );

  return true;
}

//=============================================================================
/*!
 * \brief Set needed event listeners and create a submesh for a copied mesh
 *
 * This method is called only if a submesh has HYP_OK algo_state.
 */
//=============================================================================

void StdMeshers_Import_1D2D::SetEventListener(SMESH_subMesh* subMesh)
{
  if ( !_sourceHyp )
  {
    const TopoDS_Shape& tgtShape = subMesh->GetSubShape();
    SMESH_Mesh*         tgtMesh  = subMesh->GetFather();
    Hypothesis_Status aStatus;
    CheckHypothesis( *tgtMesh, tgtShape, aStatus );
  }
  StdMeshers_Import_1D::setEventListener( subMesh, _sourceHyp );
}
void StdMeshers_Import_1D2D::SubmeshRestored(SMESH_subMesh* subMesh)
{
  SetEventListener(subMesh);
}

//=============================================================================
/*!
 * Predict nb of mesh entities created by Compute()
 */
//=============================================================================

bool StdMeshers_Import_1D2D::Evaluate(SMESH_Mesh &         theMesh,
                                      const TopoDS_Shape & theShape,
                                      MapShapeNbElems&     aResMap)
{
  if ( !_sourceHyp ) return false;

  const vector<SMESH_Group*>& srcGroups = _sourceHyp->GetGroups();
  if ( srcGroups.empty() )
    return error("Invalid source groups");

  vector<int> aVec(SMDSEntity_Last,0);

  bool toCopyMesh, toCopyGroups;
  _sourceHyp->GetCopySourceMesh(toCopyMesh, toCopyGroups);
  if ( toCopyMesh ) // the whole mesh is copied
  {
    vector<SMESH_Mesh*> srcMeshes = _sourceHyp->GetSourceMeshes();
    for ( unsigned i = 0; i < srcMeshes.size(); ++i )
    {
      SMESH_subMesh* sm = StdMeshers_Import_1D::getSubMeshOfCopiedMesh( theMesh, *srcMeshes[i]);
      if ( !sm || aResMap.count( sm )) continue; // already counted
      const SMDS_MeshInfo& aMeshInfo = srcMeshes[i]->GetMeshDS()->GetMeshInfo();
      for (int i = 0; i < SMDSEntity_Last; i++)
        aVec[i] = aMeshInfo.NbEntities((SMDSAbs_EntityType)i);
    }
  }
  else
  {
    // std-like iterator used to get coordinates of nodes of mesh element
    typedef SMDS_StdIterator< SMESH_TNodeXYZ, SMDS_ElemIteratorPtr > TXyzIterator;

    SMESH_MesherHelper helper(theMesh);
    helper.SetSubShape(theShape);

    const TopoDS_Face& geomFace = TopoDS::Face( theShape );

    // take into account nodes on vertices
    TopExp_Explorer exp( theShape, TopAbs_VERTEX );
    for ( ; exp.More(); exp.Next() )
      theMesh.GetSubMesh( exp.Current())->Evaluate( aResMap );

    // to count now many times a link between nodes encounters,
    // negative nb additionally means that a link is quadratic
    map<SMESH_TLink, int> linkCount;
    map<SMESH_TLink, int>::iterator link2Nb;

    // count faces and nodes imported from groups
    set<const SMDS_MeshNode* > allNodes;
    gp_XY uv;
    double minGroupTol = 1e100;
    for ( int iG = 0; iG < srcGroups.size(); ++iG )
    {
      const SMESHDS_GroupBase* srcGroup = srcGroups[iG]->GetGroupDS();
      const double groupTol = 0.5 * sqrt( getMinElemSize2( srcGroup ));
      minGroupTol = std::min( groupTol, minGroupTol );
      SMDS_ElemIteratorPtr srcElems = srcGroup->GetElements();
      SMDS_MeshNode *tmpNode =helper.AddNode(0,0,0);
      while ( srcElems->more() ) // loop on group contents
      {
        const SMDS_MeshElement* face = srcElems->next();
        // find out if face is located on geomEdge by projecting
        // a gravity center of face to geomFace
        gp_XYZ gc(0,0,0);
        gc = accumulate( TXyzIterator(face->nodesIterator()), TXyzIterator(), gc)/face->NbNodes();
        tmpNode->setXYZ( gc.X(), gc.Y(), gc.Z());
        if ( helper.CheckNodeUV( geomFace, tmpNode, uv, groupTol, /*force=*/true ))
        {
          ++aVec[ face->GetEntityType() ];

          // collect links
          int nbConers = face->NbCornerNodes();
          for ( int i = 0; i < face->NbNodes(); ++i )
          {
            const SMDS_MeshNode* n1 = face->GetNode(i);
            allNodes.insert( n1 );
            if ( i < nbConers )
            {
              const SMDS_MeshNode* n2 = face->GetNode( (i+1)%nbConers );
              link2Nb = linkCount.insert( make_pair( SMESH_TLink( n1, n2 ), 0)).first;
              if ( (*link2Nb).second )
                link2Nb->second += (link2Nb->second < 0 ) ? -1 : 1;
              else
                link2Nb->second += ( face->IsQuadratic() ) ? -1 : 1;
            }
          }
        }
      }
      helper.GetMeshDS()->RemoveNode(tmpNode);
    }

    int nbNodes = allNodes.size();
    allNodes.clear();

    // count nodes and edges on geom edges

    double u;
    for ( exp.Init(theShape, TopAbs_EDGE); exp.More(); exp.Next() )
    {
      TopoDS_Edge geomEdge = TopoDS::Edge( exp.Current() );
      SMESH_subMesh* sm = theMesh.GetSubMesh( geomEdge );
      vector<int>& edgeVec = aResMap[sm];
      if ( edgeVec.empty() )
      {
        edgeVec.resize(SMDSEntity_Last,0);
        for ( link2Nb = linkCount.begin(); link2Nb != linkCount.end(); )
        {
          const SMESH_TLink& link = (*link2Nb).first;
          int nbFacesOfLink = Abs( link2Nb->second );
          bool eraseLink = ( nbFacesOfLink != 1 );
          if ( nbFacesOfLink == 1 )
          {
            if ( helper.CheckNodeU( geomEdge, link.node1(), u, minGroupTol, /*force=*/true )&&
                 helper.CheckNodeU( geomEdge, link.node2(), u, minGroupTol, /*force=*/true ))
            {
              bool isQuadratic = ( link2Nb->second < 0 );
              ++edgeVec[ isQuadratic ? SMDSEntity_Quad_Edge : SMDSEntity_Edge ];
              ++edgeVec[ SMDSEntity_Node ];
              --nbNodes;
              eraseLink = true;
            }
          }
          if ( eraseLink )
            linkCount.erase(link2Nb++);
          else
            link2Nb++;
        }
        if ( edgeVec[ SMDSEntity_Node] > 0 )
          --edgeVec[ SMDSEntity_Node ]; // for one node on vertex
      }
      else if ( !helper.IsSeamShape( geomEdge ) ||
                geomEdge.Orientation() == TopAbs_FORWARD )
      {
        nbNodes -= 1+edgeVec[ SMDSEntity_Node ];
      }
    }

    aVec[SMDSEntity_Node] = nbNodes;
  }

  SMESH_subMesh * sm = theMesh.GetSubMesh(theShape);
  aResMap.insert(make_pair(sm,aVec));

  return true;
}
