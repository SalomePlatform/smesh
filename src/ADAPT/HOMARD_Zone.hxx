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
//  File   : HOMARD_Zone.hxx
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

#ifndef _HOMARD_Zone_HXX_
#define _HOMARD_Zone_HXX_

#include "HOMARD.hxx"

#include <vector>
#include <string>
#include <list>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

class HOMARDIMPL_EXPORT HOMARD_Zone
{
public:
  HOMARD_Zone();
  ~HOMARD_Zone();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

// Caracteristiques
  void                          SetType( int Type );
  int                           GetType() const;

  void                          SetBox( double X0, double X1, double X2,
                                        double X3, double X4, double X5 );

  void                          SetCylinder( double X0, double X1, double X2, double X3,
                                             double X4, double X5, double X6, double X7 );

  void                          SetPipe( double X0, double X1, double X2, double X3,
                                         double X4, double X5, double X6, double X7, double X8 );

  void                          SetSphere( double X0, double X1, double X2, double X3 );

  std::vector<double>           GetCoords() const;

  void                          SetLimit( double X0, double X1, double X2 );
  std::vector<double>           GetLimit() const;

// Liens avec les autres structures
  void                          AddHypo( const char* NomHypo );
  const std::list<std::string>& GetHypo() const;
  void                          SupprHypo( const char* NomHypo );
  void                          SupprHypos();

private:
  std::string                   _Name;
  int                           _Type;
  std::list<std::string>        _ListHypo;
  double                        _Xmin, _Xmax, _Ymin, _Ymax, _Zmin, _Zmax;
  double                        _Xcentre, _Ycentre, _Zcentre, _Rayon, _Rayonint;
  double                        _Xaxe, _Yaxe, _Zaxe, _Haut;
  double                        _Xincr, _Yincr, _Zincr;
};

#endif
