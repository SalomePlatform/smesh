//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : SMESH_HypoFilter.cxx
//  Module : SMESH
//  $Header$

#include "SMESH_HypoFilter.hxx"

#include "SMESH_Hypothesis.hxx"
#include "SMESH_subMesh.hxx"

using namespace std;


//=======================================================================
//function : NamePredicate::Value
//purpose  : 
//=======================================================================

bool SMESH_HypoFilter::NamePredicate::IsOk (const SMESH_Hypothesis* aHyp,
                                            const TopoDS_Shape&     /*aShape*/ ) const
{
  return ( _name == aHyp->GetName() );
}

//=======================================================================
//function : TypePredicate::Value
//purpose  : 
//=======================================================================

int SMESH_HypoFilter::TypePredicate::Value( const SMESH_Hypothesis* aHyp ) const
{
  return aHyp->GetType();
};

//=======================================================================
//function : DimPredicate::Value
//purpose  : 
//=======================================================================

int SMESH_HypoFilter::DimPredicate::Value( const SMESH_Hypothesis* aHyp ) const
{
  return aHyp->GetDim();
}

//=======================================================================
//function : ApplicablePredicate::IsOk
//purpose  : 
//=======================================================================

bool SMESH_HypoFilter::ApplicablePredicate::IsOk(const SMESH_Hypothesis* aHyp,
                                                 const TopoDS_Shape&     /*aShape*/) const
{
  return SMESH_subMesh::IsApplicableHypotesis( aHyp, (TopAbs_ShapeEnum)_shapeType );
};

//=======================================================================
//function : ApplicablePredicate::ApplicablePredicate
//purpose  : 
//=======================================================================

SMESH_HypoFilter::ApplicablePredicate::ApplicablePredicate( const TopoDS_Shape& theShape )
{
  _shapeType = ( theShape.IsNull() ? TopAbs_SHAPE : theShape.ShapeType());
}

//=======================================================================
//function : InstancePredicate::IsOk
//purpose  : 
//=======================================================================

bool SMESH_HypoFilter::InstancePredicate::IsOk(const SMESH_Hypothesis* aHyp,
                                               const TopoDS_Shape&     /*aShape*/) const
{
  return _hypo == aHyp;
}

//=======================================================================
//function : IsGlobalPredicate::IsOk
//purpose  : 
//=======================================================================

bool SMESH_HypoFilter::IsGlobalPredicate::IsOk(const SMESH_Hypothesis* aHyp,
                                               const TopoDS_Shape&     aShape) const
{
  return ( !_mainShape.IsNull() && !aShape.IsNull() && _mainShape.IsSame( aShape ));
}

//=======================================================================
//function : SMESH_HypoFilter
//purpose  : 
//=======================================================================

SMESH_HypoFilter::SMESH_HypoFilter()
{
}

//=======================================================================
//function : SMESH_HypoFilter
//purpose  : 
//=======================================================================

SMESH_HypoFilter::SMESH_HypoFilter( SMESH_HypoPredicate* aPredicate, bool notNagate )
{
  add( notNagate ? AND : AND_NOT, aPredicate );
}

//=======================================================================
//function : And
//purpose  : 
//=======================================================================

void SMESH_HypoFilter::And( SMESH_HypoPredicate* aPredicate )
{
  add( AND, aPredicate );
}

//=======================================================================
//function : AndNot
//purpose  : 
//=======================================================================

void SMESH_HypoFilter::AndNot( SMESH_HypoPredicate* aPredicate )
{
  add( AND_NOT, aPredicate );
}

//=======================================================================
//function : Or
//purpose  : 
//=======================================================================

void SMESH_HypoFilter::Or( SMESH_HypoPredicate* aPredicate )
{
  add( OR, aPredicate );
}

//=======================================================================
//function : OrNot
//purpose  : Return predicates
//=======================================================================

void SMESH_HypoFilter::OrNot( SMESH_HypoPredicate* aPredicate )
{
  add( OR_NOT, aPredicate );
}

//=======================================================================
//function : Is
//purpose  : 
//=======================================================================

SMESH_HypoPredicate* SMESH_HypoFilter::Is(const SMESH_Hypothesis* theHypo)
{
  return new InstancePredicate( theHypo );
}

//=======================================================================
//function : IsAlgo
//purpose  : 
//=======================================================================

SMESH_HypoPredicate* SMESH_HypoFilter::IsAlgo()
{
  return new TypePredicate( MORE, SMESHDS_Hypothesis::PARAM_ALGO );
}

//=======================================================================
//function : IsGlobal
//purpose  : 
//=======================================================================

 SMESH_HypoPredicate* SMESH_HypoFilter::IsGlobal(const TopoDS_Shape& theMainShape)
{
  return new IsGlobalPredicate( theMainShape );
}

//=======================================================================
//function : HasName
//purpose  : 
//=======================================================================

SMESH_HypoPredicate* SMESH_HypoFilter::HasName(const string & theName)
{
  return new NamePredicate( theName );
}

//=======================================================================
//function : HasDim
//purpose  : 
//=======================================================================

SMESH_HypoPredicate* SMESH_HypoFilter::HasDim(const int theDim)
{
  return new DimPredicate( EQUAL, theDim );
}

//=======================================================================
//function : IsApplicableTo
//purpose  : 
//=======================================================================

SMESH_HypoPredicate* SMESH_HypoFilter::IsApplicableTo(const TopoDS_Shape& theShape)
{
  return new ApplicablePredicate( theShape );
}

//=======================================================================
//function : HasType
//purpose  : 
//=======================================================================

SMESH_HypoPredicate* SMESH_HypoFilter::HasType(const int theHypType)
{
  return new TypePredicate( EQUAL, theHypType );
}

//=======================================================================
//function : IsOk
//purpose  : 
//=======================================================================

bool SMESH_HypoFilter::IsOk (const SMESH_Hypothesis* aHyp,
                             const TopoDS_Shape&     aShape) const
{
  if ( myPredicates.empty() )
    return true;

  bool ok = ( myPredicates.front()->_logical_op <= AND_NOT );
  list<SMESH_HypoPredicate*>::const_iterator pred = myPredicates.begin();
  for ( ; pred != myPredicates.end(); ++pred )
  {
    bool ok2 = (*pred)->IsOk( aHyp, aShape );
    switch ( (*pred)->_logical_op ) {
    case AND:     ok = ok && ok2; break;
    case AND_NOT: ok = ok && !ok2; break;
    case OR:      ok = ok || ok2; break;
    case OR_NOT:  ok = ok || !ok2; break;
    default:;
    }
  }
  return ok;
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================

void SMESH_HypoFilter::Init  ( SMESH_HypoPredicate* aPredicate, bool notNagate )
{
  list<SMESH_HypoPredicate*>::const_iterator pred = myPredicates.begin();
  for ( ; pred != myPredicates.end(); ++pred )
    delete *pred;

  add( notNagate ? AND : AND_NOT, aPredicate );
}


//=======================================================================
//function : IsOk
//purpose  : 
//=======================================================================

SMESH_HypoFilter::~SMESH_HypoFilter()
{
  Init(0);
}

