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

#ifndef _SMESH_CONTROLS_HXX_
#define _SMESH_CONTROLS_HXX_

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "SMDSAbs_ElementType.hxx"

class SMDS_Mesh;
class gp_Pnt;
class gp_XYZ;
class TColgp_SequenceOfXYZ;

namespace SMESH{
  namespace Controls{
    class Functor{
    public:
      ~Functor(){}
      virtual void SetMesh( SMDS_Mesh* theMesh ) = 0;
    };
    typedef boost::shared_ptr<Functor> FunctorPtr;

    class NumericalFunctor: public virtual Functor{
    public:
      NumericalFunctor();
      virtual void SetMesh( SMDS_Mesh* theMesh );
      virtual double GetValue( long theElementId ) = 0;
      virtual SMDSAbs_ElementType GetType() const = 0;
      
    protected:
      bool getPoints( const int theId, 
		      TColgp_SequenceOfXYZ& theRes ) const;
    protected:
      SMDS_Mesh* myMesh;
    };
    typedef boost::shared_ptr<NumericalFunctor> NumericalFunctorPtr;
  
  
    /*
      Class       : SMESH_MinimumAngle
      Description : Functor for calculation of minimum angle
    */
    class MinimumAngle: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : AspectRatio
      Description : Functor for calculating aspect ratio
    */
    class AspectRatio: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : Warping
      Description : Functor for calculating warping
    */
    class Warping: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
      
    private:
      double ComputeA( const gp_XYZ&, const gp_XYZ&, 
		       const gp_XYZ&, const gp_XYZ& ) const;
    };
  
  
    /*
      Class       : Taper
      Description : Functor for calculating taper
    */
    class Taper: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
    
  
    /*
      Class       : Skew
      Description : Functor for calculating skew in degrees
    */
    class Skew: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
  
    
    /*
      Class       : Area
      Description : Functor for calculating area
    */
    class Area: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : Length
      Description : Functor for calculating length of edge
    */
    class Length: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
  
  
    /*
      Class       : MultiConnection
      Description : Functor for calculating number of faces conneted to the edge
    */
    class MultiConnection: public virtual NumericalFunctor{
    public:
      virtual double GetValue( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
    };
    
  
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
    typedef boost::shared_ptr<Predicate> PredicatePtr;
    
  
  
    /*
      Class       : FreeBorders
      Description : Predicate for free borders
    */
    class FreeBorders: public virtual Predicate{
    public:
      FreeBorders();
      virtual void SetMesh( SMDS_Mesh* theMesh );
      virtual bool IsSatisfy( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
            
    protected:
      SMDS_Mesh* myMesh;
    };
   

    /*
      Class       : FreeEdges
      Description : Predicate for free Edges
    */
    class FreeEdges: public virtual Predicate{
    public:
      FreeEdges();
      virtual void SetMesh( SMDS_Mesh* theMesh );
      virtual bool IsSatisfy( long theElementId );
      virtual SMDSAbs_ElementType GetType() const;
      struct Border{
	long PntId[2];
	Border(long thePntId1, long thePntId2);
      };
      typedef long TElemId;
      typedef std::map<TElemId,Border> Borders;
      void GetBoreders(Borders& theBorders);
      
    protected:
      SMDS_Mesh* myMesh;
    };
    typedef boost::shared_ptr<FreeEdges> FreeEdgesPtr;
   
    
    /*
      Class       : Comparator
      Description : Base class for comparators
    */
    class Comparator: public virtual Predicate{
    public:
      Comparator();
      virtual ~Comparator();
      virtual void SetMesh( SMDS_Mesh* theMesh );
      virtual void SetMargin(double theValue);
      virtual void SetNumFunctor(NumericalFunctorPtr theFunct);
      virtual bool IsSatisfy( long theElementId ) = 0;
      virtual SMDSAbs_ElementType GetType() const;
  
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
      virtual void SetMesh( SMDS_Mesh* theMesh );
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
      virtual void SetMesh( SMDS_Mesh* theMesh );
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
      FILTER
    */
    class Filter{
    public:
      Filter();
      virtual ~Filter();
      virtual void SetPredicate(PredicatePtr thePred);
      typedef std::vector<long> TIdSequence;
      virtual TIdSequence GetElementsId( SMDS_Mesh* theMesh );
  
    protected:
      PredicatePtr myPredicate;
    };
  };  
};


#endif
