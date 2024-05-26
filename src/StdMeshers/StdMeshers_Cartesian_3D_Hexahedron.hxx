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
//  File   : StdMeshers_Cartesian_3D_Hexahedron.hxx
//  Module : SMESH
//  Purpose: Make BodyFitting mesh algorithm more modular and testable
//

#ifndef _SMESH_Cartesian_3D_HEXAHEDRON_HXX_
#define _SMESH_Cartesian_3D_HEXAHEDRON_HXX_

// BOOST 
#include <boost/container/flat_map.hpp>

// STD
#include <utilities.h>
#include <vector>

// SMESH
#include "SMESH_StdMeshers.hxx"
#include "StdMeshers_Cartesian_3D_Grid.hxx"

using namespace std;
using namespace SMESH;
namespace
{
  // --------------------------------------------------------------------------
  /*!
   * \brief Return cells sharing a link
   */
  struct CellsAroundLink
  {
    int    _iDir;
    int    _dInd[4][3];
    size_t _nbCells[3];
    int    _i,_j,_k;
    Grid*  _grid;

    CellsAroundLink( Grid* grid, int iDir ):
      _iDir( iDir ),
      _dInd{ {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} },
      _nbCells{ grid->_coords[0].size() - 1,
          grid->_coords[1].size() - 1,
          grid->_coords[2].size() - 1 },
      _grid( grid )
    {
      const int iDirOther[3][2] = {{ 1,2 },{ 0,2 },{ 0,1 }};
      _dInd[1][ iDirOther[iDir][0] ] = -1;
      _dInd[2][ iDirOther[iDir][1] ] = -1;
      _dInd[3][ iDirOther[iDir][0] ] = -1; _dInd[3][ iDirOther[iDir][1] ] = -1;
    }
    void Init( int i, int j, int k, int link12 = 0 )
    {
      int iL = link12 % 4;
      _i = i - _dInd[iL][0];
      _j = j - _dInd[iL][1];
      _k = k - _dInd[iL][2];
    }
    bool GetCell( int iL, int& i, int& j, int& k, int& cellIndex, int& linkIndex )
    {
      i =  _i + _dInd[iL][0];
      j =  _j + _dInd[iL][1];
      k =  _k + _dInd[iL][2];
      if ( i < 0 || i >= (int)_nbCells[0] ||
           j < 0 || j >= (int)_nbCells[1] ||
           k < 0 || k >= (int)_nbCells[2] )
        return false;
      cellIndex = _grid->CellIndex( i,j,k );
      linkIndex = iL + _iDir * 4;
      return true;
    }
  };
}
  
// --------------------------------------------------------------------------
/*!
  * \brief Class representing topology of the hexahedron and creating a mesh
  *        volume basing on analysis of hexahedron intersection with geometry
  */
class STDMESHERS_EXPORT Hexahedron
{
  // --------------------------------------------------------------------------------
  struct _Face;
  struct _Link;
  enum IsInternalFlag { IS_NOT_INTERNAL, IS_INTERNAL, IS_CUT_BY_INTERNAL_FACE };
  // --------------------------------------------------------------------------------
  struct _Node //!< node either at a hexahedron corner or at intersection
  {
    const SMDS_MeshNode*    _node;        // mesh node at hexahedron corner
    const SMDS_MeshNode*    _boundaryCornerNode; // missing mesh node due to hex truncation on the boundary
    const B_IntersectPoint* _intPoint;
    const _Face*            _usedInFace;
    char                    _isInternalFlags;

