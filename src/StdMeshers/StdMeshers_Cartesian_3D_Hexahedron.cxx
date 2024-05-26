// Copyright (C) 2016-2024  CEA, EDF
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : StdMeshers_Cartesian_3D_Hexahedron.cxx
//  Module : SMESH
//  Purpose: Make BodyFitting mesh algorithm more modular and testable
//

#include "StdMeshers_Cartesian_3D_Hexahedron.hxx"

using namespace SMESH;
using namespace gridtools;

std::mutex _eMutex;

#ifdef WITH_TBB
  // --------------------------------------------------------------------------
  /*!
   * \brief Hexahedron computing volumes in one thread
   */
  struct ParallelHexahedron
  {
    vector< Hexahedron* >& _hexVec;
    ParallelHexahedron( vector< Hexahedron* >& hv ): _hexVec(hv) {}
    void operator() ( const tbb::blocked_range<size_t>& r ) const
    {
      for ( size_t i = r.begin(); i != r.end(); ++i )
        if ( Hexahedron* hex = _hexVec[ i ] )
          hex->computeElements();
    }
  };
#endif

//=============================================================================
// Implementation of internal utils
//=============================================================================
/*!
  * \brief adjust \a i to have \a val between values[i] and values[i+1]
  */
inline void locateValue( int & i, double val, const vector<double>& values,
                          int& di, double tol )
{
  //val += values[0]; // input \a val is measured from 0.
  if ( i > (int) values.size()-2 )
    i = values.size()-2;
  else
    while ( i+2 < (int) values.size() && val > values[ i+1 ])
      ++i;
  while ( i > 0 && val < values[ i ])
    --i;

  if ( i > 0 && val - values[ i ] < tol )
    di = -1;
  else if ( i+2 < (int) values.size() && values[ i+1 ] - val < tol )
    di = 1;
  else
    di = 0;
}

//================================================================================
/*!
  * \brief Find existing triangulation of a polygon
  */
int findExistingTriangulation( const SMDS_MeshElement*              polygon,
                                //const SMDS_Mesh*                     mesh,
                                std::vector< const SMDS_MeshNode* >& nodes )
{
  int nbSplits = 0;
  nodes.clear();
  std::vector<const SMDS_MeshNode *>    twoNodes(2);
  std::vector<const SMDS_MeshElement *> foundFaces; foundFaces.reserve(10);
  std::set< const SMDS_MeshElement * >  avoidFaces; avoidFaces.insert( polygon );

  const int nbPolyNodes = polygon->NbCornerNodes();
  twoNodes[1] = polygon->GetNode( nbPolyNodes - 1 );
  for ( int iN = 0; iN < nbPolyNodes; ++iN ) // loop on border links of polygon
  {
    twoNodes[0] = polygon->GetNode( iN );

    int nbFaces = SMDS_Mesh::GetElementsByNodes( twoNodes, foundFaces, SMDSAbs_Face );
    int nbOkFaces = 0;
    for ( int iF = 0; iF < nbFaces; ++iF ) // keep faces lying over polygon
    {
      if ( avoidFaces.count( foundFaces[ iF ]))
        continue;
      int i, nbFaceNodes = foundFaces[ iF ]->NbCornerNodes();
      for ( i = 0; i < nbFaceNodes; ++i )
      {
        const SMDS_MeshNode* n = foundFaces[ iF ]->GetNode( i );
        bool isCommonNode = ( n == twoNodes[0] ||
                              n == twoNodes[1] ||
                              polygon->GetNodeIndex( n ) >= 0 );
        if ( !isCommonNode )
          break;
      }
      if ( i == nbFaceNodes ) // all nodes of foundFaces[iF] are shared with polygon
        if ( nbOkFaces++ != iF )
          foundFaces[ nbOkFaces-1 ] = foundFaces[ iF ];
    }
    if ( nbOkFaces > 0 )
    {
      int iFaceSelected = 0;
      if ( nbOkFaces > 1 ) // select a face with minimal distance from polygon
      {
        double minDist = Precision::Infinite();
        for ( int iF = 0; iF < nbOkFaces; ++iF )
        {
          int i, nbFaceNodes = foundFaces[ iF ]->NbCornerNodes();
          gp_XYZ gc = SMESH_NodeXYZ( foundFaces[ iF ]->GetNode( 0 ));
          for ( i = 1; i < nbFaceNodes; ++i )
            gc += SMESH_NodeXYZ( foundFaces[ iF ]->GetNode( i ));
          gc /= nbFaceNodes;

          double dist = SMESH_MeshAlgos::GetDistance( polygon, gc );
          if ( dist < minDist )
          {
            minDist = dist;
            iFaceSelected = iF;
          }
        }
      }
      if ( foundFaces[ iFaceSelected ]->NbCornerNodes() != 3 )
        return 0;
      nodes.insert( nodes.end(),
                    foundFaces[ iFaceSelected ]->begin_nodes(),
                    foundFaces[ iFaceSelected ]->end_nodes());
      if ( !SMESH_MeshAlgos::IsRightOrder( foundFaces[ iFaceSelected ],
                                            twoNodes[0], twoNodes[1] ))
      {
        // reverse just added nodes
        std::reverse( nodes.end() - 3, nodes.end() );
      }
      avoidFaces.insert( foundFaces[ iFaceSelected ]);
      nbSplits++;
    }

    twoNodes[1] = twoNodes[0];

  } // loop on polygon nodes

  return nbSplits;
}
//================================================================================
/*!
  * \brief Divide a polygon into triangles and modify accordingly an adjacent polyhedron
  */
void splitPolygon( const SMDS_MeshElement*         polygon,
                    SMDS_VolumeTool &               volume,
                    const int                       facetIndex,
                    const TGeomID                   faceID,
                    const TGeomID                   solidID,
                    SMESH_MeshEditor::ElemFeatures& face,
                    SMESH_MeshEditor&               editor,
                    const bool                      reinitVolume)
{
  SMESH_MeshAlgos::Triangulate divider(/*optimize=*/false);
  bool triangulationExist = false;
  int nbTrias = findExistingTriangulation( polygon, face.myNodes );
  if ( nbTrias > 0 )
    triangulationExist = true;
  else
    nbTrias = divider.GetTriangles( polygon, face.myNodes );
  face.myNodes.resize( nbTrias * 3 );

  SMESH_MeshEditor::ElemFeatures newVolumeDef;
  newVolumeDef.Init( volume.Element() );
  newVolumeDef.SetID( volume.Element()->GetID() );

  newVolumeDef.myPolyhedQuantities.reserve( volume.NbFaces() + nbTrias );
  newVolumeDef.myNodes.reserve( volume.NbNodes() + nbTrias * 3 );

  SMESHDS_Mesh* meshDS = editor.GetMeshDS();
  SMDS_MeshElement* newTriangle;
  for ( int iF = 0, nF = volume.NbFaces(); iF < nF; iF++ )
  {
    if ( iF == facetIndex )
    {
      newVolumeDef.myPolyhedQuantities.push_back( 3 );
      newVolumeDef.myNodes.insert( newVolumeDef.myNodes.end(),
                                    face.myNodes.begin(),
                                    face.myNodes.begin() + 3 );
      meshDS->RemoveFreeElement( polygon, 0, false );
      if ( !triangulationExist )
      {
        newTriangle = meshDS->AddFace( face.myNodes[0], face.myNodes[1], face.myNodes[2] );
        meshDS->SetMeshElementOnShape( newTriangle, faceID );
      }
    }
    else
    {
      const SMDS_MeshNode** nn = volume.GetFaceNodes( iF );
      const size_t nbFaceNodes = volume.NbFaceNodes ( iF );
      newVolumeDef.myPolyhedQuantities.push_back( nbFaceNodes );
      newVolumeDef.myNodes.insert( newVolumeDef.myNodes.end(), nn, nn + nbFaceNodes );
    }
  }

  for ( size_t iN = 3; iN < face.myNodes.size(); iN += 3 )
  {
    newVolumeDef.myPolyhedQuantities.push_back( 3 );
    newVolumeDef.myNodes.insert( newVolumeDef.myNodes.end(),
                                  face.myNodes.begin() + iN,
                                  face.myNodes.begin() + iN + 3 );
    if ( !triangulationExist )
    {
      newTriangle = meshDS->AddFace( face.myNodes[iN], face.myNodes[iN+1], face.myNodes[iN+2] );
      meshDS->SetMeshElementOnShape( newTriangle, faceID );
    }
  }

  meshDS->RemoveFreeElement( volume.Element(), 0, false );
  SMDS_MeshElement* newVolume = editor.AddElement( newVolumeDef.myNodes, newVolumeDef );
  meshDS->SetMeshElementOnShape( newVolume, solidID );

  if ( reinitVolume )
  {
    volume.Set( 0 );
    volume.Set( newVolume );
  }
  return;
}

  //================================================================================
/*!
  * \brief Look for a FACE supporting all given nodes made on EDGEs and VERTEXes
  */
TGeomID findCommonFace( const std::vector< const SMDS_MeshNode* > & nn,
                        const SMESH_Mesh*                           mesh )
{
  TGeomID faceID = 0;
  TGeomID shapeIDs[20];
  for ( size_t iN = 0; iN < nn.size(); ++iN )
    shapeIDs[ iN ] = nn[ iN ]->GetShapeID();

  SMESH_subMesh* sm = mesh->GetSubMeshContaining( shapeIDs[ 0 ]);
  for ( const SMESH_subMesh * smFace : sm->GetAncestors() )
  {
    if ( smFace->GetSubShape().ShapeType() != TopAbs_FACE )
      continue;

    faceID = smFace->GetId();

    for ( size_t iN = 1; iN < nn.size() &&  faceID; ++iN )
    {
      if ( !smFace->DependsOn( shapeIDs[ iN ]))
        faceID = 0;
    }
    if ( faceID > 0 )
      break;
  }
  return faceID;
}


//================================================================================
/*!
  * \brief Creates topology of the hexahedron
  */
Hexahedron::Hexahedron(Grid* grid)
  : _grid( grid ), _nbFaceIntNodes(0), _hasTooSmall( false )
{
  _polygons.reserve(100); // to avoid reallocation;

  //set nodes shift within grid->_nodes from the node 000 
  size_t dx = _grid->NodeIndexDX();
  size_t dy = _grid->NodeIndexDY();
  size_t dz = _grid->NodeIndexDZ();
  size_t i000 = 0;
  size_t i100 = i000 + dx;
  size_t i010 = i000 + dy;
  size_t i110 = i010 + dx;
  size_t i001 = i000 + dz;
  size_t i101 = i100 + dz;
  size_t i011 = i010 + dz;
  size_t i111 = i110 + dz;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V000 )] = i000;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V100 )] = i100;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V010 )] = i010;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V110 )] = i110;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V001 )] = i001;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V101 )] = i101;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V011 )] = i011;
  grid->_nodeShift[ SMESH_Block::ShapeIndex( SMESH_Block::ID_V111 )] = i111;

  vector< int > idVec;
  // set nodes to links
  for ( int linkID = SMESH_Block::ID_Ex00; linkID <= SMESH_Block::ID_E11z; ++linkID )
  {
    SMESH_Block::GetEdgeVertexIDs( linkID, idVec );
    _Link& link = _hexLinks[ SMESH_Block::ShapeIndex( linkID )];
    link._nodes[0] = &_hexNodes[ SMESH_Block::ShapeIndex( idVec[0] )];
    link._nodes[1] = &_hexNodes[ SMESH_Block::ShapeIndex( idVec[1] )];
  }

  // set links to faces
  int interlace[4] = { 0, 3, 1, 2 }; // to walk by links around a face: { u0, 1v, u1, 0v }
  for ( int faceID = SMESH_Block::ID_Fxy0; faceID <= SMESH_Block::ID_F1yz; ++faceID )
  {
    _Face& quad = _hexQuads[ SMESH_Block::ShapeIndex( faceID )];
    quad._name = (SMESH_Block::TShapeID) faceID;

    SMESH_Block::GetFaceEdgesIDs( faceID, idVec );
    bool revFace = ( faceID == SMESH_Block::ID_Fxy0 ||
                      faceID == SMESH_Block::ID_Fx1z ||
                      faceID == SMESH_Block::ID_F0yz );
    quad._links.resize(4);
    vector<_OrientedLink>::iterator         frwLinkIt = quad._links.begin();
    vector<_OrientedLink>::reverse_iterator revLinkIt = quad._links.rbegin();
    for ( int i = 0; i < 4; ++i )
    {
      bool revLink = revFace;
      if ( i > 1 ) // reverse links u1 and v0
        revLink = !revLink;
      _OrientedLink& link = revFace ? *revLinkIt++ : *frwLinkIt++;
      link = _OrientedLink( & _hexLinks[ SMESH_Block::ShapeIndex( idVec[interlace[i]] )],
                            revLink );
    }
  }
}
//================================================================================
/*!
  * \brief Copy constructor
  */
Hexahedron::Hexahedron( const Hexahedron& other, size_t i, size_t j, size_t k, int cellID )
  :_grid( other._grid ), _nbFaceIntNodes(0), _i( i ), _j( j ), _k( k ), _hasTooSmall( false )
{
  _polygons.reserve(100); // to avoid reallocation;

  // copy topology
  for ( int i = 0; i < 12; ++i )
  {
    const _Link& srcLink = other._hexLinks[ i ];
    _Link&       tgtLink = this->_hexLinks[ i ];
    tgtLink._nodes[0] = _hexNodes + ( srcLink._nodes[0] - other._hexNodes );
    tgtLink._nodes[1] = _hexNodes + ( srcLink._nodes[1] - other._hexNodes );
  }

  for ( int i = 0; i < 6; ++i )
  {
    const _Face& srcQuad = other._hexQuads[ i ];
    _Face&       tgtQuad = this->_hexQuads[ i ];
    tgtQuad._name = srcQuad._name;
    tgtQuad._links.resize(4);
    for ( int j = 0; j < 4; ++j )
    {
      const _OrientedLink& srcLink = srcQuad._links[ j ];
      _OrientedLink&       tgtLink = tgtQuad._links[ j ];
      tgtLink._reverse = srcLink._reverse;
      tgtLink._link    = _hexLinks + ( srcLink._link - other._hexLinks );
    }
  }
  
  if (SALOME::VerbosityActivated())
    _cellID = cellID;
}

void Hexahedron::_Node::Add( const E_IntersectPoint* ip )
{
  const std::lock_guard<std::mutex> lock(_eMutex);
  // Possible cases before Add(ip):
  ///  1) _node != 0 --> _Node at hex corner ( _intPoint == 0 || _intPoint._node == 0 )
  ///  2) _node == 0 && _intPoint._node != 0  -->  link intersected by FACE
  ///  3) _node == 0 && _intPoint._node == 0  -->  _Node at EDGE intersection
  //
  // If ip is added in cases 1) and 2) _node position must be changed to ip._shapeID
  //   at creation of elements
  // To recognize this case, set _intPoint._node = Node()
  const SMDS_MeshNode* node = Node();
  if ( !_intPoint ) {
    _intPoint = ip;
  }
  else {
    ip->Add( _intPoint->_faceIDs );
    _intPoint = ip;
  }
  if ( node )
    _node = _intPoint->_node = node;
}

//================================================================================
/*!
  * \brief Return IDs of SOLIDs interfering with this Hexahedron
  */
size_t Hexahedron::getSolids( TGeomID ids[] )
{
  if ( _grid->_geometry.IsOneSolid() )
  {
    ids[0] = _grid->GetSolid()->ID();
    return 1;
  }
  // count intersection points belonging to each SOLID
  TID2Nb id2NbPoints;
  id2NbPoints.reserve( 3 );

  _origNodeInd = _grid->NodeIndex( _i,_j,_k );
  for ( int iN = 0; iN < 8; ++iN )
  {
    _hexNodes[iN]._node     = _grid->_nodes   [ _origNodeInd + _grid->_nodeShift[iN] ];
    _hexNodes[iN]._intPoint = _grid->_gridIntP[ _origNodeInd + _grid->_nodeShift[iN] ];

    if ( _hexNodes[iN]._intPoint ) // intersection with a FACE
    {
      for ( size_t iF = 0; iF < _hexNodes[iN]._intPoint->_faceIDs.size(); ++iF )
      {
        const vector< TGeomID > & solidIDs =
          _grid->GetSolidIDs( _hexNodes[iN]._intPoint->_faceIDs[iF] );
        for ( size_t i = 0; i < solidIDs.size(); ++i )
          insertAndIncrement( solidIDs[i], id2NbPoints );
      }
    }
    else if ( _hexNodes[iN]._node ) // node inside a SOLID
    {
      insertAndIncrement( _hexNodes[iN]._node->GetShapeID(), id2NbPoints );
    }
  }

  for ( int iL = 0; iL < 12; ++iL )
  {
    const _Link& link = _hexLinks[ iL ];
    for ( size_t iP = 0; iP < link._fIntPoints.size(); ++iP )
    {
      for ( size_t iF = 0; iF < link._fIntPoints[iP]->_faceIDs.size(); ++iF )
      {
        const vector< TGeomID > & solidIDs =
          _grid->GetSolidIDs( link._fIntPoints[iP]->_faceIDs[iF] );
        for ( size_t i = 0; i < solidIDs.size(); ++i )
          insertAndIncrement( solidIDs[i], id2NbPoints );
      }
    }
  }

  for ( size_t iP = 0; iP < _eIntPoints.size(); ++iP )
  {
    const vector< TGeomID > & solidIDs = _grid->GetSolidIDs( _eIntPoints[iP]->_shapeID );
    for ( size_t i = 0; i < solidIDs.size(); ++i )
      insertAndIncrement( solidIDs[i], id2NbPoints );
  }

  size_t nbSolids = 0;
  for ( TID2Nb::iterator id2nb = id2NbPoints.begin(); id2nb != id2NbPoints.end(); ++id2nb )
    if ( id2nb->second >= 3 )
      ids[ nbSolids++ ] = id2nb->first;

  return nbSolids;
}

//================================================================================
/*!
  * \brief Count cuts by INTERNAL FACEs and set _Node::_isInternalFlags
  */
bool Hexahedron::isCutByInternalFace( IsInternalFlag & maxFlag )
{
  TID2Nb id2NbPoints;
  id2NbPoints.reserve( 3 );

  for ( size_t iN = 0; iN < _intNodes.size(); ++iN )
    for ( size_t iF = 0; iF < _intNodes[iN]._intPoint->_faceIDs.size(); ++iF )
    {
      if ( _grid->IsInternal( _intNodes[iN]._intPoint->_faceIDs[iF]))
        insertAndIncrement( _intNodes[iN]._intPoint->_faceIDs[iF], id2NbPoints );
    }
  for ( size_t iN = 0; iN < 8; ++iN )
    if ( _hexNodes[iN]._intPoint )
      for ( size_t iF = 0; iF < _hexNodes[iN]._intPoint->_faceIDs.size(); ++iF )
      {
        if ( _grid->IsInternal( _hexNodes[iN]._intPoint->_faceIDs[iF]))
          insertAndIncrement( _hexNodes[iN]._intPoint->_faceIDs[iF], id2NbPoints );
      }

  maxFlag = IS_NOT_INTERNAL;
  for ( TID2Nb::iterator id2nb = id2NbPoints.begin(); id2nb != id2NbPoints.end(); ++id2nb )
  {
    TGeomID        intFace = id2nb->first;
    IsInternalFlag intFlag = ( id2nb->second >= 3 ? IS_CUT_BY_INTERNAL_FACE : IS_INTERNAL );
    if ( intFlag > maxFlag )
      maxFlag = intFlag;

    for ( size_t iN = 0; iN < _intNodes.size(); ++iN )
      if ( _intNodes[iN].IsOnFace( intFace ))
        _intNodes[iN].SetInternal( intFlag );

    for ( size_t iN = 0; iN < 8; ++iN )
      if ( _hexNodes[iN].IsOnFace( intFace ))
        _hexNodes[iN].SetInternal( intFlag );
  }

  return maxFlag;
}

//================================================================================
/*!
  * \brief Return any FACE interfering with this Hexahedron
  */
