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
#include "SALOME_GenericObj_i.hh"

class SMESHDS_Mesh;
class gp_Pnt;
class gp_XYZ;
class TColgp_SequenceOfXYZ;

namespace SMESH{

/*
                                FUNCTORS
*/

/*
  Class       : NumericalFunctor_i
  Description : Base class for numerical functors 
  
    PortableServer::POA_ptr thePOA

*/

class NumericalFunctor_i: public virtual POA_SMESH::NumericalFunctor,
                          public virtual SALOME::GenericObj_i
{
public:
                          NumericalFunctor_i();
  void                    SetMesh( SMESH_Mesh_ptr theMesh );
  virtual int             GetType() const = 0;

protected:
  bool                    getPoints( const int             theId, 
                                     TColgp_SequenceOfXYZ& theRes ) const;
protected:
  SMESHDS_Mesh*           myMesh;
};

/*
  Class       : SMESH_MinimumAngleFunct
  Description : Functor for calculation of minimum angle
*/

class MinimumAngle_i: public virtual POA_SMESH::MinimumAngle,
                      public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue( CORBA::Long theElementId );
  virtual int             GetType() const;
};

/*
  Class       : AspectRatio_i
  Description : Functor for calculating aspect ratio
*/

class AspectRatio_i: public virtual POA_SMESH::AspectRatio,
                     public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue(CORBA::Long theElementId);
  virtual int             GetType() const;
};

/*
  Class       : Warping_i
  Description : Functor for calculating warping
*/

class Warping_i: public virtual POA_SMESH::Warping,
                 public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue(CORBA::Long theElementId);
  virtual int             GetType() const;

private:
  double                  ComputeA( const gp_XYZ&, const gp_XYZ&, 
                                    const gp_XYZ&, const gp_XYZ& ) const;
};

/*
  Class       : Taper_i
  Description : Functor for calculating taper
*/

class Taper_i: public virtual POA_SMESH::Taper,
               public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue( CORBA::Long theElementId );
  virtual int             GetType() const;
};

/*
  Class       : Skew_i
  Description : Functor for calculating skew in degrees
*/

class Skew_i: public virtual POA_SMESH::Skew,
              public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue( CORBA::Long theElementId );
  virtual int             GetType() const;
};

/*
  Class       : Area_i
  Description : Functor for calculating area
*/

class Area_i: public virtual POA_SMESH::Area,
              public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue( CORBA::Long theElementId );
  virtual int             GetType() const;
};

/*
  Class       : Length_i
  Description : Functor for calculating length of edge
*/

class Length_i: public virtual POA_SMESH::Length,
                public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue( CORBA::Long theElementId );
  virtual int             GetType() const;
};

/*
  Class       : MultiConnection_i
  Description : Functor for calculating number of faces conneted to the edge
*/

class MultiConnection_i: public virtual POA_SMESH::MultiConnection,
                        public virtual NumericalFunctor_i
{
public:
  CORBA::Double           GetValue( CORBA::Long theElementId );
  virtual int             GetType() const;
};


/*
                            PREDICATES
*/

/*
  Class       : Predicate_i
  Description : Base class for all predicates
*/

class Predicate_i: public virtual POA_SMESH::Predicate,
                   public virtual SALOME::GenericObj_i
{
public:
                          Predicate_i();
  virtual int             GetType() const = 0;
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
  void                    SetMesh( SMESH_Mesh_ptr theMesh );
  CORBA::Boolean          IsSatisfy( CORBA::Long theElementId );
  virtual int             GetType() const;

protected:
  SMESHDS_Mesh*           myMesh;
};

/*
  Class       : Comparator_i
  Description : Base class for comparators
*/

class Comparator_i: public virtual POA_SMESH::Comparator,
                    public virtual Predicate_i
{
public:
                          Comparator_i();
  virtual                 ~Comparator_i();

  void                    SetMesh( SMESH_Mesh_ptr theMesh );
  void                    SetMargin( CORBA::Double );
  void                    SetNumFunctor( NumericalFunctor_ptr );

  virtual int             GetType() const;

protected:
  CORBA::Double           myMargin;
  NumericalFunctor_i*     myFunctor;
};

