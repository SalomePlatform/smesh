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
//  File   : SMESH_Filter_i.cxx
//  Author : Alexey Petrov, OCC
//  Module : SMESH


#include "SMESH_Filter_i.hxx"

#include "SMDS_Iterator.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDSAbs_ElementType.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESHDS_Mesh.hxx"

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Precision.hxx>
#include <TColgp_SequenceOfXYZ.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

/*
                            AUXILIARY METHODS 
*/

static inline double getAngle( const gp_XYZ& P1, const gp_XYZ& P2, const gp_XYZ& P3 )
{
  return gp_Vec( P1 - P2 ).Angle( gp_Vec( P3 - P2 ) );
}

static inline double getArea( const gp_XYZ& P1, const gp_XYZ& P2, const gp_XYZ& P3 )
{
  gp_Vec aVec1( P2 - P1 );
  gp_Vec aVec2( P3 - P1 );
  return ( aVec1 ^ aVec2 ).Magnitude() * 0.5;
}

static inline double getArea( const gp_Pnt& P1, const gp_Pnt& P2, const gp_Pnt& P3 )
{
  return getArea( P1.XYZ(), P2.XYZ(), P3.XYZ() );
}

static inline double getDistance( const gp_XYZ& P1, const gp_XYZ& P2 )
{
  double aDist = gp_Pnt( P1 ).Distance( gp_Pnt( P2 ) );
  return aDist;
}

static int getNbMultiConnection( SMESHDS_Mesh* theMesh, const int theId )
{
  if ( theMesh == 0 )
    return 0;

  const SMDS_MeshElement* anEdge = theMesh->FindElement( theId );
  if ( anEdge == 0 || anEdge->GetType() != SMDSAbs_Edge || anEdge->NbNodes() != 2 )
    return 0;

  TColStd_MapOfInteger aMap;

  int aResult = 0;
  SMDS_ElemIteratorPtr anIter = anEdge->nodesIterator();
  if ( anIter != 0 )
  {
    while( anIter->more() )
    {
      const SMDS_MeshNode* aNode = (SMDS_MeshNode*)anIter->next();
      if ( aNode == 0 )
        return 0;
      SMDS_ElemIteratorPtr anElemIter = aNode->GetInverseElementIterator();
      while( anElemIter->more() )
      {
        const SMDS_MeshElement* anElem = anElemIter->next();
        if ( anElem != 0 && anElem->GetType() != SMDSAbs_Edge )
        {
          int anId = anElem->GetID();

          if ( anIter->more() )              // i.e. first node
            aMap.Add( anId );
          else if ( aMap.Contains( anId ) )
            aResult++;
        }
      }
//      delete anElemIter;
    }
//    delete anIter;
  }

  return aResult;
}

using namespace std;
using namespace SMESH;

/*
                                FUNCTORS
*/

/*
  Class       : NumericalFunctor_i
  Description : Base class for numerical functors
*/

NumericalFunctor_i::NumericalFunctor_i()
: SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  myMesh = 0;
  SMESH_Gen_i::GetPOA()->activate_object( this );
}

void NumericalFunctor_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  SMESH_Mesh_i* anImplPtr = 
    dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( theMesh ).in() );
  myMesh = anImplPtr ? anImplPtr->GetImpl().GetMeshDS() : 0;
}

bool NumericalFunctor_i::getPoints( const int             theId,
                                    TColgp_SequenceOfXYZ& theRes ) const
{
  theRes.Clear();

  if ( myMesh == 0 )
    return false;

  // Get nodes of the face
  const SMDS_MeshElement* anElem = myMesh->FindElement( theId );
  if ( anElem == 0 || anElem->GetType() != GetType() )
    return false;

  int nbNodes = anElem->NbNodes();

  SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
  if ( anIter != 0 )
  {
    while( anIter->more() )
    {
      const SMDS_MeshNode* aNode = (SMDS_MeshNode*)anIter->next();
      if ( aNode != 0 )
        theRes.Append( gp_XYZ( aNode->X(), aNode->Y(), aNode->Z() ) );
    }

//    delete anIter;
  }

  return true;
}


/*
  Class       : SMESH_MinimumAngleFunct
  Description : Functor for calculation of minimum angle
*/