TGeomID Hexahedron::getAnyFace() const
{
  TID2Nb id2NbPoints;
  id2NbPoints.reserve( 3 );

  for ( size_t iN = 0; iN < _intNodes.size(); ++iN )
    for ( size_t iF = 0; iF < _intNodes[iN]._intPoint->_faceIDs.size(); ++iF )
      insertAndIncrement( _intNodes[iN]._intPoint->_faceIDs[iF], id2NbPoints );

  for ( size_t iN = 0; iN < 8; ++iN )
    if ( _hexNodes[iN]._intPoint )
      for ( size_t iF = 0; iF < _hexNodes[iN]._intPoint->_faceIDs.size(); ++iF )
        insertAndIncrement( _hexNodes[iN]._intPoint->_faceIDs[iF], id2NbPoints );

  for ( unsigned int minNb = 3; minNb > 0; --minNb )
    for ( TID2Nb::iterator id2nb = id2NbPoints.begin(); id2nb != id2NbPoints.end(); ++id2nb )
      if ( id2nb->second >= minNb )
        return id2nb->first;

  return 0;
}

//================================================================================
/*!
  * \brief Initializes IJK by Hexahedron index
  */
void Hexahedron::setIJK( size_t iCell )
{
  size_t iNbCell = _grid->_coords[0].size() - 1;
  size_t jNbCell = _grid->_coords[1].size() - 1;
  _i = iCell % iNbCell;
  _j = ( iCell % ( iNbCell * jNbCell )) / iNbCell;
  _k = iCell / iNbCell / jNbCell;
}

//================================================================================
/*!
  * \brief Initializes its data by given grid cell (countered from zero)
  */
void Hexahedron::init( size_t iCell )
{
  setIJK( iCell );
  init( _i, _j, _k );
}

//================================================================================
/*!
  * \brief Initializes its data by given grid cell nodes and intersections
  */
void Hexahedron::init( size_t i, size_t j, size_t k, const Solid* solid )
{
  _i = i; _j = j; _k = k;

  bool isCompute = solid;
  if ( !solid )
    solid = _grid->GetSolid();

  // set nodes of grid to nodes of the hexahedron and
  // count nodes at hexahedron corners located IN and ON geometry
  _nbCornerNodes = _nbBndNodes = 0;
  _origNodeInd   = _grid->NodeIndex( i,j,k );
  for ( int iN = 0; iN < 8; ++iN )
  {
    _hexNodes[iN]._isInternalFlags = 0;

    // Grid  node 
    _hexNodes[iN]._node     = _grid->_nodes   [ _origNodeInd + _grid->_nodeShift[iN] ];
    _hexNodes[iN]._intPoint = _grid->_gridIntP[ _origNodeInd + _grid->_nodeShift[iN] ];

    if ( _grid->_allBorderNodes[ _origNodeInd + _grid->_nodeShift[iN] ] ) 
      _hexNodes[iN]._boundaryCornerNode = _grid->_allBorderNodes [ _origNodeInd + _grid->_nodeShift[iN] ];
    
    if ( _hexNodes[iN]._node && !solid->Contains( _hexNodes[iN]._node->GetShapeID() ))
      _hexNodes[iN]._node = 0;

    if ( _hexNodes[iN]._intPoint && !solid->ContainsAny( _hexNodes[iN]._intPoint->_faceIDs ))
      _hexNodes[iN]._intPoint = 0;

    _nbCornerNodes += bool( _hexNodes[iN]._node );
    _nbBndNodes    += bool( _hexNodes[iN]._intPoint );
  }
  _sideLength[0] = _grid->_coords[0][i+1] - _grid->_coords[0][i];
  _sideLength[1] = _grid->_coords[1][j+1] - _grid->_coords[1][j];
  _sideLength[2] = _grid->_coords[2][k+1] - _grid->_coords[2][k];

  _intNodes.clear();
  _vIntNodes.clear();

  if ( !isCompute )
    return;

  if ( _nbFaceIntNodes + _eIntPoints.size()                  > 0 &&
        _nbFaceIntNodes + _eIntPoints.size() + _nbCornerNodes > 3)
  {
    _intNodes.reserve( 3 * ( _nbBndNodes + _nbFaceIntNodes + _eIntPoints.size() ));

    // this method can be called in parallel, so use own helper
    SMESH_MesherHelper helper( *_grid->_helper->GetMesh() );

    // Create sub-links (_Link::_splits) by splitting links with _Link::_fIntPoints
    // ---------------------------------------------------------------
    _Link split;
    for ( int iLink = 0; iLink < 12; ++iLink )
    {
      _Link& link = _hexLinks[ iLink ];
      link._fIntNodes.clear();
      link._fIntNodes.reserve( link._fIntPoints.size() );
      for ( size_t i = 0; i < link._fIntPoints.size(); ++i )
        if ( solid->ContainsAny( link._fIntPoints[i]->_faceIDs ))
        {
          _intNodes.push_back( _Node( 0, link._fIntPoints[i] ));
          link._fIntNodes.push_back( & _intNodes.back() );
        }

      link._splits.clear();
      split._nodes[ 0 ] = link._nodes[0];
      bool isOut = ( ! link._nodes[0]->Node() );
      bool checkTransition;
      for ( size_t i = 0; i < link._fIntNodes.size(); ++i )
      {
        const bool isGridNode = ( ! link._fIntNodes[i]->Node() );
        if ( !isGridNode ) // intersection non-coincident with a grid node
        {
          if ( split._nodes[ 0 ]->Node() && !isOut )
          {
            split._nodes[ 1 ] = link._fIntNodes[i];
            link._splits.push_back( split );
          }
          split._nodes[ 0 ] = link._fIntNodes[i];
          checkTransition = true;
        }
        else // FACE intersection coincident with a grid node (at link ends)
        {
          checkTransition = ( i == 0 && link._nodes[0]->Node() );
        }
        if ( checkTransition )
        {
          const vector< TGeomID >& faceIDs = link._fIntNodes[i]->_intPoint->_faceIDs;
          if ( _grid->IsInternal( faceIDs.back() ))
            isOut = false;
          else if ( faceIDs.size() > 1 || _eIntPoints.size() > 0 )
            isOut = isOutPoint( link, i, helper, solid );
          else
          {
            bool okTransi = _grid->IsCorrectTransition( faceIDs[0], solid );
            switch ( link._fIntNodes[i]->FaceIntPnt()->_transition ) {
            case Trans_OUT: isOut = okTransi;  break;
            case Trans_IN : isOut = !okTransi; break;
            default:
              isOut = isOutPoint( link, i, helper, solid );
            }
          }
        }
      }
      if ( link._nodes[ 1 ]->Node() && split._nodes[ 0 ]->Node() && !isOut )
      {
        split._nodes[ 1 ] = link._nodes[1];
        link._splits.push_back( split );
      }
    }

    // Create _Node's at intersections with EDGEs.
    // --------------------------------------------
    // 1) add this->_eIntPoints to _Face::_eIntNodes
    // 2) fill _intNodes and _vIntNodes
    //
    const double tol2 = _grid->_tol * _grid->_tol * 4;
    int facets[3], nbFacets, subEntity;

    for ( int iF = 0; iF < 6; ++iF )
      _hexQuads[ iF ]._eIntNodes.clear();

    for ( size_t iP = 0; iP < _eIntPoints.size(); ++iP )
    {
      if ( !solid->ContainsAny( _eIntPoints[iP]->_faceIDs ))
        continue;
      nbFacets = getEntity( _eIntPoints[iP], facets, subEntity );
      _Node* equalNode = 0;
      switch( nbFacets ) {
      case 1: // in a _Face
      {
        _Face& quad = _hexQuads[ facets[0] - SMESH_Block::ID_FirstF ];
        equalNode = findEqualNode( quad._eIntNodes, _eIntPoints[ iP ], tol2 );
        if ( equalNode ) {
          equalNode->Add( _eIntPoints[ iP ] );
        }
        else {
          _intNodes.push_back( _Node( 0, _eIntPoints[ iP ]));
          quad._eIntNodes.push_back( & _intNodes.back() );
        }
        break;
      }
      case 2: // on a _Link
      {
        _Link& link = _hexLinks[ subEntity - SMESH_Block::ID_FirstE ];
        if ( link._splits.size() > 0 )
        {
          equalNode = findEqualNode( link._fIntNodes, _eIntPoints[ iP ], tol2 );
          if ( equalNode )
            equalNode->Add( _eIntPoints[ iP ] );
          else if ( link._splits.size() == 1 &&
                    link._splits[0]._nodes[0] &&
                    link._splits[0]._nodes[1] )
            link._splits.clear(); // hex edge is divided by _eIntPoints[iP]
        }
        //else
        if ( !equalNode )
        {
          _intNodes.push_back( _Node( 0, _eIntPoints[ iP ]));
          bool newNodeUsed = false;
          for ( int iF = 0; iF < 2; ++iF )
          {
            _Face& quad = _hexQuads[ facets[iF] - SMESH_Block::ID_FirstF ];
            equalNode = findEqualNode( quad._eIntNodes, _eIntPoints[ iP ], tol2 );
            if ( equalNode ) {
              equalNode->Add( _eIntPoints[ iP ] );
            }
            else {
              quad._eIntNodes.push_back( & _intNodes.back() );
              newNodeUsed = true;
            }
          }
          if ( !newNodeUsed )
            _intNodes.pop_back();
        }
        break;
      }
      case 3: // at a corner
      {
        _Node& node = _hexNodes[ subEntity - SMESH_Block::ID_FirstV ];
        if ( node.Node() )
        {
          if ( node._intPoint )
            node._intPoint->Add( _eIntPoints[ iP ]->_faceIDs, _eIntPoints[ iP ]->_node );
        }
        else
        {
          _intNodes.push_back( _Node( 0, _eIntPoints[ iP ]));
          for ( int iF = 0; iF < 3; ++iF )
          {
            _Face& quad = _hexQuads[ facets[iF] - SMESH_Block::ID_FirstF ];
            equalNode = findEqualNode( quad._eIntNodes, _eIntPoints[ iP ], tol2 );
            if ( equalNode ) {
              equalNode->Add( _eIntPoints[ iP ] );
            }
            else {
              quad._eIntNodes.push_back( & _intNodes.back() );
            }
          }
        }
        break;
      }
      } // switch( nbFacets )

      if ( nbFacets == 0 ||
            _grid->ShapeType( _eIntPoints[ iP ]->_shapeID ) == TopAbs_VERTEX )
      {
        equalNode = findEqualNode( _vIntNodes, _eIntPoints[ iP ], tol2 );
        if ( equalNode ) {
          equalNode->Add( _eIntPoints[ iP ] );
        }
        else if ( nbFacets == 0 ) {
          if ( _intNodes.empty() || _intNodes.back().EdgeIntPnt() != _eIntPoints[ iP ])
            _intNodes.push_back( _Node( 0, _eIntPoints[ iP ]));
          _vIntNodes.push_back( & _intNodes.back() );
        }
      }
    } // loop on _eIntPoints
  }

  else if (( 3 < _nbCornerNodes && _nbCornerNodes < 8 ) || // _nbFaceIntNodes == 0
            ( !_grid->_geometry.IsOneSolid() ))
  {
    _Link split;
    // create sub-links (_splits) of whole links
    for ( int iLink = 0; iLink < 12; ++iLink )
    {
      _Link& link = _hexLinks[ iLink ];
      link._splits.clear();
      if ( link._nodes[ 0 ]->Node() && link._nodes[ 1 ]->Node() )
      {
        split._nodes[ 0 ] = link._nodes[0];
        split._nodes[ 1 ] = link._nodes[1];
        link._splits.push_back( split );
      }
    }
  }
  return;

} // init( _i, _j, _k )

//================================================================================
/*!
  * \brief Compute mesh volumes resulted from intersection of the Hexahedron
  */
void Hexahedron::computeElements( const Solid* solid, int solidIndex )
{
  if ( !solid )
  {
    solid = _grid->GetSolid();
    if ( !_grid->_geometry.IsOneSolid() )
    {
      TGeomID solidIDs[20] = { 0 };
      size_t nbSolids = getSolids( solidIDs );
      if ( nbSolids > 1 )
      {
        for ( size_t i = 0; i < nbSolids; ++i )
        {
          solid = _grid->GetSolid( solidIDs[i] );
          computeElements( solid, i );
          if ( !_volumeDefs._nodes.empty() && i < nbSolids - 1 )
            _volumeDefs.SetNext( new _volumeDef( _volumeDefs ));
        }
        return;
      }
      solid = _grid->GetSolid( solidIDs[0] );
    }
  }

  init( _i, _j, _k, solid ); // get nodes and intersections from grid nodes and split links

  int nbIntersections = _nbFaceIntNodes + _eIntPoints.size();
  if ( _nbCornerNodes + nbIntersections < 4 )
    return;

  if ( _nbBndNodes == _nbCornerNodes && nbIntersections == 0 && isInHole() )
    return; // cell is in a hole

  IsInternalFlag intFlag = IS_NOT_INTERNAL;
  if ( solid->HasInternalFaces() && this->isCutByInternalFace( intFlag ))
  {
    for ( _SplitIterator it( _hexLinks ); it.More(); it.Next() )
    {
      if ( compute( solid, intFlag ))
        _volumeDefs.SetNext( new _volumeDef( _volumeDefs ));
    }
  }
  else
  {
    if ( solidIndex >= 0 )
      intFlag = IS_CUT_BY_INTERNAL_FACE;

    compute( solid, intFlag );
  }
}

//================================================================================
/*!
  * \brief Compute mesh volumes resulted from intersection of the Hexahedron
  */
void Hexahedron::defineHexahedralFaces( std::vector< _OrientedLink >& splits, std::vector<_Node*>& chainNodes, std::set< TGeomID >& concaveFaces, bool toCheckSideDivision )
{
  for ( int iF = 0; iF < 6; ++iF ) // loop on 6 sides of a hexahedron
  {
    _Face& quad = _hexQuads[ iF ] ;

    _polygons.resize( _polygons.size() + 1 );
    _Face* polygon = &_polygons.back();
    polygon->_polyLinks.reserve( 20 );
    polygon->_name = quad._name;

    splits.clear();
    for ( int iE = 0; iE < 4; ++iE ) // loop on 4 sides of a quadrangle
      for ( size_t iS = 0; iS < quad._links[ iE ].NbResultLinks(); ++iS )
        splits.push_back( quad._links[ iE ].ResultLink( iS ));

    if ( splits.size() == 4 &&
          isQuadOnFace( iF )) // check if a quad on FACE is not split
    {
      polygon->_links.swap( splits );
      continue; // goto the next quad
    }

    // add splits of links to a polygon and add _polyLinks to make
    // polygon's boundary closed

    int nbSplits = splits.size();
    if (( nbSplits == 1 ) &&
        ( quad._eIntNodes.empty() ||
          splits[0].FirstNode()->IsLinked( splits[0].LastNode()->_intPoint )))
      //( quad._eIntNodes.empty() || _nbCornerNodes + nbIntersections > 6 ))
      nbSplits = 0;

    for ( size_t iP = 0; iP < quad._eIntNodes.size(); ++iP )
      if ( quad._eIntNodes[ iP ]->IsUsedInFace( polygon ))
        quad._eIntNodes[ iP ]->_usedInFace = 0;

    size_t nbUsedEdgeNodes = 0;
    _Face* prevPolyg = 0; // polygon previously created from this quad

    while ( nbSplits > 0 )
    {
      size_t iS = 0;
      while ( !splits[ iS ] )
        ++iS;

      if ( !polygon->_links.empty() )
      {
        _polygons.resize( _polygons.size() + 1 );
        polygon = &_polygons.back();
        polygon->_polyLinks.reserve( 20 );
        polygon->_name = quad._name;
      }
      polygon->_links.push_back( splits[ iS ] );
      splits[ iS++ ]._link = 0;
      --nbSplits;

      _Node* nFirst = polygon->_links.back().FirstNode();
      _Node *n1,*n2 = polygon->_links.back().LastNode();
      for ( ; nFirst != n2 && iS < splits.size(); ++iS )
      {
        _OrientedLink& split = splits[ iS ];
        if ( !split ) continue;

        n1 = split.FirstNode();
        if ( n1 == n2 &&
              n1->_intPoint &&
              (( n1->_intPoint->_faceIDs.size() > 1 && toCheckSideDivision ) ||
              ( n1->_isInternalFlags )))
        {
          // n1 is at intersection with EDGE
          if ( findChainOnEdge( splits, polygon->_links.back(), split, concaveFaces,
                                iS, quad, chainNodes ))
          {
            for ( size_t i = 1; i < chainNodes.size(); ++i )
              polygon->AddPolyLink( chainNodes[i-1], chainNodes[i], prevPolyg );
            if ( chainNodes.back() != n1 ) // not a partial cut by INTERNAL FACE
            {
              prevPolyg = polygon;
              n2 = chainNodes.back();
              continue;
            }
          }
        }
        else if ( n1 != n2 )
        {
          // try to connect to intersections with EDGEs
          if ( quad._eIntNodes.size() > nbUsedEdgeNodes  &&
                findChain( n2, n1, quad, chainNodes ))
          {
            for ( size_t i = 1; i < chainNodes.size(); ++i )
            {
              polygon->AddPolyLink( chainNodes[i-1], chainNodes[i] );
              nbUsedEdgeNodes += ( chainNodes[i]->IsUsedInFace( polygon ));
            }
            if ( chainNodes.back() != n1 )
            {
              n2 = chainNodes.back();
              --iS;
              continue;
            }
          }
          // try to connect to a split ending on the same FACE
          else
          {
            _OrientedLink foundSplit;
            for ( size_t i = iS; i < splits.size() && !foundSplit; ++i )
              if (( foundSplit = splits[ i ]) &&
                  ( n2->IsLinked( foundSplit.FirstNode()->_intPoint )))
              {
                iS = i - 1;
              }
              else
              {
                foundSplit._link = 0;
              }
            if ( foundSplit )
            {
              if ( n2 != foundSplit.FirstNode() )
              {
                polygon->AddPolyLink( n2, foundSplit.FirstNode() );
                n2 = foundSplit.FirstNode();
              }
              continue;
            }
            else
            {
              if ( n2->IsLinked( nFirst->_intPoint ))
                break;
              polygon->AddPolyLink( n2, n1, prevPolyg );
            }
          }
        } // if ( n1 != n2 )

        polygon->_links.push_back( split );
        split._link = 0;
        --nbSplits;
        n2 = polygon->_links.back().LastNode();

      } // loop on splits

      if ( nFirst != n2 ) // close a polygon
      {
        if ( !findChain( n2, nFirst, quad, chainNodes ))
        {
          if ( !closePolygon( polygon, chainNodes ))
            if ( !isImplementEdges() )
              chainNodes.push_back( nFirst );
        }
        for ( size_t i = 1; i < chainNodes.size(); ++i )
        {
          polygon->AddPolyLink( chainNodes[i-1], chainNodes[i], prevPolyg );
          nbUsedEdgeNodes += bool( chainNodes[i]->IsUsedInFace( polygon ));
        }
      }

      if ( polygon->_links.size() < 3 && nbSplits > 0 )
      {
        polygon->_polyLinks.clear();
        polygon->_links.clear();
      }
    } // while ( nbSplits > 0 )

    if ( polygon->_links.size() < 3 )
    {
      _polygons.pop_back();
    }
  }  //
}

//================================================================================
/*!
  * \brief Compute mesh volumes resulted from intersection of the Hexahedron
  */
