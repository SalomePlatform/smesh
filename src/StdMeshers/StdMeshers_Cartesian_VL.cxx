// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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
// File      : StdMeshers_Cartesian_VL.cxx
// Created   : Tue May 24 13:03:09 2022
// Author    : Edward AGAPOV (eap)

#include "StdMeshers_Cartesian_VL.hxx"
#include "StdMeshers_ViscousLayers.hxx"

#include <SMDS_MeshGroup.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESHDS_SubMesh.hxx>
#include <SMESH_Algo.hxx>
#include <SMESH_MeshAlgos.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MeshEditor.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_TypeDefs.hxx>
#include <SMESH_subMesh.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepGProp.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

using namespace StdMeshers_Cartesian_VL;

namespace
{
  typedef int                     TGeomID; // IDs of sub-shapes

  /*!
   * \brief Temporary mesh
   */
  struct TmpMesh: public SMESH_Mesh
  {
    TmpMesh() {
      _isShapeToMesh = (_id = 0);
      _meshDS  = new SMESHDS_Mesh( _id, true );
    }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Edge of viscous layers; goes from a grid node by normal to boundary
   */
  struct VLEdge
  {
    std::vector< SMESH_NodeXYZ > _nodes;
    double                       _uv[2]; // position of TgtNode() on geometry
    double                       _length; 

    const SMDS_MeshNode* TgtNode() const { return _nodes.back().Node(); }
  };
  typedef NCollection_DataMap< const SMDS_MeshNode*, VLEdge* > TNode2VLEdge;
  // --------------------------------------------------------------------------
  /*!
   * \brief VLEdge's of one shape
   */
  struct VLEdgesOnShape
  {
    std::vector< VLEdge > _edges;
    TopoDS_Shape          _initShape;
    TopoDS_Shape          _offsetShape;
    int                   _initShapeID;
    bool                  _hasVL;
    bool                  _toCheckCoinc; // to check if nodes on shape boundary coincide
  };

  //================================================================================
  /*!
   * \brief Project a point on offset FACE to EDGEs of an initial FACE
   *  \param [in] offP - point to project
   *  \param [in] initFace - FACE to project on
   *  \return gp_Pnt - projection point
   */
  //================================================================================

  gp_Pnt projectToWire( const gp_Pnt&      offP,
                        const TopoDS_Face& initFace,
                        gp_Pnt2d &         uv )
  {
    double   minDist = Precision::Infinite();
    const double tol = Precision::Confusion();
    gp_Pnt              proj, projction;
    Standard_Real       param;
    ShapeAnalysis_Curve projector;

    for ( TopExp_Explorer eExp( initFace, TopAbs_EDGE ); eExp.More(); eExp.Next() )
    {
      BRepAdaptor_Curve initCurve( TopoDS::Edge( eExp.Current() ));
      //const double f = initCurve.FirstParameter(), l = initCurve.LastParameter();
      double dist = projector.Project( initCurve, offP, tol, proj, param, /*adjustToEnds=*/false );
      if ( dist < minDist )
      {
        projction = proj;
        minDist = dist;
      }
    }
    uv.SetCoord(0.,0.); // !!!!!!!
    return projction;
  }

  //================================================================================
  /*!
   * \brief Project nodes from offset FACE to initial FACE
   *  \param [inout] theEOS - VL edges on a geom FACE
   *  \param [inout] theOffsetMDS - offset mesh to fill in
   */
  //================================================================================