CORBA::Double MinimumAngle_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P )  || P.Length() != 3 && P.Length() != 4 )
    return 0;

  double aMin;

  if ( P.Length() == 3 )
  {
    double A0 = getAngle( P( 3 ), P( 1 ), P( 2 ) );
    double A1 = getAngle( P( 1 ), P( 2 ), P( 3 ) );
    double A2 = getAngle( P( 2 ), P( 3 ), P( 1 ) );

    aMin = Min( A0, Min( A1, A2 ) );
  }
  else
  {
    double A0 = getAngle( P( 4 ), P( 1 ), P( 2 ) );
    double A1 = getAngle( P( 1 ), P( 2 ), P( 3 ) );
    double A2 = getAngle( P( 2 ), P( 3 ), P( 4 ) );
    double A3 = getAngle( P( 3 ), P( 4 ), P( 1 ) );
    
    aMin = Min( Min( A0, A1 ), Min( A2, A3 ) );
  }
  
  return aMin * 180 / PI;
}

int MinimumAngle_i::GetType() const
{
  return SMDSAbs_Face;
}

/*
  Class       : AspectRatio_i
  Description : Functor for calculating aspect ratio
*/

CORBA::Double AspectRatio_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P )  || P.Length() != 3 && P.Length() != 4 )
    return 0;

  int nbNodes = P.Length();

  // Compute lengths of the sides

  double aLen[ nbNodes ];
  for ( int i = 0; i < nbNodes - 1; i++ )
    aLen[ i ] = getDistance( P( i + 1 ), P( i + 2 ) );
  aLen[ nbNodes - 1 ] = getDistance( P( 1 ), P( nbNodes ) );

  // Compute aspect ratio

  if ( nbNodes == 3 ) 
  {
    double aMaxLen = Max( aLen[ 0 ], Max( aLen[ 1 ], aLen[ 2 ] ) );
    double anArea = getArea( P( 1 ), P( 2 ), P( 3 ) );
    static double aCoef = sqrt( 3. ) / 4;

    return anArea != 0 ? aCoef * aMaxLen * aMaxLen / anArea : 0;
  }
  else
  {
    double aMaxLen = Max( Max( aLen[ 0 ], aLen[ 1 ] ), Max( aLen[ 2 ], aLen[ 3 ] ) );
    double aMinLen = Min( Min( aLen[ 0 ], aLen[ 1 ] ), Min( aLen[ 2 ], aLen[ 3 ] ) );
    
    return aMinLen != 0 ? aMaxLen / aMinLen : 0;
  }
}

int AspectRatio_i::GetType() const
{
  return SMDSAbs_Face;
}

/*
  Class       : Warping_i
  Description : Functor for calculating warping
*/

CORBA::Double Warping_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P ) || P.Length() != 4 )
    return 0;

  gp_XYZ G = ( P( 1 ) + P( 2 ) + P( 3 ) + P( 4 ) ) / 4;

  double A1 = ComputeA( P( 1 ), P( 2 ), P( 3 ), G );
  double A2 = ComputeA( P( 2 ), P( 3 ), P( 4 ), G );
  double A3 = ComputeA( P( 3 ), P( 4 ), P( 1 ), G );
  double A4 = ComputeA( P( 4 ), P( 1 ), P( 2 ), G );

  return Max( Max( A1, A2 ), Max( A3, A4 ) );
}

double Warping_i::ComputeA( const gp_XYZ& thePnt1,
                            const gp_XYZ& thePnt2,
                            const gp_XYZ& thePnt3,
                            const gp_XYZ& theG ) const
{
  double aLen1 = gp_Pnt( thePnt1 ).Distance( gp_Pnt( thePnt2 ) );
  double aLen2 = gp_Pnt( thePnt2 ).Distance( gp_Pnt( thePnt3 ) );
  double L = Min( aLen1, aLen2 ) * 0.5;

  gp_XYZ GI = ( thePnt2 - thePnt1 ) / 2. - theG;
  gp_XYZ GJ = ( thePnt3 - thePnt2 ) / 2. - theG;
  gp_XYZ N  = GI.Crossed( GJ );
  N.Normalize();

  double H = gp_Vec( thePnt2 - theG ).Dot( gp_Vec( N ) );
  return asin( fabs( H / L ) ) * 180 / PI;
}

int Warping_i::GetType() const
{
  return SMDSAbs_Face;
}

