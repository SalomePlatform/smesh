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
//  File   : SMDS_LinearEdge.cxx
//  Author : Jean-Michel BOULCOURT
//  Module : SMESH
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_LinearEdge.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_SetIterator.hxx"

#include <boost/make_shared.hpp>

//=======================================================================
//function : SMDS_LinearEdge
//purpose  : 
//=======================================================================

SMDS_LinearEdge::SMDS_LinearEdge(const SMDS_MeshNode * node1,
                                 const SMDS_MeshNode * node2)
{
  myNodes[0] = node1;
  myNodes[1] = node2;
}

int SMDS_LinearEdge::NbNodes() const
{
  return 2;
}

int SMDS_LinearEdge::NbEdges() const
{
  return 1;
}

int SMDS_LinearEdge::NbFaces() const
{
  return 0;
}

int SMDS_LinearEdge::GetNodeIndex( const SMDS_MeshNode* node ) const
{
  if ( node == myNodes[0] ) return 0;
  if ( node == myNodes[1] ) return 1;
  return -1;
}

SMDS_ElemIteratorPtr SMDS_LinearEdge::nodesIterator() const
{
  return boost::make_shared< SMDS_NodeArrayElemIterator >( &myNodes[0], &myNodes[0] + NbNodes() );
}

SMDS_NodeIteratorPtr SMDS_LinearEdge::nodeIterator() const
{
  return boost::make_shared< SMDS_NodeArrayIterator >( &myNodes[0], &myNodes[0] + NbNodes() );
}

/*
 * \brief Return node by its index
 * \param ind - node index
 * \retval const SMDS_MeshNode* - the node
 */
const SMDS_MeshNode* SMDS_LinearEdge::GetNode(const int ind) const
{
  return myNodes[ind];
}

//=======================================================================
//function : ChangeNodes
//purpose  : 
//=======================================================================

bool SMDS_LinearEdge::ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes)
{
  myNodes[0] = nodes[0];
  myNodes[1] = nodes[1];
  return nbNodes == 2;
}
