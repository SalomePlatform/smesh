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

#include <set>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Precision.hxx>
#include <TColgp_SequenceOfXYZ.hxx>
#include <TColStd_MapOfInteger.hxx>

#include "SMDS_Mesh.hxx"
#include "SMDS_Iterator.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include "SMESH_Controls.hxx"

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

static int getNbMultiConnection( SMDS_Mesh* theMesh, const int theId )
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
    }
  }

  return aResult;
}


using namespace SMESH::Controls;

/*
                                FUNCTORS
*/

/*
  Class       : NumericalFunctor
  Description : Base class for numerical functors
*/
NumericalFunctor::NumericalFunctor():
  myMesh(NULL)
{}

void NumericalFunctor::SetMesh( SMDS_Mesh* theMesh )
{
  myMesh = theMesh;
}

bool NumericalFunctor::getPoints( const int theId,
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
  }

  return true;
}


/*
  Class       : MinimumAngle
  Description : Functor for calculation of minimum angle
*/
double MinimumAngle::GetValue( long theId )
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

SMDSAbs_ElementType MinimumAngle::GetType() const
{
  return SMDSAbs_Face;
}


/*
  Class       : AspectRatio
  Description : Functor for calculating aspect ratio
*/
double AspectRatio::GetValue( long theId )
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

SMDSAbs_ElementType AspectRatio::GetType() const
{
  return SMDSAbs_Face;
}


/*
  Class       : Warping
  Description : Functor for calculating warping
*/
double Warping::GetValue( long theId )
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