/*
  Class       : Taper_i
  Description : Functor for calculating taper
*/

CORBA::Double Taper_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P ) || P.Length() != 4 )
    return 0;

  // Compute taper
  double J1 = getArea( P( 4 ), P( 1 ), P( 2 ) ) / 2;
  double J2 = getArea( P( 3 ), P( 1 ), P( 2 ) ) / 2;
  double J3 = getArea( P( 2 ), P( 3 ), P( 4 ) ) / 2;
  double J4 = getArea( P( 3 ), P( 4 ), P( 1 ) ) / 2;

  double JA = 0.25 * ( J1 + J2 + J3 + J4 );

  double T1 = fabs( ( J1 - JA ) / JA );
  double T2 = fabs( ( J2 - JA ) / JA );
  double T3 = fabs( ( J3 - JA ) / JA );
  double T4 = fabs( ( J4 - JA ) / JA );

  return Max( Max( T1, T2 ), Max( T3, T4 ) );
}

int Taper_i::GetType() const
{
  return SMDSAbs_Face;
}

/*
  Class       : Skew_i
  Description : Functor for calculating skew in degrees
*/

static inline double skewAngle( const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3 )
{
  gp_XYZ p12 = ( p2 + p1 ) / 2;
  gp_XYZ p23 = ( p3 + p2 ) / 2;
  gp_XYZ p31 = ( p3 + p1 ) / 2;

  return gp_Vec( p31 - p2 ).Angle( p12 - p23 );
}

CORBA::Double Skew_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P )  || P.Length() != 3 && P.Length() != 4 )
    return 0;

  // Compute skew
  static double PI2 = PI / 2;
  if ( P.Length() == 3 )
  {
    double A0 = fabs( PI2 - skewAngle( P( 3 ), P( 1 ), P( 2 ) ) );
    double A1 = fabs( PI2 - skewAngle( P( 1 ), P( 2 ), P( 3 ) ) );
    double A2 = fabs( PI2 - skewAngle( P( 2 ), P( 3 ), P( 1 ) ) );

    return Max( A0, Max( A1, A2 ) ) * 180 / PI;
  }
  else 
  {
    gp_XYZ p12 = ( P( 1 ) + P( 2 ) ) / 2;
    gp_XYZ p23 = ( P( 2 ) + P( 3 ) ) / 2;
    gp_XYZ p34 = ( P( 3 ) + P( 4 ) ) / 2;
    gp_XYZ p41 = ( P( 4 ) + P( 1 ) ) / 2;
    
    double A = fabs( PI2 - gp_Vec( p34 - p12 ).Angle( p23 - p41 ) );

    return A * 180 / PI;
  }
}

int Skew_i::GetType() const
{
  return SMDSAbs_Face;
}

/*
  Class       : Area_i
  Description : Functor for calculating area
*/

CORBA::Double Area_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P )  || P.Length() != 3 && P.Length() != 4 )
    return 0;

  if ( P.Length() == 3 )
    return getArea( P( 1 ), P( 2 ), P( 3 ) );
  else
    return getArea( P( 1 ), P( 2 ), P( 3 ) ) + getArea( P( 1 ), P( 3 ), P( 4 ) );
}

int Area_i::GetType() const
{
  return SMDSAbs_Face;
}

/*
  Class       : Length_i
  Description : Functor for calculating length off edge
*/

CORBA::Double Length_i::GetValue( CORBA::Long theId )
{
  TColgp_SequenceOfXYZ P;
  return getPoints( theId, P ) && P.Length() == 2 ? getDistance( P( 1 ), P( 2 ) ) : 0;
}

int Length_i::GetType() const
{
  return SMDSAbs_Edge;
}

/*
  Class       : MultiConnection_i
  Description : Functor for calculating number of faces conneted to the edge
*/

CORBA::Double MultiConnection_i::GetValue( CORBA::Long theId )
{
  return getNbMultiConnection( myMesh, theId );
}

int MultiConnection_i::GetType() const
{
  return SMDSAbs_Edge;
}

/*
                            PREDICATES
*/

/*
  Class       : Predicate_i
  Description : Base class for all predicates
*/
Predicate_i::Predicate_i()
: SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  SMESH_Gen_i::GetPOA()->activate_object( this );
}


