// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_Filter_i.hxx
//  Author : Alexey Petrov, OCC
//  Module : SMESH

#ifndef _SMESH_FILTER_I_HXX_
#define _SMESH_FILTER_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

#include <LDOM_Document.hxx>
#include <TopoDS_Shape.hxx>

#include "SALOME_GenericObj_i.hh"
#include "SMESH_ControlsDef.hxx"

#include <list>

class SMESH_GroupBase_i;


namespace SMESH
{
  /*!
   * \brief Object notified on change of base objects and
   *        notifying dependent objects in its turn.
   *        This interface is used to track the following dependencies:
   *        - GroupOnFiler depending on Filter predicates
   *        - Filter depending on a Group via FT_BelongToMeshGroup predicate 
   */
  struct NotifyerAndWaiter
  {
    virtual void OnBaseObjModified(NotifyerAndWaiter* obj, bool removed) {};
    // specific reaction on modification of a base object

    void Modified( bool removed=false, NotifyerAndWaiter* who = 0);
    // calls OnBaseObjModified(), if who != 0, and myWaiters[i]->Modified(who)

    void AddModifWaiter    ( NotifyerAndWaiter* waiter ); // adds a dependent object to notify
    void RemoveModifWaiter ( NotifyerAndWaiter* waiter ); // CALL IT when a waiter dies!!!
    bool ContainModifWaiter( NotifyerAndWaiter* waiter );
    std::list<NotifyerAndWaiter*> myWaiters;
  };
  // ================================================================================
  /*
    FUNCTORS
  */
  
