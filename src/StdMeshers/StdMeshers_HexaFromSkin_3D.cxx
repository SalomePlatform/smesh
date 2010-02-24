//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : StdMeshers_HexaFromSkin_3D.cxx
// Created   : Wed Jan 27 12:28:07 2010
// Author    : Edward AGAPOV (eap)


#include "StdMeshers_HexaFromSkin_3D.hxx"

#include "SMDS_VolumeOfNodes.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_MesherHelper.hxx"

#include "utilities.h"

// Define error message
#ifdef _DEBUG_
#define BAD_MESH_ERR \
  error(SMESH_Comment("Can't detect block-wise structure of the input 2D mesh.\n" \
                      __FILE__ ":" )<<__LINE__)
#else
#define BAD_MESH_ERR \
  error(SMESH_Comment("Can't detect block-wise structure of the input 2D mesh"))
#endif

// Debug output
#define _DUMP_(msg) // cout << msg << endl



namespace
{
  enum EBoxSides //!< sides of the block
    {
      B_BOTTOM=0, B_RIGHT, B_TOP, B_LEFT, B_FRONT, B_BACK, B_UNDEFINED
    };
  const char* SBoxSides[] = //!< names of block sides
    {
      "BOTTOM", "RIGHT", "TOP", "LEFT", "FRONT", "BACK", "UNDEFINED"
    };
  enum EQuadEdge //!< edges of quadrangle side
    {
      Q_BOTTOM, Q_RIGHT, Q_TOP, Q_LEFT, Q_UNDEFINED
    };

  //================================================================================
  /*!
   * \brief return true if a node is at block corner
   */
  //================================================================================

  bool isCornerNode( const SMDS_MeshNode* n )
  {
    return n && n->NbInverseElements( SMDSAbs_Face ) % 2;
  }

  //================================================================================
  /*!
   * \brief check element type
   */
  //================================================================================

  bool isQuadrangle(const SMDS_MeshElement* e)
  {
    return ( e && e->NbNodes() == ( e->IsQuadratic() ? 8 : 4 ));
  }

  //================================================================================
  /*!
   * \brief return opposite node of a quadrangle face
   */
  //================================================================================

  const SMDS_MeshNode* oppositeNode(const SMDS_MeshElement* quad, int iNode)
  {
    return quad->GetNode( (iNode+2) % 4 );
  }

  //================================================================================
  /*!
   * \brief Convertor of a pair of integers to a sole index
   */
  struct _Indexer
  {
    int _xSize, _ySize;
    _Indexer( int xSize=0, int ySize=0 ): _xSize(xSize), _ySize(ySize) {}
    int size() const { return _xSize * _ySize; }
    int operator()(int x, int y) const { return y * _xSize + x; }
  };
  //================================================================================
  /*!
   * \brief Oriented convertor of a pair of integers to a sole index 
   */
  class _OrientedIndexer : public _Indexer
  {
  public:
    enum OriFlags //!< types of block side orientation
      {
        REV_X = 1, REV_Y = 2, SWAP_XY = 4, MAX_ORI = REV_X|REV_Y|SWAP_XY
      };
    _OrientedIndexer( const _Indexer& indexer, const int oriFlags ):
      _Indexer( indexer._xSize, indexer._ySize ),
      _xSize (indexer._xSize), _ySize(indexer._ySize),
      _xRevFun((oriFlags & REV_X) ? & reverse : & lazy),
      _yRevFun((oriFlags & REV_Y) ? & reverse : & lazy),
      _swapFun((oriFlags & SWAP_XY ) ? & swap : & lazy)
    {
      (*_swapFun)( _xSize, _ySize );
    }
    //!< Return index by XY
    int operator()(int x, int y) const
    {
      (*_xRevFun)( x, const_cast<int&>( _xSize ));
      (*_yRevFun)( y, const_cast<int&>( _ySize ));
      (*_swapFun)( x, y );
      return _Indexer::operator()(x,y);
    }
    //!< Return index for a corner
    int corner(bool xMax, bool yMax) const
    {
      int x = xMax, y = yMax, size = 2;
      (*_xRevFun)( x, size );
      (*_yRevFun)( y, size );
      (*_swapFun)( x, y );
      return _Indexer::operator()(x ? _Indexer::_xSize-1 : 0 , y ? _Indexer::_ySize-1 : 0);
    }
    int xSize() const { return _xSize; }
    int ySize() const { return _ySize; }
  private:
    _Indexer _indexer;
    int _xSize, _ySize;

    typedef void (*TFun)(int& x, int& y);
    TFun _xRevFun, _yRevFun, _swapFun;
    
