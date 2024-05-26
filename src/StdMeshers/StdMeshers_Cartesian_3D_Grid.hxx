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
//  File   : StdMeshers_Cartesian_3D_Grid.hxx
//  Module : SMESH
//  Purpose: Make BodyFitting mesh algorithm more modular and testable
//

#ifndef _SMESH_Cartesian_3D_GRID_HXX_
#define _SMESH_Cartesian_3D_GRID_HXX_

#ifdef WITH_TBB
#include <tbb/parallel_for.h>
#endif

#include <utilities.h>

// STD
#include <algorithm>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>


// SMESH
#include "SMESH_StdMeshers.hxx"
#include "StdMeshers_FaceSide.hxx"
#include "StdMeshers_CartesianParameters3D.hxx"

#include <ObjectPool.hxx>
#include <SMDS_LinearEdge.hxx>
#include <SMDS_MeshNode.hxx>
#include <SMDS_VolumeOfNodes.hxx>
#include <SMDS_VolumeTool.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_Block.hxx>
#include <SMESH_Comment.hxx>
#include <SMESH_ControlsDef.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_MeshAlgos.hxx>
#include <SMESH_MeshEditor.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_subMesh.hxx>
#include <SMESH_subMeshEventListener.hxx>

#include <utilities.h>
#include <Utils_ExceptHandlers.hxx>

#include <GEOMUtils.hxx>

//OCC
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
#include <TopTools_DataMapOfShapeInteger.hxx>
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

// All utility structs used in Grid and hexahedron class will be included here
// Ideally each one of this should define their own testable class
using namespace std;
using namespace SMESH;
namespace gridtools
{
  typedef int                     TGeomID; // IDs of sub-shapes
  typedef TopTools_ShapeMapHasher TShapeHasher; // non-oriented shape hasher
  typedef std::array< int, 3 >    TIJK;

  const TGeomID theUndefID = 1e+9;

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
   * \brief Sub-entities of a FACE neighboring its concave VERTEX.
   *        Help to avoid linking nodes on EDGEs that seem connected
   *        by the concave FACE but the link actually lies outside the FACE
   */
  struct ConcaveFace
  {
    TGeomID _concaveFace;
    TGeomID _edge1, _edge2;
    TGeomID _v1,    _v2;
    ConcaveFace( int f=0, int e1=0, int e2=0, int v1=0, int v2=0 )
      : _concaveFace(f), _edge1(e1), _edge2(e2), _v1(v1), _v2(v2) {}
    bool HasEdge( TGeomID edge ) const { return edge == _edge1 || edge == _edge2; }
    bool HasVertex( TGeomID v  ) const { return v == _v1 || v == _v2; }
    void SetEdge( TGeomID edge ) { ( _edge1 ? _edge2 : _edge1 ) = edge; }
    void SetVertex( TGeomID v  ) { ( _v1 ? _v2 : _v1 ) = v; }
  };
  typedef NCollection_DataMap< TGeomID, ConcaveFace > TConcaveVertex2Face;
  // --------------------------------------------------------------------------
  /*!
   * \brief Container of IDs of SOLID sub-shapes
   */
  class Solid // sole SOLID contains all sub-shapes
  {
    TGeomID             _id; // SOLID id
    bool                _hasInternalFaces;
    TConcaveVertex2Face _concaveVertex; // concave VERTEX -> ConcaveFace
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
    void SetConcave( TGeomID V, TGeomID F, TGeomID E1, TGeomID E2, TGeomID V1, TGeomID V2  )
    { _concaveVertex.Bind( V, ConcaveFace{ F, E1, E2, V1, V2 }); }
    bool HasConcaveVertex() const { return !_concaveVertex.IsEmpty(); }
    const ConcaveFace* GetConcave( TGeomID V ) const { return _concaveVertex.Seek( V ); }
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
   * \brief Hold a vector of TGeomID and clear it at destruction
   */
  class GeomIDVecHelder
  {
    typedef std::vector< TGeomID > TVector;
    const TVector& myVec;
    bool           myOwn;

