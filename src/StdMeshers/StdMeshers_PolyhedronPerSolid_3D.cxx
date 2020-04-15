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

// File      : StdMeshers_PolyhedronPerSolid_3D.cxx
// Module    : SMESH
// Created   : Fri Oct 20 11:37:07 2006
// Author    : Edward AGAPOV (eap)
//
#include "StdMeshers_PolyhedronPerSolid_3D.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_ProxyMesh.hxx"
#include "SMESH_subMesh.hxx"
#include "StdMeshers_PolygonPerFace_2D.hxx"
#include "StdMeshers_Regular_1D.hxx"
#include "StdMeshers_ViscousLayers.hxx"

#include <TopExp_Explorer.hxx>

#include <vector>
#include <set>

namespace
{
  struct _EdgeMesher : public StdMeshers_Regular_1D
  {
    _EdgeMesher( int hypId, SMESH_Gen* gen )
      : StdMeshers_Regular_1D( hypId, gen )
    {
      _hypType = NB_SEGMENTS;
      _ivalue[ NB_SEGMENTS_IND ] = 1;
    }
  };

  //=======================================================================
  //function : addHexa
  //purpose  :
  //=======================================================================

  const SMDS_MeshElement* addHexa( std::vector< const SMDS_MeshElement* >& faces,
                                   const std::vector< int > &              quantities,
                                   SMESH_MesherHelper &                    helper )
  {
    const SMDS_MeshElement* newHexa = 0;

    // check nb of nodes in faces
    for ( size_t i = 0; i < quantities.size(); ++i )
      if ( quantities[ i ] != 4 )
        return newHexa;

    // look for a top face
    const SMDS_MeshElement* topFace = 0;
    const SMDS_MeshElement* botFace = faces[0];
    std::vector< const SMDS_MeshNode* > nodes( 16 ); // last 8 is a working buffer
    nodes.assign( botFace->begin_nodes(), botFace->end_nodes() );
    for ( size_t iF = 1; iF < faces.size() &&  !topFace; ++iF )
    {
      bool hasCommonNode = false;
      for ( int iN = 0; iN < quantities[ 0 ] &&  !hasCommonNode; ++iN )
        hasCommonNode = ( faces[ iF ]->GetNodeIndex( nodes[ iN ]) >= 0 );

      if ( !hasCommonNode )
        topFace = faces[ iF ];
    }

    nodes.resize( 8 ); // set top nodes after hexa nodes - [8-11]
    nodes.insert( nodes.end(), topFace->begin_nodes(), topFace->end_nodes() );
    nodes.resize( 12 );
    nodes.insert( nodes.end(), nodes.begin() + 8, nodes.begin() + 12 );

    // find corresponding nodes of top and bottom by finding a side face including 2 node of each
    SMESHDS_Mesh* mesh = helper.GetMeshDS();
    const SMDS_MeshElement* sideFace = 0;
    size_t i;
    for ( i = 8; i < nodes.size()-1 &&  !sideFace; ++i )
    {
      sideFace = mesh->FindFace( nodes[0], nodes[1], nodes[ i ], nodes[ i + 1 ]);
    }
    if ( !sideFace )
      return newHexa;

    --i; // restore after ++i in the loop
    bool botOriRight = SMESH_MeshAlgos::IsRightOrder( sideFace, nodes[ 0 ], nodes[ 1 ] );
    bool topOriRight = SMESH_MeshAlgos::IsRightOrder( sideFace, nodes[ i ], nodes[ i + 1 ] );
    if ( botOriRight == topOriRight )
    {
      nodes[ 4 ] = nodes[ i + 1 ];
      nodes[ 5 ] = nodes[ i + 0 ];
      nodes[ 6 ] = nodes[ i + 3 ];
      nodes[ 7 ] = nodes[ i + 2 ];
    }
    else
    {
      nodes[ 4 ] = nodes[ i + 0 ];
      nodes[ 5 ] = nodes[ i + 1 ];
      nodes[ 6 ] = nodes[ i + 2 ];
      nodes[ 7 ] = nodes[ i + 3 ];
    }

    newHexa = helper.AddVolume( nodes[ 0 ], nodes[ 1 ], nodes[ 2 ], nodes[ 3 ],
                                nodes[ 4 ], nodes[ 5 ], nodes[ 6 ], nodes[ 7 ]);

    return newHexa;
  }