    static void lazy(int&, int&) {}
    static void reverse(int& x, int& size) { x = size - x - 1; }
    static void swap(int& x, int& y) { std::swap(x,y); }
  };
  //================================================================================
  /*!
   * \brief Structure corresponding to the meshed side of block
   */
  struct _BlockSide
  {
    vector<const SMDS_MeshNode*> _grid;
    _Indexer                     _index;
    int                          _nbBlocksExpected;
    int                          _nbBlocksFound;

#ifdef _DEBUG_
#define _grid_access_(args) _grid.at( args )
#else
#define _grid_access_(args) _grid[ args ]
#endif
    //!< Return node at XY
    const SMDS_MeshNode* getNode(int x, int y) const { return _grid_access_( _index( x, y )); }
    //!< Set node at XY
    void setNode(int x, int y, const SMDS_MeshNode* n) { _grid_access_( _index( x, y )) = n; }
    //!< Return a corner node
    const SMDS_MeshNode* getCornerNode(bool isXMax, bool isYMax) const
    {
      return getNode( isXMax ? _index._xSize-1 : 0 , isYMax ? _index._ySize-1 : 0 );
    }
    const SMDS_MeshElement* getCornerFace(const SMDS_MeshNode* cornerNode) const;
    //!< True if all blocks this side belongs to have beem found
    bool isBound() const { return _nbBlocksExpected <= _nbBlocksFound; }
    //!< Return coordinates of node at XY
    gp_XYZ getXYZ(int x, int y) const { return SMESH_MeshEditor::TNodeXYZ( getNode( x, y )); }
    //!< Return gravity center of the four corners and the middle node
    gp_XYZ getGC() const
    {
      gp_XYZ xyz =
        getXYZ( 0, 0 ) +
        getXYZ( _index._xSize-1, 0 ) +
        getXYZ( 0, _index._ySize-1 ) +
        getXYZ( _index._xSize-1, _index._ySize-1 ) +
        getXYZ( _index._xSize/2, _index._ySize/2 );
      return xyz / 5;
    }
    //!< Return number of mesh faces
    int getNbFaces() const { return (_index._xSize-1) * (_index._ySize-1); }
  };
  //================================================================================
  /*!
   * \brief _BlockSide with changed orientation
   */
  struct _OrientedBlockSide
  {
    _BlockSide*       _side;
    _OrientedIndexer  _index;

    _OrientedBlockSide( _BlockSide* side=0, const int oriFlags=0 ):
      _side(side), _index(side ? side->_index : _Indexer(), oriFlags ) {}
    //!< return coordinates by XY
    gp_XYZ xyz(int x, int y) const
    {
      return SMESH_MeshEditor::TNodeXYZ( _side->_grid_access_( _index( x, y )) );
    }
    //!< safely return a node by XY
    const SMDS_MeshNode* node(int x, int y) const
    {
      int i = _index( x, y );
      return ( i < 0 || i >= _side->_grid.size()) ? 0 : _side->_grid[i];
    }
    //!< Return a corner node
    const SMDS_MeshNode* cornerNode(bool isXMax, bool isYMax) const
    {
      return _side->_grid_access_( _index.corner( isXMax, isYMax ));
    }
    //!< return its size in nodes
    int getHoriSize() const { return _index.xSize(); }
    int getVertSize() const  { return _index.ySize(); }
    //!< True if _side has been initialized
    operator bool() const { return _side; }
    //! Direct access to _side
    const _BlockSide* operator->() const { return _side; }
    _BlockSide* operator->() { return _side; }
  };
  //================================================================================
  /*!
   * \brief Meshed skin of block
   */
  struct _Block
  {
    _OrientedBlockSide _side[6]; // 6 sides of a sub-block

    const _OrientedBlockSide& getSide(int i) const { return _side[i]; }
  };
  //================================================================================
  /*!
   * \brief Skin mesh possibly containing several meshed blocks
   */
  class _Skin
  {
  public:

    int findBlocks(SMESH_Mesh& mesh);
    //!< return i-th block
    const _Block& getBlock(int i) const { return _blocks[i]; }
    //!< return error description
    const SMESH_Comment& error() const { return _error; }

  private:
    bool fillSide( _BlockSide& side, const SMDS_MeshElement* cornerQuad);
    bool fillRowsUntilCorner(const SMDS_MeshElement* quad,
                             const SMDS_MeshNode*    n1,
                             const SMDS_MeshNode*    n2,
                             vector<const SMDS_MeshNode*>& verRow1,
                             vector<const SMDS_MeshNode*>& verRow2,
                             bool alongN1N2 );
    _OrientedBlockSide findBlockSide( EBoxSides startBlockSide,
                                      EQuadEdge sharedSideEdge1,
                                      EQuadEdge sharedSideEdge2);
    //!< update own data and data of the side bound to block
    void setSideBoundToBlock( _BlockSide& side )
    {
      side._nbBlocksFound++;
      if ( side.isBound() )
      {
        _corner2sides[ side.getCornerNode(0,0) ].erase( &side );
        _corner2sides[ side.getCornerNode(1,0) ].erase( &side );
        _corner2sides[ side.getCornerNode(0,1) ].erase( &side );
        _corner2sides[ side.getCornerNode(1,1) ].erase( &side );
      }
    }
    //!< store reason of error
    int error(const SMESH_Comment& reason) { _error = reason; return 0; }

    SMESH_Comment      _error;

    list< _BlockSide > _allSides;
    vector< _Block >   _blocks;

    map< const SMDS_MeshNode*, set< _BlockSide* > > _corner2sides;
  };

  //================================================================================
  /*!
   * \brief Find and return number of submeshes corresponding to blocks
   */
  //================================================================================

