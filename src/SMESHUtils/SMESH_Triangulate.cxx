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
// File      : SMESH_Triangulate.cxx
// Created   : Thu Jan 18 18:00:13 2018
// Author    : Edward AGAPOV (eap)

// Extracted from ../DriverSTL/DriverSTL_W_SMDS_Mesh.cxx

#include "SMESH_MeshAlgos.hxx"

#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <gp_Ax2.hxx>

#include <boost/container/flat_set.hpp>

using namespace SMESH_MeshAlgos;

namespace
{
  struct Node // node of a triangle
  {
    size_t _triaIndex; // triangle index == index of the 1st triangle node in triangulation array
    size_t _nodeIndex; // node index within triangle [0-2]

    //! return node index within the node array
    size_t Index() const { return  _triaIndex + _nodeIndex; }

    //! return local 3-d index [0-2]
    static size_t ThirdIndex( size_t i1, size_t i2 )
    {
      size_t i3 = ( i2 + 1 ) % 3;
      if ( i3 == i1 )
        i3 = ( i2 + 2 ) % 3;
      return i3;
    }
    //! return 3-d node index within the node array
    static size_t ThirdIndex( const Node& n1, const Node& n2 )
    {
      return n1._triaIndex + ThirdIndex( n1._nodeIndex, n2._nodeIndex );
    }
    bool operator<(const Node& other) const { return _triaIndex < other._triaIndex; }
  };
  typedef boost::container::flat_set< Node >                 TriaNodeSet;

}
/*!
 * \brief Vertex of a polygon. Together with 2 neighbor Vertices represents a triangle
 */
struct Triangulate::PolyVertex
{
  SMESH_NodeXYZ _nxyz;
  size_t        _index;
  gp_XY         _xy;
  PolyVertex*   _prev;
  PolyVertex*   _next;

  void   SetNodeAndNext( const SMDS_MeshNode* n, PolyVertex& v, size_t index );
  void   GetTriaNodes( const SMDS_MeshNode** nodes, size_t* nodeIndices) const;
  double TriaArea() const;
  bool   IsInsideTria( const PolyVertex* v );
  PolyVertex* Delete();

  // compare PolyVertex'es by node
  bool operator()(const PolyVertex* a, const PolyVertex* b) const
  {
    return ( a->_nxyz.Node() <  b->_nxyz.Node() );
  }
  // set of PolyVertex sorted by mesh node
  typedef boost::container::flat_set< PolyVertex*, PolyVertex > PVSet;
};

struct Triangulate::Data
{
  std::vector< PolyVertex > _pv;
  std::vector< size_t >     _nodeIndex;
  PolyVertex::PVSet         _uniqueNodePV;
};

struct Triangulate::Optimizer
{
  std::vector< TriaNodeSet > _nodeUsage; // inclusions of a node in triangles

  //================================================================================
  /*!
   * \brief Optimize triangles by edge swapping
   *  \param [inout] nodes - polygon triangulation, i.e. connectivity of all triangles to optimize
   *  \param [in] points - coordinates of nodes of the input polygon
   *  \param [in] nodeIndices - indices of triangulation nodes within the input polygon
   */
  //================================================================================

