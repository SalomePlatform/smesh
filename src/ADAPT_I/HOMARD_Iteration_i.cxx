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

#include "HOMARD_Iteration_i.hxx"
#include "HOMARD_Gen_i.hxx"
#include "HOMARD_Iteration.hxx"
#include "HOMARD_DriverTools.hxx"

#include "SALOMEDS_Tool.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Iteration_i::HOMARD_Iteration_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Iteration_i::HOMARD_Iteration_i( CORBA::ORB_ptr orb,
                                        HOMARD::HOMARD_Gen_var engine )
{
  MESSAGE("constructor");
  _gen_i = engine;
  _orb = orb;
  myHomardIteration = new ::HOMARD_Iteration();
  ASSERT( myHomardIteration );
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Iteration_i::~HOMARD_Iteration_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetName( const char* Name )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetName( Name );
}
//=============================================================================
char* HOMARD_Iteration_i::GetName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetName().c_str() );
}
//=============================================================================
CORBA::Long  HOMARD_Iteration_i::Delete( CORBA::Long Option )
{
  ASSERT( myHomardIteration );
  char* IterName = GetName() ;
  MESSAGE ( "Delete : destruction de l'iteration " << IterName << ", Option = " << Option );
  return _gen_i->DeleteIteration(IterName, Option) ;
}
//=============================================================================
char* HOMARD_Iteration_i::GetDumpPython()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Iteration_i::Dump() const
{
  return HOMARD::Dump( *myHomardIteration );
}
//=============================================================================
bool HOMARD_Iteration_i::Restore( const std::string& stream )
{
  return HOMARD::Restore( *myHomardIteration, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetDirNameLoc( const char* NomDir )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetDirNameLoc( NomDir );
}
//=============================================================================
char* HOMARD_Iteration_i::GetDirNameLoc()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetDirNameLoc().c_str() );
}
//=============================================================================
char* HOMARD_Iteration_i::GetDirName()
{
  ASSERT( myHomardIteration );
  std::string casename = myHomardIteration->GetCaseName() ;
  HOMARD::HOMARD_Cas_ptr caseiter = _gen_i->GetCase(casename.c_str()) ;
  std::string dirnamecase = caseiter->GetDirName() ;
  std::string dirname = dirnamecase + "/" +  GetDirNameLoc() ;
  return CORBA::string_dup( dirname.c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::SetNumber( CORBA::Long NumIter )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetNumber( NumIter );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetNumber()
{
  ASSERT( myHomardIteration );
  return myHomardIteration->GetNumber() ;
}
//=============================================================================
void HOMARD_Iteration_i::SetState( CORBA::Long Etat )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetState( Etat );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetState()
{
  ASSERT( myHomardIteration );
  return myHomardIteration->GetState() ;
}
//=============================================================================
void HOMARD_Iteration_i::SetMeshName( const char* NomMesh )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetMeshName( NomMesh );
}
//=============================================================================
char* HOMARD_Iteration_i::GetMeshName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetMeshName().c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::SetMeshFile( const char* MeshFile )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetMeshFile( MeshFile );
}
//=============================================================================
char* HOMARD_Iteration_i::GetMeshFile()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetMeshFile().c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::SetFieldFile( const char* FieldFile )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetFieldFile( FieldFile );
}
//=============================================================================
char* HOMARD_Iteration_i::GetFieldFile()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetFieldFile().c_str() );
}
//=============================================================================
// Instants pour le champ de pilotage
//=============================================================================
void HOMARD_Iteration_i::SetTimeStep( CORBA::Long TimeStep )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetTimeStep( TimeStep );
}
//=============================================================================
void HOMARD_Iteration_i::SetTimeStepRank( CORBA::Long TimeStep, CORBA::Long Rank )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetTimeStepRank( TimeStep, Rank );
}
//=============================================================================
void HOMARD_Iteration_i::SetTimeStepRankLast()
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetTimeStepRankLast();
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetTimeStep()
{
  ASSERT( myHomardIteration );
  return  CORBA::Long( myHomardIteration->GetTimeStep() );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetRank()
{
  ASSERT( myHomardIteration );
  return  CORBA::Long( myHomardIteration->GetRank() );
}
//=============================================================================
// Instants pour un champ a interpoler
//=============================================================================
void HOMARD_Iteration_i::SetFieldInterpTimeStep( const char* FieldInterp, CORBA::Long TimeStep )
{
  SetFieldInterpTimeStepRank( FieldInterp, TimeStep, TimeStep );
}
//=============================================================================
void HOMARD_Iteration_i::SetFieldInterpTimeStepRank( const char* FieldInterp, CORBA::Long TimeStep, CORBA::Long Rank )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetFieldInterpTimeStepRank( FieldInterp, TimeStep, Rank );
}
//=============================================================================
HOMARD::listeFieldInterpTSRsIter* HOMARD_Iteration_i::GetFieldInterpsTimeStepRank()
{
  ASSERT( myHomardIteration );
  const std::list<std::string>& ListString = myHomardIteration->GetFieldInterpsTimeStepRank();
  HOMARD::listeFieldInterpTSRsIter_var aResult = new HOMARD::listeFieldInterpTSRsIter;
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
void HOMARD_Iteration_i::SetFieldInterp( const char* FieldInterp )
{
  myHomardIteration->SetFieldInterp( FieldInterp );
}
//=============================================================================
HOMARD::listeFieldInterpsIter* HOMARD_Iteration_i::GetFieldInterps()
{
  ASSERT( myHomardIteration );
  const std::list<std::string>& ListString = myHomardIteration->GetFieldInterps();
  HOMARD::listeFieldInterpsIter_var aResult = new HOMARD::listeFieldInterpsIter;
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
void HOMARD_Iteration_i::SupprFieldInterps()
{
  ASSERT( myHomardIteration );
  myHomardIteration->SupprFieldInterps();
}
//=============================================================================
void HOMARD_Iteration_i::SetLogFile( const char* LogFile )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetLogFile( LogFile );
}
//=============================================================================
char* HOMARD_Iteration_i::GetLogFile()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetLogFile().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::Compute(CORBA::Long etatMenage, CORBA::Long Option)
{
  MESSAGE ( "Compute : calcul d'une iteration, etatMenage = "<<etatMenage<<", Option = "<<Option );
  ASSERT( myHomardIteration );
//
// Nom de l'iteration
  char* IterName = GetName() ;
  CORBA::Long modeHOMARD = 1 ;
  CORBA::Long Option1 = 1 ;
  MESSAGE ( "Compute : calcul de l'teration " << IterName );
  return _gen_i->Compute(IterName, etatMenage, modeHOMARD, Option1, Option) ;
}
//=============================================================================
void HOMARD_Iteration_i::MeshInfo(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte)
{
  MESSAGE ( "MeshInfo : information sur le maillage associe a une iteration" );
  ASSERT( myHomardIteration );
//
  int Option = 1 ;
  MeshInfoOption( Qual, Diam, Conn, Tail, Inte, Option ) ;
//
  return ;
}
//=============================================================================
void HOMARD_Iteration_i::MeshInfoOption(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte, CORBA::Long Option)
{
  MESSAGE ( "MeshInfoOption : information sur le maillage associe a une iteration" );
  ASSERT( myHomardIteration );
//
// Nom de l'iteration
  char* IterName = GetName() ;
  CORBA::Long etatMenage = -1 ;
  CORBA::Long modeHOMARD = 7 ;
  if ( Qual != 0 ) { modeHOMARD = modeHOMARD*5 ; }
  if ( Diam != 0 ) { modeHOMARD = modeHOMARD*19 ; }
  if ( Conn != 0 ) { modeHOMARD = modeHOMARD*11 ; }
  if ( Tail != 0 ) { modeHOMARD = modeHOMARD*13 ; }
  if ( Inte != 0 ) { modeHOMARD = modeHOMARD*3 ; }
  MESSAGE ( "MeshInfoOption : information sur le maillage de l'iteration " << IterName );
  CORBA::Long Option2 = 1 ;
  CORBA::Long codret = _gen_i->Compute(IterName, etatMenage, modeHOMARD, Option, Option2) ;
  MESSAGE ( "MeshInfoOption : codret = " << codret );
//
  return ;
}
//=============================================================================
void HOMARD_Iteration_i::SetFileInfo( const char* FileInfo )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetFileInfo( FileInfo );
}
//=============================================================================
char* HOMARD_Iteration_i::GetFileInfo()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetFileInfo().c_str() );
}
//=============================================================================
//=============================================================================
// Liens avec les autres iterations
//=============================================================================
//=============================================================================
HOMARD::HOMARD_Iteration_ptr HOMARD_Iteration_i::NextIteration( const char* IterName )
{
// Nom de l'iteration parent
  char* NomIterParent = GetName() ;
  MESSAGE ( "NextIteration : creation de l'iteration " << IterName << " comme fille de " << NomIterParent );
  return _gen_i->CreateIteration(IterName, NomIterParent) ;
}
//=============================================================================
void HOMARD_Iteration_i::LinkNextIteration( const char* NomIteration )
{
  ASSERT( myHomardIteration );
  myHomardIteration->LinkNextIteration( NomIteration );
}
//=============================================================================
void HOMARD_Iteration_i::UnLinkNextIteration( const char* NomIteration )
{
  ASSERT( myHomardIteration );
  myHomardIteration->UnLinkNextIteration( NomIteration );
}
//=============================================================================
HOMARD::listeIterFilles* HOMARD_Iteration_i::GetIterations()
{
  ASSERT( myHomardIteration );
  const std::list<std::string>& maListe = myHomardIteration->GetIterations();
  HOMARD::listeIterFilles_var aResult = new HOMARD::listeIterFilles;
  aResult->length( maListe.size() );
  std::list<std::string>::const_iterator it;
  int i = 0;
  for ( it = maListe.begin(); it != maListe.end(); it++ )
  {
    aResult[i++] = CORBA::string_dup( (*it).c_str() );
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Iteration_i::SetIterParentName( const char* NomIterParent )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetIterParentName( NomIterParent );
}
//=============================================================================
char* HOMARD_Iteration_i::GetIterParentName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetIterParentName().c_str() );
}
//=============================================================================
HOMARD::HOMARD_Iteration_ptr HOMARD_Iteration_i::GetIterParent()
{
// Nom de l'iteration parent
  char* NomIterParent = GetIterParentName() ;
  MESSAGE ( "GetIterParent : NomIterParent = " << NomIterParent );
  return _gen_i->GetIteration(NomIterParent) ;
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetCaseName( const char* NomCas )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetCaseName( NomCas );
}
//=============================================================================
char* HOMARD_Iteration_i::GetCaseName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetCaseName().c_str() );
}
//=============================================================================
void HOMARD_Iteration_i::AssociateHypo( const char* NomHypo )
{
  ASSERT( myHomardIteration );
//
// Nom de l'iteration
  char* IterName = GetName() ;
  MESSAGE ( ". IterName = " << IterName );
  return _gen_i->AssociateIterHypo(IterName, NomHypo) ;
}
//=============================================================================
void HOMARD_Iteration_i::SetHypoName( const char* NomHypo )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetHypoName( NomHypo );
}
//=============================================================================
char* HOMARD_Iteration_i::GetHypoName()
{
  ASSERT( myHomardIteration );
  return CORBA::string_dup( myHomardIteration->GetHypoName().c_str() );
}
//=============================================================================
//=============================================================================
// Divers
//=============================================================================
//=============================================================================
void HOMARD_Iteration_i::SetInfoCompute( CORBA::Long MessInfo )
{
  ASSERT( myHomardIteration );
  myHomardIteration->SetInfoCompute( MessInfo );
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetInfoCompute()
{
  ASSERT( myHomardIteration );
  return myHomardIteration->GetInfoCompute() ;
}
