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
#include "HOMARD_Boundary_i.hxx"
#include "HOMARD_Gen_i.hxx"
#include "HOMARD_Boundary.hxx"
#include "HOMARD_DriverTools.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Boundary_i::HOMARD_Boundary_i()
{
  MESSAGE( "Default constructor, not for use" );
  ASSERT( 0 );
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Boundary_i::HOMARD_Boundary_i( CORBA::ORB_ptr orb,
			      HOMARD::HOMARD_Gen_var engine )
{
  MESSAGE( "HOMARD_Boundary_i" );
  _gen_i = engine;
  _orb = orb;
  myHomardBoundary = new ::HOMARD_Boundary();
  ASSERT( myHomardBoundary );
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Boundary_i::~HOMARD_Boundary_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetName( const char* Name )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetName( Name );
}
//=============================================================================
char* HOMARD_Boundary_i::GetName()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetName().c_str() );
}
//=============================================================================
CORBA::Long  HOMARD_Boundary_i::Delete()
{
  ASSERT( myHomardBoundary );
  char* BoundaryName = GetName() ;
  MESSAGE ( "Delete : destruction de la frontiere " << BoundaryName );
  return _gen_i->DeleteBoundary(BoundaryName) ;
}
//=============================================================================
char* HOMARD_Boundary_i::GetDumpPython()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetDumpPython().c_str() );
}
//=============================================================================
std::string HOMARD_Boundary_i::Dump() const
{
  return HOMARD::Dump( *myHomardBoundary );
}
//=============================================================================
bool HOMARD_Boundary_i::Restore( const std::string& stream )
{
  return HOMARD::Restore( *myHomardBoundary, stream );
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetType( CORBA::Long Type )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetType( Type );
}
//=============================================================================
CORBA::Long HOMARD_Boundary_i::GetType()
{
  ASSERT( myHomardBoundary );
  return  CORBA::Long( myHomardBoundary->GetType() );
}
//=============================================================================
void HOMARD_Boundary_i::SetMeshName( const char* MeshName )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetMeshName( MeshName );
}
//=============================================================================
char* HOMARD_Boundary_i::GetMeshName()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetMeshName().c_str() );
}
//=============================================================================
void HOMARD_Boundary_i::SetDataFile( const char* DataFile )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetDataFile( DataFile );
  int PublisMeshIN = _gen_i->GetPublisMeshIN () ;
  if ( PublisMeshIN != 0 ) { _gen_i->PublishResultInSmesh(DataFile, 0); }
}
//=============================================================================
char* HOMARD_Boundary_i::GetDataFile()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetDataFile().c_str() );
}
//=============================================================================
void HOMARD_Boundary_i::SetCylinder( double X0, double X1, double X2, double X3, double X4, double X5, double X6 )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetCylinder( X0, X1, X2, X3, X4, X5, X6 );
}
//=============================================================================
void HOMARD_Boundary_i::SetSphere( double Xcentre, double Ycentre, double ZCentre, double rayon )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetSphere( Xcentre, Ycentre, ZCentre, rayon );
}
//=============================================================================
void HOMARD_Boundary_i::SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1, double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2)
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetConeR( Xcentre1, Ycentre1, Zcentre1, Rayon1, Xcentre2, Ycentre2, Zcentre2, Rayon2 );
}
//=============================================================================
void HOMARD_Boundary_i::SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle, double Xcentre, double Ycentre, double Zcentre)
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetConeA( Xaxe, Yaxe, Zaxe, Angle, Xcentre, Ycentre, Zcentre );
}
//=============================================================================
void HOMARD_Boundary_i::SetTorus( double X0, double X1, double X2, double X3, double X4, double X5, double X6, double X7 )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetTorus( X0, X1, X2, X3, X4, X5, X6, X7 );
}
//=============================================================================
HOMARD::double_array* HOMARD_Boundary_i::GetCoords()
{
  ASSERT( myHomardBoundary );
  HOMARD::double_array_var aResult = new HOMARD::double_array();
  std::vector<double> mesCoor = myHomardBoundary->GetCoords();
  aResult->length( mesCoor .size() );
  std::vector<double>::const_iterator it;
  int i = 0;
  for ( it = mesCoor.begin(); it != mesCoor.end(); it++ )
    aResult[i++] = (*it);
  return aResult._retn();
}
//=============================================================================
void HOMARD_Boundary_i::SetLimit( double Xincr, double Yincr, double Zincr )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetLimit( Xincr, Yincr, Zincr );
}
//=============================================================================
HOMARD::double_array* HOMARD_Boundary_i::GetLimit()
{
  ASSERT( myHomardBoundary );
  HOMARD::double_array_var aResult = new HOMARD::double_array();
  std::vector<double> mesCoor = myHomardBoundary->GetLimit();
  aResult->length( mesCoor .size() );
  std::vector<double>::const_iterator it;
  int i = 0;
  for ( it = mesCoor.begin(); it != mesCoor.end(); it++ )
    aResult[i++] = (*it);
  return aResult._retn();
}
//=============================================================================
void HOMARD_Boundary_i::AddGroup( const char* Group)
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->AddGroup( Group );
}
//=============================================================================
void HOMARD_Boundary_i::SetGroups(const HOMARD::ListGroupType& ListGroup)
{
  ASSERT( myHomardBoundary );
  std::list<std::string> ListString;
  for ( int i = 0; i < ListGroup.length(); i++ )
  {
      ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardBoundary->SetGroups( ListString );
}
//=============================================================================
HOMARD::ListGroupType*  HOMARD_Boundary_i::GetGroups()
{
  ASSERT( myHomardBoundary );
  const std::list<std::string>& ListString = myHomardBoundary->GetGroups();
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
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetCaseCreation( const char* NomCaseCreation )
{
  ASSERT( myHomardBoundary );
  myHomardBoundary->SetCaseCreation( NomCaseCreation );
}
//=============================================================================
char* HOMARD_Boundary_i::GetCaseCreation()
{
  ASSERT( myHomardBoundary );
  return CORBA::string_dup( myHomardBoundary->GetCaseCreation().c_str() );
}