  int _Skin::findBlocks(SMESH_Mesh& mesh)
  {
    SMESHDS_Mesh* meshDS = mesh.GetMeshDS();

    // Find a node at any block corner

    SMDS_NodeIteratorPtr nIt = meshDS->nodesIterator(/*idInceasingOrder=*/true);
    if ( !nIt->more() ) return error("Empty mesh");

    const SMDS_MeshNode* nCorner = 0;
    while ( nIt->more() )
    {
      nCorner = nIt->next();
      if ( isCornerNode( nCorner ))
        break;
      else
        nCorner = 0;
    }
    if ( !nCorner )
      return BAD_MESH_ERR;

    // --------------------------------------------------------------------
    // Find all block sides starting from mesh faces sharing the corner node
    // --------------------------------------------------------------------

    int nbFacesOnSides = 0;
    TIDSortedElemSet cornerFaces; // corner faces of found _BlockSide's
    list< const SMDS_MeshNode* > corners( 1, nCorner );
    list< const SMDS_MeshNode* >::iterator corner = corners.begin();
    while ( corner != corners.end() )
    {
      SMDS_ElemIteratorPtr faceIt = (*corner)->GetInverseElementIterator( SMDSAbs_Face );
      while ( faceIt->more() )
      {
        const SMDS_MeshElement* face = faceIt->next();
        if ( !cornerFaces.insert( face ).second )
          continue; // already loaded block side

        if ( !isQuadrangle( face ))
          return error("Non-quadrangle elements in the input mesh");

        if ( _allSides.empty() || !_allSides.back()._grid.empty() )
          _allSides.push_back( _BlockSide() );

        _BlockSide& side = _allSides.back();
        if ( !fillSide( side, face ) )
        {
          if ( !_error.empty() )
            return false;
        }
        else
        {
          for ( int isXMax = 0; isXMax < 2; ++isXMax )
            for ( int isYMax = 0; isYMax < 2; ++isYMax )
            {
              const SMDS_MeshNode* nCorner = side.getCornerNode(isXMax,isYMax );
              if ( !isCornerNode( nCorner ))
                return BAD_MESH_ERR;
              _corner2sides[ nCorner ].insert( &side );
              corners.push_back( nCorner );
              cornerFaces.insert( side.getCornerFace( nCorner ));
            }
          nbFacesOnSides += side.getNbFaces();
        }
      }
      ++corner;

      // find block sides of other domains if any
      if ( corner == corners.end() && nbFacesOnSides < mesh.NbQuadrangles() )
      {
        while ( nIt->more() )
        {
          nCorner = nIt->next();
          if ( isCornerNode( nCorner ))
            corner = corners.insert( corner, nCorner );
        }
        nbFacesOnSides = mesh.NbQuadrangles();
      }
    }
    
    if ( _allSides.empty() )
      return BAD_MESH_ERR;
    if ( _allSides.back()._grid.empty() )
      _allSides.pop_back();

    // ---------------------------
    // Organize sides into blocks
    // ---------------------------

    // analyse sharing of sides by blocks
    int nbBlockSides = 0; // nb of block sides taking into account their sharing
    list < _BlockSide >::iterator sideIt = _allSides.begin();
    for ( ; sideIt != _allSides.end(); ++sideIt )
    {
      _BlockSide& side = *sideIt;
      bool isSharedSide = true;
      for ( int isXMax = 0; isXMax < 2; ++isXMax )
        for ( int isYMax = 0; isYMax < 2; ++isYMax )
          if ( _corner2sides[ side.getCornerNode(isXMax,isYMax) ].size() < 5 )
            isSharedSide = false;
      side._nbBlocksFound = 0;
      side._nbBlocksExpected = isSharedSide ? 2 : 1;
      nbBlockSides += side._nbBlocksExpected;
    }

    // find sides of each block
    int nbBlocks = 0;
    while ( nbBlockSides >= 6 )
    {
      // get any side not bound to all blocks to belongs to
      sideIt = _allSides.begin();
      while ( sideIt != _allSides.end() && sideIt->isBound())
        ++sideIt;

      // start searching for block sides from the got side
      bool ok = true;
      if ( _blocks.empty() || _blocks.back()._side[B_FRONT] )
        _blocks.resize( _blocks.size() + 1 );

      _Block& block = _blocks.back();
      block._side[B_FRONT] = &(*sideIt);
      setSideBoundToBlock( *sideIt );
      nbBlockSides--;
      
      // edges of neighbour sides of B_FRONT corresponding to front's edges
      EQuadEdge edgeOfFront[4] = { Q_BOTTOM, Q_RIGHT, Q_TOP, Q_LEFT };
      EQuadEdge edgeToFind [4] = { Q_BOTTOM, Q_LEFT, Q_BOTTOM, Q_LEFT };
      for ( int i = Q_BOTTOM; ok && i <= Q_LEFT; ++i )
        ok = ( block._side[i] = findBlockSide( B_FRONT, edgeOfFront[i], edgeToFind[i]));
      if ( ok )
        ok = ( block._side[B_BACK] = findBlockSide( B_TOP, Q_TOP, Q_TOP ));

      // count the found sides
      _DUMP_(endl);
      for (int i = 0; i < B_UNDEFINED; ++i )
      {
        _DUMP_("** Block side "<< SBoxSides[i]);
        if ( block._side[ i ] )
        {
          if ( ok && i != B_FRONT)
          {
            setSideBoundToBlock( *block._side[ i ]._side );
            nbBlockSides--;
          }
          _DUMP_("Corner 0,0 "<< block._side[ i ].cornerNode(0,0));
          _DUMP_("Corner 1,0 "<< block._side[ i ].cornerNode(1,0));    
          _DUMP_("Corner 1,1 "<< block._side[ i ].cornerNode(1,1));
          _DUMP_("Corner 0,1 "<< block._side[ i ].cornerNode(0,1));
        }
        else
        {
          _DUMP_("Not found"<<endl);
        }
      }
      if ( !ok )
        block._side[0] = block._side[1] = block._side[2] =
          block._side[3] = block._side[4] = block._side[5] = 0;
      else
        nbBlocks++;
    }
    if ( nbBlocks == 0 && _error.empty() )
      return BAD_MESH_ERR;

    return nbBlocks;
  }