bool Hexahedron::compute( const Solid* solid, const IsInternalFlag intFlag )
{
  _polygons.clear();
  _polygons.reserve( 20 );

  for ( int iN = 0; iN < 8; ++iN )
    _hexNodes[iN]._usedInFace = 0;

  if ( intFlag & IS_CUT_BY_INTERNAL_FACE && !_grid->_toAddEdges ) // Issue #19913
    preventVolumesOverlapping();

  std::set< TGeomID > concaveFaces; // to avoid connecting nodes laying on them

  if ( solid->HasConcaveVertex() )
  {
    for ( const E_IntersectPoint* ip : _eIntPoints )
    {
      if ( const ConcaveFace* cf = solid->GetConcave( ip->_shapeID ))
        if ( this->hasEdgesAround( cf ))
          concaveFaces.insert( cf->_concaveFace );
    }
    if ( concaveFaces.empty() || concaveFaces.size() * 3  < _eIntPoints.size() )
      for ( const _Node& hexNode: _hexNodes )
      {
        if ( hexNode._node && hexNode._intPoint && hexNode._intPoint->_faceIDs.size() >= 3 )
          if ( const ConcaveFace* cf = solid->GetConcave( hexNode._node->GetShapeID() ))
            if ( this->hasEdgesAround( cf ))
              concaveFaces.insert( cf->_concaveFace );
      }
  }

  // Create polygons from quadrangles
  // --------------------------------

  vector< _OrientedLink > splits;
  vector<_Node*>          chainNodes;
  _Face*                  coplanarPolyg;

  const bool hasEdgeIntersections = !_eIntPoints.empty();
  const bool toCheckSideDivision = isImplementEdges() || intFlag & IS_CUT_BY_INTERNAL_FACE;
  defineHexahedralFaces( splits, chainNodes, concaveFaces, toCheckSideDivision );

  // Create polygons closing holes in a polyhedron
  // ----------------------------------------------

  // clear _usedInFace
  for ( size_t iN = 0; iN < _intNodes.size(); ++iN )
    _intNodes[ iN ]._usedInFace = 0;

  // add polygons to their links and mark used nodes
  for ( size_t iP = 0; iP < _polygons.size(); ++iP )
  {
    _Face& polygon = _polygons[ iP ];
    for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
    {
      polygon._links[ iL ].AddFace( &polygon );
      polygon._links[ iL ].FirstNode()->_usedInFace = &polygon;
    }
  }
  // find free links
  vector< _OrientedLink* > freeLinks;
  freeLinks.reserve(20);
  for ( size_t iP = 0; iP < _polygons.size(); ++iP )
  {
    _Face& polygon = _polygons[ iP ];
    for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
      if ( polygon._links[ iL ].NbFaces() < 2 )
        freeLinks.push_back( & polygon._links[ iL ]);
  }
  int nbFreeLinks = freeLinks.size();
  if ( nbFreeLinks == 1 ) return false;

  // put not used intersection nodes to _vIntNodes
  int nbVertexNodes = 0; // nb not used vertex nodes
  {
    for ( size_t iN = 0; iN < _vIntNodes.size(); ++iN )
      nbVertexNodes += ( !_vIntNodes[ iN ]->IsUsedInFace() );

    const double tol = 1e-3 * Min( Min( _sideLength[0], _sideLength[1] ), _sideLength[0] );
    for ( size_t iN = _nbFaceIntNodes; iN < _intNodes.size(); ++iN )
    {
      if ( _intNodes[ iN ].IsUsedInFace() ) continue;
      if ( dynamic_cast< const F_IntersectPoint* >( _intNodes[ iN ]._intPoint )) continue;
      _Node* equalNode = findEqualNode( _vIntNodes, _intNodes[ iN ].EdgeIntPnt(), tol*tol );
      if ( !equalNode )
      {
        _vIntNodes.push_back( &_intNodes[ iN ]);
        ++nbVertexNodes;
      }
    }
  }

  std::set<TGeomID> usedFaceIDs;
  std::vector< TGeomID > faces;
  TGeomID curFace = 0;
  const size_t nbQuadPolygons = _polygons.size();
  E_IntersectPoint ipTmp;
  std::map< TGeomID, std::vector< const B_IntersectPoint* > > tmpAddedFace; // face added to _intPoint
  // std::cout << "2\n";
  // create polygons by making closed chains of free links
  size_t iPolygon = _polygons.size();
  while ( nbFreeLinks > 0 )
  {
    if ( iPolygon == _polygons.size() )
    {
      _polygons.resize( _polygons.size() + 1 );
      _polygons[ iPolygon ]._polyLinks.reserve( 20 );
      _polygons[ iPolygon ]._links.reserve( 20 );
    }
    _Face& polygon = _polygons[ iPolygon ];

    _OrientedLink* curLink = 0;
    _Node*         curNode;
    if (( !hasEdgeIntersections ) ||
        ( nbFreeLinks < 4 && nbVertexNodes == 0 ))
    {
      // get a remaining link to start from
      for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
        if (( curLink = freeLinks[ iL ] ))
          freeLinks[ iL ] = 0;
      polygon._links.push_back( *curLink );
      --nbFreeLinks;
      do
      {
        // find all links connected to curLink
        curNode = curLink->FirstNode();
        curLink = 0;
        for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
          if ( freeLinks[ iL ] && freeLinks[ iL ]->LastNode() == curNode )
          {
            curLink = freeLinks[ iL ];
            freeLinks[ iL ] = 0;
            --nbFreeLinks;
            polygon._links.push_back( *curLink );
          }
      } while ( curLink );
    }
    else // there are intersections with EDGEs
    {
      // get a remaining link to start from, one lying on minimal nb of FACEs
      {
        typedef pair< TGeomID, int > TFaceOfLink;
        TFaceOfLink faceOfLink( -1, -1 );
        TFaceOfLink facesOfLink[3] = { faceOfLink, faceOfLink, faceOfLink };
        for ( size_t iL = 0; iL < freeLinks.size(); ++iL )
          if ( freeLinks[ iL ] )
          {
            faces = freeLinks[ iL ]->GetNotUsedFace( usedFaceIDs );
            if ( faces.size() == 1 )
            {
              faceOfLink = TFaceOfLink( faces[0], iL );
              if ( !freeLinks[ iL ]->HasEdgeNodes() )
                break;
              facesOfLink[0] = faceOfLink;
            }
            else if ( facesOfLink[0].first < 0 )
            {
              faceOfLink = TFaceOfLink(( faces.empty() ? -1 : faces[0]), iL );
              facesOfLink[ 1 + faces.empty() ] = faceOfLink;
            }
          }
        for ( int i = 0; faceOfLink.first < 0 && i < 3; ++i )
          faceOfLink = facesOfLink[i];

        if ( faceOfLink.first < 0 ) // all faces used
        {
          for ( size_t iL = 0; iL < freeLinks.size() && faceOfLink.first < 1; ++iL )
            if (( curLink = freeLinks[ iL ]))
            {
              faceOfLink.first = 
                curLink->FirstNode()->IsLinked( curLink->LastNode()->_intPoint );
              faceOfLink.second = iL;
            }
          usedFaceIDs.clear();
        }
        curFace = faceOfLink.first;
        curLink = freeLinks[ faceOfLink.second ];
        freeLinks[ faceOfLink.second ] = 0;
      }
      usedFaceIDs.insert( curFace );
      polygon._links.push_back( *curLink );
      --nbFreeLinks;

      // find all links lying on a curFace
      do
      {
        // go forward from curLink
        curNode = curLink->LastNode();
        curLink = 0;
        for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
          if ( freeLinks[ iL ] &&
                freeLinks[ iL ]->FirstNode() == curNode &&
                freeLinks[ iL ]->LastNode()->IsOnFace( curFace ))
          {
            curLink = freeLinks[ iL ];
            freeLinks[ iL ] = 0;
            polygon._links.push_back( *curLink );
            --nbFreeLinks;
          }
      } while ( curLink );

      std::reverse( polygon._links.begin(), polygon._links.end() );

      curLink = & polygon._links.back();
      do
      {
        // go backward from curLink
        curNode = curLink->FirstNode();
        curLink = 0;
        for ( size_t iL = 0; iL < freeLinks.size() && !curLink; ++iL )
          if ( freeLinks[ iL ] &&
                freeLinks[ iL ]->LastNode() == curNode &&
                freeLinks[ iL ]->FirstNode()->IsOnFace( curFace ))
          {
            curLink = freeLinks[ iL ];
            freeLinks[ iL ] = 0;
            polygon._links.push_back( *curLink );
            --nbFreeLinks;
          }
      } while ( curLink );

      curNode = polygon._links.back().FirstNode();

      if ( polygon._links[0].LastNode() != curNode )
      {
        if ( nbVertexNodes > 0 )
        {
          // add links with _vIntNodes if not already used
          chainNodes.clear();
          for ( size_t iN = 0; iN < _vIntNodes.size(); ++iN )
            if ( !_vIntNodes[ iN ]->IsUsedInFace() &&
                  _vIntNodes[ iN ]->IsOnFace( curFace ))
            {
              _vIntNodes[ iN ]->_usedInFace = &polygon;
              chainNodes.push_back( _vIntNodes[ iN ] );
            }
          if ( chainNodes.size() > 1 &&
                curFace != _grid->PseudoIntExtFaceID() ) /////// TODO
          {
            sortVertexNodes( chainNodes, curNode, curFace );
          }
          for ( size_t i = 0; i < chainNodes.size(); ++i )
          {
            polygon.AddPolyLink( chainNodes[ i ], curNode );
            curNode = chainNodes[ i ];
            freeLinks.push_back( &polygon._links.back() );
            ++nbFreeLinks;
          }
          nbVertexNodes -= chainNodes.size();
        }
        // if ( polygon._links.size() > 1 )
        {
          polygon.AddPolyLink( polygon._links[0].LastNode(), curNode );
          freeLinks.push_back( &polygon._links.back() );
          ++nbFreeLinks;
        }
      }
    } // if there are intersections with EDGEs

    if ( polygon._links.size() < 2 ||
          polygon._links[0].LastNode() != polygon._links.back().FirstNode() )
    {
      _polygons.clear();
      break; // closed polygon not found -> invalid polyhedron
    }

    if ( polygon._links.size() == 2 )
    {
      if ( freeLinks.back() == &polygon._links.back() )
      {
        freeLinks.pop_back();
        --nbFreeLinks;
      }
      if ( polygon._links.front().NbFaces() > 0 )
        polygon._links.back().AddFace( polygon._links.front()._link->_faces[0] );
      if ( polygon._links.back().NbFaces() > 0 )
        polygon._links.front().AddFace( polygon._links.back()._link->_faces[0] );

      if ( iPolygon == _polygons.size()-1 )
        _polygons.pop_back();
    }
    else // polygon._links.size() >= 2
    {
      // add polygon to its links
      for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
      {
        polygon._links[ iL ].AddFace( &polygon );
        polygon._links[ iL ].Reverse();
      }
      if ( /*hasEdgeIntersections &&*/ iPolygon == _polygons.size() - 1 )
      {
        // check that a polygon does not lie on a hexa side
        coplanarPolyg = 0;
        for ( size_t iL = 0; iL < polygon._links.size() && !coplanarPolyg; ++iL )
        {
          if ( polygon._links[ iL ].NbFaces() < 2 )
            continue; // it's a just added free link
          // look for a polygon made on a hexa side and sharing
          // two or more haxa links
          size_t iL2;
          coplanarPolyg = polygon._links[ iL ]._link->_faces[0];
          for ( iL2 = iL + 1; iL2 < polygon._links.size(); ++iL2 )
            if ( polygon._links[ iL2 ]._link->_faces[0] == coplanarPolyg &&
                  !coplanarPolyg->IsPolyLink( polygon._links[ iL  ]) &&
                  !coplanarPolyg->IsPolyLink( polygon._links[ iL2 ]) &&
                  coplanarPolyg < & _polygons[ nbQuadPolygons ])
              break;
          if ( iL2 == polygon._links.size() )
            coplanarPolyg = 0;
        }
        if ( coplanarPolyg ) // coplanar polygon found
        {
          freeLinks.resize( freeLinks.size() - polygon._polyLinks.size() );
          nbFreeLinks -= polygon._polyLinks.size();

          ipTmp._faceIDs.resize(1);
          ipTmp._faceIDs[0] = curFace;

          // fill freeLinks with links not shared by coplanarPolyg and polygon
          for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
            if ( polygon._links[ iL ]._link->_faces[1] &&
                  polygon._links[ iL ]._link->_faces[0] != coplanarPolyg )
            {
              _Face* p = polygon._links[ iL ]._link->_faces[0];
              for ( size_t iL2 = 0; iL2 < p->_links.size(); ++iL2 )
                if ( p->_links[ iL2 ]._link == polygon._links[ iL ]._link )
                {
                  freeLinks.push_back( & p->_links[ iL2 ] );
                  ++nbFreeLinks;
                  freeLinks.back()->RemoveFace( &polygon );
                  break;
                }
            }
          for ( size_t iL = 0; iL < coplanarPolyg->_links.size(); ++iL )
            if ( coplanarPolyg->_links[ iL ]._link->_faces[1] &&
                  coplanarPolyg->_links[ iL ]._link->_faces[1] != &polygon )
            {
              _Face* p = coplanarPolyg->_links[ iL ]._link->_faces[0];
              if ( p == coplanarPolyg )
                p = coplanarPolyg->_links[ iL ]._link->_faces[1];
              for ( size_t iL2 = 0; iL2 < p->_links.size(); ++iL2 )
                if ( p->_links[ iL2 ]._link == coplanarPolyg->_links[ iL ]._link )
                {
                  // set links of coplanarPolyg in place of used freeLinks
                  // to re-create coplanarPolyg next
                  size_t iL3 = 0;
                  for ( ; iL3 < freeLinks.size() && freeLinks[ iL3 ]; ++iL3 );
                  if ( iL3 < freeLinks.size() )
                    freeLinks[ iL3 ] = ( & p->_links[ iL2 ] );
                  else
                    freeLinks.push_back( & p->_links[ iL2 ] );
                  ++nbFreeLinks;
                  freeLinks[ iL3 ]->RemoveFace( coplanarPolyg );
                  //  mark nodes of coplanarPolyg as lying on curFace
                  for ( int iN = 0; iN < 2; ++iN )
                  {
                    _Node* n = freeLinks[ iL3 ]->_link->_nodes[ iN ];
                    bool added = false;
                    if ( n->_intPoint ) added = n->_intPoint->Add( ipTmp._faceIDs );
                    else                        n->_intPoint = &ipTmp;
                    if ( added )
                      tmpAddedFace[ ipTmp._faceIDs[0] ].push_back( n->_intPoint );
                  }
                  break;
                }
            }
          // set coplanarPolyg to be re-created next
          for ( size_t iP = 0; iP < _polygons.size(); ++iP )
            if ( coplanarPolyg == & _polygons[ iP ] )
            {
              iPolygon = iP;
              _polygons[ iPolygon ]._links.clear();
              _polygons[ iPolygon ]._polyLinks.clear();
              break;
            }
          _polygons.pop_back();
          usedFaceIDs.erase( curFace );
          continue;
        } // if ( coplanarPolyg )
      } // if ( hasEdgeIntersections ) - search for coplanarPolyg

      iPolygon = _polygons.size();

    } // end of case ( polygon._links.size() > 2 )
  } // while ( nbFreeLinks > 0 )
  // std::cout << "3\n";
  for ( auto & face_ip : tmpAddedFace )
  {
    curFace = face_ip.first;
    for ( const B_IntersectPoint* ip : face_ip.second )
    {
      auto it = std::find( ip->_faceIDs.begin(), ip->_faceIDs.end(), curFace );
      if ( it != ip->_faceIDs.end() )
        ip->_faceIDs.erase( it );
    }
  }

  if ( _polygons.size() < 3 )
    return false;

  // check volume size
  double volSize = 0;
  _hasTooSmall = ! checkPolyhedronSize( intFlag & IS_CUT_BY_INTERNAL_FACE, volSize );

  for ( size_t i = 0; i < 8; ++i )
    if ( _hexNodes[ i ]._intPoint == &ipTmp )
      _hexNodes[ i ]._intPoint = 0;

  if ( _hasTooSmall )
    return false; // too small volume


  // Try to find out names of no-name polygons (issue # 19887)
  if ( _grid->IsToRemoveExcessEntities() && _polygons.back()._name == SMESH_Block::ID_NONE )
  {
    gp_XYZ uvwCenter = 0.5 * ( _grid->_coords[0][_i] + _grid->_coords[0][_i+1] ) * _grid->_axes[0] +
                       0.5 * ( _grid->_coords[1][_j] + _grid->_coords[1][_j+1] ) * _grid->_axes[1] +
                       0.5 * ( _grid->_coords[2][_k] + _grid->_coords[2][_k+1] ) * _grid->_axes[2];
                       
    for ( size_t i = _polygons.size() - 1; _polygons[i]._name == SMESH_Block::ID_NONE; --i )
    {
      _Face& face = _polygons[ i ];
      Bnd_Box bb;
      gp_Pnt uvw;
      for ( size_t iL = 0; iL < face._links.size(); ++iL )
      {
        _Node* n = face._links[ iL ].FirstNode();
        if ( n )
        {
          gp_XYZ p = SMESH_NodeXYZ( n->Node() );
          _grid->ComputeUVW( p, uvw.ChangeCoord().ChangeData() );
          bb.Add( uvw );  
        }
      }
      gp_Pnt pMin = bb.CornerMin();
      if ( bb.IsXThin( _grid->_tol ))
        face._name = pMin.X() < uvwCenter.X() ? SMESH_Block::ID_F0yz : SMESH_Block::ID_F1yz;
      else if ( bb.IsYThin( _grid->_tol ))
        face._name = pMin.Y() < uvwCenter.Y() ? SMESH_Block::ID_Fx0z : SMESH_Block::ID_Fx1z;
      else if ( bb.IsZThin( _grid->_tol ))
        face._name = pMin.Z() < uvwCenter.Z() ? SMESH_Block::ID_Fxy0 : SMESH_Block::ID_Fxy1;
    }
  }


  /* This call is irrelevant here because _volumeDefs datas are were not filled! 
  or .... it is potentialy filled by other thread?? */
  _volumeDefs._nodes.clear();
  _volumeDefs._quantities.clear();
  _volumeDefs._names.clear();
  // create a classic cell if possible

  int nbPolygons = 0;
  for ( size_t iF = 0; iF < _polygons.size(); ++iF )
    nbPolygons += (_polygons[ iF ]._links.size() > 2 );

  //const int nbNodes = _nbCornerNodes + nbIntersections;
  int nbNodes = 0;
  for ( size_t i = 0; i < 8; ++i )
    nbNodes += _hexNodes[ i ].IsUsedInFace();
  for ( size_t i = 0; i < _intNodes.size(); ++i )
    nbNodes += _intNodes[ i ].IsUsedInFace();

  bool isClassicElem = false;
  if (      nbNodes == 8 && nbPolygons == 6 ) isClassicElem = addHexa();
  else if ( nbNodes == 4 && nbPolygons == 4 ) isClassicElem = addTetra();
  else if ( nbNodes == 6 && nbPolygons == 5 ) isClassicElem = addPenta();
  else if ( nbNodes == 5 && nbPolygons == 5 ) isClassicElem = addPyra ();
  if ( !isClassicElem )
  {
    for ( size_t iF = 0; iF < _polygons.size(); ++iF )
    {
      const size_t nbLinks = _polygons[ iF ]._links.size();
      if ( nbLinks < 3 ) continue;
      _volumeDefs._quantities.push_back( nbLinks );
      _volumeDefs._names.push_back( _polygons[ iF ]._name );
      for ( size_t iL = 0; iL < nbLinks; ++iL )
        _volumeDefs._nodes.push_back( _polygons[ iF ]._links[ iL ].FirstNode() );
    }
  }
  _volumeDefs._solidID = solid->ID();
  _volumeDefs._size    = volSize;

  return !_volumeDefs._nodes.empty();
}

template<typename Type>
void computeHexa(Type& hex)
{
  if ( hex ) 
    hex->computeElements();
}

//================================================================================
/*!
  * \brief Create elements in the mesh
  */