  void optimize( std::vector< const SMDS_MeshNode*>& nodes,
                 std::vector< PolyVertex > &         points,
                 std::vector< size_t > &             nodeIndices)
  {
    // for each node of the polygon, remember triangles using it
    _nodeUsage.resize( points.size() );
    for ( size_t i = 0; i < points.size(); ++i ) // clear old data
    {
      _nodeUsage[ i ].clear();
    }
    for ( size_t i = 0, iTria = 0; i < nodeIndices.size(); ++iTria )
    {
      _nodeUsage[ nodeIndices[ i++ ]].insert({ iTria * 3, 0 });
      _nodeUsage[ nodeIndices[ i++ ]].insert({ iTria * 3, 1 });
      _nodeUsage[ nodeIndices[ i++ ]].insert({ iTria * 3, 2 });
    }

    // optimization
    for ( size_t iTria = 0; iTria < nodeIndices.size(); iTria += 3 )
    {
      double badness1 = computeBadness( nodeIndices[ iTria + 0 ],
                                        nodeIndices[ iTria + 1 ],
                                        nodeIndices[ iTria + 2 ],
                                        points );
      for ( size_t i = 0; i < 3; ++i ) // loop on triangle edges to find a neighbor triangle
      {
        size_t i1 = iTria + i; // node index in nodeIndices
        size_t i2 = iTria + ( i + 1 ) % 3;
        size_t ind1 = nodeIndices[ i1 ]; // node index in points
        size_t ind2 = nodeIndices[ i2 ];
        TriaNodeSet & usage1 = _nodeUsage[ ind1 ]; // triangles using a node
        TriaNodeSet & usage2 = _nodeUsage[ ind2 ];
        if ( usage1.size() < 2 ||
             usage2.size() < 2 )
          continue;

        // look for another triangle using two nodes
        TriaNodeSet::iterator usIt1 = usage1.begin();
        for ( ; usIt1 != usage1.end(); ++usIt1 )
        {
          if ( usIt1->_triaIndex == iTria )
            continue; // current triangle
          TriaNodeSet::iterator usIt2 = usage2.find( *usIt1 );
          if ( usIt2 == usage2.end() )
            continue; // no common _triaIndex in two usages

          size_t i3 = iTria + ( i + 2 ) % 3;
          size_t i4 = Node::ThirdIndex( *usIt1, *usIt2 ); // 4th node of quadrangle
          size_t ind3 = nodeIndices[ i3 ];
          size_t ind4 = nodeIndices[ i4 ];

          double badness2 = computeBadness( ind2, ind1, ind4, points );
          double badness3 = computeBadness( ind1, ind4, ind3, points, /*checkArea=*/true );
          double badness4 = computeBadness( ind2, ind3, ind4, points, /*checkArea=*/true );

          if ( Max( badness1, badness2 ) < Max( badness3, badness4 ))
            continue;

          // swap edge by modifying nodeIndices

          nodeIndices[ i2 ] = ind4;
          _nodeUsage[ ind4 ].insert({ iTria, i2 - iTria });
          _nodeUsage[ ind2 ].erase ({ iTria, i2 - iTria });

          i1 = usIt1->Index();
          nodeIndices[ i1 ] = ind3;
          _nodeUsage[ ind3 ].insert( *usIt1 );
          _nodeUsage[ ind1 ].erase ( *usIt1 );

          --i; // to re-check a current edge
          badness1 = badness3;
          break;
        }
      }
    }

    // update nodes by updated nodeIndices
    for ( size_t i = 0; i < nodeIndices.size(); ++i )
      nodes[ i ] = points[ nodeIndices[ i ]]._nxyz.Node();

    return;
  }

  //================================================================================
  /*!
   * \brief Return 1./area. Initially: max cos^2 of triangle angles
   */
  //================================================================================

  double computeBadness( size_t i1, size_t i2, size_t i3,
                         std::vector< PolyVertex > & points,
                         bool                        checkArea = false )
  {
    if ( checkArea )
    {
      points[ i2 ]._prev = & points[ i1 ];
      points[ i2 ]._next = & points[ i3 ];
      double a = points[ i2 ].TriaArea();
      // if ( a < 0 )
      //   return std::numeric_limits<double>::max();
      // return 1. / a;

      if ( a < 0 )
        return 2;
    }
    const gp_XY & p1 = points[ i1 ]._xy;
    const gp_XY & p2 = points[ i2 ]._xy;
    const gp_XY & p3 = points[ i3 ]._xy;
    gp_XY vec[3] = { p2 - p1,
                     p3 - p2,
                     p1 - p3 };
    double len[3] = { vec[0].SquareModulus(),
                      vec[1].SquareModulus(),
                      vec[2].SquareModulus() };
    if ( len[0] < gp::Resolution() ||
         len[1] < gp::Resolution() ||
         len[2] < gp::Resolution() )
      return 2;

    double maxCos2 = 0;
    for ( int i = 0; i < 3; ++i )
    {
      int i2 = ( i+1 ) % 3;
      double dot = -vec[ i ] * vec[ i2 ];
      if ( dot > 0 )
        maxCos2 = Max( maxCos2, dot * dot / len[ i ] / len[ i2 ] );
    }
    return maxCos2;
  }
};