  //================================================================================
  /*!
   * \brief Fill block side data starting from its corner quadrangle
   */
  //================================================================================

  bool _Skin::fillSide( _BlockSide& side, const SMDS_MeshElement* cornerQuad)
  {
    // Find out size of block side mesured in nodes and by the way find two rows
    // of nodes in two directions.

    int x, y, nbX, nbY;
    const SMDS_MeshElement* firstQuad = cornerQuad;
    {
      // get corner node of cornerQuad
      const SMDS_MeshNode* nCorner = 0;
      for ( int i = 0; i < 4 && !nCorner; ++i )
        if ( isCornerNode( firstQuad->GetNode(i)))
          nCorner = firstQuad->GetNode(i);
      if ( !nCorner ) return false;

      // get a node on block edge
      int iCorner = firstQuad->GetNodeIndex( nCorner );
      const SMDS_MeshNode* nOnEdge = firstQuad->GetNode( (iCorner+1) % 4);

      // find out size of block side
      vector<const SMDS_MeshNode*> horRow1, horRow2, verRow1, verRow2;
      if ( !fillRowsUntilCorner( firstQuad, nCorner, nOnEdge, horRow1, horRow2, true ) ||
           !fillRowsUntilCorner( firstQuad, nCorner, nOnEdge, verRow1, verRow2, false ))
        return false;
      nbX = horRow1.size(), nbY = verRow1.size();

      // store found nodes
      side._index._xSize = horRow1.size();
      side._index._ySize = verRow1.size();
      side._grid.resize( side._index.size(), NULL );

      for ( x = 0; x < horRow1.size(); ++x )
      {
        side.setNode( x, 0, horRow1[x] );
        side.setNode( x, 1, horRow2[x] );
      }
      for ( y = 0; y < verRow1.size(); ++y )
      {
        side.setNode( 0, y, verRow1[y] );
        side.setNode( 1, y, verRow2[y] );
      }
    }
    // Find the rest nodes

    y = 1; // y of the row to fill
    TIDSortedElemSet emptySet, avoidSet;
    while ( ++y < nbY )
    {
      // get next firstQuad in the next row of quadrangles
      //
      //          n2up
      //     o---o               <- y row
      //     |   |
      //     o---o  o  o  o  o   <- found nodes
      //n1down    n2down       
      //
      int i1down, i2down, i2up;
      const SMDS_MeshNode* n1down = side.getNode( 0, y-1 );
      const SMDS_MeshNode* n2down = side.getNode( 1, y-1 );
      avoidSet.clear(); avoidSet.insert( firstQuad );
      firstQuad = SMESH_MeshEditor::FindFaceInSet( n1down, n2down, emptySet, avoidSet,
                                                   &i1down, &i2down);
      if ( !isQuadrangle( firstQuad ))
        return BAD_MESH_ERR;

      const SMDS_MeshNode* n2up = oppositeNode( firstQuad, i1down );
      avoidSet.clear(); avoidSet.insert( firstQuad );

      // find the rest nodes in the y-th row by faces in the row

      x = 1; 
      while ( ++x < nbX )
      {
        const SMDS_MeshElement* quad = SMESH_MeshEditor::FindFaceInSet( n2up, n2down, emptySet,
                                                                        avoidSet, &i2up, &i2down);
        if ( !isQuadrangle( quad ))
          return BAD_MESH_ERR;

        n2up   = oppositeNode( quad, i2down );
        n2down = oppositeNode( quad, i2up );
        avoidSet.clear(); avoidSet.insert( quad );

        side.setNode( x, y, n2up );
      }
    }

    return true;
  }

  //================================================================================
  /*!
   * \brief Try to find a block side adjacent to the given side by given edge
   */
  //================================================================================

