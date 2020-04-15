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
// File      : SMESH_Delaunay.cxx
// Created   : Wed Apr 19 15:41:15 2017
// Author    : Edward AGAPOV (eap)

#include "SMESH_Delaunay.hxx"

#include "SMESH_Comment.hxx"
#include "SMESH_File.hxx"
#include "SMESH_MeshAlgos.hxx"

#include <BRepAdaptor_Surface.hxx>
#include <BRepMesh_Delaun.hxx>

#include <Basics_OCCTVersion.hxx>

//================================================================================
/*!
 * \brief Construct a Delaunay triangulation of given boundary nodes
 *  \param [in] boundaryNodes - vector of nodes of a wire
 *  \param [in] face - the face
 *  \param [in] faceID - the face ID
 */
//================================================================================

SMESH_Delaunay::SMESH_Delaunay(const std::vector< const UVPtStructVec* > & boundaryNodes,
                               const TopoDS_Face&                          face,
                               const int                                   faceID)
  : _face( face ), _faceID( faceID ), _scale( 1., 1. )
{
  // compute _scale
  BRepAdaptor_Surface surf( face );
  if ( surf.GetType() != GeomAbs_Plane )
  {
    const int nbDiv = 100;
    const double uRange = surf.LastUParameter() - surf.FirstUParameter();
    const double vRange = surf.LastVParameter() - surf.FirstVParameter();
    const double uFixed = surf.FirstUParameter() + 0.5 * uRange;
    const double vFixed = surf.FirstVParameter() + 0.5 * vRange;
    const double dU = uRange / nbDiv;
    const double dV = vRange / nbDiv;
    double u = surf.FirstUParameter(), v = surf.FirstVParameter();
    gp_Pnt p0U = surf.Value( u, v ), p0V = p0U;
    double lenU = 0, lenV = 0;
    for ( ; u < surf.LastUParameter(); u += dU, v += dV )
    {
      gp_Pnt p1U = surf.Value( u, vFixed );
      lenU += p1U.Distance( p0U );
      p0U = p1U;
      gp_Pnt p1V = surf.Value( uFixed, v );
      lenV += p1V.Distance( p0V );
      p0V = p1V;
    }
    _scale.SetCoord( lenU / uRange, lenV / vRange );
  }

  // count boundary points
  int iP = 1, nbP = 0;
  for ( size_t iW = 0; iW < boundaryNodes.size(); ++iW ) // loop on wires
  {
    nbP += boundaryNodes[iW]->size();
    if ( boundaryNodes[iW]->front().node == boundaryNodes[iW]->back().node )
      --nbP; // 1st and last points coincide
  }
  _bndNodes.resize( nbP );

  // fill boundary points
#if OCC_VERSION_LARGE <= 0x07030000
  BRepMesh::Array1OfVertexOfDelaun bndVert( 1, 1 + nbP );
#else
  IMeshData::Array1OfVertexOfDelaun bndVert( 1, 1 + nbP );
#endif
  BRepMesh_Vertex v( 0, 0, BRepMesh_Frontier );
  for ( size_t iW = 0; iW < boundaryNodes.size(); ++iW )
  {
    const UVPtStructVec& bndPnt = *boundaryNodes[iW];
    int i = 0, nb = bndPnt.size();
    if ( bndPnt[0].node == bndPnt.back().node )
      --nb;
    for ( ; i < nb;  ++i, ++iP )
    {
      _bndNodes[ iP-1 ] = bndPnt[i].node;
      bndPnt[i].node->setIsMarked( true );

      v.ChangeCoord() = bndPnt[i].UV().Multiplied( _scale );
      bndVert( iP )   = v;
    }
  }

  // triangulate the srcFace in 2D
  BRepMesh_Delaun Delaunay( bndVert );
  _triaDS = Delaunay.Result();
}

//================================================================================
/*!
 * \brief Prepare to the exploration of nodes
 */
//================================================================================

void SMESH_Delaunay::InitTraversal(const int nbNodesToVisit)
{
  _nbNodesToVisit = (size_t) nbNodesToVisit;
  _nbVisitedNodes = _iBndNode = 0;
  _noTriQueue.clear();
}

//================================================================================
/*!
 * \brief Return a node with its Barycentric Coordinates within the triangle
 *        defined by its node indices (zero based)
 *  \param [out] bc - Barycentric Coordinates of the returned node
 *  \param [out] triaNodes - indices of triangle nodes
 *  \return const SMDS_MeshNode* - the next node or NULL
 */
//================================================================================