  /*
    Class       : Functor_i
    Description : An abstact class for all functors 
  */
  class SMESH_I_EXPORT Functor_i: public virtual POA_SMESH::Functor,
                                  public virtual SALOME::GenericObj_i
  {
  public:
    virtual void                    SetMesh( SMESH_Mesh_ptr theMesh );
    Controls::FunctorPtr            GetFunctor() { return myFunctorPtr; }
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
  class SMESH_I_EXPORT NumericalFunctor_i: public virtual POA_SMESH::NumericalFunctor,
                                           public virtual Functor_i
  {
  public:
    CORBA::Double                   GetValue( CORBA::Long theElementId );
    SMESH::Histogram*               GetHistogram(CORBA::Short   nbIntervals,
                                                 CORBA::Boolean isLogarithmic);
    SMESH::Histogram*               GetLocalHistogram(CORBA::Short               nbIntervals,
                                                      CORBA::Boolean            isLogarithmic,
                                                      SMESH::SMESH_IDSource_ptr object);
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
  class SMESH_I_EXPORT MinimumAngle_i: public virtual POA_SMESH::MinimumAngle,
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
  class SMESH_I_EXPORT AspectRatio_i: public virtual POA_SMESH::AspectRatio,
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
  class SMESH_I_EXPORT AspectRatio3D_i: public virtual POA_SMESH::AspectRatio3D,
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
  class SMESH_I_EXPORT Warping_i: public virtual POA_SMESH::Warping,
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
  class SMESH_I_EXPORT Taper_i: public virtual POA_SMESH::Taper,
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
  class SMESH_I_EXPORT Skew_i: public virtual POA_SMESH::Skew,
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
  class SMESH_I_EXPORT Area_i: public virtual POA_SMESH::Area,
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
  class SMESH_I_EXPORT Volume3D_i: public virtual POA_SMESH::Volume3D,
                                   public virtual NumericalFunctor_i
  {
  public:
    Volume3D_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : MaxElementLength2D_i
    Description : Functor for calculating maximum length of 2D element
  */
  class SMESH_I_EXPORT MaxElementLength2D_i: public virtual POA_SMESH::MaxElementLength2D,
                                             public virtual NumericalFunctor_i
  {
  public:
    MaxElementLength2D_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : MaxElementLength3D_i
    Description : Functor for calculating maximum length of 3D element
  */
  class SMESH_I_EXPORT MaxElementLength3D_i: public virtual POA_SMESH::MaxElementLength3D,
                                             public virtual NumericalFunctor_i
  {
  public:
    MaxElementLength3D_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : Length_i
    Description : Functor for calculating length of edge
  */
  class SMESH_I_EXPORT Length_i: public virtual POA_SMESH::Length,
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
  class SMESH_I_EXPORT Length2D_i: public virtual POA_SMESH::Length2D,
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
  class SMESH_I_EXPORT MultiConnection_i: public virtual POA_SMESH::MultiConnection,
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
  class SMESH_I_EXPORT MultiConnection2D_i: public virtual POA_SMESH::MultiConnection2D,
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
    Class       : BallDiameter_i
    Description : Functor returning diameter of a ball element
  */
  class SMESH_I_EXPORT BallDiameter_i: public virtual POA_SMESH::BallDiameter,
                                       public virtual NumericalFunctor_i
  {
  public:
    BallDiameter_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : NodeConnectivityNumber_i
    Description : Functor returning diameter of a ball element
  */
  class SMESH_I_EXPORT NodeConnectivityNumber_i: public virtual POA_SMESH::NodeConnectivityNumber,
                                       public virtual NumericalFunctor_i
  {
  public:
    NodeConnectivityNumber_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    PREDICATES
  */
  /*
    Class       : Predicate_i
    Description : Base class for all predicates
  */
  class SMESH_I_EXPORT Predicate_i: public virtual POA_SMESH::Predicate,
                                    public virtual Functor_i
  {
  public:
    CORBA::Boolean                  IsSatisfy( CORBA::Long theElementId );
    CORBA::Long                     NbSatisfying( SMESH::SMESH_IDSource_ptr obj );
    Controls::PredicatePtr          GetPredicate();
    
  protected:
    Controls::PredicatePtr          myPredicatePtr;
  };
  
  
  /*
    Class       : BadOrientedVolume_i
    Description : Verify whether a mesh volume is incorrectly oriented from
    the point of view of MED convention
  */
  class SMESH_I_EXPORT BadOrientedVolume_i: public virtual POA_SMESH::BadOrientedVolume,
                                            public virtual Predicate_i
  {
  public:
    BadOrientedVolume_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : BareBorderVolume_i
    Description : Verify whether a mesh volume has a free facet without a face on it
  */
  class SMESH_I_EXPORT BareBorderVolume_i: public virtual POA_SMESH::BareBorderVolume,
                                           public virtual Predicate_i
  {
  public:
    BareBorderVolume_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : BareBorderFace_i
    Description : Verify whether a mesh face has a free border without an edge on it
  */
  class SMESH_I_EXPORT BareBorderFace_i: public virtual POA_SMESH::BareBorderFace,
                                         public virtual Predicate_i
  {
  public:
    BareBorderFace_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : OverConstrainedVolume_i
    Description : Verify whether a mesh volume has only one facet shared with other volumes
  */
  class SMESH_I_EXPORT OverConstrainedVolume_i: public virtual POA_SMESH::OverConstrainedVolume,
                                                public virtual Predicate_i
  {
  public:
    OverConstrainedVolume_i();
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : OverConstrainedFace_i
    Description : Verify whether a mesh face has only one border shared with other faces
  */
  class SMESH_I_EXPORT OverConstrainedFace_i: public virtual POA_SMESH::OverConstrainedFace,
                                              public virtual Predicate_i
  {
  public:
    OverConstrainedFace_i();
    FunctorType                     GetFunctorType();
  };

  /*
    Class       : BelongToMeshGroup_i
    Description : Verify whether a mesh element is included into a mesh group
  */
  class SMESH_I_EXPORT BelongToMeshGroup_i: public virtual POA_SMESH::BelongToMeshGroup,
                                            public virtual Predicate_i
  {
    std::string                    myID; // IOR or StoreName
    SMESH::SMESH_GroupBase_var     myGroup;
    Controls::BelongToMeshGroupPtr myBelongToMeshGroup;
  public:
    BelongToMeshGroup_i();
    ~BelongToMeshGroup_i();
    void                       SetGroup( SMESH::SMESH_GroupBase_ptr theGroup );
    void                       SetGroupID( const char* theID ); // IOR or StoreName
    SMESH::SMESH_GroupBase_ptr GetGroup();

    std::string                GetGroupID();
    FunctorType                GetFunctorType();
    //virtual void               SetMesh( SMESH_Mesh_ptr theMesh );
  };

  /*
    Class       : BelongToGeom_i
    Description : Predicate for selection on geometrical support
  */
  class SMESH_I_EXPORT BelongToGeom_i: public virtual POA_SMESH::BelongToGeom,
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

    void                            SetTolerance( CORBA::Double );
    CORBA::Double                   GetTolerance();
    
  protected:
    Controls::BelongToGeomPtr       myBelongToGeomPtr;
    char*                           myShapeName;
    char*                           myShapeID;
  };
  
  /*
    Class       : BelongToSurface_i
    Description : Verify whether mesh element lie in pointed Geom planar object
  */
  class SMESH_I_EXPORT BelongToSurface_i: public virtual POA_SMESH::BelongToSurface,
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
    
    void                            SetUseBoundaries( CORBA::Boolean theUseBndRestrictions );
    CORBA::Boolean                  GetUseBoundaries();

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
  class SMESH_I_EXPORT BelongToPlane_i: public virtual POA_SMESH::BelongToPlane,
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
  class SMESH_I_EXPORT BelongToCylinder_i: public virtual POA_SMESH::BelongToCylinder,
                                           public virtual BelongToSurface_i
  {
  public:
    BelongToCylinder_i();
    void                            SetCylinder( GEOM::GEOM_Object_ptr theGeom, ElementType theType );
    FunctorType                     GetFunctorType();
  };

  /*
    Class       : BelongToGenSurface_i
    Description : Verify whether mesh element lie on pointed Geom surfasic object
  */
  class BelongToGenSurface_i: public virtual POA_SMESH::BelongToGenSurface,
                              public virtual BelongToSurface_i
  {
  public:
    BelongToGenSurface_i();
    void                            SetSurface( GEOM::GEOM_Object_ptr theGeom, ElementType theType );
    FunctorType                     GetFunctorType();
  };
  
  /*
    Class       : LyingOnGeom_i
    Description : Predicate for selection on geometrical support(lying or partially lying)
  */
  class SMESH_I_EXPORT LyingOnGeom_i: public virtual POA_SMESH::LyingOnGeom,
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

    void                            SetTolerance( CORBA::Double );
    CORBA::Double                   GetTolerance();
    
  protected:
    Controls::LyingOnGeomPtr        myLyingOnGeomPtr;
    char*                           myShapeName;
    char*                           myShapeID;
  };
  
  /*
    Class       : FreeBorders_i
    Description : Predicate for free borders
  */
  class SMESH_I_EXPORT FreeBorders_i: public virtual POA_SMESH::FreeBorders,
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
  class SMESH_I_EXPORT FreeEdges_i: public virtual POA_SMESH::FreeEdges,
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
    Class       : FreeFaces_i
    Description : Predicate for free faces
  */
  class SMESH_I_EXPORT FreeFaces_i: public virtual POA_SMESH::FreeFaces,
                                    public virtual Predicate_i
  {
  public:
    FreeFaces_i();
    FunctorType                     GetFunctorType();
  };
  

  /*
    Class       : FreeNodes_i
    Description : Predicate for free nodes
  */
  class SMESH_I_EXPORT FreeNodes_i: public virtual POA_SMESH::FreeNodes,
                                    public virtual Predicate_i
  {
  public:
    FreeNodes_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : EqualNodes_i
    Description : Predicate for equal nodes
  */
  class SMESH_I_EXPORT EqualNodes_i: public virtual POA_SMESH::EqualNodes,
                                     public virtual Predicate_i
  {
  public:
    EqualNodes_i();
    FunctorType                     GetFunctorType();
    void                            SetTolerance( double );
    double                          GetTolerance();

  private:
    Controls::CoincidentNodesPtr myCoincidentNodesPtr;
  };
  /*
    Class       : EqualEdges_i
    Description : Predicate for equal edges
  */
  class SMESH_I_EXPORT EqualEdges_i: public virtual POA_SMESH::EqualEdges,
                                     public virtual Predicate_i
  {
  public:
    EqualEdges_i();
    FunctorType                     GetFunctorType();
  };
  /*
    Class       : EqualFaces_i
    Description : Predicate for equal Faces
  */
  class SMESH_I_EXPORT EqualFaces_i: public virtual POA_SMESH::EqualFaces,
                                     public virtual Predicate_i
  {
  public:
    EqualFaces_i();
    FunctorType                     GetFunctorType();
  };
  /*
    Class       : EqualVolumes_i
    Description : Predicate for equal Volumes
  */
  class SMESH_I_EXPORT EqualVolumes_i: public virtual POA_SMESH::EqualVolumes,
                                       public virtual Predicate_i
  {
  public:
    EqualVolumes_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    Class       : RangeOfIds_i
    Description : Predicate for Range of Ids
  */
  class SMESH_I_EXPORT RangeOfIds_i: public virtual POA_SMESH::RangeOfIds,
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
    Class       : LinearOrQuadratic_i
    Description : Verify whether a mesh element is linear
  */
  class SMESH_I_EXPORT LinearOrQuadratic_i: public virtual POA_SMESH::LinearOrQuadratic,
                                            public virtual Predicate_i
  {
  public:
    LinearOrQuadratic_i();
    FunctorType                    GetFunctorType();
    void                           SetElementType( ElementType theType );

  private:
   Controls::LinearOrQuadraticPtr  myLinearOrQuadraticPtr;
  };
  
  /*
    Class       : GroupColor_i
    Description : Functor for check color of group to whic mesh element belongs to
  */
  class SMESH_I_EXPORT GroupColor_i: public virtual POA_SMESH::GroupColor,
                                     public virtual Predicate_i
  {
  public:
    GroupColor_i();
    FunctorType             GetFunctorType();

    void                    SetElementType( ElementType theType );
    void                    SetColorStr( const char* theColor );
    char*                   GetColorStr();

  private:
    Controls::GroupColorPtr myGroupColorPtr;
  };
  
  /*
    Class       : ElemGeomType_i
    Description : Functor for check element geometry type
  */
  class SMESH_I_EXPORT ElemGeomType_i: public virtual POA_SMESH::ElemGeomType,
                                       public virtual Predicate_i
  {
  public:
    ElemGeomType_i();
    FunctorType             GetFunctorType();

    void                    SetElementType ( ElementType  theType );
    void                    SetGeometryType( GeometryType theType );
    GeometryType            GetGeometryType() const;

  private:
    Controls::ElemGeomTypePtr myElemGeomTypePtr;
  };

  /*
    Class       : ElemEntityType_i
    Description : Functor for check element entity type
  */
  class SMESH_I_EXPORT ElemEntityType_i: public virtual POA_SMESH::ElemEntityType,
                                         public virtual Predicate_i
  {
  public:
    ElemEntityType_i();
    FunctorType             GetFunctorType();

    void                    SetElementType ( ElementType  theType );
    void                    SetEntityType( EntityType theEntityType );
    EntityType              GetEntityType() const;

  private:
    Controls::ElemEntityTypePtr myElemEntityTypePtr;
  };
  
  /*
    Class       : CoplanarFaces_i
    Description : Returns true if a mesh face is a coplanar neighbour to a given one
  */
  class SMESH_I_EXPORT CoplanarFaces_i: public virtual POA_SMESH::CoplanarFaces,
                                        public virtual Predicate_i
  {
  public:
    CoplanarFaces_i();
    FunctorType             GetFunctorType();

    void                    SetFace ( CORBA::Long theFaceID );
    void                    SetTolerance( CORBA::Double theToler );
    char*                   GetFaceAsString () const;
    CORBA::Long             GetFace () const;
    CORBA::Double           GetTolerance () const;
  private:
    Controls::CoplanarFacesPtr myCoplanarFacesPtr;
  };

  /*
   * Class       : ConnectedElements_i
   * Description : Returns true if an element is connected via other elements to the element
   *               located at a given point.
   */
  class SMESH_I_EXPORT ConnectedElements_i: public virtual POA_SMESH::ConnectedElements,
                                            public virtual Predicate_i
  {
  public:
    ConnectedElements_i();
    FunctorType             GetFunctorType();

    void                    SetElementType( ElementType theType );
    void                    SetPoint( CORBA::Double x, CORBA::Double y, CORBA::Double z );
    void                    SetVertex( GEOM::GEOM_Object_ptr vertex )
      throw (SALOME::SALOME_Exception);
    void                    SetNode ( CORBA::Long nodeID )
      throw (SALOME::SALOME_Exception);
    void                    SetThreshold ( const char* threshold,
                                           SMESH::ConnectedElements::ThresholdType type )
      throw (SALOME::SALOME_Exception);
    char*                   GetThreshold ( SMESH::ConnectedElements::ThresholdType& type );

  private:
    Controls::ConnectedElementsPtr          myConnectedElementsPtr;
    std::string                             myVertexID;
  };
  
  /*
    Class       : Comparator_i
    Description : Base class for comparators
  */
  class SMESH_I_EXPORT Comparator_i: public virtual POA_SMESH::Comparator,
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
  class SMESH_I_EXPORT LessThan_i: public virtual POA_SMESH::LessThan,
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
  class SMESH_I_EXPORT MoreThan_i: public virtual POA_SMESH::MoreThan,
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
  class SMESH_I_EXPORT EqualTo_i: public virtual POA_SMESH::EqualTo,
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
  class SMESH_I_EXPORT LogicalNOT_i: public virtual POA_SMESH::LogicalNOT,
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
  class SMESH_I_EXPORT LogicalBinary_i: public virtual POA_SMESH::LogicalBinary,
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
  class SMESH_I_EXPORT LogicalAND_i: public virtual POA_SMESH::LogicalAND,
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
  class SMESH_I_EXPORT LogicalOR_i: public virtual POA_SMESH::LogicalOR,
                                    public virtual LogicalBinary_i
  {
  public:
    LogicalOR_i();
    FunctorType                     GetFunctorType();
  };
  
  
  /*
    FILTER
  */
  class SMESH_I_EXPORT Filter_i: public virtual POA_SMESH::Filter,
                                 public virtual SALOME::GenericObj_i,
                                 public NotifyerAndWaiter
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

    void FindBaseObjects();
    // finds groups it depends on

    virtual void OnBaseObjModified(NotifyerAndWaiter* group, bool removed);
    // notified on change of myBaseGroups[i]

    // =========================
    // SMESH_IDSource interface
    // =========================
    virtual SMESH::long_array*           GetIDs();
    virtual SMESH::long_array*           GetMeshInfo();
    virtual SMESH::long_array*           GetNbElementsByType();
    virtual SMESH::array_of_ElementType* GetTypes();
    virtual SMESH::SMESH_Mesh_ptr        GetMesh();
    virtual bool                         IsMeshInfoCorrect() { return true; }
    virtual SALOMEDS::TMPFile*           GetVtkUgStream();

  private:
    Controls::Filter myFilter;
    Predicate_i*     myPredicate;
    SMESH_Mesh_var   myMesh;

    std::vector< SMESH_GroupBase_i* > myBaseGroups;
  };


  /*
    FILTER LIBRARY
  */
  class SMESH_I_EXPORT FilterLibrary_i: public virtual POA_SMESH::FilterLibrary,
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
  
  class SMESH_I_EXPORT FilterManager_i: public virtual POA_SMESH::FilterManager,
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
    MaxElementLength2D_ptr    CreateMaxElementLength2D();
    MaxElementLength3D_ptr    CreateMaxElementLength3D();
    Length_ptr                CreateLength();
    Length2D_ptr              CreateLength2D();
    NodeConnectivityNumber_ptr CreateNodeConnectivityNumber();
    MultiConnection_ptr       CreateMultiConnection();
    MultiConnection2D_ptr     CreateMultiConnection2D();
    BallDiameter_ptr          CreateBallDiameter();
    
    BelongToMeshGroup_ptr     CreateBelongToMeshGroup();
    BelongToGeom_ptr          CreateBelongToGeom();
    BelongToPlane_ptr         CreateBelongToPlane();
    BelongToCylinder_ptr      CreateBelongToCylinder();
    BelongToGenSurface_ptr    CreateBelongToGenSurface();
    
    LyingOnGeom_ptr           CreateLyingOnGeom();
    
    FreeBorders_ptr           CreateFreeBorders();
    FreeEdges_ptr             CreateFreeEdges();
    FreeNodes_ptr             CreateFreeNodes();
    FreeFaces_ptr             CreateFreeFaces();

    EqualNodes_ptr            CreateEqualNodes();
    EqualEdges_ptr            CreateEqualEdges();
    EqualFaces_ptr            CreateEqualFaces();
    EqualVolumes_ptr          CreateEqualVolumes();

    RangeOfIds_ptr            CreateRangeOfIds();
    BadOrientedVolume_ptr     CreateBadOrientedVolume();
    BareBorderFace_ptr        CreateBareBorderFace();
    BareBorderVolume_ptr      CreateBareBorderVolume();
    OverConstrainedFace_ptr   CreateOverConstrainedFace();
    OverConstrainedVolume_ptr CreateOverConstrainedVolume();
    LinearOrQuadratic_ptr     CreateLinearOrQuadratic();
    GroupColor_ptr            CreateGroupColor();
    ElemGeomType_ptr          CreateElemGeomType();
    ElemEntityType_ptr        CreateElemEntityType();
    CoplanarFaces_ptr         CreateCoplanarFaces();
    ConnectedElements_ptr     CreateConnectedElements();

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

  const char*        FunctorTypeToString(SMESH::FunctorType ft);
  SMESH::FunctorType StringToFunctorType(const char*       str);
}


#endif