  //=======================================================================
  //function : addTetra
  //purpose  :
  //=======================================================================

  const SMDS_MeshElement* addTetra( std::vector< const SMDS_MeshElement* >& faces,
                                    const std::vector< int > &              quantities,
                                    SMESH_MesherHelper &                    helper )
  {
    const SMDS_MeshElement* newTetra = 0;

    // check nb of nodes in faces
    for ( size_t i = 0; i < quantities.size(); ++i )
      if ( quantities[ i ] != 3 )
        return newTetra;

    const SMDS_MeshElement* botFace = faces[0];

    std::vector< const SMDS_MeshNode* > nodes( 6 );
    nodes.assign( botFace->begin_nodes(), botFace->end_nodes() );
    nodes.resize( 3 );

    const SMDS_MeshNode* topNode = 0;
    for ( size_t i = 0; i < 3 &&  !topNode; ++i )
    {
      topNode = faces[ 1 ]->GetNode( i );
      if ( botFace->GetNodeIndex( topNode ) >= 0 )
        topNode = 0;
    }
    if ( !topNode )
      return newTetra;

    nodes.push_back( topNode );

    newTetra = helper.AddVolume( nodes[ 0 ], nodes[ 1 ], nodes[ 2 ], nodes[ 3 ]);

    return newTetra;
  }

  //=======================================================================
  //function : addPenta
  //purpose  :
  //=======================================================================

  const SMDS_MeshElement* addPenta( std::vector< const SMDS_MeshElement* >& faces,
                                    const std::vector< int > &              quantities,
                                    SMESH_MesherHelper &                    helper )
  {
    const SMDS_MeshElement* newPenta = 0;

    // check nb of nodes in faces and find triangle faces
    int trias[2] = { -1, -1 };
    for ( size_t i = 0; i < quantities.size(); ++i )
      if ( quantities[ i ] != 4 )
      {
        if ( quantities[ i ] != 3 )
          return newPenta;
        int iTria = ( trias[0] != -1 );
        if ( trias[ iTria ] != -1 )
          return newPenta;
        trias[ iTria ] = i;
      }
    if ( trias[1] == -1 )
      return newPenta;

    int iSide = trias[0] + 1;
    if ( iSide == trias[1] )
      ++iSide;

    const SMDS_MeshElement* botFace  = faces[ trias[0]];
    const SMDS_MeshElement* topFace  = faces[ trias[1]];
    const SMDS_MeshElement* sideFace = faces[ iSide ];
    const SMDS_MeshNode* nodes[ 6 ] = { 0,0,0,0,0,0 };
    for ( int i = 0 ; i < 3; ++i )
    {
      const SMDS_MeshNode* botNode = botFace->GetNode( i );
      if ( sideFace->GetNodeIndex( botNode ) < 0 )
        nodes[2] = botNode;
      else
        nodes[ bool( nodes[0] )] = botNode;

      const SMDS_MeshNode* topNode = topFace->GetNode( i );
      if ( sideFace->GetNodeIndex( topNode ) < 0 )
        nodes[5] = topNode;
      else
        nodes[ 3 + bool( nodes[3]) ] = topNode;
    }
    bool botOriRight = SMESH_MeshAlgos::IsRightOrder( sideFace, nodes[ 0 ], nodes[ 1 ]);
    bool topOriRight = SMESH_MeshAlgos::IsRightOrder( sideFace, nodes[ 3 ], nodes[ 4 ]);
    if ( botOriRight == topOriRight )
      std::swap( nodes[ 3 ], nodes[ 4 ]);

    newPenta = helper.AddVolume( nodes[ 0 ], nodes[ 1 ], nodes[ 2 ],
                                 nodes[ 3 ], nodes[ 4 ], nodes[ 5 ]);

    return newPenta;
  }

