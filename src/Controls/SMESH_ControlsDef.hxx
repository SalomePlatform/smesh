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

#ifndef _SMESH_CONTROLSDEF_HXX_
#define _SMESH_CONTROLSDEF_HXX_

#include <set>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <gp_XYZ.hxx>
#include <Geom_Surface.hxx>
#include <TColStd_SequenceOfInteger.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TCollection_AsciiString.hxx>

#include "SMDSAbs_ElementType.hxx"
#include "SMDS_MeshNode.hxx"

#include "SMESH_Controls.hxx"

class SMDS_MeshElement;
class SMDS_MeshFace;
class SMDS_MeshNode;
class SMDS_Mesh;

class SMESHDS_Mesh;
class SMESHDS_SubMesh;

class gp_Pnt;
class TopoDS_Shape;


namespace SMESH{
  namespace Controls{

    class TSequenceOfXYZ: public std::vector<gp_XYZ>
    {
    public:
      typedef std::vector<gp_XYZ> TSuperClass;
      TSequenceOfXYZ()
      {}

      TSequenceOfXYZ(size_type n):
	TSuperClass(n)
      {}

      TSequenceOfXYZ(size_type n, const value_type& t):
	TSuperClass(n,t)
      {}

      TSequenceOfXYZ(const TSequenceOfXYZ& theSequenceOfXYZ):
	TSuperClass(theSequenceOfXYZ)
      {}

      template <class InputIterator>
      TSequenceOfXYZ(InputIterator theBegin, InputIterator theEnd):
	TSuperClass(theBegin,theEnd)
      {}

      TSequenceOfXYZ& operator=(const TSequenceOfXYZ& theSequenceOfXYZ){
	TSuperClass::operator=(theSequenceOfXYZ);
	return *this;
      }

      reference operator()(size_type n){
	return TSuperClass::operator[](n-1);
      }

      const_reference operator()(size_type n) const{
	return TSuperClass::operator[](n-1);
      }

    private:
      reference operator[](size_type n);

      const_reference operator[](size_type n) const;
    };

    class Functor
    {
    public:
      ~Functor(){}
      virtual void SetMesh( const SMDS_Mesh* theMesh ) = 0;
      virtual SMDSAbs_ElementType GetType() const = 0;
    };

    class NumericalFunctor: public virtual Functor{
    public:
      NumericalFunctor();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual double GetValue( long theElementId );
      virtual double GetValue(const TSequenceOfXYZ& thePoints) { return -1.0;};
      virtual SMDSAbs_ElementType GetType() const = 0;
      virtual double GetBadRate( double Value, int nbNodes ) const = 0;
      long  GetPrecision() const;
      void  SetPrecision( const long thePrecision );
      
      bool GetPoints(const int theId, 
		     TSequenceOfXYZ& theRes) const;
      static bool GetPoints(const SMDS_MeshElement* theElem, 
			    TSequenceOfXYZ& theRes);
    protected:
      const SMDS_Mesh* myMesh;
      long       myPrecision;
    };
  
  
    /*
      Class       : SMESH_MinimumAngle
      Description : Functor for calculation of minimum angle
    */
    class MinimumAngle: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : AspectRatio
      Description : Functor for calculating aspect ratio
    */
    class AspectRatio: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : AspectRatio3D
      Description : Functor for calculating aspect ratio of 3D elems.
    */
    class AspectRatio3D: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : Warping
      Description : Functor for calculating warping
    */
    class Warping: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
      
    private:
      double ComputeA( const gp_XYZ&, const gp_XYZ&, const gp_XYZ&, const gp_XYZ& ) const;
    };
  
  
    /*
      Class       : Taper
      Description : Functor for calculating taper
    */
    class Taper: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
    
  
    /*
      Class       : Skew
      Description : Functor for calculating skew in degrees
    */
    class Skew: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
  
    
    /*
      Class       : Area
      Description : Functor for calculating area
    */
    class Area: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : Length
      Description : Functor for calculating length of edge
    */
    class Length: public virtual NumericalFunctor{
    public:
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
  