/*
  Class       : LessThan_i
  Description : Comparator "<"
*/

class LessThan_i: public virtual POA_SMESH::LessThan,
                  public virtual Comparator_i
{
public:
  CORBA::Boolean          IsSatisfy( CORBA::Long theElementId );
};

/*
  Class       : MoreThan_i
  Description : Comparator ">"
*/
class MoreThan_i: public virtual POA_SMESH::MoreThan,
                  public virtual Comparator_i
{
public:
  CORBA::Boolean          IsSatisfy( CORBA::Long theElementId );
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

  CORBA::Boolean          IsSatisfy( CORBA::Long theElementId );

  void                    SetTolerance( CORBA::Double );

private:
  CORBA::Double           myToler;
};

/*
  Class       : Logical_i
  Description : Base class for logical predicate
*/

class Logical_i: public virtual POA_SMESH::Logical,
                 public virtual Predicate_i
  
{
};

/*
  Class       : LogicalNOT_i
  Description : Logical NOT predicate
*/

class LogicalNOT_i: public virtual POA_SMESH::LogicalNOT,
                    public virtual Logical_i
{
public:
                          LogicalNOT_i();
  virtual                 ~LogicalNOT_i();

  CORBA::Boolean          IsSatisfy( CORBA::Long );

  void                    SetMesh( SMESH_Mesh_ptr );
  void                    SetPredicate( Predicate_ptr );

  virtual int             GetType() const;

private:
  Predicate_i*            myPredicate;
};


/*
  Class       : LogicalBinary_i
  Description : Base class for binary logical predicate
*/

class LogicalBinary_i: public virtual POA_SMESH::LogicalBinary,
		                   public virtual Logical_i
{
public:
                          LogicalBinary_i();
  virtual                 ~LogicalBinary_i();

  void                    SetMesh( SMESH_Mesh_ptr );

  void                    SetPredicate1( Predicate_ptr );
  void                    SetPredicate2( Predicate_ptr );

  virtual int             GetType() const;

protected:
  Predicate_i*            myPredicate1;
  Predicate_i*            myPredicate2;
};

/*
  Class       : LogicalAND_i
  Description : Logical AND
*/

class LogicalAND_i: public virtual POA_SMESH::LogicalAND,
                    public virtual LogicalBinary_i
{
public:
  CORBA::Boolean          IsSatisfy( CORBA::Long theElementId );
};

/*
  Class       : LogicalOR_i
  Description : Logical OR
*/

class LogicalOR_i: public virtual POA_SMESH::LogicalOR,
                   public virtual LogicalBinary_i
{
public:
  CORBA::Boolean          IsSatisfy( CORBA::Long theElementId );
};


/*
                               FILTER
*/

class Filter_i: public virtual POA_SMESH::Filter,
	              public virtual SALOME::GenericObj_i
{
public:
                          Filter_i();
  virtual                 ~Filter_i();
  void                    SetPredicate(Predicate_ptr );
  long_array*             GetElementsId(SMESH_Mesh_ptr );
  void                    SetMesh( SMESH_Mesh_ptr );

protected:
  Predicate_i*            myPredicate;
};


/*
                            FILTER MANAGER
*/

class FilterManager_i: public virtual POA_SMESH::FilterManager,
		                   public virtual SALOME::GenericObj_i
{
public:
                          FilterManager_i();
  MinimumAngle_ptr        CreateMinimumAngle();
  AspectRatio_ptr         CreateAspectRatio();
  Warping_ptr             CreateWarping();
  Taper_ptr               CreateTaper();
  Skew_ptr                CreateSkew();
  Area_ptr                CreateArea();
  Length_ptr              CreateLength();
  MultiConnection_ptr     CreateMultiConnection();
  
  FreeBorders_ptr         CreateFreeBorders();

  LessThan_ptr            CreateLessThan();
  MoreThan_ptr            CreateMoreThan();
  EqualTo_ptr             CreateEqualTo();
  
  LogicalNOT_ptr          CreateLogicalNOT();
  LogicalAND_ptr          CreateLogicalAND();
  LogicalOR_ptr           CreateLogicalOR();

  Filter_ptr              CreateFilter();
};



};


#endif