/*
  Class       : FreeBorders_i
  Description : Predicate for free borders
*/

FreeBorders_i::FreeBorders_i()
{
  myMesh = 0;
}

void FreeBorders_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  SMESH_Mesh_i* anImplPtr =
    dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( theMesh ).in() );
  myMesh = anImplPtr ? anImplPtr->GetImpl().GetMeshDS() : 0;
}

CORBA::Boolean FreeBorders_i::IsSatisfy( CORBA::Long theId )
{
  return getNbMultiConnection( myMesh, theId ) == 1;
}

int FreeBorders_i::GetType() const
{
  return SMDSAbs_Edge;
}

/*
  Class       : Comparator_i
  Description : Base class for comparators
*/

Comparator_i::Comparator_i()
{
  myMargin  = 0;
  myFunctor = 0;
}

Comparator_i::~Comparator_i()
{
  if ( myFunctor != 0 )
    myFunctor->Destroy();
}

void Comparator_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  if ( myFunctor != 0 )
    myFunctor->SetMesh( theMesh );
}

void Comparator_i::SetMargin( CORBA::Double theValue )
{
  myMargin = theValue;
}

void Comparator_i::SetNumFunctor( NumericalFunctor_ptr theFunct )
{
  if ( myFunctor != 0 )
    myFunctor->Destroy();

  myFunctor = dynamic_cast<NumericalFunctor_i*>( SMESH_Gen_i::GetServant( theFunct ).in() );

  if ( myFunctor != 0 )
    myFunctor->Register();
}

int Comparator_i::GetType() const
{
  return myFunctor != 0 ? myFunctor->GetType() : SMDSAbs_All;
}

/*
  Class       : LessThan_i
  Description : Comparator "<"
*/

CORBA::Boolean LessThan_i::IsSatisfy( CORBA::Long theId )
{
  return myFunctor != 0 && myFunctor->GetValue( theId ) < myMargin;
}

/*
  Class       : MoreThan_i
  Description : Comparator ">"
*/

CORBA::Boolean MoreThan_i::IsSatisfy( CORBA::Long theId )
{
  return myFunctor != 0 && myFunctor->GetValue( theId ) > myMargin;
}

/*
  Class       : EqualTo_i
  Description : Comparator "="
*/
EqualTo_i::EqualTo_i()
{
  myToler = Precision::Confusion();
}

CORBA::Boolean EqualTo_i::IsSatisfy( CORBA::Long theId )
{
  return myFunctor != 0 && fabs( myFunctor->GetValue( theId ) - myMargin ) < myToler;
}

void EqualTo_i::SetTolerance( CORBA::Double theToler )
{
  myToler = theToler;
}


/*
  Class       : LogicalNOT_i
  Description : Logical NOT predicate
*/

LogicalNOT_i::LogicalNOT_i()
{
  myPredicate = 0;
}

LogicalNOT_i::~LogicalNOT_i()
{
  if ( myPredicate )
    myPredicate->Destroy();
}

CORBA::Boolean LogicalNOT_i::IsSatisfy( CORBA::Long theId )
{
  return myPredicate !=0 && !myPredicate->IsSatisfy( theId );
}

void LogicalNOT_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  if ( myPredicate != 0 )
    myPredicate->SetMesh( theMesh );
}

void LogicalNOT_i::SetPredicate( Predicate_ptr thePred )
{
  if ( myPredicate != 0 )
    myPredicate->Destroy();

  myPredicate = dynamic_cast<Predicate_i*>( SMESH_Gen_i::GetServant( thePred ).in() );

  if ( myPredicate != 0 )
    myPredicate->Register();
}

int LogicalNOT_i::GetType() const
{
  return myPredicate != 0 ? myPredicate->GetType() : SMDSAbs_All;
}


/*
  Class       : LogicalBinary_i
  Description : Base class for binary logical predicate
*/

LogicalBinary_i::LogicalBinary_i()
{
  myPredicate1 = 0;
  myPredicate2 = 0;
}
LogicalBinary_i::~LogicalBinary_i()
{
  if ( myPredicate1 != 0 )
    myPredicate1->Destroy();

  if ( myPredicate2 != 0 )
    myPredicate2->Destroy();
}