  public:
    GeomIDVecHelder( const TVector& idVec, bool isOwner ): myVec( idVec ), myOwn( isOwner ) {}
    GeomIDVecHelder( const GeomIDVecHelder& holder ): myVec( holder.myVec ), myOwn( holder.myOwn )
    {
      const_cast< bool& >( holder.myOwn ) = false;
    }
    ~GeomIDVecHelder() { if ( myOwn ) const_cast<TVector&>( myVec ).clear(); }
    size_t size() const { return myVec.size(); }
    TGeomID operator[]( size_t i ) const { return i < size() ? myVec[i] : theUndefID; }
    bool operator==( const GeomIDVecHelder& other ) const { return myVec == other.myVec; }
    bool contain( const TGeomID& id ) const {
      return std::find( myVec.begin(), myVec.end(), id ) != myVec.end();
    }
    TGeomID otherThan( const TGeomID& id ) const {
      for ( const TGeomID& id2 : myVec )
        if ( id != id2 )
          return id2;
      return theUndefID;
    }
    TGeomID oneCommon( const GeomIDVecHelder& other ) const {
      TGeomID common = theUndefID;
      for ( const TGeomID& id : myVec )
        if ( other.contain( id ))
        {
          if ( common != theUndefID )
            return theUndefID;
          common = id;
        }
      return common;
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

    TopTools_DataMapOfShapeInteger _shape2NbNodes; // nb of pre-existing nodes on shapes

    Controls::ElementsOnShape _edgeClassifier;
    Controls::ElementsOnShape _vertexClassifier;

    bool IsOneSolid() const { return _solidByID.size() < 2; }
    GeomIDVecHelder GetSolidIDsByShapeID( const vector< TGeomID >& shapeIDs ) const;
  };

  // --------------------------------------------------------------------------
  /*!
   * \brief Common data of any intersection between a Grid and a shape
   */
  struct B_IntersectPoint
  {
    // This two class members are being updated in a non thread safe way.
    // See Add method modify _node and _faceIDs class members dinamicaly during execution
    // of Hexahedron.compute() method.
    // std::mutex _mutex;
    mutable const SMDS_MeshNode* _node;
    mutable vector< TGeomID >    _faceIDs;

    B_IntersectPoint(): _node(NULL) {}
    bool Add( const vector< TGeomID >& fIDs, const SMDS_MeshNode* n=NULL ) const;
    TGeomID HasCommonFace( const B_IntersectPoint * other, TGeomID avoidFace=-1 ) const;
    size_t GetCommonFaces( const B_IntersectPoint * other, TGeomID * commonFaces ) const;
    bool IsOnFace( TGeomID faceID ) const;
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

    bool operator< ( const F_IntersectPoint& o ) const { 
      return _paramOnLine < o._paramOnLine;         
    }
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
    void SetLineIndex(size_t i)
    {
      _curInd[_iVar2] = i / _size[_iVar1];
      _curInd[_iVar1] = i % _size[_iVar1];
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
    bool IsValidIndexOnLine (size_t i) const { return  i < _size[ _iConst ]; }
    size_t NbLines() const { return _size[_iVar1] * _size[_iVar2]; }
  };

  class Tools 
  { 
    public:
      Tools() = delete;
      
      //================================================================================
      /*!
      * \brief computes exact bounding box with axes parallel to given ones
      */
      //================================================================================
      static void GetExactBndBox( const vector< TopoDS_Shape >& faceVec, const double* axesDirs, Bnd_Box& shapeBox );
  };
} // end namespace gridtools

using namespace gridtools;  
class STDMESHERS_EXPORT Grid
{

public:
    vector< double >   _coords[3]; // coordinates of grid nodes
    gp_XYZ             _axes  [3]; // axis directions
    vector< GridLine > _lines [3]; //    in 3 directions
    double             _tol, _minCellSize;
    gp_XYZ             _origin;
    gp_Mat             _invB; // inverted basis of _axes

