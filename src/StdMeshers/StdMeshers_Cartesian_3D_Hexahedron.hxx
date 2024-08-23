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

namespace StdMeshers
{
namespace Cartesian3D
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
    StdMeshers::Cartesian3D::Grid*  _grid;

    CellsAroundLink( StdMeshers::Cartesian3D::Grid* grid, int iDir ):
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
      const StdMeshers::Cartesian3D::B_IntersectPoint* _intPoint;
      const _Face*            _usedInFace;
      char                    _isInternalFlags;

      _Node(const SMDS_MeshNode* n=0, const StdMeshers::Cartesian3D::B_IntersectPoint* ip=0)
        :_node(n), _boundaryCornerNode(0), _intPoint(ip), _usedInFace(0), _isInternalFlags(0) {}
      const SMDS_MeshNode*    Node() const
      { return ( _intPoint && _intPoint->_node ) ? _intPoint->_node : _node; }
      const SMDS_MeshNode*    BoundaryNode() const
      { return _node ? _node : _boundaryCornerNode; }
      const StdMeshers::Cartesian3D::E_IntersectPoint* EdgeIntPnt() const
      { return static_cast< const StdMeshers::Cartesian3D::E_IntersectPoint* >( _intPoint ); }
      const StdMeshers::Cartesian3D::F_IntersectPoint* FaceIntPnt() const
      { return static_cast< const StdMeshers::Cartesian3D::F_IntersectPoint* >( _intPoint ); }
      const std::vector< StdMeshers::Cartesian3D::TGeomID >& faces() const { return _intPoint->_faceIDs; }
      StdMeshers::Cartesian3D::TGeomID face(size_t i) const { return _intPoint->_faceIDs[ i ]; }
      void SetInternal( IsInternalFlag intFlag ) { _isInternalFlags |= intFlag; }
      bool IsCutByInternal() const { return _isInternalFlags & IS_CUT_BY_INTERNAL_FACE; }
      bool IsUsedInFace( const _Face* polygon = 0 )
      {
        return polygon ? ( _usedInFace == polygon ) : bool( _usedInFace );
      }
      StdMeshers::Cartesian3D::TGeomID IsLinked( const StdMeshers::Cartesian3D::B_IntersectPoint* other,
                                                 StdMeshers::Cartesian3D::TGeomID avoidFace=-1 ) const // returns id of a common face
      {
        return _intPoint ? _intPoint->HasCommonFace( other, avoidFace ) : 0;
      }
      bool IsOnFace( StdMeshers::Cartesian3D::TGeomID faceID ) const // returns true if faceID is found
      {
        return _intPoint ? _intPoint->IsOnFace( faceID ) : false;
      }
      size_t GetCommonFaces( const StdMeshers::Cartesian3D::B_IntersectPoint * other,
                             StdMeshers::Cartesian3D::TGeomID* common ) const
      {
        return _intPoint && other ? _intPoint->GetCommonFaces( other, common ) : 0;
      }
      gp_Pnt Point() const
      {
        if ( const SMDS_MeshNode* n = Node() )
          return SMESH_NodeXYZ( n );
        if ( const StdMeshers::Cartesian3D::E_IntersectPoint* eip =
             dynamic_cast< const StdMeshers::Cartesian3D::E_IntersectPoint* >( _intPoint ))
          return eip->_point;
        return gp_Pnt( 1e100, 0, 0 );
      }
      StdMeshers::Cartesian3D::TGeomID ShapeID() const
      {
        if ( const StdMeshers::Cartesian3D::E_IntersectPoint* eip =
             dynamic_cast< const StdMeshers::Cartesian3D::E_IntersectPoint* >( _intPoint ))
          return eip->_shapeID;
        return 0;
      }

      void Add( const StdMeshers::Cartesian3D::E_IntersectPoint* ip );
      void clear();

