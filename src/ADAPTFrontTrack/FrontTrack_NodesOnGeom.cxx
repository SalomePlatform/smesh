// Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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
// File      : FrontTrack_NodesOnGeom.cxx
// Created   : Tue Apr 25 20:48:23 2017
// Author    : Edward AGAPOV (eap)

#include "FrontTrack_NodesOnGeom.hxx"
#include "FrontTrack_Utils.hxx"

#include <MEDCouplingMemArray.hxx>

#include <cstdio>
#include <cstdlib>
#include <list>
#include <stdexcept>

namespace
{
  /*!
   * \brief Close a file at destruction
   */
  struct FileCloser
  {
    FILE * _file;

    FileCloser( FILE * file ): _file( file ) {}
    ~FileCloser() { if ( _file ) ::fclose( _file ); }
  };
}

//================================================================================
/*!
 * \brief Read node ids from a file and find shapes for projection
 *  \param [in] theNodeFile - a name of file holding IDs of nodes that
 *         will be moved onto geometry
 *  \param [in] theXaoGroups - a tool returning FT_Projector's by XAO group name
 *  \param [inout] theNodeCoords - array of node coordinates
 *  \param [in] theAllProjectorsByDim - all projectors of 2 dimensions, ordered so that
 *         a vector index corresponds to a XAO sub-shape ID
 */
//================================================================================

void FT_NodesOnGeom::read( const std::string&            theNodeFile,
                           const FT_Utils::XaoGroups&    theXaoGroups,
                           MEDCoupling::DataArrayDouble* theNodeCoords,
                           std::vector< FT_Projector > * theAllProjectorsByDim )
{
  _nodeCoords = theNodeCoords;

  FILE * file = ::fopen( theNodeFile.c_str(), "r" );
  if ( !file )
    throw std::invalid_argument( "Can't open an input node file: " + theNodeFile );

  FileCloser fileCloser( file );

  // -------------------------------------
  // get shape dimension by the file name
  // -------------------------------------

  // hope the file name is something like "frnD.**" with n in (1,2)
  int dimPos = theNodeFile.size() - 5;
  if ( theNodeFile[ dimPos ] == '2' )
    _shapeDim = 2;
  else if ( theNodeFile[ dimPos ] == '1' )
    _shapeDim = 1;
  else
    throw std::invalid_argument( "Can't define dimension by node file name " + theNodeFile );
#ifdef _DEBUG_
  std::cout << ". Dimension of the file " << theNodeFile << ": " << _shapeDim << std::endl;
#endif

  // -------------------------------------
  // read geom group names; several lines
  // -------------------------------------

  std::vector< std::string > geomNames;

  const int maxLineLen = 256;
  char line[ maxLineLen ];

  long int pos = ::ftell( file );
  while ( ::fgets( line, maxLineLen, file )) // read a line
  {
    if ( ::feof( file ))
    {
      return; // no nodes in the file
    }

    // check if the line describes node ids in format 3I10 (e.g. "       120         1        43\n")
    size_t lineLen = strlen( line );
    if ( lineLen  >= 31        &&
         ::isdigit( line[9] )  &&
         line[10] == ' '       &&
         ::isdigit( line[19] ) &&
         line[20] == ' '       &&
         ::isdigit( line[29] ) &&
         ::isspace( line[30] ))
      break;

    geomNames.push_back( line + 1 ); // skip the 1st white space

    pos = ::ftell( file ); // remember the position to return if the next line holds node ids
  }

  ::fseek( file, pos, SEEK_SET ); // return to the 1st line holding nodes ids


  // --------------
  // read node ids
  // --------------

  FT_NodeToMove nodeIds;
  std::vector< int > ids;

  const int nbNodes = theNodeCoords->getNumberOfTuples(); // to check validity of node IDs

  while ( ::fgets( line, maxLineLen, file )) // read a line
  {
    // find node ids in the line

    char *beg = line, *end = 0;
    long int id;

    ids.clear();
    while (( id = ::strtol( beg, &end, 10 )) &&
           ( beg != end ))
    {
      ids.push_back( id );
      if ( id > nbNodes )
        throw std::invalid_argument( "Too large node ID: " + FT_Utils::toStr( id ));
      beg = end;
    }

    if ( ids.size() >= 3 )
    {
      std::vector< int >::iterator i = ids.begin();
      nodeIds._nodeToMove = *i;
      nodeIds._neighborNodes.assign( ++i, ids.end() );

      _nodes.push_back( nodeIds );
    }

    if ( ::feof( file ))
      break;
  }

  // -----------------------------------------------------------------
  // try to find FT_Projector's to boundary sub-shapes by group names
  // -----------------------------------------------------------------

  _allProjectors = & theAllProjectorsByDim[ _shapeDim - 1 ];

  _projectors.reserve( geomNames.size() );
  std::vector< const FT_Projector* >  projectors;

  for ( size_t i = 0; i < geomNames.size(); ++i )
  {
    std::string & groupName = geomNames[i];
#ifdef _DEBUG_
    std::cout << ". Group name: " << groupName << std::endl;
#endif

    // remove trailing white spaces
    for ( int iC = groupName.size() - 1; iC >= 0; --iC )
    {
      if ( ::isspace( groupName[iC] ) )
        groupName.resize( iC );
      else
        break;
    }
    if ( groupName.empty() )
      continue;

    _groupNames.push_back( groupName ); // keep _groupNames for easier debug :)

    // get projectors by group name
    theXaoGroups.getProjectors( groupName, _shapeDim,
                                theAllProjectorsByDim[ _shapeDim-1 ], projectors );
  }

  // ------------------------------
  // check the found FT_Projector's
  // ------------------------------

  if ( projectors.size() == 1 )
  {
    _projectors.push_back( *projectors[ 0 ]);
  }
  else
  {
    Bnd_Box nodesBox;
    for ( size_t i = 0; i < _nodes.size(); ++i )
      nodesBox.Add( getPoint( _nodes[i]._nodeToMove ));

    if ( projectors.size() > 1 )
    {
      // more than one boundary shape;
      // try to filter off unnecessary projectors using a bounding box of nodes
      for ( size_t i = 0; i < projectors.size(); ++i )
        if ( !nodesBox.IsOut( projectors[ i ]->getBoundingBox() ))
          _projectors.push_back( *projectors[ i ]);
    }

    if ( _projectors.empty() )
    {
      // select projectors using a bounding box of nodes
      std::vector< FT_Projector > & allProjectors = *_allProjectors;
      for ( size_t i = 0; i < allProjectors.size(); ++i )
        if ( !nodesBox.IsOut( allProjectors[ i ].getBoundingBox() ))
          _projectors.push_back( allProjectors[ i ]);

      if ( _projectors.empty() && !_nodes.empty() )
        throw std::runtime_error("No boundary shape found for nodes in file " + theNodeFile );
    }
  }

  // prepare for projection - create real projectors
  for ( size_t i = 0; i < _projectors.size(); ++i )
    _projectors[ i ].prepareForProjection();

}