int Hexahedron::MakeElements(SMESH_MesherHelper&                      helper,
                              const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap, 
                                const int numOfThreads )
{
  SMESHDS_Mesh* mesh = helper.GetMeshDS();

  CellsAroundLink c( _grid, 0 );
  const size_t nbGridCells = c._nbCells[0] * c._nbCells[1] * c._nbCells[2];
  vector< Hexahedron* > allHexa( nbGridCells, 0 );
  int nbIntHex = 0;

  // set intersection nodes from GridLine's to links of allHexa
  int i,j,k, cellIndex, iLink;
  for ( int iDir = 0; iDir < 3; ++iDir )
  {
    // loop on GridLine's parallel to iDir
    LineIndexer lineInd = _grid->GetLineIndexer( iDir );
    CellsAroundLink fourCells( _grid, iDir );
    for ( ; lineInd.More(); ++lineInd )
    {
      GridLine& line = _grid->_lines[ iDir ][ lineInd.LineIndex() ];
      multiset< F_IntersectPoint >::const_iterator ip = line._intPoints.begin();
      for ( ; ip != line._intPoints.end(); ++ip )
      {
        // if ( !ip->_node ) continue; // intersection at a grid node
        lineInd.SetIndexOnLine( ip->_indexOnLine );
        fourCells.Init( lineInd.I(), lineInd.J(), lineInd.K() );
        for ( int iL = 0; iL < 4; ++iL ) // loop on 4 cells sharing a link
        {
          if ( !fourCells.GetCell( iL, i,j,k, cellIndex, iLink ))
            continue;
          Hexahedron *& hex = allHexa[ cellIndex ];
          if ( !hex)
          {
            hex = new Hexahedron( *this, i, j, k, cellIndex );
            ++nbIntHex;
          }
          hex->_hexLinks[iLink]._fIntPoints.push_back( &(*ip) );
          hex->_nbFaceIntNodes += bool( ip->_node );
        }
      }
    }
  }

  // implement geom edges into the mesh
  addEdges( helper, allHexa, edge2faceIDsMap );

  // add not split hexahedra to the mesh
  int nbAdded = 0;
  TGeomID solidIDs[20];
  vector< Hexahedron* > intHexa; intHexa.reserve( nbIntHex );
  vector< const SMDS_MeshElement* > boundaryVolumes; boundaryVolumes.reserve( nbIntHex * 1.1 );
  
  for ( size_t i = 0; i < allHexa.size(); ++i )
  {
    // initialize this by not cut allHexa[ i ]
    Hexahedron * & hex = allHexa[ i ];
    if ( hex ) // split hexahedron
    {
      intHexa.push_back( hex );
      if ( hex->_nbFaceIntNodes > 0 ||
            hex->_eIntPoints.size() > 0 ||
            hex->getSolids( solidIDs ) > 1 )
        continue; // treat intersected hex later in parallel
      this->init( hex->_i, hex->_j, hex->_k );
    }
    else
    {
      this->init( i ); // == init(i,j,k)
    }
    if (( _nbCornerNodes == 8 ) &&
        ( _nbBndNodes < _nbCornerNodes || !isInHole() ))
    {
      // order of _hexNodes is defined by enum SMESH_Block::TShapeID
      SMDS_MeshElement* el =
        mesh->AddVolume( _hexNodes[0].Node(), _hexNodes[2].Node(),
                          _hexNodes[3].Node(), _hexNodes[1].Node(),
                          _hexNodes[4].Node(), _hexNodes[6].Node(),
                          _hexNodes[7].Node(), _hexNodes[5].Node() );
      TGeomID solidID = 0;
      if ( _nbBndNodes < _nbCornerNodes )
      {
        for ( int iN = 0; iN < 8 &&  !solidID; ++iN )
          if ( !_hexNodes[iN]._intPoint ) // no intersection
            solidID = _hexNodes[iN].Node()->GetShapeID();
      }
      else
      {
        getSolids( solidIDs );
        solidID = solidIDs[0];
      }
      mesh->SetMeshElementOnShape( el, solidID );
      ++nbAdded;
      if ( hex )
        intHexa.pop_back();
      if ( _grid->_toCreateFaces && _nbBndNodes >= 3 )
      {
        boundaryVolumes.push_back( el );
        el->setIsMarked( true );
      }
    }
    else if ( _nbCornerNodes > 3 && !hex )
    {
      // all intersections of hex with geometry are at grid nodes
      hex = new Hexahedron( *this, _i, _j, _k, i );
      intHexa.push_back( hex );
    }
  }
  
  // compute definitions of volumes resulted from hexadron intersection
#ifdef WITH_TBB
  parallel_for(intHexa.begin(), intHexa.end(), computeHexa<Hexahedron*>, numOfThreads ); 
#else
  for ( size_t i = 0; i < intHexa.size(); ++i )
    if ( Hexahedron * hex = intHexa[ i ] )
      hex->computeElements();
#endif

  // simplify polyhedrons
  if ( _grid->IsToRemoveExcessEntities() )
  {
    for ( size_t i = 0; i < intHexa.size(); ++i )
      if ( Hexahedron * hex = intHexa[ i ] )
        hex->removeExcessSideDivision( allHexa );

    for ( size_t i = 0; i < intHexa.size(); ++i )
      if ( Hexahedron * hex = intHexa[ i ] )
        hex->removeExcessNodes( allHexa );
  }

  // add volumes
  for ( size_t i = 0; i < intHexa.size(); ++i )
    if ( Hexahedron * hex = intHexa[ i ] )
      nbAdded += hex->addVolumes( helper );
  
  // fill boundaryVolumes with volumes neighboring too small skipped volumes
  if ( _grid->_toCreateFaces )
  {
    for ( size_t i = 0; i < intHexa.size(); ++i )
      if ( Hexahedron * hex = intHexa[ i ] )
        hex->getBoundaryElems( boundaryVolumes );
  }

  // merge nodes on outer sub-shapes with pre-existing ones
  TopTools_DataMapIteratorOfDataMapOfShapeInteger s2nIt( _grid->_geometry._shape2NbNodes );
  for ( ; s2nIt.More(); s2nIt.Next() )
    if ( s2nIt.Value() > 0 )
      if ( SMESHDS_SubMesh* sm = mesh->MeshElements( s2nIt.Key() ))
      {
        TIDSortedNodeSet smNodes( SMDS_MeshElement::iterator( sm->GetNodes() ),
                                  SMDS_MeshElement::iterator() );
        SMESH_MeshEditor::TListOfListOfNodes equalNodes;
        SMESH_MeshEditor editor( helper.GetMesh() );
        editor.FindCoincidentNodes( smNodes, 10 * _grid->_tol, equalNodes,
                                    /*SeparateCornersAndMedium =*/ false);
        if ((int) equalNodes.size() <= s2nIt.Value() )
          editor.MergeNodes( equalNodes );
      }

  // create boundary mesh faces
  addFaces( helper, boundaryVolumes );

  // create mesh edges
  addSegments( helper, edge2faceIDsMap );

  for ( size_t i = 0; i < allHexa.size(); ++i )
    if ( allHexa[ i ] )
      delete allHexa[ i ];

  return nbAdded;
}

//================================================================================
/*!
  * \brief Implements geom edges into the mesh
  */
void Hexahedron::addEdges(SMESH_MesherHelper&                      helper,
                          vector< Hexahedron* >&                   hexes,
                          const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap)
{
  if ( edge2faceIDsMap.empty() ) return;

  // Prepare planes for intersecting with EDGEs
  GridPlanes pln[3];
  {
    for ( int iDirZ = 0; iDirZ < 3; ++iDirZ ) // iDirZ gives normal direction to planes
    {
      GridPlanes& planes = pln[ iDirZ ];
      int iDirX = ( iDirZ + 1 ) % 3;
      int iDirY = ( iDirZ + 2 ) % 3;
      planes._zNorm  = ( _grid->_axes[ iDirX ] ^ _grid->_axes[ iDirY ] ).Normalized();
      planes._zProjs.resize ( _grid->_coords[ iDirZ ].size() );
      planes._zProjs [0] = 0;
      const double       zFactor = _grid->_axes[ iDirZ ] * planes._zNorm;
      const vector< double > & u = _grid->_coords[ iDirZ ];
      for ( size_t i = 1; i < planes._zProjs.size(); ++i )
      {
        planes._zProjs [i] = zFactor * ( u[i] - u[0] );
      }
    }
  }
  const double deflection = _grid->_minCellSize / 20.;
  const double tol        = _grid->_tol;
  E_IntersectPoint ip;

  TColStd_MapOfInteger intEdgeIDs; // IDs of not shared INTERNAL EDGES

  // Intersect EDGEs with the planes
  map< TGeomID, vector< TGeomID > >::const_iterator e2fIt = edge2faceIDsMap.begin();
  for ( ; e2fIt != edge2faceIDsMap.end(); ++e2fIt )
  {
    const TGeomID  edgeID = e2fIt->first;
    const TopoDS_Edge & E = TopoDS::Edge( _grid->Shape( edgeID ));
    BRepAdaptor_Curve curve( E );
    TopoDS_Vertex v1 = helper.IthVertex( 0, E, false );
    TopoDS_Vertex v2 = helper.IthVertex( 1, E, false );

    ip._faceIDs = e2fIt->second;
    ip._shapeID = edgeID;

    bool isInternal = ( ip._faceIDs.size() == 1 && _grid->IsInternal( edgeID ));
    if ( isInternal )
    {
      intEdgeIDs.Add( edgeID );
      intEdgeIDs.Add( _grid->ShapeID( v1 ));
      intEdgeIDs.Add( _grid->ShapeID( v2 ));
    }

    // discretize the EDGE
    GCPnts_UniformDeflection discret( curve, deflection, true );
    if ( !discret.IsDone() || discret.NbPoints() < 2 )
      continue;

    // perform intersection
    E_IntersectPoint* eip, *vip = 0;
    for ( int iDirZ = 0; iDirZ < 3; ++iDirZ )
    {
      GridPlanes& planes = pln[ iDirZ ];
      int      iDirX = ( iDirZ + 1 ) % 3;
      int      iDirY = ( iDirZ + 2 ) % 3;
      double    xLen = _grid->_coords[ iDirX ].back() - _grid->_coords[ iDirX ][0];
      double    yLen = _grid->_coords[ iDirY ].back() - _grid->_coords[ iDirY ][0];
      double    zLen = _grid->_coords[ iDirZ ].back() - _grid->_coords[ iDirZ ][0];
      int dIJK[3], d000[3] = { 0,0,0 };
      double o[3] = { _grid->_coords[0][0],
                      _grid->_coords[1][0],
                      _grid->_coords[2][0] };

      // locate the 1st point of a segment within the grid
      gp_XYZ p1     = discret.Value( 1 ).XYZ();
      double u1     = discret.Parameter( 1 );
      double zProj1 = planes._zNorm * ( p1 - _grid->_origin );

      _grid->ComputeUVW( p1, ip._uvw );
      int iX1 = int(( ip._uvw[iDirX] - o[iDirX]) / xLen * (_grid->_coords[ iDirX ].size() - 1));
      int iY1 = int(( ip._uvw[iDirY] - o[iDirY]) / yLen * (_grid->_coords[ iDirY ].size() - 1));
      int iZ1 = int(( ip._uvw[iDirZ] - o[iDirZ]) / zLen * (_grid->_coords[ iDirZ ].size() - 1));
      locateValue( iX1, ip._uvw[iDirX], _grid->_coords[ iDirX ], dIJK[ iDirX ], tol );
      locateValue( iY1, ip._uvw[iDirY], _grid->_coords[ iDirY ], dIJK[ iDirY ], tol );
      locateValue( iZ1, ip._uvw[iDirZ], _grid->_coords[ iDirZ ], dIJK[ iDirZ ], tol );

      int ijk[3]; // grid index where a segment intersects a plane
      ijk[ iDirX ] = iX1;
      ijk[ iDirY ] = iY1;
      ijk[ iDirZ ] = iZ1;

      // add the 1st vertex point to a hexahedron
      if ( iDirZ == 0 )
      {
        ip._point   = p1;
        ip._shapeID = _grid->ShapeID( v1 );
        vip = _grid->Add( ip );
        _grid->UpdateFacesOfVertex( *vip, v1 );
        if ( isInternal )
          vip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
        if ( !addIntersection( vip, hexes, ijk, d000 ))
          _grid->Remove( vip );
        ip._shapeID = edgeID;
      }
      for ( int iP = 2; iP <= discret.NbPoints(); ++iP )
      {
        // locate the 2nd point of a segment within the grid
        gp_XYZ p2     = discret.Value( iP ).XYZ();
        double u2     = discret.Parameter( iP );
        double zProj2 = planes._zNorm * ( p2 - _grid->_origin );
        int    iZ2    = iZ1;
        if ( Abs( zProj2 - zProj1 ) > std::numeric_limits<double>::min() )
        {
          locateValue( iZ2, zProj2, planes._zProjs, dIJK[ iDirZ ], tol );

          // treat intersections with planes between 2 end points of a segment
          int dZ = ( iZ1 <= iZ2 ) ? +1 : -1;
          int iZ = iZ1 + ( iZ1 < iZ2 );
          for ( int i = 0, nb = Abs( iZ1 - iZ2 ); i < nb; ++i, iZ += dZ )
          {
            ip._point = findIntPoint( u1, zProj1, u2, zProj2,
                                      planes._zProjs[ iZ ],
                                      curve, planes._zNorm, _grid->_origin );
            _grid->ComputeUVW( ip._point.XYZ(), ip._uvw );
            locateValue( ijk[iDirX], ip._uvw[iDirX], _grid->_coords[iDirX], dIJK[iDirX], tol );
            locateValue( ijk[iDirY], ip._uvw[iDirY], _grid->_coords[iDirY], dIJK[iDirY], tol );
            ijk[ iDirZ ] = iZ;

            // add ip to hex "above" the plane
            eip = _grid->Add( ip );
            if ( isInternal )
              eip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
            dIJK[ iDirZ ] = 0;
            bool added = addIntersection( eip, hexes, ijk, dIJK);

            // add ip to hex "below" the plane
            ijk[ iDirZ ] = iZ-1;
            if ( !addIntersection( eip, hexes, ijk, dIJK ) &&
                  !added )
              _grid->Remove( eip );
          }
        }
        iZ1    = iZ2;
        p1     = p2;
        u1     = u2;
        zProj1 = zProj2;
      }
      // add the 2nd vertex point to a hexahedron
      if ( iDirZ == 0 )
      {
        ip._point   = p1;
        ip._shapeID = _grid->ShapeID( v2 );
        _grid->ComputeUVW( p1, ip._uvw );
        locateValue( ijk[iDirX], ip._uvw[iDirX], _grid->_coords[iDirX], dIJK[iDirX], tol );
        locateValue( ijk[iDirY], ip._uvw[iDirY], _grid->_coords[iDirY], dIJK[iDirY], tol );
        ijk[ iDirZ ] = iZ1;
        bool sameV = ( v1.IsSame( v2 ));
        if ( !sameV )
        {
          vip = _grid->Add( ip );
          _grid->UpdateFacesOfVertex( *vip, v2 );
          if ( isInternal )
            vip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
        }
        if ( !addIntersection( vip, hexes, ijk, d000 ) && !sameV )
          _grid->Remove( vip );
        ip._shapeID = edgeID;
      }
    } // loop on 3 grid directions
  } // loop on EDGEs


  if ( intEdgeIDs.Size() > 0 )
    cutByExtendedInternal( hexes, intEdgeIDs );

  return;
}

//================================================================================
/*!
  * \brief Fully cut hexes that are partially cut by INTERNAL FACE.
  *        Cut them by extended INTERNAL FACE.
  */
void Hexahedron::cutByExtendedInternal( std::vector< Hexahedron* >& hexes,
                                        const TColStd_MapOfInteger& intEdgeIDs )
{
  IntAna_IntConicQuad intersection;
  SMESHDS_Mesh* meshDS = _grid->_helper->GetMeshDS();
  const double tol2 = _grid->_tol * _grid->_tol;

  for ( size_t iH = 0; iH < hexes.size(); ++iH )
  {
    Hexahedron* hex = hexes[ iH ];
    if ( !hex || hex->_eIntPoints.size() < 2 )
      continue;
    if ( !intEdgeIDs.Contains( hex->_eIntPoints.back()->_shapeID ))
      continue;

    // get 3 points on INTERNAL FACE to construct a cutting plane
    gp_Pnt p1 = hex->_eIntPoints[0]->_point;
    gp_Pnt p2 = hex->_eIntPoints[1]->_point;
    gp_Pnt p3 = hex->mostDistantInternalPnt( iH, p1, p2 );

    gp_Vec norm = gp_Vec( p1, p2 ) ^ gp_Vec( p1, p3 );
    gp_Pln pln;
    try {
      pln = gp_Pln( p1, norm );
    }
    catch(...)
    {
      continue;
    }

    TGeomID intFaceID = hex->_eIntPoints.back()->_faceIDs.front(); // FACE being "extended"
    TGeomID   solidID = _grid->GetSolid( intFaceID )->ID();

    // cut links by the plane
    //bool isCut = false;
    for ( int iLink = 0; iLink < 12; ++iLink )
    {
      _Link& link = hex->_hexLinks[ iLink ];
      if ( !link._fIntPoints.empty() )
      {
        // if ( link._fIntPoints[0]->_faceIDs.back() == _grid->PseudoIntExtFaceID() )
        //   isCut = true;
        continue; // already cut link
      }
      if ( !link._nodes[0]->Node() ||
            !link._nodes[1]->Node() )
        continue; // outside link

      if ( link._nodes[0]->IsOnFace( intFaceID ))
      {
        if ( link._nodes[0]->_intPoint->_faceIDs.back() != _grid->PseudoIntExtFaceID() )
          if ( p1.SquareDistance( link._nodes[0]->Point() ) < tol2  ||
                p2.SquareDistance( link._nodes[0]->Point() ) < tol2 )
            link._nodes[0]->_intPoint->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
        continue; // link is cut by FACE being "extended"
      }
      if ( link._nodes[1]->IsOnFace( intFaceID ))
      {
        if ( link._nodes[1]->_intPoint->_faceIDs.back() != _grid->PseudoIntExtFaceID() )
          if ( p1.SquareDistance( link._nodes[1]->Point() ) < tol2  ||
                p2.SquareDistance( link._nodes[1]->Point() ) < tol2 )
            link._nodes[1]->_intPoint->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
        continue; // link is cut by FACE being "extended"
      }
      gp_Pnt p4 = link._nodes[0]->Point();
      gp_Pnt p5 = link._nodes[1]->Point();
      gp_Lin line( p4, gp_Vec( p4, p5 ));

      intersection.Perform( line, pln );
      if ( !intersection.IsDone() ||
            intersection.IsInQuadric() ||
            intersection.IsParallel() ||
            intersection.NbPoints() < 1 )
        continue;

      double u = intersection.ParamOnConic(1);
      if ( u + _grid->_tol < 0 )
        continue;
      int       iDir = iLink / 4;
      int      index = (&hex->_i)[iDir];
      double linkLen = _grid->_coords[iDir][index+1] - _grid->_coords[iDir][index];
      if ( u - _grid->_tol > linkLen )
        continue;

      if ( u < _grid->_tol ||
            u > linkLen - _grid->_tol ) // intersection at grid node
      {
        int  i = ! ( u < _grid->_tol ); // [0,1]
        int iN = link._nodes[ i ] - hex->_hexNodes; // [0-7]

        const F_IntersectPoint * & ip = _grid->_gridIntP[ hex->_origNodeInd +
                                                          _grid->_nodeShift[iN] ];
        if ( !ip )
        {
          ip = _grid->_extIntPool.getNew();
          ip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
          //ip->_transition = Trans_INTERNAL;
        }
        else if ( ip->_faceIDs.back() != _grid->PseudoIntExtFaceID() )
        {
          ip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
        }
        hex->_nbFaceIntNodes++;
        //isCut = true;
      }
      else
      {
        const gp_Pnt&      p = intersection.Point( 1 );
        F_IntersectPoint* ip = _grid->_extIntPool.getNew();
        ip->_node = meshDS->AddNode( p.X(), p.Y(), p.Z() );
        ip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
        ip->_transition = Trans_INTERNAL;
        meshDS->SetNodeInVolume( ip->_node, solidID );

        CellsAroundLink fourCells( _grid, iDir );
        fourCells.Init( hex->_i, hex->_j, hex->_k, iLink );
        int i,j,k, cellIndex;
        for ( int iC = 0; iC < 4; ++iC ) // loop on 4 cells sharing the link
        {
          if ( !fourCells.GetCell( iC, i,j,k, cellIndex, iLink ))
            continue;
          Hexahedron * h = hexes[ cellIndex ];
          if ( !h )
            h = hexes[ cellIndex ] = new Hexahedron( *this, i, j, k, cellIndex );
          h->_hexLinks[iLink]._fIntPoints.push_back( ip );
          h->_nbFaceIntNodes++;
          //isCut = true;
        }
      }
    }

    // if ( isCut )
    //   for ( size_t i = 0; i < hex->_eIntPoints.size(); ++i )
    //   {
    //     if ( _grid->IsInternal( hex->_eIntPoints[i]->_shapeID ) &&
    //          ! hex->_eIntPoints[i]->IsOnFace( _grid->PseudoIntExtFaceID() ))
    //       hex->_eIntPoints[i]->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
    //   }
    continue;

  } // loop on all hexes
  return;
}

//================================================================================
/*!
  * \brief Return intersection point on INTERNAL FACE most distant from given ones
  */