double Warping::ComputeA( const gp_XYZ& thePnt1,
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

SMDSAbs_ElementType Warping::GetType() const
{
  return SMDSAbs_Face;
}


/*
  Class       : Taper
  Description : Functor for calculating taper
*/
double Taper::GetValue( long theId )
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

SMDSAbs_ElementType Taper::GetType() const
{
  return SMDSAbs_Face;
}


/*
  Class       : Skew
  Description : Functor for calculating skew in degrees
*/
static inline double skewAngle( const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3 )
{
  gp_XYZ p12 = ( p2 + p1 ) / 2;
  gp_XYZ p23 = ( p3 + p2 ) / 2;
  gp_XYZ p31 = ( p3 + p1 ) / 2;

  return gp_Vec( p31 - p2 ).Angle( p12 - p23 );
}

double Skew::GetValue( long theId )
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

SMDSAbs_ElementType Skew::GetType() const
{
  return SMDSAbs_Face;
}


/*
  Class       : Area
  Description : Functor for calculating area
*/
double Area::GetValue( long theId )
{
  TColgp_SequenceOfXYZ P;
  if ( !getPoints( theId, P )  || P.Length() != 3 && P.Length() != 4 )
    return 0;

  if ( P.Length() == 3 )
    return getArea( P( 1 ), P( 2 ), P( 3 ) );
  else
    return getArea( P( 1 ), P( 2 ), P( 3 ) ) + getArea( P( 1 ), P( 3 ), P( 4 ) );
}

SMDSAbs_ElementType Area::GetType() const
{
  return SMDSAbs_Face;
}


/*
  Class       : Length
  Description : Functor for calculating length off edge
*/
double Length::GetValue( long theId )
{
  TColgp_SequenceOfXYZ P;
  return getPoints( theId, P ) && P.Length() == 2 ? getDistance( P( 1 ), P( 2 ) ) : 0;
}

SMDSAbs_ElementType Length::GetType() const
{
  return SMDSAbs_Edge;
}


/*
  Class       : MultiConnection
  Description : Functor for calculating number of faces conneted to the edge
*/
double MultiConnection::GetValue( long theId )
{
  return getNbMultiConnection( myMesh, theId );
}

SMDSAbs_ElementType MultiConnection::GetType() const
{
  return SMDSAbs_Edge;
}


/*
                            PREDICATES
*/

/*
  Class       : FreeBorders
  Description : Predicate for free borders
*/

FreeBorders::FreeBorders()
{
  myMesh = 0;
}

void FreeBorders::SetMesh( SMDS_Mesh* theMesh )
{
  myMesh = theMesh;
}

bool FreeBorders::IsSatisfy( long theId )
{
  return getNbMultiConnection( myMesh, theId ) == 1;
}

SMDSAbs_ElementType FreeBorders::GetType() const
{
  return SMDSAbs_Edge;
}


/*
  Class       : FreeEdges
  Description : Predicate for free Edges
*/
FreeEdges::FreeEdges()
{
  myMesh = 0;
}

void FreeEdges::SetMesh( SMDS_Mesh* theMesh )
{
  myMesh = theMesh;
}

bool FreeEdges::IsSatisfy( long theId )
{
  return getNbMultiConnection( myMesh, theId ) == 1;
}

SMDSAbs_ElementType FreeEdges::GetType() const
{
  return SMDSAbs_Face;
}

FreeEdges::Border::Border(long thePntId1, long thePntId2){
  PntId[0] = thePntId1;  PntId[1] = thePntId2;
  if(thePntId1 > thePntId2){
    PntId[1] = thePntId1;  PntId[0] = thePntId2;
  }
}

//bool operator<(const FreeEdges::Border& x, const FreeEdges::Border& y){
//  if(x.PntId[0] < y.PntId[0]) return true;
//  if(x.PntId[0] == y.PntId[0])
//    if(x.PntId[1] < y.PntId[1]) return true;
//  return false;
//}

namespace SMESH{
  namespace Controls{
    struct EdgeBorder: public FreeEdges::Border{
      long ElemId;
      EdgeBorder(long theElemId, long thePntId1, long thePntId2):
	FreeEdges::Border(thePntId1,thePntId2), 
	ElemId(theElemId)
      {}
    };
    
    bool operator<(const FreeEdges::Border& x, const FreeEdges::Border& y){
      if(x.PntId[0] < y.PntId[0]) return true;
      if(x.PntId[0] == y.PntId[0])
	if(x.PntId[1] < y.PntId[1]) return true;
      return false;
    }

    typedef std::set<EdgeBorder> EdgeBorderS;

    inline void UpdateBorders(const EdgeBorder& theBorder,
			      EdgeBorderS& theRegistry, 
			      EdgeBorderS& theContainer)
    {
      if(theRegistry.find(theBorder) == theRegistry.end()){
	theRegistry.insert(theBorder);
	theContainer.insert(theBorder);
      }else{
	theContainer.erase(theBorder);
      }
    }

  }
}

void FreeEdges::GetBoreders(Borders& theBorders)
{
  EdgeBorderS aRegistry;
  EdgeBorderS aContainer;
  SMDS_FaceIteratorPtr anIter = myMesh->facesIterator();
  for(; anIter->more(); ){
    const SMDS_MeshFace* anElem = anIter->next();
    long anElemId = anElem->GetID();
    SMDS_ElemIteratorPtr aNodesIter = anElem->nodesIterator();
    long aNodeId[2];
    const SMDS_MeshElement* aNode;
    if(aNodesIter->more()){
      aNode = aNodesIter->next();
      aNodeId[0] = aNodeId[1] = aNode->GetID();
    }	
    for(; aNodesIter->more(); ){
      aNode = aNodesIter->next();
      EdgeBorder aBorder(anElemId,aNodeId[1],aNode->GetID());
      aNodeId[1] = aNode->GetID();
      //std::cout<<aBorder.PntId[0]<<"; "<<aBorder.PntId[1]<<"; "<<aBorder.ElemId<<"\n";
      UpdateBorders(aBorder,aRegistry,aContainer);
    }
    EdgeBorder aBorder(anElemId,aNodeId[0],aNodeId[1]);
    UpdateBorders(aBorder,aRegistry,aContainer);
  }
  //std::cout<<"aContainer.size() = "<<aContainer.size()<<"\n";
  if(aContainer.size()){
    EdgeBorderS::const_iterator anIter = aContainer.begin();
    for(; anIter != aContainer.end(); anIter++){
      const EdgeBorder& aBorder = *anIter;
      //std::cout<<aBorder.PntId[0]<<"; "<<aBorder.PntId[1]<<"; "<<aBorder.ElemId<<"\n";
      theBorders.insert(Borders::value_type(aBorder.ElemId,aBorder));
    }
  }
}


/*
  Class       : Comparator
  Description : Base class for comparators
*/
Comparator::Comparator():
  myMargin(0)
{}

Comparator::~Comparator()
{}

void Comparator::SetMesh( SMDS_Mesh* theMesh )
{
  if ( myFunctor )
    myFunctor->SetMesh( theMesh );
}

void Comparator::SetMargin( double theValue )
{
  myMargin = theValue;
}

void Comparator::SetNumFunctor( NumericalFunctorPtr theFunct )
{
  myFunctor = theFunct;
}

SMDSAbs_ElementType Comparator::GetType() const
{
  return myFunctor ? myFunctor->GetType() : SMDSAbs_All;
}


/*
  Class       : LessThan
  Description : Comparator "<"
*/
bool LessThan::IsSatisfy( long theId )
{
  return myFunctor && myFunctor->GetValue( theId ) < myMargin;
}


/*
  Class       : MoreThan
  Description : Comparator ">"
*/
bool MoreThan::IsSatisfy( long theId )
{
  return myFunctor && myFunctor->GetValue( theId ) > myMargin;
}


/*
  Class       : EqualTo
  Description : Comparator "="
*/
EqualTo::EqualTo():
  myToler(Precision::Confusion())
{}

bool EqualTo::IsSatisfy( long theId )
{
  return myFunctor && fabs( myFunctor->GetValue( theId ) - myMargin ) < myToler;
}

void EqualTo::SetTolerance( double theToler )
{
  myToler = theToler;
}


/*
  Class       : LogicalNOT
  Description : Logical NOT predicate
*/
LogicalNOT::LogicalNOT()
{}

LogicalNOT::~LogicalNOT()
{}

bool LogicalNOT::IsSatisfy( long theId )
{
  return myPredicate && !myPredicate->IsSatisfy( theId );
}

void LogicalNOT::SetMesh( SMDS_Mesh* theMesh )
{
  if ( myPredicate )
    myPredicate->SetMesh( theMesh );
}

void LogicalNOT::SetPredicate( PredicatePtr thePred )
{
  myPredicate = thePred;
}

SMDSAbs_ElementType LogicalNOT::GetType() const
{
  return myPredicate ? myPredicate->GetType() : SMDSAbs_All;
}


/*
  Class       : LogicalBinary
  Description : Base class for binary logical predicate
*/
LogicalBinary::LogicalBinary()
{}

LogicalBinary::~LogicalBinary()
{}

void LogicalBinary::SetMesh( SMDS_Mesh* theMesh )
{
  if ( myPredicate1 )
    myPredicate1->SetMesh( theMesh );

  if ( myPredicate2 )
    myPredicate2->SetMesh( theMesh );
}

void LogicalBinary::SetPredicate1( PredicatePtr thePredicate )
{
  myPredicate1 = thePredicate;
}

void LogicalBinary::SetPredicate2( PredicatePtr thePredicate )
{
  myPredicate2 = thePredicate;
}

SMDSAbs_ElementType LogicalBinary::GetType() const
{
  if ( !myPredicate1 || !myPredicate2 )
    return SMDSAbs_All;

  SMDSAbs_ElementType aType1 = myPredicate1->GetType();
  SMDSAbs_ElementType aType2 = myPredicate2->GetType();

  return aType1 == aType2 ? aType1 : SMDSAbs_All;
}


/*
  Class       : LogicalAND
  Description : Logical AND
*/
bool LogicalAND::IsSatisfy( long theId )
{
  return 
    myPredicate1 && 
    myPredicate2 && 
    myPredicate1->IsSatisfy( theId ) && 
    myPredicate2->IsSatisfy( theId );
}


/*
  Class       : LogicalOR
  Description : Logical OR
*/
bool LogicalOR::IsSatisfy( long theId )
{
  return 
    myPredicate1 && 
    myPredicate2 && 
    myPredicate1->IsSatisfy( theId ) || 
    myPredicate2->IsSatisfy( theId );
}


/*
                              FILTER
*/

Filter::Filter()
{}

Filter::~Filter()
{}

void Filter::SetPredicate( PredicatePtr thePredicate )
{
  myPredicate = thePredicate;
}

Filter::TIdSequence
Filter::GetElementsId( SMDS_Mesh* theMesh )
{
  TIdSequence aSequence;
  if ( !theMesh || !myPredicate ) return aSequence;

  myPredicate->SetMesh( theMesh );

  SMDSAbs_ElementType aType = myPredicate->GetType();
  switch(aType){
  case SMDSAbs_Edge:{
    SMDS_EdgeIteratorPtr anIter = theMesh->edgesIterator();
    if ( anIter != 0 ) {
      while( anIter->more() ) {
	const SMDS_MeshElement* anElem = anIter->next();
	long anId = anElem->GetID();
	if ( myPredicate->IsSatisfy( anId ) )
	  aSequence.push_back( anId );
      }
    }
  }
  case SMDSAbs_Face:{
    SMDS_FaceIteratorPtr anIter = theMesh->facesIterator();
    if ( anIter != 0 ) {
      while( anIter->more() ) {
	const SMDS_MeshElement* anElem = anIter->next();
	long anId = anElem->GetID();
	if ( myPredicate->IsSatisfy( anId ) )
	  aSequence.push_back( anId );
      }
    }
  }
  }
  return aSequence;
}