const SMDS_MeshNode* SMESH_Delaunay::NextNode( double bc[3], int triaNodes[3] )
{
  while ( _nbVisitedNodes < _nbNodesToVisit )
  {
    while ( !_noTriQueue.empty() )
    {
      const SMDS_MeshNode*     node = _noTriQueue.front().first;
      const BRepMesh_Triangle* tria = _noTriQueue.front().second;
      _noTriQueue.pop_front();
      if ( node->isMarked() )
        continue;
      ++_nbVisitedNodes;
      node->setIsMarked( true );

      // find a Delaunay triangle containing the src node
      gp_XY uv = getNodeUV( _face, node );
      tria = FindTriangle( uv, tria, bc, triaNodes );
      if ( tria )
      {
        addCloseNodes( node, tria, _faceID, _noTriQueue );
        return node;
      }
    }
    for ( ; _iBndNode < _bndNodes.size() &&  _noTriQueue.empty();  ++_iBndNode )
    {
      if ( const BRepMesh_Triangle* tria = GetTriangleNear( _iBndNode ))
        addCloseNodes( _bndNodes[ _iBndNode ], tria, _faceID, _noTriQueue );
    }
    if ( _noTriQueue.empty() )
      break;
  }

  // if ( _nbVisitedNodes < _nbNodesToVisit )
  //   _nbVisitedNodes = std::numeric_limits<int>::max();
  return NULL;
}

//================================================================================
/*!
 * \brief Find a Delaunay triangle containing a given 2D point and return
 *        barycentric coordinates within the found triangle
 */
//================================================================================

const BRepMesh_Triangle* SMESH_Delaunay::FindTriangle( const gp_XY&             UV,
                                                       const BRepMesh_Triangle* tria,
                                                       double                   bc[3],
                                                       int                      triaNodes[3] )
{
  int   nodeIDs[3];
  gp_XY nodeUVs[3];
  int   linkIDs[3];
  Standard_Boolean ori[3];

  gp_XY uv = UV.Multiplied( _scale );

  while ( tria )
  {
    // check if the uv is in tria

    _triaDS->ElementNodes( *tria, nodeIDs );
    nodeUVs[0] = _triaDS->GetNode( nodeIDs[0] ).Coord();
    nodeUVs[1] = _triaDS->GetNode( nodeIDs[1] ).Coord();
    nodeUVs[2] = _triaDS->GetNode( nodeIDs[2] ).Coord();

    SMESH_MeshAlgos::GetBarycentricCoords( uv,
                                           nodeUVs[0], nodeUVs[1], nodeUVs[2],
                                           bc[0], bc[1] );
    if ( bc[0] >= 0 && bc[1] >= 0 && bc[0] + bc[1] <= 1 )
    {
      if ( _triaDS->GetNode( nodeIDs[0] ).Movability() != BRepMesh_Frontier ||
           _triaDS->GetNode( nodeIDs[1] ).Movability() != BRepMesh_Frontier ||
           _triaDS->GetNode( nodeIDs[2] ).Movability() != BRepMesh_Frontier )
      {
        return 0;
      }
      bc[2] = 1 - bc[0] - bc[1];
      triaNodes[0] = nodeIDs[0] - 1;
      triaNodes[1] = nodeIDs[1] - 1;
      triaNodes[2] = nodeIDs[2] - 1;
      return tria;
    }

    // look for a neighbor triangle, which is adjacent to a link intersected
    // by a segment( triangle center -> uv )

    gp_XY gc = ( nodeUVs[0] + nodeUVs[1] + nodeUVs[2] ) / 3.;
    gp_XY seg = uv - gc;

    tria->Edges( linkIDs, ori );

    const BRepMesh_Triangle* prevTria = tria;
    tria = 0;

    for ( int i = 0; i < 3; ++i )
    {
      const BRepMesh_PairOfIndex & triIDs = _triaDS->ElementsConnectedTo( linkIDs[i] );
      if ( triIDs.Extent() < 2 )
        continue; // no neighbor triangle

      // check if a link intersects gc2uv
      const BRepMesh_Edge & link = _triaDS->GetLink( linkIDs[i] );
      const BRepMesh_Vertex & n1 = _triaDS->GetNode( link.FirstNode() );
      const BRepMesh_Vertex & n2 = _triaDS->GetNode( link.LastNode() );
      gp_XY uv1 = n1.Coord();
      gp_XY lin = n2.Coord() - uv1; // link direction

      double crossSegLin = seg ^ lin;
      if ( Abs( crossSegLin ) < std::numeric_limits<double>::min() )
        continue; // parallel

      double uSeg = ( uv1 - gc ) ^ lin / crossSegLin;
      if ( 0. <= uSeg && uSeg <= 1. )
      {
        tria = & _triaDS->GetElement( triIDs.Index( 1 ));
        if ( tria == prevTria )
          tria = & _triaDS->GetElement( triIDs.Index( 2 ));
        if ( tria->Movability() != BRepMesh_Deleted )
          break;
      }
    }
  }
  return tria;
}

