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
//  File   : HOMARD_Boundary.hxx
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

#ifndef _HOMARD_Boundary_HXX_
#define _HOMARD_Boundary_HXX_

#include "HOMARD.hxx"

#include <vector>
#include <string>
#include <list>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

class HOMARDIMPL_EXPORT HOMARD_Boundary
{
public:
  HOMARD_Boundary();
  ~HOMARD_Boundary();

// Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

  std::string                   GetDumpPython() const;

// Caracteristiques
  void                          SetType( int Type );
  int                           GetType() const;

  void                          SetMeshName( const char* MeshName );
  std::string                   GetMeshName() const;

  void                          SetDataFile( const char* DataFile );
  std::string                   GetDataFile() const;

  void                          SetCylinder( double X0, double X1, double X2, double X3,
                                             double X4, double X5, double X6 );
  void                          SetSphere( double X0, double X1, double X2, double X3 );
  void                          SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1,
                                          double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2);
  void                          SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle,
                                          double Xcentre, double Ycentre, double ZCentre);
  void                          SetTorus( double X0, double X1, double X2, double X3,
                                             double X4, double X5, double X6, double X7 );

  std::vector<double>           GetCoords() const;

  void                          SetLimit( double X0, double X1, double X2 );
  std::vector<double>           GetLimit() const;

  void                          AddGroup( const char* LeGroupe);
  void                          SetGroups(const std::list<std::string>& ListGroup );
  const std::list<std::string>& GetGroups() const;

// Liens avec les autres structures
  std::string                   GetCaseCreation() const;
  void                          SetCaseCreation( const char* NomCasCreation );

private:
  std::string                   _Name;
  std::string                   _NomCasCreation;
  std::string                   _DataFile;
  std::string                   _MeshName;
  int                           _Type;
  double                        _Xmin, _Xmax, _Ymin, _Ymax, _Zmin, _Zmax;
  double                        _Xaxe, _Yaxe, _Zaxe;
  double                        _Xcentre, _Ycentre, _Zcentre, _rayon;
  double                        _Xincr, _Yincr, _Zincr;
  double                        _Xcentre1, _Ycentre1, _Zcentre1, _Rayon1;
  double                        _Xcentre2, _Ycentre2, _Zcentre2, _Rayon2;
  double                        _Angle;

  std::list<std::string>        _ListGroupSelected;

};


#endif
