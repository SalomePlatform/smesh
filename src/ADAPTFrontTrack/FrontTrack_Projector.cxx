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
// File      : FrontTrack_Projector.cxx
// Created   : Wed Apr 26 20:33:55 2017
// Author    : Edward AGAPOV (eap)

#include "FrontTrack_Projector.hxx"

#include <BRepAdaptor_Curve.hxx>
#include <BRepBndLib.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRep_Tool.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Circ.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>
#include <gp_Vec.hxx>

#include <limits>

//-----------------------------------------------------------------------------
/*!
 * \brief Root class of a projector of a point to a boundary shape
 */
struct FT_RealProjector
{
  virtual ~FT_RealProjector() {}

  /*!
   * \brief Project a point to a boundary shape
   *  \param [in] point - the point to project
   *  \param [out] newSolution - position on the shape (U or UV) found during the projection
   *  \param [in] prevSolution - position already found during the projection of a neighbor point
   *  \return gp_Pnt - the projection point
   */
  virtual gp_Pnt project( const gp_Pnt& point,
                          double*       newSolution,
                          const double* prevSolution = 0) = 0;

  /*!
   * \brief Project a point to a boundary shape and check if the projection is within
   *        the shape boundary
   *  \param [in] point - the point to project
   *  \param [in] maxDist2 - the maximal allowed square distance between point and projection
   *  \param [out] projection - the projection point
   *  \param [out] newSolution - position on the shape (U or UV) found during the projection
   *  \param [in] prevSolution - position already found during the projection of a neighbor point
   *  \return bool - false if the projection point lies out of the shape boundary or
   the distance the point and the projection is more than sqrt(maxDist2)
  */
  virtual bool projectAndClassify( const gp_Pnt& point,
                                   const double  maxDist2,
                                   gp_Pnt&       projection,
                                   double*       newSolution,
                                   const double* prevSolution = 0) = 0;

  // return true if a previously found solution can be used to speed up the projection

  virtual bool canUsePrevSolution() const { return false; }


  double _dist; // distance between the point being projected and its projection
};

namespace // actual projection algorithms
{
  const double theEPS = 1e-12;

  //================================================================================
  /*!
   * \brief Projector to any curve
   */
  //================================================================================

  struct CurveProjector : public FT_RealProjector
  {
    BRepAdaptor_Curve   _curve;
    double              _tol;
    ShapeAnalysis_Curve _projector;
    double              _uRange[2];

    //-----------------------------------------------------------------------------
    CurveProjector( const TopoDS_Edge& e, const double tol ):
      _curve( e ), _tol( tol )
    {
      BRep_Tool::Range( e, _uRange[0], _uRange[1] );
    }

    //-----------------------------------------------------------------------------
    // project a point to the curve
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
#ifdef _DEBUG_
    std::cout << ".. project a point to the curve prevSolution = " << prevSolution << std::endl;
#endif
      gp_Pnt         proj;
      Standard_Real param;

      if ( prevSolution )
      {
        _dist = _projector.NextProject( prevSolution[0], _curve, P, _tol, proj, param );
      }
      else
      {
        _dist = _projector.Project( _curve, P, _tol, proj, param, false );
      }
#ifdef _DEBUG_
    std::cout << "..    _dist : " << _dist << std::endl;
#endif
      proj = _curve.Value( param );