    _Node(const SMDS_MeshNode* n=0, const B_IntersectPoint* ip=0)
      :_node(n), _intPoint(ip), _usedInFace(0), _isInternalFlags(0) {} 
    const SMDS_MeshNode*    Node() const
    { return ( _intPoint && _intPoint->_node ) ? _intPoint->_node : _node; }
    const SMDS_MeshNode*    BoundaryNode() const
    { return _node ? _node : _boundaryCornerNode; }
    const E_IntersectPoint* EdgeIntPnt() const
    { return static_cast< const E_IntersectPoint* >( _intPoint ); }
    const F_IntersectPoint* FaceIntPnt() const
    { return static_cast< const F_IntersectPoint* >( _intPoint ); }
    const vector< TGeomID >& faces() const { return _intPoint->_faceIDs; }
    TGeomID face(size_t i) const { return _intPoint->_faceIDs[ i ]; }
    void SetInternal( IsInternalFlag intFlag ) { _isInternalFlags |= intFlag; }
    bool IsCutByInternal() const { return _isInternalFlags & IS_CUT_BY_INTERNAL_FACE; }
    bool IsUsedInFace( const _Face* polygon = 0 )
    {
      return polygon ? ( _usedInFace == polygon ) : bool( _usedInFace );
    }
    TGeomID IsLinked( const B_IntersectPoint* other,
                      TGeomID                 avoidFace=-1 ) const // returns id of a common face
    {
      return _intPoint ? _intPoint->HasCommonFace( other, avoidFace ) : 0;
    }
    bool IsOnFace( TGeomID faceID ) const // returns true if faceID is found
    {
      return _intPoint ? _intPoint->IsOnFace( faceID ) : false;
    }
    size_t GetCommonFaces( const B_IntersectPoint * other, TGeomID* common ) const
    {
      return _intPoint && other ? _intPoint->GetCommonFaces( other, common ) : 0;
    }
    gp_Pnt Point() const
    {
      if ( const SMDS_MeshNode* n = Node() )
        return SMESH_NodeXYZ( n );
      if ( const E_IntersectPoint* eip =
            dynamic_cast< const E_IntersectPoint* >( _intPoint ))
        return eip->_point;
      return gp_Pnt( 1e100, 0, 0 );
    }
    TGeomID ShapeID() const
    {
      if ( const E_IntersectPoint* eip = dynamic_cast< const E_IntersectPoint* >( _intPoint ))
        return eip->_shapeID;
      return 0;
    }

    void Add( const E_IntersectPoint* ip );
  };
  // --------------------------------------------------------------------------------
  struct _Link // link connecting two _Node's
  {
    _Node* _nodes[2];
    _Face* _faces[2]; // polygons sharing a link
    vector< const F_IntersectPoint* > _fIntPoints; // GridLine intersections with FACEs
    vector< _Node* >                  _fIntNodes;   // _Node's at _fIntPoints
    vector< _Link >                   _splits;
    _Link(): _faces{ 0, 0 } {}
  };
  // --------------------------------------------------------------------------------
  struct _OrientedLink
  {
    _Link* _link;
    bool   _reverse;
    _OrientedLink( _Link* link=0, bool reverse=false ): _link(link), _reverse(reverse) {}
    void Reverse() { _reverse = !_reverse; }
    size_t NbResultLinks() const { return _link->_splits.size(); }
    _OrientedLink ResultLink(int i) const
    {
      return _OrientedLink(&_link->_splits[_reverse ? NbResultLinks()-i-1 : i],_reverse);
    }
    _Node* FirstNode() const { return _link->_nodes[ _reverse ]; }
    _Node* LastNode()  const { return _link->_nodes[ !_reverse ]; }
    operator bool() const { return _link; }
    vector< TGeomID > GetNotUsedFace(const set<TGeomID>& usedIDs ) const // returns supporting FACEs
    {
      vector< TGeomID > faces;
      const B_IntersectPoint *ip0, *ip1;
      if (( ip0 = _link->_nodes[0]->_intPoint ) &&
          ( ip1 = _link->_nodes[1]->_intPoint ))
      {
        for ( size_t i = 0; i < ip0->_faceIDs.size(); ++i )
          if ( ip1->IsOnFace ( ip0->_faceIDs[i] ) &&
                !usedIDs.count( ip0->_faceIDs[i] ) )
            faces.push_back( ip0->_faceIDs[i] );
      }
      return faces;
    }
    bool HasEdgeNodes() const
    {
      return ( dynamic_cast< const E_IntersectPoint* >( _link->_nodes[0]->_intPoint ) ||
                dynamic_cast< const E_IntersectPoint* >( _link->_nodes[1]->_intPoint ));
    }
    int NbFaces() const
    {
      return !_link->_faces[0] ? 0 : 1 + bool( _link->_faces[1] );
    }
    void AddFace( _Face* f )
    {
      if ( _link->_faces[0] )
      {
        _link->_faces[1] = f;
      }
      else
      {
        _link->_faces[0] = f;
        _link->_faces[1] = 0;
      }
    }
    void RemoveFace( _Face* f )
    {
      if ( !_link->_faces[0] ) return;

      if ( _link->_faces[1] == f )
      {
        _link->_faces[1] = 0;
      }
      else if ( _link->_faces[0] == f )
      {
        _link->_faces[0] = 0;
        if ( _link->_faces[1] )
        {
          _link->_faces[0] = _link->_faces[1];
          _link->_faces[1] = 0;
        }
      }
    }
  };
  // --------------------------------------------------------------------------------
  struct _SplitIterator //! set to _hexLinks splits on one side of INTERNAL FACEs
  {
    struct _Split // data of a link split
    {
      int    _linkID;          // hex link ID
      _Node* _nodes[2];
      int    _iCheckIteration; // iteration where split is tried as Hexahedron split
      _Link* _checkedSplit;    // split set to hex links
      bool   _isUsed;          // used in a volume

