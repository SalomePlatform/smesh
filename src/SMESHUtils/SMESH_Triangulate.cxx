// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

using namespace SMESH_MeshAlgos;

//================================================================================
/*!
 * \brief Initialization
 */
//================================================================================

void Triangulate::PolyVertex::SetNodeAndNext( const SMDS_MeshNode* n,
                                              PolyVertex&          v )
{
  _nxyz.Set( n );
  _next = &v;
  v._prev = this;
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

void Triangulate::PolyVertex::GetTriaNodes( const SMDS_MeshNode** nodes) const
{
  nodes[0] = _prev->_nxyz._node;
  nodes[1] =  this->_nxyz._node;
  nodes[2] = _next->_nxyz._node;
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
  const double tol = -1e-12;
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
                               const size_t                        nbNodes )
{
  // connect nodes into a ring
  _pv.resize( nbNodes );
  for ( size_t i = 1; i < nbNodes; ++i )
    _pv[i-1].SetNodeAndNext( nodes[i-1], _pv[i] );
  _pv[ nbNodes-1 ].SetNodeAndNext( nodes[ nbNodes-1 ], _pv[0] );

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
  for ( size_t i = 0; i < nbNodes; ++i )
  {
    gp_XYZ p = _pv[i]._nxyz - p0;
    _pv[i]._xy.SetX( axes.XDirection().XYZ() * p );
    _pv[i]._xy.SetY( axes.YDirection().XYZ() * p );
  }

  // in a loop, find triangles with positive area and having no vertices inside
  int iN = 0, nbTria = nbNodes - 2;
  nodes.reserve( nbTria * 3 );
  const double minArea = 1e-6;
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
      v->GetTriaNodes( &nodes[ iN ] );
      iN += 3;
      v = v->Delete();
      if ( --nbVertices == 3 )
      {
        // last triangle remains
        v->GetTriaNodes( &nodes[ iN ] );
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
      v->GetTriaNodes( &nodes[ iN ] );
      iN += 3;
      v = v->Delete();
      if ( --nbVertices == 3 )
      {
        // last triangle remains
        v->GetTriaNodes( &nodes[ iN ] );
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
    v->GetTriaNodes( &nodes[ iN ] );
    iN += 3;
    v = v->Delete();
    --nbVertices;
  }

  return true;

} // triangulate()

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