    /*
      Class       : Length2D
      Description : Functor for calculating length of edge
    */
    class Length2D: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
      struct Value{
	double myLength;
	long myPntId[2];
	Value(double theLength, long thePntId1, long thePntId2);
	bool operator<(const Value& x) const;
      };
      typedef std::set<Value> TValues;
      void GetValues(TValues& theValues);
      
    };
    typedef boost::shared_ptr<Length2D> Length2DPtr;

    /*
      Class       : MultiConnection
      Description : Functor for calculating number of faces conneted to the edge
    */
    class MultiConnection: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
    };
    
    /*
      Class       : MultiConnection2D
      Description : Functor for calculating number of faces conneted to the edge
    */
    class MultiConnection2D: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual double GetValue( const TSequenceOfXYZ& thePoints );
      virtual double GetBadRate( double Value, int nbNodes ) const;
      virtual SMDSAbs_ElementType GetType() const;
      struct Value{
	long myPntId[2];
	Value(long thePntId1, long thePntId2);
	bool operator<(const Value& x) const;
      };
      typedef std::map<Value,int> MValues;

      void GetValues(MValues& theValues);
    };
    typedef boost::shared_ptr<MultiConnection2D> MultiConnection2DPtr;
    /*
      PREDICATES
    */
    /*
      Class       : Predicate
      Description : Base class for all predicates
    */
    class Predicate: public virtual Functor{
    public:
      virtual bool IsSatisfy( long theElementId ) = 0;
      virtual SMDSAbs_ElementType GetType() const = 0;
    };
    
  
  
    /*
      Class       : FreeBorders
      Description : Predicate for free borders
    */
    class FreeBorders: public virtual Predicate{
    public:
      FreeBorders();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual bool IsSatisfy( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
            
    protected:
      const SMDS_Mesh* myMesh;
    };
   

    /*
      Class       : BadOrientedVolume
      Description : Predicate bad oriented volumes
    */
    class BadOrientedVolume: public virtual Predicate{
    public:
      BadOrientedVolume();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual bool IsSatisfy( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
            
    protected:
      const SMDS_Mesh* myMesh;
    };
   

    /*
      Class       : FreeEdges
      Description : Predicate for free Edges
    */
    class FreeEdges: public virtual Predicate{
    public:
      FreeEdges();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual bool IsSatisfy( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
      static bool IsFreeEdge( const SMDS_MeshNode** theNodes, const int theFaceId  );
      typedef long TElemId;
      struct Border{
	TElemId myElemId;
	TElemId myPntId[2];
	Border(long theElemId, long thePntId1, long thePntId2);
	bool operator<(const Border& x) const;
      };
      typedef std::set<Border> TBorders;
      void GetBoreders(TBorders& theBorders);
      
    protected:
      const SMDS_Mesh* myMesh;
    };
    typedef boost::shared_ptr<FreeEdges> FreeEdgesPtr;


    /*
      Class       : RangeOfIds
      Description : Predicate for Range of Ids.
                    Range may be specified with two ways.
                    1. Using AddToRange method
                    2. With SetRangeStr method. Parameter of this method is a string
                       like as "1,2,3,50-60,63,67,70-"
    */
    class RangeOfIds: public virtual Predicate
    {
    public:
                                    RangeOfIds();
      virtual void                  SetMesh( const SMDS_Mesh* theMesh );
      virtual bool                  IsSatisfy( long theNodeId );
      virtual SMDSAbs_ElementType   GetType() const;
      virtual void                  SetType( SMDSAbs_ElementType theType );

      bool                          AddToRange( long theEntityId );
      void                          GetRangeStr( TCollection_AsciiString& );
      bool                          SetRangeStr( const TCollection_AsciiString& );

    protected:
      const SMDS_Mesh*              myMesh;

      TColStd_SequenceOfInteger     myMin;
      TColStd_SequenceOfInteger     myMax;
      TColStd_MapOfInteger          myIds;

      SMDSAbs_ElementType           myType;
    };
    
    typedef boost::shared_ptr<RangeOfIds> RangeOfIdsPtr;
   
    
    /*
      Class       : Comparator
      Description : Base class for comparators
    */
    class Comparator: public virtual Predicate{
    public:
      Comparator();
      virtual ~Comparator();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual void SetMargin(double theValue);
      virtual void SetNumFunctor(NumericalFunctorPtr theFunct);
      virtual bool IsSatisfy( long theElementId ) = 0;
      virtual SMDSAbs_ElementType GetType() const;
      double  GetMargin();
  
    protected:
      double myMargin;
      NumericalFunctorPtr myFunctor;
    };
    typedef boost::shared_ptr<Comparator> ComparatorPtr;
  
  
    /*
      Class       : LessThan
      Description : Comparator "<"
    */
    class LessThan: public virtual Comparator{
    public:
      virtual bool IsSatisfy( long theElementId );
    };
  
  
    /*
      Class       : MoreThan
      Description : Comparator ">"
    */
    class MoreThan: public virtual Comparator{
    public:
      virtual bool IsSatisfy( long theElementId );
    };
  
  
    /*
      Class       : EqualTo
      Description : Comparator "="
    */
    class EqualTo: public virtual Comparator{
    public:
      EqualTo();
      virtual bool IsSatisfy( long theElementId );
      virtual void SetTolerance( double theTol );
      virtual double GetTolerance();
  
    private:
      double myToler;
    };
    typedef boost::shared_ptr<EqualTo> EqualToPtr;
  
    
    /*
      Class       : LogicalNOT
      Description : Logical NOT predicate
    */
    class LogicalNOT: public virtual Predicate{
    public:
      LogicalNOT();
      virtual ~LogicalNOT();
      virtual bool IsSatisfy( long theElementId );
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual void SetPredicate(PredicatePtr thePred);
      virtual SMDSAbs_ElementType GetType() const;
  
    private:
      PredicatePtr myPredicate;
    };
    typedef boost::shared_ptr<LogicalNOT> LogicalNOTPtr;
    
  
    /*
      Class       : LogicalBinary
      Description : Base class for binary logical predicate
    */
    class LogicalBinary: public virtual Predicate{
    public:
      LogicalBinary();
      virtual ~LogicalBinary();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual void SetPredicate1(PredicatePtr thePred);
      virtual void SetPredicate2(PredicatePtr thePred);
      virtual SMDSAbs_ElementType GetType() const;
  
    protected:
      PredicatePtr myPredicate1;
      PredicatePtr myPredicate2;
    };
    typedef boost::shared_ptr<LogicalBinary> LogicalBinaryPtr;
  
  
    /*
      Class       : LogicalAND
      Description : Logical AND
    */
    class LogicalAND: public virtual LogicalBinary{
    public:
      virtual bool IsSatisfy( long theElementId );
    };
  
  
    /*
      Class       : LogicalOR
      Description : Logical OR
    */
    class LogicalOR: public virtual LogicalBinary{
    public:
      virtual bool IsSatisfy( long theElementId );
    };
  
  
    /*
      Class       : ManifoldPart
      Description : Predicate for manifold part of mesh
    */
    class ManifoldPart: public virtual Predicate{
    public:

      /* internal class for algorithm uses */
      class Link
      {
      public:
        Link( SMDS_MeshNode* theNode1,
              SMDS_MeshNode* theNode2 );
        ~Link();
        
        bool IsEqual( const ManifoldPart::Link& theLink ) const;
        bool operator<(const ManifoldPart::Link& x) const;
        
        SMDS_MeshNode* myNode1;
        SMDS_MeshNode* myNode2;
      };

      bool IsEqual( const ManifoldPart::Link& theLink1,
                    const ManifoldPart::Link& theLink2 );
      
      typedef std::set<ManifoldPart::Link>                TMapOfLink;
      typedef std::vector<SMDS_MeshFace*>                 TVectorOfFacePtr;
      typedef std::vector<ManifoldPart::Link>             TVectorOfLink;
      typedef std::map<SMDS_MeshFace*,int>                TDataMapFacePtrInt;
      typedef std::map<ManifoldPart::Link,SMDS_MeshFace*> TDataMapOfLinkFacePtr;
      
      ManifoldPart();
      ~ManifoldPart();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      // inoke when all parameters already set
      virtual bool IsSatisfy( long theElementId );
      virtual      SMDSAbs_ElementType GetType() const;

      void    SetAngleTolerance( const double theAngToler );
      double  GetAngleTolerance() const;
      void    SetIsOnlyManifold( const bool theIsOnly );
      void    SetStartElem( const long  theStartElemId );

    private:
      bool    process();
      bool    findConnected( const TDataMapFacePtrInt& theAllFacePtrInt,
                             SMDS_MeshFace*            theStartFace,
                             TMapOfLink&               theNonManifold,
                             TColStd_MapOfInteger&     theResFaces );
      bool    isInPlane( const SMDS_MeshFace* theFace1,
                          const SMDS_MeshFace* theFace2 );
      void    expandBoundary( TMapOfLink&            theMapOfBoundary,
                              TVectorOfLink&         theSeqOfBoundary,
                              TDataMapOfLinkFacePtr& theDMapLinkFacePtr,
                              TMapOfLink&            theNonManifold,
                              SMDS_MeshFace*         theNextFace ) const;

     void     getFacesByLink( const Link& theLink,
                              TVectorOfFacePtr& theFaces ) const;

    private:
      const SMDS_Mesh*      myMesh;
      TColStd_MapOfInteger  myMapIds;
      TColStd_MapOfInteger  myMapBadGeomIds;
      TVectorOfFacePtr      myAllFacePtr;
      TDataMapFacePtrInt    myAllFacePtrIntDMap;
      double                myAngToler;
      bool                  myIsOnlyManifold;
      long                  myStartElemId;

    };
    typedef boost::shared_ptr<ManifoldPart> ManifoldPartPtr;
                         

    /*
      Class       : ElementsOnSurface
      Description : Predicate elements that lying on indicated surface
                    (plane or cylinder)
    */
    class ElementsOnSurface : public virtual Predicate {
    public:
      ElementsOnSurface();
      ~ElementsOnSurface();
      virtual void SetMesh( const SMDS_Mesh* theMesh );
      virtual bool IsSatisfy( long theElementId );
      virtual      SMDSAbs_ElementType GetType() const;

      void    SetTolerance( const double theToler );
      double  GetTolerance() const;
      void    SetSurface( const TopoDS_Shape& theShape,
                          const SMDSAbs_ElementType theType );

    private:
      void    process();
      void    process( const SMDS_MeshElement* theElem  );
      bool    isOnSurface( const SMDS_MeshNode* theNode ) const;

    private:
      const SMDS_Mesh*      myMesh;
      TColStd_MapOfInteger  myIds;
      SMDSAbs_ElementType   myType;
      Handle(Geom_Surface)  mySurf;
      double                myToler;
    };
    
    typedef boost::shared_ptr<ElementsOnSurface> ElementsOnSurfacePtr;
      

    /*
      FILTER
    */
    class Filter{
    public:
      Filter();
      virtual ~Filter();
      virtual void SetPredicate(PredicatePtr thePred);

      typedef std::vector<long> TIdSequence;

      virtual 
      void
      GetElementsId( const SMDS_Mesh* theMesh,
		     TIdSequence& theSequence );

      static
      void
      GetElementsId( const SMDS_Mesh* theMesh, 
		     PredicatePtr thePredicate,
		     TIdSequence& theSequence );
      
    protected:
      PredicatePtr myPredicate;
    };
  };  
};


#endif