  //=======================================================================
  //function : addPyra
  //purpose  :
  //=======================================================================

  const SMDS_MeshElement* addPyra( std::vector< const SMDS_MeshElement* >& faces,
                                   const std::vector< int > &              quantities,
                                   SMESH_MesherHelper &                    helper )
  {
    const SMDS_MeshElement* newPyra = 0;

    // check nb of nodes in faces
    int iBot = -1;
    for ( size_t i = 0; i < quantities.size(); ++i )
      if ( quantities[ i ] != 3 )
      {
        if ( quantities[ i ] != 4 || iBot != -1 )
          return newPyra;
        iBot = i;
      }

    const SMDS_MeshElement* botFace = faces[ iBot ];

    std::vector< const SMDS_MeshNode* > nodes( 8 );
    nodes.assign( botFace->begin_nodes(), botFace->end_nodes() );
    nodes.resize( 4 );

    const SMDS_MeshNode* topNode = 0;
    for ( size_t i = 0; i < 4 &&  !topNode; ++i )
    {
      topNode = faces[ 1 ]->GetNode( i );
      if ( botFace->GetNodeIndex( topNode ) >= 0 )
        topNode = 0;
    }
    if ( !topNode )
      return newPyra;

    nodes.push_back( topNode );

    newPyra = helper.AddVolume( nodes[ 0 ], nodes[ 1 ], nodes[ 2 ], nodes[ 3 ], nodes[4] );

    return newPyra;
  }

  //=======================================================================
  //function : addHPrism
  //purpose  : add hexagonal prism
  //=======================================================================

  const SMDS_MeshElement* addHPrism( std::vector< const SMDS_MeshElement* >& faces,
                                     const std::vector< int > &              quantities,
                                     SMESH_MesherHelper &                    helper )
  {
    const SMDS_MeshElement* newHexPrism = 0;

    // check nb of nodes in faces and find hexagons
    int hexa[2] = { -1, -1 };
    for ( size_t i = 0; i < quantities.size(); ++i )
      if ( quantities[ i ] != 4 )
      {
        if ( quantities[ i ] != 6 )
          return newHexPrism;
        int iHex = ( hexa[0] != -1 );
        if ( hexa[ iHex ] != -1 )
          return newHexPrism;
        hexa[ iHex ] = i;
      }
    if ( hexa[1] == -1 )
      return newHexPrism;

    int iSide = hexa[0] + 1;
    if ( iSide == hexa[1] )
      ++iSide;

    const SMDS_MeshElement* botFace = faces[ hexa[ 0 ]];
    const SMDS_MeshElement* topFace = faces[ hexa[ 1 ]];
    std::vector< const SMDS_MeshNode* > nodes( 24 ); // last 12 is a working buffer

    nodes.assign( botFace->begin_nodes(), botFace->end_nodes() );
    nodes.resize( 12 ); // set top nodes after hexa nodes - [12-17]
    nodes.insert( nodes.end(), topFace->begin_nodes(), topFace->end_nodes() );
    nodes.resize( 18 );
    nodes.insert( nodes.end(), nodes.begin() + 12, nodes.begin() + 18 );

    // find corresponding nodes of top and bottom by finding a side face including 2 node of each
    SMESHDS_Mesh* mesh = helper.GetMeshDS();
    const SMDS_MeshElement* sideFace = 0;
    size_t i;
    for ( i = 12; i < nodes.size()-1 &&  !sideFace; ++i )
    {
      sideFace = mesh->FindFace( nodes[0], nodes[1], nodes[ i ], nodes[ i + 1 ]);
    }
    if ( !sideFace )
      return newHexPrism;

    --i; // restore after ++i in the loop
    bool botOriRight = SMESH_MeshAlgos::IsRightOrder( sideFace, nodes[ 0 ], nodes[ 1 ] );
    bool topOriRight = SMESH_MeshAlgos::IsRightOrder( sideFace, nodes[ i ], nodes[ i + 1 ] );
    if ( botOriRight == topOriRight )
    {
      nodes[ 6  ] = nodes[ i + 1 ];
      nodes[ 7  ] = nodes[ i + 0 ];
      nodes[ 8  ] = nodes[ i + 5 ];
      nodes[ 9  ] = nodes[ i + 4 ];
      nodes[ 10 ] = nodes[ i + 3 ];
      nodes[ 11 ] = nodes[ i + 2 ];
    }
    else
    {
      nodes[ 6  ] = nodes[ i + 0 ];
      nodes[ 7  ] = nodes[ i + 1 ];
      nodes[ 8  ] = nodes[ i + 2 ];
      nodes[ 9  ] = nodes[ i + 3 ];
      nodes[ 10 ] = nodes[ i + 4 ];
      nodes[ 11 ] = nodes[ i + 5 ];
    }

    newHexPrism = helper.AddVolume( nodes[ 0 ], nodes[ 1 ], nodes[ 2 ],
                                    nodes[ 3 ], nodes[ 4 ], nodes[ 5 ],
                                    nodes[ 6 ], nodes[ 7 ], nodes[ 8 ],
                                    nodes[ 9 ], nodes[10 ], nodes[11 ]);

    return newHexPrism;
  }