gp_Pnt Hexahedron::mostDistantInternalPnt( int hexIndex, const gp_Pnt& p1, const gp_Pnt& p2 )
{
  gp_Pnt resultPnt = p1;

  double maxDist2 = 0;
  for ( int iLink = 0; iLink < 12; ++iLink ) // check links
  {
    _Link& link = _hexLinks[ iLink ];
    for ( size_t i = 0; i < link._fIntPoints.size(); ++i )
      if ( _grid->PseudoIntExtFaceID() != link._fIntPoints[i]->_faceIDs[0] &&
            _grid->IsInternal( link._fIntPoints[i]->_faceIDs[0] ) &&
            link._fIntPoints[i]->_node )
      {
        gp_Pnt p = SMESH_NodeXYZ( link._fIntPoints[i]->_node );
        double d = p1.SquareDistance( p );
        if ( d > maxDist2 )
        {
          resultPnt = p;
          maxDist2  = d;
        }
        else
        {
          d = p2.SquareDistance( p );
          if ( d > maxDist2 )
          {
            resultPnt = p;
            maxDist2  = d;
          }
        }
      }
  }
  setIJK( hexIndex );
  _origNodeInd = _grid->NodeIndex( _i,_j,_k );

  for ( size_t iN = 0; iN < 8; ++iN ) // check corners
  {
    _hexNodes[iN]._node     = _grid->_nodes   [ _origNodeInd + _grid->_nodeShift[iN] ];
    _hexNodes[iN]._intPoint = _grid->_gridIntP[ _origNodeInd + _grid->_nodeShift[iN] ];
    if ( _hexNodes[iN]._intPoint )
      for ( size_t iF = 0; iF < _hexNodes[iN]._intPoint->_faceIDs.size(); ++iF )
      {
        if ( _grid->IsInternal( _hexNodes[iN]._intPoint->_faceIDs[iF]))
        {
          gp_Pnt p = SMESH_NodeXYZ( _hexNodes[iN]._node );
          double d = p1.SquareDistance( p );
          if ( d > maxDist2 )
          {
            resultPnt = p;
            maxDist2  = d;
          }
          else
          {
            d = p2.SquareDistance( p );
            if ( d > maxDist2 )
            {
              resultPnt = p;
              maxDist2  = d;
            }
          }
        }
      }
  }
  if ( maxDist2 < _grid->_tol * _grid->_tol )
    return p1;

  return resultPnt;
}

//================================================================================
/*!
  * \brief Finds intersection of a curve with a plane
  *  \param [in] u1 - parameter of one curve point
  *  \param [in] proj1 - projection of the curve point to the plane normal
  *  \param [in] u2 - parameter of another curve point
  *  \param [in] proj2 - projection of the other curve point to the plane normal
  *  \param [in] proj - projection of a point where the curve intersects the plane
  *  \param [in] curve - the curve
  *  \param [in] axis - the plane normal
  *  \param [in] origin - the plane origin
  *  \return gp_Pnt - the found intersection point
  */
gp_Pnt Hexahedron::findIntPoint( double u1, double proj1,
                                  double u2, double proj2,
                                  double proj,
                                  BRepAdaptor_Curve& curve,
                                  const gp_XYZ& axis,
                                  const gp_XYZ& origin)
{
  double r = (( proj - proj1 ) / ( proj2 - proj1 ));
  double u = u1 * ( 1 - r ) + u2 * r;
  gp_Pnt p = curve.Value( u );
  double newProj =  axis * ( p.XYZ() - origin );
  if ( Abs( proj - newProj ) > _grid->_tol / 10. )
  {
    if ( r > 0.5 )
      return findIntPoint( u2, proj2, u, newProj, proj, curve, axis, origin );
    else
      return findIntPoint( u1, proj2, u, newProj, proj, curve, axis, origin );
  }
  return p;
}

//================================================================================
/*!
  * \brief Returns indices of a hexahedron sub-entities holding a point
  *  \param [in] ip - intersection point
  *  \param [out] facets - 0-3 facets holding a point
  *  \param [out] sub - index of a vertex or an edge holding a point
  *  \return int - number of facets holding a point
  */
int Hexahedron::getEntity( const E_IntersectPoint* ip, int* facets, int& sub )
{
  enum { X = 1, Y = 2, Z = 4 }; // == 001, 010, 100
  int nbFacets = 0;
  int vertex = 0, edgeMask = 0;

  if ( Abs( _grid->_coords[0][ _i   ] - ip->_uvw[0] ) < _grid->_tol ) {
    facets[ nbFacets++ ] = SMESH_Block::ID_F0yz;
    edgeMask |= X;
  }
  else if ( Abs( _grid->_coords[0][ _i+1 ] - ip->_uvw[0] ) < _grid->_tol ) {
    facets[ nbFacets++ ] = SMESH_Block::ID_F1yz;
    vertex   |= X;
    edgeMask |= X;
  }
  if ( Abs( _grid->_coords[1][ _j   ] - ip->_uvw[1] ) < _grid->_tol ) {
    facets[ nbFacets++ ] = SMESH_Block::ID_Fx0z;
    edgeMask |= Y;
  }
  else if ( Abs( _grid->_coords[1][ _j+1 ] - ip->_uvw[1] ) < _grid->_tol ) {
    facets[ nbFacets++ ] = SMESH_Block::ID_Fx1z;
    vertex   |= Y;
    edgeMask |= Y;
  }
  if ( Abs( _grid->_coords[2][ _k   ] - ip->_uvw[2] ) < _grid->_tol ) {
    facets[ nbFacets++ ] = SMESH_Block::ID_Fxy0;
    edgeMask |= Z;
  }
  else if ( Abs( _grid->_coords[2][ _k+1 ] - ip->_uvw[2] ) < _grid->_tol ) {
    facets[ nbFacets++ ] = SMESH_Block::ID_Fxy1;
    vertex   |= Z;
    edgeMask |= Z;
  }

  switch ( nbFacets )
  {
  case 0: sub = 0;         break;
  case 1: sub = facets[0]; break;
  case 2: {
    const int edge [3][8] = {
      { SMESH_Block::ID_E00z, SMESH_Block::ID_E10z,
        SMESH_Block::ID_E01z, SMESH_Block::ID_E11z },
      { SMESH_Block::ID_E0y0, SMESH_Block::ID_E1y0, 0, 0,
        SMESH_Block::ID_E0y1, SMESH_Block::ID_E1y1 },
      { SMESH_Block::ID_Ex00, 0, SMESH_Block::ID_Ex10, 0,
        SMESH_Block::ID_Ex01, 0, SMESH_Block::ID_Ex11 }
    };
    switch ( edgeMask ) {
    case X | Y: sub = edge[ 0 ][ vertex ]; break;
    case X | Z: sub = edge[ 1 ][ vertex ]; break;
    default:    sub = edge[ 2 ][ vertex ];
    }
    break;
  }
  //case 3:
  default:
    sub = vertex + SMESH_Block::ID_FirstV;
  }

  return nbFacets;
}
//================================================================================
/*!
  * \brief Adds intersection with an EDGE
  */
bool Hexahedron::addIntersection( const E_IntersectPoint* ip,
                                  vector< Hexahedron* >&  hexes,
                                  int ijk[], int dIJK[] )
{
  bool added = false;

  size_t hexIndex[4] = {
    _grid->CellIndex( ijk[0], ijk[1], ijk[2] ),
    dIJK[0] ? _grid->CellIndex( ijk[0]+dIJK[0], ijk[1], ijk[2] ) : -1,
    dIJK[1] ? _grid->CellIndex( ijk[0], ijk[1]+dIJK[1], ijk[2] ) : -1,
    dIJK[2] ? _grid->CellIndex( ijk[0], ijk[1], ijk[2]+dIJK[2] ) : -1
  };
  for ( int i = 0; i < 4; ++i )
  {
    if ( hexIndex[i] < hexes.size() && hexes[ hexIndex[i] ] )
    {
      Hexahedron* h = hexes[ hexIndex[i] ];
      h->_eIntPoints.reserve(2);
      h->_eIntPoints.push_back( ip );
      added = true;

      // check if ip is really inside the hex
      if (SALOME::VerbosityActivated() && h->isOutParam( ip->_uvw ))
        throw SALOME_Exception("ip outside a hex");
    }
  }
  return added;
}
//================================================================================
/*!
  * \brief Check if a hexahedron facet lies on a FACE
  *        Also return true if the facet does not interfere with any FACE
  */
bool Hexahedron::isQuadOnFace( const size_t iQuad )
{
  _Face& quad = _hexQuads[ iQuad ] ;

  int nbGridNodesInt = 0; // nb FACE intersections at grid nodes
  int nbNoGeomNodes  = 0;
  for ( int iE = 0; iE < 4; ++iE )
  {
    nbNoGeomNodes = ( !quad._links[ iE ].FirstNode()->_intPoint &&
                      quad._links[ iE ].NbResultLinks() == 1      );
    nbGridNodesInt +=
      ( quad._links[ iE ].FirstNode()->_intPoint &&
        quad._links[ iE ].NbResultLinks() == 1   &&
        quad._links[ iE ].ResultLink( 0 ).FirstNode() == quad._links[ iE ].FirstNode() &&
        quad._links[ iE ].ResultLink( 0 ).LastNode()  == quad._links[ iE ].LastNode()   );
  }
  if ( nbNoGeomNodes == 4 )
    return true;

  if ( nbGridNodesInt == 4 ) // all quad nodes are at FACE intersection
  {
    size_t iEmin = 0, minNbFaces = 1000;
    for ( int iE = 0; iE < 4; ++iE ) // look for a node with min nb FACEs
    {
      size_t nbFaces = quad._links[ iE ].FirstNode()->faces().size();
      if ( minNbFaces > nbFaces )
      {
        iEmin = iE;
        minNbFaces = nbFaces;
      }
    }
    // check if there is a FACE passing through all 4 nodes
    for ( const TGeomID& faceID : quad._links[ iEmin ].FirstNode()->faces() )
    {
      bool allNodesAtFace = true;
      for ( size_t iE = 0; iE < 4 &&  allNodesAtFace; ++iE )
        allNodesAtFace = ( iE == iEmin ||
                            quad._links[ iE ].FirstNode()->IsOnFace( faceID ));
      if ( allNodesAtFace ) // quad if on faceID
        return true;
    }
  }
  return false;
}
//================================================================================
/*!
  * \brief Finds nodes at a path from one node to another via intersections with EDGEs
  */
bool Hexahedron::findChain( _Node*          n1,
                            _Node*          n2,
                            _Face&          quad,
                            vector<_Node*>& chn )
{
  chn.clear();
  chn.push_back( n1 );
  for ( size_t iP = 0; iP < quad._eIntNodes.size(); ++iP )
    if ( !quad._eIntNodes[ iP ]->IsUsedInFace( &quad ) &&
          n1->IsLinked( quad._eIntNodes[ iP ]->_intPoint ) &&
          n2->IsLinked( quad._eIntNodes[ iP ]->_intPoint ))
    {
      chn.push_back( quad._eIntNodes[ iP ]);
      chn.push_back( n2 );
      quad._eIntNodes[ iP ]->_usedInFace = &quad;
      return true;
    }
  bool found;
  do
  {
    found = false;
    for ( size_t iP = 0; iP < quad._eIntNodes.size(); ++iP )
      if ( !quad._eIntNodes[ iP ]->IsUsedInFace( &quad ) &&
            chn.back()->IsLinked( quad._eIntNodes[ iP ]->_intPoint ))
      {
        chn.push_back( quad._eIntNodes[ iP ]);
        found = ( quad._eIntNodes[ iP ]->_usedInFace = &quad );
        break;
      }
  } while ( found && ! chn.back()->IsLinked( n2->_intPoint ) );

  if ( chn.back() != n2 && chn.back()->IsLinked( n2->_intPoint ))
    chn.push_back( n2 );

  return chn.size() > 1;
}
//================================================================================
/*!
  * \brief Try to heal a polygon whose ends are not connected
  */
bool Hexahedron::closePolygon( _Face* polygon, vector<_Node*>& chainNodes ) const
{
  int i = -1, nbLinks = polygon->_links.size();
  if ( nbLinks < 3 )
    return false;
  vector< _OrientedLink > newLinks;
  // find a node lying on the same FACE as the last one
  _Node*       node = polygon->_links.back().LastNode();
  TGeomID avoidFace = node->IsLinked( polygon->_links.back().FirstNode()->_intPoint );
  for ( i = nbLinks - 2; i >= 0; --i )
    if ( node->IsLinked( polygon->_links[i].FirstNode()->_intPoint, avoidFace ))
      break;
  if ( i >= 0 )
  {
    for ( ; i < nbLinks; ++i )
      newLinks.push_back( polygon->_links[i] );
  }
  else
  {
    // find a node lying on the same FACE as the first one
    node      = polygon->_links[0].FirstNode();
    avoidFace = node->IsLinked( polygon->_links[0].LastNode()->_intPoint );
    for ( i = 1; i < nbLinks; ++i )
      if ( node->IsLinked( polygon->_links[i].LastNode()->_intPoint, avoidFace ))
        break;
    if ( i < nbLinks )
      for ( nbLinks = i + 1, i = 0; i < nbLinks; ++i )
        newLinks.push_back( polygon->_links[i] );
  }
  if ( newLinks.size() > 1 )
  {
    polygon->_links.swap( newLinks );
    chainNodes.clear();
    chainNodes.push_back( polygon->_links.back().LastNode() );
    chainNodes.push_back( polygon->_links[0].FirstNode() );
    return true;
  }
  return false;
}
//================================================================================
/*!
  * \brief Finds nodes on the same EDGE as the first node of avoidSplit.
  *
  * This function is for
  * 1) a case where an EDGE lies on a quad which lies on a FACE
  *    so that a part of quad in ON and another part is IN
  * 2) INTERNAL FACE passes through the 1st node of avoidSplit
  */
bool Hexahedron::findChainOnEdge( const vector< _OrientedLink >& splits,
                                  const _OrientedLink&           prevSplit,
                                  const _OrientedLink&           avoidSplit,
                                  const std::set< TGeomID > &    concaveFaces,
                                  size_t &                       iS,
                                  _Face&                         quad,
                                  vector<_Node*>&                chn )
{
  _Node* pn1 = prevSplit.FirstNode();
  _Node* pn2 = prevSplit.LastNode(); // pn2 is on EDGE, if not on INTERNAL FACE
  _Node* an3 = avoidSplit.LastNode();
  TGeomID avoidFace = pn1->IsLinked( pn2->_intPoint ); // FACE under the quad
  if ( avoidFace < 1 && pn1->_intPoint )
    return false;

  chn.clear();

  if ( !quad._eIntNodes.empty() ) // connect pn2 with EDGE intersections
  {
    chn.push_back( pn2 );
    bool found;
    do
    {
      found = false;
      for ( size_t iP = 0; iP < quad._eIntNodes.size(); ++iP )
        if (( !quad._eIntNodes[ iP ]->IsUsedInFace( &quad )) &&
            ( chn.back()->IsLinked( quad._eIntNodes[ iP ]->_intPoint, avoidFace )) &&
            ( !avoidFace || quad._eIntNodes[ iP ]->IsOnFace( avoidFace )))
        {
          chn.push_back( quad._eIntNodes[ iP ]);
          found = ( quad._eIntNodes[ iP ]->_usedInFace = &quad );
          break;
        }
    } while ( found );
    pn2 = chn.back();
  }

  _Node* n = 0, *stopNode = avoidSplit.LastNode();

  if ( pn2 == prevSplit.LastNode() && // pn2 is at avoidSplit.FirstNode()
        !isCorner( stopNode ))         // stopNode is in the middle of a _hexLinks
  {
    // move stopNode to a _hexNodes
    for ( int iE = 0; iE < 4; ++iE ) // loop on 4 sides of a quadrangle
      for ( size_t iL = 0; iL < quad._links[ iE ].NbResultLinks(); ++iL )
      {
        const _Link* sideSplit = & quad._links[ iE ]._link->_splits[ iL ];
        if ( sideSplit == avoidSplit._link )
        {
          if ( quad._links[ iE ].LastNode()->Node() )
            stopNode = quad._links[ iE ].LastNode();
          iE = 4;
          break;
        }
      }
  }

  // connect pn2 (probably new, at _eIntNodes) with a split

  int i, iConn = 0;
  size_t nbCommon;
  TGeomID commonFaces[20];
  _Node* nPrev = nullptr;
  for ( i = splits.size()-1; i >= 0; --i )
  {
    if ( !splits[i] )
      continue;

    bool stop = false;
    for ( int is1st = 0; is1st < 2; ++is1st )
    {
      _Node* nConn = is1st ? splits[i].FirstNode() : splits[i].LastNode();
      if ( nConn == nPrev )
      {
        if ( n == nConn )
          iConn = i;
        continue;
      }
      nPrev = nConn;
      if (( stop = ( nConn == stopNode )))
        break;
      // find a FACE connecting nConn with pn2 but not with an3
      if (( nConn != pn1 ) &&
          ( nConn->_intPoint && !nConn->_intPoint->_faceIDs.empty() ) &&
          ( nbCommon = nConn->GetCommonFaces( pn2->_intPoint, commonFaces )))
      {
        bool a3Coonect = true;
        for ( size_t iF = 0; iF < nbCommon &&  a3Coonect; ++iF )
          a3Coonect = an3->IsOnFace( commonFaces[ iF ]) || concaveFaces.count( commonFaces[ iF ]);
        if ( a3Coonect )
          continue;

        if ( !n )
        {
          n = nConn;
          iConn = i + !is1st;
        }
        if ( nbCommon > 1 ) // nConn is linked with pn2 by an EDGE
        {
          n = nConn;
          iConn = i + !is1st;
          stop = true;
          break;
        }
      }
    }
    if ( stop )
    {
      i = iConn;
      break;
    }
  }

  if ( n && n != stopNode )
  {
    if ( chn.empty() )
      chn.push_back( pn2 );
    chn.push_back( n );
    iS = i-1;
    return true;
  }
  else if ( !chn.empty() && chn.back()->_isInternalFlags )
  {
    // INTERNAL FACE partially cuts the quad
    for ( int ip = chn.size() - 2; ip >= 0; --ip )
      chn.push_back( chn[ ip ]);
    return true;
  }
  return false;
}
//================================================================================
/*!
  * \brief Checks transition at the ginen intersection node of a link
  */
bool Hexahedron::isOutPoint( _Link& link, int iP,
                              SMESH_MesherHelper& helper, const Solid* solid ) const
{
  bool isOut = false;

  if ( link._fIntNodes[iP]->faces().size() == 1 &&
        _grid->IsInternal( link._fIntNodes[iP]->face(0) ))
    return false;

  const bool moreIntPoints = ( iP+1 < (int) link._fIntNodes.size() );

  // get 2 _Node's
  _Node* n1 = link._fIntNodes[ iP ];
  if ( !n1->Node() )
    n1 = link._nodes[0];
  _Node* n2 = moreIntPoints ? link._fIntNodes[ iP+1 ] : 0;
  if ( !n2 || !n2->Node() )
    n2 = link._nodes[1];
  if ( !n2->Node() )
    return true;

  // get all FACEs under n1 and n2
  set< TGeomID > faceIDs;
  if ( moreIntPoints ) faceIDs.insert( link._fIntNodes[iP+1]->faces().begin(),
                                        link._fIntNodes[iP+1]->faces().end() );
  if ( n2->_intPoint ) faceIDs.insert( n2->_intPoint->_faceIDs.begin(),
                                        n2->_intPoint->_faceIDs.end() );
  if ( faceIDs.empty() )
    return false; // n2 is inside
  if ( n1->_intPoint ) faceIDs.insert( n1->_intPoint->_faceIDs.begin(),
                                        n1->_intPoint->_faceIDs.end() );
  faceIDs.insert( link._fIntNodes[iP]->faces().begin(),
                  link._fIntNodes[iP]->faces().end() );

  // get a point between 2 nodes
  gp_Pnt p1      = n1->Point();
  gp_Pnt p2      = n2->Point();
  gp_Pnt pOnLink = 0.8 * p1.XYZ() + 0.2 * p2.XYZ();

  TopLoc_Location loc;

  set< TGeomID >::iterator faceID = faceIDs.begin();
  for ( ; faceID != faceIDs.end(); ++faceID )
  {
    // project pOnLink on a FACE
    if ( *faceID < 1 || !solid->Contains( *faceID )) continue;
    const TopoDS_Face& face = TopoDS::Face( _grid->Shape( *faceID ));
    GeomAPI_ProjectPointOnSurf& proj = helper.GetProjector( face, loc, 0.1*_grid->_tol );
    gp_Pnt testPnt = pOnLink.Transformed( loc.Transformation().Inverted() );
    proj.Perform( testPnt );
    if ( proj.IsDone() && proj.NbPoints() > 0 )       
    {
      Standard_Real u,v;
      proj.LowerDistanceParameters( u,v );

      if ( proj.LowerDistance() <= 0.1 * _grid->_tol )
      {
        isOut = false;
      }
      else
      {
        // find isOut by normals
        gp_Dir normal;
        if ( GeomLib::NormEstim( BRep_Tool::Surface( face, loc ),
                                  gp_Pnt2d( u,v ),
                                  0.1*_grid->_tol,
                                  normal ) < 3 )
        {
          if ( solid->Orientation( face ) == TopAbs_REVERSED )
            normal.Reverse();
          gp_Vec v( proj.NearestPoint(), testPnt );
          isOut = ( v * normal > 0 );
        }
      }
      if ( !isOut )
      {
        // classify a projection
        if ( !n1->IsOnFace( *faceID ) || !n2->IsOnFace( *faceID ))
        {
          BRepTopAdaptor_FClass2d cls( face, Precision::Confusion() );
          TopAbs_State state = cls.Perform( gp_Pnt2d( u,v ));
          if ( state == TopAbs_OUT )
          {
            isOut = true;
            continue;
          }
        }
        return false;
      }
    }
  }
  return isOut;
}
//================================================================================
/*!
  * \brief Sort nodes on a FACE
  */
