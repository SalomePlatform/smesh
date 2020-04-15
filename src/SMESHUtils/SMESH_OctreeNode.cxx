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

//  SMESH SMESH_OctreeNode : Octree with Nodes set
//  inherites class SMESH_Octree
//  File      : SMESH_OctreeNode.cxx
//  Created   : Tue Jan 16 16:00:00 2007
//  Author    : Nicolas Geimer & Aurelien Motteux (OCC)
//  Module    : SMESH
//
#include "SMESH_OctreeNode.hxx"

#include "SMDS_SetIterator.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_TypeDefs.hxx"

#include <gp_Pnt.hxx>

using namespace std;

//===============================================================
/*!
 * \brief Constructor : Build all the Octree using Compute()
 * \param theNodes - Set of nodes, the Octree is built from this nodes
 * \param maxLevel - Maximum level for the leaves
 * \param maxNbNodes - Maximum number of nodes, a leaf can contain
 * \param minBoxSize - Minimal size of the Octree Box
 */
//================================================================

SMESH_OctreeNode::SMESH_OctreeNode (const TIDSortedNodeSet & theNodes, const int maxLevel,
                                    const int maxNbNodes , const double minBoxSize )
  :SMESH_Octree( new Limit( maxLevel,minBoxSize,maxNbNodes)),
   myNodes( theNodes.begin(), theNodes.end() )
{
  compute();
}

//================================================================================
/*!
 * \brief Constructor used to allocate a child
 */
//================================================================================

SMESH_OctreeNode::SMESH_OctreeNode ():SMESH_Octree()
{
}

//================================================================================
/*!
 * \brief Return max number of nodes in a tree leaf
 */
//================================================================================

int SMESH_OctreeNode::getMaxNbNodes() const
{
  return ((Limit*)myLimit)->myMaxNbNodes;
}

//==================================================================================
/*!
 * \brief Construct an empty SMESH_OctreeNode used by SMESH_Octree::buildChildren()
 */
//==================================================================================

SMESH_Octree* SMESH_OctreeNode::newChild() const
{
  return new SMESH_OctreeNode();
}

//======================================
/*!
 * \brief Compute the first bounding box
 *
 * We take the max/min coord of the nodes
 */
//======================================

Bnd_B3d* SMESH_OctreeNode::buildRootBox()
{
  Bnd_B3d* box = new Bnd_B3d;
  for ( size_t i = 0; i < myNodes.size(); ++i )
    box->Add( SMESH_NodeXYZ( myNodes[ i ]));

  if ((int) myNodes.size() <= getMaxNbNodes() )
    myIsLeaf = true;

  return box;
}

//====================================================================================
/*!
 * \brief Tells us if Node is inside the current box with the precision "precision"
 * \param Node - Node
 * \param precision - The box is enlarged with this precision
 * \retval bool - True if Node is in the box within precision
 */
//====================================================================================

const bool SMESH_OctreeNode::isInside ( const gp_XYZ& p, const double precision )
{
  if ( precision <= 0.)
    return !( getBox()->IsOut(p) );

  Bnd_B3d BoxWithPrecision = *getBox();
  BoxWithPrecision.Enlarge( precision );
  return ! BoxWithPrecision.IsOut(p);
}

//================================================
/*!
 * \brief Set the data of the children
 * Shares the father's data with each of his child
 */
//================================================

void SMESH_OctreeNode::buildChildrenData()
{
  gp_XYZ min = getBox()->CornerMin();
  gp_XYZ max = getBox()->CornerMax();
  gp_XYZ mid = (min + max)/2.;

  for ( int i = 0; i < 8; i++ )
  {
    SMESH_OctreeNode* myChild = static_cast<SMESH_OctreeNode*>( myChildren[ i ]);
    myChild->myNodes.reserve( myNodes.size() / 8 );
  }

  for ( size_t i = 0; i < myNodes.size(); ++i )
  {
    SMESH_NodeXYZ n = myNodes[ i ];
    int ChildBoxNum = getChildIndex( n.X(), n.Y(), n.Z(), mid );
    SMESH_OctreeNode* myChild = static_cast<SMESH_OctreeNode*>( myChildren[ ChildBoxNum ]);
    myChild->myNodes.push_back( myNodes[ i ]);
  }
  SMESHUtils::FreeVector( myNodes );

  for ( int i = 0; i < 8; i++ )
  {
    SMESH_OctreeNode* myChild = static_cast<SMESH_OctreeNode*>( myChildren[ i ]);
    if ((int) myChild->myNodes.size() <= getMaxNbNodes() )
    {
      myChild->myIsLeaf = true;
      if ( myChild->myNodes.empty() )
        SMESHUtils::FreeVector( myChild->myNodes );
    }
  }
}