      _Split( _Link & split, int iLink ):
        _linkID( iLink ), _nodes{ split._nodes[0], split._nodes[1] },
        _iCheckIteration( 0 ), _isUsed( false )
      {}
      bool IsCheckedOrUsed( bool used ) const { return used ? _isUsed : _iCheckIteration > 0; }
    };
    _Link*                _hexLinks;
    std::vector< _Split > _splits;
    int                   _iterationNb;
    size_t                _nbChecked;
    size_t                _nbUsed;
    std::vector< _Node* > _freeNodes; // nodes reached while composing a split set

    _SplitIterator( _Link* hexLinks ):
      _hexLinks( hexLinks ), _iterationNb(0), _nbChecked(0), _nbUsed(0)
    {
      _freeNodes.reserve( 12 );
      _splits.reserve( 24 );
      for ( int iL = 0; iL < 12; ++iL )
        for ( size_t iS = 0; iS < _hexLinks[ iL ]._splits.size(); ++iS )
          _splits.emplace_back( _hexLinks[ iL ]._splits[ iS ], iL );
      Next();
    }
    bool More() const { return _nbUsed < _splits.size(); }
    bool Next();
  };
  // --------------------------------------------------------------------------------
  struct _Face
  {
    SMESH_Block::TShapeID   _name;
    vector< _OrientedLink > _links;       // links on GridLine's
    vector< _Link >         _polyLinks;   // links added to close a polygonal face
    vector< _Node* >        _eIntNodes;   // nodes at intersection with EDGEs

    _Face():_name( SMESH_Block::ID_NONE )
    {}
    bool IsPolyLink( const _OrientedLink& ol )
    {
      return _polyLinks.empty() ? false :
        ( &_polyLinks[0] <= ol._link &&  ol._link <= &_polyLinks.back() );
    }
    void AddPolyLink(_Node* n0, _Node* n1, _Face* faceToFindEqual=0)
    {
      if ( faceToFindEqual && faceToFindEqual != this ) {
        for ( size_t iL = 0; iL < faceToFindEqual->_polyLinks.size(); ++iL )
          if ( faceToFindEqual->_polyLinks[iL]._nodes[0] == n1 &&
                faceToFindEqual->_polyLinks[iL]._nodes[1] == n0 )
          {
            _links.push_back
              ( _OrientedLink( & faceToFindEqual->_polyLinks[iL], /*reverse=*/true ));
            return;
          }
      }
      _Link l;
      l._nodes[0] = n0;
      l._nodes[1] = n1;
      _polyLinks.push_back( l );
      _links.push_back( _OrientedLink( &_polyLinks.back() ));
    }
  };
  // --------------------------------------------------------------------------------
  struct _volumeDef // holder of nodes of a volume mesh element
  {
    typedef void* _ptr;

    struct _nodeDef
    {
      const SMDS_MeshNode*    _node; // mesh node at hexahedron corner
      const B_IntersectPoint* _intPoint;

      _nodeDef(): _node(0), _intPoint(0) {}
      _nodeDef( _Node* n ): _node( n->_node), _intPoint( n->_intPoint ) {}
      const SMDS_MeshNode*    Node() const
      { return ( _intPoint && _intPoint->_node ) ? _intPoint->_node : _node; }
      const E_IntersectPoint* EdgeIntPnt() const
      { return static_cast< const E_IntersectPoint* >( _intPoint ); }
      _ptr Ptr() const { return Node() ? (_ptr) Node() : (_ptr) EdgeIntPnt(); }
      bool operator==(const _nodeDef& other ) const { return Ptr() == other.Ptr(); }
    };

