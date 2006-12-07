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
// File      : StdMeshers_Projection_1D.cxx
// Module    : SMESH
// Created   : Fri Oct 20 11:37:07 2006
// Author    : Edward AGAPOV (eap)


#include "StdMeshers_Projection_1D.hxx"

#include "StdMeshers_ProjectionSource1D.hxx"
#include "StdMeshers_ProjectionUtils.hxx"

#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "SMESH_Gen.hxx"

#include <BRepAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>

#include "utilities.h"


using namespace std;

#define RETURN_BAD_RESULT(msg) { MESSAGE(msg); return false; }

typedef StdMeshers_ProjectionUtils TAssocTool;

//=======================================================================
//function : StdMeshers_Projection_1D
//purpose  : 
//=======================================================================

StdMeshers_Projection_1D::StdMeshers_Projection_1D(int hypId, int studyId, SMESH_Gen* gen)
  :SMESH_1D_Algo(hypId, studyId, gen)
{
  _name = "Projection_1D";
  _shapeType = (1 << TopAbs_EDGE);	// 1 bit per shape type

  _compatibleHypothesis.push_back("ProjectionSource1D");
  _sourceHypo = 0;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshers_Projection_1D::~StdMeshers_Projection_1D()
{}

//=======================================================================
//function : CheckHypothesis
//purpose  : 
//=======================================================================

bool StdMeshers_Projection_1D::CheckHypothesis(SMESH_Mesh&                          aMesh,
                                               const TopoDS_Shape&                  aShape,
                                               SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  _sourceHypo = 0;
  list <const SMESHDS_Hypothesis * >::const_iterator itl;

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

  aStatus = SMESH_Hypothesis::HYP_OK;

  if (hypName == "ProjectionSource1D")
  {
    _sourceHypo = static_cast<const StdMeshers_ProjectionSource1D *>(theHyp);

    // Check hypo parameters

    SMESH_Mesh* srcMesh = _sourceHypo->GetSourceMesh();
    SMESH_Mesh* tgtMesh = & aMesh;
    if ( !srcMesh )
      srcMesh = tgtMesh;

    // check vertices
    if ( _sourceHypo->HasVertexAssociation() )
    {
      // source and target vertices
      if ( !TAssocTool::IsSubShape( _sourceHypo->GetSourceVertex(), srcMesh ) ||
           !TAssocTool::IsSubShape( _sourceHypo->GetTargetVertex(), tgtMesh ) ||
           !TAssocTool::IsSubShape( _sourceHypo->GetTargetVertex(), aShape )  ||
           !TAssocTool::IsSubShape( _sourceHypo->GetSourceVertex(),
                                    _sourceHypo->GetSourceEdge() ))
        aStatus = SMESH_Hypothesis::HYP_BAD_PARAMETER;
    }
    // check source edge
    if ( !TAssocTool::IsSubShape( _sourceHypo->GetSourceEdge(), srcMesh ))
      aStatus = SMESH_Hypothesis::HYP_BAD_PARAMETER;
  }
  else
  {
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;
  }
  return ( aStatus == HYP_OK );
}

//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================

bool StdMeshers_Projection_1D::Compute(SMESH_Mesh& theMesh, const TopoDS_Shape& theShape)
{
  if ( !_sourceHypo )
    return false;

  TopoDS_Edge tgtEdge = TopoDS::Edge( theShape.Oriented(TopAbs_FORWARD));
  TopoDS_Edge srcEdge = TopoDS::Edge( _sourceHypo->GetSourceEdge().Oriented(TopAbs_FORWARD));

  TopoDS_Vertex tgtV[2], srcV[2];
  TopExp::Vertices( tgtEdge, tgtV[0], tgtV[1] );
  TopExp::Vertices( srcEdge, srcV[0], srcV[1] );

  SMESH_Mesh * srcMesh = _sourceHypo->GetSourceMesh(); 
  SMESH_Mesh * tgtMesh = & theMesh;
  if ( !srcMesh )
    srcMesh = tgtMesh;

  SMESHDS_Mesh * meshDS = theMesh.GetMeshDS();

  // ---------------------------
  // Make subshapes association
  // ---------------------------

  TAssocTool::TShapeShapeMap shape2ShapeMap;
  TAssocTool::InitVertexAssociation( _sourceHypo, shape2ShapeMap );
  if ( !TAssocTool::FindSubShapeAssociation( tgtEdge, tgtMesh, srcEdge, srcMesh,
                                             shape2ShapeMap) )
    RETURN_BAD_RESULT("FindSubShapeAssociation failed");

  // ----------------------------------------------
  // Assure that mesh on a source edge is computed
  // ----------------------------------------------

  SMESH_subMesh* srcSubMesh = srcMesh->GetSubMesh( srcEdge );
  //SMESH_subMesh* tgtSubMesh = tgtMesh->GetSubMesh( tgtEdge );

  if ( tgtMesh == srcMesh ) {
    if ( !TAssocTool::MakeComputed( srcSubMesh ))
      RETURN_BAD_RESULT("Impossible to compute the source mesh");
  }
  else {
    if ( !srcSubMesh->IsMeshComputed() )
      RETURN_BAD_RESULT("Source mesh is not computed");
  }
  // -----------------------------------------------
  // Find out nodes distribution on the source edge
  // -----------------------------------------------

  double srcLength = EdgeLength( srcEdge );
  double tgtLength = EdgeLength( tgtEdge );
  
  vector< double > params; // sorted parameters of nodes on the source edge
  if ( !SMESH_Algo::GetNodeParamOnEdge( srcMesh->GetMeshDS(), srcEdge, params ))
    RETURN_BAD_RESULT("Bad node params on the source edge");

  int i, nbNodes = params.size();

  vector< double > lengths( nbNodes - 1 ); // lengths of segments of the source edge
  if ( srcLength > 0 )
  {
    BRepAdaptor_Curve curveAdaptor( srcEdge );
    for ( i = 1; i < nbNodes; ++i )
      lengths[ i-1 ] = GCPnts_AbscissaPoint::Length( curveAdaptor, params[i-1], params[i]);
  }
  else // degenerated source edge
  {
    for ( i = 1; i < nbNodes; ++i )
      lengths[ i-1 ] = params[i] - params[i-1];
    srcLength = params.back() - params[0];
  }

  bool reverse = ( srcV[0].IsSame( shape2ShapeMap( tgtV[1] )));
  if ( shape2ShapeMap.IsBound( tgtEdge )) // case of closed edge
    reverse = ( shape2ShapeMap( tgtEdge ).Orientation() == TopAbs_REVERSED );
  if ( reverse ) // reverse lengths of segments
    std::reverse( lengths.begin(), lengths.end() );

  // ----------
  // Make mesh
  // ----------

  // vector of target nodes
  vector< const SMDS_MeshNode* > nodes ( nbNodes );

  // Get the first and last nodes
  // -----------------------------

  SMESHDS_SubMesh* smV0 = meshDS->MeshElements( tgtV[0] );
  SMESHDS_SubMesh* smV1 = meshDS->MeshElements( tgtV[1] );
  if ( !smV0 || !smV1 )
    RETURN_BAD_RESULT("No submeshes on vertices");

  SMDS_NodeIteratorPtr nItV0 = smV0->GetNodes();
  SMDS_NodeIteratorPtr nItV1 = smV1->GetNodes();
  if ( !nItV0->more() || !nItV1->more() )
    RETURN_BAD_RESULT("No nodes on vertices");

  nodes.front() = nItV0->next();
  nodes.back()  = nItV1->next();

  // Compute parameters on the target edge and make internal nodes
  // --------------------------------------------------------------

  vector< double > tgtParams( nbNodes );

  BRep_Tool::Range( tgtEdge, tgtParams.front(), tgtParams.back() );
  if ( tgtLength <= 0 )
    tgtLength = tgtParams.back() - tgtParams.front();
  double dl = tgtLength / srcLength;

  if ( tgtLength > 0 )
  {
    BRepAdaptor_Curve curveAdaptor( tgtEdge );

    // compute params on internal nodes
    for ( i = 1; i < nbNodes - 1; ++i )
    {
      // computes a point on a <curveAdaptor> at the given distance
      // from the point at given parameter.
      GCPnts_AbscissaPoint Discret( curveAdaptor, dl * lengths[ i-1 ], tgtParams[ i-1 ] );
      if ( !Discret.IsDone() )
        RETURN_BAD_RESULT(" GCPnts_AbscissaPoint failed");
      tgtParams[ i ] = Discret.Parameter();
    }
    // make internal nodes 
    for ( i = 1; i < nbNodes - 1; ++i )
    {
      gp_Pnt P = curveAdaptor.Value( tgtParams[ i ]);
      SMDS_MeshNode* node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnEdge( node, tgtEdge, tgtParams[ i ]);
      nodes[ i ] = node;
    }
  }
  else // degenerated target edge
  {
    // compute params and make internal nodes
    gp_Pnt P = BRep_Tool::Pnt( tgtV[0] );

    for ( i = 1; i < nbNodes - 1; ++i )
    {
      SMDS_MeshNode* node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      tgtParams[ i ] = tgtParams[ i-1 ] + dl * lengths[ i-1 ];
      meshDS->SetNodeOnEdge( node, tgtEdge, tgtParams[ i ]);
      nodes[ i ] = node;
    }
  }

  // Quadratic mesh?
  // ----------------

  bool quadratic = false;
  SMDS_ElemIteratorPtr elemIt = srcSubMesh->GetSubMeshDS()->GetElements();
  if ( elemIt->more() )
    quadratic = elemIt->next()->IsQuadratic();
  else {
    SMDS_NodeIteratorPtr nodeIt = srcSubMesh->GetSubMeshDS()->GetNodes();
    while ( nodeIt->more() && !quadratic )
      quadratic = SMESH_MesherHelper::IsMedium( nodeIt->next() );
  }
  // enough nodes to make all edges quadratic?
  if ( quadratic && ( nbNodes < 3 || ( nbNodes % 2 != 1 )))
    RETURN_BAD_RESULT("Wrong nb nodes to make quadratic mesh");

  // Create edges
  // -------------

  SMDS_MeshElement* edge = 0;
  int di = quadratic ? 2 : 1;
  for ( i = di; i < nbNodes; i += di)
  {
    if ( quadratic )
      edge = meshDS->AddEdge( nodes[i-2], nodes[i], nodes[i-1] );
    else
      edge = meshDS->AddEdge( nodes[i-1], nodes[i] );
    meshDS->SetMeshElementOnShape(edge, tgtEdge );
  }

  return true;
}

//=============================================================================
/*!
 * \brief Sets a default event listener to submesh of the source edge
  * \param subMesh - submesh where algo is set
 *
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 * Arranges that CLEAN event is translated from source submesh to
 * the submesh
 */
//=============================================================================

void StdMeshers_Projection_1D::SetEventListener(SMESH_subMesh* subMesh)
{
  if ( _sourceHypo && ! _sourceHypo->GetSourceEdge().IsNull() )
  {
    SMESH_Mesh* srcMesh = _sourceHypo->GetSourceMesh();
    if ( !srcMesh )
      srcMesh = subMesh->GetFather();

    SMESH_subMesh* srcEdgeSM =
      srcMesh->GetSubMesh( _sourceHypo->GetSourceEdge() );

    if ( srcEdgeSM != subMesh )
      subMesh->SetEventListener( new SMESH_subMeshEventListener(true),
                                 SMESH_subMeshEventListenerData::MakeData( subMesh ),
                                 srcEdgeSM );
  }
}
  