  _OrientedBlockSide _Skin::findBlockSide( EBoxSides startBlockSide,
                                           EQuadEdge sharedSideEdge1,
                                           EQuadEdge sharedSideEdge2)
  {
    _Block& block = _blocks.back();
    _OrientedBlockSide& side1 = block._side[ startBlockSide ];

    // get corner nodes of the given block edge
    bool xMax1=0, yMax1=0, xMax2=1, yMax2=1;
    switch( sharedSideEdge1 )
    {
    case Q_BOTTOM: yMax2 = 0; break;
    case Q_RIGHT:  xMax1 = 1; break;
    case Q_TOP:    yMax1 = 1; break;
    case Q_LEFT:   xMax2 = 0; break;
    default:
      error(SMESH_Comment("Internal error at")<<__FILE__<<":"<<__LINE__);
      return 0;
    }
    const SMDS_MeshNode* n1 = side1.cornerNode( xMax1, yMax1);
    const SMDS_MeshNode* n2 = side1.cornerNode( xMax2, yMax2);

    set< _BlockSide* >& sidesWithN1 = _corner2sides[ n1 ];
    set< _BlockSide* >& sidesWithN2 = _corner2sides[ n2 ];
    if ( sidesWithN1.empty() || sidesWithN2.empty() )
    {
      _DUMP_("no sides by nodes "<< n1->GetID() << " " << n2->GetID() );
      return 0;
    }

    // find all sides sharing both nodes n1 and n2
    set< _BlockSide* > sidesOnEdge;
    set< _BlockSide* >::iterator sideIt;
    std::set_intersection( sidesWithN1.begin(), sidesWithN1.end(),
                           sidesWithN2.begin(), sidesWithN2.end(),
                           inserter( sidesOnEdge, sidesOnEdge.begin()));

    // exclude loaded sides of block from sidesOnEdge
    int nbLoadedSides = 0;
    for (int i = 0; i < B_UNDEFINED; ++i )
    {
      if ( block._side[ i ] )
      {
        nbLoadedSides++;
        sidesOnEdge.erase( block._side[ i ]._side );
      }
    }
    int nbSidesOnEdge = sidesOnEdge.size();
    _DUMP_("nbSidesOnEdge "<< nbSidesOnEdge);
    if ( nbSidesOnEdge == 0 )
      return 0;

    _BlockSide* foundSide = 0;
    if ( nbSidesOnEdge == 1 || nbSidesOnEdge == 2 && nbLoadedSides == 1 )
    {
      foundSide = *sidesOnEdge.begin();
    }
    else
    {
      // Select one of found sides most close to startBlockSide

      // gravity center of already loaded block sides
      gp_XYZ gc;
      for (int i = 0; i < B_UNDEFINED; ++i )
        if ( block._side[ i ] )
        {
          gc += block._side[ i ]._side->getGC();
          nbLoadedSides++;
        }
      gc /= nbLoadedSides;

      gp_XYZ p1 ( n1->X(),n1->Y(),n1->Z()),  p2 (n2->X(),n2->Y(),n2->Z());
      gp_Vec p2p1( p1 - p2 );

      const SMDS_MeshElement* face1 = side1->getCornerFace( n1 );
      gp_XYZ p1Op = SMESH_MeshEditor::TNodeXYZ( oppositeNode( face1, face1->GetNodeIndex(n1)));
      gp_Vec side1Dir( p1, p1Op );
      
      map < double , _BlockSide* > angleOfSide;
      for (sideIt = sidesOnEdge.begin(); sideIt != sidesOnEdge.end(); ++sideIt )
      {
        _BlockSide* sideI = *sideIt;
        const SMDS_MeshElement* faceI = sideI->getCornerFace( n1 );
        gp_XYZ p1Op = SMESH_MeshEditor::TNodeXYZ( oppositeNode( faceI, faceI->GetNodeIndex(n1)));
        gp_Vec sideIDir( p1, p1Op );
        double angle = side1Dir.AngleWithRef( sideIDir, p2p1 );
        if ( angle < 0 ) angle += 2 * PI;
        angleOfSide.insert( make_pair( angle, sideI ));
      }
      gp_Vec gcDir( p1, gc );
      double gcAngle = side1Dir.AngleWithRef( gcDir, p2p1 );
      foundSide = gcAngle < 0 ? angleOfSide.rbegin()->second : angleOfSide.begin()->second;
    }

    // Orient the found side correctly

    // corners of found side corresponding to nodes n1 and n2
    xMax1= yMax1 = 0; xMax2 = yMax2 = 1;
    switch( sharedSideEdge2 )
    {
    case Q_BOTTOM: yMax2 = 0; break;
    case Q_RIGHT:  xMax1 = 1; break;
    case Q_TOP:    yMax1 = 1; break;
    case Q_LEFT:   xMax2 = 0; break;
    default:
      error(SMESH_Comment("Internal error at")<<__FILE__<<":"<<__LINE__);
      return 0;
    }
    for ( int ori = 0; ori < _OrientedIndexer::MAX_ORI+1; ++ori )
    {
      _OrientedBlockSide orientedSide( foundSide, ori );
      const SMDS_MeshNode* n12 = orientedSide.cornerNode( xMax1, yMax1);
      const SMDS_MeshNode* n22 = orientedSide.cornerNode( xMax2, yMax2);
      if ( n1 == n12 && n2 == n22 )
        return orientedSide;
    }
    error(SMESH_Comment("Failed to orient a block side found by edge ")<<sharedSideEdge1
          << " of side " << startBlockSide
          << " of block " << _blocks.size());
    return 0;
  }

  //================================================================================
  /*!
   * \brief: Fill rows (which are actually columns,if !alongN1N2) of nodes starting
   * from the given quadrangle until another block corner encounters.
   *  n1 and n2 are at bottom of quad, n1 is at block corner.
   */
  //================================================================================

