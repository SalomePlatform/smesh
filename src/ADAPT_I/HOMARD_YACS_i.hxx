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

#ifndef _HOMARD_YACS_I_HXX_
#define _HOMARD_YACS_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(HOMARD_Gen)
#include CORBA_SERVER_HEADER(HOMARD_YACS)

#include "HOMARD_i.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <string>

class HOMARD_YACS;

class HOMARDENGINE_EXPORT HOMARD_YACS_i:
  public virtual Engines_Component_i,
  public virtual POA_HOMARD::HOMARD_YACS,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_YACS_i( CORBA::ORB_ptr orb,
                      HOMARD::HOMARD_Gen_var gen_i );
  HOMARD_YACS_i();

  virtual ~HOMARD_YACS_i();

// Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete( CORBA::Long Option );

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

// Caracteristiques
  void                   SetType( CORBA::Long Type );
  CORBA::Long            GetType();

  void                   SetMaxIter( CORBA::Long MaxIter );
  CORBA::Long            GetMaxIter();
  void                   SetMaxNode( CORBA::Long MaxNode );
  CORBA::Long            GetMaxNode();
  void                   SetMaxElem( CORBA::Long MaxElem );
  CORBA::Long            GetMaxElem();

  void                   SetTestConvergence( CORBA::Long TypeTest, CORBA::Double VRef );
  CORBA::Long            GetTestConvergenceType();
  CORBA::Double          GetTestConvergenceVRef();

  void                   SetDirName( const char* NomDir );
  char*                  GetDirName();

  void                   SetMeshFile( const char* MeshFile );
  char*                  GetMeshFile();

  void                   SetScriptFile( const char* ScriptFile );
  char*                  GetScriptFile();

  void                   SetXMLFile( const char* XMLFile );
  char*                  GetXMLFile();

  CORBA::Long            Write() ;
  CORBA::Long            WriteOnFile( const char* XMLFile ) ;

  // Liens avec les autres structures
  void                   SetCaseName( const char* NomCas );
  char*                  GetCaseName();

// Divers

private:
  ::HOMARD_YACS*    myHomardYACS;

  CORBA::ORB_ptr         _orb;
  HOMARD::HOMARD_Gen_var _gen_i;
};

#endif