//================================================================================
/*!
 * \brief Return a triangle sharing a given boundary node
 *  \param [in] iBndNode - index of the boundary node
 *  \return const BRepMesh_Triangle* - a found triangle
 */
//================================================================================

const BRepMesh_Triangle* SMESH_Delaunay::GetTriangleNear( int iBndNode )
{
  if ( iBndNode >= _triaDS->NbNodes() )
    return 0;
  int nodeIDs[3];
  int nbNbNodes = _bndNodes.size();
#if OCC_VERSION_LARGE <= 0x07030000
  typedef BRepMesh::ListOfInteger TLinkList;
#else
  typedef IMeshData::ListOfInteger TLinkList;
#endif
  const TLinkList &       linkIds = _triaDS->LinksConnectedTo( iBndNode + 1 );
  TLinkList::const_iterator iLink = linkIds.cbegin();
  for ( ; iLink != linkIds.cend(); ++iLink )
  {
    const BRepMesh_PairOfIndex & triaIds = _triaDS->ElementsConnectedTo( *iLink );
    {
      const BRepMesh_Triangle& tria = _triaDS->GetElement( triaIds.Index(1) );
      if ( tria.Movability() != BRepMesh_Deleted )
      {
        _triaDS->ElementNodes( tria, nodeIDs );
        if ( nodeIDs[0]-1 < nbNbNodes &&
             nodeIDs[1]-1 < nbNbNodes &&
             nodeIDs[2]-1 < nbNbNodes )
          return &tria;
      }
    }
    if ( triaIds.Extent() > 1 )
    {
      const BRepMesh_Triangle& tria = _triaDS->GetElement( triaIds.Index(2) );
      if ( tria.Movability() != BRepMesh_Deleted )
      {
        _triaDS->ElementNodes( tria, nodeIDs );
        if ( nodeIDs[0]-1 < nbNbNodes &&
             nodeIDs[1]-1 < nbNbNodes &&
             nodeIDs[2]-1 < nbNbNodes )
          return &tria;
      }
    }
  }
  return 0;
}

//================================================================================
/*!
 * \brief Return UV of the i-th source boundary node (zero based)
 */
//================================================================================

gp_XY SMESH_Delaunay::GetBndUV(const int iNode) const
{
  return _triaDS->GetNode( iNode+1 ).Coord();
}

//================================================================================
/*!
 * \brief Add non-marked nodes surrounding a given one to a queue
 */
//================================================================================

void SMESH_Delaunay::addCloseNodes( const SMDS_MeshNode*     node,
                                    const BRepMesh_Triangle* tria,
                                    const int                faceID,
                                    TNodeTriaList &          _noTriQueue )
{
  // find in-FACE nodes
  SMDS_ElemIteratorPtr elems = node->GetInverseElementIterator(SMDSAbs_Face);
  while ( elems->more() )
  {
    const SMDS_MeshElement* elem = elems->next();
    if ( elem->getshapeId() == faceID )
    {
      for ( int i = 0, nb = elem->NbNodes(); i < nb; ++i )
      {
        const SMDS_MeshNode* n = elem->GetNode( i );
        if ( !n->isMarked() /*&& n->getshapeId() == faceID*/ )
          _noTriQueue.push_back( std::make_pair( n, tria ));
      }
    }
  }
}

//================================================================================
/*!
 * \brief Write a python script that creates an equal mesh in Mesh module
 */
//================================================================================

void SMESH_Delaunay::ToPython() const
{
  SMESH_Comment text;
  text << "import salome, SMESH\n";
  text << "salome.salome_init()\n";
  text << "from salome.smesh import smeshBuilder\n";
  text << "smesh = smeshBuilder.New()\n";
  text << "mesh=smesh.Mesh()\n";
  const char* endl = "\n";

  for ( int i = 0; i < _triaDS->NbNodes(); ++i )
  {
    const BRepMesh_Vertex& v = _triaDS->GetNode( i+1 );
    text << "mesh.AddNode( " << v.Coord().X() << ", " << v.Coord().Y() << ", 0 )" << endl;
  }

  int nodeIDs[3];
  for ( int i = 0; i < _triaDS->NbElements(); ++i )
  {
    const BRepMesh_Triangle& t = _triaDS->GetElement( i+1 );
    if ( t.Movability() == BRepMesh_Deleted )
      continue;
    _triaDS->ElementNodes( t, nodeIDs );
    text << "mesh.AddFace([ " << nodeIDs[0] << ", " << nodeIDs[1] << ", " << nodeIDs[2] << " ])" << endl;
  }

  const char* fileName = "/tmp/Delaunay.py";
  SMESH_File file( fileName, false );
  file.remove();
  file.openForWriting();
  file.write( text.c_str(), text.size() );
  std::cout << "exec(open('" << fileName << "', 'rb').read())";
}