//================================================================================
/*!
 * \brief Initialization
 */
//================================================================================

void Triangulate::PolyVertex::SetNodeAndNext( const SMDS_MeshNode* n,
                                              PolyVertex&          v,
                                              size_t               index )
{
  _nxyz.Set( n );
  _next = &v;
  v._prev = this;
  _index = index;
}
//================================================================================
/*!
 * \brief Remove self from a polygon
 */
//================================================================================

Triangulate::PolyVertex* Triangulate::PolyVertex::Delete()
{
  _prev->_next = _next;
  _next->_prev = _prev;
  return _next;
}

//================================================================================
/*!
 * \brief Return nodes of a triangle
 */
//================================================================================

  void Triangulate::PolyVertex::GetTriaNodes( const SMDS_MeshNode** nodes,
                                              size_t*               nodeIndices) const
{
  nodes[0] = _prev->_nxyz._node;
  nodes[1] =  this->_nxyz._node;
  nodes[2] = _next->_nxyz._node;
  nodeIndices[0] = _prev->_index;
  nodeIndices[1] =  this->_index;
  nodeIndices[2] = _next->_index;
}

//================================================================================
/*!
 * \brief Compute triangle area
 */
//================================================================================

inline static double Area( const gp_XY& xy0, const gp_XY& xy1, const gp_XY& xy2 )
{
  gp_XY vPrev = xy0 - xy1;
  gp_XY vNext = xy2 - xy1;
  return vNext ^ vPrev;
}

//================================================================================
/*!
 * \brief Compute triangle area
 */
//================================================================================

double Triangulate::PolyVertex::TriaArea() const
{
  return Area( _prev->_xy, this->_xy, _next->_xy );
}

//================================================================================
/*!
 * \brief Check if a vertex is inside a triangle
 */
//================================================================================

bool Triangulate::PolyVertex::IsInsideTria( const PolyVertex* v )
{
  if ( this ->_nxyz == v->_nxyz ||
       _prev->_nxyz == v->_nxyz ||
       _next->_nxyz == v->_nxyz )
    return false;

  gp_XY p = _prev->_xy - v->_xy;
  gp_XY t =  this->_xy - v->_xy;
  gp_XY n = _next->_xy - v->_xy;
  const double tol = -1e-7;
  return (( p ^ t ) >= tol &&
          ( t ^ n ) >= tol &&
          ( n ^ p ) >= tol );
  // return ( Area( _prev, this, v ) > 0 &&
  //          Area( this, _next, v ) > 0 &&
  //          Area( _next, _prev, v ) > 0 );
}

//================================================================================
/*!
 * \brief Triangulate a polygon. Assure correct orientation for concave polygons
 */
//================================================================================

