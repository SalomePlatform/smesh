//  HOMARD HOMARD : implementation of HOMARD idl descriptions
//
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
//  File   : HOMARD_YACS.cxx
//  Author : Gerald NICOLAS, EDF
//  Module : HOMARD
//
// Remarques :
// L'ordre de description des fonctions est le meme dans tous les fichiers
// HOMARD_aaaa.idl, HOMARD_aaaa.hxx, HOMARD_aaaa.cxx, HOMARD_aaaa_i.hxx, HOMARD_aaaa_i.cxx :
// 1. Les generalites : Name, Delete, DumpPython, Dump, Restore
// 2. Les caracteristiques
// 3. Le lien avec les autres structures
//
// Quand les 2 fonctions Setxxx et Getxxx sont presentes, Setxxx est decrit en premier

#include "HOMARD_YACS.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================
HOMARD_YACS::HOMARD_YACS():
  _Name( "" ),
  _NomCas( "" ),
  _NomDir( "" ),
  _MeshFile( "" ),
  _ScriptFile( "" )
{
  MESSAGE("HOMARD_YACS");
}
//=============================================================================
/*!
 *
 */
//=============================================================================
HOMARD_YACS::~HOMARD_YACS()
{
  MESSAGE("~HOMARD_YACS");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_YACS::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_YACS::GetName() const
{
  return _Name;
}
//=============================================================================
std::string HOMARD_YACS::GetDumpPython() const
{
  MESSAGE (". Ecriture du schema " << _Name );
  std::ostringstream aScript;
  aScript << "\n# Creation of the schema " << _Name << "\n";
// Creation du schema
  aScript << "\t" << _Name << " = " << _NomCas << ".CreateYACSSchema(\"" << _Name << "\", \"" << _ScriptFile << "\", \"" << _NomDir << "\", \"" << _MeshFile << "\")\n";
// Le type de schema
  aScript << "\t" << _Name << ".SetType(" << _Type << ")\n";
// Les controles de convergences
  if ( _MaxIter > 0 ) { aScript << "\t" << _Name << ".SetMaxIter(" << _MaxIter << ")\n"; }
  if ( _MaxNode > 0 ) { aScript << "\t" << _Name << ".SetMaxNode(" << _MaxNode << ")\n"; }
  if ( _MaxElem > 0 ) { aScript << "\t" << _Name << ".SetMaxElem(" << _MaxElem << ")\n"; }

// L'ecriture du schema
  aScript << "\tcodret = " << _Name << ".Write()\n";

  MESSAGE (". Fin de l ecriture du schema " << _Name );

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques de la convergence
//=============================================================================
//=============================================================================
void HOMARD_YACS::SetType( int Type )
{
  _Type = Type;
}
//=============================================================================
int HOMARD_YACS::GetType() const
{
  return _Type;
}
//=============================================================================
void HOMARD_YACS::SetMaxIter( int MaxIter )
{
  _MaxIter = MaxIter;
}
//=============================================================================
int HOMARD_YACS::GetMaxIter() const
{
  return _MaxIter;
}
//=============================================================================
void HOMARD_YACS::SetMaxNode( int MaxNode )
{
  _MaxNode = MaxNode;
}
//=============================================================================
int HOMARD_YACS::GetMaxNode() const
{
  return _MaxNode;
}
//=============================================================================
void HOMARD_YACS::SetMaxElem( int MaxElem )
{
  _MaxElem = MaxElem;
}
//=============================================================================
int HOMARD_YACS::GetMaxElem() const
{
  return _MaxElem;
}
//=============================================================================
void HOMARD_YACS::SetTestConvergence( int TypeTest, double VRef )
{
  _TypeTest = TypeTest;
  _VRef = VRef;
}
//=============================================================================
int HOMARD_YACS::GetTestConvergenceType() const
{
  return _TypeTest;
}
//=============================================================================
double HOMARD_YACS::GetTestConvergenceVRef() const
{
  return _VRef;
}
//=============================================================================
//=============================================================================
// Repertoire et fichiers
//=============================================================================
//=============================================================================
//=============================================================================
void HOMARD_YACS::SetDirName( const char* NomDir )
{
  _NomDir = std::string( NomDir );
}
//=============================================================================
std::string HOMARD_YACS::GetDirName() const
{
   return _NomDir;
}
//=============================================================================
void HOMARD_YACS::SetMeshFile( const char* MeshFile )
{
  _MeshFile = std::string( MeshFile );
}
//=============================================================================
std::string HOMARD_YACS::GetMeshFile() const
{
  return _MeshFile;
}
//=============================================================================
void HOMARD_YACS::SetScriptFile( const char* ScriptFile )
{
  _ScriptFile = std::string( ScriptFile );
}
//=============================================================================
std::string HOMARD_YACS::GetScriptFile() const
{
  return _ScriptFile;
}
//=============================================================================
void HOMARD_YACS::SetXMLFile( const char* XMLFile )
{
  _XMLFile = std::string( XMLFile );
}
//=============================================================================
std::string HOMARD_YACS::GetXMLFile() const
{
  return _XMLFile;
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_YACS::SetCaseName( const char* NomCas )
{
  _NomCas = std::string( NomCas );
}
//=============================================================================
std::string HOMARD_YACS::GetCaseName() const
{
  return _NomCas;
}
//=============================================================================
//=============================================================================
// Divers
//=============================================================================
//=============================================================================
