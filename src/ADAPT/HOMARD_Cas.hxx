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
//  File   : HOMARD_Cas.hxx
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

#ifndef _HOMARD_CAS_HXX_
#define _HOMARD_CAS_HXX_

#include "HOMARD.hxx"

#include <string>
#include <list>
#include <vector>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

class HOMARDIMPL_EXPORT HOMARD_Cas
{
public:
  HOMARD_Cas();
  ~HOMARD_Cas();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

// Caracteristiques
  int                           SetDirName( const char* NomDir );
  std::string                   GetDirName() const;

  int                           GetNumberofIter();

  void                          SetConfType( int ConfType );
  const int                     GetConfType() const;

  void                          SetExtType( int ExtType );
  const int                     GetExtType() const;

  void                          SetBoundingBox( const std::vector<double>& extremas );
  const std::vector<double>&    GetBoundingBox() const;

  void                          AddGroup( const char* Group);
  void                          SetGroups( const std::list<std::string>& ListGroup );
  const std::list<std::string>& GetGroups() const;
  void                          SupprGroups();

  void                          AddBoundary( const char* Boundary );
  void                          AddBoundaryGroup( const char* Boundary, const char* Group );
  const std::list<std::string>& GetBoundaryGroup() const;
  void                          SupprBoundaryGroup();

  void                          SetPyram( int Pyram );
  const int                     GetPyram() const;

// Liens avec les autres structures
  std::string                   GetIter0Name() const;

  void                          AddIteration( const char* NomIteration );
  const std::list<std::string>& GetIterations() const;
  void                          SupprIterations();

private:
  std::string                   _Name;
  std::string                   _NomDir;
  int                           _ConfType;
  int                           _ExtType;
  int                           _Etat;

  std::vector<double>           _Boite;         // cf HomardQTCommun pour structure du vecteur
  std::list<std::string>        _ListGroup;
  std::list<std::string>        _ListBoundaryGroup;

  int                           _Pyram;

  typedef std::string           IterName;
  typedef std::list<IterName>   IterNames;
  IterNames                     _ListIter;
};

#endif