bool Triangulate::triangulate( std::vector< const SMDS_MeshNode*>& nodes,
                               const size_t                        nbNodes)
{
  std::vector< PolyVertex >&    _pv = _data->_pv;
  std::vector< size_t >& _nodeIndex = _data->_nodeIndex;
  PolyVertex::PVSet&  _uniqueNodePV = _data->_uniqueNodePV;

  // connect nodes into a ring
  _pv.resize( nbNodes );
  for ( size_t i = 1; i < nbNodes; ++i )
    _pv[i-1].SetNodeAndNext( nodes[i-1], _pv[i], /*index=*/i-1 );
  _pv[ nbNodes-1 ].SetNodeAndNext( nodes[ nbNodes-1 ], _pv[0], nbNodes-1 );

  // assure correctness of PolyVertex::_index as a node can encounter more than once
  // within a polygon boundary
  if ( _optimizer && nbNodes > 4 )
  {
    _uniqueNodePV.clear();
    for ( size_t i = 0; i < nbNodes; ++i )
    {
      PolyVertex::PVSet::iterator pv = _uniqueNodePV.insert( &_pv[i] ).first;
      _pv[i]._index = (*pv)->_index;
    }
  }

  // get a polygon normal
  gp_XYZ normal(0,0,0), p0,v01,v02;
  p0  = _pv[0]._nxyz;
  v01 = _pv[1]._nxyz - p0;
  for ( size_t i = 2; i < nbNodes; ++i )
  {
    v02 = _pv[i]._nxyz - p0;
    normal += v01 ^ v02;
    v01 = v02;
  }
  // project nodes to the found plane
  gp_Ax2 axes;
  try {
    axes = gp_Ax2( p0, normal, v01 );
  }
  catch ( Standard_Failure ) {
    return false;
  }
  double factor = 1.0, modulus = normal.Modulus();
  if ( modulus < 1e-2 )
    factor = 1. / sqrt( modulus );
  for ( size_t i = 0; i < nbNodes; ++i )
  {
    gp_XYZ p = _pv[i]._nxyz - p0;
    _pv[i]._xy.SetX( axes.XDirection().XYZ() * p * factor);
    _pv[i]._xy.SetY( axes.YDirection().XYZ() * p * factor );
  }

  // compute minimal triangle area
  double sumArea = 0;
  if ( factor == 1.0 )
    sumArea = modulus;
  else
    for ( size_t i = 0; i < nbNodes; ++i )
      sumArea += _pv[i].TriaArea();
  const double minArea = 1e-6 * sumArea / ( nbNodes - 2 );

  // in a loop, find triangles with positive area and having no vertices inside
  int iN = 0, nbTria = nbNodes - 2;
  nodes.resize( nbTria * 3 );
  _nodeIndex.resize( nbTria * 3 );
  PolyVertex* v = &_pv[0], *vi;
  int nbVertices = nbNodes, nbBadTria = 0, isGoodTria;
  while ( nbBadTria < nbVertices )
  {
    if (( isGoodTria = v->TriaArea() > minArea ))
    {
      for ( vi = v->_next->_next;
            vi != v->_prev;
            vi = vi->_next )
      {
        if ( v->IsInsideTria( vi ))
          break;
      }
      isGoodTria = ( vi == v->_prev );
    }
    if ( isGoodTria )
    {
      v->GetTriaNodes( &nodes[ iN ], &_nodeIndex[ iN ] );
      iN += 3;
      v = v->Delete();
      if ( --nbVertices == 3 )
      {
        // last triangle remains
        v->GetTriaNodes( &nodes[ iN ], &_nodeIndex[ iN ] );
        if ( _optimizer )
          _optimizer->optimize( nodes, _pv, _nodeIndex );
        return true;
      }
      nbBadTria = 0;
    }
    else
    {
      v = v->_next;
      ++nbBadTria;
    }
  }

  // the polygon is invalid; add triangles with positive area
  nbBadTria = 0;
  while ( nbBadTria < nbVertices )
  {
    isGoodTria = v->TriaArea() > minArea;
    if ( isGoodTria )
    {
      v->GetTriaNodes( &nodes[ iN ], &_nodeIndex[ iN ] );
      iN += 3;
      v = v->Delete();
      if ( --nbVertices == 3 )
      {
        // last triangle remains
        v->GetTriaNodes( &nodes[ iN ], &_nodeIndex[ iN ] );
        return true;
      }
      nbBadTria = 0;
    }
    else
    {
      v = v->_next;
      ++nbBadTria;
    }
  }

  // add all the rest triangles
  while ( nbVertices >= 3 )
  {
    v->GetTriaNodes( &nodes[ iN ], &_nodeIndex[ iN ] );
    iN += 3;
    v = v->Delete();
    --nbVertices;
  }

  return true;

} // triangulate()

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