void LogicalBinary_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  if ( myPredicate1 != 0 )
    myPredicate1->SetMesh( theMesh );

  if ( myPredicate2 != 0 )
    myPredicate2->SetMesh( theMesh );
}

void LogicalBinary_i::SetPredicate1( Predicate_ptr thePredicate )
{
  if ( myPredicate1 != 0 )
    myPredicate1->Destroy();

  myPredicate1 = dynamic_cast<Predicate_i*>( SMESH_Gen_i::GetServant( thePredicate ).in() );

  if ( myPredicate1 != 0 )
    myPredicate1->Register();
}

void LogicalBinary_i::SetPredicate2( Predicate_ptr thePredicate )
{
  if ( myPredicate2 != 0 )
    myPredicate2->Destroy();

  myPredicate2 = dynamic_cast<Predicate_i*>( SMESH_Gen_i::GetServant( thePredicate ).in() );

  if ( myPredicate2 != 0 )
    myPredicate2->Register();
}

int LogicalBinary_i::GetType() const
{
  if ( myPredicate1 == 0 || myPredicate2 == 0 )
    return SMDSAbs_All;

  int aType1 = myPredicate1->GetType();
  int aType2 = myPredicate2->GetType();

  return aType1 == aType2 ? aType1 : SMDSAbs_All;
}

/*
  Class       : LogicalAND_i
  Description : Logical AND
*/

CORBA::Boolean LogicalAND_i::IsSatisfy( CORBA::Long theId )
{
  return myPredicate1 != 0 && 
         myPredicate2 != 0 && 
         myPredicate1->IsSatisfy( theId ) && myPredicate2->IsSatisfy( theId );;
}

/*
  Class       : LogicalOR_i
  Description : Logical OR
*/

CORBA::Boolean LogicalOR_i::IsSatisfy( CORBA::Long theId )
{
  return myPredicate1 != 0 && 
         myPredicate2 != 0 && 
         myPredicate1->IsSatisfy( theId ) || myPredicate2->IsSatisfy( theId );
}


/*
                              FILTER
*/

Filter_i::Filter_i()
{
  myPredicate = 0;
}

Filter_i::~Filter_i()
{
  if ( myPredicate != 0 )
    myPredicate->Destroy();
}

void Filter_i::SetPredicate( Predicate_ptr thePredicate )
{
  if ( myPredicate != 0 )
    myPredicate->Destroy();

  myPredicate = dynamic_cast<Predicate_i*>( SMESH_Gen_i::GetServant( thePredicate ).in() );

  if ( myPredicate != 0 )
    myPredicate->Register();
}

void Filter_i::SetMesh( SMESH_Mesh_ptr theMesh )
{
  if ( myPredicate != 0 )
    myPredicate->SetMesh( theMesh );
}

SMESH::long_array* Filter_i::GetElementsId( SMESH_Mesh_ptr theMesh )
{

  SetMesh( theMesh );

  SMESH_Mesh_i* anImplPtr = 
    dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( theMesh ).in() );

  TColStd_ListOfInteger aList;

  if ( anImplPtr != 0 )
  {
    SMESHDS_Mesh* aMesh = anImplPtr->GetImpl().GetMeshDS();

    if ( myPredicate != 0 )
    {
      int aType = myPredicate->GetType();
      
      if ( aType == SMDSAbs_Edge )
      {
        SMDS_EdgeIteratorPtr anIter = aMesh->edgesIterator();
        if ( anIter != 0 )
        {
          while( anIter->more() )
          {
            const SMDS_MeshElement* anElem = anIter->next();
            if ( myPredicate->IsSatisfy( anElem->GetID() ) )
              aList.Append( anElem->GetID() );
          }
        }
//        delete anIter;
      }
      else if ( aType == SMDSAbs_Face )
      {
        SMDS_FaceIteratorPtr anIter = aMesh->facesIterator();
        if ( anIter != 0 )
        {
          while( anIter->more() )
          {
            const SMDS_MeshElement* anElem = anIter->next();
            if ( myPredicate->IsSatisfy( anElem->GetID() ) )
              aList.Append( anElem->GetID() );
          }
        }
//        delete anIter;
      }
    }
  }
  
  SMESH::long_array_var anArray = new SMESH::long_array;
  
  anArray->length( aList.Extent() );
  TColStd_ListIteratorOfListOfInteger anIter( aList );
  int i = 0;
  for( ; anIter.More(); anIter.Next() )
    anArray[ i++ ] = anIter.Value();

  return anArray._retn();
}