  bool _Skin::fillRowsUntilCorner(const SMDS_MeshElement*       quad,
                                  const SMDS_MeshNode*          n1,
                                  const SMDS_MeshNode*          n2,
                                  vector<const SMDS_MeshNode*>& row1,
                                  vector<const SMDS_MeshNode*>& row2,
                                  const bool                    alongN1N2 )
  {
    const SMDS_MeshNode* corner1 = n1;

    // Store nodes of quad in the rows and find new n1 and n2 to get
    // the next face so that new n2 is on block edge
    int i1 = quad->GetNodeIndex( n1 );
    int i2 = quad->GetNodeIndex( n2 );
    row1.clear(); row2.clear();
    row1.push_back( n1 );
    if ( alongN1N2 )
    {
      row1.push_back( n2 );
      row2.push_back( oppositeNode( quad, i2 ));
      row2.push_back( n1 = oppositeNode( quad, i1 ));
    }
    else
    {
      row2.push_back( n2 );
      row1.push_back( n2 = oppositeNode( quad, i2 ));
      row2.push_back( n1 = oppositeNode( quad, i1 ));
    }

    // Find the rest nodes
    TIDSortedElemSet emptySet, avoidSet;
    while ( !isCornerNode( n2 ))
    {
      avoidSet.clear(); avoidSet.insert( quad );
      quad = SMESH_MeshEditor::FindFaceInSet( n1, n2, emptySet, avoidSet, &i1, &i2 );
      if ( !isQuadrangle( quad ))
        return BAD_MESH_ERR;

      row1.push_back( n2 = oppositeNode( quad, i1 ));
      row2.push_back( n1 = oppositeNode( quad, i2 ));
    }
    return n1 != corner1;
  }

  //================================================================================
  /*!
   * \brief Return a corner face by a corner node
   */
  //================================================================================

  const SMDS_MeshElement* _BlockSide::getCornerFace(const SMDS_MeshNode* cornerNode) const
  {
    int x, y, isXMax, isYMax, found = 0;
    for ( isXMax = 0; isXMax < 2; ++isXMax )
    {
      for ( isYMax = 0; isYMax < 2; ++isYMax )
      {
        x = isXMax ? _index._xSize-1 : 0;
        y = isYMax ? _index._ySize-1 : 0;
        found = ( getNode(x,y) == cornerNode );
        if ( found ) break;
      }
      if ( found ) break;
    }
    if ( !found ) return 0;
    int dx = isXMax ? -1 : +1;
    int dy = isYMax ? -1 : +1;
    const SMDS_MeshNode* n1 = getNode(x,y);
    const SMDS_MeshNode* n2 = getNode(x+dx,y);
    const SMDS_MeshNode* n3 = getNode(x,y+dy);
    const SMDS_MeshNode* n4 = getNode(x+dx,y+dy);
    return SMDS_Mesh::FindFace(n1, n2, n3, n4 );
  }

}

//=======================================================================
//function : StdMeshers_HexaFromSkin_3D
//purpose  : 
//=======================================================================

StdMeshers_HexaFromSkin_3D::StdMeshers_HexaFromSkin_3D(int hypId, int studyId, SMESH_Gen* gen)
  :SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("StdMeshers_HexaFromSkin_3D::StdMeshers_HexaFromSkin_3D");
  _name = "HexaFromSkin_3D";
}

StdMeshers_HexaFromSkin_3D::~StdMeshers_HexaFromSkin_3D()
{
  MESSAGE("StdMeshers_HexaFromSkin_3D::~StdMeshers_HexaFromSkin_3D");
}

//================================================================================
/*!
 * \brief Main method, which generates hexaheda
 */
//================================================================================

