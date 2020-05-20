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
//  File   : HOMARD_Hypothesis.hxx
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

#ifndef _HOMARD_HYPOTHESIS_HXX_
#define _HOMARD_HYPOTHESIS_HXX_

#include "HOMARD.hxx"

#include <string>
#include <list>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

class HOMARDIMPL_EXPORT HOMARD_Hypothesis
{
public:
  HOMARD_Hypothesis();
  ~HOMARD_Hypothesis();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

// Caracteristiques
  void                          SetAdapType( int TypeAdap );
  int                           GetAdapType() const;
  void                          SetRefinTypeDera( int TypeRaff, int TypeDera );
  int                           GetRefinType() const;
  int                           GetUnRefType() const;

  void                          SetField( const char* FieldName );
  std::string                   GetFieldName() const;
  void                          SetUseField( int UsField );
  int                           GetUseField()    const;

  void                          SetUseComp( int UsCmpI );
  int                           GetUseComp()    const;
  void                          AddComp( const char* NomComp );
  void                          SupprComp( const char* NomComp );
  void                          SupprComps();
  const std::list<std::string>& GetComps() const;

  void                          SetRefinThr( int TypeThR, double ThreshR );
  int                           GetRefinThrType()   const;
  double                        GetThreshR()   const;
  void                          SetUnRefThr( int TypeThC, double ThreshC );
  int                           GetUnRefThrType()   const;
  double                        GetThreshC()   const;

  void                          SetNivMax( int NivMax );
  const int                     GetNivMax() const;

  void                          SetDiamMin( double DiamMin );
  const double                  GetDiamMin() const;

  void                          SetAdapInit( int AdapInit );
  const int                     GetAdapInit() const;

  void                          SetExtraOutput( int ExtraOutput );
  const int                     GetExtraOutput() const;

  void                          AddGroup( const char* Group);
  void                          SupprGroup( const char* Group );
  void                          SupprGroups();
  void                          SetGroups(const std::list<std::string>& ListGroup );
  const std::list<std::string>& GetGroups() const;

  void                          SetTypeFieldInterp( int TypeFieldInterp );
  int                           GetTypeFieldInterp() const;
  void                          AddFieldInterpType( const char* FieldInterp, int TypeInterp );
  void                          SupprFieldInterp( const char* FieldInterp );
  void                          SupprFieldInterps();
  const std::list<std::string>& GetFieldInterps() const;

// Liens avec les autres structures
  void                          SetCaseCreation( const char* NomCasCreation );
  std::string                   GetCaseCreation() const;

  void                          LinkIteration( const char* NomIter );
  void                          UnLinkIteration( const char* NomIter );
  void                          UnLinkIterations();
  const std::list<std::string>& GetIterations() const;

  void                          AddZone( const char* NomZone, int TypeUse );
  void                          SupprZone( const char* NomZone );
  void                          SupprZones();
  const std::list<std::string>& GetZones() const;

private:
  std::string                   _Name;
  std::string                   _NomCasCreation;

  int                           _TypeAdap; // -1 pour une adapation Uniforme,
                                           //  0 si l adaptation depend des zones,
                                           //  1 pour des champs

  int                           _TypeRaff;
  int                           _TypeDera;

  std::string                   _Field;
  int                           _TypeThR;
  int                           _TypeThC;
  double                        _ThreshR;
  double                        _ThreshC;
  int                           _UsField;
  int                           _UsCmpI;
  int                           _TypeFieldInterp; // 0 pour aucune interpolation,
                                                  // 1 pour interpolation de tous les champs,
                                                  // 2 pour une liste
  int                           _NivMax;
  double                        _DiamMin;
  int                           _AdapInit;
  int                           _ExtraOutput;

  std::list<std::string>        _ListIter;
  std::list<std::string>        _ListZone;
  std::list<std::string>        _ListComp;
  std::list<std::string>        _ListGroupSelected;
  std::list<std::string>        _ListFieldInterp;

};

#endif