      friend std::ostream& operator<<(std::ostream& os, const _Node& node)
      {
        if (node._node)
        {
          os << "Node at hexahedron corner: ";
          node._node->Print(os); 
        }
        else if (node._intPoint && node._intPoint->_node)
        {
          os << "Node at intersection point: ";
          node._intPoint->_node->Print(os); // intersection point
        }
        else
          os << "mesh node is null\n";

        return os;
      }
    };

    // --------------------------------------------------------------------------------
    struct _Link // link connecting two _Node's
    {
      static const std::size_t nodesNum = 2;

      _Node* _nodes[nodesNum];
      _Face* _faces[nodesNum]; // polygons sharing a link
      std::vector< const StdMeshers::Cartesian3D::F_IntersectPoint* > _fIntPoints; // GridLine intersections with FACEs
      std::vector< _Node* > _fIntNodes;   // _Node's at _fIntPoints
      std::vector< _Link >  _splits;
      _Link(): _nodes{ 0, 0 }, _faces{ 0, 0 } {}

      void clear();

      friend std::ostream& operator<<(std::ostream& os, const _Link& link)
      {
        os << "Link:\n";

        for (std::size_t i = 0; i < nodesNum; ++i)
        {
          if (link._nodes[i])
            os << *link._nodes[i];
          else
            os << "link node with index " << i << " is null\n";
        }

        os << "_fIntPoints: " << link._fIntPoints.size() << '\n';
        os << "_fIntNodes: " << link._fIntNodes.size() << '\n';
        os << "_splits: " << link._splits.size() << '\n';

        return os;
      }
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

      // returns supporting FACEs
      std::vector< StdMeshers::Cartesian3D::TGeomID > GetNotUsedFaces
      (const std::set<StdMeshers::Cartesian3D::TGeomID>& usedIDs ) const
      {
        std::vector< StdMeshers::Cartesian3D::TGeomID > faces;
        const StdMeshers::Cartesian3D::B_IntersectPoint *ip0, *ip1;
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
        return ( dynamic_cast< const StdMeshers::Cartesian3D::E_IntersectPoint* >( _link->_nodes[0]->_intPoint ) ||
                 dynamic_cast< const StdMeshers::Cartesian3D::E_IntersectPoint* >( _link->_nodes[1]->_intPoint ));
      }
      int NbFaces() const
      {
        return !_link->_faces[0] ? 0 : 1 + bool( _link->_faces[1] );
      }
      void AddFace( _Face* f )
      {
        if ( _link->_faces[0] ) {
          _link->_faces[1] = f;
        }
        else {
          _link->_faces[0] = f;
          _link->_faces[1] = 0;
        }
      }
      void RemoveFace( const _Face* f )
      {
        if ( !_link->_faces[0] ) return;

        if ( _link->_faces[1] == f ) {
          _link->_faces[1] = 0;
        }
        else if ( _link->_faces[0] == f ) {
          _link->_faces[0] = 0;
          if ( _link->_faces[1] ) {
            _link->_faces[0] = _link->_faces[1];
            _link->_faces[1] = 0;
          }
        }
      }

      friend std::ostream& operator<<(std::ostream& os, const _OrientedLink& link)
      {
        if (link._link)
          os << "Oriented " << *link._link;
        else
          os << "Oriented link is null\n";

        return os;
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
      std::vector< _OrientedLink > _links;       // links on GridLine's
      std::vector< _Link >         _polyLinks;   // links added to close a polygonal face
      std::vector< _Node* >        _eIntNodes;   // nodes at intersection with EDGEs

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

      friend std::ostream& operator<<(std::ostream& os, const _Face& face)
      {
        os << "Face " << face._name << '\n';

        os << "Links on GridLines: \n";
        for (const auto& link : face._links)
        {
          os << link;
        }

        os << "Links added to close a polygonal face: \n";
        for (const auto& link : face._polyLinks)
        {
          os << link;
        }

        os << "Nodes at intersection with EDGEs: \n";
        for (const auto node : face._eIntNodes)
        {
          if (node)
          {
            os << *node;
          }
        }

        return os;
      }
    };

    // --------------------------------------------------------------------------------
    struct _volumeDef // holder of nodes of a volume mesh element
    {
      typedef void* _ptr;

      struct _nodeDef
      {
        const SMDS_MeshNode*    _node; // mesh node at hexahedron corner
        const StdMeshers::Cartesian3D::B_IntersectPoint* _intPoint;

        _nodeDef(): _node(0), _intPoint(0) {}
        _nodeDef( _Node* n ): _node( n->_node), _intPoint( n->_intPoint ) {}
        const SMDS_MeshNode*    Node() const
        { return ( _intPoint && _intPoint->_node ) ? _intPoint->_node : _node; }
        const StdMeshers::Cartesian3D::E_IntersectPoint* EdgeIntPnt() const
        { return static_cast< const StdMeshers::Cartesian3D::E_IntersectPoint* >( _intPoint ); }
        _ptr Ptr() const { return Node() ? (_ptr) Node() : (_ptr) EdgeIntPnt(); }
        bool operator==(const _nodeDef& other ) const { return Ptr() == other.Ptr(); }

        friend std::ostream& operator<<(std::ostream& os, const _nodeDef& node)
        {
          if (node._node)
          {
            os << "Node at hexahedron corner: ";
            node._node->Print(os); 
          }
          else if (node._intPoint && node._intPoint->_node)
          {
            os << "Node at intersection point: ";
            node._intPoint->_node->Print(os); // intersection point
          }
          else
            os << "mesh node is null\n";

          return os;
        }
      };

      std::vector< _nodeDef >              _nodes;
      std::vector< int >                   _quantities;
      _volumeDef*                          _next; // to store several _volumeDefs in a chain
      StdMeshers::Cartesian3D::TGeomID     _solidID;
      double                               _size;
      const SMDS_MeshElement*              _volume; // new volume
      std::vector<const SMDS_MeshElement*> _brotherVolume; // produced due to poly split
      std::vector< SMESH_Block::TShapeID > _names; // name of side a polygon originates from

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

      std::vector<std::set<std::pair<int, int>>> getPolygonsEdges() const;
      std::vector<std::set<std::pair<int, int>>> findOpenEdges() const;
      int getStartNodeIndex(const int polygon) const;
      std::map<int, std::vector<int>> findOverlappingPolygons() const;
      bool divideOverlappingPolygons();
      bool fixOpenEdgesPolygons();
      bool capOpenEdgesPolygons(const std::vector<std::set<std::pair<int, int>>>& edgesByPolygon);
      bool removeOpenEdgesPolygons(const std::vector<std::set<std::pair<int, int>>>& edgesByPolygon);

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

        friend std::ostream& operator<<(std::ostream& os, const _linkDef& link)
        {
          os << "Link def:\n";

          os << link._node1;
          if (link.first)
            os << "first: " << link.first;

          if (link.second)
            os << "second: " << link.second;

          os << "_loopIndex: " << link._loopIndex << '\n';

          return os;
        }
      };
    };

    // topology of a hexahedron
    static const std::size_t HEX_NODES_NUM = 8;
    static const std::size_t HEX_LINKS_NUM = 12;
    static const std::size_t HEX_QUADS_NUM = 6;
    _Node _hexNodes [HEX_NODES_NUM];
    _Link _hexLinks [HEX_LINKS_NUM];
    _Face _hexQuads [HEX_QUADS_NUM];

    // faces resulted from hexahedron intersection
    std::vector< _Face > _polygons;

    // intresections with EDGEs
    std::vector< const StdMeshers::Cartesian3D::E_IntersectPoint* > _eIntPoints;

    // additional nodes created at intersection points
    std::vector< _Node > _intNodes;

    // nodes inside the hexahedron (at VERTEXes) refer to _intNodes
    std::vector< _Node* > _vIntNodes;

    // computed volume elements
    _volumeDef _volumeDefs;

    StdMeshers::Cartesian3D::Grid*       _grid;
    double      _sideLength[3];
    int         _nbCornerNodes, _nbFaceIntNodes, _nbBndNodes;
    int         _origNodeInd; // index of _hexNodes[0] node within the _grid
    size_t      _i,_j,_k;
    bool        _hasTooSmall;
    int         _cellID;

  public:
    Hexahedron(StdMeshers::Cartesian3D::Grid* grid);
    int MakeElements(SMESH_MesherHelper&                      helper,
                     const TEdge2faceIDsMap& edge2faceIDsMap,
                     const int numOfThreads = 1 );
    void computeElements( const Solid* solid = 0, int solidIndex = -1 );

  private:
    Hexahedron(const Hexahedron& other, size_t i, size_t j, size_t k, int cellID );
    void init( size_t i, size_t j, size_t k, const Solid* solid=0 );
    void init( size_t i );
    void clearNodesLinkedToNull(const Solid* solid, SMESH_MesherHelper& helper);
    bool isSplittedLink(const Solid* solid, SMESH_MesherHelper& helper, const Hexahedron::_Link& linkIn) const;
    void setIJK( size_t i );
    /*Auxiliary methods to extract operations from monolitic compute method*/
    void defineHexahedralFaces( const Solid* solid, const IsInternalFlag intFlag );
    bool compute( const Solid* solid, const IsInternalFlag intFlag );
    size_t getSolids( StdMeshers::Cartesian3D::TGeomID ids[] );
    bool isCutByInternalFace( IsInternalFlag & maxFlag );
    void addEdges(SMESH_MesherHelper&         helper,
                  std::vector< Hexahedron* >& intersectedHex,
                  const TEdge2faceIDsMap&     edge2faceIDsMap);
    gp_Pnt findIntPoint( double u1, double proj1, double u2, double proj2,
                         double proj, BRepAdaptor_Curve& curve,
                         const gp_XYZ& axis, const gp_XYZ& origin );
    int  getEntity( const StdMeshers::Cartesian3D::E_IntersectPoint* ip, int* facets, int& sub );
    bool addIntersection( const StdMeshers::Cartesian3D::E_IntersectPoint* ip,
                          std::vector< Hexahedron* >&  hexes,
                          int ijk[], int dIJK[] );
    bool isQuadOnFace( const size_t iQuad );
    bool findChain( _Node* n1, _Node* n2, _Face& quad, std::vector<_Node*>& chainNodes );
    bool closePolygon( _Face* polygon, std::vector<_Node*>& chainNodes ) const;
    bool findChainOnEdge( const std::vector< _OrientedLink >& splits,
                          const _OrientedLink&                prevSplit,
                          const _OrientedLink&                avoidSplit,
                          const std::set< StdMeshers::Cartesian3D::TGeomID > & concaveFaces,
                          size_t &                            iS,
                          _Face&                              quad,
                          std::vector<_Node*>&                chn);
    typedef std::pair< StdMeshers::Cartesian3D::TGeomID, int > TFaceOfLink; // (face, link)
    static TFaceOfLink findStartLink(const std::vector< _OrientedLink* >& freeLinks,
                                     std::set< StdMeshers::Cartesian3D::TGeomID >& usedFaceIDs);
    size_t findCoplanarPolygon
           (const _Face& thePolygon,
            const size_t nbQuadPolygons,
            std::vector< _OrientedLink* >& freeLinks,
            int& nbFreeLinks,
            const E_IntersectPoint& ipTmp,
            std::set< StdMeshers::Cartesian3D::TGeomID >& usedFaceIDs,
            std::map< StdMeshers::Cartesian3D::TGeomID, std::vector< const B_IntersectPoint* > >& tmpAddedFace,
            const StdMeshers::Cartesian3D::TGeomID& curFace);
    int  addVolumes( SMESH_MesherHelper& helper );
    void addFaces( SMESH_MesherHelper& helper,
                   const std::vector< const SMDS_MeshElement* > & boundaryVolumes );
    void addSegments( SMESH_MesherHelper&     helper,
                      const TEdge2faceIDsMap& edge2faceIDsMap );
    void getVolumes( std::vector< const SMDS_MeshElement* > & volumes );
    void getBoundaryElems( std::vector< const SMDS_MeshElement* > & boundaryVolumes );
    void removeExcessSideDivision(const std::vector< Hexahedron* >& allHexa);
    void removeExcessNodes(std::vector< Hexahedron* >& allHexa);
    void preventVolumesOverlapping();
    StdMeshers::Cartesian3D::TGeomID getAnyFace() const;
    void cutByExtendedInternal( std::vector< Hexahedron* >& hexes,
                                const TColStd_MapOfInteger& intEdgeIDs );
    gp_Pnt mostDistantInternalPnt( int hexIndex, const gp_Pnt& p1, const gp_Pnt& p2 );
    bool isOutPoint( _Link& link, int iP, SMESH_MesherHelper& helper, const Solid* solid ) const;
    void sortVertexNodes(std::vector<_Node*>& nodes,
                         _Node* curNode,
                         StdMeshers::Cartesian3D::TGeomID face);
    bool isInHole() const;
    bool hasStrangeEdge() const;
    bool checkPolyhedronSize( bool isCutByInternalFace, double & volSize ) const;
    int checkPolyhedronValidity( _volumeDef* volDef, std::vector<std::vector<int>>& splitQuantities,
                                 std::vector<std::vector<const SMDS_MeshNode*>>& splitNodes );
    const SMDS_MeshElement* addPolyhedronToMesh( _volumeDef* volDef,
                                                 SMESH_MesherHelper& helper,
                                                 const std::vector<const SMDS_MeshNode*>& nodes,
                                                 const std::vector<int>& quantities );
    bool addHexa ();
    bool addTetra();
    bool addPenta();
    bool addPyra ();
    bool debugDumpLink( _Link* link );
    _Node* findEqualNode( std::vector< _Node* >&       nodes,
                          const StdMeshers::Cartesian3D::E_IntersectPoint* ip,
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

    typedef boost::container::flat_map< StdMeshers::Cartesian3D::TGeomID, size_t > TID2Nb;
    static void insertAndIncrement( StdMeshers::Cartesian3D::TGeomID id, TID2Nb& id2nbMap )
    {
      TID2Nb::value_type s0( id, 0 );
      TID2Nb::iterator id2nb = id2nbMap.insert( s0 ).first;
      id2nb->second++;
    }
  }; // class Hexahedron
} // end namespace Cartesian3D
} // end namespace StdMeshers

#endif