bool StdMeshers_HexaFromSkin_3D::Compute(SMESH_Mesh & aMesh, SMESH_MesherHelper* aHelper)
{
  _Skin skin;
  int nbBlocks = skin.findBlocks(aMesh);
  if ( nbBlocks == 0 )
    return error( skin.error());

  vector< vector< const SMDS_MeshNode* > > columns;
  int x, xSize, y, ySize, z, zSize;
  _Indexer colIndex;

  for ( int i = 0; i < nbBlocks; ++i )
  {
    const _Block& block = skin.getBlock( i );

    // ------------------------------------------
    // Fill columns of nodes with existing nodes
    // ------------------------------------------

    xSize = block.getSide(B_BOTTOM).getHoriSize();
    ySize = block.getSide(B_BOTTOM).getVertSize();
    zSize = block.getSide(B_FRONT ).getVertSize();
    int X = xSize - 1, Y = ySize - 1, Z = zSize - 1;
    colIndex = _Indexer( xSize, ySize );
    columns.resize( colIndex.size() );

    // fill node columns by front and back box sides
    for ( x = 0; x < xSize; ++x ) {
      vector< const SMDS_MeshNode* >& column0 = columns[ colIndex( x, 0 )];
      vector< const SMDS_MeshNode* >& column1 = columns[ colIndex( x, Y )];
      column0.resize( zSize );
      column1.resize( zSize );
      for ( z = 0; z < zSize; ++z ) {
        column0[ z ] = block.getSide(B_FRONT).node( x, z );
        column1[ z ] = block.getSide(B_BACK) .node( x, z );
      }
    }
    // fill node columns by left and right box sides
    for ( y = 1; y < ySize-1; ++y ) {
      vector< const SMDS_MeshNode* >& column0 = columns[ colIndex( 0, y )];
      vector< const SMDS_MeshNode* >& column1 = columns[ colIndex( X, y )];
      column0.resize( zSize );
      column1.resize( zSize );
      for ( z = 0; z < zSize; ++z ) {
        column0[ z ] = block.getSide(B_LEFT) .node( y, z );
        column1[ z ] = block.getSide(B_RIGHT).node( y, z );
      }
    }
    // get nodes from top and bottom box sides
    for ( x = 1; x < xSize-1; ++x ) {
      for ( y = 1; y < ySize-1; ++y ) {
        vector< const SMDS_MeshNode* >& column = columns[ colIndex( x, y )];
        column.resize( zSize );
        column.front() = block.getSide(B_BOTTOM).node( x, y );
        column.back()  = block.getSide(B_TOP)   .node( x, y );
      }
    }

    // ----------------------------
    // Add internal nodes of a box
    // ----------------------------
    // projection points of internal nodes on box subshapes by which
    // coordinates of internal nodes are computed
    vector<gp_XYZ> pointOnShape( SMESH_Block::ID_Shell );

    // projections on vertices are constant
    pointOnShape[ SMESH_Block::ID_V000 ] = block.getSide(B_BOTTOM).xyz( 0, 0 );
    pointOnShape[ SMESH_Block::ID_V100 ] = block.getSide(B_BOTTOM).xyz( X, 0 );
    pointOnShape[ SMESH_Block::ID_V010 ] = block.getSide(B_BOTTOM).xyz( 0, Y );
    pointOnShape[ SMESH_Block::ID_V110 ] = block.getSide(B_BOTTOM).xyz( X, Y );
    pointOnShape[ SMESH_Block::ID_V001 ] = block.getSide(B_TOP).xyz( 0, 0 );
    pointOnShape[ SMESH_Block::ID_V101 ] = block.getSide(B_TOP).xyz( X, 0 );
    pointOnShape[ SMESH_Block::ID_V011 ] = block.getSide(B_TOP).xyz( 0, Y );
    pointOnShape[ SMESH_Block::ID_V111 ] = block.getSide(B_TOP).xyz( X, Y );

    for ( x = 1; x < xSize-1; ++x )
    {
      gp_XYZ params; // normalized parameters of internal node within a unit box
      params.SetCoord( 1, x / double(X) );
      for ( y = 1; y < ySize-1; ++y )
      {
        params.SetCoord( 2, y / double(Y) );
        // column to fill during z loop
        vector< const SMDS_MeshNode* >& column = columns[ colIndex( x, y )];
        // projections on horizontal edges
        pointOnShape[ SMESH_Block::ID_Ex00 ] = block.getSide(B_BOTTOM).xyz( x, 0 );
        pointOnShape[ SMESH_Block::ID_Ex10 ] = block.getSide(B_BOTTOM).xyz( x, Y );
        pointOnShape[ SMESH_Block::ID_E0y0 ] = block.getSide(B_BOTTOM).xyz( 0, y );
        pointOnShape[ SMESH_Block::ID_E1y0 ] = block.getSide(B_BOTTOM).xyz( X, y );
        pointOnShape[ SMESH_Block::ID_Ex01 ] = block.getSide(B_TOP).xyz( x, 0 );
        pointOnShape[ SMESH_Block::ID_Ex11 ] = block.getSide(B_TOP).xyz( x, Y );
        pointOnShape[ SMESH_Block::ID_E0y1 ] = block.getSide(B_TOP).xyz( 0, y );
        pointOnShape[ SMESH_Block::ID_E1y1 ] = block.getSide(B_TOP).xyz( X, y );
        // projections on horizontal sides
        pointOnShape[ SMESH_Block::ID_Fxy0 ] = block.getSide(B_BOTTOM).xyz( x, y );
        pointOnShape[ SMESH_Block::ID_Fxy1 ] = block.getSide(B_TOP)   .xyz( x, y );
        for ( z = 1; z < zSize-1; ++z ) // z loop
        {
          params.SetCoord( 3, z / double(Z) );
          // projections on vertical edges
          pointOnShape[ SMESH_Block::ID_E00z ] = block.getSide(B_FRONT).xyz( 0, z );    
          pointOnShape[ SMESH_Block::ID_E10z ] = block.getSide(B_FRONT).xyz( X, z );    
          pointOnShape[ SMESH_Block::ID_E01z ] = block.getSide(B_BACK).xyz( 0, z );    
          pointOnShape[ SMESH_Block::ID_E11z ] = block.getSide(B_BACK).xyz( X, z );
          // projections on vertical sides
          pointOnShape[ SMESH_Block::ID_Fx0z ] = block.getSide(B_FRONT).xyz( x, z );    
          pointOnShape[ SMESH_Block::ID_Fx1z ] = block.getSide(B_BACK) .xyz( x, z );    
          pointOnShape[ SMESH_Block::ID_F0yz ] = block.getSide(B_LEFT) .xyz( y, z );    
          pointOnShape[ SMESH_Block::ID_F1yz ] = block.getSide(B_RIGHT).xyz( y, z );

          // compute internal node coordinates
          gp_XYZ coords;
          SMESH_Block::ShellPoint( params, pointOnShape, coords );
          column[ z ] = aHelper->AddNode( coords.X(), coords.Y(), coords.Z() );

#ifdef DEB_GRID
          // debug
          //cout << "----------------------------------------------------------------------"<<endl;
          //for ( int id = SMESH_Block::ID_V000; id < SMESH_Block::ID_Shell; ++id)
          //{
          //  gp_XYZ p = pointOnShape[ id ];
          //  SMESH_Block::DumpShapeID( id,cout)<<" ( "<<p.X()<<", "<<p.Y()<<", "<<p.Z()<<" )"<<endl;
          //}
          //cout << "Params: ( "<< params.X()<<", "<<params.Y()<<", "<<params.Z()<<" )"<<endl;
          //cout << "coords: ( "<< coords.X()<<", "<<coords.Y()<<", "<<coords.Z()<<" )"<<endl;
#endif
        }
      }
    }
    // ----------------
    // Add hexahedrons
    // ----------------

    // find out orientation
    const SMDS_MeshNode* n000 = block.getSide(B_BOTTOM).cornerNode( 0, 0 );
    const SMDS_MeshNode* n100 = block.getSide(B_BOTTOM).cornerNode( 1, 0 );
    const SMDS_MeshNode* n010 = block.getSide(B_BOTTOM).cornerNode( 0, 1 );
    const SMDS_MeshNode* n110 = block.getSide(B_BOTTOM).cornerNode( 1, 1 );
    const SMDS_MeshNode* n001 = block.getSide(B_TOP).cornerNode( 0, 0 );
    const SMDS_MeshNode* n101 = block.getSide(B_TOP).cornerNode( 1, 0 );
    const SMDS_MeshNode* n011 = block.getSide(B_TOP).cornerNode( 0, 1 );
    const SMDS_MeshNode* n111 = block.getSide(B_TOP).cornerNode( 1, 1 );
    SMDS_VolumeOfNodes probeVolume (n000,n010,n110,n100,
                                    n001,n011,n111,n101);
    bool isForw = SMDS_VolumeTool( &probeVolume ).IsForward();

    // add elements
    for ( x = 0; x < xSize-1; ++x ) {
      for ( y = 0; y < ySize-1; ++y ) {
        vector< const SMDS_MeshNode* >& col00 = columns[ colIndex( x, y )];
        vector< const SMDS_MeshNode* >& col10 = columns[ colIndex( x+1, y )];
        vector< const SMDS_MeshNode* >& col01 = columns[ colIndex( x, y+1 )];
        vector< const SMDS_MeshNode* >& col11 = columns[ colIndex( x+1, y+1 )];
        // bottom face normal of a hexa mush point outside the volume
        if ( isForw )
          for ( z = 0; z < zSize-1; ++z )
            aHelper->AddVolume(col00[z],   col01[z],   col11[z],   col10[z],
                               col00[z+1], col01[z+1], col11[z+1], col10[z+1]);
        else
          for ( z = 0; z < zSize-1; ++z )
            aHelper->AddVolume(col00[z],   col10[z],   col11[z],   col01[z],
                               col00[z+1], col10[z+1], col11[z+1], col01[z+1]);
      }
    }
  } // loop on blocks

  return true;
}