    vector< _nodeDef >      _nodes;
    vector< int >           _quantities;
    _volumeDef*             _next; // to store several _volumeDefs in a chain
    TGeomID                 _solidID;
    double                  _size;
    const SMDS_MeshElement* _volume; // new volume
    std::vector<const SMDS_MeshElement*> _brotherVolume; // produced due to poly split 

    vector< SMESH_Block::TShapeID > _names; // name of side a polygon originates from

    _volumeDef(): _next(0), _solidID(0), _size(0), _volume(0) {}
    ~_volumeDef() { delete _next; }
    _volumeDef( _volumeDef& other ):
      _next(0), _solidID( other._solidID ), _size( other._size ), _volume( other._volume )
    { _nodes.swap( other._nodes ); _quantities.swap( other._quantities ); other._volume = 0;
      _names.swap( other._names ); }

    size_t size() const { return 1 + ( _next ? _next->size() : 0 ); } // nb _volumeDef in a chain
    _volumeDef* at(int index)
    { return index == 0 ? this : ( _next ? _next->at(index-1) : _next ); }

    void Set( _Node** nodes, int nb )
    { _nodes.assign( nodes, nodes + nb ); }

    void SetNext( _volumeDef* vd )
    { if ( _next ) { _next->SetNext( vd ); } else { _next = vd; }}

    bool IsEmpty() const { return (( _nodes.empty() ) &&
                                    ( !_next || _next->IsEmpty() )); }
    bool IsPolyhedron() const { return ( !_quantities.empty() ||
                                          ( _next && !_next->_quantities.empty() )); }


    struct _linkDef: public std::pair<_ptr,_ptr> // to join polygons in removeExcessSideDivision()
    {
      _nodeDef _node1;//, _node2;
      mutable /*const */_linkDef *_prev, *_next;
      size_t _loopIndex;

      _linkDef():_prev(0), _next(0) {}

      void init( const _nodeDef& n1, const _nodeDef& n2, size_t iLoop )
      {
        _node1     = n1; //_node2 = n2;
        _loopIndex = iLoop;
        first      = n1.Ptr();
        second     = n2.Ptr();
        if ( first > second ) std::swap( first, second );
      }
      void setNext( _linkDef* next )
      {
        _next = next;
        next->_prev = this;
      }
    };
  };

  // topology of a hexahedron
  _Node _hexNodes [8];
  _Link _hexLinks [12];
  _Face _hexQuads [6];

  // faces resulted from hexahedron intersection
  vector< _Face > _polygons;

  // intresections with EDGEs
  vector< const E_IntersectPoint* > _eIntPoints;

  // additional nodes created at intersection points
  vector< _Node > _intNodes;

  // nodes inside the hexahedron (at VERTEXes) refer to _intNodes
  vector< _Node* > _vIntNodes;

  // computed volume elements
  _volumeDef _volumeDefs;

