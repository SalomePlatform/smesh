// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

// Remarques :
// L'ordre de description des fonctions est le meme dans tous les fichiers
// HOMARD_aaaa.idl, HOMARD_aaaa.hxx, HOMARD_aaaa.cxx, HOMARD_aaaa_i.hxx, HOMARD_aaaa_i.cxx :
// 1. Les generalites : Name, Delete, DumpPython, Dump, Restore
// 2. Les caracteristiques
// 3. Le lien avec les autres structures
//
// Quand les 2 fonctions Setxxx et Getxxx sont presentes, Setxxx est decrit en premier
//

#include "HOMARD_Hypothesis_i.hxx"
#include "HOMARD_Gen_i.hxx"
#include "HOMARD_Hypothesis.hxx"
#include "HOMARD_DriverTools.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Hypothesis_i::HOMARD_Hypothesis_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Hypothesis_i::HOMARD_Hypothesis_i( CORBA::ORB_ptr orb,
                                          HOMARD::HOMARD_Gen_var engine )
{
  MESSAGE( "standard constructor" );
  _gen_i = engine;
  _orb = orb;
  myHomardHypothesis = new ::HOMARD_Hypothesis();
  ASSERT( myHomardHypothesis );
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Hypothesis_i::~HOMARD_Hypothesis_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis_i::SetName( const char* Name )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetName( Name );
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetName()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetName().c_str() );
}
//=============================================================================
CORBA::Long  HOMARD_Hypothesis_i::Delete()
{
  ASSERT( myHomardHypothesis );
  char* HypoName = GetName() ;
  MESSAGE ( "Delete : destruction de l'hypothese " << HypoName );
  return _gen_i->DeleteHypo(HypoName) ;
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetDumpPython()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Hypothesis_i::Dump() const
{
  return HOMARD::Dump( *myHomardHypothesis );
}
//=============================================================================
bool HOMARD_Hypothesis_i::Restore( const std::string& stream )
{
  return HOMARD::Restore( *myHomardHypothesis, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis_i::SetUnifRefinUnRef( CORBA::Long TypeRaffDera )
{
  ASSERT( myHomardHypothesis );
  VERIFICATION( (TypeRaffDera==1) || (TypeRaffDera==-1) );
  int TypeRaff, TypeDera ;
  if ( TypeRaffDera == 1 )
  {
    TypeRaff = 1 ;
    TypeDera = 0 ;
  }
  else if ( TypeRaffDera == -1 )
  {
    TypeRaff = 0 ;
    TypeDera = 1 ;
  }
  myHomardHypothesis->SetAdapType( -1 );
  myHomardHypothesis->SetRefinTypeDera( TypeRaff, TypeDera );
}
//=============================================================================
HOMARD::listeTypes* HOMARD_Hypothesis_i::GetAdapRefinUnRef()
{
  ASSERT( myHomardHypothesis );
  HOMARD::listeTypes_var aResult = new HOMARD::listeTypes;
  aResult->length( 3 );
  aResult[0] = CORBA::Long( myHomardHypothesis->GetAdapType() );
  aResult[1] = CORBA::Long( myHomardHypothesis->GetRefinType() );
  aResult[2] = CORBA::Long( myHomardHypothesis->GetUnRefType() );
  return aResult._retn();
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetAdapType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetAdapType() );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetRefinType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetRefinType() );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetUnRefType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetUnRefType() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetField( const char* FieldName )
{
  myHomardHypothesis->SetField( FieldName );
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetFieldName()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetFieldName().c_str() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetUseField( CORBA::Long UsField )
{
  myHomardHypothesis->SetUseField( UsField );
}
//=============================================================================
HOMARD::InfosHypo* HOMARD_Hypothesis_i::GetField()
{
  ASSERT( myHomardHypothesis );
  HOMARD::InfosHypo* aInfosHypo = new HOMARD::InfosHypo();
  aInfosHypo->FieldName  = CORBA::string_dup( myHomardHypothesis->GetFieldName().c_str() );
  aInfosHypo->TypeThR    = CORBA::Long( myHomardHypothesis->GetRefinThrType() );
  aInfosHypo->ThreshR    = CORBA::Double( myHomardHypothesis->GetThreshR() );
  aInfosHypo->TypeThC    = CORBA::Long( myHomardHypothesis->GetUnRefThrType() );
  aInfosHypo->ThreshC    = CORBA::Double( myHomardHypothesis->GetThreshC() );
  aInfosHypo->UsField    = CORBA::Long( myHomardHypothesis->GetUseField() );
  aInfosHypo->UsCmpI     = CORBA::Long( myHomardHypothesis->GetUseComp() );
  return aInfosHypo;
}
//=============================================================================
void HOMARD_Hypothesis_i::SetUseComp( CORBA::Long UsCmpI )
{
  myHomardHypothesis->SetUseComp( UsCmpI );
}
//=============================================================================
void HOMARD_Hypothesis_i::AddComp( const char* NomComp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddComp( NomComp );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprComp( const char* NomComp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprComp(NomComp);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprComps()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprComps();
}
//=============================================================================
HOMARD::listeComposantsHypo* HOMARD_Hypothesis_i::GetComps()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetComps();
  HOMARD::listeComposantsHypo_var aResult = new HOMARD::listeComposantsHypo;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetRefinThr( CORBA::Long TypeThR, CORBA::Double ThreshR )
{
  myHomardHypothesis->SetAdapType( 1 );
  if ( TypeThR > 0 )
  {
    int TypeDera = myHomardHypothesis->GetUnRefType() ;
    myHomardHypothesis->SetRefinTypeDera( 1, TypeDera );
  }
  myHomardHypothesis->SetRefinThr( TypeThR, ThreshR );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetRefinThrType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetRefinThrType() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetUnRefThr( CORBA::Long TypeThC, CORBA::Double ThreshC )
{
  myHomardHypothesis->SetAdapType( 1 );
  if ( TypeThC > 0 )
  {
    int TypeRaff = myHomardHypothesis->GetRefinType() ;
    myHomardHypothesis->SetRefinTypeDera( TypeRaff, 1 );
  }
  myHomardHypothesis->SetUnRefThr( TypeThC, ThreshC );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetUnRefThrType()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetUnRefThrType() );
}
//=============================================================================
void HOMARD_Hypothesis_i::SetNivMax( CORBA::Long NivMax )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetNivMax( NivMax );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetNivMax()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetNivMax();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetDiamMin( CORBA::Double DiamMin )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetDiamMin( DiamMin );
}
//=============================================================================
CORBA::Double HOMARD_Hypothesis_i::GetDiamMin()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetDiamMin();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetAdapInit( CORBA::Long AdapInit )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetAdapInit( AdapInit );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetAdapInit()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetAdapInit();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetExtraOutput( CORBA::Long ExtraOutput )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetExtraOutput( ExtraOutput );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetExtraOutput()
{
  ASSERT( myHomardHypothesis );
  return myHomardHypothesis->GetExtraOutput();
}
//=============================================================================
void HOMARD_Hypothesis_i::AddGroup( const char* Group)
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddGroup( Group );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprGroup( const char* Group )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprGroup(Group);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprGroups()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprGroups();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetGroups(const HOMARD::ListGroupType& ListGroup)
{
  ASSERT( myHomardHypothesis );
  std::list<std::string> ListString;
  for ( int i = 0; i < ListGroup.length(); i++ )
  {
    ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardHypothesis->SetGroups( ListString );
}
//=============================================================================
HOMARD::ListGroupType*  HOMARD_Hypothesis_i::GetGroups()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetGroups();
  HOMARD::ListGroupType_var aResult = new HOMARD::ListGroupType;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Hypothesis_i::SetTypeFieldInterp( CORBA::Long TypeFieldInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetTypeFieldInterp( TypeFieldInterp );
}
//=============================================================================
CORBA::Long HOMARD_Hypothesis_i::GetTypeFieldInterp()
{
  ASSERT( myHomardHypothesis );
  return CORBA::Long( myHomardHypothesis->GetTypeFieldInterp() );
}
//=============================================================================
void HOMARD_Hypothesis_i::AddFieldInterp( const char* FieldInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddFieldInterpType( FieldInterp, 0 );
}
//=============================================================================
void HOMARD_Hypothesis_i::AddFieldInterpType( const char* FieldInterp, CORBA::Long TypeInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddFieldInterpType( FieldInterp, TypeInterp );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprFieldInterp( const char* FieldInterp )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprFieldInterp(FieldInterp);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprFieldInterps()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprFieldInterps();
}
//=============================================================================
HOMARD::listeFieldInterpsHypo* HOMARD_Hypothesis_i::GetFieldInterps()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetFieldInterps();
  HOMARD::listeFieldInterpsHypo_var aResult = new HOMARD::listeFieldInterpsHypo;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Hypothesis_i::SetCaseCreation( const char* NomCaseCreation )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SetCaseCreation( NomCaseCreation );
}
//=============================================================================
char* HOMARD_Hypothesis_i::GetCaseCreation()
{
  ASSERT( myHomardHypothesis );
  return CORBA::string_dup( myHomardHypothesis->GetCaseCreation().c_str() );
}
//=============================================================================
void HOMARD_Hypothesis_i::LinkIteration( const char* NomIteration )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->LinkIteration( NomIteration );
}
//=============================================================================
void HOMARD_Hypothesis_i::UnLinkIteration( const char* NomIteration )
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->UnLinkIteration( NomIteration );
}
//=============================================================================
HOMARD::listeIters* HOMARD_Hypothesis_i::GetIterations()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetIterations();
  HOMARD::listeIters_var aResult = new HOMARD::listeIters;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Hypothesis_i::AddZone( const char* NomZone, CORBA::Long TypeUse )
{
  MESSAGE ("Dans AddZone pour " << NomZone << " et TypeUse = " << TypeUse ) ;
  ASSERT( myHomardHypothesis );
  VERIFICATION( (TypeUse==1) || (TypeUse==-1) );
  myHomardHypothesis->SetAdapType( 0 );
  int TypeRaff, TypeDera ;
  if ( TypeUse == 1 )
  {
    TypeRaff = 1 ;
    TypeDera = myHomardHypothesis->GetUnRefType() ;
  }
  else if ( TypeUse == -1 )
  {
    TypeRaff = myHomardHypothesis->GetRefinType() ;
    TypeDera = 1 ;
  }
  myHomardHypothesis->SetRefinTypeDera( TypeRaff, TypeDera );
  char* NomHypo = GetName() ;
  return _gen_i->AssociateHypoZone(NomHypo, NomZone, TypeUse) ;
}
//=============================================================================
void HOMARD_Hypothesis_i::AddZone0( const char* NomZone, CORBA::Long TypeUse )
{
  MESSAGE ("Dans AddZone0 pour " << NomZone) ;
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->AddZone( NomZone, TypeUse );
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprZone(const char * NomZone)
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprZone( NomZone);
}
//=============================================================================
void HOMARD_Hypothesis_i::SupprZones()
{
  ASSERT( myHomardHypothesis );
  myHomardHypothesis->SupprZones();
}
//=============================================================================
HOMARD::listeZonesHypo* HOMARD_Hypothesis_i::GetZones()
{
  ASSERT( myHomardHypothesis );
  const std::list<std::string>& ListString = myHomardHypothesis->GetZones();
  HOMARD::listeZonesHypo_var aResult = new HOMARD::listeZonesHypo;
  aResult->length( ListString.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = ListString.begin(); it != ListString.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