//================================================================================
/*!
 * \brief Evaluate nb of hexa
 */
//================================================================================

bool StdMeshers_HexaFromSkin_3D::Evaluate(SMESH_Mesh &         aMesh,
                                          const TopoDS_Shape & aShape,
                                          MapShapeNbElems&     aResMap)
{
  _Skin skin;
  int nbBlocks = skin.findBlocks(aMesh);
  if ( nbBlocks == 0 )
    return error( skin.error());

  bool secondOrder = aMesh.NbFaces( ORDER_QUADRATIC );

  int entity = secondOrder ? SMDSEntity_Quad_Hexa : SMDSEntity_Hexa;
  vector<int>& nbByType = aResMap[ aMesh.GetSubMesh( aShape )];
  if ( entity >= nbByType.size() )
    nbByType.resize( SMDSEntity_Last, 0 );

  for ( int i = 0; i < nbBlocks; ++i )
  {
    const _Block& block = skin.getBlock( i );

    int nbX = block.getSide(B_BOTTOM).getHoriSize();
    int nbY = block.getSide(B_BOTTOM).getVertSize();
    int nbZ = block.getSide(B_FRONT ).getVertSize();

    int nbHexa  = (nbX-1) * (nbY-1) * (nbZ-1);
    int nbNodes = (nbX-2) * (nbY-2) * (nbZ-2);
    if ( secondOrder )
      nbNodes +=
        (nbX-2) * (nbY-2) * (nbZ-1) +
        (nbX-2) * (nbY-1) * (nbZ-2) +
        (nbX-1) * (nbY-2) * (nbZ-2);


    nbByType[ entity ] += nbHexa;
    nbByType[ SMDSEntity_Node ] += nbNodes;
  }

  return true;
}

//================================================================================
/*!
 * \brief Abstract method must be defined but does nothing
 */
//================================================================================

bool StdMeshers_HexaFromSkin_3D::CheckHypothesis(SMESH_Mesh&, const TopoDS_Shape&,
                                                 Hypothesis_Status& aStatus)
{
  aStatus = SMESH_Hypothesis::HYP_OK;
  return true;
}

//================================================================================
/*!
 * \brief Abstract method must be defined but just reports an error as this
 *  algo is not intended to work with shapes
 */
//================================================================================

bool StdMeshers_HexaFromSkin_3D::Compute(SMESH_Mesh&, const TopoDS_Shape&)
{
  return error("Algorithm can't work with geometrical shapes");
}