void Hexahedron::sortVertexNodes(vector<_Node*>& nodes, _Node* curNode, TGeomID faceID)
{
  if ( nodes.size() > 20 ) return;

  // get shapes under nodes
  TGeomID nShapeIds[20], *nShapeIdsEnd = &nShapeIds[0] + nodes.size();
  for ( size_t i = 0; i < nodes.size(); ++i )
    if ( !( nShapeIds[i] = nodes[i]->ShapeID() ))
      return;

  // get shapes of the FACE
  const TopoDS_Face&  face = TopoDS::Face( _grid->Shape( faceID ));
  list< TopoDS_Edge > edges;
  list< int >         nbEdges;
  int nbW = SMESH_Block::GetOrderedEdges (face, edges, nbEdges);
  if ( nbW > 1 ) {
    // select a WIRE - remove EDGEs of irrelevant WIREs from edges
    list< TopoDS_Edge >::iterator e = edges.begin(), eEnd = e;
    list< int >::iterator nE = nbEdges.begin();
    for ( ; nbW > 0; ++nE, --nbW )
    {
      std::advance( eEnd, *nE );
      for ( ; e != eEnd; ++e )
        for ( int i = 0; i < 2; ++i )
        {
          TGeomID id = i==0 ?
            _grid->ShapeID( *e ) :
            _grid->ShapeID( SMESH_MesherHelper::IthVertex( 0, *e ));
          if (( id > 0 ) &&
              ( std::find( &nShapeIds[0], nShapeIdsEnd, id ) != nShapeIdsEnd ))
          {
            edges.erase( eEnd, edges.end() ); // remove rest wires
            e = eEnd = edges.end();
            --e;
            nbW = 0;
            break;
          }
        }
      if ( nbW > 0 )
        edges.erase( edges.begin(), eEnd ); // remove a current irrelevant wire
    }
  }
  // rotate edges to have the first one at least partially out of the hexa
  list< TopoDS_Edge >::iterator e = edges.begin(), eMidOut = edges.end();
  for ( ; e != edges.end(); ++e )
  {
    if ( !_grid->ShapeID( *e ))
      continue;
    bool isOut = false;
    gp_Pnt p;
    double uvw[3], f,l;
    for ( int i = 0; i < 2 && !isOut; ++i )
    {
      if ( i == 0 )
      {
        TopoDS_Vertex v = SMESH_MesherHelper::IthVertex( 0, *e );
        p = BRep_Tool::Pnt( v );
      }
      else if ( eMidOut == edges.end() )
      {
        TopLoc_Location loc;
        Handle(Geom_Curve) c = BRep_Tool::Curve( *e, loc, f, l);
        if ( c.IsNull() ) break;
        p = c->Value( 0.5 * ( f + l )).Transformed( loc );
      }
      else
      {
        continue;
      }

      _grid->ComputeUVW( p.XYZ(), uvw );
      if ( isOutParam( uvw ))
      {
        if ( i == 0 )
          isOut = true;
        else
          eMidOut = e;
      }
    }
    if ( isOut )
      break;
  }
  if ( e != edges.end() )
    edges.splice( edges.end(), edges, edges.begin(), e );
  else if ( eMidOut != edges.end() )
    edges.splice( edges.end(), edges, edges.begin(), eMidOut );

  // sort nodes according to the order of edges
  _Node*  orderNodes   [20];
  //TGeomID orderShapeIDs[20];
  size_t nbN = 0;
  TGeomID id, *pID = 0;
  for ( e = edges.begin(); e != edges.end(); ++e )
  {
    if (( id = _grid->ShapeID( SMESH_MesherHelper::IthVertex( 0, *e ))) &&
        (( pID = std::find( &nShapeIds[0], nShapeIdsEnd, id )) != nShapeIdsEnd ))
    {
      //orderShapeIDs[ nbN ] = id;
      orderNodes   [ nbN++ ] = nodes[ pID - &nShapeIds[0] ];
      *pID = -1;
    }
    if (( id = _grid->ShapeID( *e )) &&
        (( pID = std::find( &nShapeIds[0], nShapeIdsEnd, id )) != nShapeIdsEnd ))
    {
      //orderShapeIDs[ nbN ] = id;
      orderNodes   [ nbN++ ] = nodes[ pID - &nShapeIds[0] ];
      *pID = -1;
    }
  }
  if ( nbN != nodes.size() )
    return;

  bool reverse = ( orderNodes[0    ]->Point().SquareDistance( curNode->Point() ) >
                    orderNodes[nbN-1]->Point().SquareDistance( curNode->Point() ));

  for ( size_t i = 0; i < nodes.size(); ++i )
    nodes[ i ] = orderNodes[ reverse ? nbN-1-i : i ];
}

//================================================================================
/*!
  * \brief Adds computed elements to the mesh
  */
int Hexahedron::addVolumes( SMESH_MesherHelper& helper )
{
  F_IntersectPoint noIntPnt;
  const bool toCheckNodePos = _grid->IsToCheckNodePos();
  const bool useQuanta      = _grid->_toUseQuanta;

  int nbAdded = 0;
  // add elements resulted from hexahedron intersection
  for ( _volumeDef* volDef = &_volumeDefs; volDef; volDef = volDef->_next )
  {
    vector< const SMDS_MeshNode* > nodes( volDef->_nodes.size() );
    for ( size_t iN = 0; iN < nodes.size(); ++iN )
    {
      if ( !( nodes[iN] = volDef->_nodes[iN].Node() ))
      {
        if ( const E_IntersectPoint* eip = volDef->_nodes[iN].EdgeIntPnt() )
        {
          nodes[iN] = volDef->_nodes[iN]._intPoint->_node =
            helper.AddNode( eip->_point.X(),
                            eip->_point.Y(),
                            eip->_point.Z() );
          if ( _grid->ShapeType( eip->_shapeID ) == TopAbs_VERTEX )
            helper.GetMeshDS()->SetNodeOnVertex( nodes[iN], eip->_shapeID );
          else
            helper.GetMeshDS()->SetNodeOnEdge( nodes[iN], eip->_shapeID );
        }
        else
          throw SALOME_Exception("Bug: no node at intersection point");
      }
      else if ( volDef->_nodes[iN]._intPoint &&
                volDef->_nodes[iN]._intPoint->_node == volDef->_nodes[iN]._node )
      {
        // Update position of node at EDGE intersection;
        // see comment to _Node::Add( E_IntersectPoint )
        SMESHDS_Mesh* mesh = helper.GetMeshDS();
        TGeomID    shapeID = volDef->_nodes[iN].EdgeIntPnt()->_shapeID;
        mesh->UnSetNodeOnShape( nodes[iN] );
        if ( _grid->ShapeType( shapeID ) == TopAbs_VERTEX )
          mesh->SetNodeOnVertex( nodes[iN], shapeID );
        else
          mesh->SetNodeOnEdge( nodes[iN], shapeID );
      }
      else if ( toCheckNodePos &&
                !nodes[iN]->isMarked() &&
                _grid->ShapeType( nodes[iN]->GetShapeID() ) == TopAbs_FACE )
      {
        _grid->SetOnShape( nodes[iN], noIntPnt, /*v=*/nullptr,/*unset=*/true );
        nodes[iN]->setIsMarked( true );
      }
    } // loop to get nodes

    const SMDS_MeshElement* v = 0;      
    if ( !volDef->_quantities.empty() )
    {                      
      if ( !useQuanta )
      {
        // split polyhedrons of with disjoint volumes
        std::vector<std::vector<int>> splitQuantities;
        std::vector<std::vector< const SMDS_MeshNode* > > splitNodes;
        if ( checkPolyhedronValidity( volDef, splitQuantities, splitNodes ) == 1 )
          v = addPolyhedronToMesh( volDef, helper, nodes, volDef->_quantities );
        else
        {
          int counter = -1;
          for (size_t id = 0; id < splitQuantities.size(); id++)
          {
            v = addPolyhedronToMesh( volDef, helper, splitNodes[ id ], splitQuantities[ id ] );
            if ( id < splitQuantities.size()-1 )
              volDef->_brotherVolume.push_back( v );
            counter++;
          }
          nbAdded += counter;
        }
      }
      else
      {
        const double quanta = _grid->_quanta;
        double polyVol      = volDef->_size;
        double hexaVolume   = _sideLength[0] * _sideLength[1] * _sideLength[2];          
        if ( hexaVolume > 0.0 && polyVol/hexaVolume >= quanta /*set the volume if the relation is satisfied*/)
          v = helper.AddVolume( _hexNodes[0].BoundaryNode(), _hexNodes[2].BoundaryNode(),
                                _hexNodes[3].BoundaryNode(), _hexNodes[1].BoundaryNode(),
                                _hexNodes[4].BoundaryNode(), _hexNodes[6].BoundaryNode(),
                                _hexNodes[7].BoundaryNode(), _hexNodes[5].BoundaryNode() );
        
      }
    }
    else
    {
      switch ( nodes.size() )
      {
      case 8: v = helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3],
                                    nodes[4],nodes[5],nodes[6],nodes[7] );
        break;
      case 4: v = helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3] );
        break;
      case 6: v = helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3],nodes[4],nodes[5] );
        break;
      case 5: v = helper.AddVolume( nodes[0],nodes[1],nodes[2],nodes[3],nodes[4] );
        break;
      }
    }
    volDef->_volume = v;
    nbAdded += bool( v );

  } // loop on _volumeDefs chain

  // avoid creating overlapping volumes (bos #24052)
  if ( nbAdded > 1 )
  {
    double sumSize = 0, maxSize = 0;
    _volumeDef* maxSizeDef = nullptr;
    for ( _volumeDef* volDef = &_volumeDefs; volDef; volDef = volDef->_next )
    {
      if ( !volDef->_volume )
        continue;
      sumSize += volDef->_size;
      if ( volDef->_size > maxSize )
      {
        maxSize    = volDef->_size;
        maxSizeDef = volDef;
      }
    }
    if ( sumSize > _sideLength[0] * _sideLength[1] * _sideLength[2] * 1.05 )
    {
      for ( _volumeDef* volDef = &_volumeDefs; volDef; volDef = volDef->_next )
        if ( volDef != maxSizeDef && volDef->_volume )
        {
          helper.GetMeshDS()->RemoveFreeElement( volDef->_volume, /*sm=*/nullptr,
                                                  /*fromGroups=*/false );
          volDef->_volume = nullptr;
          //volDef->_nodes.clear();
          --nbAdded;
        }
    }
  }

  for ( _volumeDef* volDef = &_volumeDefs; volDef; volDef = volDef->_next )
  {
    if ( volDef->_volume )
    {
      helper.GetMeshDS()->SetMeshElementOnShape( volDef->_volume, volDef->_solidID );
      for (auto broVol : volDef->_brotherVolume )
      {
        helper.GetMeshDS()->SetMeshElementOnShape( broVol, volDef->_solidID );
      }
    }
  }

  return nbAdded;
}
//================================================================================
/*!
  * \brief Return true if the element is in a hole
  * \remark consider a cell to be in a hole if all links in any direction
  *          comes OUT of geometry
  */
bool Hexahedron::isInHole() const
{
  if ( !_vIntNodes.empty() )
    return false;

  const size_t ijk[3] = { _i, _j, _k };
  F_IntersectPoint curIntPnt;

  // consider a cell to be in a hole if all links in any direction
  // comes OUT of geometry
  for ( int iDir = 0; iDir < 3; ++iDir )
  {
    const vector<double>& coords = _grid->_coords[ iDir ];
    LineIndexer               li = _grid->GetLineIndexer( iDir );
    li.SetIJK( _i,_j,_k );
    size_t lineIndex[4] = { li.LineIndex  (),
                            li.LineIndex10(),
                            li.LineIndex01(),
                            li.LineIndex11() };
    bool allLinksOut = true, hasLinks = false;
    for ( int iL = 0; iL < 4 && allLinksOut; ++iL ) // loop on 4 links parallel to iDir
    {
      const _Link& link = _hexLinks[ iL + 4*iDir ];
      // check transition of the first node of a link
      const F_IntersectPoint* firstIntPnt = 0;
      if ( link._nodes[0]->Node() ) // 1st node is a hexa corner
      {
        curIntPnt._paramOnLine = coords[ ijk[ iDir ]] - coords[0] + _grid->_tol;
        const GridLine& line = _grid->_lines[ iDir ][ lineIndex[ iL ]];
        if ( !line._intPoints.empty() )
        {
          multiset< F_IntersectPoint >::const_iterator ip =
            line._intPoints.upper_bound( curIntPnt );
          --ip;
          firstIntPnt = &(*ip);
        }
      }
      else if ( !link._fIntPoints.empty() )
      {
        firstIntPnt = link._fIntPoints[0];
      }

      if ( firstIntPnt )
      {
        hasLinks = true;
        allLinksOut = ( firstIntPnt->_transition == Trans_OUT &&
                        !_grid->IsShared( firstIntPnt->_faceIDs[0] ));
      }
    }
    if ( hasLinks && allLinksOut )
      return true;
  }
  return false;
}

//================================================================================
/*!
  * \brief Check if a polyherdon has an edge lying on EDGE shared by strange FACE
  *        that will be meshed by other algo
  */
bool Hexahedron::hasStrangeEdge() const
{
  if ( _eIntPoints.size() < 2 )
    return false;

  TopTools_MapOfShape edges;
  for ( size_t i = 0; i < _eIntPoints.size(); ++i )
  {
    if ( !_grid->IsStrangeEdge( _eIntPoints[i]->_shapeID ))
      continue;
    const TopoDS_Shape& s = _grid->Shape( _eIntPoints[i]->_shapeID );
    if ( s.ShapeType() == TopAbs_EDGE )
    {
      if ( ! edges.Add( s ))
        return true; // an EDGE encounters twice
    }
    else
    {
      PShapeIteratorPtr edgeIt = _grid->_helper->GetAncestors( s,
                                                                *_grid->_helper->GetMesh(),
                                                                TopAbs_EDGE );
      while ( const TopoDS_Shape* edge = edgeIt->next() )
        if ( ! edges.Add( *edge ))
          return true; // an EDGE encounters twice
    }
  }
  return false;
}

//================================================================================
/*!
  * \brief Return true if a polyhedron passes _sizeThreshold criterion
  */
bool Hexahedron::checkPolyhedronSize( bool cutByInternalFace, double & volume) const
{
  volume = 0;

  if ( cutByInternalFace && !_grid->_toUseThresholdForInternalFaces )
  {
    // check if any polygon fully lies on shared/internal FACEs
    for ( size_t iP = 0; iP < _polygons.size(); ++iP )
    {
      const _Face& polygon = _polygons[iP];
      if ( polygon._links.empty() )
        continue;
      bool allNodesInternal = true;
      for ( size_t iL = 0; iL < polygon._links.size() &&  allNodesInternal; ++iL )
      {
        _Node* n = polygon._links[ iL ].FirstNode();
        allNodesInternal = (( n->IsCutByInternal() ) ||
                            ( n->_intPoint && _grid->IsAnyShared( n->_intPoint->_faceIDs )));
      }
      if ( allNodesInternal )
        return true;
    }
  }
  for ( size_t iP = 0; iP < _polygons.size(); ++iP )
  {
    const _Face& polygon = _polygons[iP];
    if ( polygon._links.empty() )
      continue;
    gp_XYZ area (0,0,0);
    gp_XYZ p1 = polygon._links[ 0 ].FirstNode()->Point().XYZ();
    for ( size_t iL = 0; iL < polygon._links.size(); ++iL )
    {
      gp_XYZ p2 = polygon._links[ iL ].LastNode()->Point().XYZ();
      area += p1 ^ p2;
      p1 = p2;
    }
    volume += p1 * area;
  }
  volume /= 6;

  if ( this->hasStrangeEdge() && volume > 1e-13 )
    return true;

  double initVolume = _sideLength[0] * _sideLength[1] * _sideLength[2];

  return volume > initVolume / _grid->_sizeThreshold;
}

//================================================================================
/*!
  * \brief Check that all faces in polyhedron are connected so a unique volume is defined.
  *        We test that it is possible to go from any node to all nodes in the polyhedron.
  *        The set of nodes that can be visit within then defines a unique element.
  *        In case more than one polyhedron is detected. The function return the set of quantities and nodes defining separates elements.
  *        Reference to issue #bos[38521][EDF] Generate polyhedron with separate volume.
  */
int Hexahedron::checkPolyhedronValidity( _volumeDef* volDef, std::vector<std::vector<int>>& splitQuantities, 
                                          std::vector<std::vector<const SMDS_MeshNode*>>& splitNodes )
{  
  int mySet = 1;
  std::map<int,int> numberOfSets; // define set id with the number of faces associated!
  if ( !volDef->_quantities.empty() )
  {
    auto connectivity = volDef->_quantities;
    int accum = 0;
    std::vector<bool> allFaces( connectivity.size(), false );
    std::set<int> elementSet;
    allFaces[ 0 ] = true; // the first node below to the first face
    size_t connectedFaces = 1;
    // Start filling the set with the nodes of the first face
    splitQuantities.push_back( { connectivity[ 0 ] } );
    splitNodes.push_back( { volDef->_nodes[ 0 ].Node() } );
    elementSet.insert( volDef->_nodes[ 0 ].Node()->GetID() );
    for (int n = 1; n < connectivity[ 0 ]; n++)
    {
      elementSet.insert( volDef->_nodes[ n ].Node()->GetID() );
      splitNodes.back().push_back( volDef->_nodes[ n ].Node() );
    }
    
    numberOfSets.insert( std::pair<int,int>(mySet,1) );
    while ( connectedFaces != allFaces.size() )
    {
      for (size_t innerId = 1; innerId < connectivity.size(); innerId++)
      {
        if ( innerId == 1 )
          accum = connectivity[ 0 ];

        if ( !allFaces[ innerId ] )
        {
          int faceCounter = 0;
          for (int n = 0; n < connectivity[ innerId ]; n++)
          {
            int nodeId = volDef->_nodes[ accum + n ].Node()->GetID();
            if ( elementSet.count( nodeId ) != 0 ) 
              faceCounter++;
          }
          if ( faceCounter >= 2 ) // found coincidences nodes
          {
            for (int n = 0; n < connectivity[ innerId ]; n++)
            {
              int nodeId = volDef->_nodes[ accum + n ].Node()->GetID();
              // insert new nodes so other faces can be identified as belowing to the element
              splitNodes.back().push_back( volDef->_nodes[ accum + n ].Node() );
              elementSet.insert( nodeId );
            }
            allFaces[ innerId ] = true;
            splitQuantities.back().push_back( connectivity[ innerId ] );
            numberOfSets[ mySet ]++;
            connectedFaces++;
            innerId = 0; // to restart searching!
          }
        }
        accum += connectivity[ innerId ];
      }

      if ( connectedFaces != allFaces.size() )
      {
        // empty the set, and fill it with nodes of a unvisited face!
        elementSet.clear();
        accum = connectivity[ 0 ];
        for (size_t faceId = 1; faceId < connectivity.size(); faceId++)
        {
          if ( !allFaces[ faceId ] )
          {
            splitNodes.push_back( { volDef->_nodes[ accum ].Node() } );
            elementSet.insert( volDef->_nodes[ accum ].Node()->GetID() );
            for (int n = 1; n < connectivity[ faceId ]; n++)
            {
              elementSet.insert( volDef->_nodes[ accum + n ].Node()->GetID() );
              splitNodes.back().push_back( volDef->_nodes[ accum + n ].Node() );
            }

            splitQuantities.push_back( { connectivity[ faceId ] } );
            allFaces[ faceId ] = true;
            connectedFaces++;
            break;
          }
          accum += connectivity[ faceId ];
        }
        mySet++;
        numberOfSets.insert( std::pair<int,int>(mySet,1) );
      }
    }

    if ( numberOfSets.size() > 1 )
    {
      bool allMoreThan2Faces = true;
      for( auto k : numberOfSets )
      {
        if ( k.second <= 2 )
          allMoreThan2Faces &= false;
      }
      
      if ( allMoreThan2Faces )
      {        
        // The separate objects are suspect to be closed
        return numberOfSets.size();        
      }
      else
      {
        // Have to index the last face nodes to the final set
        // contrary case return as it were a valid polyhedron for backward compatibility
        return 1;  
      }
    }
  }
  return numberOfSets.size();
}