/*
                            FILTER MANAGER
*/

FilterManager_i::FilterManager_i()
: SALOME::GenericObj_i( SMESH_Gen_i::GetPOA() )
{
  SMESH_Gen_i::GetPOA()->activate_object( this );
}

MinimumAngle_ptr FilterManager_i::CreateMinimumAngle()
{
  SMESH::MinimumAngle_i* aServant = new SMESH::MinimumAngle_i();
  SMESH::MinimumAngle_var anObj = aServant->_this();
  return anObj._retn();
}


AspectRatio_ptr FilterManager_i::CreateAspectRatio()
{
  SMESH::AspectRatio_i* aServant = new SMESH::AspectRatio_i();
  SMESH::AspectRatio_var anObj = aServant->_this();
  return anObj._retn();
}


Warping_ptr FilterManager_i::CreateWarping()
{
  SMESH::Warping_i* aServant = new SMESH::Warping_i();
  SMESH::Warping_var anObj = aServant->_this();
  return anObj._retn();
}


Taper_ptr FilterManager_i::CreateTaper()
{
  SMESH::Taper_i* aServant = new SMESH::Taper_i();
  SMESH::Taper_var anObj = aServant->_this();
  return anObj._retn();
}


Skew_ptr FilterManager_i::CreateSkew()
{
  SMESH::Skew_i* aServant = new SMESH::Skew_i();
  SMESH::Skew_var anObj = aServant->_this();
  return anObj._retn();
}


Area_ptr FilterManager_i::CreateArea()
{
  SMESH::Area_i* aServant = new SMESH::Area_i();
  SMESH::Area_var anObj = aServant->_this();
  return anObj._retn();
}


Length_ptr FilterManager_i::CreateLength()
{
  SMESH::Length_i* aServant = new SMESH::Length_i();
  SMESH::Length_var anObj = aServant->_this();
  return anObj._retn();
}


MultiConnection_ptr FilterManager_i::CreateMultiConnection()
{
  SMESH::MultiConnection_i* aServant = new SMESH::MultiConnection_i();
  SMESH::MultiConnection_var anObj = aServant->_this();
  return anObj._retn();
}


FreeBorders_ptr FilterManager_i::CreateFreeBorders()
{
  SMESH::FreeBorders_i* aServant = new SMESH::FreeBorders_i();
  SMESH::FreeBorders_var anObj = aServant->_this();
  return anObj._retn();
}

LessThan_ptr FilterManager_i::CreateLessThan()
{
  SMESH::LessThan_i* aServant = new SMESH::LessThan_i();
  SMESH::LessThan_var anObj = aServant->_this();
  return anObj._retn();
}


MoreThan_ptr FilterManager_i::CreateMoreThan()
{
  SMESH::MoreThan_i* aServant = new SMESH::MoreThan_i();
  SMESH::MoreThan_var anObj = aServant->_this();
  return anObj._retn();
}

EqualTo_ptr FilterManager_i::CreateEqualTo()
{
  SMESH::EqualTo_i* aServant = new SMESH::EqualTo_i();
  SMESH::EqualTo_var anObj = aServant->_this();
  return anObj._retn();
}


LogicalNOT_ptr FilterManager_i::CreateLogicalNOT()
{
  SMESH::LogicalNOT_i* aServant = new SMESH::LogicalNOT_i();
  SMESH::LogicalNOT_var anObj = aServant->_this();
  return anObj._retn();
}


LogicalAND_ptr FilterManager_i::CreateLogicalAND()
{
  SMESH::LogicalAND_i* aServant = new SMESH::LogicalAND_i();
  SMESH::LogicalAND_var anObj = aServant->_this();
  return anObj._retn();
}


LogicalOR_ptr FilterManager_i::CreateLogicalOR()
{
  SMESH::LogicalOR_i* aServant = new SMESH::LogicalOR_i();
  SMESH::LogicalOR_var anObj = aServant->_this();
  return anObj._retn();
}

Filter_ptr FilterManager_i::CreateFilter()
{
  SMESH::Filter_i* aServant = new SMESH::Filter_i();
  SMESH::Filter_var anObj = aServant->_this();
  return anObj._retn();
}