//================================================================================
/*!
 * \brief Project nodes to the shapes and move them to new positions
 */
//================================================================================

void FT_NodesOnGeom::projectAndMove()
{
  _OK = true;
//
// 1. Préalables
//
  // check if all the shapes are planar
  bool isAllPlanar = true;
  for ( size_t i = 0; i < _projectors.size() &&  isAllPlanar; ++i )
    isAllPlanar = _projectors[i].isPlanarBoundary();
  if ( isAllPlanar )
    return;

  // set nodes in the order suitable for optimal projection
  putNodesInOrder();

  // project and move nodes

  std::vector< FT_NodeToMove* > notProjectedNodes;
  size_t iP, iProjector;
  gp_Pnt newXyz;

#ifdef _DEBUG_
    std::cout << ".. _projectors.size() = " << _projectors.size() << std::endl;
    std::cout << ".. _nodesOrder.size() = " << _nodesOrder.size() << std::endl;
#endif
//
// 2. Calculs
// 2.1. Avec plusieurs shapes
//
  if ( _projectors.size() > 1 )
  {
    // the nodes are to be projected onto several boundary shapes;
    // in addition to the projecting, classification on a shape is necessary
    // in order to find out on which of the shapes a node is to be projected

    iProjector = 0;
    for ( size_t i = 0; i < _nodesOrder.size(); ++i )
    {
      FT_NodeToMove& nn = _nodes[ _nodesOrder[ i ]];
      gp_Pnt        xyz = getPoint( nn._nodeToMove );
      gp_Pnt       xyz1 = getPoint( nn._neighborNodes[0] );
      gp_Pnt       xyz2 = getPoint( nn._neighborNodes[1] );
      double   maxDist2 = xyz1.SquareDistance( xyz2 ) / 4.;
      if ( _projectors[ iProjector ].projectAndClassify( xyz, maxDist2, newXyz,
                                                         nn._params, nn._nearParams ))
      {
        moveNode( nn._nodeToMove, newXyz );
      }
      else // a node is not on iProjector-th shape, find the shape it is on
      {
        for ( iP = 1; iP < _projectors.size(); ++iP ) // check _projectors other than iProjector
        {
          iProjector = ( iProjector + 1 ) % _projectors.size();
          if ( _projectors[ iProjector ].projectAndClassify( xyz, maxDist2, newXyz,
                                                             nn._params, nn._nearParams ))
          {
            moveNode( nn._nodeToMove, newXyz );
            break;
          }
        }
        if ( iP == _projectors.size() )
        {
          notProjectedNodes.push_back( &nn );

#ifdef _DEBUG_
          std::cerr << "Warning: no shape found for node " << nn._nodeToMove << std::endl;
          if ( !_groupNames.empty() )
            std::cerr << "Warning:    group -- " << _groupNames[0] << std::endl;
#endif
        }
      }
    }
  }
//
// 2.2. Avec une seule shape
//
  else // one shape
  {
    for ( size_t i = 0; i < _nodesOrder.size(); ++i )
    {
      FT_NodeToMove& nn = _nodes[ _nodesOrder[ i ]];
      gp_Pnt        xyz = getPoint( nn._nodeToMove );
      gp_Pnt       xyz1 = getPoint( nn._neighborNodes[0] );
      gp_Pnt       xyz2 = getPoint( nn._neighborNodes[1] );

// maxDist2 : le quart du carré de la distance entre les deux voisins du noeud à bouger
      double   maxDist2 = xyz1.SquareDistance( xyz2 ) / 4.;
#ifdef _DEBUG_
    std::cout << "\n.. maxDist2 = " << maxDist2 << " entre " << nn._neighborNodes[0] << " et " << nn._neighborNodes[1] << " - milieu " << nn._nodeToMove << " - d/2 = " << sqrt(maxDist2) << " - d = " << sqrt(xyz1.SquareDistance( xyz2 )) << std::endl;
#endif
      if ( _projectors[ 0 ].project( xyz, maxDist2, newXyz,
                                     nn._params, nn._nearParams ))
        moveNode( nn._nodeToMove, newXyz );
      else
        notProjectedNodes.push_back( &nn );
    }
  }
//
// 3. Bilan
//
  if ( !notProjectedNodes.empty() )
  {
    // project nodes that are not projected by any of _projectors;
    // a proper projector is selected by evaluation of a distance between neighbor nodes
    // and a shape

    std::vector< FT_Projector > & projectors = *_allProjectors;

    iProjector = 0;
    for ( size_t i = 0; i < notProjectedNodes.size(); ++i )
    {
      FT_NodeToMove& nn = *notProjectedNodes[ i ];
      gp_Pnt        xyz = getPoint( nn._nodeToMove );
      gp_Pnt       xyz1 = getPoint( nn._neighborNodes[0] );
      gp_Pnt       xyz2 = getPoint( nn._neighborNodes[1] );
      double   maxDist2 = xyz1.SquareDistance( xyz2 ) / 4.;
      double       tol2 = 1e-6 * maxDist2;

      bool ok;
      for ( iP = 0; iP < projectors.size(); ++iP )
      {
        projectors[ iProjector ].prepareForProjection();
        projectors[ iProjector ].tryWithoutPrevSolution( true );

        if (( ok = projectors[ iProjector ].isOnShape( xyz1, tol2, nn._params, nn._nearParams )) &&
            ( ok = projectors[ iProjector ].isOnShape( xyz2, tol2, nn._params, nn._params )))
        {
          if ( nn._neighborNodes.size() == 4 )
          {
            gp_Pnt xyz1 = getPoint( nn._neighborNodes[2] );
            gp_Pnt xyz2 = getPoint( nn._neighborNodes[3] );
            if (( ok = projectors[ iProjector ].isOnShape( xyz1, tol2, nn._params, nn._params )))
              ok     = projectors[ iProjector ].isOnShape( xyz2, tol2, nn._params, nn._params );
          }
        }

        if ( ok && projectors[iProjector].project( xyz, maxDist2, newXyz, nn._params, nn._params ))
        {
          moveNode( nn._nodeToMove, newXyz );
          break;
        }
        iProjector = ( iProjector + 1 ) % projectors.size();
      }
      if ( iP == projectors.size() )
      {
        _OK = false;

        std::cerr << "Error: not projected node " << nn._nodeToMove << std::endl;
      }
    }
  }
}