Triangulate::Triangulate( bool optimize ): _optimizer(0)
{
  _data = new Data;
  if ( optimize )
    _optimizer = new Optimizer;
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

Triangulate::~Triangulate()
{
  delete _data;
  delete _optimizer;
  _optimizer = 0;
}

//================================================================================
/*!
 * \brief Return nb triangles in a decomposed mesh face
 *  \retval int - number of triangles
 */
//================================================================================

int Triangulate::GetNbTriangles( const SMDS_MeshElement* face )
{
  // WARNING: counting triangles must be coherent with GetTriangles()
  switch ( face->GetEntityType() )
  {
  case SMDSEntity_BiQuad_Triangle:
  case SMDSEntity_BiQuad_Quadrangle:
    return face->NbNodes() - 1;
    // case SMDSEntity_Triangle:
    // case SMDSEntity_Quad_Triangle:
    // case SMDSEntity_Quadrangle:
    // case SMDSEntity_Quad_Quadrangle:
    // case SMDSEntity_Polygon:
    // case SMDSEntity_Quad_Polygon:
  default:
    return face->NbNodes() - 2;
  }
  return 0;
}

//================================================================================
/*!
 * \brief Decompose a mesh face into triangles
 *  \retval int - number of triangles
 */
//================================================================================

int Triangulate::GetTriangles( const SMDS_MeshElement*             face,
                               std::vector< const SMDS_MeshNode*>& nodes)
{
  if ( face->GetType() != SMDSAbs_Face )
    return 0;

  // WARNING: decomposing into triangles must be coherent with getNbTriangles()
  int nbTria, i = 0, nbNodes = face->NbNodes();
  SMDS_NodeIteratorPtr nIt = face->interlacedNodesIterator();
  nodes.resize( nbNodes * 3 );
  nodes[ i++ ] = nIt->next();
  nodes[ i++ ] = nIt->next();

  const SMDSAbs_EntityType type = face->GetEntityType();
  switch ( type )
  {
  case SMDSEntity_BiQuad_Triangle:
  case SMDSEntity_BiQuad_Quadrangle:

    nbTria = ( type == SMDSEntity_BiQuad_Triangle ) ? 6 : 8;
    nodes[ i++ ] = face->GetNode( nbTria );
    for ( i = 3; i < 3*(nbTria-1); i += 3 )
    {
      nodes[ i+0 ] = nodes[ i-2 ];
      nodes[ i+1 ] = nIt->next();
      nodes[ i+2 ] = nodes[ 2 ];
    }
    nodes[ i+0 ] = nodes[ i-2 ];
    nodes[ i+1 ] = nodes[ 0 ];
    nodes[ i+2 ] = nodes[ 2 ];
    break;

  case SMDSEntity_Triangle:

    nbTria = 1;
    nodes[ i++ ] = nIt->next();
    break;

  default:

    // case SMDSEntity_Quad_Triangle:
    // case SMDSEntity_Quadrangle:
    // case SMDSEntity_Quad_Quadrangle:
    // case SMDSEntity_Polygon:
    // case SMDSEntity_Quad_Polygon:

    nbTria = nbNodes - 2;
    while ( nIt->more() )
      nodes[ i++ ] = nIt->next();

    if ( nbTria > 1 && !triangulate( nodes, nbNodes ))
    {
      nIt = face->interlacedNodesIterator();
      nodes[ 0 ] = nIt->next();
      nodes[ 1 ] = nIt->next();
      nodes[ 2 ] = nIt->next();
      for ( i = 3; i < 3*nbTria; i += 3 )
      {
        nodes[ i+0 ] = nodes[ 0 ];
        nodes[ i+1 ] = nodes[ i-1 ];
        nodes[ i+2 ] = nIt->next();
      }
    }
  }

  return nbTria;
}
