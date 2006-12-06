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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
// File      : StdMeshers_Projection_2D.cxx
// Module    : SMESH
// Created   : Fri Oct 20 11:37:07 2006
// Author    : Edward AGAPOV (eap)


#include "StdMeshers_Projection_2D.hxx"

#include "StdMeshers_ProjectionSource2D.hxx"
#include "StdMeshers_ProjectionUtils.hxx"

#include "SMESHDS_Hypothesis.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_Pattern.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMDS_EdgePosition.hxx"

#include "utilities.h"

#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <BRep_Tool.hxx>



using namespace std;

#define RETURN_BAD_RESULT(msg) { MESSAGE(msg); return false; }

typedef StdMeshers_ProjectionUtils TAssocTool;

//=======================================================================
//function : StdMeshers_Projection_2D
//purpose  : 
//=======================================================================

StdMeshers_Projection_2D::StdMeshers_Projection_2D(int hypId, int studyId, SMESH_Gen* gen)
  :SMESH_2D_Algo(hypId, studyId, gen)
{
  _name = "Projection_2D";
  _shapeType = (1 << TopAbs_FACE);	// 1 bit per shape type

  _compatibleHypothesis.push_back("ProjectionSource2D");
  _sourceHypo = 0;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshers_Projection_2D::~StdMeshers_Projection_2D()
{}

//=======================================================================
//function : CheckHypothesis
//purpose  : 
//=======================================================================

bool StdMeshers_Projection_2D::CheckHypothesis(SMESH_Mesh&                          theMesh,
                                               const TopoDS_Shape&                  theShape,
                                               SMESH_Hypothesis::Hypothesis_Status& theStatus)
{
  list <const SMESHDS_Hypothesis * >::const_iterator itl;

  const list <const SMESHDS_Hypothesis * >&hyps = GetUsedHypothesis(theMesh, theShape);
  if ( hyps.size() == 0 )
  {
    theStatus = HYP_MISSING;
    return false;  // can't work with no hypothesis
  }

  if ( hyps.size() > 1 )
  {
    theStatus = HYP_ALREADY_EXIST;
    return false;
  }

  const SMESHDS_Hypothesis *theHyp = hyps.front();

  string hypName = theHyp->GetName();

  theStatus = HYP_OK;

  if (hypName == "ProjectionSource2D")
  {
    _sourceHypo = static_cast<const StdMeshers_ProjectionSource2D *>(theHyp);

    // Check hypo parameters

    SMESH_Mesh* srcMesh = _sourceHypo->GetSourceMesh();
    SMESH_Mesh* tgtMesh = & theMesh;
    if ( !srcMesh )
      srcMesh = tgtMesh;

    // check vertices
    if ( _sourceHypo->HasVertexAssociation() )
    {
      // source vertices
      TopoDS_Shape edge = TAssocTool::GetEdgeByVertices
        ( srcMesh, _sourceHypo->GetSourceVertex(1), _sourceHypo->GetSourceVertex(2) );
      if ( edge.IsNull() ||
           !TAssocTool::IsSubShape( edge, srcMesh ) ||
           !TAssocTool::IsSubShape( edge, _sourceHypo->GetSourceFace() ))
      {
        SCRUTE((edge.IsNull()));
        SCRUTE((TAssocTool::IsSubShape( edge, srcMesh )));
        SCRUTE((TAssocTool::IsSubShape( edge, _sourceHypo->GetSourceFace() )));
        theStatus = HYP_BAD_PARAMETER;
      }
      else
      {
        // target vertices
        edge = TAssocTool::GetEdgeByVertices
          ( tgtMesh, _sourceHypo->GetTargetVertex(1), _sourceHypo->GetTargetVertex(2) );
        if ( edge.IsNull() ||
             !TAssocTool::IsSubShape( edge, tgtMesh ) ||
             !TAssocTool::IsSubShape( edge, theShape ))
        {
          SCRUTE((edge.IsNull()));
          SCRUTE((TAssocTool::IsSubShape( edge, tgtMesh )));
          SCRUTE((TAssocTool::IsSubShape( edge, theShape )));
          theStatus = HYP_BAD_PARAMETER;
        }
      }
    }
    // check a source face
    if ( !TAssocTool::IsSubShape( _sourceHypo->GetSourceFace(), srcMesh )) {
      MESSAGE("Bad source face"); 
      theStatus = HYP_BAD_PARAMETER;
    }
  }
  else
  {
    theStatus = HYP_INCOMPATIBLE;
  }
  return ( theStatus == HYP_OK );
}

namespace {


  //================================================================================
  /*!
   * \brief define if a node is new or old
    * \param node - node to check
    * \retval bool - true if the node existed before Compute() is called
   */
  //================================================================================

  bool isOldNode( const SMDS_MeshNode* node )
  {
    // old nodes are shared by edges and new ones are shared
    // only by faces created by mapper
    bool isOld = false;
    SMDS_ElemIteratorPtr invElem = node->GetInverseElementIterator();
    while ( !isOld && invElem->more() )
      isOld = ( invElem->next()->GetType() == SMDSAbs_Edge );
    return isOld;
  }

  //================================================================================
  /*!
   * \brief Class to remove mesh built by pattern mapper on edges
   * and vertices in the case of failure of projection algo.
   * It does it's job at destruction
   */
  //================================================================================

  class MeshCleaner {
    SMESH_subMesh* sm;
  public:
    MeshCleaner( SMESH_subMesh* faceSubMesh ): sm(faceSubMesh) {}
    ~MeshCleaner() { Clean(sm); }
    void Release() { sm = 0; } // mesh will not be removed
    static void Clean( SMESH_subMesh* sm )
    {
      if ( !sm ) return;
      switch ( sm->GetSubShape().ShapeType() ) {
      case TopAbs_VERTEX:
      case TopAbs_EDGE: {
        SMDS_NodeIteratorPtr nIt = sm->GetSubMeshDS()->GetNodes();
        SMESHDS_Mesh* mesh = sm->GetFather()->GetMeshDS();
        while ( nIt->more() ) {
          const SMDS_MeshNode* node = nIt->next();
          if ( !isOldNode( node ) )
            mesh->RemoveNode( node );
        }
        // do not break but iterate over DependsOn()
      }
      default:
        const map< int, SMESH_subMesh * >& subSM = sm->DependsOn();
        map< int, SMESH_subMesh * >::const_iterator i_sm = subSM.begin();
        for ( ; i_sm != subSM.end(); ++i_sm )
          Clean( i_sm->second );
      }
    }
  };

  //================================================================================
  /*!
   * \brief find new nodes belonging to one free border of mesh on face
    * \param sm - submesh on edge or vertex containg nodes to choose from
    * \param face - the face bound the submesh
    * \param u2nodes - map to fill with nodes
    * \param seamNodes - set of found nodes
    * \retval bool - is a success
   */
  //================================================================================

  bool getBoundaryNodes ( SMESH_subMesh*                        sm,
                          const TopoDS_Face&                    face,
                          map< double, const SMDS_MeshNode* > & u2nodes,
                          set< const SMDS_MeshNode* > &         seamNodes)
  {
    u2nodes.clear();
    seamNodes.clear();
    if ( !sm || !sm->GetSubMeshDS() )
      RETURN_BAD_RESULT("Null submesh");

    SMDS_NodeIteratorPtr nIt = sm->GetSubMeshDS()->GetNodes();
    switch ( sm->GetSubShape().ShapeType() ) {

    case TopAbs_VERTEX: {
      while ( nIt->more() ) {
        const SMDS_MeshNode* node = nIt->next();
        if ( isOldNode( node ) ) continue;
        u2nodes.insert( make_pair( 0., node ));
        seamNodes.insert( node );
        return true;
      }
      break;
    }
    case TopAbs_EDGE: {
      
      // Get submeshes of sub-vertices
      const map< int, SMESH_subMesh * >& subSM = sm->DependsOn();
      if ( subSM.size() != 2 )
        RETURN_BAD_RESULT("there must be 2 submeshes of sub-vertices"
                          " but we have " << subSM.size());
      SMESH_subMesh* smV1 = subSM.begin()->second;
      SMESH_subMesh* smV2 = subSM.rbegin()->second;
      if ( !smV1->IsMeshComputed() || !smV2->IsMeshComputed() )
        RETURN_BAD_RESULT("Empty vertex submeshes");

      // Look for a new node on V1
      nIt = smV1->GetSubMeshDS()->GetNodes();
      const SMDS_MeshNode* nV1 = 0;
      while ( nIt->more() && !nV1 ) {
        const SMDS_MeshNode* node = nIt->next();
        if ( !isOldNode( node ) ) nV1 = node;
      }
      if ( !nV1 )
        RETURN_BAD_RESULT("No new node found on V1");

      // Find a new node connected to nV1 and belonging to edge submesh;
      const SMDS_MeshNode* nE = 0;
      SMESHDS_SubMesh* smDS = sm->GetSubMeshDS();
      SMDS_ElemIteratorPtr vElems = nV1->GetInverseElementIterator();
      while ( vElems->more() && !nE ) {
        const SMDS_MeshElement* elem = vElems->next();
        if ( elem->GetType() != SMDSAbs_Face )
          continue; // new nodes are shared by faces
        int nbNodes = elem->NbNodes();
        if ( elem->IsQuadratic() )
          nbNodes /= 2;
        int iV1 = elem->GetNodeIndex( nV1 );
        // try next aftre nV1
        int iE = SMESH_MesherHelper::WrapIndex( iV1 + 1, nbNodes );
        if ( smDS->Contains( elem->GetNode( iE ) ))
          nE = elem->GetNode( iE );
        if ( !nE ) {
          // try node before nV1
          iE = SMESH_MesherHelper::WrapIndex( iV1 - 1, nbNodes );
          if ( smDS->Contains( elem->GetNode( iE )))
            nE = elem->GetNode( iE );
        }
        if ( nE && elem->IsQuadratic() ) { // find medium node between nV1 and nE
          if ( Abs( iV1 - iE ) == 1 )
            nE = elem->GetNode( Min ( iV1, iE ) + nbNodes );
          else
            nE = elem->GetNode( elem->NbNodes() - 1 );
        }
      }
      if ( !nE )
        RETURN_BAD_RESULT("new node on edge not found");

      // Get the whole free border of a face
      list< const SMDS_MeshNode* > bordNodes;
      list< const SMDS_MeshElement* > bordFaces;
      if ( !SMESH_MeshEditor::FindFreeBorder (nV1, nE, nV1, bordNodes, bordFaces ))
        RETURN_BAD_RESULT("free border of a face not found by nodes " <<
                          nV1->GetID() << " " << nE->GetID() );

      // Insert nodes of the free border to the map until node on V2 encountered
      SMESHDS_SubMesh* v2smDS = smV2->GetSubMeshDS();
      list< const SMDS_MeshNode* >::iterator bordIt = bordNodes.begin();
      bordIt++; // skip nV1
      for ( ; bordIt != bordNodes.end(); ++bordIt ) {
        const SMDS_MeshNode* node = *bordIt;
        if ( v2smDS->Contains( node ))
          break;
        if ( node->GetPosition()->GetTypeOfPosition() != SMDS_TOP_EDGE )
          RETURN_BAD_RESULT("Bad node position type: node " << node->GetID() <<
                            " pos type " << node->GetPosition()->GetTypeOfPosition());
        const SMDS_EdgePosition* pos =
          static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
        u2nodes.insert( make_pair( pos->GetUParameter(), node ));
        seamNodes.insert( node );
      }
      if ( u2nodes.size() != seamNodes.size() )
        RETURN_BAD_RESULT("Bad node params on edge " << sm->GetId() <<
                          ", " << u2nodes.size() << " != " << seamNodes.size() );
      return true;
    }
    default:;
    }
    RETURN_BAD_RESULT ("Unexpected submesh type");

  } // bool getBoundaryNodes()

} // namespace

//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

bool StdMeshers_Projection_2D::Compute(SMESH_Mesh& theMesh, const TopoDS_Shape& theShape)
{
  if ( !_sourceHypo )
    return false;

  TopoDS_Face tgtFace = TopoDS::Face( theShape.Oriented(TopAbs_FORWARD));
  TopoDS_Face srcFace = TopoDS::Face( _sourceHypo->GetSourceFace().Oriented(TopAbs_FORWARD));

  SMESH_Mesh * srcMesh = _sourceHypo->GetSourceMesh(); 
  SMESH_Mesh * tgtMesh = & theMesh;
  if ( !srcMesh )
    srcMesh = tgtMesh;

  SMESHDS_Mesh * meshDS = theMesh.GetMeshDS();

  SMESH_MesherHelper helper( theMesh );
  helper.SetSubShape( tgtFace );

  // ---------------------------
  // Make subshapes association
  // ---------------------------

  TAssocTool::TShapeShapeMap shape2ShapeMap;
  TAssocTool::InitVertexAssociation( _sourceHypo, shape2ShapeMap );
  if ( !TAssocTool::FindSubShapeAssociation( tgtFace, tgtMesh, srcFace, srcMesh,
                                             shape2ShapeMap) )
    RETURN_BAD_RESULT("FindSubShapeAssociation failed");

  // ----------------------------------------------
  // Assure that mesh on a source Face is computed
  // ----------------------------------------------

  SMESH_subMesh* srcSubMesh = srcMesh->GetSubMesh( srcFace );
  SMESH_subMesh* tgtSubMesh = tgtMesh->GetSubMesh( tgtFace );

  if ( tgtMesh == srcMesh ) {
    if ( !TAssocTool::MakeComputed( srcSubMesh ))
      RETURN_BAD_RESULT("Impossible to compute the source mesh");
  }
  else {
    if ( !srcSubMesh->IsMeshComputed() )
      RETURN_BAD_RESULT("Source mesh is not computed");
  }

  // --------------------
  // Prepare to mapping 
  // --------------------

  // Load pattern from the source face
  SMESH_Pattern mapper;
  mapper.Load( srcMesh, srcFace );
  if ( mapper.GetErrorCode() != SMESH_Pattern::ERR_OK )
    RETURN_BAD_RESULT("SMESH_Pattern::Load() failed");

  // Find the first target vertex corresponding to first vertex of the <mapper>
  // and <theReverse> flag needed to call mapper.Apply()

  TopoDS_Vertex srcV1 = TopoDS::Vertex( mapper.GetSubShape( 1 ));
  if ( srcV1.IsNull() )
    RETURN_BAD_RESULT("Mesh is not bound to the face");
  if ( !shape2ShapeMap.IsBound( srcV1 ))
    RETURN_BAD_RESULT("Not associated vertices, srcV1 " << srcV1.TShape().operator->() );
  TopoDS_Vertex tgtV1 = TopoDS::Vertex( shape2ShapeMap( srcV1 ));

  if ( !TAssocTool::IsSubShape( srcV1, srcFace ))
    RETURN_BAD_RESULT("Wrong srcV1 " << srcV1.TShape().operator->());
  if ( !TAssocTool::IsSubShape( tgtV1, tgtFace ))
    RETURN_BAD_RESULT("Wrong tgtV1 " << tgtV1.TShape().operator->());

  // try to find out orientation by order of edges
  bool reverse = false;
  list< TopoDS_Edge > tgtEdges, srcEdges;
  list< int > nbEdgesInWires;
  SMESH_Block::GetOrderedEdges( tgtFace, tgtV1, tgtEdges, nbEdgesInWires);
  SMESH_Block::GetOrderedEdges( srcFace, srcV1, srcEdges, nbEdgesInWires);
  if ( nbEdgesInWires.front() > 1 ) // possible to find out
  {
    TopoDS_Edge srcE1 = srcEdges.front(), tgtE1 = tgtEdges.front();
    reverse = ( ! srcE1.IsSame( shape2ShapeMap( tgtE1 )));
    if ( BRep_Tool::IsClosed( tgtE1, tgtFace )) {
      reverse = ( srcE1.Orientation() == tgtE1.Orientation() );
      if ( _sourceHypo->GetSourceFace().Orientation() != theShape.Orientation() )
        reverse = !reverse;
    }
  }
  else if ( nbEdgesInWires.front() == 1 )
  {
    // TODO::Compare orientation of curves in a sole edge
    //RETURN_BAD_RESULT("Not implemented case");
  }
  else
  {
    RETURN_BAD_RESULT("Bad result from SMESH_Block::GetOrderedEdges()");
  }

  // --------------------
  // Perform 2D mapping 
  // --------------------

  // Compute mesh on a target face

  mapper.Apply( tgtFace, tgtV1, reverse );
  if ( mapper.GetErrorCode() != SMESH_Pattern::ERR_OK )
    RETURN_BAD_RESULT("SMESH_Pattern::Apply() failed");

  // Create the mesh

  const bool toCreatePolygons = false, toCreatePolyedrs = false;
  mapper.MakeMesh( tgtMesh, toCreatePolygons, toCreatePolyedrs );
  if ( mapper.GetErrorCode() != SMESH_Pattern::ERR_OK )
    RETURN_BAD_RESULT("SMESH_Pattern::MakeMesh() failed");

  // it will remove mesh built by pattern mapper on edges and vertices
  // in failure case
  MeshCleaner cleaner( tgtSubMesh );

  // -------------------------------------------------------------------------
  // mapper doesn't take care of nodes already existing on edges and vertices,
  // so we must merge nodes created by it with existing ones 
  // -------------------------------------------------------------------------

  SMESH_MeshEditor editor( tgtMesh );
  SMESH_MeshEditor::TListOfListOfNodes groupsOfNodes;

  // Make groups of nodes to merge

  // loop on edge and vertex submeshes of a target face
  const map< int, SMESH_subMesh * >& subSM = tgtSubMesh->DependsOn();
  map< int, SMESH_subMesh * >::const_iterator i_subSM = subSM.begin();
  for ( ; i_subSM != subSM.end(); ++i_subSM )
  {
    SMESH_subMesh*     sm = i_subSM->second;
    SMESHDS_SubMesh* smDS = sm->GetSubMeshDS();

    // Sort new and old nodes of a submesh separately

    bool isSeam = helper.IsSeamShape( sm->GetId() );

    enum { NEW_NODES, OLD_NODES };
    map< double, const SMDS_MeshNode* > u2nodesMaps[2], u2nodesOnSeam;
    map< double, const SMDS_MeshNode* >::iterator u_oldNode, u_newNode, u_newOnSeam, newEnd;
    set< const SMDS_MeshNode* > seamNodes;

    // mapper puts on a seam edge nodes from 2 edges
    if ( isSeam && ! getBoundaryNodes ( sm, tgtFace, u2nodesOnSeam, seamNodes ))
      RETURN_BAD_RESULT("getBoundaryNodes() failed");

    SMDS_NodeIteratorPtr nIt = smDS->GetNodes();
    while ( nIt->more() )
    {
      const SMDS_MeshNode* node = nIt->next();
      bool isOld = isOldNode( node );

      if ( !isOld && isSeam ) { // new node on a seam edge
        if ( seamNodes.find( node ) != seamNodes.end())
          continue; // node is already in the map
      }

      // sort nodes on edges by its position
      map< double, const SMDS_MeshNode* > & pos2nodes = u2nodesMaps[ isOld ];
      switch ( node->GetPosition()->GetTypeOfPosition() )
      {
      case  SMDS_TOP_VERTEX: {
        pos2nodes.insert( make_pair( 0, node ));
        break;
      }
      case  SMDS_TOP_EDGE:   {
        const SMDS_EdgePosition* pos =
          static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
        pos2nodes.insert( make_pair( pos->GetUParameter(), node ));
        break;
      }
      default:
        RETURN_BAD_RESULT("Wrong node position type: "<<
                          node->GetPosition()->GetTypeOfPosition());
      }
    }
    if ( u2nodesMaps[ OLD_NODES ].size() != u2nodesMaps[ NEW_NODES ].size() )
      RETURN_BAD_RESULT("Different nb of old and new nodes " <<
                        u2nodesMaps[ OLD_NODES ].size() << " != " <<
                        u2nodesMaps[ NEW_NODES ].size());
    if ( isSeam && u2nodesMaps[ OLD_NODES ].size() != u2nodesOnSeam.size() )
      RETURN_BAD_RESULT("Different nb of old and seam nodes " <<
                        u2nodesMaps[ OLD_NODES ].size() << " != " << u2nodesOnSeam.size());

    // Make groups of nodes to merge
    u_oldNode = u2nodesMaps[ OLD_NODES ].begin(); 
    u_newNode = u2nodesMaps[ NEW_NODES ].begin();
    newEnd    = u2nodesMaps[ NEW_NODES ].end();
    u_newOnSeam = u2nodesOnSeam.begin();
    for ( ; u_newNode != newEnd; ++u_newNode, ++u_oldNode ) {
      groupsOfNodes.push_back( list< const SMDS_MeshNode* >() );
      groupsOfNodes.back().push_back( u_oldNode->second );
      groupsOfNodes.back().push_back( u_newNode->second );
      if ( isSeam )
        groupsOfNodes.back().push_back( (u_newOnSeam++)->second );
    }
  }

  // Merge

  editor.MergeNodes( groupsOfNodes );

  // ---------------------------
  // Check elements orientation
  // ---------------------------

  TopoDS_Face face = tgtFace;
  if ( !theMesh.IsMainShape( tgtFace ))
  {
    // find the main shape
    TopoDS_Shape mainShape = meshDS->ShapeToMesh();
    switch ( mainShape.ShapeType() ) {
    case TopAbs_SHELL:
    case TopAbs_SOLID: break;
    default:
      TopTools_ListIteratorOfListOfShape ancestIt = theMesh.GetAncestors( face );
      for ( ; ancestIt.More(); ancestIt.Next() ) {
        TopAbs_ShapeEnum type = ancestIt.Value().ShapeType();
        if ( type == TopAbs_SOLID ) {
          mainShape = ancestIt.Value();
          break;
        } else if ( type == TopAbs_SHELL ) {
          mainShape = ancestIt.Value();
        }
      }
    }
    // find tgtFace in the main solid or shell to know it's true orientation.
    TopExp_Explorer exp( mainShape, TopAbs_FACE );
    for ( ; exp.More(); exp.Next() ) {
      if ( tgtFace.IsSame( exp.Current() )) {
        face = TopoDS::Face( exp.Current() );
        break;
      }
    }
  }
  // Fix orientation
  if ( SMESH_Algo::IsReversedSubMesh( face, meshDS ))
  {
    SMDS_ElemIteratorPtr eIt = meshDS->MeshElements( face )->GetElements();
    while ( eIt->more() ) {
      const SMDS_MeshElement* e = eIt->next();
      if ( e->GetType() == SMDSAbs_Face && !editor.Reorient( e ))
        RETURN_BAD_RESULT("Pb of SMESH_MeshEditor::Reorient()");
    }
  }

  cleaner.Release(); // do not remove mesh

  return true;
}

//=============================================================================
/*!
 * \brief Sets a default event listener to submesh of the source face
  * \param subMesh - submesh where algo is set
 *
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 * Arranges that CLEAN event is translated from source submesh to
 * the submesh
 */
//=============================================================================

void StdMeshers_Projection_2D::SetEventListener(SMESH_subMesh* subMesh)
{
  if ( _sourceHypo && ! _sourceHypo->GetSourceFace().IsNull() )
  {
    SMESH_Mesh* srcMesh = _sourceHypo->GetSourceMesh();
    if ( !srcMesh )
      srcMesh = subMesh->GetFather();

    SMESH_subMesh* srcFaceSM =
      srcMesh->GetSubMesh( _sourceHypo->GetSourceFace() );

    subMesh->SetEventListener( new SMESH_subMeshEventListener(),
                               SMESH_subMeshEventListenerData::MakeData( subMesh ),
                               srcFaceSM );
  }
}
