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
//  File   : HOMARD_YACS.hxx
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

#ifndef _HOMARD_YACS_HXX_
#define _HOMARD_YACS_HXX_

#include "HOMARD.hxx"

#include <string>
#include <list>

class HOMARDIMPL_EXPORT HOMARD_YACS
{
public:
  HOMARD_YACS();
  ~HOMARD_YACS();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

// Caracteristiques
  void                          SetType( int Type );
  int                           GetType() const;

  void                          SetMaxIter( int MaxIter );
  int                           GetMaxIter() const;
  void                          SetMaxNode( int MaxNode );
  int                           GetMaxNode() const;
  void                          SetMaxElem( int MaxElem );
  int                           GetMaxElem() const;

  void                          SetTestConvergence( int TypeTest, double VRef );
  int                           GetTestConvergenceType() const;
  double                        GetTestConvergenceVRef() const;

  void                          SetDirName( const char* NomDir );
  std::string                   GetDirName() const;

  void                          SetMeshFile( const char* MeshFile );
  std::string                   GetMeshFile() const;

  void                          SetScriptFile( const char* ScriptFile );
  std::string                   GetScriptFile() const;

  void                          SetXMLFile( const char* XMLFile );
  std::string                   GetXMLFile() const;

// Liens avec les autres structures
  void                          SetCaseName( const char* NomCas );
  std::string                   GetCaseName() const;


// Divers

private:
  std::string                   _Name;
  std::string                   _NomCas;
  std::string                   _NomDir;
  std::string                   _MeshFile;
  std::string                   _ScriptFile;
  int                           _Type;
  int                           _MaxIter;
  int                           _MaxNode;
  int                           _MaxElem;
  int                           _TypeTest;
  double                        _VRef;
  std::string                   _XMLFile;
};

#endif
