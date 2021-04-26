// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshers_Cartesian_3D.cxx
//  Module : SMESH
//
#include "StdMeshers_Cartesian_3D.hxx"
#include "StdMeshers_CartesianParameters3D.hxx"

#include "ObjectPool.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Comment.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_subMeshEventListener.hxx"
#include "StdMeshers_FaceSide.hxx"

#include <utilities.h>
#include <Utils_ExceptHandlers.hxx>

#include <GEOMUtils.hxx>

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepTools.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_B3d.hxx>
#include <Bnd_Box.hxx>
#include <ElSLib.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomLib.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <IntAna_IntLinTorus.hxx>
#include <IntAna_Quadric.hxx>
#include <IntCurveSurface_TransitionOnCurve.hxx>
#include <IntCurvesFace_Intersector.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_TShape.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>

#include <limits>

#include <boost/container/flat_map.hpp>

//#undef WITH_TBB
#ifdef WITH_TBB

#ifdef WIN32
// See https://docs.microsoft.com/en-gb/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
// Windows 10 = 0x0A00  
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#endif

#include <tbb/parallel_for.h>
//#include <tbb/enumerable_thread_specific.h>
#endif

using namespace std;
using namespace SMESH;

#ifdef _DEBUG_
//#define _MY_DEBUG_
#endif

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================

StdMeshers_Cartesian_3D::StdMeshers_Cartesian_3D(int hypId, SMESH_Gen * gen)
  :SMESH_3D_Algo(hypId, gen)
{
  _name = "Cartesian_3D";
  _shapeType = (1 << TopAbs_SOLID);       // 1 bit /shape type
  _compatibleHypothesis.push_back("CartesianParameters3D");

  _onlyUnaryInput = false;          // to mesh all SOLIDs at once
  _requireDiscreteBoundary = false; // 2D mesh not needed
  _supportSubmeshes = false;        // do not use any existing mesh
}