      newSolution[0] = param;

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to a curve and check if the projection is within the curve boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
#ifdef _DEBUG_
    std::cout << ".. project a point to a curve and check " << std::endl;
#endif
      projection = project( point, newSolution, prevSolution );
      return ( _uRange[0] < newSolution[0] && newSolution[0] < _uRange[1] &&
               _dist * _dist < maxDist2 );
    }

    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return true; }
  };

  //================================================================================
  /*!
   * \brief Projector to a straight curve. Don't project, classify only
   */
  //================================================================================

  struct LineProjector : public FT_RealProjector
  {
    gp_Pnt _p0, _p1;

    //-----------------------------------------------------------------------------
    LineProjector( TopoDS_Edge e )
    {
      e.Orientation( TopAbs_FORWARD );
      _p0 = BRep_Tool::Pnt( TopExp::FirstVertex( e ));
      _p1 = BRep_Tool::Pnt( TopExp::LastVertex ( e ));
    }

    //-----------------------------------------------------------------------------
    // does nothing
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      return P;
    }
    //-----------------------------------------------------------------------------
    // check if a point lies within the line segment
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      gp_Vec edge( _p0, _p1 );
      gp_Vec p0p ( _p0, point  );
      double u = ( edge * p0p ) / edge.SquareMagnitude();  // param [0,1] on the edge
      projection = ( 1. - u ) * _p0.XYZ() + u * _p1.XYZ(); // projection of the point on the edge
      if ( u < 0 || 1 < u )
        return false;

      // check distance
      return point.SquareDistance( projection ) < theEPS * theEPS;
    }
  };

  //================================================================================
  /*!
   * \brief Projector to a circular edge
   */
  //================================================================================

  struct CircleProjector : public FT_RealProjector
  {
    gp_Circ _circle;
    double _uRange[2];

    //-----------------------------------------------------------------------------
    CircleProjector( const gp_Circ& c, const double f, const double l ):
      _circle( c )
    {
      _uRange[0] = f;
      _uRange[1] = l;
    }

    //-----------------------------------------------------------------------------
    // project a point to the circle
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      // assume that P is already on the the plane of circle, since
      // it is in the middle of two points lying on the circle

      // move P to the circle
      const gp_Pnt& O = _circle.Location();
      gp_Vec radiusVec( O, P );
      double radius = radiusVec.Magnitude();
      if ( radius < std::numeric_limits<double>::min() )
        return P; // P in on the axe

      gp_Pnt proj = O.Translated( radiusVec.Multiplied( _circle.Radius() / radius ));

      _dist = _circle.Radius() - radius;

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project and check if a projection lies within the circular edge
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      _dist = -1;
      projection = project( point, newSolution );
      if ( _dist < 0 || // ?
           _dist * _dist > maxDist2 )
        return false;

      newSolution[0] = ElCLib::Parameter( _circle, projection );
      return ( _uRange[0] < newSolution[0] && newSolution[0] < _uRange[1] );
    }
  };

  //================================================================================
  /*!
   * \brief Projector to any surface
   */
  //================================================================================

  struct SurfaceProjector : public FT_RealProjector
  {
    ShapeAnalysis_Surface    _projector;
    double                   _tol;
    BRepTopAdaptor_FClass2d* _classifier;

    //-----------------------------------------------------------------------------
    SurfaceProjector( const TopoDS_Face& face, const double tol, BRepTopAdaptor_FClass2d* cls ):
      _projector( BRep_Tool::Surface( face )),
      _tol( tol ),
      _classifier( cls )
    {
    }
    //-----------------------------------------------------------------------------
    // delete _classifier
    ~SurfaceProjector()
    {
      delete _classifier;
    }

    //-----------------------------------------------------------------------------
    // project a point to a surface
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      gp_Pnt2d uv;

      if ( prevSolution )
      {
        gp_Pnt2d prevUV( prevSolution[0], prevSolution[1] );
        uv = _projector.NextValueOfUV( prevUV, P, _tol );
      }
      else
      {
        uv = _projector.ValueOfUV( P, _tol );
      }

      uv.Coord( newSolution[0], newSolution[1] );

      gp_Pnt proj = _projector.Value( uv );

      _dist = _projector.Gap();

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to a surface and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      projection = project( point, newSolution, prevSolution );
      return ( _dist * _dist < maxDist2 )  &&  classify( newSolution );
    }

    //-----------------------------------------------------------------------------
    // check if the projection is within the shape boundary
    bool classify( const double* newSolution )
    {
      TopAbs_State state = _classifier->Perform( gp_Pnt2d( newSolution[0], newSolution[1]) );
      return ( state != TopAbs_OUT );
    }

    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return true; }
  };

  //================================================================================
  /*!
   * \brief Projector to a plane. Don't project, classify only
   */
  //================================================================================

  struct PlaneProjector : public SurfaceProjector
  {
    gp_Pln _plane;
    bool   _isRealPlane; // false means that a surface is planar but parametrization is different

    //-----------------------------------------------------------------------------
    PlaneProjector( const gp_Pln&            pln,
                    const TopoDS_Face&       face,
                    BRepTopAdaptor_FClass2d* cls,
                    bool                     isRealPlane=true):
      SurfaceProjector( face, 0, cls ),
      _plane( pln ),
      _isRealPlane( isRealPlane )
    {}

    //-----------------------------------------------------------------------------
    // does nothing
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      return P;
    }
    //-----------------------------------------------------------------------------
    // check if a point lies within the boundry of the planar face
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      if ( _isRealPlane )
      {
        ElSLib::PlaneParameters( _plane.Position(), point, newSolution[0], newSolution[1]);
        projection = ElSLib::PlaneValue ( newSolution[0], newSolution[1], _plane.Position() );
        if ( projection.SquareDistance( point ) > theEPS * theEPS )
          return false;

        return SurfaceProjector::classify( newSolution );
      }
      else
      {
        return SurfaceProjector::projectAndClassify( point, maxDist2, projection,
                                                     newSolution, prevSolution );
      }
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a cylinder
   */
  //================================================================================

  struct CylinderProjector : public SurfaceProjector
  {
    gp_Cylinder _cylinder;

    //-----------------------------------------------------------------------------
    CylinderProjector( const gp_Cylinder&       c,
                       const TopoDS_Face&       face,
                       BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _cylinder( c )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the cylinder
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      // project the point P to the cylinder axis -> Pp
      const gp_Pnt& O   = _cylinder.Position().Location();
      const gp_Dir& axe = _cylinder.Position().Direction();
      gp_Vec       trsl = gp_Vec( axe ).Multiplied( gp_Vec( O, P ).Dot( axe ));
      gp_Pnt       Pp   = O.Translated( trsl );

      // move Pp to the cylinder
      gp_Vec radiusVec( Pp, P );
      double radius = radiusVec.Magnitude();
      if ( radius < std::numeric_limits<double>::min() )
        return P; // P in on the axe

      gp_Pnt proj = Pp.Translated( radiusVec.Multiplied( _cylinder.Radius() / radius ));

      _dist = _cylinder.Radius() - radius;

      return proj;
    }
    //-----------------------------------------------------------------------------
    // project a point to the cylinder and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      ElSLib::CylinderParameters( _cylinder.Position(), _cylinder.Radius(), point,
                                  newSolution[0], newSolution[1]);
      projection = ElSLib::CylinderValue( newSolution[0], newSolution[1],
                                          _cylinder.Position(), _cylinder.Radius() );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a cone
   */
  //================================================================================

  struct ConeProjector : public SurfaceProjector
  {
    gp_Cone _cone;

    //-----------------------------------------------------------------------------
    ConeProjector( const gp_Cone&           c,
                   const TopoDS_Face&       face,
                   BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _cone( c )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the cone
    virtual gp_Pnt project( const gp_Pnt& point,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      ElSLib::ConeParameters( _cone.Position(), _cone.RefRadius(), _cone.SemiAngle(),
                              point, newSolution[0], newSolution[1]);
      gp_Pnt proj = ElSLib::ConeValue( newSolution[0], newSolution[1],
                                       _cone.Position(), _cone.RefRadius(), _cone.SemiAngle() );
      _dist = point.Distance( proj );

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to the cone and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      projection = project( point, newSolution, prevSolution );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a sphere
   */
  //================================================================================

  struct SphereProjector : public SurfaceProjector
  {
    gp_Sphere _sphere;

    //-----------------------------------------------------------------------------
    SphereProjector( const gp_Sphere&         s,
                     const TopoDS_Face&       face,
                     BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _sphere( s )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the sphere
    virtual gp_Pnt project( const gp_Pnt& P,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      // move Pp to the Sphere
      const gp_Pnt& O = _sphere.Location();
      gp_Vec radiusVec( O, P );
      double radius = radiusVec.Magnitude();
      if ( radius < std::numeric_limits<double>::min() )
        return P; // P is on O

      gp_Pnt proj = O.Translated( radiusVec.Multiplied( _sphere.Radius() / radius ));

      _dist = _sphere.Radius() - radius;

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to the sphere and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      ElSLib::SphereParameters( _sphere.Position(), _sphere.Radius(), point,
                                  newSolution[0], newSolution[1]);
      projection = ElSLib::SphereValue( newSolution[0], newSolution[1],
                                        _sphere.Position(), _sphere.Radius() );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Projector to a torus
   */
  //================================================================================

  struct TorusProjector : public SurfaceProjector
  {
    gp_Torus _torus;

    //-----------------------------------------------------------------------------
    TorusProjector( const gp_Torus&          t,
                    const TopoDS_Face&       face,
                    BRepTopAdaptor_FClass2d* cls ):
      SurfaceProjector( face, 0, cls ),
      _torus( t )
    {}

    //-----------------------------------------------------------------------------
    // project a point to the torus
    virtual gp_Pnt project( const gp_Pnt& point,
                            double*       newSolution,
                            const double* prevSolution = 0)
    {
      ElSLib::TorusParameters( _torus.Position(), _torus.MajorRadius(), _torus.MinorRadius(),
                               point, newSolution[0], newSolution[1]);
      gp_Pnt proj = ElSLib::TorusValue( newSolution[0], newSolution[1],
                                        _torus.Position(), _torus.MajorRadius(), _torus.MinorRadius() );
      _dist = point.Distance( proj );

      return proj;
    }

    //-----------------------------------------------------------------------------
    // project a point to the torus and check if the projection is within the surface boundary
    virtual bool projectAndClassify( const gp_Pnt& point,
                                     const double  maxDist2,
                                     gp_Pnt&       projection,
                                     double*       newSolution,
                                     const double* prevSolution = 0)
    {
      projection = project( point, newSolution, prevSolution );

      return ( _dist * _dist < maxDist2 )  &&  SurfaceProjector::classify( newSolution );
    }
    //-----------------------------------------------------------------------------
    // return true if a previously found solution can be used to speed up the projection
    virtual bool canUsePrevSolution() const { return false; }
  };

  //================================================================================
  /*!
   * \brief Check if a curve can be considered straight
   */
  //================================================================================

  bool isStraight( const GeomAdaptor_Curve& curve, const double tol )
  {
    // rough check: evaluate how far from a straight line connecting the curve ends
    // stand several internal points of the curve

    const double  f = curve.FirstParameter();
    const double  l = curve.LastParameter();
    const gp_Pnt pf = curve.Value( f );
    const gp_Pnt pl = curve.Value( l );
    const gp_Vec lineVec( pf, pl );
    const double lineLen2 = lineVec.SquareMagnitude();
    if ( lineLen2 < std::numeric_limits< double >::min() )
      return false; // E seems closed

    const double nbSamples = 7;
    for ( int i = 0; i < nbSamples; ++i )
    {
      const double  r = ( i + 1 ) / nbSamples;
      const gp_Pnt pi = curve.Value( f * r + l * ( 1 - r ));
      const gp_Vec vi( pf, pi );
      const double h2 = lineVec.Crossed( vi ).SquareMagnitude() / lineLen2;
      if ( h2 > tol * tol )
        return false;
    }

    // thorough check
    GCPnts_UniformDeflection divider( curve, tol );
    return ( divider.IsDone() && divider.NbPoints() < 3 );
  }
}

//================================================================================
/*!
 * \brief Initialize with a boundary shape
 */
//================================================================================

FT_Projector::FT_Projector(const TopoDS_Shape& shape)
{
  _realProjector = 0;
  setBoundaryShape( shape );
  _tryWOPrevSolution = false;
}

//================================================================================
/*!
 * \brief Copy another projector
 */
//================================================================================

FT_Projector::FT_Projector(const FT_Projector& other)
{
  _realProjector = 0;
  _shape = other._shape;
  _bndBox = other._bndBox;
  _tryWOPrevSolution = false;
}

//================================================================================
/*!
 * \brief Destructor. Delete _realProjector
 */
//================================================================================

FT_Projector::~FT_Projector()
{
  delete _realProjector;
}

//================================================================================
/*!
 * \brief Initialize with a boundary shape. Compute the bounding box
 */
//================================================================================

void FT_Projector::setBoundaryShape(const TopoDS_Shape& shape)
{
  delete _realProjector; _realProjector = 0;
  _shape = shape;
  if ( shape.IsNull() )
    return;

  BRepBndLib::Add( shape, _bndBox );
  _bndBox.Enlarge( 1e-5 * sqrt( _bndBox.SquareExtent() ));
}

//================================================================================
/*!
 * \brief Create a real projector
 */
//================================================================================

void FT_Projector::prepareForProjection()
{
  if ( _shape.IsNull() || _realProjector )
    return;

  if ( _shape.ShapeType() == TopAbs_EDGE )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( _shape );

    double tol = 1e-6 * sqrt( _bndBox.SquareExtent() );

    double f,l;
    Handle(Geom_Curve) curve = BRep_Tool::Curve( edge, f,l );
    if ( curve.IsNull() )
      return; // degenerated edge

    GeomAdaptor_Curve acurve( curve, f, l );
    switch ( acurve.GetType() )
    {
    case GeomAbs_Line:
      _realProjector = new LineProjector( edge );
      break;
    case GeomAbs_Circle:
      _realProjector = new CircleProjector( acurve.Circle(), f, l );
      break;
    case GeomAbs_BezierCurve:
    case GeomAbs_BSplineCurve:
    case GeomAbs_OffsetCurve:
    case GeomAbs_OtherCurve:
      if ( isStraight( acurve, tol ))
      {
        _realProjector = new LineProjector( edge );
        break;
      }
    case GeomAbs_Ellipse:
    case GeomAbs_Hyperbola:
    case GeomAbs_Parabola:
      _realProjector = new CurveProjector( edge, tol );
    }
  }
  else if ( _shape.ShapeType() == TopAbs_FACE )
  {
    TopoDS_Face face = TopoDS::Face( _shape );

    Handle(Geom_Surface) surface = BRep_Tool::Surface( face );
    if ( surface.IsNull() )
      return;

    GeomAdaptor_Surface asurface( surface );
    Standard_Real tol   = BRep_Tool::Tolerance( face );
    Standard_Real toluv = Min( asurface.UResolution( tol ), asurface.VResolution( tol ));
    BRepTopAdaptor_FClass2d* classifier = new BRepTopAdaptor_FClass2d( face, toluv );

    switch ( asurface.GetType() )
    {
    case GeomAbs_Plane:
      _realProjector = new PlaneProjector( asurface.Plane(), face, classifier );
      break;
    case GeomAbs_Cylinder:
      _realProjector = new CylinderProjector( asurface.Cylinder(), face, classifier );
      break;
    case GeomAbs_Sphere:
      _realProjector = new SphereProjector( asurface.Sphere(), face, classifier );
      break;
    case GeomAbs_Cone:
      _realProjector = new ConeProjector( asurface.Cone(), face, classifier );
      break;
    case GeomAbs_Torus:
      _realProjector = new TorusProjector( asurface.Torus(), face, classifier );
      break;
    case GeomAbs_BezierSurface:
    case GeomAbs_BSplineSurface:
    case GeomAbs_SurfaceOfRevolution:
    case GeomAbs_SurfaceOfExtrusion:
    case GeomAbs_OffsetSurface:
    case GeomAbs_OtherSurface:
      GeomLib_IsPlanarSurface isPlaneCheck( surface, tol );
      if ( isPlaneCheck.IsPlanar() )
      {
        _realProjector = new PlaneProjector( isPlaneCheck.Plan(), face, classifier,
                                             /*isRealPlane=*/false);
      }
      else
      {
        _realProjector = new SurfaceProjector( face, tol, classifier );
      }
      break;
    }

    if ( !_realProjector )
      delete classifier;
  }
}

//================================================================================
/*!
 * \brief Return true if projection is not needed
 */
//================================================================================

bool FT_Projector::isPlanarBoundary() const
{
  return ( dynamic_cast< LineProjector*  >( _realProjector ) ||
           dynamic_cast< PlaneProjector* >( _realProjector ) );
}

//================================================================================
/*!
 * \brief Check if a point lies on the boundary shape
 *  \param [in] point - the point to check
 *  \param [in] tol2 - a square tolerance allowing to decide whether a point is on the shape
 *  \param [in] newSolution - position on the shape (U or UV) of the point found
 *         during projecting
 *  \param [in] prevSolution - position on the shape (U or UV) of a neighbor point
 *  \return bool - \c true if the point lies on the boundary shape
 *
 * This method is used to select a shape by checking if all neighbor nodes of a node to move
 * lie on a shape.
 */
//================================================================================

bool FT_Projector::isOnShape( const gp_Pnt& point,
                              const double  tol2,
                              double*       newSolution,
                              const double* prevSolution)
{
  if ( _bndBox.IsOut( point ) || !_realProjector )
    return false;

  gp_Pnt proj;
  if ( isPlanarBoundary() )
    return projectAndClassify( point, tol2, proj, newSolution, prevSolution );

  return project( point, tol2, proj, newSolution, prevSolution );
}

//================================================================================
/*!
 * \brief Project a point to the boundary shape
 *  \param [in] point - the point to project
 *  \param [in] maxDist2 - the maximal square distance between the point and the projection
 *  \param [out] projection - the projection
 *  \param [out] newSolution - position on the shape (U or UV) of the point found
 *         during projecting
 *  \param [in] prevSolution - already found position on the shape (U or UV) of a neighbor point
 *  \return bool - false if the distance between the point and the projection
 *         is more than sqrt(maxDist2)
 *
 * This method is used to project a node in the case where only one shape is found by name
 */
//================================================================================

bool FT_Projector::project( const gp_Pnt& point,
                            const double  maxDist2,
                            gp_Pnt&       projection,
                            double*       newSolution,
                            const double* prevSolution)
{
  if ( !_realProjector )
    return false;

  _realProjector->_dist = 1e100;
  projection = _realProjector->project( point, newSolution, prevSolution );

  bool ok = ( _realProjector->_dist * _realProjector->_dist < maxDist2 );
  if ( !ok && _tryWOPrevSolution && prevSolution )
  {
    projection = _realProjector->project( point, newSolution );
    ok = ( _realProjector->_dist * _realProjector->_dist < maxDist2 );
  }
  return ok;
}

//================================================================================
/*!
 * \brief Project a point to the boundary shape and check if the projection lies within
 *        the shape boundary
 *  \param [in] point - the point to project
 *  \param [in] maxDist2 - the maximal square distance between the point and the projection
 *  \param [out] projection - the projection
 *  \param [out] newSolution - position on the shape (U or UV) of the point found
 *         during projecting
 *  \param [in] prevSolution - already found position on the shape (U or UV) of a neighbor point
 *  \return bool - false if the projection point lies out of the shape boundary or
 *          the distance between the point and the projection is more than sqrt(maxDist2)
 *
 * This method is used to project a node in the case where several shapes are selected for
 * projection of a node group
 */
//================================================================================

bool FT_Projector::projectAndClassify( const gp_Pnt& point,
                                       const double  maxDist2,
                                       gp_Pnt&       projection,
                                       double*       newSolution,
                                       const double* prevSolution)
{
  if ( _bndBox.IsOut( point ) || !_realProjector )
    return false;

  bool ok = _realProjector->projectAndClassify( point, maxDist2, projection,
                                                newSolution, prevSolution );
  if ( !ok && _tryWOPrevSolution && prevSolution )
    ok = _realProjector->projectAndClassify( point, maxDist2, projection, newSolution );

  return ok;
}

//================================================================================
/*!
 * \brief Return true if a previously found solution can be used to speed up the projection
 */
//================================================================================

bool FT_Projector::canUsePrevSolution() const
{
  return ( _realProjector && _realProjector->canUsePrevSolution() );
}