  //=======================================================================
  //function : addPoly
  //purpose  :
  //=======================================================================

  const SMDS_MeshElement* addPoly( std::vector< const SMDS_MeshElement* >& faces,
                                   const std::vector< int > &              quantities,
                                   SMESH_MesherHelper &                    helper )
  {
    const SMDS_MeshElement* newPoly = 0;

    std::vector< const SMDS_MeshNode* > nodes;
    for ( size_t iF = 0; iF < faces.size(); ++iF )
      nodes.insert( nodes.end(), faces[iF]->begin_nodes(), faces[iF]->end_nodes() );

    newPoly = helper.AddPolyhedralVolume( nodes, quantities );

    return newPoly;
  }

} // namespace

//=======================================================================
//function : StdMeshers_PolyhedronPerSolid_3D
//purpose  :
//=======================================================================

StdMeshers_PolyhedronPerSolid_3D::StdMeshers_PolyhedronPerSolid_3D(int        hypId,
                                                                   SMESH_Gen* gen)
  :SMESH_3D_Algo(hypId, gen),
   myEdgeMesher( new _EdgeMesher( gen->GetANewId(), gen )),
   myFaceMesher( new StdMeshers_PolygonPerFace_2D( gen->GetANewId(), gen ))
{
  _name = "PolyhedronPerSolid_3D";
  _requireDiscreteBoundary = false;
  _supportSubmeshes = true;
  _compatibleHypothesis.push_back("ViscousLayers");
  _neededLowerHyps[0] = _neededLowerHyps[1] = _neededLowerHyps[2] = true;
}

//=======================================================================
//function : ~StdMeshers_PolyhedronPerSolid_3D
//purpose  :
//=======================================================================

StdMeshers_PolyhedronPerSolid_3D::~StdMeshers_PolyhedronPerSolid_3D()
{
  delete myEdgeMesher;
  delete myFaceMesher;
}

//=======================================================================
//function : CheckHypothesis
//purpose  :
//=======================================================================

bool StdMeshers_PolyhedronPerSolid_3D::CheckHypothesis(SMESH_Mesh&         theMesh,
                                                       const TopoDS_Shape& theShape,
                                                       Hypothesis_Status&  theStatus)
{
  myViscousLayersHyp = NULL;

  const std::list<const SMESHDS_Hypothesis*>& hyps =
    GetUsedHypothesis( theMesh, theShape, /*ignoreAuxiliary=*/false);
  std::list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  if ( h == hyps.end())
  {
    theStatus = SMESH_Hypothesis::HYP_OK;
    return true;
  }

  // only StdMeshers_ViscousLayers can be used
  theStatus = HYP_OK;
  for ( ; h != hyps.end(); ++h )
  {
    if ( !(myViscousLayersHyp = dynamic_cast< const StdMeshers_ViscousLayers*> ( *h )))
      break;
  }
  if ( !myViscousLayersHyp )
    theStatus = HYP_INCOMPATIBLE;
  else
    error( myViscousLayersHyp->CheckHypothesis( theMesh, theShape, theStatus ));

  return theStatus == HYP_OK;
}