//=============================================================================
/*!
 * Check presence of a hypothesis
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::CheckHypothesis (SMESH_Mesh&          aMesh,
                                               const TopoDS_Shape&  aShape,
                                               Hypothesis_Status&   aStatus)
{
  aStatus = SMESH_Hypothesis::HYP_MISSING;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  if ( h == hyps.end())
  {
    return false;
  }

  for ( ; h != hyps.end(); ++h )
  {
    if (( _hyp = dynamic_cast<const StdMeshers_CartesianParameters3D*>( *h )))
    {
      aStatus = _hyp->IsDefined() ? HYP_OK : HYP_BAD_PARAMETER;
      break;
    }
  }

  return aStatus == HYP_OK;
}

namespace
{
  typedef int TGeomID; // IDs of sub-shapes

  //=============================================================================
  // Definitions of internal utils
  // --------------------------------------------------------------------------
  enum Transition {
    Trans_TANGENT = IntCurveSurface_Tangent,
    Trans_IN      = IntCurveSurface_In,
    Trans_OUT     = IntCurveSurface_Out,
    Trans_APEX,
    Trans_INTERNAL // for INTERNAL FACE
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Container of IDs of SOLID sub-shapes
   */
  class Solid // sole SOLID contains all sub-shapes
  {
    TGeomID _id; // SOLID id
    bool    _hasInternalFaces;
  public:
    virtual ~Solid() {}
    virtual bool Contains( TGeomID /*subID*/ ) const { return true; }
    virtual bool ContainsAny( const vector< TGeomID>& /*subIDs*/ ) const { return true; }
    virtual TopAbs_Orientation Orientation( const TopoDS_Shape& s ) const { return s.Orientation(); }
    virtual bool IsOutsideOriented( TGeomID /*faceID*/ ) const { return true; }
    void SetID( TGeomID id ) { _id = id; }
    TGeomID ID() const { return _id; }
    void SetHasInternalFaces( bool has ) { _hasInternalFaces = has; }
    bool HasInternalFaces() const { return _hasInternalFaces; }
  };
  // --------------------------------------------------------------------------
  class OneOfSolids : public Solid
  {
    TColStd_MapOfInteger _subIDs;
    TopTools_MapOfShape  _faces; // keep FACE orientation
    TColStd_MapOfInteger _outFaceIDs; // FACEs of shape_to_mesh oriented outside the SOLID
  public:
    void Init( const TopoDS_Shape& solid,
               TopAbs_ShapeEnum    subType,
               const SMESHDS_Mesh* mesh );
    virtual bool Contains( TGeomID i ) const { return i == ID() || _subIDs.Contains( i ); }
    virtual bool ContainsAny( const vector< TGeomID>& subIDs ) const
    {
      for ( size_t i = 0; i < subIDs.size(); ++i ) if ( Contains( subIDs[ i ])) return true;
      return false;
    }
    virtual TopAbs_Orientation Orientation( const TopoDS_Shape& face ) const
    {
      const TopoDS_Shape& sInMap = const_cast< OneOfSolids* >(this)->_faces.Added( face );
      return sInMap.Orientation();
    }
    virtual bool IsOutsideOriented( TGeomID faceID ) const
    {
      return faceID == 0 || _outFaceIDs.Contains( faceID );
    }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Geom data
   */
  struct Geometry
  {
    TopoDS_Shape                _mainShape;
    vector< vector< TGeomID > > _solidIDsByShapeID;// V/E/F ID -> SOLID IDs
    Solid                       _soleSolid;
    map< TGeomID, OneOfSolids > _solidByID;
    TColStd_MapOfInteger        _boundaryFaces; // FACEs on boundary of mesh->ShapeToMesh()
    TColStd_MapOfInteger        _strangeEdges; // EDGEs shared by strange FACEs
    TGeomID                     _extIntFaceID; // pseudo FACE - extension of INTERNAL FACE

    Controls::ElementsOnShape _edgeClassifier;
    Controls::ElementsOnShape _vertexClassifier;

    bool IsOneSolid() const { return _solidByID.size() < 2; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Common data of any intersection between a Grid and a shape
   */
  struct B_IntersectPoint
  {
    mutable const SMDS_MeshNode* _node;
    mutable vector< TGeomID >    _faceIDs;

    B_IntersectPoint(): _node(NULL) {}
    void Add( const vector< TGeomID >& fIDs, const SMDS_MeshNode* n=0 ) const;
    int HasCommonFace( const B_IntersectPoint * other, int avoidFace=-1 ) const;
    bool IsOnFace( int faceID ) const;
    virtual ~B_IntersectPoint() {}
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Data of intersection between a GridLine and a TopoDS_Face
   */
  struct F_IntersectPoint : public B_IntersectPoint
  {
    double             _paramOnLine;
    double             _u, _v;
    mutable Transition _transition;
    mutable size_t     _indexOnLine;

    bool operator< ( const F_IntersectPoint& o ) const { return _paramOnLine < o._paramOnLine; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Data of intersection between GridPlanes and a TopoDS_EDGE
   */
  struct E_IntersectPoint : public B_IntersectPoint
  {
    gp_Pnt  _point;
    double  _uvw[3];
    TGeomID _shapeID; // ID of EDGE or VERTEX
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief A line of the grid and its intersections with 2D geometry
   */
  struct GridLine
  {
    gp_Lin _line;
    double _length; // line length
    multiset< F_IntersectPoint > _intPoints;

    void RemoveExcessIntPoints( const double tol );
    TGeomID GetSolidIDBefore( multiset< F_IntersectPoint >::iterator ip,
                              const TGeomID                          prevID,
                              const Geometry&                        geom);
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Planes of the grid used to find intersections of an EDGE with a hexahedron
   */
  struct GridPlanes
  {
    gp_XYZ           _zNorm;
    vector< gp_XYZ > _origins; // origin points of all planes in one direction
    vector< double > _zProjs;  // projections of origins to _zNorm
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Iterator on the parallel grid lines of one direction
   */
  struct LineIndexer
  {
    size_t _size  [3];
    size_t _curInd[3];
    size_t _iVar1, _iVar2, _iConst;
    string _name1, _name2, _nameConst;
    LineIndexer() {}
    LineIndexer( size_t sz1, size_t sz2, size_t sz3,
                 size_t iv1, size_t iv2, size_t iConst,
                 const string& nv1, const string& nv2, const string& nConst )
    {
      _size[0] = sz1; _size[1] = sz2; _size[2] = sz3;
      _curInd[0] = _curInd[1] = _curInd[2] = 0;
      _iVar1 = iv1; _iVar2 = iv2; _iConst = iConst;
      _name1 = nv1; _name2 = nv2; _nameConst = nConst;
    }

    size_t I() const { return _curInd[0]; }
    size_t J() const { return _curInd[1]; }
    size_t K() const { return _curInd[2]; }
    void SetIJK( size_t i, size_t j, size_t k )
    {
      _curInd[0] = i; _curInd[1] = j; _curInd[2] = k;
    }
    void operator++()
    {
      if ( ++_curInd[_iVar1] == _size[_iVar1] )
        _curInd[_iVar1] = 0, ++_curInd[_iVar2];
    }
    bool More() const { return _curInd[_iVar2] < _size[_iVar2]; }
    size_t LineIndex   () const { return _curInd[_iVar1] + _curInd[_iVar2]* _size[_iVar1]; }
    size_t LineIndex10 () const { return (_curInd[_iVar1] + 1 ) + _curInd[_iVar2]* _size[_iVar1]; }
    size_t LineIndex01 () const { return _curInd[_iVar1] + (_curInd[_iVar2] + 1 )* _size[_iVar1]; }
    size_t LineIndex11 () const { return (_curInd[_iVar1] + 1 ) + (_curInd[_iVar2] + 1 )* _size[_iVar1]; }
    void SetIndexOnLine (size_t i)  { _curInd[ _iConst ] = i; }
    size_t NbLines() const { return _size[_iVar1] * _size[_iVar2]; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Container of GridLine's
   */
  struct Grid
  {
    vector< double >   _coords[3]; // coordinates of grid nodes
    gp_XYZ             _axes  [3]; // axis directions
    vector< GridLine > _lines [3]; //    in 3 directions
    double             _tol, _minCellSize;
    gp_XYZ             _origin;
    gp_Mat             _invB; // inverted basis of _axes

    // index shift within _nodes of nodes of a cell from the 1st node
    int                _nodeShift[8];

    vector< const SMDS_MeshNode* >    _nodes; // mesh nodes at grid nodes
    vector< const F_IntersectPoint* > _gridIntP; // grid node intersection with geometry
    ObjectPool< E_IntersectPoint >    _edgeIntPool; // intersections with EDGEs
    ObjectPool< F_IntersectPoint >    _extIntPool; // intersections with extended INTERNAL FACEs
    //list< E_IntersectPoint >          _edgeIntP; // intersections with EDGEs

    Geometry                          _geometry;
    bool                              _toAddEdges;
    bool                              _toCreateFaces;
    bool                              _toConsiderInternalFaces;
    bool                              _toUseThresholdForInternalFaces;
    double                            _sizeThreshold;

    SMESH_MesherHelper*               _helper;

    size_t CellIndex( size_t i, size_t j, size_t k ) const
    {
      return i + j*(_coords[0].size()-1) + k*(_coords[0].size()-1)*(_coords[1].size()-1);
    }
    size_t NodeIndex( size_t i, size_t j, size_t k ) const
    {
      return i + j*_coords[0].size() + k*_coords[0].size()*_coords[1].size();
    }
    size_t NodeIndexDX() const { return 1; }
    size_t NodeIndexDY() const { return _coords[0].size(); }
    size_t NodeIndexDZ() const { return _coords[0].size() * _coords[1].size(); }

    LineIndexer GetLineIndexer(size_t iDir) const;

    E_IntersectPoint* Add( const E_IntersectPoint& ip )
    {
      E_IntersectPoint* eip = _edgeIntPool.getNew();
      *eip = ip;
      return eip;
    }
    void Remove( E_IntersectPoint* eip ) { _edgeIntPool.destroy( eip ); }

    TGeomID ShapeID( const TopoDS_Shape& s ) const;
    const TopoDS_Shape& Shape( TGeomID id ) const;
    TopAbs_ShapeEnum ShapeType( TGeomID id ) const { return Shape(id).ShapeType(); }
    void InitGeometry( const TopoDS_Shape& theShape );
    void InitClassifier( const TopoDS_Shape&        mainShape,
                         TopAbs_ShapeEnum           shapeType,
                         Controls::ElementsOnShape& classifier );
    void GetEdgesToImplement( map< TGeomID, vector< TGeomID > > & edge2faceMap,
                              const TopoDS_Shape&                 shape,
                              const vector< TopoDS_Shape >&       faces );
    void SetSolidFather( const TopoDS_Shape& s, const TopoDS_Shape& theShapeToMesh );
    bool IsShared( TGeomID faceID ) const;
    bool IsAnyShared( const std::vector< TGeomID >& faceIDs ) const;
    bool IsInternal( TGeomID faceID ) const {
      return ( faceID == PseudoIntExtFaceID() ||
               Shape( faceID ).Orientation() == TopAbs_INTERNAL ); }
    bool IsSolid( TGeomID shapeID ) const {
      if ( _geometry.IsOneSolid() ) return _geometry._soleSolid.ID() == shapeID;
      else                          return _geometry._solidByID.count( shapeID ); }
    bool IsStrangeEdge( TGeomID id ) const { return _geometry._strangeEdges.Contains( id ); }
    TGeomID PseudoIntExtFaceID() const { return _geometry._extIntFaceID; }
    Solid* GetSolid( TGeomID solidID = 0 );
    Solid* GetOneOfSolids( TGeomID solidID );
    const vector< TGeomID > & GetSolidIDs( TGeomID subShapeID ) const;
    bool IsCorrectTransition( TGeomID faceID, const Solid* solid );
    bool IsBoundaryFace( TGeomID face ) const { return _geometry._boundaryFaces.Contains( face ); }
    void SetOnShape( const SMDS_MeshNode* n, const F_IntersectPoint& ip, bool unset=false );
    bool IsToCheckNodePos() const { return !_toAddEdges && _toCreateFaces; }
    bool IsToRemoveExcessEntities() const { return !_toAddEdges; }

    void SetCoordinates(const vector<double>& xCoords,
                        const vector<double>& yCoords,
                        const vector<double>& zCoords,
                        const double*         axesDirs,
                        const Bnd_Box&        bndBox );
    void ComputeUVW(const gp_XYZ& p, double uvw[3]);
    void ComputeNodes(SMESH_MesherHelper& helper);
  };
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
  // --------------------------------------------------------------------------
  /*!
   * \brief Intersector of TopoDS_Face with all GridLine's
   */
  struct FaceGridIntersector
  {
    TopoDS_Face _face;
    TGeomID     _faceID;
    Grid*       _grid;
    Bnd_Box     _bndBox;
    IntCurvesFace_Intersector* _surfaceInt;
    vector< std::pair< GridLine*, F_IntersectPoint > > _intersections;

    FaceGridIntersector(): _grid(0), _surfaceInt(0) {}
    void Intersect();

    void StoreIntersections()
    {
      for ( size_t i = 0; i < _intersections.size(); ++i )
      {
        multiset< F_IntersectPoint >::iterator ip =
          _intersections[i].first->_intPoints.insert( _intersections[i].second );
        ip->_faceIDs.reserve( 1 );
        ip->_faceIDs.push_back( _faceID );
      }
    }
    const Bnd_Box& GetFaceBndBox()
    {
      GetCurveFaceIntersector();
      return _bndBox;
    }
    IntCurvesFace_Intersector* GetCurveFaceIntersector()
    {
      if ( !_surfaceInt )
      {
        _surfaceInt = new IntCurvesFace_Intersector( _face, Precision::PConfusion() );
        _bndBox     = _surfaceInt->Bounding();
        if ( _bndBox.IsVoid() )
          BRepBndLib::Add (_face, _bndBox);
      }
      return _surfaceInt;
    }
    bool IsThreadSafe(set< const Standard_Transient* >& noSafeTShapes) const;
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Intersector of a surface with a GridLine
   */
  struct FaceLineIntersector
  {
    double      _tol;
    double      _u, _v, _w; // params on the face and the line
    Transition  _transition; // transition at intersection (see IntCurveSurface.cdl)
    Transition  _transIn, _transOut; // IN and OUT transitions depending of face orientation

    gp_Pln      _plane;
    gp_Cylinder _cylinder;
    gp_Cone     _cone;
    gp_Sphere   _sphere;
    gp_Torus    _torus;
    IntCurvesFace_Intersector* _surfaceInt;

    vector< F_IntersectPoint > _intPoints;

    void IntersectWithPlane   (const GridLine& gridLine);
    void IntersectWithCylinder(const GridLine& gridLine);
    void IntersectWithCone    (const GridLine& gridLine);
    void IntersectWithSphere  (const GridLine& gridLine);
    void IntersectWithTorus   (const GridLine& gridLine);
    void IntersectWithSurface (const GridLine& gridLine);

    bool UVIsOnFace() const;
    void addIntPoint(const bool toClassify=true);
    bool isParamOnLineOK( const double linLength )
    {
      return -_tol < _w && _w < linLength + _tol;
    }
    FaceLineIntersector():_surfaceInt(0) {}
    ~FaceLineIntersector() { if (_surfaceInt ) delete _surfaceInt; _surfaceInt = 0; }
  };
  // --------------------------------------------------------------------------
  /*!
   * \brief Class representing topology of the hexahedron and creating a mesh
   *        volume basing on analysis of hexahedron intersection with geometry
   */
  class Hexahedron
  {
    // --------------------------------------------------------------------------------
    struct _Face;
    struct _Link;
    enum IsInternalFlag { IS_NOT_INTERNAL, IS_INTERNAL, IS_CUT_BY_INTERNAL_FACE };
    // --------------------------------------------------------------------------------
    struct _Node //!< node either at a hexahedron corner or at intersection
    {
      const SMDS_MeshNode*    _node; // mesh node at hexahedron corner
      const B_IntersectPoint* _intPoint;
      const _Face*            _usedInFace;
      char                    _isInternalFlags;

      _Node(const SMDS_MeshNode* n=0, const B_IntersectPoint* ip=0)
        :_node(n), _intPoint(ip), _usedInFace(0), _isInternalFlags(0) {} 
      const SMDS_MeshNode*    Node() const
      { return ( _intPoint && _intPoint->_node ) ? _intPoint->_node : _node; }
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
      void Add( const E_IntersectPoint* ip )
      {
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
      int NbResultLinks() const { return _link->_splits.size(); }
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
      vector< int >      _quantities;
      _volumeDef*             _next; // to store several _volumeDefs in a chain
      TGeomID                 _solidID;
      const SMDS_MeshElement* _volume; // new volume

      vector< SMESH_Block::TShapeID > _names; // name of side a polygon originates from

      _volumeDef(): _next(0), _solidID(0), _volume(0) {}
      ~_volumeDef() { delete _next; }
      _volumeDef( _volumeDef& other ):
        _next(0), _solidID( other._solidID ), _volume( other._volume )
      { _nodes.swap( other._nodes ); _quantities.swap( other._quantities ); other._volume = 0;
        _names.swap( other._names ); }

      size_t size() const { return 1 + ( _next ? _next->size() : 0 ); }
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

#ifdef _DEBUG_
    int         _cellID;
#endif

  public:
    Hexahedron(Grid* grid);
    int MakeElements(SMESH_MesherHelper&                      helper,
                     const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap);
    void computeElements( const Solid* solid = 0, int solidIndex = -1 );

  private:
    Hexahedron(const Hexahedron& other, size_t i, size_t j, size_t k, int cellID );
    void init( size_t i, size_t j, size_t k, const Solid* solid=0 );
    void init( size_t i );
    void setIJK( size_t i );
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
    bool findChain( _Node* n1, _Node* n2, _Face& quad, vector<_Node*>& chainNodes );
    bool closePolygon( _Face* polygon, vector<_Node*>& chainNodes ) const;
    bool findChainOnEdge( const vector< _OrientedLink >& splits,
                          const _OrientedLink&           prevSplit,
                          const _OrientedLink&           avoidSplit,
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
    bool checkPolyhedronSize( bool isCutByInternalFace ) const;
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
  // --------------------------------------------------------------------------
  /*!
   * \brief Structure intersecting certain nb of faces with GridLine's in one thread
   */
  struct ParallelIntersector
  {
    vector< FaceGridIntersector >& _faceVec;
    ParallelIntersector( vector< FaceGridIntersector >& faceVec): _faceVec(faceVec){}
    void operator() ( const tbb::blocked_range<size_t>& r ) const
    {
      for ( size_t i = r.begin(); i != r.end(); ++i )
        _faceVec[i].Intersect();
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
  //=============================================================================
  /*
   * Remove coincident intersection points
   */
  void GridLine::RemoveExcessIntPoints( const double tol )
  {
    if ( _intPoints.size() < 2 ) return;

    set< Transition > tranSet;
    multiset< F_IntersectPoint >::iterator ip1, ip2 = _intPoints.begin();
    while ( ip2 != _intPoints.end() )
    {
      tranSet.clear();
      ip1 = ip2++;
      while ( ip2 != _intPoints.end() && ip2->_paramOnLine - ip1->_paramOnLine <= tol )
      {
        tranSet.insert( ip1->_transition );
        tranSet.insert( ip2->_transition );
        ip2->Add( ip1->_faceIDs );
        _intPoints.erase( ip1 );
        ip1 = ip2++;
      }
      if ( tranSet.size() > 1 ) // points with different transition coincide
      {
        bool isIN  = tranSet.count( Trans_IN );
        bool isOUT = tranSet.count( Trans_OUT );
        if ( isIN && isOUT )
          (*ip1)._transition = Trans_TANGENT;
        else
          (*ip1)._transition = isIN ? Trans_IN : Trans_OUT;
      }
    }
  }
  //================================================================================
  /*
   * Return ID of SOLID for nodes before the given intersection point
   */
  TGeomID GridLine::GetSolidIDBefore( multiset< F_IntersectPoint >::iterator ip,
                                      const TGeomID                          prevID,
                                      const Geometry&                        geom )
  {
    if ( ip == _intPoints.begin() )
      return 0;

    if ( geom.IsOneSolid() )
    {
      bool isOut = true;
      switch ( ip->_transition ) {
      case Trans_IN:      isOut = true;            break;
      case Trans_OUT:     isOut = false;           break;
      case Trans_TANGENT: isOut = ( prevID != 0 ); break;
      case Trans_APEX:
      {
        // singularity point (apex of a cone)
        multiset< F_IntersectPoint >::iterator ipBef = ip, ipAft = ++ip;
        if ( ipAft == _intPoints.end() )
          isOut = false;
        else
        {
          --ipBef;
          if ( ipBef->_transition != ipAft->_transition )
            isOut = ( ipBef->_transition == Trans_OUT );
          else
            isOut = ( ipBef->_transition != Trans_OUT );
        }
        break;
      }
      case Trans_INTERNAL: isOut = false;
      default:;
      }
      return isOut ? 0 : geom._soleSolid.ID();
    }

    const vector< TGeomID >& solids = geom._solidIDsByShapeID[ ip->_faceIDs[ 0 ]];

    --ip;
    if ( ip->_transition == Trans_INTERNAL )
      return prevID;

    const vector< TGeomID >& solidsBef = geom._solidIDsByShapeID[ ip->_faceIDs[ 0 ]];

    if ( ip->_transition == Trans_IN ||
         ip->_transition == Trans_OUT )
    {
      if ( solidsBef.size() == 1 )
        return ( solidsBef[0] == prevID ) ? 0 : solidsBef[0];

      return solidsBef[ solidsBef[0] == prevID ];
    }

    if ( solidsBef.size() == 1 )
      return solidsBef[0];

    for ( size_t i = 0; i < solids.size(); ++i )
    {
      vector< TGeomID >::const_iterator it =
        std::find( solidsBef.begin(), solidsBef.end(), solids[i] );
      if ( it != solidsBef.end() )
        return solids[i];
    }
    return 0;
  }
  //================================================================================
  /*
   * Adds face IDs
   */
  void B_IntersectPoint::Add( const vector< TGeomID >& fIDs,
                              const SMDS_MeshNode*     n) const
  {
    if ( _faceIDs.empty() )
      _faceIDs = fIDs;
    else
      for ( size_t i = 0; i < fIDs.size(); ++i )
      {
        vector< TGeomID >::iterator it =
          std::find( _faceIDs.begin(), _faceIDs.end(), fIDs[i] );
        if ( it == _faceIDs.end() )
          _faceIDs.push_back( fIDs[i] );
      }
    if ( !_node )
      _node = n;
  }
  //================================================================================
  /*
   * Returns index of a common face if any, else zero
   */
  int B_IntersectPoint::HasCommonFace( const B_IntersectPoint * other, int avoidFace ) const
  {
    if ( other )
      for ( size_t i = 0; i < other->_faceIDs.size(); ++i )
        if ( avoidFace != other->_faceIDs[i] &&
             IsOnFace   ( other->_faceIDs[i] ))
          return other->_faceIDs[i];
    return 0;
  }
  //================================================================================
  /*
   * Returns \c true if \a faceID in in this->_faceIDs
   */
  bool B_IntersectPoint::IsOnFace( int faceID ) const // returns true if faceID is found
  {
    vector< TGeomID >::const_iterator it =
      std::find( _faceIDs.begin(), _faceIDs.end(), faceID );
    return ( it != _faceIDs.end() );
  }
  //================================================================================
  /*
   * OneOfSolids initialization
   */
  void OneOfSolids::Init( const TopoDS_Shape& solid,
                          TopAbs_ShapeEnum    subType,
                          const SMESHDS_Mesh* mesh )
  {
    SetID( mesh->ShapeToIndex( solid ));

    if ( subType == TopAbs_FACE )
      SetHasInternalFaces( false );

    for ( TopExp_Explorer sub( solid, subType ); sub.More(); sub.Next() )
    {
      _subIDs.Add( mesh->ShapeToIndex( sub.Current() ));
      if ( subType == TopAbs_FACE )
      {
        _faces.Add( sub.Current() );
        if ( sub.Current().Orientation() == TopAbs_INTERNAL )
          SetHasInternalFaces( true );

        TGeomID faceID = mesh->ShapeToIndex( sub.Current() );
        if ( sub.Current().Orientation() == TopAbs_INTERNAL ||
             sub.Current().Orientation() == mesh->IndexToShape( faceID ).Orientation() )
          _outFaceIDs.Add( faceID );
      }
    }
  }
  //================================================================================
  /*
   * Return an iterator on GridLine's in a given direction
   */
  LineIndexer Grid::GetLineIndexer(size_t iDir) const
  {
    const size_t indices[] = { 1,2,0, 0,2,1, 0,1,2 };
    const string s      [] = { "X", "Y", "Z" };
    LineIndexer li( _coords[0].size(),  _coords[1].size(),    _coords[2].size(),
                    indices[iDir*3],    indices[iDir*3+1],    indices[iDir*3+2],
                    s[indices[iDir*3]], s[indices[iDir*3+1]], s[indices[iDir*3+2]]);
    return li;
  }
  //=============================================================================
  /*
   * Creates GridLine's of the grid
   */
  void Grid::SetCoordinates(const vector<double>& xCoords,
                            const vector<double>& yCoords,
                            const vector<double>& zCoords,
                            const double*         axesDirs,
                            const Bnd_Box&        shapeBox)
  {
    _coords[0] = xCoords;
    _coords[1] = yCoords;
    _coords[2] = zCoords;

    _axes[0].SetCoord( axesDirs[0],
                       axesDirs[1],
                       axesDirs[2]);
    _axes[1].SetCoord( axesDirs[3],
                       axesDirs[4],
                       axesDirs[5]);
    _axes[2].SetCoord( axesDirs[6],
                       axesDirs[7],
                       axesDirs[8]);
    _axes[0].Normalize();
    _axes[1].Normalize();
    _axes[2].Normalize();

    _invB.SetCols( _axes[0], _axes[1], _axes[2] );
    _invB.Invert();

    // compute tolerance
    _minCellSize = Precision::Infinite();
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      for ( size_t i = 1; i < _coords[ iDir ].size(); ++i )
      {
        double cellLen = _coords[ iDir ][ i ] - _coords[ iDir ][ i-1 ];
        if ( cellLen < _minCellSize )
          _minCellSize = cellLen;
      }
    }
    if ( _minCellSize < Precision::Confusion() )
      throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                SMESH_Comment("Too small cell size: ") << _minCellSize );
    _tol = _minCellSize / 1000.;

    // attune grid extremities to shape bounding box

    double sP[6]; // aXmin, aYmin, aZmin, aXmax, aYmax, aZmax
    shapeBox.Get(sP[0],sP[1],sP[2],sP[3],sP[4],sP[5]);
    double* cP[6] = { &_coords[0].front(), &_coords[1].front(), &_coords[2].front(),
                      &_coords[0].back(),  &_coords[1].back(),  &_coords[2].back() };
    for ( int i = 0; i < 6; ++i )
      if ( fabs( sP[i] - *cP[i] ) < _tol )
        *cP[i] = sP[i];// + _tol/1000. * ( i < 3 ? +1 : -1 );

    for ( int iDir = 0; iDir < 3; ++iDir )
    {
      if ( _coords[iDir][0] - sP[iDir] > _tol )
      {
        _minCellSize = Min( _minCellSize, _coords[iDir][0] - sP[iDir] );
        _coords[iDir].insert( _coords[iDir].begin(), sP[iDir] + _tol/1000.);
      }
      if ( sP[iDir+3] - _coords[iDir].back() > _tol  )
      {
        _minCellSize = Min( _minCellSize, sP[iDir+3] - _coords[iDir].back() );
        _coords[iDir].push_back( sP[iDir+3] - _tol/1000.);
      }
    }
    _tol = _minCellSize / 1000.;

    _origin = ( _coords[0][0] * _axes[0] +
                _coords[1][0] * _axes[1] +
                _coords[2][0] * _axes[2] );

    // create lines
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      LineIndexer li = GetLineIndexer( iDir );
      _lines[iDir].resize( li.NbLines() );
      double len = _coords[ iDir ].back() - _coords[iDir].front();
      for ( ; li.More(); ++li )
      {
        GridLine& gl = _lines[iDir][ li.LineIndex() ];
        gl._line.SetLocation( _coords[0][li.I()] * _axes[0] +
                              _coords[1][li.J()] * _axes[1] +
                              _coords[2][li.K()] * _axes[2] );
        gl._line.SetDirection( _axes[ iDir ]);
        gl._length = len;
      }
    }
  }
  //================================================================================
  /*
   * Return local ID of shape
   */
  TGeomID Grid::ShapeID( const TopoDS_Shape& s ) const
  {
    return _helper->GetMeshDS()->ShapeToIndex( s );
  }
  //================================================================================
  /*
   * Return a shape by its local ID
   */
  const TopoDS_Shape& Grid::Shape( TGeomID id ) const
  {
    return _helper->GetMeshDS()->IndexToShape( id );
  }
  //================================================================================
  /*
   * Initialize _geometry
   */
  void Grid::InitGeometry( const TopoDS_Shape& theShapeToMesh )
  {
    SMESH_Mesh* mesh = _helper->GetMesh();

    _geometry._mainShape = theShapeToMesh;
    _geometry._extIntFaceID = mesh->GetMeshDS()->MaxShapeIndex() * 100;
    _geometry._soleSolid.SetID( 0 );
    _geometry._soleSolid.SetHasInternalFaces( false );

    InitClassifier( theShapeToMesh, TopAbs_VERTEX, _geometry._vertexClassifier );
    InitClassifier( theShapeToMesh, TopAbs_EDGE  , _geometry._edgeClassifier );

    TopExp_Explorer solidExp( theShapeToMesh, TopAbs_SOLID );

    bool isSeveralSolids = false;
    if ( _toConsiderInternalFaces ) // check nb SOLIDs
    {
      solidExp.Next();
      isSeveralSolids = solidExp.More();
      _toConsiderInternalFaces = isSeveralSolids;
      solidExp.ReInit();

      if ( !isSeveralSolids ) // look for an internal FACE
      {
        TopExp_Explorer fExp( theShapeToMesh, TopAbs_FACE );
        for ( ; fExp.More() &&  !_toConsiderInternalFaces; fExp.Next() )
          _toConsiderInternalFaces = ( fExp.Current().Orientation() == TopAbs_INTERNAL );

        _geometry._soleSolid.SetHasInternalFaces( _toConsiderInternalFaces );
        _geometry._soleSolid.SetID( ShapeID( solidExp.Current() ));
      }
      else // fill Geometry::_solidByID
      {
        for ( ; solidExp.More(); solidExp.Next() )
        {
          OneOfSolids & solid = _geometry._solidByID[ ShapeID( solidExp.Current() )];
          solid.Init( solidExp.Current(), TopAbs_FACE,   mesh->GetMeshDS() );
          solid.Init( solidExp.Current(), TopAbs_EDGE,   mesh->GetMeshDS() );
          solid.Init( solidExp.Current(), TopAbs_VERTEX, mesh->GetMeshDS() );
        }
      }
    }
    else
    {
      _geometry._soleSolid.SetID( ShapeID( solidExp.Current() ));
    }

    if ( !_toCreateFaces )
    {
      int nbSolidsGlobal = _helper->Count( mesh->GetShapeToMesh(), TopAbs_SOLID, false );
      int nbSolidsLocal  = _helper->Count( theShapeToMesh,         TopAbs_SOLID, false );
      _toCreateFaces = ( nbSolidsLocal < nbSolidsGlobal );
    }

    TopTools_IndexedMapOfShape faces;
    if ( _toCreateFaces || isSeveralSolids )
      TopExp::MapShapes( theShapeToMesh, TopAbs_FACE, faces );

    // find boundary FACEs on boundary of mesh->ShapeToMesh()
    if ( _toCreateFaces )
      for ( int i = 1; i <= faces.Size(); ++i )
        if ( faces(i).Orientation() != TopAbs_INTERNAL &&
             _helper->NbAncestors( faces(i), *mesh, TopAbs_SOLID ) == 1 )
        {
          _geometry._boundaryFaces.Add( ShapeID( faces(i) ));
        }

    if ( isSeveralSolids )
      for ( int i = 1; i <= faces.Size(); ++i )
      {
        SetSolidFather( faces(i), theShapeToMesh );
        for ( TopExp_Explorer eExp( faces(i), TopAbs_EDGE ); eExp.More(); eExp.Next() )
        {
          const TopoDS_Edge& edge = TopoDS::Edge( eExp.Current() );
          SetSolidFather( edge, theShapeToMesh );
          SetSolidFather( _helper->IthVertex( 0, edge ), theShapeToMesh );
          SetSolidFather( _helper->IthVertex( 1, edge ), theShapeToMesh );
        }
      }
    return;
  }
  //================================================================================
  /*
   * Store ID of SOLID as father of its child shape ID
   */
  void Grid::SetSolidFather( const TopoDS_Shape& s, const TopoDS_Shape& theShapeToMesh )
  {
    if ( _geometry._solidIDsByShapeID.empty() )
      _geometry._solidIDsByShapeID.resize( _helper->GetMeshDS()->MaxShapeIndex() + 1 );

    vector< TGeomID > & solidIDs = _geometry._solidIDsByShapeID[ ShapeID( s )];
    if ( !solidIDs.empty() )
      return;
    solidIDs.reserve(2);
    PShapeIteratorPtr solidIt = _helper->GetAncestors( s,
                                                       *_helper->GetMesh(),
                                                       TopAbs_SOLID,
                                                       & theShapeToMesh );
    while ( const TopoDS_Shape* solid = solidIt->next() )
      solidIDs.push_back( ShapeID( *solid ));
  }
  //================================================================================
  /*
   * Return IDs of solids given sub-shape belongs to
   */
  const vector< TGeomID > & Grid::GetSolidIDs( TGeomID subShapeID ) const
  {
    return _geometry._solidIDsByShapeID[ subShapeID ];
  }
  //================================================================================
  /*
   * Check if a sub-shape belongs to several SOLIDs
   */
  bool Grid::IsShared( TGeomID shapeID ) const
  {
    return !_geometry.IsOneSolid() && ( _geometry._solidIDsByShapeID[ shapeID ].size() > 1 );
  }
  //================================================================================
  /*
   * Check if any of FACEs belongs to several SOLIDs
   */
  bool Grid::IsAnyShared( const std::vector< TGeomID >& faceIDs ) const
  {
    for ( size_t i = 0; i < faceIDs.size(); ++i )
      if ( IsShared( faceIDs[ i ]))
        return true;
    return false;
  }
  //================================================================================
  /*
   * Return Solid by ID
   */
  Solid* Grid::GetSolid( TGeomID solidID )
  {
    if ( !solidID || _geometry.IsOneSolid() || _geometry._solidByID.empty() )
      return & _geometry._soleSolid;

    return & _geometry._solidByID[ solidID ];
  }
  //================================================================================
  /*
   * Return OneOfSolids by ID
   */
  Solid* Grid::GetOneOfSolids( TGeomID solidID )
  {
    map< TGeomID, OneOfSolids >::iterator is2s = _geometry._solidByID.find( solidID );
    if ( is2s != _geometry._solidByID.end() )
      return & is2s->second;

    return & _geometry._soleSolid;
  }
  //================================================================================
  /*
   * Check if transition on given FACE is correct for a given SOLID
   */
  bool Grid::IsCorrectTransition( TGeomID faceID, const Solid* solid )
  {
    if ( _geometry.IsOneSolid() )
      return true;

    const vector< TGeomID >& solidIDs = _geometry._solidIDsByShapeID[ faceID ];
    return solidIDs[0] == solid->ID();
  }

  //================================================================================
  /*
   * Assign to geometry a node at FACE intersection
   */
  void Grid::SetOnShape( const SMDS_MeshNode* n, const F_IntersectPoint& ip, bool unset )
  {
    TopoDS_Shape s;
    SMESHDS_Mesh* mesh = _helper->GetMeshDS();
    if ( ip._faceIDs.size() == 1 )
    {
      mesh->SetNodeOnFace( n, ip._faceIDs[0], ip._u, ip._v );
    }
    else if ( _geometry._vertexClassifier.IsSatisfy( n, &s ))
    {
      if ( unset ) mesh->UnSetNodeOnShape( n );
      mesh->SetNodeOnVertex( n, TopoDS::Vertex( s ));
    }
    else if ( _geometry._edgeClassifier.IsSatisfy( n, &s ))
    {
      if ( unset ) mesh->UnSetNodeOnShape( n );
      mesh->SetNodeOnEdge( n, TopoDS::Edge( s ));
    }
    else if ( ip._faceIDs.size() > 0 )
    {
      mesh->SetNodeOnFace( n, ip._faceIDs[0], ip._u, ip._v );
    }
    else if ( !unset && _geometry.IsOneSolid() )
    {
      mesh->SetNodeInVolume( n, _geometry._soleSolid.ID() );
    }
  }
  //================================================================================
  /*
   * Initialize a classifier
   */
  void Grid::InitClassifier( const TopoDS_Shape&        mainShape,
                             TopAbs_ShapeEnum           shapeType,
                             Controls::ElementsOnShape& classifier )
  {
    TopTools_IndexedMapOfShape shapes;
    TopExp::MapShapes( mainShape, shapeType, shapes );

    TopoDS_Compound compound; BRep_Builder builder;
    builder.MakeCompound( compound );
    for ( int i = 1; i <= shapes.Size(); ++i )
      builder.Add( compound, shapes(i) );

    classifier.SetMesh( _helper->GetMeshDS() );
    //classifier.SetTolerance( _tol ); // _tol is not initialised
    classifier.SetShape( compound, SMDSAbs_Node );
  }

  //================================================================================
  /*
   * Return EDGEs with FACEs to implement into the mesh
   */
  void Grid::GetEdgesToImplement( map< TGeomID, vector< TGeomID > > & edge2faceIDsMap,
                                  const TopoDS_Shape&                 shape,
                                  const vector< TopoDS_Shape >&       faces )
  {
    // check if there are strange EDGEs
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes( _helper->GetMesh()->GetShapeToMesh(), TopAbs_FACE, faceMap );
    int nbFacesGlobal = faceMap.Size();
    faceMap.Clear( false );
    TopExp::MapShapes( shape, TopAbs_FACE, faceMap );
    int nbFacesLocal  = faceMap.Size();
    bool hasStrangeEdges = ( nbFacesGlobal > nbFacesLocal );
    if ( !_toAddEdges && !hasStrangeEdges )
      return; // no FACEs in contact with those meshed by other algo

    for ( size_t i = 0; i < faces.size(); ++i )
    {
      _helper->SetSubShape( faces[i] );
      for ( TopExp_Explorer eExp( faces[i], TopAbs_EDGE ); eExp.More(); eExp.Next() )
      {
        const TopoDS_Edge& edge = TopoDS::Edge( eExp.Current() );
        if ( hasStrangeEdges )
        {
          bool hasStrangeFace = false;
          PShapeIteratorPtr faceIt = _helper->GetAncestors( edge, *_helper->GetMesh(), TopAbs_FACE);
          while ( const TopoDS_Shape* face = faceIt->next() )
            if (( hasStrangeFace = !faceMap.Contains( *face )))
              break;
          if ( !hasStrangeFace && !_toAddEdges )
            continue;
          _geometry._strangeEdges.Add( ShapeID( edge ));
          _geometry._strangeEdges.Add( ShapeID( _helper->IthVertex( 0, edge )));
          _geometry._strangeEdges.Add( ShapeID( _helper->IthVertex( 1, edge )));
        }
        if ( !SMESH_Algo::isDegenerated( edge ) &&
             !_helper->IsRealSeam( edge ))
        {
          edge2faceIDsMap[ ShapeID( edge )].push_back( ShapeID( faces[i] ));
        }
      }
    }
    return;
  }

  //================================================================================
  /*
   * Computes coordinates of a point in the grid CS
   */
  void Grid::ComputeUVW(const gp_XYZ& P, double UVW[3])
  {
    gp_XYZ p = P * _invB;
    p.Coord( UVW[0], UVW[1], UVW[2] );
  }
  //================================================================================
  /*
   * Creates all nodes
   */
  void Grid::ComputeNodes(SMESH_MesherHelper& helper)
  {
    // state of each node of the grid relative to the geometry
    const size_t nbGridNodes = _coords[0].size() * _coords[1].size() * _coords[2].size();
    const TGeomID undefID = 1e+9;
    vector< TGeomID > shapeIDVec( nbGridNodes, undefID );
    _nodes.resize( nbGridNodes, 0 );
    _gridIntP.resize( nbGridNodes, NULL );

    SMESHDS_Mesh* mesh = helper.GetMeshDS();

    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      LineIndexer li = GetLineIndexer( iDir );

      // find out a shift of node index while walking along a GridLine in this direction
      li.SetIndexOnLine( 0 );
      size_t nIndex0 = NodeIndex( li.I(), li.J(), li.K() );
      li.SetIndexOnLine( 1 );
      const size_t nShift = NodeIndex( li.I(), li.J(), li.K() ) - nIndex0;
      
      const vector<double> & coords = _coords[ iDir ];
      for ( ; li.More(); ++li ) // loop on lines in iDir
      {
        li.SetIndexOnLine( 0 );
        nIndex0 = NodeIndex( li.I(), li.J(), li.K() );

        GridLine& line = _lines[ iDir ][ li.LineIndex() ];
        const gp_XYZ lineLoc = line._line.Location().XYZ();
        const gp_XYZ lineDir = line._line.Direction().XYZ();

        line.RemoveExcessIntPoints( _tol );
        multiset< F_IntersectPoint >&     intPnts = line._intPoints;
        multiset< F_IntersectPoint >::iterator ip = intPnts.begin();

        // Create mesh nodes at intersections with geometry
        // and set OUT state of nodes between intersections

        TGeomID solidID = 0;
        const double* nodeCoord = & coords[0];
        const double* coord0    = nodeCoord;
        const double* coordEnd  = coord0 + coords.size();
        double nodeParam = 0;
        for ( ; ip != intPnts.end(); ++ip )
        {
          solidID = line.GetSolidIDBefore( ip, solidID, _geometry );

          // set OUT state or just skip IN nodes before ip
          if ( nodeParam < ip->_paramOnLine - _tol )
          {
            while ( nodeParam < ip->_paramOnLine - _tol )
            {
              TGeomID & nodeShapeID = shapeIDVec[ nIndex0 + nShift * ( nodeCoord-coord0 ) ];
              nodeShapeID = Min( solidID, nodeShapeID );
              if ( ++nodeCoord <  coordEnd )
                nodeParam = *nodeCoord - *coord0;
              else
                break;
            }
            if ( nodeCoord == coordEnd ) break;
          }
          // create a mesh node on a GridLine at ip if it does not coincide with a grid node
          if ( nodeParam > ip->_paramOnLine + _tol )
          {
            gp_XYZ xyz = lineLoc + ip->_paramOnLine * lineDir;
            ip->_node = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
            ip->_indexOnLine = nodeCoord-coord0-1;
            SetOnShape( ip->_node, *ip );
          }
          // create a mesh node at ip coincident with a grid node
          else
          {
            int nodeIndex = nIndex0 + nShift * ( nodeCoord-coord0 );
            if ( !_nodes[ nodeIndex ] )
            {
              gp_XYZ xyz = lineLoc + nodeParam * lineDir;
              _nodes   [ nodeIndex ] = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
              //_gridIntP[ nodeIndex ] = & * ip;
              //SetOnShape( _nodes[ nodeIndex ], *ip );
            }
            if ( _gridIntP[ nodeIndex ] )
              _gridIntP[ nodeIndex ]->Add( ip->_faceIDs );
            else
              _gridIntP[ nodeIndex ] = & * ip;
            // ip->_node        = _nodes[ nodeIndex ]; -- to differ from ip on links
            ip->_indexOnLine = nodeCoord-coord0;
            if ( ++nodeCoord < coordEnd )
              nodeParam = *nodeCoord - *coord0;
          }
        }
        // set OUT state to nodes after the last ip
        for ( ; nodeCoord < coordEnd; ++nodeCoord )
          shapeIDVec[ nIndex0 + nShift * ( nodeCoord-coord0 ) ] = 0;
      }
    }

    // Create mesh nodes at !OUT nodes of the grid

    for ( size_t z = 0; z < _coords[2].size(); ++z )
      for ( size_t y = 0; y < _coords[1].size(); ++y )
        for ( size_t x = 0; x < _coords[0].size(); ++x )
        {
          size_t nodeIndex = NodeIndex( x, y, z );
          if ( !_nodes[ nodeIndex ] &&
               0 < shapeIDVec[ nodeIndex ] && shapeIDVec[ nodeIndex ] < undefID )
          {
            gp_XYZ xyz = ( _coords[0][x] * _axes[0] +
                           _coords[1][y] * _axes[1] +
                           _coords[2][z] * _axes[2] );
            _nodes[ nodeIndex ] = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
            mesh->SetNodeInVolume( _nodes[ nodeIndex ], shapeIDVec[ nodeIndex ]);
          }
          else if ( _nodes[ nodeIndex ] && _gridIntP[ nodeIndex ] /*&&
                    !_nodes[ nodeIndex]->GetShapeID()*/ )
          {
            SetOnShape( _nodes[ nodeIndex ], *_gridIntP[ nodeIndex ]);
          }
        }

#ifdef _MY_DEBUG_
    // check validity of transitions
    const char* trName[] = { "TANGENT", "IN", "OUT", "APEX" };
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      LineIndexer li = GetLineIndexer( iDir );
      for ( ; li.More(); ++li )
      {
        multiset< F_IntersectPoint >& intPnts = _lines[ iDir ][ li.LineIndex() ]._intPoints;
        if ( intPnts.empty() ) continue;
        if ( intPnts.size() == 1 )
        {
          if ( intPnts.begin()->_transition != Trans_TANGENT &&
               intPnts.begin()->_transition != Trans_APEX )
          throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                    SMESH_Comment("Wrong SOLE transition of GridLine (")
                                    << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                    << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.begin()->_transition] );
        }
        else
        {
          if ( intPnts.begin()->_transition == Trans_OUT )
            throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                      SMESH_Comment("Wrong START transition of GridLine (")
                                      << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                      << ") along " << li._nameConst
                                      << ": " << trName[ intPnts.begin()->_transition ]);
          if ( intPnts.rbegin()->_transition == Trans_IN )
            throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                      SMESH_Comment("Wrong END transition of GridLine (")
                                      << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                      << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.rbegin()->_transition ]);
        }
      }
    }
#endif
  }

  //=============================================================================
  /*
   * Intersects TopoDS_Face with all GridLine's
   */
  void FaceGridIntersector::Intersect()
  {
    FaceLineIntersector intersector;
    intersector._surfaceInt = GetCurveFaceIntersector();
    intersector._tol        = _grid->_tol;
    intersector._transOut   = _face.Orientation() == TopAbs_REVERSED ? Trans_IN : Trans_OUT;
    intersector._transIn    = _face.Orientation() == TopAbs_REVERSED ? Trans_OUT : Trans_IN;

    typedef void (FaceLineIntersector::* PIntFun )(const GridLine& gridLine);
    PIntFun interFunction;

    bool isDirect = true;
    BRepAdaptor_Surface surf( _face );
    switch ( surf.GetType() ) {
    case GeomAbs_Plane:
      intersector._plane = surf.Plane();
      interFunction = &FaceLineIntersector::IntersectWithPlane;
      isDirect = intersector._plane.Direct();
      break;
    case GeomAbs_Cylinder:
      intersector._cylinder = surf.Cylinder();
      interFunction = &FaceLineIntersector::IntersectWithCylinder;
      isDirect = intersector._cylinder.Direct();
      break;
    case GeomAbs_Cone:
      intersector._cone = surf.Cone();
      interFunction = &FaceLineIntersector::IntersectWithCone;
      //isDirect = intersector._cone.Direct();
      break;
    case GeomAbs_Sphere:
      intersector._sphere = surf.Sphere();
      interFunction = &FaceLineIntersector::IntersectWithSphere;
      isDirect = intersector._sphere.Direct();
      break;
    case GeomAbs_Torus:
      intersector._torus = surf.Torus();
      interFunction = &FaceLineIntersector::IntersectWithTorus;
      //isDirect = intersector._torus.Direct();
      break;
    default:
      interFunction = &FaceLineIntersector::IntersectWithSurface;
    }
    if ( !isDirect )
      std::swap( intersector._transOut, intersector._transIn );

    _intersections.clear();
    for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
    {
      if ( surf.GetType() == GeomAbs_Plane )
      {
        // check if all lines in this direction are parallel to a plane
        if ( intersector._plane.Axis().IsNormal( _grid->_lines[iDir][0]._line.Position(),
                                                 Precision::Angular()))
          continue;
        // find out a transition, that is the same for all lines of a direction
        gp_Dir plnNorm = intersector._plane.Axis().Direction();
        gp_Dir lineDir = _grid->_lines[iDir][0]._line.Direction();
        intersector._transition =
          ( plnNorm * lineDir < 0 ) ? intersector._transIn : intersector._transOut;
      }
      if ( surf.GetType() == GeomAbs_Cylinder )
      {
        // check if all lines in this direction are parallel to a cylinder
        if ( intersector._cylinder.Axis().IsParallel( _grid->_lines[iDir][0]._line.Position(),
                                                      Precision::Angular()))
          continue;
      }

      // intersect the grid lines with the face
      for ( size_t iL = 0; iL < _grid->_lines[iDir].size(); ++iL )
      {
        GridLine& gridLine = _grid->_lines[iDir][iL];
        if ( _bndBox.IsOut( gridLine._line )) continue;

        intersector._intPoints.clear();
        (intersector.*interFunction)( gridLine ); // <- intersection with gridLine
        for ( size_t i = 0; i < intersector._intPoints.size(); ++i )
          _intersections.push_back( make_pair( &gridLine, intersector._intPoints[i] ));
      }
    }

    if ( _face.Orientation() == TopAbs_INTERNAL )
    {
      for ( size_t i = 0; i < _intersections.size(); ++i )
        if ( _intersections[i].second._transition == Trans_IN ||
             _intersections[i].second._transition == Trans_OUT )
        {
          _intersections[i].second._transition = Trans_INTERNAL;
        }
    }
    return;
  }
  //================================================================================
  /*
   * Return true if (_u,_v) is on the face
   */
  bool FaceLineIntersector::UVIsOnFace() const
  {
    TopAbs_State state = _surfaceInt->ClassifyUVPoint(gp_Pnt2d( _u,_v ));
    return ( state == TopAbs_IN || state == TopAbs_ON );
  }
  //================================================================================
  /*
   * Store an intersection if it is IN or ON the face
   */
  void FaceLineIntersector::addIntPoint(const bool toClassify)
  {
    if ( !toClassify || UVIsOnFace() )
    {
      F_IntersectPoint p;
      p._paramOnLine = _w;
      p._u           = _u;
      p._v           = _v;
      p._transition  = _transition;
      _intPoints.push_back( p );
    }
  }
  //================================================================================
  /*
   * Intersect a line with a plane
   */
  void FaceLineIntersector::IntersectWithPlane(const GridLine& gridLine)
  {
    IntAna_IntConicQuad linPlane( gridLine._line, _plane, Precision::Angular());
    _w = linPlane.ParamOnConic(1);
    if ( isParamOnLineOK( gridLine._length ))
    {
      ElSLib::Parameters(_plane, linPlane.Point(1) ,_u,_v);
      addIntPoint();
    }
  }
  //================================================================================
  /*
   * Intersect a line with a cylinder
   */
  void FaceLineIntersector::IntersectWithCylinder(const GridLine& gridLine)
  {
    IntAna_IntConicQuad linCylinder( gridLine._line, _cylinder );
    if ( linCylinder.IsDone() && linCylinder.NbPoints() > 0 )
    {
      _w = linCylinder.ParamOnConic(1);
      if ( linCylinder.NbPoints() == 1 )
        _transition = Trans_TANGENT;
      else
        _transition = _w < linCylinder.ParamOnConic(2) ? _transIn : _transOut;
      if ( isParamOnLineOK( gridLine._length ))
      {
        ElSLib::Parameters(_cylinder, linCylinder.Point(1) ,_u,_v);
        addIntPoint();
      }
      if ( linCylinder.NbPoints() > 1 )
      {
        _w = linCylinder.ParamOnConic(2);
        if ( isParamOnLineOK( gridLine._length ))
        {
          ElSLib::Parameters(_cylinder, linCylinder.Point(2) ,_u,_v);
          _transition = ( _transition == Trans_OUT ) ? Trans_IN : Trans_OUT;
          addIntPoint();
        }
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a cone
   */
  void FaceLineIntersector::IntersectWithCone (const GridLine& gridLine)
  {
    IntAna_IntConicQuad linCone(gridLine._line,_cone);
    if ( !linCone.IsDone() ) return;
    gp_Pnt P;
    gp_Vec du, dv, norm;
    for ( int i = 1; i <= linCone.NbPoints(); ++i )
    {
      _w = linCone.ParamOnConic( i );
      if ( !isParamOnLineOK( gridLine._length )) continue;
      ElSLib::Parameters(_cone, linCone.Point(i) ,_u,_v);
      if ( UVIsOnFace() )
      {
        ElSLib::D1( _u, _v, _cone, P, du, dv );
        norm = du ^ dv;
        double normSize2 = norm.SquareMagnitude();
        if ( normSize2 > Precision::Angular() * Precision::Angular() )
        {
          double cos = norm.XYZ() * gridLine._line.Direction().XYZ();
          cos /= sqrt( normSize2 );
          if ( cos < -Precision::Angular() )
            _transition = _transIn;
          else if ( cos > Precision::Angular() )
            _transition = _transOut;
          else
            _transition = Trans_TANGENT;
        }
        else
        {
          _transition = Trans_APEX;
        }
        addIntPoint( /*toClassify=*/false);
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a sphere
   */
  void FaceLineIntersector::IntersectWithSphere  (const GridLine& gridLine)
  {
    IntAna_IntConicQuad linSphere(gridLine._line,_sphere);
    if ( linSphere.IsDone() && linSphere.NbPoints() > 0 )
    {
      _w = linSphere.ParamOnConic(1);
      if ( linSphere.NbPoints() == 1 )
        _transition = Trans_TANGENT;
      else
        _transition = _w < linSphere.ParamOnConic(2) ? _transIn : _transOut;
      if ( isParamOnLineOK( gridLine._length ))
      {
        ElSLib::Parameters(_sphere, linSphere.Point(1) ,_u,_v);
        addIntPoint();
      }
      if ( linSphere.NbPoints() > 1 )
      {
        _w = linSphere.ParamOnConic(2);
        if ( isParamOnLineOK( gridLine._length ))
        {
          ElSLib::Parameters(_sphere, linSphere.Point(2) ,_u,_v);
          _transition = ( _transition == Trans_OUT ) ? Trans_IN : Trans_OUT;
          addIntPoint();
        }
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a torus
   */
  void FaceLineIntersector::IntersectWithTorus   (const GridLine& gridLine)
  {
    IntAna_IntLinTorus linTorus(gridLine._line,_torus);
    if ( !linTorus.IsDone()) return;
    gp_Pnt P;
    gp_Vec du, dv, norm;
    for ( int i = 1; i <= linTorus.NbPoints(); ++i )
    {
      _w = linTorus.ParamOnLine( i );
      if ( !isParamOnLineOK( gridLine._length )) continue;
      linTorus.ParamOnTorus( i, _u,_v );
      if ( UVIsOnFace() )
      {
        ElSLib::D1( _u, _v, _torus, P, du, dv );
        norm = du ^ dv;
        double normSize = norm.Magnitude();
        double cos = norm.XYZ() * gridLine._line.Direction().XYZ();
        cos /= normSize;
        if ( cos < -Precision::Angular() )
          _transition = _transIn;
        else if ( cos > Precision::Angular() )
          _transition = _transOut;
        else
          _transition = Trans_TANGENT;
        addIntPoint( /*toClassify=*/false);
      }
    }
  }
  //================================================================================
  /*
   * Intersect a line with a non-analytical surface
   */
  void FaceLineIntersector::IntersectWithSurface (const GridLine& gridLine)
  {
    _surfaceInt->Perform( gridLine._line, 0.0, gridLine._length );
    if ( !_surfaceInt->IsDone() ) return;
    for ( int i = 1; i <= _surfaceInt->NbPnt(); ++i )
    {
      _transition = Transition( _surfaceInt->Transition( i ) );
      _w = _surfaceInt->WParameter( i );
      addIntPoint(/*toClassify=*/false);
    }
  }
  //================================================================================
  /*
   * check if its face can be safely intersected in a thread
   */
  bool FaceGridIntersector::IsThreadSafe(set< const Standard_Transient* >& noSafeTShapes) const
  {
    bool isSafe = true;

    // check surface
    TopLoc_Location loc;
    Handle(Geom_Surface) surf = BRep_Tool::Surface( _face, loc );
    Handle(Geom_RectangularTrimmedSurface) ts =
      Handle(Geom_RectangularTrimmedSurface)::DownCast( surf );
    while( !ts.IsNull() ) {
      surf = ts->BasisSurface();
      ts = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);
    }
    if ( surf->IsKind( STANDARD_TYPE(Geom_BSplineSurface )) ||
         surf->IsKind( STANDARD_TYPE(Geom_BezierSurface )))
      if ( !noSafeTShapes.insert( _face.TShape().get() ).second )
        isSafe = false;

    double f, l;
    TopExp_Explorer exp( _face, TopAbs_EDGE );
    for ( ; exp.More(); exp.Next() )
    {
      bool edgeIsSafe = true;
      const TopoDS_Edge& e = TopoDS::Edge( exp.Current() );
      // check 3d curve
      {
        Handle(Geom_Curve) c = BRep_Tool::Curve( e, loc, f, l);
        if ( !c.IsNull() )
        {
          Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c);
          while( !tc.IsNull() ) {
            c = tc->BasisCurve();
            tc = Handle(Geom_TrimmedCurve)::DownCast(c);
          }
          if ( c->IsKind( STANDARD_TYPE(Geom_BSplineCurve )) ||
               c->IsKind( STANDARD_TYPE(Geom_BezierCurve )))
            edgeIsSafe = false;
        }
      }
      // check 2d curve
      if ( edgeIsSafe )
      {
        Handle(Geom2d_Curve) c2 = BRep_Tool::CurveOnSurface( e, surf, loc, f, l);
        if ( !c2.IsNull() )
        {
          Handle(Geom2d_TrimmedCurve) tc = Handle(Geom2d_TrimmedCurve)::DownCast(c2);
          while( !tc.IsNull() ) {
            c2 = tc->BasisCurve();
            tc = Handle(Geom2d_TrimmedCurve)::DownCast(c2);
          }
          if ( c2->IsKind( STANDARD_TYPE(Geom2d_BSplineCurve )) ||
               c2->IsKind( STANDARD_TYPE(Geom2d_BezierCurve )))
            edgeIsSafe = false;
        }
      }
      if ( !edgeIsSafe && !noSafeTShapes.insert( e.TShape().get() ).second )
        isSafe = false;
    }
    return isSafe;
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
#ifdef _DEBUG_
    _cellID = cellID;
#else
    (void)cellID; // unused in release mode
#endif
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

      _hexNodes[iN]._node     = _grid->_nodes   [ _origNodeInd + _grid->_nodeShift[iN] ];
      _hexNodes[iN]._intPoint = _grid->_gridIntP[ _origNodeInd + _grid->_nodeShift[iN] ];

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
      _intNodes.reserve( 3 * _nbBndNodes + _nbFaceIntNodes + _eIntPoints.size() );

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
      const double tol2 = _grid->_tol * _grid->_tol;
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
        TGeomID solidIDs[20];
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
  bool Hexahedron::compute( const Solid* solid, const IsInternalFlag intFlag )
  {
    _polygons.clear();
    _polygons.reserve( 20 );

    for ( int iN = 0; iN < 8; ++iN )
      _hexNodes[iN]._usedInFace = 0;

    if ( intFlag & IS_CUT_BY_INTERNAL_FACE && !_grid->_toAddEdges ) // Issue #19913
      preventVolumesOverlapping();

    // Create polygons from quadrangles
    // --------------------------------

    vector< _OrientedLink > splits;
    vector<_Node*>          chainNodes;
    _Face*                  coplanarPolyg;

    const bool hasEdgeIntersections = !_eIntPoints.empty();
    const bool toCheckSideDivision = isImplementEdges() || intFlag & IS_CUT_BY_INTERNAL_FACE;

    for ( int iF = 0; iF < 6; ++iF ) // loop on 6 sides of a hexahedron
    {
      _Face& quad = _hexQuads[ iF ] ;

      _polygons.resize( _polygons.size() + 1 );
      _Face* polygon = &_polygons.back();
      polygon->_polyLinks.reserve( 20 );
      polygon->_name = quad._name;

      splits.clear();
      for ( int iE = 0; iE < 4; ++iE ) // loop on 4 sides of a quadrangle
        for ( int iS = 0; iS < quad._links[ iE ].NbResultLinks(); ++iS )
          splits.push_back( quad._links[ iE ].ResultLink( iS ));

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
            if ( findChainOnEdge( splits, polygon->_links.back(), split, iS, quad, chainNodes ))
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
    }  // loop on 6 hexahedron sides

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
        _Node* equalNode =
          findEqualNode( _vIntNodes, _intNodes[ iN ].EdgeIntPnt(), tol*tol );
        if ( !equalNode )
        {
          _vIntNodes.push_back( &_intNodes[ iN ]);
          ++nbVertexNodes;
        }
      }
    }

    set<TGeomID> usedFaceIDs;
    vector< TGeomID > faces;
    TGeomID curFace = 0;
    const size_t nbQuadPolygons = _polygons.size();
    E_IntersectPoint ipTmp;

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
        return false; // closed polygon not found -> invalid polyhedron

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

            // an E_IntersectPoint used to mark nodes of coplanarPolyg
            // as lying on curFace while they are not at intersection with geometry
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
                      if ( n->_intPoint ) n->_intPoint->Add( ipTmp._faceIDs );
                      else                n->_intPoint = &ipTmp;
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

    // check volume size
    _hasTooSmall = ! checkPolyhedronSize( intFlag & IS_CUT_BY_INTERNAL_FACE );

    for ( size_t i = 0; i < 8; ++i )
      if ( _hexNodes[ i ]._intPoint == &ipTmp )
        _hexNodes[ i ]._intPoint = 0;

    if ( _hasTooSmall )
      return false; // too small volume


    // Try to find out names of no-name polygons (issue # 19887)
    if ( _grid->IsToRemoveExcessEntities() && _polygons.back()._name == SMESH_Block::ID_NONE )
    {
      gp_XYZ uvwCenter =
        0.5 * ( _grid->_coords[0][_i] + _grid->_coords[0][_i+1] ) * _grid->_axes[0] +
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
          gp_XYZ p = SMESH_NodeXYZ( n->Node() );
          _grid->ComputeUVW( p, uvw.ChangeCoord().ChangeData() );
          bb.Add( uvw );
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

    _volumeDefs._nodes.clear();
    _volumeDefs._quantities.clear();
    _volumeDefs._names.clear();

    // create a classic cell if possible

    int nbPolygons = 0;
    for ( size_t iF = 0; iF < _polygons.size(); ++iF )
      nbPolygons += (_polygons[ iF ]._links.size() > 0 );

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
        if ( nbLinks == 0 ) continue;
        _volumeDefs._quantities.push_back( nbLinks );
        _volumeDefs._names.push_back( _polygons[ iF ]._name );
        for ( size_t iL = 0; iL < nbLinks; ++iL )
          _volumeDefs._nodes.push_back( _polygons[ iF ]._links[ iL ].FirstNode() );
      }
    }
    _volumeDefs._solidID = solid->ID();

    return !_volumeDefs._nodes.empty();
  }
  //================================================================================
  /*!
   * \brief Create elements in the mesh
   */
  int Hexahedron::MakeElements(SMESH_MesherHelper&                      helper,
                               const map< TGeomID, vector< TGeomID > >& edge2faceIDsMap)
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
        // all intersection of hex with geometry are at grid nodes
        hex = new Hexahedron( *this, _i, _j, _k, i );
        intHexa.push_back( hex );
      }
    }

    // compute definitions of volumes resulted from hexadron intersection
#ifdef WITH_TBB
    tbb::parallel_for ( tbb::blocked_range<size_t>( 0, intHexa.size() ),
                        ParallelHexahedron( intHexa ),
                        tbb::simple_partitioner()); // computeElements() is called here
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
            vip = _grid->Add( ip );
          if ( isInternal && !sameV )
            vip->_faceIDs.push_back( _grid->PseudoIntExtFaceID() );
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
#ifdef _DEBUG_
        // check if ip is really inside the hex
        if ( h->isOutParam( ip->_uvw ))
          throw SALOME_Exception("ip outside a hex");
#endif
      }
    }
    return added;
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
    _Node*   node = polygon->_links.back().LastNode();
    int avoidFace = node->IsLinked( polygon->_links.back().FirstNode()->_intPoint );
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
                                    size_t &                       iS,
                                    _Face&                         quad,
                                    vector<_Node*>&                chn )
  {
    _Node* pn1 = prevSplit.FirstNode();
    _Node* pn2 = prevSplit.LastNode();
    int avoidFace = pn1->IsLinked( pn2->_intPoint ); // FACE under the quad
    if ( avoidFace < 1 && pn1->_intPoint )
      return false;

    _Node* n = 0, *stopNode = avoidSplit.LastNode();

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

    int i;
    for ( i = splits.size()-1; i >= 0; --i ) // connect new pn2 (at _eIntNodes) with a split
    {
      if ( !splits[i] )
        continue;

      n = splits[i].LastNode();
      if ( n == stopNode )
        break;
      if (( n != pn1 ) &&
          ( n->IsLinked( pn2->_intPoint, avoidFace )) &&
          ( !avoidFace || n->IsOnFace( avoidFace )))
        break;

      n = splits[i].FirstNode();
      if ( n == stopNode )
        break;
      if (( n->IsLinked( pn2->_intPoint, avoidFace )) &&
          ( !avoidFace || n->IsOnFace( avoidFace )))
        break;
      n = 0;
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
      for ( int i = chn.size() - 2; i >= 0; --i )
        chn.push_back( chn[ i ]);
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
          _grid->SetOnShape( nodes[iN], noIntPnt, /*unset=*/true );
          nodes[iN]->setIsMarked( true );
        }
      }

      const SMDS_MeshElement* v = 0;
      if ( !volDef->_quantities.empty() )
      {
        v = helper.AddPolyhedralVolume( nodes, volDef->_quantities );
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
      if (( volDef->_volume = v ))
      {
        helper.GetMeshDS()->SetMeshElementOnShape( v, volDef->_solidID );
        ++nbAdded;
      }
    }

    return nbAdded;
  }
  //================================================================================
  /*!
   * \brief Return true if the element is in a hole
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
  bool Hexahedron::checkPolyhedronSize( bool cutByInternalFace ) const
  {
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
    if ( this->hasStrangeEdge() )
      return true;

    double volume = 0;
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

    double initVolume = _sideLength[0] * _sideLength[1] * _sideLength[2];

    return volume > initVolume / _grid->_sizeThreshold;
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
   * \brief Dump a link and return \c false
   */
  bool Hexahedron::debugDumpLink( Hexahedron::_Link* link )
  {
#ifdef _DEBUG_
    gp_Pnt p1 = link->_nodes[0]->Point(), p2 = link->_nodes[1]->Point();
    cout << "BUG: not shared link. IKJ = ( "<< _i << " " << _j << " " << _k << " )" << endl
         << "n1 (" << p1.X() << ", "<< p1.Y() << ", "<< p1.Z() << " )" << endl
         << "n2 (" << p2.X() << ", "<< p2.Y() << ", "<< p2.Z() << " )" << endl;
#else
    (void)link; // unused in release mode
#endif
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
    int nbTrias = divider.GetTriangles( polygon, face.myNodes );
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
        newTriangle = meshDS->AddFace( face.myNodes[0], face.myNodes[1], face.myNodes[2] );
        meshDS->SetMeshElementOnShape( newTriangle, faceID );
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
      newTriangle = meshDS->AddFace( face.myNodes[iN], face.myNodes[iN+1], face.myNodes[iN+2] );
      meshDS->SetMeshElementOnShape( newTriangle, faceID );
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
        bool isBoundary = vTool.IsFreeFace( iF );
        if ( isBoundary )
        {
          bndFacets.push_back( iF );
        }
        else if ( hasInternal )
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
          for ( size_t iN = 0; iN < nbFaceNodes &&  !faceID; ++iN )
          {
            // look for a father FACE of EDGEs and VERTEXes
            const TopoDS_Shape& s1 = _grid->Shape( nn[ iN   ]->GetShapeID() );
            const TopoDS_Shape& s2 = _grid->Shape( nn[ iN+1 ]->GetShapeID() );
            if ( s1 != s2 && s1.ShapeType() == TopAbs_EDGE && s2.ShapeType() == TopAbs_EDGE )
            {
              TopoDS_Shape f = helper.GetCommonAncestor( s1, s2, *helper.GetMesh(), TopAbs_FACE );
              if ( !f.IsNull() )
                faceID = _grid->ShapeID( f );
            }
          }

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
            if ( !faceID && !isBoundary )
              continue;
          }
        }
        // orient a new face according to supporting FACE orientation in shape_to_mesh
        if ( !solid->IsOutsideOriented( faceID ))
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

        // split a polygon that will be used by other 3D algorithm
        if ( faceID && nbFaceNodes > 4 &&
             !_grid->IsInternal( faceID ) &&
             !_grid->IsShared( faceID ) &&
             !_grid->IsBoundaryFace( faceID ))
        {
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
        const SMDS_MeshElement* anyFace = *facesToOrient.begin();
        editor.Reorient2D( facesToOrient, direction, anyFace );
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
      if ( volDef->_volume && !volDef->_volume->isMarked() )
      {
        volDef->_volume->setIsMarked( true );
        boundaryElems.push_back( volDef->_volume );

        if ( _grid->IsToCheckNodePos() ) // un-mark nodes marked in addVolumes()
          for ( size_t iN = 0; iN < volDef->_nodes.size(); ++iN )
            volDef->_nodes[iN].Node()->setIsMarked( false );
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

  //================================================================================
  /*!
   * \brief computes exact bounding box with axes parallel to given ones
   */
  //================================================================================

  void getExactBndBox( const vector< TopoDS_Shape >& faceVec,
                       const double*                 axesDirs,
                       Bnd_Box&                      shapeBox )
  {
    BRep_Builder b;
    TopoDS_Compound allFacesComp;
    b.MakeCompound( allFacesComp );
    for ( size_t iF = 0; iF < faceVec.size(); ++iF )
      b.Add( allFacesComp, faceVec[ iF ] );

    double sP[6]; // aXmin, aYmin, aZmin, aXmax, aYmax, aZmax
    shapeBox.Get(sP[0],sP[1],sP[2],sP[3],sP[4],sP[5]);
    double farDist = 0;
    for ( int i = 0; i < 6; ++i )
      farDist = Max( farDist, 10 * sP[i] );

    gp_XYZ axis[3] = { gp_XYZ( axesDirs[0], axesDirs[1], axesDirs[2] ),
                       gp_XYZ( axesDirs[3], axesDirs[4], axesDirs[5] ),
                       gp_XYZ( axesDirs[6], axesDirs[7], axesDirs[8] ) };
    axis[0].Normalize();
    axis[1].Normalize();
    axis[2].Normalize();

    gp_Mat basis( axis[0], axis[1], axis[2] );
    gp_Mat bi = basis.Inverted();

    gp_Pnt pMin, pMax;
    for ( int iDir = 0; iDir < 3; ++iDir )
    {
      gp_XYZ axis0 = axis[ iDir ];
      gp_XYZ axis1 = axis[ ( iDir + 1 ) % 3 ];
      gp_XYZ axis2 = axis[ ( iDir + 2 ) % 3 ];
      for ( int isMax = 0; isMax < 2; ++isMax )
      {
        double shift = isMax ? farDist : -farDist;
        gp_XYZ orig = shift * axis0;
        gp_XYZ norm = axis1 ^ axis2;
        gp_Pln pln( orig, norm );
        norm = pln.Axis().Direction().XYZ();
        BRepBuilderAPI_MakeFace plane( pln, -farDist, farDist, -farDist, farDist );

        gp_Pnt& pAxis = isMax ? pMax : pMin;
        gp_Pnt pPlane, pFaces;
        double dist = GEOMUtils::GetMinDistance( plane, allFacesComp, pPlane, pFaces );
        if ( dist < 0 )
        {
          Bnd_B3d bb;
          gp_XYZ corner;
          for ( int i = 0; i < 2; ++i ) {
            corner.SetCoord( 1, sP[ i*3 ]);
            for ( int j = 0; j < 2; ++j ) {
              corner.SetCoord( 2, sP[ i*3 + 1 ]);
              for ( int k = 0; k < 2; ++k )
              {
                corner.SetCoord( 3, sP[ i*3 + 2 ]);
                corner *= bi;
                bb.Add( corner );
              }
            }
          }
          corner = isMax ? bb.CornerMax() : bb.CornerMin();
          pAxis.SetCoord( iDir+1, corner.Coord( iDir+1 ));
        }
        else
        {
          gp_XYZ pf = pFaces.XYZ() * bi;
          pAxis.SetCoord( iDir+1, pf.Coord( iDir+1 ) );
        }
      }
    } // loop on 3 axes

    shapeBox.SetVoid();
    shapeBox.Add( pMin );
    shapeBox.Add( pMax );

    return;
  }

} // namespace

//=============================================================================
/*!
 * \brief Generates 3D structured Cartesian mesh in the internal part of
 * solid shapes and polyhedral volumes near the shape boundary.
 *  \param theMesh - mesh to fill in
 *  \param theShape - a compound of all SOLIDs to mesh
 *  \retval bool - true in case of success
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::Compute(SMESH_Mesh &         theMesh,
                                      const TopoDS_Shape & theShape)
{
  // The algorithm generates the mesh in following steps:

  // 1) Intersection of grid lines with the geometry boundary.
  // This step allows to find out if a given node of the initial grid is
  // inside or outside the geometry.

  // 2) For each cell of the grid, check how many of it's nodes are outside
  // of the geometry boundary. Depending on a result of this check
  // - skip a cell, if all it's nodes are outside
  // - skip a cell, if it is too small according to the size threshold
  // - add a hexahedron in the mesh, if all nodes are inside
  // - add a polyhedron in the mesh, if some nodes are inside and some outside

  _computeCanceled = false;

  SMESH_MesherHelper helper( theMesh );
  SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();

  try
  {
    Grid grid;
    grid._helper                         = &helper;
    grid._toAddEdges                     = _hyp->GetToAddEdges();
    grid._toCreateFaces                  = _hyp->GetToCreateFaces();
    grid._toConsiderInternalFaces        = _hyp->GetToConsiderInternalFaces();
    grid._toUseThresholdForInternalFaces = _hyp->GetToUseThresholdForInternalFaces();
    grid._sizeThreshold                  = _hyp->GetSizeThreshold();
    grid.InitGeometry( theShape );

    vector< TopoDS_Shape > faceVec;
    {
      TopTools_MapOfShape faceMap;
      TopExp_Explorer fExp;
      for ( fExp.Init( theShape, TopAbs_FACE ); fExp.More(); fExp.Next() )
      {
        bool isNewFace = faceMap.Add( fExp.Current() );
        if ( !grid._toConsiderInternalFaces )
          if ( !isNewFace || fExp.Current().Orientation() == TopAbs_INTERNAL )
            // remove an internal face
            faceMap.Remove( fExp.Current() );
      }
      faceVec.reserve( faceMap.Extent() );
      faceVec.assign( faceMap.cbegin(), faceMap.cend() );
    }
    vector<FaceGridIntersector> facesItersectors( faceVec.size() );
    Bnd_Box shapeBox;
    for ( size_t i = 0; i < faceVec.size(); ++i )
    {
      facesItersectors[i]._face   = TopoDS::Face( faceVec[i] );
      facesItersectors[i]._faceID = grid.ShapeID( faceVec[i] );
      facesItersectors[i]._grid   = &grid;
      shapeBox.Add( facesItersectors[i].GetFaceBndBox() );
    }
    getExactBndBox( faceVec, _hyp->GetAxisDirs(), shapeBox );


    vector<double> xCoords, yCoords, zCoords;
    _hyp->GetCoordinates( xCoords, yCoords, zCoords, shapeBox );

    grid.SetCoordinates( xCoords, yCoords, zCoords, _hyp->GetAxisDirs(), shapeBox );

    if ( _computeCanceled ) return false;

#ifdef WITH_TBB
    { // copy partner faces and curves of not thread-safe types
      set< const Standard_Transient* > tshapes;
      BRepBuilderAPI_Copy copier;
      for ( size_t i = 0; i < facesItersectors.size(); ++i )
      {
        if ( !facesItersectors[i].IsThreadSafe( tshapes ))
        {
          copier.Perform( facesItersectors[i]._face );
          facesItersectors[i]._face = TopoDS::Face( copier );
        }
      }
    }
    // Intersection of grid lines with the geometry boundary.
    tbb::parallel_for ( tbb::blocked_range<size_t>( 0, facesItersectors.size() ),
                        ParallelIntersector( facesItersectors ),
                        tbb::simple_partitioner());
#else
    for ( size_t i = 0; i < facesItersectors.size(); ++i )
      facesItersectors[i].Intersect();
#endif

    // put intersection points onto the GridLine's; this is done after intersection
    // to avoid contention of facesItersectors for writing into the same GridLine
    // in case of parallel work of facesItersectors
    for ( size_t i = 0; i < facesItersectors.size(); ++i )
      facesItersectors[i].StoreIntersections();

    if ( _computeCanceled ) return false;

    // create nodes on the geometry
    grid.ComputeNodes( helper );

    if ( _computeCanceled ) return false;

    // get EDGEs to take into account
    map< TGeomID, vector< TGeomID > > edge2faceIDsMap;
    grid.GetEdgesToImplement( edge2faceIDsMap, theShape, faceVec );

    // create volume elements
    Hexahedron hex( &grid );
    int nbAdded = hex.MakeElements( helper, edge2faceIDsMap );

    if ( nbAdded > 0 )
    {
      if ( !grid._toConsiderInternalFaces )
      {
        // make all SOLIDs computed
        TopExp_Explorer solidExp( theShape, TopAbs_SOLID );
        if ( SMESHDS_SubMesh* sm1 = meshDS->MeshElements( solidExp.Current()) )
        {
          SMDS_ElemIteratorPtr volIt = sm1->GetElements();
          for ( ; solidExp.More() && volIt->more(); solidExp.Next() )
          {
            const SMDS_MeshElement* vol = volIt->next();
            sm1->RemoveElement( vol );
            meshDS->SetMeshElementOnShape( vol, solidExp.Current() );
          }
        }
      }
      // make other sub-shapes computed
      setSubmeshesComputed( theMesh, theShape );
    }

    // remove free nodes
    //if ( SMESHDS_SubMesh * smDS = meshDS->MeshElements( helper.GetSubShapeID() ))
    {
      std::vector< const SMDS_MeshNode* > nodesToRemove;
      // get intersection nodes
      for ( int iDir = 0; iDir < 3; ++iDir )
      {
        vector< GridLine >& lines = grid._lines[ iDir ];
        for ( size_t i = 0; i < lines.size(); ++i )
        {
          multiset< F_IntersectPoint >::iterator ip = lines[i]._intPoints.begin();
          for ( ; ip != lines[i]._intPoints.end(); ++ip )
            if ( ip->_node && ip->_node->NbInverseElements() == 0 && !ip->_node->isMarked() )
            {
              nodesToRemove.push_back( ip->_node );
              ip->_node->setIsMarked( true );
            }
        }
      }
      // get grid nodes
      for ( size_t i = 0; i < grid._nodes.size(); ++i )
        if ( grid._nodes[i] && grid._nodes[i]->NbInverseElements() == 0 &&
             !grid._nodes[i]->isMarked() )
        {
          nodesToRemove.push_back( grid._nodes[i] );
          grid._nodes[i]->setIsMarked( true );
        }

      // do remove
      for ( size_t i = 0; i < nodesToRemove.size(); ++i )
        meshDS->RemoveFreeNode( nodesToRemove[i], /*smD=*/0, /*fromGroups=*/false );
    }

    return nbAdded;

  }
  // SMESH_ComputeError is not caught at SMESH_submesh level for an unknown reason
  catch ( SMESH_ComputeError& e)
  {
    return error( SMESH_ComputeErrorPtr( new SMESH_ComputeError( e )));
  }
  return false;
}

//=============================================================================
/*!
 *  Evaluate
 */
//=============================================================================

bool StdMeshers_Cartesian_3D::Evaluate(SMESH_Mesh &         /*theMesh*/,
                                       const TopoDS_Shape & /*theShape*/,
                                       MapShapeNbElems&     /*theResMap*/)
{
  // TODO
//   std::vector<int> aResVec(SMDSEntity_Last);
//   for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aResVec[i] = 0;
//   if(IsQuadratic) {
//     aResVec[SMDSEntity_Quad_Cartesian] = nb2d_face0 * ( nb2d/nb1d );
//     int nb1d_face0_int = ( nb2d_face0*4 - nb1d ) / 2;
//     aResVec[SMDSEntity_Node] = nb0d_face0 * ( 2*nb2d/nb1d - 1 ) - nb1d_face0_int * nb2d/nb1d;
//   }
//   else {
//     aResVec[SMDSEntity_Node] = nb0d_face0 * ( nb2d/nb1d - 1 );
//     aResVec[SMDSEntity_Cartesian] = nb2d_face0 * ( nb2d/nb1d );
//   }
//   SMESH_subMesh * sm = aMesh.GetSubMesh(aShape);
//   aResMap.insert(std::make_pair(sm,aResVec));

  return true;
}

//=============================================================================
namespace
{
  /*!
   * \brief Event listener setting/unsetting _alwaysComputed flag to
   *        submeshes of inferior levels to prevent their computing
   */
  struct _EventListener : public SMESH_subMeshEventListener
  {
    string _algoName;

    _EventListener(const string& algoName):
      SMESH_subMeshEventListener(/*isDeletable=*/true,"StdMeshers_Cartesian_3D::_EventListener"),
      _algoName(algoName)
    {}
    // --------------------------------------------------------------------------------
    // setting/unsetting _alwaysComputed flag to submeshes of inferior levels
    //
    static void setAlwaysComputed( const bool     isComputed,
                                   SMESH_subMesh* subMeshOfSolid)
    {
      SMESH_subMeshIteratorPtr smIt =
        subMeshOfSolid->getDependsOnIterator(/*includeSelf=*/false, /*complexShapeFirst=*/false);
      while ( smIt->more() )
      {
        SMESH_subMesh* sm = smIt->next();
        sm->SetIsAlwaysComputed( isComputed );
      }
      subMeshOfSolid->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
    }

    // --------------------------------------------------------------------------------
    // unsetting _alwaysComputed flag if "Cartesian_3D" was removed
    //
    virtual void ProcessEvent(const int          /*event*/,
                              const int          eventType,
                              SMESH_subMesh*     subMeshOfSolid,
                              SMESH_subMeshEventListenerData* /*data*/,
                              const SMESH_Hypothesis*         /*hyp*/ = 0)
    {
      if ( eventType == SMESH_subMesh::COMPUTE_EVENT )
      {
        setAlwaysComputed( subMeshOfSolid->GetComputeState() == SMESH_subMesh::COMPUTE_OK,
                           subMeshOfSolid );
      }
      else
      {
        SMESH_Algo* algo3D = subMeshOfSolid->GetAlgo();
        if ( !algo3D || _algoName != algo3D->GetName() )
          setAlwaysComputed( false, subMeshOfSolid );
      }
    }

    // --------------------------------------------------------------------------------
    // set the event listener
    //
    static void SetOn( SMESH_subMesh* subMeshOfSolid, const string& algoName )
    {
      subMeshOfSolid->SetEventListener( new _EventListener( algoName ),
                                        /*data=*/0,
                                        subMeshOfSolid );
    }

  }; // struct _EventListener

} // namespace

//================================================================================
/*!
 * \brief Sets event listener to submeshes if necessary
 *  \param subMesh - submesh where algo is set
 * This method is called when a submesh gets HYP_OK algo_state.
 * After being set, event listener is notified on each event of a submesh.
 */
//================================================================================

void StdMeshers_Cartesian_3D::SetEventListener(SMESH_subMesh* subMesh)
{
  _EventListener::SetOn( subMesh, GetName() );
}

//================================================================================
/*!
 * \brief Set _alwaysComputed flag to submeshes of inferior levels to avoid their computing
 */
//================================================================================

void StdMeshers_Cartesian_3D::setSubmeshesComputed(SMESH_Mesh&         theMesh,
                                                   const TopoDS_Shape& theShape)
{
  for ( TopExp_Explorer soExp( theShape, TopAbs_SOLID ); soExp.More(); soExp.Next() )
    _EventListener::setAlwaysComputed( true, theMesh.GetSubMesh( soExp.Current() ));
}
