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
//  File   : HOMARD_Iteration.hxx
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

#ifndef _HOMARD_ITER_HXX_
#define _HOMARD_ITER_HXX_

#include "HOMARD.hxx"

#include <string>
#include <list>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

class HOMARDIMPL_EXPORT HOMARD_Iteration
{
public:
  HOMARD_Iteration();
  ~HOMARD_Iteration();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

// Caracteristiques
  void                          SetDirNameLoc( const char* NomDir );
  std::string                   GetDirNameLoc() const;

  void                          SetNumber( int NumIter );
  int                           GetNumber() const;

  void                          SetState( int etat );
  int                           GetState() const;

  void                          SetMeshName( const char* NomMesh );
  std::string                   GetMeshName() const;

  void                          SetMeshFile( const char* MeshFile );
  std::string                   GetMeshFile() const;

  void                          SetFieldFile( const char* FieldFile );
  std::string                   GetFieldFile() const;
// Instants pour le champ de pilotage
  void                          SetTimeStep( int TimeStep );
  void                          SetTimeStepRank( int TimeStep, int Rank );
  void                          SetTimeStepRankLast();
  int                           GetTimeStep() const;
  int                           GetRank() const;
// Instants pour un champ a interpoler
  void                          SetFieldInterpTimeStep( const char* FieldInterp, int TimeStep );
  void                          SetFieldInterpTimeStepRank( const char* FieldInterp, int TimeStep, int Rank );
  const std::list<std::string>& GetFieldInterpsTimeStepRank() const;
  void                          SetFieldInterp( const char* FieldInterp );
  const std::list<std::string>& GetFieldInterps() const;
  void                          SupprFieldInterps();

  void                          SetLogFile( const char* LogFile );
  std::string                   GetLogFile() const;

  void                          SetFileInfo( const char* FileInfo );
  std::string                   GetFileInfo() const;

// Liens avec les autres iterations
  void                          LinkNextIteration( const char* NomIteration );
  void                          UnLinkNextIteration( const char* NomIteration );
  void                          UnLinkNextIterations();
  const std::list<std::string>& GetIterations() const;

  void                          SetIterParentName( const char* iterParent );
  std::string                   GetIterParentName() const;

// Liens avec les autres structures
  void                          SetCaseName( const char* NomCas );
  std::string                   GetCaseName() const;

  void                          SetHypoName( const char* NomHypo );
  std::string                   GetHypoName() const;

// Divers
  void                          SetInfoCompute( int MessInfo );
  int                           GetInfoCompute() const;

private:
  std::string                   _Name;
  int                           _Etat;
  int                           _NumIter;
  std::string                   _NomMesh;
  std::string                   _MeshFile;
  std::string                   _FieldFile;
  int                           _TimeStep;
  int                           _Rank;
  std::string                   _LogFile;
  std::string                   _IterParent;
  std::string                   _NomHypo;
  std::string                   _NomCas;
  std::string                   _NomDir;
  std::list<std::string>        _mesIterFilles;
  std::string                   _FileInfo;
  int                           _MessInfo;
  // La liste des champs retenus par l'hypothese
  std::list<std::string>        _ListFieldInterp;
  // La liste des triplets (champs, pas de temps, numero d'ordre) retenus par l'iteration
  std::list<std::string>        _ListFieldInterpTSR;
};

#endif