//=======================================================================
//function : Compute
//purpose  :
//=======================================================================

bool StdMeshers_PolyhedronPerSolid_3D::Compute(SMESH_Mesh&         theMesh,
                                               const TopoDS_Shape& theShape)
{
  const SMDS_MeshElement* newVolume = 0;

  SMESH_subMesh* sm = theMesh.GetSubMesh( theShape );
  SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator( /*includeSelf=*/true,
                                                            /*complexFirst=*/false);
  while ( smIt->more() )
  {
    sm = smIt->next();
    if ( !sm->IsEmpty() )
      continue;

    const TopoDS_Shape & shape = sm->GetSubShape();
    switch ( shape.ShapeType() )
    {
    case TopAbs_VERTEX:
      sm->ComputeStateEngine( SMESH_subMesh::COMPUTE );
      break;

    case TopAbs_EDGE:
      sm->ComputeStateEngine( SMESH_subMesh::COMPUTE );
      if ( sm->IsEmpty() )
        myEdgeMesher->Compute( theMesh, shape );
      break;

    case TopAbs_FACE:
      sm->ComputeStateEngine( SMESH_subMesh::COMPUTE );
      if ( sm->IsEmpty() && !myFaceMesher->Compute( theMesh, shape ))
      {
        sm->GetComputeError() = myFaceMesher->GetComputeError();
        sm->GetComputeError()->myAlgo = myFaceMesher;
        return false;
      }
      break;

    case TopAbs_SOLID:
    {
      SMESH_MesherHelper helper( theMesh );
      helper.SetElementsOnShape( true );
      _quadraticMesh = helper.IsQuadraticSubMesh( shape );

      SMESH_ProxyMesh::Ptr proxymesh( new SMESH_ProxyMesh( theMesh ));
      if ( myViscousLayersHyp )
      {
        proxymesh = myViscousLayersHyp->Compute( theMesh, theShape );
        if ( !proxymesh )
          return false;
      }

      std::vector< const SMDS_MeshElement* > faces;
      faces.reserve( 20 );

      for ( TopExp_Explorer faceEx( shape, TopAbs_FACE ); faceEx.More(); faceEx.Next() )
      {
        const SMESHDS_SubMesh* smDS = proxymesh->GetSubMesh( faceEx.Current() );
        for ( SMDS_ElemIteratorPtr faceIt = smDS->GetElements(); faceIt->more(); )
          faces.push_back( faceIt->next() );
      }

      bool useMediumNodes = false;
      if ( !_quadraticMesh && theMesh.GetMeshDS()->GetMeshInfo().NbFaces( ORDER_QUADRATIC ))
        for ( size_t i = 0; i < faces.size() &&  !useMediumNodes ; ++i )
          useMediumNodes = faces[ i ]->IsQuadratic();

      std::vector< int > quantities( faces.size() );
      std::set< const SMDS_MeshNode* > nodes;
      for ( size_t i = 0; i < faces.size(); ++i )
      {
        quantities[ i ] = useMediumNodes ? faces[ i ]->NbNodes() : faces[ i ]->NbCornerNodes();
        for ( int iN = 0; iN < quantities[ i ]; ++iN )
          nodes.insert( faces[ i ]->GetNode( iN ));
      }

      const size_t nbNodes = nodes.size(), nbFaces = faces.size();
      if (      nbNodes == 8  && nbFaces == 6 ) newVolume = addHexa  ( faces, quantities, helper );
      else if ( nbNodes == 4  && nbFaces == 4 ) newVolume = addTetra ( faces, quantities, helper );
      else if ( nbNodes == 6  && nbFaces == 5 ) newVolume = addPenta ( faces, quantities, helper );
      else if ( nbNodes == 5  && nbFaces == 5 ) newVolume = addPyra  ( faces, quantities, helper );
      else if ( nbNodes == 12 && nbFaces == 8 ) newVolume = addHPrism( faces, quantities, helper );
      if ( !newVolume )
        newVolume = addPoly ( faces, quantities, helper );

      if ( newVolume )
      {
        SMESH::Controls::BadOrientedVolume checker;
        checker.SetMesh( theMesh.GetMeshDS() );
        if ( checker.IsSatisfy( newVolume->GetID() ))
        {
          SMESH_MeshEditor editor( &theMesh );
          editor.Reorient( newVolume );
        }
      }
    }
    default:;

    } // switch ( shape.ShapeType() )
  } // loop on sub-meshes

  return newVolume;
}

