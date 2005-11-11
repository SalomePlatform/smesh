//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_Filter_i.hxx
//  Author : Alexey Petrov, OCC
//  Module : SMESH

#ifndef _SMESH_FILTER_I_HXX_
#define _SMESH_FILTER_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

#include <LDOM_Document.hxx>
#include <TopoDS_Shape.hxx>

#include "SALOME_GenericObj_i.hh"
#include "SMESH_ControlsDef.hxx"

class SMESHDS_Mesh;

namespace SMESH
{

  namespace Controls
  {
    
    /*
      Class       : BelongToGeom
      Description : Predicate for verifying whether entiy belong to
      specified geometrical support
    */
    class BelongToGeom: public virtual Predicate
    {
    public:
      BelongToGeom();
      
      virtual void                    SetMesh( const SMDS_Mesh* theMesh );
      virtual void                    SetGeom( const TopoDS_Shape& theShape );
      
      virtual bool                    IsSatisfy( long theElementId );
      
      virtual void                    SetType( SMDSAbs_ElementType theType );
      virtual                         SMDSAbs_ElementType GetType() const;
      
      TopoDS_Shape                    GetShape();
      const SMESHDS_Mesh*             GetMeshDS() const;
      
    private:
      TopoDS_Shape                    myShape;
      const SMESHDS_Mesh*             myMeshDS;
      SMDSAbs_ElementType             myType;
    };
    typedef boost::shared_ptr<BelongToGeom> BelongToGeomPtr;
    
    /*
      Class       : LyingOnGeom
      Description : Predicate for verifying whether entiy lying or partially lying on
      specified geometrical support
    */
    class LyingOnGeom: public virtual Predicate
    {
    public:
      LyingOnGeom();
      
      virtual void                    SetMesh( const SMDS_Mesh* theMesh );
      virtual void                    SetGeom( const TopoDS_Shape& theShape );
      
      virtual bool                    IsSatisfy( long theElementId );
      
      virtual void                    SetType( SMDSAbs_ElementType theType );
      virtual                         SMDSAbs_ElementType GetType() const;
      
      TopoDS_Shape                    GetShape();
      const SMESHDS_Mesh*             GetMeshDS() const;
      
      virtual bool                    Contains( const SMESHDS_Mesh*     theMeshDS,
						const TopoDS_Shape&     theShape,
						const SMDS_MeshElement* theElem,
						TopAbs_ShapeEnum        theFindShapeEnum,
						TopAbs_ShapeEnum        theAvoidShapeEnum = TopAbs_SHAPE );
    private:
      TopoDS_Shape                    myShape;
      const SMESHDS_Mesh*             myMeshDS;
      SMDSAbs_ElementType             myType;
    };
    typedef boost::shared_ptr<LyingOnGeom> LyingOnGeomPtr;
  }
  
  /*
    FUNCTORS
  */
  
  /*
    Class       : Functor_i
    Description : An abstact class for all functors 
  */
  class Functor_i: public virtual POA_SMESH::Functor,
		   public virtual SALOME::GenericObj_i
  {
  public:
    void                            SetMesh( SMESH_Mesh_ptr theMesh );
    Controls::FunctorPtr            GetFunctor(){ return myFunctorPtr;}
    ElementType                     GetElementType();
    
  protected:
    Functor_i();
    ~Functor_i();
  protected:                                
    Controls::FunctorPtr            myFunctorPtr;
  };
  