  void projectToFace( VLEdgesOnShape & theEOS,
                      SMESHDS_Mesh*    theOffsetMDS,
                      TNode2VLEdge &   theN2E )
  {
    SMESHDS_SubMesh* sm = theOffsetMDS->MeshElements( theEOS._offsetShape );
    if ( !sm || sm->NbElements() == 0 || sm->NbNodes() == 0 )
      return;

    theEOS._edges.resize( sm->NbNodes() );

    const TopoDS_Face& initFace = TopoDS::Face( theEOS._initShape );
    ShapeAnalysis_Surface projector( BRep_Tool::Surface( initFace ));
    const double clsfTol = 1e2 * BRep_Tool::MaxTolerance( initFace, TopAbs_VERTEX );
    BRepTopAdaptor_FClass2d classifier( initFace, clsfTol );

    const double        tol = Precision::Confusion();
    //const double f = initCurve.FirstParameter(), l = initCurve.LastParameter();
    gp_Pnt              proj;

    int iN = 0;
    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); ++iN )
    {
      SMESH_NodeXYZ offP = nIt->next();
      gp_Pnt2d uv = projector.ValueOfUV( offP, tol );
      TopAbs_State st = classifier.Perform( uv );
      if ( st == TopAbs_IN || st == TopAbs_ON )
      {
        proj = projector.Value( uv );
      }
      else
      {
        proj = projectToWire( offP, initFace, uv );
      }
      if ( st == TopAbs_ON || st == TopAbs_OUT )
        theEOS._toCheckCoinc = true;

      VLEdge & vlEdge = theEOS._edges[ iN ];
      vlEdge._nodes.push_back( offP.Node() );
      vlEdge._nodes.push_back( theOffsetMDS->AddNode( proj.X(), proj.Y(), proj.Z() ));
      vlEdge._uv[0] = uv.X();
      vlEdge._uv[1] = uv.Y();
      //vlEdge._length = proj.Distance( offP );

      theN2E.Bind( offP.Node(), &vlEdge );
    }
    return;
    
  }

  //================================================================================
  /*!
   * \brief Project nodes from offset EDGE to initial EDGE
   *  \param [inout] theEOS - VL edges on a geom EDGE
   *  \param [inout] theOffsetMDS - offset mesh to add new nodes to
   */
  //================================================================================

  void projectToEdge( VLEdgesOnShape & theEOS,
                      SMESHDS_Mesh*    theOffsetMDS,
                      TNode2VLEdge &   theN2E,
                      bool createVertex )
  {
    SMESHDS_SubMesh* sm = theOffsetMDS->MeshElements( theEOS._offsetShape );
 
    if ( !sm || sm->NbElements() == 0 )
      return;

    int addVertexNode = createVertex ? 1 : 0;
    theEOS._edges.resize( sm->NbNodes() + addVertexNode ); // +1 to set the vertex 
    
    ShapeAnalysis_Curve projector;
    BRepAdaptor_Curve   initCurve( TopoDS::Edge( theEOS._initShape ));
    const double        tol = Precision::Confusion();
    const double f = initCurve.FirstParameter(), l = initCurve.LastParameter();
    gp_Pnt              proj;
    Standard_Real       param;

    int iN = 0;
    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); ++iN )
    {
      SMESH_NodeXYZ offP = nIt->next();
      double dist = projector.Project( initCurve, offP, tol, proj, param, /*adjustToEnds=*/false );
      bool paramOK = ( f < param && param < l );
      if ( !paramOK )
      {
        if ( param < f )
          param = f;
        else if ( param > l )
          param = l;
        theEOS._toCheckCoinc = true;
      }
      proj = initCurve.Value( param );

      VLEdge & vlEdge = theEOS._edges[ iN ];
      vlEdge._nodes.push_back( offP.Node() );
      vlEdge._nodes.push_back( theOffsetMDS->AddNode( proj.X(), proj.Y(), proj.Z() ));
      vlEdge._uv[0] = param;
      vlEdge._length = paramOK ? dist : proj.Distance( offP );

      theN2E.Bind( offP.Node(), &vlEdge );
    }

    if ( createVertex )
    {
      // It is possible to define the vertex projections from the existing edges
      // EOS._offsetShape the edge generated from the original edge
      // Get the first vertex of both edges to define the connecting edges
      auto offsetEdge = TopoDS::Edge( theEOS._offsetShape );
      auto initEdge   = TopoDS::Edge( theEOS._initShape );
      TopoDS_Vertex offsetVertex;
      TopoDS_Vertex initVertex;
      
      if ( offsetEdge.Orientation() == TopAbs_FORWARD )
      {
        offsetVertex  = TopExp::FirstVertex ( offsetEdge );
        initVertex    = TopExp::FirstVertex ( initEdge ); 
      }
      else
      {
        offsetVertex  = TopExp::LastVertex ( offsetEdge );
        initVertex    = TopExp::LastVertex ( initEdge ); 
      }

      VLEdge & vlEdge = theEOS._edges[ iN ];
      vlEdge._nodes.resize( 2 );
      vlEdge._nodes[0] = SMESH_Algo::VertexNode( offsetVertex, theOffsetMDS );
      
      gp_Pnt offP = BRep_Tool::Pnt( initVertex );
      
      vlEdge._nodes[1] = theOffsetMDS->AddNode( offP.X(), offP.Y(), offP.Z() );
      theN2E.Bind( vlEdge._nodes[0].Node(), &vlEdge );
    }
    return;
  }  

  //================================================================================
  /*!
   * \brief Compute heights of viscous layers and finds FACEs with VL
   *  \param [in] hyp - viscous layers hypothesis
   *  \param [in] mesh - the main mesh
   *  \param [in] shape - the main shape
   *  \param [out] vlH - heights of viscous layers to compute
   *  \param [out] shapesWVL - IDs of shapes with VL
   *  \return bool - isOK
   */
  //================================================================================

  void computeVLHeight( const StdMeshers_ViscousLayers* hyp,
                        std::vector< double > &         vlH )
  {
    const double  T = hyp->GetTotalThickness();
    const double  f = hyp->GetStretchFactor();
    const int     N = hyp->GetNumberLayers();
    const double h0 = hyp->Get1stLayerThickness( T, f, N );

    vlH.reserve( hyp->GetNumberLayers() );
    vlH.push_back( h0 );
    for ( double h = h0 * f; (int)vlH.size() < N-1; h *= f )
      vlH.push_back( h + vlH.back() );
    vlH.push_back( T );
  }

  //================================================================================
  /*!
   * \brief Create intermediate nodes on VLEdge
   *  \param [inout] vlEdge - VLEdge to divide
   *  \param [in] vlH - thicknesses of layers
   *  \param [inout] mesh - the mesh no fill in
   */
  //================================================================================

  void divideVLEdge( VLEdge*                      vlEdge,
                     const std::vector< double >& vlH,
                     SMESHDS_Mesh*                mesh )
  {
    SMESH_NodeXYZ lastNode = vlEdge->_nodes.back();
    SMESH_NodeXYZ frstNode = vlEdge->_nodes[0];
    gp_XYZ dir = frstNode - lastNode;

    vlEdge->_nodes.resize( vlH.size() + 1 );

    for ( size_t i = 1; i < vlH.size(); ++i )
    {
      double r = vlH[ i-1 ] / vlH.back();
      gp_XYZ p = lastNode + dir * r;
      vlEdge->_nodes[ vlH.size() - i ] = mesh->AddNode( p.X(), p.Y(), p.Z() );
    }
    vlEdge->_nodes.back() = lastNode;
  }

  //================================================================================
  /*!
   * \brief Create a polyhedron from nodes of VLEdge's
   *  \param [in] edgesVec - the VLEdge's
   *  \param [in] vNodes - node buffer
   *  \param [in] editor - editor of offset mesh
   */
  //================================================================================

  bool makePolyhedron( const std::vector< VLEdge*> &         edgesVec,
                       std::vector< const SMDS_MeshNode* > & vNodes,
                       SMESH_MeshEditor&                     editor,
                       SMESH_MeshEditor::ElemFeatures        elemType)
  {
    elemType.SetPoly( true );
    elemType.myPolyhedQuantities.clear();
    elemType.myNodes.clear();

    // add facets of walls
    size_t nbBaseNodes = edgesVec.size();
    for ( size_t iN = 0; iN < nbBaseNodes; ++iN )
    {
      VLEdge* e0 = edgesVec[ iN ];
      VLEdge* e1 = edgesVec[( iN + 1 ) % nbBaseNodes ];
      size_t nbN0 = e0->_nodes.size();
      size_t nbN1 = e1->_nodes.size();
      if ( nbN0 == nbN1 )
      {
        for ( size_t i = 1; i < nbN0; ++i )
        {
          elemType.myNodes.push_back( e1->_nodes[ i - 1 ].Node());
          elemType.myNodes.push_back( e1->_nodes[ i     ].Node());
          elemType.myNodes.push_back( e0->_nodes[ i     ].Node());
          elemType.myNodes.push_back( e0->_nodes[ i - 1 ].Node());
          elemType.myPolyhedQuantities.push_back( 4 );
        }
      }
      else
      {
        for ( size_t i = 0; i < nbN1; ++i )
          elemType.myNodes.push_back( e1->_nodes[ i ].Node() );
        for ( size_t i = 0; i < nbN0; ++i )
          elemType.myNodes.push_back( e0->_nodes[ nbN0 - 1 - i ].Node() );
        elemType.myPolyhedQuantities.push_back( nbN0 + nbN1 );
      }
    }

    // add facets of top
    vNodes.clear();
    for ( size_t iN = 0; iN < nbBaseNodes; ++iN )
    {
      VLEdge* e0 = edgesVec[ iN ];
      elemType.myNodes.push_back( e0->_nodes.back().Node() );
      vNodes.push_back( e0->_nodes[ 0 ].Node());
    }
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    // add facets of bottom
    elemType.myNodes.insert( elemType.myNodes.end(), vNodes.rbegin(), vNodes.rend() );
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    const SMDS_MeshElement* vol = editor.AddElement( elemType.myNodes, elemType );
    vol->setIsMarked( true ); // to add to group

    return vol;
  }

  //================================================================================
  /*!
   * \brief Transform prism nodal connectivity to that of polyhedron
   *  \param [inout] nodes - nodes of prism, return nodes of polyhedron
   *  \param [inout] elemType - return quantities of polyhedron,
   */
  //================================================================================

  void prism2polyhedron( std::vector< const SMDS_MeshNode* > & nodes,
                         SMESH_MeshEditor::ElemFeatures &      elemType )
  {
    elemType.myPolyhedQuantities.clear();
    elemType.myNodes.clear();

    // walls
    size_t nbBaseNodes = nodes.size() / 2;
    for ( size_t i = 0; i < nbBaseNodes; ++i )
    {
      int iNext = ( i + 1 ) % nbBaseNodes;
      elemType.myNodes.push_back( nodes[ iNext ]);
      elemType.myNodes.push_back( nodes[ i ]);
      elemType.myNodes.push_back( nodes[ i + nbBaseNodes ]);
      elemType.myNodes.push_back( nodes[ iNext + nbBaseNodes ]);
      elemType.myPolyhedQuantities.push_back( 4 );
    }

    // base
    elemType.myNodes.insert( elemType.myNodes.end(), nodes.begin(), nodes.begin() + nbBaseNodes );
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    // top
    elemType.myNodes.insert( elemType.myNodes.end(), nodes.rbegin(), nodes.rbegin() + nbBaseNodes );
    elemType.myPolyhedQuantities.push_back( nbBaseNodes );

    nodes.swap( elemType.myNodes );
  }

  //================================================================================
  /*!
   * \brief Create prisms from faces
   *  \param [in] theEOS - shape to treat
   *  \param [inout] theMesh - offset mesh to fill in
   */
  //================================================================================

  bool makePrisms( VLEdgesOnShape & theEOS,
                   SMESH_Mesh*      theMesh,
                   TNode2VLEdge   & theN2E )
  {
    SMESHDS_SubMesh* sm = theMesh->GetMeshDS()->MeshElements( theEOS._offsetShape );
    if ( !sm || sm->NbElements() == 0 )
      return true;

    SMESH_MeshEditor editor( theMesh );
    SMESH_MeshEditor::ElemFeatures volumElem( SMDSAbs_Volume );

    std::vector< const SMDS_MeshNode* > vNodes;
    std::vector< VLEdge*> edgesVec;
    for ( SMDS_ElemIteratorPtr eIt = sm->GetElements(); eIt->more(); )
    {
      const SMDS_MeshElement* face = eIt->next();
      if ( face->GetType() != SMDSAbs_Face )
        continue;

      const int nbNodes = face->NbCornerNodes();
      edgesVec.resize( nbNodes );
      vNodes.resize( nbNodes * 2 );
      int maxNbLayer = 0, minNbLayer = IntegerLast();
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* n = face->GetNode( i );
        if ( !theN2E.IsBound( n ))
          return false;
        edgesVec[ i ] = theN2E( n );
        maxNbLayer = Max( maxNbLayer, edgesVec[i]->_nodes.size() - 1 );
        minNbLayer = Min( minNbLayer, edgesVec[i]->_nodes.size() - 1 );
      }
      if ( maxNbLayer == minNbLayer )
      {
        size_t nbPrism = edgesVec[0]->_nodes.size() - 1;
        for ( size_t iP = 0; iP < nbPrism; ++iP )
        {
          vNodes.resize( nbNodes * 2 );
          for ( int i = 0; i < nbNodes; ++i )
          {
            vNodes[ i           ] = edgesVec[ i ]->_nodes[ iP + 1 ].Node();
            vNodes[ i + nbNodes ] = edgesVec[ i ]->_nodes[ iP ].Node();
          }
          volumElem.SetPoly( nbNodes > 4 );
          if ( volumElem.myIsPoly )
            prism2polyhedron( vNodes, volumElem );

          if ( const SMDS_MeshElement* vol = editor.AddElement( vNodes, volumElem ))
            vol->setIsMarked( true ); // to add to group          
        }
      }
      else // at inlet/outlet
        makePolyhedron( edgesVec, vNodes, editor, volumElem );

      editor.ClearLastCreated();

      // move the face to the top of prisms, on mesh boundary
      //theMesh->GetMeshDS()->ChangeElementNodes( face, fNodes.data(), nbNodes );
    }
    return true;
  }

  //================================================================================
  /*!
   * \brief Create faces from edges
   *  \param [inout] theEOS - shape to treat
   *  \param [inout] theMesh - offset mesh to fill in
   *  \param [inout] theN2E - map of node to VLEdge
   *  \param [inout] theFaceID - ID of WOVL FACE for new faces to set on
   *  \param [in]    isMainShape2D - used to identify the geometry where the new elements are included
   *  \return bool - ok
   */
  //================================================================================

  bool makeFaces( VLEdgesOnShape & theEOS,
                  SMESH_Mesh*      theMesh,
                  TNode2VLEdge   & theN2E,
                  const TGeomID    theFaceID,
                  bool isMainShape2D = false )
  {
    SMESHDS_SubMesh* sm = theMesh->GetMeshDS()->MeshElements( theEOS._offsetShape );
    if ( !sm || sm->NbElements() == 0 )
      return true;

    SMESH_MeshEditor editor( theMesh );
    SMESH_MeshEditor::ElemFeatures elem2D( SMDSAbs_Face );

    std::vector< const SMDS_MeshNode* > fNodes( 4 );
    std::vector<const SMDS_MeshElement *> foundVolum;
    std::vector< VLEdge*> edgesVec;
    TIDSortedElemSet  refSetFace;
    
    // Check orientation of face and re
    gp_XYZ refNormalVector(0.0,0.0,0.0);
    if ( isMainShape2D )
    {
      SMESHDS_Mesh* offsetMDS = theMesh->GetMeshDS();
      for ( SMDS_ElemIteratorPtr eIt = offsetMDS->elementsIterator(); eIt->more(); )
      {
        const SMDS_MeshElement* refFace = eIt->next(); // define the ref
        if ( refFace->GetType() == SMDSAbs_Face )
        {
          SMESH_MeshAlgos::FaceNormal( refFace, refNormalVector, /*normalized=*/true );
          break;
        }
      }
      if ( refNormalVector.X() == 0.0 && refNormalVector.Y() == 0.0 && refNormalVector.Z() == 0.0 )
        throw SALOME_Exception("No 2D element found in the mesh!\n");

    }
    
    for ( SMDS_ElemIteratorPtr eIt = sm->GetElements(); eIt->more(); )
    {
      const SMDS_MeshElement* edge = eIt->next();
      if ( edge->GetType() != SMDSAbs_Edge )
        continue;

      const int nbNodes = 2; //edge->NbCornerNodes();
      edgesVec.resize( nbNodes );
      fNodes.resize( nbNodes * 2 );
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* n = edge->GetNode( i );
        if ( !theN2E.IsBound( n ))
          return false;
        edgesVec[ i ] = theN2E( n );
      }
      size_t nbFaces = edgesVec[0]->_nodes.size() - 1;

      for ( size_t iF = 0; iF < nbFaces; ++iF )
      {
        fNodes[ 0 ] = edgesVec[ 0 ]->_nodes[ iF ].Node();
        fNodes[ 1 ] = edgesVec[ 1 ]->_nodes[ iF ].Node();
        fNodes[ 2 ] = edgesVec[ 1 ]->_nodes[ iF + 1 ].Node();
        fNodes[ 3 ] = edgesVec[ 0 ]->_nodes[ iF + 1 ].Node();

        if ( const SMDS_MeshElement* face = editor.AddElement( fNodes, elem2D ))
        {
          theMesh->GetMeshDS()->SetMeshElementOnShape( face, theFaceID );

          if ( SMDS_Mesh::GetElementsByNodes( fNodes, foundVolum, SMDSAbs_Volume ))
          {
            TIDSortedElemSet faces = { face }, volumes = { foundVolum[0] };
            editor.Reorient2DBy3D( faces, volumes, /*outside=*/true );
          }
          else if ( isMainShape2D )
          {
            gp_XYZ elementNormal;
            SMESH_MeshAlgos::FaceNormal( face, elementNormal, /*normalized=*/true );
            if ( elementNormal * refNormalVector < 0.0 /* diff orientation from the ref element */)
              editor.Reorient( face );
          }
        }
      }
      editor.ClearLastCreated();

      // move the face to the top of prisms, on mesh boundary
      //theMesh->GetMeshDS()->ChangeElementNodes( face, fNodes.data(), nbNodes );
    }    
    return true;
  }

  //================================================================================
  /*!
   * \brief Append the offset mesh to the initial one
   */
  //================================================================================

  void copyMesh( SMESH_Mesh* theFromMesh,
                 SMESH_Mesh* theToMesh,
                 int         theSolidID)
  {
    SMESHDS_Mesh* offsetMDS = theFromMesh->GetMeshDS();
    SMESHDS_Mesh*   initMDS = theToMesh->GetMeshDS();

    const smIdType nShift = initMDS->NbNodes();

    for ( SMDS_NodeIteratorPtr nIt = offsetMDS->nodesIterator(); nIt->more(); )
    {
      SMESH_NodeXYZ pOff = nIt->next();
      const SMDS_MeshNode* n = initMDS->AddNodeWithID( pOff.X(), pOff.Y(), pOff.Z(),
                                                       nShift + pOff.Node()->GetID() );
      initMDS->SetNodeInVolume( n, theSolidID );
    }

    SMESH_MeshEditor editor( theToMesh );
    SMESH_MeshEditor::ElemFeatures elemType;
    std::vector<smIdType> nIniIDs;

    for ( SMDS_ElemIteratorPtr eIt = offsetMDS->elementsIterator(); eIt->more(); )
    {
      const SMDS_MeshElement* offElem = eIt->next();
      const int nbNodes = offElem->NbNodes();
      nIniIDs.resize( nbNodes );
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* offNode = offElem->GetNode( i );
        nIniIDs[ i ] = offNode->GetID() + nShift;
      }
      elemType.Init( offElem, /*basicOnly=*/false );
      const SMDS_MeshElement* iniElem = editor.AddElement( nIniIDs, elemType );
      initMDS->SetMeshElementOnShape( iniElem, theSolidID );
      iniElem->setIsMarked( offElem->isMarked() );
      editor.ClearLastCreated();
    }
    return;
  }

  //================================================================================
  /*!
   * \brief set elements of layers boundary to sub-meshes
   *  \param [in] theEOS - vlEdges of a sub-shape
   *  \param [inout] theMesh - the mesh
   *  \param [in] theN2E - map of node to vlEdge
   *  \param [in] theNShift - nb of nodes in the mesh before adding the offset mesh
   */
  //================================================================================

  void setBnd2Sub( VLEdgesOnShape &     theEOS,
                   SMESH_Mesh*          theInitMesh,
                   SMESH_Mesh*          theOffsMesh,
                   const TNode2VLEdge & theN2E,
                   const smIdType       theNShift,
                   TIDSortedNodeSet&    theNodesToCheckCoinc )
  {
    SMESHDS_Mesh* offsetMDS = theOffsMesh->GetMeshDS();
    SMESHDS_Mesh*   initMDS = theInitMesh->GetMeshDS();

    SMESHDS_SubMesh* sm = offsetMDS->MeshElements( theEOS._offsetShape );
    if ( !sm || ( sm->NbElements() + sm->NbNodes() == 0 ))
      return;

    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); )
    {
      const SMDS_MeshNode* offNode = nIt->next();
      VLEdge* vlEd = theN2E( offNode );
      const SMDS_MeshNode* nOffBnd = vlEd->_nodes.back().Node();
      const SMDS_MeshNode* nIniBnd = initMDS->FindNode( nOffBnd->GetID() + theNShift );
      initMDS->UnSetNodeOnShape( nIniBnd );

      switch( theEOS._initShape.ShapeType() ) {
      case TopAbs_FACE:   initMDS->SetNodeOnFace( nIniBnd, TopoDS::Face( theEOS._initShape ),
                                                  vlEd->_uv[0], vlEd->_uv[1] );
        break;
      case TopAbs_EDGE:   initMDS->SetNodeOnEdge( nIniBnd, TopoDS::Edge( theEOS._initShape ),
                                                  vlEd->_uv[0]);
        break;
      case TopAbs_VERTEX: initMDS->SetNodeOnVertex( nIniBnd, TopoDS::Vertex( theEOS._initShape ));
        break;
      default:;
      }
    }

    std::vector< const SMDS_MeshNode* > iniNodes, iniNodesBnd;
    std::vector< VLEdge*> edgesVec;
    for ( SMDS_ElemIteratorPtr eIt = sm->GetElements(); eIt->more(); )
    {
      const SMDS_MeshElement* offElem = eIt->next();
      if ( offElem->GetType() != SMDSAbs_Face &&
           offElem->GetType() != SMDSAbs_Edge )
        continue;

      const int nbNodes = offElem->NbCornerNodes();
      iniNodes.resize( nbNodes );
      iniNodesBnd.resize( nbNodes );
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* nOff = offElem->GetNode( i );
        const SMDS_MeshNode* nIni = initMDS->FindNode( nOff->GetID() + theNShift );
        iniNodes[ i ] = nIni;
        if ( !theN2E.IsBound( nOff ))
          break;
        VLEdge* vlEd = theN2E( nOff );
        const SMDS_MeshNode* nOffBnd = vlEd->_nodes.back().Node();
        const SMDS_MeshNode* nIniBnd = initMDS->FindNode( nOffBnd->GetID() + theNShift );
        iniNodesBnd[ i ] = nIniBnd;
      }
      if ( const SMDS_MeshElement* iniElem = initMDS->FindElement( iniNodes ))
      {
        initMDS->UnSetElementOnShape( iniElem );
        initMDS->SetMeshElementOnShape( iniElem, theEOS._initShape );

        // move the face to the top of prisms, on init mesh boundary
        initMDS->ChangeElementNodes( iniElem, iniNodesBnd.data(), nbNodes );

        if ( theEOS._toCheckCoinc )
          theNodesToCheckCoinc.insert( iniNodesBnd.begin(), iniNodesBnd.end() );
      }
    }
    return;
  }

  //================================================================================
  /*!
   * \brief set elements of WOVL face to sub-meshes
   *  \param [in] theEOS - vlEdges of a sub-shape
   *  \param [inout] theMesh - the mesh
   *  \param [in] theN2E - map of node to vlEdge
   *  \param [in] theNShift - nb of nodes in the mesh before adding the offset mesh
   */
  //================================================================================

  void setBnd2FVWL( VLEdgesOnShape &     theEOS,
                    SMESH_Mesh*          theInitMesh,
                    SMESH_Mesh*          theOffsMesh,
                    //const TNode2VLEdge & theN2E,
                    const smIdType       theNShift )
  {
    SMESHDS_Mesh* offsetMDS = theOffsMesh->GetMeshDS();
    SMESHDS_Mesh*   initMDS = theInitMesh->GetMeshDS();

    SMESHDS_SubMesh* sm = offsetMDS->MeshElements( theEOS._offsetShape );
    if ( !sm || ( sm->NbElements() + sm->NbNodes() == 0 ))
      return;

    for ( SMDS_NodeIteratorPtr nIt = sm->GetNodes(); nIt->more(); )
    {
      const SMDS_MeshNode* offNode = nIt->next();
      const SMDS_MeshNode* iniNode = initMDS->FindNode( offNode->GetID() + theNShift );
      initMDS->UnSetNodeOnShape( iniNode );

      switch( theEOS._initShape.ShapeType() ) {
      case TopAbs_FACE:   initMDS->SetNodeOnFace( iniNode, TopoDS::Face( theEOS._initShape ));
        break;
      case TopAbs_EDGE:   initMDS->SetNodeOnEdge( iniNode, TopoDS::Edge( theEOS._initShape ));
        break;
      case TopAbs_VERTEX: initMDS->SetNodeOnVertex( iniNode, TopoDS::Vertex( theEOS._initShape ));
        break;
      default:;
      }
    }

    std::vector< const SMDS_MeshNode* > iniNodes;
    for ( SMDS_ElemIteratorPtr eIt = sm->GetElements(); eIt->more(); )
    {
      const SMDS_MeshElement* offElem = eIt->next();
      // if ( offElem->GetType() != SMDSAbs_Face )
      //   continue;

      int nbNodes = offElem->NbCornerNodes();
      iniNodes.resize( nbNodes );
      for ( int i = 0; i < nbNodes; ++i )
      {
        const SMDS_MeshNode* nOff = offElem->GetNode( i );
        const SMDS_MeshNode* nIni = initMDS->FindNode( nOff->GetID() + theNShift );
        iniNodes[ i ] = nIni;
      }
      if ( const SMDS_MeshElement* iniElem = initMDS->FindElement( iniNodes ))
      {
        initMDS->UnSetElementOnShape( iniElem );
        initMDS->SetMeshElementOnShape( iniElem, theEOS._initShapeID );

        for ( const SMDS_MeshNode* n : iniNodes )
          if ( n->GetPosition()->GetDim() == 3 )
          {
            initMDS->UnSetNodeOnShape( n );
            if ( theEOS._initShape.ShapeType() == TopAbs_FACE )
              initMDS->SetNodeOnFace( n, theEOS._initShapeID );
            else
              initMDS->SetNodeOnEdge( n, theEOS._initShapeID );
          }
      }
    }
    return;
  }
} // namespace


