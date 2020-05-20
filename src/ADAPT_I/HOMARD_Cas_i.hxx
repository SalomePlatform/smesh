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

#ifndef _HOMARD_CAS_I_HXX_
#define _HOMARD_CAS_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(HOMARD_Gen)
#include CORBA_SERVER_HEADER(HOMARD_Cas)

#include "HOMARD_i.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <string>

class HOMARD_Cas;

class HOMARDENGINE_EXPORT HOMARD_Cas_i:
  public virtual Engines_Component_i,
  public virtual POA_HOMARD::HOMARD_Cas,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_Cas_i( CORBA::ORB_ptr orb, HOMARD::HOMARD_Gen_var gen_i );
  HOMARD_Cas_i();

  virtual ~HOMARD_Cas_i();

// Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete( CORBA::Long Option );

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

// Caracteristiques
  void                   SetDirName( const char* NomDir );
  char*                  GetDirName();

  CORBA::Long            GetState();

  CORBA::Long            GetNumberofIter();

  void                   SetConfType( CORBA::Long ConfType );
  CORBA::Long            GetConfType();

  void                   SetExtType( CORBA::Long ExtType );
  CORBA::Long            GetExtType();

  void                   SetBoundingBox( const HOMARD::extrema& LesExtremes );
  HOMARD::extrema*       GetBoundingBox();

  void                   AddGroup( const char* Group);
  void                   SetGroups(const HOMARD::ListGroupType& ListGroup);
  HOMARD::ListGroupType* GetGroups();

  void                   AddBoundary(const char* Boundary);
  void                   AddBoundaryGroup(const char* Boundary, const char* Group);
  HOMARD::ListBoundaryGroupType*  GetBoundaryGroup();
  void                   SupprBoundaryGroup( );

  void                   SetPyram( CORBA::Long Pyram );
  CORBA::Long            GetPyram();

  void                   MeshInfo(CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte);

// Liens avec les autres structures
  char*                  GetIter0Name();
  HOMARD::HOMARD_Iteration_ptr GetIter0() ;

  HOMARD::HOMARD_Iteration_ptr  NextIteration( const char* Name ) ;

  HOMARD::HOMARD_Iteration_ptr  LastIteration() ;

  void                   AddIteration( const char* NomIteration );

// YACS
  HOMARD::HOMARD_YACS_ptr CreateYACSSchema( const char* YACSName, const char* ScriptFile, const char* DirName, const char* MeshFile );

private:
  ::HOMARD_Cas*          myHomardCas;

  CORBA::ORB_ptr         _orb;
  HOMARD::HOMARD_Gen_var _gen_i;
};

#endif