  /*
    Class       : NumericalFunctor_i
    Description : Base class for numerical functors 
  */
  class NumericalFunctor_i: public virtual POA_SMESH::NumericalFunctor,
			    public virtual Functor_i
  {
  public:
    CORBA::Double                   GetValue( CORBA::Long theElementId );
    void                            SetPrecision( CORBA::Long thePrecision );
    CORBA::Long                     GetPrecision();
    Controls::NumericalFunctorPtr   GetNumericalFunctor();
    
  protected:
    Controls::NumericalFunctorPtr   myNumericalFunctorPtr;
  };
  
  
  /*
    Class       : SMESH_MinimumAngleFunct
    Description : Functor for calculation of minimum angle
  */
  class MinimumAngle_i: public virtual POA_SMESH::MinimumAngle,
			public virtual NumericalFunctor_i
  {
  public:
    MinimumAngle_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : AspectRatio_i
    Description : Functor for calculating aspect ratio
  */
  class AspectRatio_i: public virtual POA_SMESH::AspectRatio,
		       public virtual NumericalFunctor_i
  {
  public:
    AspectRatio_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : AspectRatio3D_i
    Description : Functor for calculating aspect ratio for 3D
  */
  class AspectRatio3D_i: public virtual POA_SMESH::AspectRatio3D,
			 public virtual NumericalFunctor_i
  {
  public:
    AspectRatio3D_i();
    FunctorType                     GetFunctorType();
  };
  

  /*
    Class       : Warping_i
    Description : Functor for calculating warping
  */
  class Warping_i: public virtual POA_SMESH::Warping,
		   public virtual NumericalFunctor_i
  {
  public:
    Warping_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : Taper_i
    Description : Functor for calculating taper
  */
  class Taper_i: public virtual POA_SMESH::Taper,
		 public virtual NumericalFunctor_i
  {
  public:
    Taper_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : Skew_i
    Description : Functor for calculating skew in degrees
  */
  class Skew_i: public virtual POA_SMESH::Skew,
		public virtual NumericalFunctor_i
  {
  public:
    Skew_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : Area_i
    Description : Functor for calculating area
  */
  class Area_i: public virtual POA_SMESH::Area,
		public virtual NumericalFunctor_i
  {
  public:
    Area_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : Volume3D_i
    Description : Functor for calculating volume of 3D element
  */
  class Volume3D_i: public virtual POA_SMESH::Volume3D,
                    public virtual NumericalFunctor_i
  {
  public:
    Volume3D_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : Length_i
    Description : Functor for calculating length of edge
  */
  class Length_i: public virtual POA_SMESH::Length,
		  public virtual NumericalFunctor_i
  {
  public:
    Length_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : Length2D_i
    Description : Functor for calculating length of edge
  */
  class Length2D_i: public virtual POA_SMESH::Length2D,
		    public virtual NumericalFunctor_i
  {
  public:
    Length2D_i();
    SMESH::Length2D::Values*        GetValues();
    FunctorType                     GetFunctorType();
    
  protected:
    Controls::Length2DPtr          myLength2DPtr;
  };
  
  
  /*
    Class       : MultiConnection_i
    Description : Functor for calculating number of faces conneted to the edge
  */
  class MultiConnection_i: public virtual POA_SMESH::MultiConnection,
			   public virtual NumericalFunctor_i
  {
  public:
    MultiConnection_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : MultiConnection2D_i
    Description : Functor for calculating number of faces conneted to the edge
  */
  class MultiConnection2D_i: public virtual POA_SMESH::MultiConnection2D,
			     public virtual NumericalFunctor_i
  {
  public:
    MultiConnection2D_i();
    SMESH::MultiConnection2D::Values*  GetValues();
    FunctorType                        GetFunctorType();
    
  protected:
    Controls::MultiConnection2DPtr     myMulticonnection2DPtr;
  };
  
  
  /*
    PREDICATES
  */
  /*
    Class       : Predicate_i
    Description : Base class for all predicates
  */
  class Predicate_i: public virtual POA_SMESH::Predicate,
		     public virtual Functor_i
  {
  public:
    CORBA::Boolean                  IsSatisfy( CORBA::Long theElementId );
    Controls::PredicatePtr          GetPredicate();
    
  protected:
    Controls::PredicatePtr          myPredicatePtr;
  };
  
  
  /*
    Class       : BadOrientedVolume_i
    Description : Verify whether a mesh volume is incorrectly oriented from
    the point of view of MED convention
  */
  class BadOrientedVolume_i: public virtual POA_SMESH::BadOrientedVolume,
			     public virtual Predicate_i
  {
  public:
    BadOrientedVolume_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : BelongToGeom_i
    Description : Predicate for selection on geometrical support
  */
  class BelongToGeom_i: public virtual POA_SMESH::BelongToGeom,
			public virtual Predicate_i
  {
  public:
    BelongToGeom_i();
    virtual                         ~BelongToGeom_i();
    
    void                            SetGeom( GEOM::GEOM_Object_ptr theGeom );
    void                            SetElementType( ElementType theType );
    FunctorType                     GetFunctorType();
    
    void                            SetGeom( const TopoDS_Shape& theShape );
    
    void                            SetShapeName( const char* theName );
    void                            SetShape( const char* theID, const char* theName );
    char*                           GetShapeName();
    char*                           GetShapeID();
    
  protected:
    Controls::BelongToGeomPtr       myBelongToGeomPtr;
    char*                           myShapeName;
    char*                           myShapeID;
  };
  
  /*
    Class       : BelongToSurface_i
    Description : Verify whether mesh element lie in pointed Geom planar object
  */
  class BelongToSurface_i: public virtual POA_SMESH::BelongToSurface,
			   public virtual Predicate_i
  {
  public:
    BelongToSurface_i( const Handle(Standard_Type)& );
    virtual                         ~BelongToSurface_i();
    
    void                            SetSurface( GEOM::GEOM_Object_ptr theGeom, ElementType theType );
    
    void                            SetShapeName( const char* theName, ElementType theType );
    void                            SetShape( const char* theID, const char* theName, ElementType theType );
    char*                           GetShapeName();
    char*                           GetShapeID();

    void                            SetTolerance( CORBA::Double );
    CORBA::Double                   GetTolerance();
    
  protected:
    Controls::ElementsOnSurfacePtr  myElementsOnSurfacePtr;
    char*                           myShapeName;
    char*                           myShapeID;
    Handle(Standard_Type)           mySurfaceType;
  };
  
  /*
    Class       : BelongToPlane_i
    Description : Verify whether mesh element lie in pointed Geom planar object
  */
  class BelongToPlane_i: public virtual POA_SMESH::BelongToPlane,
			 public virtual BelongToSurface_i
  {
  public:
    BelongToPlane_i();
    void                            SetPlane( GEOM::GEOM_Object_ptr theGeom, ElementType theType );
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : BelongToCylinder_i
    Description : Verify whether mesh element lie in pointed Geom cylindrical object
  */
  class BelongToCylinder_i: public virtual POA_SMESH::BelongToCylinder,
			    public virtual BelongToSurface_i
  {
  public:
    BelongToCylinder_i();
    void                            SetCylinder( GEOM::GEOM_Object_ptr theGeom, ElementType theType );
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : LyingOnGeom_i
    Description : Predicate for selection on geometrical support(lying or partially lying)
  */
  class LyingOnGeom_i: public virtual POA_SMESH::LyingOnGeom,
		       public virtual Predicate_i
  {
  public:
    LyingOnGeom_i();
    virtual                         ~LyingOnGeom_i();
    
    void                            SetGeom( GEOM::GEOM_Object_ptr theGeom );
    void                            SetElementType( ElementType theType );
    FunctorType                     GetFunctorType();
    
    void                            SetGeom( const TopoDS_Shape& theShape );
    
    void                            SetShapeName( const char* theName );
    void                            SetShape( const char* theID, const char* theName );
    char*                           GetShapeName();
    char*                           GetShapeID();
    
  protected:
    Controls::LyingOnGeomPtr        myLyingOnGeomPtr;
    char*                           myShapeName;
    char*                           myShapeID;
  };
  
  /*
    Class       : FreeBorders_i
    Description : Predicate for free borders
  */
  class FreeBorders_i: public virtual POA_SMESH::FreeBorders,
		       public virtual Predicate_i
  {
  public:
    FreeBorders_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : FreeEdges_i
    Description : Predicate for free edges
  */
  class FreeEdges_i: public virtual POA_SMESH::FreeEdges,
		     public virtual Predicate_i
  {
  public:
    FreeEdges_i();
    SMESH::FreeEdges::Borders*      GetBorders();
    FunctorType                     GetFunctorType();
    
  protected:
    Controls::FreeEdgesPtr          myFreeEdgesPtr;
  };
  
  
  /*
    Class       : RangeOfIds_i
    Description : Predicate for Range of Ids
  */
  class RangeOfIds_i: public virtual POA_SMESH::RangeOfIds,
		      public virtual Predicate_i
  {
  public:
    RangeOfIds_i();
    void                            SetRange( const SMESH::long_array& theIds );
    CORBA::Boolean                  SetRangeStr( const char* theRange );
    char*                           GetRangeStr();
    
    void                            SetElementType( ElementType theType );
    FunctorType                     GetFunctorType();
    
  protected:
    Controls::RangeOfIdsPtr         myRangeOfIdsPtr;
  };
  
  /*
    Class       : Comparator_i
    Description : Base class for comparators
  */
  class Comparator_i: public virtual POA_SMESH::Comparator,
		      public virtual Predicate_i
  {
  public:
    virtual                         ~Comparator_i();
    
    virtual void                    SetMargin( CORBA::Double );
    virtual void                    SetNumFunctor( NumericalFunctor_ptr );
    
    Controls::ComparatorPtr         GetComparator();
    NumericalFunctor_i*             GetNumFunctor_i();
    CORBA::Double                   GetMargin();
    
  protected:
    Comparator_i();
  protected:                                  
    Controls::ComparatorPtr         myComparatorPtr;
    NumericalFunctor_i*             myNumericalFunctor;
  };
  
  
  /*
    Class       : LessThan_i
    Description : Comparator "<"
  */
  class LessThan_i: public virtual POA_SMESH::LessThan,
		    public virtual Comparator_i
  {
  public:
    LessThan_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : MoreThan_i
    Description : Comparator ">"
  */
  class MoreThan_i: public virtual POA_SMESH::MoreThan,
		    public virtual Comparator_i
  {
  public:
    MoreThan_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : EqualTo_i
    Description : Comparator "="
  */
  class EqualTo_i: public virtual POA_SMESH::EqualTo,
		   public virtual Comparator_i
  {
  public:
    EqualTo_i();
    virtual void                    SetTolerance( CORBA::Double );
    CORBA::Double                   GetTolerance();
    FunctorType                     GetFunctorType();
    
  protected:
    Controls::EqualToPtr            myEqualToPtr;
  };
  
  
  /*
    Class       : LogicalNOT_i
    Description : Logical NOT predicate
  */
  class LogicalNOT_i: public virtual POA_SMESH::LogicalNOT,
		      public virtual Predicate_i
  {
  public:
    LogicalNOT_i();
  virtual                         ~LogicalNOT_i();
    
    virtual void                    SetPredicate( Predicate_ptr );
    Predicate_i*                    GetPredicate_i();
    FunctorType                     GetFunctorType();
    
  protected:
    Controls::LogicalNOTPtr         myLogicalNOTPtr;
    Predicate_i*                    myPredicate;
  };
  
  
  /*
    Class       : LogicalBinary_i
    Description : Base class for binary logical predicate
  */
  class LogicalBinary_i: public virtual POA_SMESH::LogicalBinary,
			 public virtual Predicate_i
  {
  public:
    virtual                         ~LogicalBinary_i();
    virtual void                    SetMesh( SMESH_Mesh_ptr theMesh );
    virtual void                    SetPredicate1( Predicate_ptr );
    virtual void                    SetPredicate2( Predicate_ptr );
    
    Controls::LogicalBinaryPtr      GetLogicalBinary();
    Predicate_i*                    GetPredicate1_i();
    Predicate_i*                    GetPredicate2_i();
    
  protected:
    LogicalBinary_i();
  protected:  
    Controls::LogicalBinaryPtr      myLogicalBinaryPtr;
    Predicate_i*                    myPredicate1;
    Predicate_i*                    myPredicate2;
  };
  
  
  /*
    Class       : LogicalAND_i
    Description : Logical AND
  */
  class LogicalAND_i: public virtual POA_SMESH::LogicalAND,
		      public virtual LogicalBinary_i
  {
  public:
    LogicalAND_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : LogicalOR_i
    Description : Logical OR
  */
  class LogicalOR_i: public virtual POA_SMESH::LogicalOR,
		     public virtual LogicalBinary_i
  {
  public:
    LogicalOR_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    FILTER
  */
  class Filter_i: public virtual POA_SMESH::Filter,
		  public virtual SALOME::GenericObj_i
  {
  public:
    Filter_i();
    ~Filter_i();
    
    virtual
    void
    SetPredicate( Predicate_ptr );

    virtual
    void
    SetMesh( SMESH_Mesh_ptr );

    virtual
    SMESH::long_array* 
    GetIDs();
    
    static
    void
    GetElementsId( Predicate_i*,
		   const SMDS_Mesh*,
		   Controls::Filter::TIdSequence& );
    static
    void           
    GetElementsId( Predicate_i*,
		   SMESH_Mesh_ptr,
		   Controls::Filter::TIdSequence& );
    
    virtual
    long_array*      
    GetElementsId( SMESH_Mesh_ptr );

    virtual
    ElementType      
    GetElementType();
    
    virtual
    CORBA::Boolean   
    GetCriteria( SMESH::Filter::Criteria_out theCriteria );

    virtual
    CORBA::Boolean
    SetCriteria( const SMESH::Filter::Criteria& theCriteria );
    
    virtual
    Predicate_ptr
    GetPredicate();

    Predicate_i*     GetPredicate_i();

  private:
    Controls::Filter myFilter;
    Predicate_i*     myPredicate;
    SMESH_Mesh_var   myMesh;
  };
  
  
  /*
    FILTER LIBRARY
  */
  class FilterLibrary_i: public virtual POA_SMESH::FilterLibrary,
			 public virtual SALOME::GenericObj_i
  {
  public:
    FilterLibrary_i( const char* theFileName );
    FilterLibrary_i();
    ~FilterLibrary_i();
    
    Filter_ptr              Copy( const char* theFilterName );
    
    CORBA::Boolean          Add     ( const char* theFilterName, Filter_ptr theFilter );
    CORBA::Boolean          AddEmpty( const char* theFilterName, ElementType theType );
    CORBA::Boolean          Delete  ( const char* theFilterName );
    CORBA::Boolean          Replace ( const char* theFilterName, 
				      const char* theNewName, 
				      Filter_ptr  theFilter );
    
    CORBA::Boolean          Save();
    CORBA::Boolean          SaveAs( const char* aFileName );
    
    CORBA::Boolean          IsPresent( const char* aFilterName );
    CORBA::Long             NbFilters( ElementType );
    string_array*           GetNames( ElementType );
    string_array*           GetAllNames();
    void                    SetFileName( const char* theFileName );
    char*                   GetFileName();
    
  private:
    char*                   myFileName;
    LDOM_Document           myDoc;
    FilterManager_var       myFilterMgr;
  };
  
  
  /*
    FILTER MANAGER
  */
  
  class FilterManager_i: public virtual POA_SMESH::FilterManager,
			 public virtual SALOME::GenericObj_i
  {
  public:
    FilterManager_i();
    ~FilterManager_i();

    MinimumAngle_ptr          CreateMinimumAngle();
    AspectRatio_ptr           CreateAspectRatio();
    AspectRatio3D_ptr         CreateAspectRatio3D();
    Warping_ptr               CreateWarping();
    Taper_ptr                 CreateTaper();
    Skew_ptr                  CreateSkew();
    Area_ptr                  CreateArea();
    Volume3D_ptr              CreateVolume3D();
    Length_ptr                CreateLength();
    Length2D_ptr              CreateLength2D();
    MultiConnection_ptr       CreateMultiConnection();
    MultiConnection2D_ptr     CreateMultiConnection2D();
    
    BelongToGeom_ptr          CreateBelongToGeom();
    BelongToPlane_ptr         CreateBelongToPlane();
    BelongToCylinder_ptr      CreateBelongToCylinder();
    
    LyingOnGeom_ptr           CreateLyingOnGeom();
    
    FreeBorders_ptr           CreateFreeBorders();
    FreeEdges_ptr             CreateFreeEdges();
    
    RangeOfIds_ptr            CreateRangeOfIds();
    
    BadOrientedVolume_ptr     CreateBadOrientedVolume();
    
    LessThan_ptr              CreateLessThan();
    MoreThan_ptr              CreateMoreThan();
    EqualTo_ptr               CreateEqualTo();
    
    LogicalNOT_ptr            CreateLogicalNOT();
    LogicalAND_ptr            CreateLogicalAND();
    LogicalOR_ptr             CreateLogicalOR();
    
    Filter_ptr                CreateFilter();
    
    FilterLibrary_ptr         LoadLibrary( const char* aFileName );
    FilterLibrary_ptr         CreateLibrary();
    CORBA::Boolean            DeleteLibrary( const char* aFileName );
  };
  
  
  Predicate_i* 
  GetPredicate( SMESH::Predicate_ptr thePredicate );
}


#endif