//================================================================================
/*!
 * \brief Create a temporary mesh used to hold elements of the offset shape
 */
//================================================================================

SMESH_Mesh* StdMeshers_Cartesian_VL::ViscousBuilder::MakeOffsetMesh()
{
  return _offsetMesh;
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

StdMeshers_Cartesian_VL::ViscousBuilder::ViscousBuilder( const StdMeshers_ViscousLayers* hyp,
                                                         const SMESH_Mesh &              mainMesh,
                                                         const TopoDS_Shape &            mainShape)
  : _hyp( hyp ), _offsetMesh( new TmpMesh )
{
  // find shapes with viscous layers
  TopTools_IndexedMapOfShape faces;
  TopExp::MapShapes( mainShape, TopAbs_FACE, faces );
  const SMESHDS_Mesh* iniMDS = mainMesh.GetMeshDS();

  if ( hyp->IsToIgnoreShapes() )
  {
    for ( int i = 1; i <= faces.Size(); ++i )
      _shapesWVL.insert( iniMDS->ShapeToIndex( faces( i )));

    for ( TGeomID& sID : hyp->GetBndShapes() )
      _shapesWVL.erase( sID );
  }
  else
  {
    for ( TGeomID& sID : hyp->GetBndShapes() )
      _shapesWVL.insert( sID );
  }

  for ( TGeomID fID : _shapesWVL )
  {
    const TopoDS_Shape & face = iniMDS->IndexToShape( fID );
    for ( TopExp_Explorer exp( face, TopAbs_EDGE ); exp.More(); exp.Next() )
      _shapesWVL.insert( iniMDS->ShapeToIndex( exp.Current() ));
    for ( TopExp_Explorer exp( face, TopAbs_VERTEX ); exp.More(); exp.Next() )
      _shapesWVL.insert( iniMDS->ShapeToIndex( exp.Current() ));
  }

  // find EDGE and adjacent FACEs w/o VL
  for ( int i = 1; i <= faces.Size(); ++i )
  {
    const TopoDS_Shape& face = faces( i );
    TGeomID fID = iniMDS->ShapeToIndex( face );
    bool isMainShape2D = (mainShape.ShapeType() == TopAbs_FACE) ? true : false;

    if ( _shapesWVL.count( fID ) && !isMainShape2D )
      continue;

    for ( TopExp_Explorer exp( face, TopAbs_EDGE ); exp.More(); exp.Next() )
      _edge2facesWOVL[ iniMDS->ShapeToIndex( exp.Current() )].push_back( fID );
  }
  
  // When 2D meshing Need to add edges where segments need to be added due to geometry shrink
  return;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshers_Cartesian_VL::ViscousBuilder::~ViscousBuilder()
{
  delete _offsetMesh; //_offsetMesh = 0;
}

//================================================================================
/*!
 * \brief Create an offset solid from a given one
 *  \param [in] theShape - input shape can be a solid, solidcompound or a compound with solids
 *  \param [in] theMesh - main mesh
 *  \param [out] theError - error description
 *  \return TopoDS_Shape - result offset shape of the same type as the received shape
 */
//================================================================================

TopoDS_Shape StdMeshers_Cartesian_VL::ViscousBuilder::MakeOffsetSolid(const TopoDS_Shape & theShape,
                                                                        SMESH_Mesh &         theMesh,
                                                                        std::string &        theError )
{
  double offset = -_hyp->GetTotalThickness();
  double    tol = Precision::Confusion();
  TopAbs_ShapeEnum typeOfShape = theShape.ShapeType();

  TopTools_IndexedMapOfShape shapeList;
  TopExp::MapShapes( theShape, TopAbs_SOLID, shapeList );
  std::vector<TopoDS_Shape> shrinkBodies;

  for ( int i = 1; i <= shapeList.Size(); ++i )
  {
    auto solid = shapeList( i );
      // If Shape is solid call direct 
    BRepOffset_MakeOffset * makeOffset = new BRepOffset_MakeOffset();
    makeOffset->Initialize( solid, offset, tol, BRepOffset_Skin, /*Intersection=*/false,
                          /*selfInter=*/false, GeomAbs_Intersection);

    // exclude inlet FACEs
    SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
    for ( TopExp_Explorer fEx( theShape, TopAbs_FACE ); fEx.More(); fEx.Next() )
    {
      TGeomID fID = meshDS->ShapeToIndex( fEx.Current() );
      if ( !_shapesWVL.count( fID ))
        makeOffset->SetOffsetOnFace( TopoDS::Face( fEx.Current()), 0 );
    }

    makeOffset->MakeOffsetShape();
    if ( makeOffset->IsDone() )
    {
      shrinkBodies.push_back( makeOffset->Shape() );
      _makeOffsetCollection.push_back( makeOffset );
    }
    else
    {
      switch ( makeOffset->Error() )
      {
      case BRepOffset_NoError:
        theError = "OK. Offset performed successfully.";break;
      case BRepOffset_BadNormalsOnGeometry:
        theError = "Degenerated normal on input data.";break;
      case BRepOffset_C0Geometry:
        theError = "C0 continuity of input data.";break;
      case BRepOffset_NullOffset:
        theError = "Null offset of all faces.";break;
      case BRepOffset_NotConnectedShell:
        theError = "Incorrect set of faces to remove, the remaining shell is not connected.";break;
      case BRepOffset_CannotTrimEdges:
        theError = "Can not trim edges.";break;
      case BRepOffset_CannotFuseVertices:
        theError = "Can not fuse vertices.";break;
      case BRepOffset_CannotExtentEdge:
        theError = "Can not extent edge.";break;
      default:
        theError = "operation not done.";
      }
      theError = "BRepOffset_MakeOffset error: " + theError;

      return TopoDS_Shape();
    }    
  }
  
  if ( typeOfShape == TopAbs_COMPOUND || typeOfShape == TopAbs_COMPSOLID )
  {
    _solidCompound.SetGlue( BOPAlgo_GlueFull );
    _solidCompound.SetToFillHistory( true );
    for ( auto solid : shrinkBodies )
      _solidCompound.AddArgument( solid );

    _solidCompound.Perform();
    return _solidCompound.Shape();
  }
  else
    return shrinkBodies[ 0 ]; // return one solid

}

//================================================================================
/*!
 * \brief Create an offset shape from a given one
 *  \param [in] theShape - input shape
 *  \param [in] theMesh - main mesh
 *  \param [out] theError - error description
 *  \return TopoDS_Shape - result offset shape
 */
//================================================================================

TopoDS_Shape
StdMeshers_Cartesian_VL::ViscousBuilder::MakeOffsetShape(const TopoDS_Shape & theShape,
                                                         SMESH_Mesh &         theMesh,
                                                         std::string &        theError )
{
  double offset                 = -_hyp->GetTotalThickness();
  double    tol                 = Precision::Confusion();
  TopAbs_ShapeEnum typeOfShape  = theShape.ShapeType();

  // Switch here for the treatment of faces
  if ( typeOfShape == TopAbs_FACE )
  {
    TopoDS_Face face = TopoDS::Face( theShape );
    GProp_GProps gprops;
    BRepGProp::SurfaceProperties(face, gprops); // Stores results in gprops
    double faceArea = gprops.Mass();

    _makeFaceOffset = BRepOffsetAPI_MakeOffset( face, GeomAbs_Intersection );
    _makeFaceOffset.Perform( offset );
    TopoDS_Wire wireFrame   = TopoDS::Wire( _makeFaceOffset.Shape() );
    TopoDS_Face shrinkFace  = TopoDS::Face( BRepBuilderAPI_MakeFace( wireFrame, false ) );
    BRepGProp::SurfaceProperties(shrinkFace, gprops); // Stores results in gprops
    double sArea = gprops.Mass();

    if ( sArea > faceArea /*recompute the shrink face because offset was done in the contrary direction as expected*/)
    {
      _makeFaceOffset.Perform( -offset );
      wireFrame  = TopoDS::Wire( _makeFaceOffset.Shape() );
      shrinkFace = TopoDS::Face( BRepBuilderAPI_MakeFace( wireFrame, false ) );
    }
    _offsetShape = shrinkFace;
    return _offsetShape;
  }
  else
  {
    _offsetShape = MakeOffsetSolid( theShape, theMesh, theError );
    return _offsetShape;
  }  
}

//================================================================================
/*!
 * \brief Return the list of sub-shape of the same type of the offset shape generated from a given initial sub-shape
 */
//================================================================================

void StdMeshers_Cartesian_VL::ViscousBuilder::getOffsetSubShape( const TopoDS_Shape& S, std::vector<TopoDS_Shape>& subShapeList )
{  
  for( auto offset : _makeOffsetCollection )
  {
    const TopTools_ListOfShape& newShapes = offset->Generated( S );
    if ( newShapes.Size() == 0 )
      continue; // keep searching

    for ( const TopoDS_Shape& ns : newShapes )
    {
      if ( ns.ShapeType() == S.ShapeType() )
      {
        if  ( _solidCompound.Arguments().Size() == 0 /* only one solid shrank*/ )
        {
          subShapeList.push_back( ns );
        }
        else
        {
          // In boolean operations the shapes are modified or deleted
          const TopTools_ListOfShape& newGlueShapes = _solidCompound.Modified( ns );
          for ( TopoDS_Shape& ngs : newGlueShapes )
            if ( ngs.ShapeType() == ns.ShapeType() /*&& !ngs.Checked()*/  )
              subShapeList.push_back( ngs );
          
          if ( newGlueShapes.Size() == 0 && !_solidCompound.IsDeleted( ns ) )
            subShapeList.push_back( ns );
        }
      }            
    }      
  }     

  // check for _makeFaceOffset in face shrink
  if ( _makeOffsetCollection.size()  == 0 )
  {
    const TopTools_ListOfShape& newShapes = _makeFaceOffset.Generated( S );
    for ( const TopoDS_Shape& ns : newShapes )
    {
      if ( ns.ShapeType() == S.ShapeType() )
        return subShapeList.push_back( ns );
    }
  }
}

bool StdMeshers_Cartesian_VL::ViscousBuilder::CheckGeometryMaps( SMESH_Mesh &        offsetMesh,
                                                                  const TopoDS_Shape & theShape  )
{
  SMESHDS_Mesh* offsetMDS       = offsetMesh.GetMeshDS();
  TopoDS_Shape shrinkGeomToMesh = offsetMDS->ShapeToMesh();  

  TopTools_IndexedMapOfShape shrinkGeomMap;
  TopExp::MapShapes( shrinkGeomToMesh, shrinkGeomMap );
  TopTools_IndexedMapOfShape offsetGeomMap;
  TopExp::MapShapes( _offsetShape, offsetGeomMap );

  // loop on sub-shapes to project nodes from offset boundary to initial boundary
  TopAbs_ShapeEnum types[3] = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE };
  for ( TopAbs_ShapeEnum shType : types )
  {
    TopTools_IndexedMapOfShape shapes;
    TopExp::MapShapes( theShape, shType, shapes );
    for ( int i = 1; i <= shapes.Size(); ++i )
    {
      // For each type of geometry check the existence of one or more equivalents
      std::vector<TopoDS_Shape> listOfShapes;
      getOffsetSubShape( shapes(i), listOfShapes );
      if ( listOfShapes.size() == 0 ) return false;
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Create prismatic mesh between _offsetShape and theShape
 *  \remark Build the viscous layer from the iteration of shrink geometry
 *  \param [out] theMesh - mesh to fill in
 *  \param [in] theShape - initial shape
 *  \return bool - is Ok
 */
//================================================================================

bool StdMeshers_Cartesian_VL::ViscousBuilder::MakeViscousLayers( SMESH_Mesh &        offsetMesh,
                                                                 SMESH_Mesh &         theMesh,
                                                                 const TopoDS_Shape & theShape )
{
  SMESHDS_Mesh* offsetMDS       = offsetMesh.GetMeshDS();
  SMESHDS_Mesh*   initMDS       = theMesh.GetMeshDS();
  TopoDS_Shape shrinkGeomToMesh = offsetMDS->ShapeToMesh();  
  bool isMainShape2D = (theShape.ShapeType() == TopAbs_FACE) ? true : false;

  // Validate map of shrink+joint geometry elements
  if ( !CheckGeometryMaps(offsetMesh, theShape ) && !isMainShape2D )
    throw SALOME_Exception("The shrink geometry does not match or respect the original topology.The viscous layer can't be build");

  
  initMDS->ClearMesh(); // avoid mesh superposition on multiple calls of addLayers
  offsetMDS->SetAllCellsNotMarked();

  TopTools_IndexedMapOfShape shrinkGeomMap;
  TopExp::MapShapes( shrinkGeomToMesh, shrinkGeomMap );

  TopTools_IndexedMapOfShape offsetGeomMap;
  TopExp::MapShapes( _offsetShape, offsetGeomMap );

  // Compute heights of viscous layers
  std::vector< double > vlH;
  computeVLHeight( _hyp, vlH );
  
  std::vector< VLEdgesOnShape > edgesOnShape;
  edgesOnShape.reserve( offsetMDS->MaxShapeIndex() + 1 );
  TNode2VLEdge n2e;

  // loop on sub-shapes to project nodes from offset boundary to initial boundary
  TopAbs_ShapeEnum types[3] = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE };
  for ( TopAbs_ShapeEnum shType : types )
  {
    TopTools_IndexedMapOfShape shapes;
    TopExp::MapShapes( theShape, shType, shapes );
    for ( int i = 1; i <= shapes.Size(); ++i )
    {
      edgesOnShape.resize( edgesOnShape.size() + 1 );
      VLEdgesOnShape& EOS = edgesOnShape.back();
      std::vector<TopoDS_Shape> listOfShapes;
      EOS._initShape    = shapes( i );
      
      // Get a list of subShapes of the same type generated from the same face
      // It is the case with split objects.
      getOffsetSubShape( EOS._initShape, listOfShapes );
      for ( TopoDS_Shape& shrinkShape : listOfShapes )
      {
        int shapeId       = offsetGeomMap.FindIndex( shrinkShape );        
        EOS._offsetShape  = shrinkGeomMap.FindKey( shapeId );     
        EOS._initShapeID  = initMDS->ShapeToIndex( EOS._initShape );
        EOS._hasVL        = _shapesWVL.count( EOS._initShapeID );

        EOS._toCheckCoinc = false;
        if ( !EOS._hasVL )
          continue;

        // project boundary nodes of offset mesh to boundary of init mesh
        // (new nodes are created in the offset mesh)
        switch( EOS._offsetShape.ShapeType() ) {
        case TopAbs_VERTEX:
        {
          EOS._edges.resize( 1 );
          EOS._edges[0]._nodes.resize( 2 );
          EOS._edges[0]._nodes[0] = SMESH_Algo::VertexNode( TopoDS::Vertex( EOS._offsetShape ),
                                                            offsetMDS );
          gp_Pnt offP = BRep_Tool::Pnt( TopoDS::Vertex( EOS._initShape ));
          EOS._edges[0]._nodes[1] = offsetMDS->AddNode( offP.X(), offP.Y(), offP.Z() );
          //EOS._edges[0]._length   = offP.Distance( EOS._edges[0]._nodes[0] );
          n2e.Bind( EOS._edges[0]._nodes[0].Node(), & EOS._edges[0] );
          break;
        }
        case TopAbs_EDGE:
        {
          projectToEdge( EOS, offsetMDS, n2e, isMainShape2D /* add vertex from edges*/ );
          break;
        }
        case TopAbs_FACE:
        {
          projectToFace( EOS, offsetMDS, n2e );
          break;
        }
        default:;
        }

        // create nodes of layers
        if ( _hyp->GetNumberLayers() > 1 )
        {
          //if ( _shapesWVL.count( EOS._initShapeID ))
          for ( size_t i = 0; i < EOS._edges.size(); ++i )
          {
            divideVLEdge( &EOS._edges[ i ], vlH, offsetMDS );
          }
        }
      } // loop on generated shrink shape
    }//loop on original shape
  } // loop on shape types
  
  // create prisms
  bool prismsOk = true;
  for ( size_t i = 0; i < edgesOnShape.size(); ++i )
  {
    VLEdgesOnShape& EOS = edgesOnShape[ i ];
    if ( EOS._initShape.ShapeType() == TopAbs_FACE && EOS._hasVL )
    {
      if ( !makePrisms( EOS, &offsetMesh, n2e ))
        prismsOk = false;
    }
  }

  if ( prismsOk )
  {
    // create faces on FACEs WOVL
    for ( size_t i = 0; i < edgesOnShape.size(); ++i )
    {
      VLEdgesOnShape& EOS = edgesOnShape[ i ];
      if ( EOS._initShape.ShapeType() == TopAbs_EDGE && EOS._hasVL )
      {
        auto e2f = _edge2facesWOVL.find( EOS._initShapeID );
        if ( e2f != _edge2facesWOVL.end() && !e2f->second.empty() )
        {
          TopoDS_Shape  f = initMDS->IndexToShape( e2f->second[0] );
          std::vector<TopoDS_Shape> listOfShapes;
          getOffsetSubShape( f, listOfShapes );
          for( TopoDS_Shape& subShape : listOfShapes )
          {
            int shapeId   = offsetGeomMap.FindIndex( subShape );
            TopoDS_Shape f2 = shrinkGeomMap.FindKey( shapeId );     
            makeFaces( EOS, & offsetMesh, n2e, offsetMDS->ShapeToIndex( f2 ) );
          }        
        }
      }
    }
  }

  if ( isMainShape2D )
  {
     // create faces on FACEs of the inflate viscous layer in 2D faces
    for ( size_t i = 0; i < edgesOnShape.size(); ++i )
    {
      VLEdgesOnShape& EOS = edgesOnShape[ i ];
      if ( EOS._initShape.ShapeType() == TopAbs_EDGE && EOS._hasVL /* iterate in market edges with viscous layer*/)
      {
        int shapeId = offsetMDS->ShapeToIndex( shrinkGeomToMesh );
        makeFaces( EOS, & offsetMesh, n2e, shapeId, isMainShape2D ); // pass face Id of shrink geometry
      }
    }
  }

   // copy offset mesh to the main one
  initMDS->Modified();
  initMDS->CompactMesh();
  smIdType nShift = initMDS->NbNodes();
  TGeomID solidID = initMDS->ShapeToIndex( theShape );
  copyMesh( & offsetMesh, & theMesh, solidID );

  if ( !prismsOk )
  {
    if ( SMESH_subMesh * sm = theMesh.GetSubMesh( theShape ))
    {
      sm->GetComputeError() =
        SMESH_ComputeError::New( COMPERR_ALGO_FAILED,
                                 "Viscous layers construction error: bad mesh on offset geometry" );
    }
    return prismsOk;
  }

  // set elements of layers boundary to sub-meshes
  TIDSortedNodeSet nodesToCheckCoinc;
  for ( size_t i = 0; i < edgesOnShape.size(); ++i )
  {
    VLEdgesOnShape& EOS = edgesOnShape[ i ];
    if ( EOS._hasVL )
      setBnd2Sub( EOS, &theMesh, &offsetMesh, n2e, nShift, nodesToCheckCoinc );
    else
      setBnd2FVWL( EOS, &theMesh, &offsetMesh, nShift );
  }

  // merge coincident nodes
  SMESH_MeshEditor editor( &theMesh );
  SMESH_MeshEditor::TListOfListOfNodes nodesToMerge;
  editor.FindCoincidentNodes( nodesToCheckCoinc, vlH[0]/50., nodesToMerge, false );
  editor.MergeNodes( nodesToMerge );

  // create a group
  if ( !_hyp->GetGroupName().empty() )
  {
    SMDS_MeshGroup* group = _hyp->CreateGroup( _hyp->GetGroupName(), theMesh, SMDSAbs_Volume );

    for ( SMDS_ElemIteratorPtr it = initMDS->elementsIterator(); it->more(); )
    {
      const SMDS_MeshElement* el = it->next();
      if ( el->isMarked() )
        group->Add( el );
    }
  }


  return prismsOk;
}