    // index shift within _nodes of nodes of a cell from the 1st node
    int                _nodeShift[8];

    vector< const SMDS_MeshNode* >    _nodes;          // mesh nodes at grid nodes
    vector< const SMDS_MeshNode* >    _allBorderNodes; // mesh nodes between the bounding box and the geometry boundary

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
    bool                              _toUseQuanta;
    double                            _quanta;

    SMESH_MesherHelper*               _helper;

    size_t CellIndex( size_t i, size_t j, size_t k ) const
    {
      return i + j*(_coords[0].size()-1) + k*(_coords[0].size()-1)*(_coords[1].size()-1);
    }
    size_t NodeIndex( size_t i, size_t j, size_t k ) const
    {
      return i + j*_coords[0].size() + k*_coords[0].size()*_coords[1].size();
    }
    size_t NodeIndex( const TIJK& ijk ) const
    {
      return NodeIndex( ijk[0], ijk[1], ijk[2] );
    }
    size_t NodeIndexDX() const { return 1; }
    size_t NodeIndexDY() const { return _coords[0].size(); }
    size_t NodeIndexDZ() const { return _coords[0].size() * _coords[1].size(); }

    LineIndexer GetLineIndexer(size_t iDir) const;
    size_t GetLineDir( const GridLine* line, size_t & index ) const;

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
    void SetOnShape( const SMDS_MeshNode* n, const F_IntersectPoint& ip,
                    TopoDS_Vertex* vertex = nullptr, bool unset = false );
    void UpdateFacesOfVertex( const B_IntersectPoint& ip, const TopoDS_Vertex& vertex );
    bool IsToCheckNodePos() const { return !_toAddEdges && _toCreateFaces; }
    bool IsToRemoveExcessEntities() const { return !_toAddEdges; }

    void SetCoordinates(const vector<double>& xCoords,
                        const vector<double>& yCoords,
                        const vector<double>& zCoords,
                        const double*         axesDirs,
                        const Bnd_Box&        bndBox );
    void ComputeUVW(const gp_XYZ& p, double uvw[3]);
    void ComputeNodes(SMESH_MesherHelper& helper);
    bool GridInitAndInterserctWithShape( const TopoDS_Shape& theShape, std::map< TGeomID, vector< TGeomID > >& edge2faceIDsMap, 
                                          const StdMeshers_CartesianParameters3D* hyp, const int numOfThreads, bool computeCanceled );
};

namespace
{

  // Implement parallel computation of Hexa with c++ thread implementation
  template<typename Iterator, class Function>
  void parallel_for(const Iterator& first, const Iterator& last, Function&& f, const int nthreads = 1)
  {
      const unsigned int group = ((last-first))/std::abs(nthreads);

      std::vector<std::thread> threads;
      threads.reserve(nthreads);
      Iterator it = first;
      for (; it < last-group; it += group) {
          // to create a thread 
          // Pass iterators by value and the function by reference!
          auto lambda = [=,&f](){ std::for_each(it, std::min(it+group, last), f);};

          // stack the threads 
          threads.push_back( std::thread( lambda ) );
      }
      std::for_each(it, last, f); // last steps while we wait for other threads
      std::for_each(threads.begin(), threads.end(), [](std::thread& x){x.join();});
  }
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
#ifdef WITH_TBB    
    bool IsThreadSafe(set< const Standard_Transient* >& noSafeTShapes) const;
#endif
  };
  
#ifdef WITH_TBB
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

  template<typename Type>
  void computeGridIntersection( Type faceGridIntersector )
  {
    faceGridIntersector.Intersect();
  }

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

#ifdef WITH_TBB
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
#endif
}

#endif