//===================================================================
/*!
 * \brief Return in Result a list of Nodes potentials to be near Node
 * \param Node - Node
 * \param precision - precision used
 * \param Result - list of Nodes potentials to be near Node
 */
//====================================================================

void SMESH_OctreeNode::AllNodesAround (const SMDS_MeshNode *              Node,
                                       std::vector<const SMDS_MeshNode*>* Result,
                                       const double                       precision)
{
  SMESH_NodeXYZ p = Node;
  if ( isInside( p, precision ))
  {
    if ( isLeaf() )
    {
      Result->insert( Result->end(), myNodes.begin(), myNodes.end() );
    }
    else
    {
      for ( int i = 0; i < 8; i++ )
      {
        SMESH_OctreeNode* myChild = static_cast<SMESH_OctreeNode*> (myChildren[i]);
        myChild->AllNodesAround( Node, Result, precision );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Return in dist2Nodes nodes mapped to their square distance from Node
 *        Tries to find a closest node.
 *  \param node - node to find nodes closest to
 *  \param dist2Nodes - map of found nodes and their distances
 *  \param precision - radius of a sphere to check nodes inside
 *  \retval bool - true if an exact overlapping found !!!
 */
//================================================================================

bool SMESH_OctreeNode::NodesAround(const gp_XYZ &                     node,
                                   map<double, const SMDS_MeshNode*>& dist2Nodes,
                                   double                             precision)
{
  if ( !dist2Nodes.empty() )
    precision = min ( precision, sqrt( dist2Nodes.begin()->first ));
  else if ( precision == 0. )
    precision = maxSize() / 2;

  if ( isInside( node, precision ))
  {
    if ( !isLeaf() )
    {
      // first check a child containing node
      gp_XYZ mid = (getBox()->CornerMin() + getBox()->CornerMax()) / 2.;
      int nodeChild  = getChildIndex( node.X(), node.Y(), node.Z(), mid );
      if ( ((SMESH_OctreeNode*) myChildren[nodeChild])->NodesAround(node, dist2Nodes, precision))
        return true;

      for (int i = 0; i < 8; i++)
        if ( i != nodeChild )
          if (((SMESH_OctreeNode*) myChildren[i])->NodesAround(node, dist2Nodes, precision))
            return true;
    }
    else if ( NbNodes() > 0 )
    {
      double minDist = precision * precision;
      for ( size_t i = 0; i < myNodes.size(); ++i )
      {
        SMESH_NodeXYZ p2 = myNodes[ i ];
        double     dist2 = ( node - p2 ).SquareModulus();
        if ( dist2 < minDist )
          dist2Nodes.insert( std::make_pair( minDist = dist2, myNodes[ i ] ));
      }
      // if ( dist2Nodes.size() > 1 ) // leave only closest node in dist2Nodes
      //   dist2Nodes.erase( ++dist2Nodes.begin(), dist2Nodes.end());

      // true if an exact overlapping found
      return ( sqrt( minDist ) <= precision * 1e-12 );
    }
  }
  return false;
}

//================================================================================
/*!
 * \brief Return a list of nodes close to a point
 *  \param [in] point - point
 *  \param [out] nodes - found nodes
 *  \param [in] precision - allowed distance from \a point
 */
//================================================================================

void SMESH_OctreeNode::NodesAround(const gp_XYZ&                      point,
                                   std::vector<const SMDS_MeshNode*>& nodes,
                                   double                             precision)
{
  if ( isInside( point, precision ))
  {
    if ( isLeaf() && NbNodes() )
    {
      double minDist2 = precision * precision;
      for ( size_t i = 0; i < myNodes.size(); ++i )
      {
        SMESH_NodeXYZ p2 = myNodes[ i ];
        double dist2 = ( point - p2 ).SquareModulus();
        if ( dist2 <= minDist2 )
          nodes.push_back( myNodes[ i ] );
      }
    }
    else if ( myChildren )
    {
      for (int i = 0; i < 8; i++)
      {
        SMESH_OctreeNode* myChild = static_cast<SMESH_OctreeNode*>( myChildren[ i ]);
        myChild->NodesAround( point, nodes, precision );
      }
    }
  }
}

//=============================
/*!
 * \brief  Return in theGroupsOfNodes a list of group of nodes close to each other within theTolerance
 * Search for all the nodes in theSetOfNodes
 * Static Method : no need to create an SMESH_OctreeNode
 * \param theSetOfNodes - set of nodes we look at, modified during research
 * \param theGroupsOfNodes - list of nodes closed to each other returned
 * \param theTolerance - Precision used, default value is 0.00001
 * \param maxLevel - Maximum level for SMESH_OctreeNode constructed, default value is -1 (Infinite)
 * \param maxNbNodes - maximum Nodes in a Leaf of the SMESH_OctreeNode constructed, default value is 5
 */
//=============================

void SMESH_OctreeNode::FindCoincidentNodes (TIDSortedNodeSet& theSetOfNodes,
                                            TListOfNodeLists* theGroupsOfNodes,
                                            const double      theTolerance,
                                            const int         maxLevel,
                                            const int         maxNbNodes)
{
  // VSR 14/10/2011: limit max number of the levels in order to avoid endless recursion
  const int MAX_LEVEL = 10;
  SMESH_OctreeNode theOctreeNode(theSetOfNodes,
                                 maxLevel < 0 ? MAX_LEVEL : maxLevel,
                                 maxNbNodes,
                                 theTolerance);
  theOctreeNode.FindCoincidentNodes (&theSetOfNodes, theTolerance, theGroupsOfNodes);
}

//=============================
/*!
 * \brief  Return in theGroupsOfNodes a list of group of nodes close to each other within theTolerance
 * Search for all the nodes in theSetOfNodes
 * \note  The Octree itself is also modified by this method
 * \param theSetOfNodes - set of nodes we look at, modified during research
 * \param theTolerance - Precision used
 * \param theGroupsOfNodes - list of nodes closed to each other returned
 */
//=============================

void SMESH_OctreeNode::FindCoincidentNodes ( TIDSortedNodeSet* theSetOfNodes,
                                             const double      theTolerance,
                                             TListOfNodeLists* theGroupsOfNodes )
{
  // un-mark all nodes; we mark nodes added to theGroupsOfNodes
  SMESH_MeshAlgos::MarkElems( SMESHUtils::elemSetIterator( *theSetOfNodes ), false );

  vector<const SMDS_MeshNode*> coincidentNodes;
  TIDCompare idLess;

  TIDSortedNodeSet::iterator it1 = theSetOfNodes->begin();
  for ( ; it1 != theSetOfNodes->end(); ++it1 )
  {
    const SMDS_MeshNode * n1 = *it1;
    if ( n1->isMarked() )
      continue;
    n1->setIsMarked( true );

    // Searching for Nodes around n1 and put them in coincidentNodes.
    // Found nodes are also erased from theSetOfNodes
    coincidentNodes.clear();
    findCoincidentNodes( n1, theSetOfNodes, &coincidentNodes, theTolerance );

    if ( !coincidentNodes.empty() )
    {
      // We build a list {n1 + his neighbors} and add this list in theGroupsOfNodes
      std::sort( coincidentNodes.begin(), coincidentNodes.end(), idLess );
      list<const SMDS_MeshNode*> newGroup;
      newGroup.push_back( n1 );
      newGroup.insert( newGroup.end(), coincidentNodes.begin(), coincidentNodes.end() );

      theGroupsOfNodes->emplace_back( newGroup );
    }
  }
}

//======================================================================================
/*!
 * \brief Return a list of nodes closed to Node and remove it from SetOfNodes
 * \note  The Octree itself is also modified by this method
 * \param Node - We're searching the nodes next to him.
 * \param SetOfNodes - set of nodes in which we erase the found nodes
 * \param Result - list of nodes closed to Node
 * \param precision - Precision used
 */
//======================================================================================

void SMESH_OctreeNode::findCoincidentNodes (const SMDS_MeshNode *              Node,
                                            TIDSortedNodeSet*                  SetOfNodes,
                                            std::vector<const SMDS_MeshNode*>* Result,
                                            const double                       precision)
{
  SMESH_NodeXYZ p1 = Node;

  if ( isInside( p1, precision ))
  {
    // I'm only looking in the leaves, since all the nodes are stored there.
    if ( isLeaf() )
    {
      const double tol2 = precision * precision;

      for ( size_t i = 0; i < myNodes.size(); ++i )
      {
        if ( myNodes[ i ]->isMarked() ) // coincident node already found
          continue;

        //if ( Node != myNodes[ i ]) // JFA: for bug 0020185
        {
          // If n2 inside the SquareDistance, we add it in Result
          bool coincide = ( p1.SquareDistance( myNodes[ i ]) <= tol2 );
          if ( coincide )
          {
            Result->push_back ( myNodes[ i ]);
            myNodes[ i ]->setIsMarked( true );
          }
        }
      }
    }
    else
    {
      // If I'm not a leaf, I'm going to see my children !
      for ( int i = 0; i < 8; i++ )
      {
        SMESH_OctreeNode* myChild = static_cast<SMESH_OctreeNode*> (myChildren[i]);
        myChild->findCoincidentNodes( Node, SetOfNodes, Result, precision );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Update data according to node movement
 */
//================================================================================

void SMESH_OctreeNode::UpdateByMoveNode( const SMDS_MeshNode* node, const gp_Pnt& toPnt )
{
  if ( isLeaf() )
  {
    std::vector< const SMDS_MeshNode* >::iterator pNode =
      std::find( myNodes.begin(), myNodes.end(), node );

    bool  nodeInMe = ( pNode != myNodes.end() );
    bool pointInMe = isInside( toPnt.Coord(), 1e-10 );

    if ( pointInMe != nodeInMe )
    {
      if ( pointInMe )
        myNodes.push_back( node );
      else
        myNodes.erase( pNode );
    }
  }
  else if ( myChildren )
  {
    gp_XYZ mid = (getBox()->CornerMin() + getBox()->CornerMax()) / 2.;
    int nodeChild  = getChildIndex( node->X(), node->Y(), node->Z(), mid );
    int pointChild = getChildIndex( toPnt.X(), toPnt.Y(), toPnt.Z(), mid );
    if ( nodeChild != pointChild )
    {
      ((SMESH_OctreeNode*) myChildren[ nodeChild  ])->UpdateByMoveNode( node, toPnt );
      ((SMESH_OctreeNode*) myChildren[ pointChild ])->UpdateByMoveNode( node, toPnt );
    }
  }
}

//================================================================================
/*!
 * \brief Return iterator over children
 */
//================================================================================

SMESH_OctreeNodeIteratorPtr SMESH_OctreeNode::GetChildrenIterator()
{
  return SMESH_OctreeNodeIteratorPtr
    ( new SMDS_SetIterator< SMESH_OctreeNode*, TBaseTree** >
      ( myChildren, (( isLeaf() || !myChildren ) ? myChildren : &myChildren[ 8 ] )));
}

//================================================================================
/*!
 * \brief Return nodes iterator
 */
//================================================================================

SMDS_NodeIteratorPtr SMESH_OctreeNode::GetNodeIterator()
{
  return boost::make_shared< SMDS_NodeVectorIterator >( myNodes.begin(), myNodes.end());
}