//=======================================================================
//function : Evaluate
//purpose  :
//=======================================================================

bool StdMeshers_PolyhedronPerSolid_3D::Evaluate(SMESH_Mesh&         theMesh,
                                                const TopoDS_Shape& theShape,
                                                MapShapeNbElems&    theResMap)
{
  _quadraticMesh = false;

  SMESH_subMesh* sm = theMesh.GetSubMesh( theShape );
  SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator( /*includeSelf=*/true,
                                                            /*complexFirst=*/false);
  while ( smIt->more() )
  {
    sm = smIt->next();

    MapShapeNbElems::iterator sm2vec = theResMap.find( sm );
    if ( sm2vec != theResMap.end() && !sm2vec->second.empty() )
      continue;

    const TopoDS_Shape & shape = sm->GetSubShape();
    switch ( shape.ShapeType() )
    {
    case TopAbs_EDGE:
      myEdgeMesher->Evaluate( theMesh, shape, theResMap );
      break;

    case TopAbs_FACE:
    {
      myFaceMesher->Evaluate( theMesh, shape, theResMap );
      std::vector<int> & quantities = theResMap[ sm ];
      _quadraticMesh = ( !quantities.empty() &&
                         ( quantities[ SMDSEntity_Quad_Triangle   ] +
                           quantities[ SMDSEntity_Quad_Quadrangle ] +
                           quantities[ SMDSEntity_Quad_Polygon    ]));
      break;
    }

    case TopAbs_SOLID:
    {
      std::vector<int> & quantities = theResMap[ sm ];
      quantities.resize( SMDSEntity_Last, 0 );

      SMESH_MesherHelper helper( theMesh );
      const int nbNodes = helper.Count( shape, TopAbs_VERTEX, /*ignoreSame=*/true );
      const int nbFaces = helper.Count( shape, TopAbs_FACE,   /*ignoreSame=*/false );

      if (      nbNodes == 8 && nbFaces == 6 )
        quantities[ _quadraticMesh ? SMDSEntity_Quad_Hexa : SMDSEntity_Hexa ] = 1;
      else if ( nbNodes == 4 && nbFaces == 4 )
        quantities[ _quadraticMesh ? SMDSEntity_Quad_Tetra : SMDSEntity_Tetra ] = 1;
      else if ( nbNodes == 6 && nbFaces == 5 )
        quantities[ _quadraticMesh ? SMDSEntity_Quad_Penta : SMDSEntity_Penta ] = 1;
      else if ( nbNodes == 5 && nbFaces == 5 )
        quantities[ _quadraticMesh ? SMDSEntity_Quad_Pyramid : SMDSEntity_Pyramid ] = 1;
      else if ( nbNodes == 12 && nbFaces == 8 )
        quantities[ /*_quadraticMesh ? SMDSEntity_Quad_Pyramid :*/ SMDSEntity_Hexagonal_Prism ] = 1;
      else
        quantities[ /*_quadraticMesh ? SMDSEntity_Quad_Polyhedra : */SMDSEntity_Polyhedra ] = 1;

      return true;
    }
    default:;

    } // switch ( shape.ShapeType() )
  } // loop on sub-meshes

  return false;
}