//================================================================================
/*!
  * \brief add original or separated polyhedrons to the mesh
  */
const SMDS_MeshElement* Hexahedron::addPolyhedronToMesh( _volumeDef* volDef,  SMESH_MesherHelper& helper, const std::vector<const SMDS_MeshNode*>& nodes, 
                                                          const std::vector<int>& quantities )
{
  const SMDS_MeshElement* v = helper.AddPolyhedralVolume( nodes, quantities );

  volDef->_size = SMDS_VolumeTool( v ).GetSize();
  if ( volDef->_size < 0 ) // invalid polyhedron
  {
    if ( ! SMESH_MeshEditor( helper.GetMesh() ).Reorient( v ) || // try to fix
        SMDS_VolumeTool( v ).GetSize() < 0 )
    {
      helper.GetMeshDS()->RemoveFreeElement( v, /*sm=*/nullptr, /*fromGroups=*/false );
      v = nullptr;
      //_hasTooSmall = true;

      if (SALOME::VerbosityActivated())
      {
        std::cout << "Remove INVALID polyhedron, _cellID = " << _cellID
                  << " ijk = ( " << _i << " " << _j << " " << _k << " ) "
                  << " solid " << volDef->_solidID << std::endl;
      }
    }
  }
  return v;
}

//================================================================================
/*!
  * \brief Tries to create a hexahedron
  */
bool Hexahedron::addHexa()
{
  int nbQuad = 0, iQuad = -1;
  for ( size_t i = 0; i < _polygons.size(); ++i )
  {
    if ( _polygons[i]._links.empty() )
      continue;
    if ( _polygons[i]._links.size() != 4 )
      return false;
    ++nbQuad;
    if ( iQuad < 0 )
      iQuad = i;
  }
  if ( nbQuad != 6 )
    return false;

  _Node* nodes[8];
  int nbN = 0;
  for ( int iL = 0; iL < 4; ++iL )
  {
    // a base node
    nodes[iL] = _polygons[iQuad]._links[iL].FirstNode();
    ++nbN;

    // find a top node above the base node
    _Link* link = _polygons[iQuad]._links[iL]._link;
    if ( !link->_faces[0] || !link->_faces[1] )
      return debugDumpLink( link );
    // a quadrangle sharing <link> with _polygons[iQuad]
    _Face* quad = link->_faces[ bool( link->_faces[0] == & _polygons[iQuad] )];
    for ( int i = 0; i < 4; ++i )
      if ( quad->_links[i]._link == link )
      {
        // 1st node of a link opposite to <link> in <quad>
        nodes[iL+4] = quad->_links[(i+2)%4].FirstNode();
        ++nbN;
        break;
      }
  }
  if ( nbN == 8 )
    _volumeDefs.Set( &nodes[0], 8 );

  return nbN == 8;
}
//================================================================================
/*!
  * \brief Tries to create a tetrahedron
  */
bool Hexahedron::addTetra()
{
  int iTria = -1;
  for ( size_t i = 0; i < _polygons.size() && iTria < 0; ++i )
    if ( _polygons[i]._links.size() == 3 )
      iTria = i;
  if ( iTria < 0 )
    return false;

  _Node* nodes[4];
  nodes[0] = _polygons[iTria]._links[0].FirstNode();
  nodes[1] = _polygons[iTria]._links[1].FirstNode();
  nodes[2] = _polygons[iTria]._links[2].FirstNode();

  _Link* link = _polygons[iTria]._links[0]._link;
  if ( !link->_faces[0] || !link->_faces[1] )
    return debugDumpLink( link );

  // a triangle sharing <link> with _polygons[0]
  _Face* tria = link->_faces[ bool( link->_faces[0] == & _polygons[iTria] )];
  for ( int i = 0; i < 3; ++i )
    if ( tria->_links[i]._link == link )
    {
      nodes[3] = tria->_links[(i+1)%3].LastNode();
      _volumeDefs.Set( &nodes[0], 4 );
      return true;
    }

  return false;
}
//================================================================================
/*!
  * \brief Tries to create a pentahedron
  */
bool Hexahedron::addPenta()
{
  // find a base triangular face
  int iTri = -1;
  for ( int iF = 0; iF < 5 && iTri < 0; ++iF )
    if ( _polygons[ iF ]._links.size() == 3 )
      iTri = iF;
  if ( iTri < 0 ) return false;

  // find nodes
  _Node* nodes[6];
  int nbN = 0;
  for ( int iL = 0; iL < 3; ++iL )
  {
    // a base node
    nodes[iL] = _polygons[ iTri ]._links[iL].FirstNode();
    ++nbN;

    // find a top node above the base node
    _Link* link = _polygons[ iTri ]._links[iL]._link;
    if ( !link->_faces[0] || !link->_faces[1] )
      return debugDumpLink( link );
    // a quadrangle sharing <link> with a base triangle
    _Face* quad = link->_faces[ bool( link->_faces[0] == & _polygons[ iTri ] )];
    if ( quad->_links.size() != 4 ) return false;
    for ( int i = 0; i < 4; ++i )
      if ( quad->_links[i]._link == link )
      {
        // 1st node of a link opposite to <link> in <quad>
        nodes[iL+3] = quad->_links[(i+2)%4].FirstNode();
        ++nbN;
        break;
      }
  }
  if ( nbN == 6 )
    _volumeDefs.Set( &nodes[0], 6 );

  return ( nbN == 6 );
}
//================================================================================
/*!
  * \brief Tries to create a pyramid
  */
bool Hexahedron::addPyra()
{
  // find a base quadrangle
  int iQuad = -1;
  for ( int iF = 0; iF < 5 && iQuad < 0; ++iF )
    if ( _polygons[ iF ]._links.size() == 4 )
      iQuad = iF;
  if ( iQuad < 0 ) return false;

  // find nodes
  _Node* nodes[5];
  nodes[0] = _polygons[iQuad]._links[0].FirstNode();
  nodes[1] = _polygons[iQuad]._links[1].FirstNode();
  nodes[2] = _polygons[iQuad]._links[2].FirstNode();
  nodes[3] = _polygons[iQuad]._links[3].FirstNode();

  _Link* link = _polygons[iQuad]._links[0]._link;
  if ( !link->_faces[0] || !link->_faces[1] )
    return debugDumpLink( link );

  // a triangle sharing <link> with a base quadrangle
  _Face* tria = link->_faces[ bool( link->_faces[0] == & _polygons[ iQuad ] )];
  if ( tria->_links.size() != 3 ) return false;
  for ( int i = 0; i < 3; ++i )
    if ( tria->_links[i]._link == link )
    {
      nodes[4] = tria->_links[(i+1)%3].LastNode();
      _volumeDefs.Set( &nodes[0], 5 );
      return true;
    }

  return false;
}
//================================================================================
/*!
  * \brief Return true if there are _eIntPoints at EDGEs forming a  concave corner
  */
bool Hexahedron::hasEdgesAround( const ConcaveFace* cf ) const
{
  int nbEdges = 0;
  ConcaveFace foundGeomHolder;
  for ( const E_IntersectPoint* ip : _eIntPoints )
  {
    if ( cf->HasEdge( ip->_shapeID ))
    {
      if ( ++nbEdges == 2 )
        return true;
      foundGeomHolder.SetEdge( ip->_shapeID );
    }
    else if ( ip->_faceIDs.size() >= 3 )
    {
      const TGeomID & vID = ip->_shapeID;
      if ( cf->HasVertex( vID ) && !foundGeomHolder.HasVertex( vID ))
      {
        if ( ++nbEdges == 2 )
          return true;
        foundGeomHolder.SetVertex( vID );
      }
    }
  }

  for ( const _Node& hexNode: _hexNodes )
  {
    if ( !hexNode._node || !hexNode._intPoint )
      continue;
    const B_IntersectPoint* ip = hexNode._intPoint;
    if ( ip->_faceIDs.size() == 2 ) // EDGE
    {
      TGeomID edgeID = hexNode._node->GetShapeID();
      if ( cf->HasEdge( edgeID ) && !foundGeomHolder.HasEdge( edgeID ))
      {
        foundGeomHolder.SetEdge( edgeID );
        if ( ++nbEdges == 2 )
          return true;
      }
    }
    else if ( ip->_faceIDs.size() >= 3 ) // VERTEX
    {
      TGeomID vID = hexNode._node->GetShapeID();
      if ( cf->HasVertex( vID ) && !foundGeomHolder.HasVertex( vID ))
      {
        if ( ++nbEdges == 2 )
          return true;
        foundGeomHolder.SetVertex( vID );
      }
    }
  }

  return false;
}
//================================================================================
/*!
  * \brief Dump a link and return \c false
  */
bool Hexahedron::debugDumpLink( Hexahedron::_Link* link )
{
  if (SALOME::VerbosityActivated())
  {
    gp_Pnt p1 = link->_nodes[0]->Point(), p2 = link->_nodes[1]->Point();
    cout << "BUG: not shared link. IKJ = ( "<< _i << " " << _j << " " << _k << " )" << endl
        << "n1 (" << p1.X() << ", "<< p1.Y() << ", "<< p1.Z() << " )" << endl
        << "n2 (" << p2.X() << ", "<< p2.Y() << ", "<< p2.Z() << " )" << endl;
  }

  return false;
}
//================================================================================
/*!
  * \brief Classify a point by grid parameters
  */
bool Hexahedron::isOutParam(const double uvw[3]) const
{
  return (( _grid->_coords[0][ _i   ] - _grid->_tol > uvw[0] ) ||
          ( _grid->_coords[0][ _i+1 ] + _grid->_tol < uvw[0] ) ||
          ( _grid->_coords[1][ _j   ] - _grid->_tol > uvw[1] ) ||
          ( _grid->_coords[1][ _j+1 ] + _grid->_tol < uvw[1] ) ||
          ( _grid->_coords[2][ _k   ] - _grid->_tol > uvw[2] ) ||
          ( _grid->_coords[2][ _k+1 ] + _grid->_tol < uvw[2] ));
}

//================================================================================
/*!
  * \brief Create mesh faces at free facets
  */
void Hexahedron::addFaces( SMESH_MesherHelper&                       helper,
                            const vector< const SMDS_MeshElement* > & boundaryVolumes )
{
  if ( !_grid->_toCreateFaces )
    return;

  SMDS_VolumeTool vTool;
  vector<int> bndFacets;
  SMESH_MeshEditor editor( helper.GetMesh() );
  SMESH_MeshEditor::ElemFeatures face( SMDSAbs_Face );
  SMESHDS_Mesh* meshDS = helper.GetMeshDS();

  bool isQuantaSet =  _grid->_toUseQuanta;    
  // check if there are internal or shared FACEs
  bool hasInternal = ( !_grid->_geometry.IsOneSolid() ||
                        _grid->_geometry._soleSolid.HasInternalFaces() );   

  for ( size_t iV = 0; iV < boundaryVolumes.size(); ++iV )
  {
    if ( !vTool.Set( boundaryVolumes[ iV ]))
      continue;
    TGeomID solidID = vTool.Element()->GetShapeID();
    Solid *   solid = _grid->GetOneOfSolids( solidID );

    // find boundary facets
    bndFacets.clear();
    for ( int iF = 0, n = vTool.NbFaces(); iF < n; iF++ )
    {
      const SMDS_MeshElement* otherVol;
      bool isBoundary = isQuantaSet ? vTool.IsFreeFaceCheckAllNodes( iF, &otherVol ) : vTool.IsFreeFace( iF, &otherVol );
      if ( isBoundary )
      {
        bndFacets.push_back( iF );
      }
      else if (( hasInternal ) ||
                ( !_grid->IsSolid( otherVol->GetShapeID() )))
      {
        // check if all nodes are on internal/shared FACEs
        isBoundary = true;
        const SMDS_MeshNode** nn = vTool.GetFaceNodes( iF );
        const size_t nbFaceNodes = vTool.NbFaceNodes ( iF );
        for ( size_t iN = 0; iN < nbFaceNodes &&  isBoundary; ++iN )
          isBoundary = ( nn[ iN ]->GetShapeID() != solidID );
        if ( isBoundary )
          bndFacets.push_back( -( iF+1 )); // !!! minus ==> to check the FACE
      }
    }
    if ( bndFacets.empty() )
      continue;

    // create faces
    if ( !vTool.IsPoly() )
      vTool.SetExternalNormal();
    for ( size_t i = 0; i < bndFacets.size(); ++i ) // loop on boundary facets
    {
      const bool    isBoundary = ( bndFacets[i] >= 0 );
      const int         iFacet = isBoundary ? bndFacets[i] : -bndFacets[i]-1;
      const SMDS_MeshNode** nn = vTool.GetFaceNodes( iFacet );
      const size_t nbFaceNodes = vTool.NbFaceNodes ( iFacet );
      face.myNodes.assign( nn, nn + nbFaceNodes );

      TGeomID faceID = 0;
      const SMDS_MeshElement* existFace = 0, *newFace = 0;

      if (( existFace = meshDS->FindElement( face.myNodes, SMDSAbs_Face )))
      {
        if ( existFace->isMarked() )
          continue; // created by this method
        faceID = existFace->GetShapeID();
      }
      else
      {
        // look for a supporting FACE
        for ( size_t iN = 0; iN < nbFaceNodes &&  !faceID; ++iN ) // look for a node on FACE
        {
          if ( nn[ iN ]->GetPosition()->GetDim() == 2 )
            faceID = nn[ iN ]->GetShapeID();
        }
        if ( faceID == 0 && !isQuantaSet /*if quanta is set boundary nodes at boundary does not coincide with any geometrical face */ )
          faceID = findCommonFace( face.myNodes, helper.GetMesh() );

        bool toCheckFace = faceID && (( !isBoundary ) ||
                                      ( hasInternal && _grid->_toUseThresholdForInternalFaces ));
        if ( toCheckFace ) // check if all nodes are on the found FACE
        {
          SMESH_subMesh* faceSM = helper.GetMesh()->GetSubMeshContaining( faceID );
          for ( size_t iN = 0; iN < nbFaceNodes &&  faceID; ++iN )
          {
            TGeomID subID = nn[ iN ]->GetShapeID();
            if ( subID != faceID && !faceSM->DependsOn( subID ))
              faceID = 0;
          }
          // if ( !faceID && !isBoundary )
          //   continue;
        }
        if ( !faceID && !isBoundary && !isQuantaSet )
          continue;
      }

      // orient a new face according to supporting FACE orientation in shape_to_mesh
      if ( !isBoundary && !solid->IsOutsideOriented( faceID ))
      {
        if ( existFace )
          editor.Reorient( existFace );
        else
          std::reverse( face.myNodes.begin(), face.myNodes.end() );
      }

      if ( ! ( newFace = existFace ))
      {
        face.SetPoly( nbFaceNodes > 4 );
        newFace = editor.AddElement( face.myNodes, face );
        if ( !newFace )
          continue;
        newFace->setIsMarked( true ); // to distinguish from face created in getBoundaryElems()
      }

      if ( faceID && _grid->IsBoundaryFace( faceID )) // face is not shared
      {
        // set newFace to the found FACE provided that it fully lies on the FACE
        for ( size_t iN = 0; iN < nbFaceNodes &&  faceID; ++iN )
          if ( nn[iN]->GetShapeID() == solidID )
          {
            if ( existFace )
              meshDS->UnSetMeshElementOnShape( existFace, _grid->Shape( faceID ));
            faceID = 0;
          }
      }

      if ( faceID && nbFaceNodes > 4 &&
            !_grid->IsInternal( faceID ) &&
            !_grid->IsShared( faceID ) &&
            !_grid->IsBoundaryFace( faceID ))
      {
        // split a polygon that will be used by other 3D algorithm
        if ( !existFace )
          splitPolygon( newFace, vTool, iFacet, faceID, solidID,
                        face, editor, i+1 < bndFacets.size() );
      }
      else
      {
        if ( faceID )
          meshDS->SetMeshElementOnShape( newFace, faceID );
        else
          meshDS->SetMeshElementOnShape( newFace, solidID );
      }
    } // loop on bndFacets
  } // loop on boundaryVolumes


  // Orient coherently mesh faces on INTERNAL FACEs

  if ( hasInternal )
  {
    TopExp_Explorer faceExp( _grid->_geometry._mainShape, TopAbs_FACE );
    for ( ; faceExp.More(); faceExp.Next() )
    {
      if ( faceExp.Current().Orientation() != TopAbs_INTERNAL )
        continue;

      SMESHDS_SubMesh* sm = meshDS->MeshElements( faceExp.Current() );
      if ( !sm ) continue;

      TIDSortedElemSet facesToOrient;
      for ( SMDS_ElemIteratorPtr fIt = sm->GetElements(); fIt->more(); )
        facesToOrient.insert( facesToOrient.end(), fIt->next() );
      if ( facesToOrient.size() < 2 )
        continue;

      gp_Dir direction(1,0,0);
      TIDSortedElemSet refFaces;
      editor.Reorient2D( facesToOrient, direction, refFaces, /*allowNonManifold=*/true );
    }
  }
  return;
}

//================================================================================
/*!
  * \brief Create mesh segments.
  */
void Hexahedron::addSegments( SMESH_MesherHelper&                      helper,
                              const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap )
{
  SMESHDS_Mesh* mesh = helper.GetMeshDS();

  std::vector<const SMDS_MeshNode*> nodes;
  std::vector<const SMDS_MeshElement *> elems;
  map< TGeomID, vector< TGeomID > >::const_iterator e2ff = edge2faceIDsMap.begin();
  for ( ; e2ff != edge2faceIDsMap.end(); ++e2ff )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( _grid->Shape( e2ff->first ));
    const TopoDS_Face& face = TopoDS::Face( _grid->Shape( e2ff->second[0] ));
    StdMeshers_FaceSide side( face, edge, helper.GetMesh(), /*isFwd=*/true, /*skipMed=*/true );
    nodes = side.GetOrderedNodes();

    elems.clear();
    if ( nodes.size() == 2 )
      // check that there is an element connecting two nodes
      if ( !mesh->GetElementsByNodes( nodes, elems ))
        continue;

    for ( size_t i = 1; i < nodes.size(); i++ )
    {
      if ( mesh->FindEdge( nodes[i-1], nodes[i] ))
        continue;
      SMDS_MeshElement* segment = mesh->AddEdge( nodes[i-1], nodes[i] );
      mesh->SetMeshElementOnShape( segment, e2ff->first );
    }
  }
  return;
}

//================================================================================
/*!
  * \brief Return created volumes and volumes that can have free facet because of
  *        skipped small volume. Also create mesh faces on free facets
  *        of adjacent not-cut volumes if the result volume is too small.
  */
void Hexahedron::getBoundaryElems( vector< const SMDS_MeshElement* > & boundaryElems )
{
  if ( _hasTooSmall /*|| _volumeDefs.IsEmpty()*/ )
  {
    // create faces around a missing small volume
    TGeomID faceID = 0;
    SMESH_MeshEditor editor( _grid->_helper->GetMesh() );
    SMESH_MeshEditor::ElemFeatures polygon( SMDSAbs_Face );
    SMESHDS_Mesh* meshDS = _grid->_helper->GetMeshDS();
    std::vector<const SMDS_MeshElement *> adjVolumes(2);
    for ( size_t iF = 0; iF < _polygons.size(); ++iF )
    {
      const size_t nbLinks = _polygons[ iF ]._links.size();
      if ( nbLinks != 4 ) continue;
      polygon.myNodes.resize( nbLinks );
      polygon.myNodes.back() = 0;
      for ( size_t iL = 0, iN = nbLinks - 1; iL < nbLinks; ++iL, --iN )
        if ( ! ( polygon.myNodes[iN] = _polygons[ iF ]._links[ iL ].FirstNode()->Node() ))
          break;
      if ( !polygon.myNodes.back() )
        continue;

      meshDS->GetElementsByNodes( polygon.myNodes, adjVolumes, SMDSAbs_Volume );
      if ( adjVolumes.size() != 1 )
        continue;
      if ( !adjVolumes[0]->isMarked() )
      {
        boundaryElems.push_back( adjVolumes[0] );
        adjVolumes[0]->setIsMarked( true );
      }

      bool sameShape = true;
      TGeomID shapeID = polygon.myNodes[0]->GetShapeID();
      for ( size_t i = 1; i < polygon.myNodes.size() && sameShape; ++i )
        sameShape = ( shapeID == polygon.myNodes[i]->GetShapeID() );

      if ( !sameShape || !_grid->IsSolid( shapeID ))
        continue; // some of shapes must be FACE

      if ( !faceID )
      {
        faceID = getAnyFace();
        if ( !faceID )
          break;
        if ( _grid->IsInternal( faceID ) ||
              _grid->IsShared( faceID ) //||
              //_grid->IsBoundaryFace( faceID ) -- commented for #19887
              ) 
          break; // create only if a new face will be used by other 3D algo
      }

      Solid * solid = _grid->GetOneOfSolids( adjVolumes[0]->GetShapeID() );
      if ( !solid->IsOutsideOriented( faceID ))
        std::reverse( polygon.myNodes.begin(), polygon.myNodes.end() );

      //polygon.SetPoly( polygon.myNodes.size() > 4 );
      const SMDS_MeshElement* newFace = editor.AddElement( polygon.myNodes, polygon );
      meshDS->SetMeshElementOnShape( newFace, faceID );
    }
  }

  // return created volumes
  for ( _volumeDef* volDef = &_volumeDefs; volDef; volDef = volDef->_next )
  {
    if ( volDef ->_volume &&
          !volDef->_volume->IsNull() &&
          !volDef->_volume->isMarked() )
    {
      volDef->_volume->setIsMarked( true );
      boundaryElems.push_back( volDef->_volume );

      if ( _grid->IsToCheckNodePos() ) // un-mark nodes marked in addVolumes()
        for ( size_t iN = 0; iN < volDef->_nodes.size(); ++iN )
          volDef->_nodes[iN].Node()->setIsMarked( false );
    }
    if ( volDef->_brotherVolume.size() > 0 )
    {
      for (auto _bro : volDef->_brotherVolume )
      {
        _bro->setIsMarked( true );
        boundaryElems.push_back( _bro );
      }        
    }
  }
}