  Grid*       _grid;
  double      _sideLength[3];
  int         _nbCornerNodes, _nbFaceIntNodes, _nbBndNodes;
  int         _origNodeInd; // index of _hexNodes[0] node within the _grid
  size_t      _i,_j,_k;
  bool        _hasTooSmall;
  int         _cellID;

public:
  Hexahedron(Grid* grid);
  int MakeElements(SMESH_MesherHelper&                      helper,
                    const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap, const int numOfThreads = 1 );
  void computeElements( const Solid* solid = 0, int solidIndex = -1 );

private:
  Hexahedron(const Hexahedron& other, size_t i, size_t j, size_t k, int cellID );
  void init( size_t i, size_t j, size_t k, const Solid* solid=0 );
  void init( size_t i );
  void setIJK( size_t i );
  /*Auxiliary methods to extract operations from monolitic compute method*/
  void defineHexahedralFaces( std::vector< _OrientedLink >& splits, std::vector<_Node*>& chainNodes, std::set< TGeomID >& concaveFaces, bool toCheckSideDivision );
  bool compute( const Solid* solid, const IsInternalFlag intFlag );
  size_t getSolids( TGeomID ids[] );
  bool isCutByInternalFace( IsInternalFlag & maxFlag );
  void addEdges(SMESH_MesherHelper&                      helper,
                vector< Hexahedron* >&                   intersectedHex,
                const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap);
  gp_Pnt findIntPoint( double u1, double proj1, double u2, double proj2,
                        double proj, BRepAdaptor_Curve& curve,
                        const gp_XYZ& axis, const gp_XYZ& origin );
  int  getEntity( const E_IntersectPoint* ip, int* facets, int& sub );
  bool addIntersection( const E_IntersectPoint* ip,
                        vector< Hexahedron* >&  hexes,
                        int ijk[], int dIJK[] );
  bool isQuadOnFace( const size_t iQuad );
  bool findChain( _Node* n1, _Node* n2, _Face& quad, vector<_Node*>& chainNodes );
  bool closePolygon( _Face* polygon, vector<_Node*>& chainNodes ) const;
  bool findChainOnEdge( const vector< _OrientedLink >& splits,
                        const _OrientedLink&           prevSplit,
                        const _OrientedLink&           avoidSplit,
                        const std::set< TGeomID > &    concaveFaces,
                        size_t &                       iS,
                        _Face&                         quad,
                        vector<_Node*>&                chn);
  int  addVolumes(SMESH_MesherHelper& helper );
  void addFaces( SMESH_MesherHelper&                       helper,
                  const vector< const SMDS_MeshElement* > & boundaryVolumes );
  void addSegments( SMESH_MesherHelper&                      helper,
                    const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap );
  void getVolumes( vector< const SMDS_MeshElement* > & volumes );
  void getBoundaryElems( vector< const SMDS_MeshElement* > & boundaryVolumes );
  void removeExcessSideDivision(const vector< Hexahedron* >& allHexa);
  void removeExcessNodes(vector< Hexahedron* >& allHexa);
  void preventVolumesOverlapping();
  TGeomID getAnyFace() const;
  void cutByExtendedInternal( std::vector< Hexahedron* >& hexes,
                              const TColStd_MapOfInteger& intEdgeIDs );
  gp_Pnt mostDistantInternalPnt( int hexIndex, const gp_Pnt& p1, const gp_Pnt& p2 );
  bool isOutPoint( _Link& link, int iP, SMESH_MesherHelper& helper, const Solid* solid ) const;
  void sortVertexNodes(vector<_Node*>& nodes, _Node* curNode, TGeomID face);
  bool isInHole() const;
  bool hasStrangeEdge() const;
  bool checkPolyhedronSize( bool isCutByInternalFace, double & volSize ) const;
  int checkPolyhedronValidity( _volumeDef* volDef, std::vector<std::vector<int>>& splitQuantities, 
                                std::vector<std::vector<const SMDS_MeshNode*>>& splitNodes );
  const SMDS_MeshElement* addPolyhedronToMesh( _volumeDef* volDef,  SMESH_MesherHelper& helper, const std::vector<const SMDS_MeshNode*>& nodes, 
                                              const std::vector<int>& quantities );
  bool addHexa ();
  bool addTetra();
  bool addPenta();
  bool addPyra ();
  bool debugDumpLink( _Link* link );
  _Node* findEqualNode( vector< _Node* >&       nodes,
                        const E_IntersectPoint* ip,
                        const double            tol2 )
  {
    for ( size_t i = 0; i < nodes.size(); ++i )
      if ( nodes[i]->EdgeIntPnt() == ip ||
            nodes[i]->Point().SquareDistance( ip->_point ) <= tol2 )
        return nodes[i];
    return 0;
  }
  bool isCorner( const _Node* node ) const { return ( node >= &_hexNodes[0] &&
                                                      node -  &_hexNodes[0] < 8 ); }
  bool hasEdgesAround( const ConcaveFace* cf ) const;
  bool isImplementEdges() const { return _grid->_edgeIntPool.nbElements(); }
  bool isOutParam(const double uvw[3]) const;

  typedef boost::container::flat_map< TGeomID, size_t > TID2Nb;
  static void insertAndIncrement( TGeomID id, TID2Nb& id2nbMap )
  {
    TID2Nb::value_type s0( id, 0 );
    TID2Nb::iterator id2nb = id2nbMap.insert( s0 ).first;
    id2nb->second++;
  }
}; // class Hexahedron

#endif