//================================================================================
/*!
 * \brief Put nodes in the order for optimal projection and set FT_NodeToMove::_nearParams
 *        to point to a FT_NodeToMove::_params of a node that will be projected earlier
 */
//================================================================================

void FT_NodesOnGeom::putNodesInOrder()
{
  if ( !_nodesOrder.empty() )
    return;

  // check if any of projectors can use parameters of a previously projected node on a shape
  // to speed up projection

  bool isPrevSolutionUsed = false;
  for ( size_t i = 0; i < _projectors.size() &&  !isPrevSolutionUsed; ++i )
    isPrevSolutionUsed = _projectors[i].canUsePrevSolution();

  if ( !isPrevSolutionUsed )
  {
    _nodesOrder.resize( _nodes.size() );
    for ( size_t i = 0; i < _nodesOrder.size(); ++i )
      _nodesOrder[ i ] = i;
    return;
  }

  // make a map to find a neighbor projected node

  // map of { FT_NodeToMove::_neighborNodes[i] } to { FT_NodeToMove* };
  // here we call FT_NodeToMove a 'link' as this data links a _neighborNodes[i] node to other nodes
  typedef NCollection_DataMap< int, std::vector< FT_NodeToMove* > > TNodeIDToLinksMap;
  TNodeIDToLinksMap neigborsMap;

  int mapSize = ( _shapeDim == 1 ) ? _nodes.size() + 1 : _nodes.size() * 3;
  neigborsMap.Clear();
  neigborsMap.ReSize( mapSize );

  std::vector< FT_NodeToMove* > linkVec, *linkVecPtr;
  const int maxNbLinks = ( _shapeDim == 1 ) ? 2 : 6; // usual nb of links

  for ( size_t i = 0; i < _nodes.size(); ++i )
  {
    FT_NodeToMove& nn = _nodes[i];
    for ( size_t iN = 0; iN < nn._neighborNodes.size(); ++iN )
    {
      if ( !( linkVecPtr = neigborsMap.ChangeSeek( nn._neighborNodes[ iN ] )))
      {
        linkVecPtr = neigborsMap.Bound( nn._neighborNodes[ iN ], linkVec );
        linkVecPtr->reserve( maxNbLinks );
      }
      linkVecPtr->push_back( & nn );
    }
  }

  // fill in _nodesOrder

  _nodesOrder.reserve( _nodes.size() );

  std::list< FT_NodeToMove* > queue;
  queue.push_back( &_nodes[0] );
  _nodes[0]._nearParams = _nodes[0]._params; // to avoid re-adding to the queue

  while ( !queue.empty() )
  {
    FT_NodeToMove* nn = queue.front();
    queue.pop_front();

    _nodesOrder.push_back( nn - & _nodes[0] );

    // add neighbors to the queue and set their _nearParams = nn->_params
    for ( size_t iN = 0; iN < nn->_neighborNodes.size(); ++iN )
    {
      std::vector< FT_NodeToMove* >& linkVec = neigborsMap( nn->_neighborNodes[ iN ]);
      for ( size_t iL = 0; iL < linkVec.size(); ++iL )
      {
        FT_NodeToMove* nnn = linkVec[ iL ];
        if ( nnn != nn && nnn->_nearParams == 0 )
        {
          nnn->_nearParams = nn->_params;
          queue.push_back( nnn );
        }
      }
    }
  }
  _nodes[0]._nearParams = 0; // reset
}

//================================================================================
/*!
 * \brief Get node coordinates. Node IDs count from a unit
 */
//================================================================================

gp_Pnt FT_NodesOnGeom::getPoint( const int nodeID )
{
  const size_t dim = _nodeCoords->getNumberOfComponents();
  const double * xyz = _nodeCoords->getConstPointer() + ( dim * ( nodeID - 1 ));
  return gp_Pnt( xyz[0], xyz[1], dim == 2 ? 0 : xyz[2] );
}

//================================================================================
/*!
 * \brief change node coordinates
 */
//================================================================================

void FT_NodesOnGeom::moveNode( const int nodeID, const gp_Pnt& newXyz )
{
  const size_t dim = _nodeCoords->getNumberOfComponents();
  double z, *xyz = _nodeCoords->getPointer() + ( dim * ( nodeID - 1 ));
  newXyz.Coord( xyz[0], xyz[1], dim == 2 ? z : xyz[2] );
}