//================================================================================
/*!
  * \brief Remove edges and nodes dividing a hexa side in the case if an adjacent
  *        volume also sharing the dividing edge is missing due to its small side.
  *        Issue #19887.
  */
//================================================================================

void Hexahedron::removeExcessSideDivision(const vector< Hexahedron* >& allHexa)
{
  if ( ! _volumeDefs.IsPolyhedron() )
    return; // not a polyhedron
    
  // look for a divided side adjacent to a small hexahedron

  int di[6] = { 0, 0, 0, 0,-1, 1 };
  int dj[6] = { 0, 0,-1, 1, 0, 0 };
  int dk[6] = {-1, 1, 0, 0, 0, 0 };

  for ( int iF = 0; iF < 6; ++iF ) // loop on 6 sides of a hexahedron
  {
    size_t neighborIndex = _grid->CellIndex( _i + di[iF],
                                              _j + dj[iF],
                                              _k + dk[iF] );
    if ( neighborIndex >= allHexa.size() ||
          !allHexa[ neighborIndex ]       ||
          !allHexa[ neighborIndex ]->_hasTooSmall )
      continue;

    // check if a side is divided into several polygons
    for ( _volumeDef* volDef = &_volumeDefs; volDef; volDef = volDef->_next )
    {
      int nbPolygons = 0, nbNodes = 0;
      for ( size_t i = 0; i < volDef->_names.size(); ++i )
        if ( volDef->_names[ i ] == _hexQuads[ iF ]._name )
        {
          ++nbPolygons;
          nbNodes += volDef->_quantities[ i ];
        }
      if ( nbPolygons < 2 )
        continue;

      // construct loops from polygons
      typedef _volumeDef::_linkDef TLinkDef;
      std::vector< TLinkDef* > loops;
      std::vector< TLinkDef > links( nbNodes );
      for ( size_t i = 0, iN = 0, iLoop = 0; iLoop < volDef->_quantities.size(); ++iLoop )
      {
        size_t nbLinks = volDef->_quantities[ iLoop ];
        if ( volDef->_names[ iLoop ] != _hexQuads[ iF ]._name )
        {
          iN += nbLinks;
          continue;
        }
        loops.push_back( & links[i] );
        for ( size_t n = 0; n < nbLinks-1; ++n, ++i, ++iN )
        {
          links[i].init( volDef->_nodes[iN], volDef->_nodes[iN+1], iLoop );
          links[i].setNext( &links[i+1] );
        }
        links[i].init( volDef->_nodes[iN], volDef->_nodes[iN-nbLinks+1], iLoop );
        links[i].setNext( &links[i-nbLinks+1] );
        ++i; ++iN;
      }

      // look for equal links in different loops and join such loops
      bool loopsJoined = false;
      std::set< TLinkDef > linkSet;
      for ( size_t iLoop = 0; iLoop < loops.size(); ++iLoop )
      {
        TLinkDef* beg = 0;
        for ( TLinkDef* l = loops[ iLoop ]; l != beg; l = l->_next ) // walk around the iLoop
        {
          std::pair< std::set< TLinkDef >::iterator, bool > it2new = linkSet.insert( *l );
          if ( !it2new.second ) // equal found, join loops
          {
            const TLinkDef* equal = &(*it2new.first);
            if ( equal->_loopIndex == l->_loopIndex )
              continue; // error?

            loopsJoined = true;

            for ( size_t i = iLoop - 1; i < loops.size(); --i )
              if ( loops[ i ] && loops[ i ]->_loopIndex == equal->_loopIndex )
                loops[ i ] = 0;

            // exclude l and equal and join two loops
            if ( l->_prev != equal )
              l->_prev->setNext( equal->_next );
            if ( equal->_prev != l )
              equal->_prev->setNext( l->_next );

            if ( volDef->_quantities[ l->_loopIndex ] > 0 )
              volDef->_quantities[ l->_loopIndex     ] *= -1;
            if ( volDef->_quantities[ equal->_loopIndex ] > 0 )
              volDef->_quantities[ equal->_loopIndex ] *= -1;

            if ( loops[ iLoop ] == l )
              loops[ iLoop ] = l->_prev->_next;
          }
          beg = loops[ iLoop ];
        }
      }
      // update volDef
      if ( loopsJoined )
      {
        // set unchanged polygons
        std::vector< int >             newQuantities;
        std::vector< _volumeDef::_nodeDef > newNodes;
        vector< SMESH_Block::TShapeID >     newNames;
        newQuantities.reserve( volDef->_quantities.size() );
        newNodes.reserve     ( volDef->_nodes.size() );
        newNames.reserve     ( volDef->_names.size() );
        for ( size_t i = 0, iLoop = 0; iLoop < volDef->_quantities.size(); ++iLoop )
        {
          if ( volDef->_quantities[ iLoop ] < 0 )
          {
            i -= volDef->_quantities[ iLoop ];
            continue;
          }
          newQuantities.push_back( volDef->_quantities[ iLoop ]);
          newNodes.insert( newNodes.end(),
                            volDef->_nodes.begin() + i,
                            volDef->_nodes.begin() + i + newQuantities.back() );
          newNames.push_back( volDef->_names[ iLoop ]);
          i += volDef->_quantities[ iLoop ];
        }

        // set joined loops
        for ( size_t iLoop = 0; iLoop < loops.size(); ++iLoop )
        {
          if ( !loops[ iLoop ] )
            continue;
          newQuantities.push_back( 0 );
          TLinkDef* beg = 0;
          for ( TLinkDef* l = loops[ iLoop ]; l != beg; l = l->_next, ++newQuantities.back() )
          {
            newNodes.push_back( l->_node1 );
            beg = loops[ iLoop ];
          }
          newNames.push_back( _hexQuads[ iF ]._name );
        }
        volDef->_quantities.swap( newQuantities );
        volDef->_nodes.swap( newNodes );
        volDef->_names.swap( newNames );
      }
    } // loop on volDef's
  } // loop on hex sides

  return;
} // removeExcessSideDivision()


//================================================================================
/*!
  * \brief Remove nodes splitting Cartesian cell edges in the case if a node
  *        is used in every cells only by two polygons sharing the edge
  *        Issue #19887.
  */
//================================================================================

void Hexahedron::removeExcessNodes(vector< Hexahedron* >& allHexa)
{
  if ( ! _volumeDefs.IsPolyhedron() )
    return; // not a polyhedron

  typedef vector< _volumeDef::_nodeDef >::iterator TNodeIt;
  vector< int > nodesInPoly[ 4 ]; // node index in _volumeDefs._nodes
  vector< int > volDefInd  [ 4 ]; // index of a _volumeDefs
  Hexahedron*   hexa       [ 4 ];
  int i,j,k, cellIndex, iLink = 0, iCellLink;
  for ( int iDir = 0; iDir < 3; ++iDir )
  {
    CellsAroundLink fourCells( _grid, iDir );
    for ( int iL = 0; iL < 4; ++iL, ++iLink ) // 4 links in a direction
    {
      _Link& link = _hexLinks[ iLink ];
      fourCells.Init( _i, _j, _k, iLink );

      for ( size_t iP = 0; iP < link._fIntPoints.size(); ++iP ) // loop on nodes on the link
      {
        bool nodeRemoved = true;
        _volumeDef::_nodeDef node; node._intPoint = link._fIntPoints[iP];

        for ( size_t i = 0, nb = _volumeDefs.size(); i < nb &&  nodeRemoved; ++i )
          if ( _volumeDef* vol = _volumeDefs.at( i ))
            nodeRemoved =
              ( std::find( vol->_nodes.begin(), vol->_nodes.end(), node ) == vol->_nodes.end() );
        if ( nodeRemoved )
          continue; // node already removed

        // check if a node encounters zero or two times in 4 cells sharing iLink
        // if so, the node can be removed from the cells
        bool       nodeIsOnEdge = true;
        int nbPolyhedraWithNode = 0;
        for ( int iC = 0; iC < 4; ++iC ) // loop on 4 cells sharing a link
        {
          nodesInPoly[ iC ].clear();
          volDefInd  [ iC ].clear();
          hexa       [ iC ] = 0;
          if ( !fourCells.GetCell( iC, i,j,k, cellIndex, iCellLink ))
            continue;
          hexa[ iC ] = allHexa[ cellIndex ];
          if ( !hexa[ iC ])
            continue;
          for ( size_t i = 0, nb = hexa[ iC ]->_volumeDefs.size(); i < nb; ++i )
            if ( _volumeDef* vol = hexa[ iC ]->_volumeDefs.at( i ))
            {
              for ( TNodeIt nIt = vol->_nodes.begin(); nIt != vol->_nodes.end(); ++nIt )
              {
                nIt = std::find( nIt, vol->_nodes.end(), node );
                if ( nIt != vol->_nodes.end() )
                {
                  nodesInPoly[ iC ].push_back( std::distance( vol->_nodes.begin(), nIt ));
                  volDefInd  [ iC ].push_back( i );
                }
                else
                  break;
              }
              nbPolyhedraWithNode += ( !nodesInPoly[ iC ].empty() );
            }
          if ( nodesInPoly[ iC ].size() != 0 &&
                nodesInPoly[ iC ].size() != 2 )
          {
            nodeIsOnEdge = false;
            break;
          }
        } // loop  on 4 cells

        // remove nodes from polyhedra
        if ( nbPolyhedraWithNode > 0 && nodeIsOnEdge )
        {
          for ( int iC = 0; iC < 4; ++iC ) // loop on 4 cells sharing the link
          {
            if ( nodesInPoly[ iC ].empty() )
              continue;
            for ( int i = volDefInd[ iC ].size() - 1; i >= 0; --i )
            {
              _volumeDef* vol = hexa[ iC ]->_volumeDefs.at( volDefInd[ iC ][ i ]);
              int nIndex = nodesInPoly[ iC ][ i ];
              // decrement _quantities
              for ( size_t iQ = 0; iQ < vol->_quantities.size(); ++iQ )
                if ( nIndex < vol->_quantities[ iQ ])
                {
                  vol->_quantities[ iQ ]--;
                  break;
                }
                else
                {
                  nIndex -= vol->_quantities[ iQ ];
                }
              vol->_nodes.erase( vol->_nodes.begin() + nodesInPoly[ iC ][ i ]);

              if ( i == 0 &&
                    vol->_nodes.size() == 6 * 4 &&
                    vol->_quantities.size() == 6 ) // polyhedron becomes hexahedron?
              {
                bool allQuads = true;
                for ( size_t iQ = 0; iQ < vol->_quantities.size() &&  allQuads; ++iQ )
                  allQuads = ( vol->_quantities[ iQ ] == 4 );
                if ( allQuads )
                {
                  // set side nodes as this: bottom, top, top, ...
                  int iTop = 0, iBot = 0; // side indices
                  for ( int iS = 0; iS < 6; ++iS )
                  {
                    if ( vol->_names[ iS ] == SMESH_Block::ID_Fxy0 )
                      iBot = iS;
                    if ( vol->_names[ iS ] == SMESH_Block::ID_Fxy1 )
                      iTop = iS;
                  }
                  if ( iBot != 0 )
                  {
                    if ( iTop == 0 )
                    {
                      std::copy( vol->_nodes.begin(),
                                  vol->_nodes.begin() + 4,
                                  vol->_nodes.begin() + 4 );
                      iTop = 1;
                    }
                    std::copy( vol->_nodes.begin() + 4 * iBot,
                                vol->_nodes.begin() + 4 * ( iBot + 1),
                                vol->_nodes.begin() );
                  }
                  if ( iTop != 1 )
                    std::copy( vol->_nodes.begin() + 4 * iTop,
                                vol->_nodes.begin() + 4 * ( iTop + 1),
                                vol->_nodes.begin() + 4 );

                  std::copy( vol->_nodes.begin() + 4,
                              vol->_nodes.begin() + 8,
                              vol->_nodes.begin() + 8 );
                  // set up top facet nodes by comparing their uvw with bottom nodes
                  E_IntersectPoint ip[8];
                  for ( int iN = 0; iN < 8; ++iN )
                  {
                    SMESH_NodeXYZ p = vol->_nodes[ iN ].Node();
                    _grid->ComputeUVW( p, ip[ iN ]._uvw );
                  }
                  const double tol2 = _grid->_tol * _grid->_tol;
                  for ( int iN = 0; iN < 4; ++iN )
                  {
                    gp_Pnt2d pBot( ip[ iN ]._uvw[0], ip[ iN ]._uvw[1] );
                    for ( int iT = 4; iT < 8; ++iT )
                    {
                      gp_Pnt2d pTop( ip[ iT ]._uvw[0], ip[ iT ]._uvw[1] );
                      if ( pBot.SquareDistance( pTop ) < tol2 )
                      {
                        // vol->_nodes[ iN + 4 ]._node = ip[ iT ]._node;
                        // vol->_nodes[ iN + 4 ]._intPoint = 0;
                        vol->_nodes[ iN + 4 ] = vol->_nodes[ iT + 4 ];
                        break;
                      }
                    }
                  }
                  vol->_nodes.resize( 8 );
                  vol->_quantities.clear();
                  //vol->_names.clear();
                }
              }
            } // loop on _volumeDefs
          } // loop on 4 cell abound a link
        } // if ( nodeIsOnEdge )
      } // loop on intersection points of a link
    } // loop on 4 links of a direction
  } // loop on 3 directions

  return;

} // removeExcessNodes()

//================================================================================
/*!
  * \brief [Issue #19913] Modify _hexLinks._splits to prevent creating overlapping volumes
  */
//================================================================================

void Hexahedron::preventVolumesOverlapping()
{
  // Cut off a quadrangle corner if two links sharing the corner
  // are shared by same two solids, in this case each of solids gets
  // a triangle for it-self.
  std::vector< TGeomID > soIDs[4];
  for ( int iF = 0; iF < 6; ++iF ) // loop on 6 sides of a hexahedron
  {
    _Face& quad = _hexQuads[ iF ] ;

    int iFOpposite = iF + ( iF % 2 ? -1 : 1 );
    _Face& quadOpp = _hexQuads[ iFOpposite ] ;

    int nbSides = 0, nbSidesOpp = 0;
    for ( int iE = 0; iE < 4; ++iE ) // loop on 4 sides of a quadrangle
    {
      nbSides    += ( quad._links   [ iE ].NbResultLinks() > 0 );
      nbSidesOpp += ( quadOpp._links[ iE ].NbResultLinks() > 0 );
    }
    if ( nbSides < 4 || nbSidesOpp != 2 )
      continue;

    for ( int iE = 0; iE < 4; ++iE )
    {
      soIDs[ iE ].clear();
      _Node* n = quad._links[ iE ].FirstNode();
      if ( n->_intPoint && n->_intPoint->_faceIDs.size() )
        soIDs[ iE ] = _grid->GetSolidIDs( n->_intPoint->_faceIDs[0] );
    }
    if ((( soIDs[0].size() >= 2 ) +
          ( soIDs[1].size() >= 2 ) +
          ( soIDs[2].size() >= 2 ) +
          ( soIDs[3].size() >= 2 ) ) < 3 )
      continue;

    bool done = false;
    for ( int i = 0; i < 4; ++i )
    {
      int i1 = _grid->_helper->WrapIndex( i + 1, 4 );
      int i2 = _grid->_helper->WrapIndex( i + 2, 4 );
      int i3 = _grid->_helper->WrapIndex( i + 3, 4 );
      if ( soIDs[i1].size() == 2 && soIDs[i ] != soIDs[i1] &&
            soIDs[i2].size() == 2 && soIDs[i1] == soIDs[i2] &&
            soIDs[i3].size() == 2 && soIDs[i2] == soIDs[i3] )
      {
        quad._links[ i1 ]._link->_splits.clear();
        quad._links[ i2 ]._link->_splits.clear();
        done = true;
        break;
      }
    }
    if ( done )
      break;
  }
  return;
} // preventVolumesOverlapping()

//================================================================================
/*!
  * \brief Set to _hexLinks a next portion of splits located on one side of INTERNAL FACEs
  */
bool Hexahedron::_SplitIterator::Next()
{
  if ( _iterationNb > 0 )
    // count used splits
    for ( size_t i = 0; i < _splits.size(); ++i )
    {
      if ( _splits[i]._iCheckIteration == _iterationNb )
      {
        _splits[i]._isUsed = _splits[i]._checkedSplit->_faces[1];
        _nbUsed += _splits[i]._isUsed;
      }
      if ( !More() )
        return false;
    }

  ++_iterationNb;

  bool toTestUsed = ( _nbChecked >= _splits.size() );
  if ( toTestUsed )
  {
    // all splits are checked; find all not used splits
    for ( size_t i = 0; i < _splits.size(); ++i )
      if ( !_splits[i].IsCheckedOrUsed( toTestUsed ))
        _splits[i]._iCheckIteration = _iterationNb;

    _nbUsed = _splits.size(); // to stop iteration
  }
  else
  {
    // get any not used/checked split to start from
    _freeNodes.clear();
    for ( size_t i = 0; i < _splits.size(); ++i )
    {
      if ( !_splits[i].IsCheckedOrUsed( toTestUsed ))
      {
        _freeNodes.push_back( _splits[i]._nodes[0] );
        _freeNodes.push_back( _splits[i]._nodes[1] );
        _splits[i]._iCheckIteration = _iterationNb;
        break;
      }
    }
    // find splits connected to the start one via _freeNodes
    for ( size_t iN = 0; iN < _freeNodes.size(); ++iN )
    {
      for ( size_t iS = 0; iS < _splits.size(); ++iS )
      {
        if ( _splits[iS].IsCheckedOrUsed( toTestUsed ))
          continue;
        int iN2 = -1;
        if (      _freeNodes[iN] == _splits[iS]._nodes[0] )
          iN2 = 1;
        else if ( _freeNodes[iN] == _splits[iS]._nodes[1] )
          iN2 = 0;
        else
          continue;
        if ( _freeNodes[iN]->_isInternalFlags > 0 )
        {
          if ( _splits[iS]._nodes[ iN2 ]->_isInternalFlags == 0 )
            continue;
          if ( !_splits[iS]._nodes[ iN2 ]->IsLinked( _freeNodes[iN]->_intPoint ))
            continue;
        }
        _splits[iS]._iCheckIteration = _iterationNb;
        _freeNodes.push_back( _splits[iS]._nodes[ iN2 ]);
      }
    }
  }
  // set splits to hex links

  for ( int iL = 0; iL < 12; ++iL )
    _hexLinks[ iL ]._splits.clear();

  _Link split;
  for ( size_t i = 0; i < _splits.size(); ++i )
  {
    if ( _splits[i]._iCheckIteration == _iterationNb )
    {
      split._nodes[0] = _splits[i]._nodes[0];
      split._nodes[1] = _splits[i]._nodes[1];
      _Link & hexLink = _hexLinks[ _splits[i]._linkID ];
      hexLink._splits.push_back( split );
      _splits[i]._checkedSplit = & hexLink._splits.back();
      ++_nbChecked;
    }
  }
  return More();
}