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

#include "HOMARD_YACS_i.hxx"
#include "HOMARD_Gen_i.hxx"
#include "HOMARD_YACS.hxx"
#include "HOMARD_DriverTools.hxx"

#include "SALOMEDS_Tool.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_YACS_i::HOMARD_YACS_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_YACS_i::HOMARD_YACS_i( CORBA::ORB_ptr orb,
                                        HOMARD::HOMARD_Gen_var engine )
{
  MESSAGE("constructor");
  _gen_i = engine;
  _orb = orb;
  myHomardYACS = new ::HOMARD_YACS();
  ASSERT( myHomardYACS );
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_YACS_i::~HOMARD_YACS_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_YACS_i::SetName( const char* Name )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetName( Name );
}
//=============================================================================
char* HOMARD_YACS_i::GetName()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetName().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::Delete( CORBA::Long Option )
{
  ASSERT( myHomardYACS );
  char* YACSName = GetName() ;
  MESSAGE ( "Delete : destruction du schema " << YACSName << ", Option = " << Option );
  return _gen_i->DeleteYACS(YACSName, Option) ;
}
//=============================================================================
char* HOMARD_YACS_i::GetDumpPython()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_YACS_i::Dump() const
{
  return HOMARD::Dump( *myHomardYACS );
}
//=============================================================================
bool HOMARD_YACS_i::Restore( const std::string& stream )
{
  return HOMARD::Restore( *myHomardYACS, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_YACS_i::SetType( CORBA::Long Type )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetType( Type );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::GetType()
{
  ASSERT( myHomardYACS );
  return  CORBA::Long( myHomardYACS->GetType() );
}
//=============================================================================
//=============================================================================
// Caracteristiques de la convergence
//=============================================================================
//=============================================================================
void HOMARD_YACS_i::SetMaxIter( CORBA::Long MaxIter )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetMaxIter( MaxIter );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::GetMaxIter()
{
  ASSERT( myHomardYACS );
  return  CORBA::Long( myHomardYACS->GetMaxIter() );
}
//=============================================================================
void HOMARD_YACS_i::SetMaxNode( CORBA::Long MaxNode )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetMaxNode( MaxNode );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::GetMaxNode()
{
  ASSERT( myHomardYACS );
  return  CORBA::Long( myHomardYACS->GetMaxNode() );
}
//=============================================================================
void HOMARD_YACS_i::SetMaxElem( CORBA::Long MaxElem )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetMaxElem( MaxElem );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::GetMaxElem()
{
  ASSERT( myHomardYACS );
  return  CORBA::Long( myHomardYACS->GetMaxElem() );
}
//=============================================================================
void HOMARD_YACS_i::SetTestConvergence( CORBA::Long TypeTest, CORBA::Double VRef)
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetTestConvergence( TypeTest, VRef );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::GetTestConvergenceType()
{
  ASSERT( myHomardYACS );
  return  CORBA::Long( myHomardYACS->GetTestConvergenceType() );
}
//=============================================================================
CORBA::Double HOMARD_YACS_i::GetTestConvergenceVRef()
{
  ASSERT( myHomardYACS );
  return  CORBA::Long( myHomardYACS->GetTestConvergenceVRef() );
}
//=============================================================================
//=============================================================================
// Repertoire et fichiers
//=============================================================================
//=============================================================================
void HOMARD_YACS_i::SetDirName( const char* NomDir )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetDirName( NomDir );
}
//=============================================================================
char* HOMARD_YACS_i::GetDirName()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetDirName().c_str() );
}
//=============================================================================
void HOMARD_YACS_i::SetMeshFile( const char* MeshFile )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetMeshFile( MeshFile );
}
//=============================================================================
char* HOMARD_YACS_i::GetMeshFile()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetMeshFile().c_str() );
}
//=============================================================================
void HOMARD_YACS_i::SetScriptFile( const char* ScriptFile )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetScriptFile( ScriptFile );
}
//=============================================================================
char* HOMARD_YACS_i::GetScriptFile()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetScriptFile().c_str() );
}
//=============================================================================
void HOMARD_YACS_i::SetXMLFile( const char* XMLFile )
{
  ASSERT( myHomardYACS );
  MESSAGE ( "SetXMLFile : SetXMLFile = " << XMLFile );
  myHomardYACS->SetXMLFile( XMLFile );
}
//=============================================================================
char* HOMARD_YACS_i::GetXMLFile()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetXMLFile().c_str() );
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_YACS_i::SetCaseName( const char* NomCas )
{
  ASSERT( myHomardYACS );
  myHomardYACS->SetCaseName( NomCas );
}
//=============================================================================
char* HOMARD_YACS_i::GetCaseName()
{
  ASSERT( myHomardYACS );
  return CORBA::string_dup( myHomardYACS->GetCaseName().c_str() );
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::Write()
{
//
  char* nomYACS = GetName() ;
  return _gen_i->YACSWrite(nomYACS) ;
}
//=============================================================================
CORBA::Long HOMARD_YACS_i::WriteOnFile( const char* XMLFile )
{
  MESSAGE ( "WriteOnFile : ecriture du schema sur " << XMLFile );
  ASSERT( myHomardYACS );
//
// Memorisation du fichier associe
  SetXMLFile( XMLFile ) ;
//
// Nom du schema
  char* nomYACS = GetName() ;
  return _gen_i->YACSWriteOnFile(nomYACS, XMLFile) ;
}
//=============================================================================
//=============================================================================
//=============================================================================
// Divers
//=============================================================================
//=============================================================================
