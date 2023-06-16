//  HOMARD HOMARD : implementation of HOMARD idl descriptions
//
// Copyright (C) 2011-2023  CEA/DEN, EDF R&D
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
//  File   : HOMARD.hxx
//  Author : Gerald NICOLAS, EDF
//  Module : HOMARD

#ifndef _SMESH_HOMARD_ADAPT_HXX_
#define _SMESH_HOMARD_ADAPT_HXX_

#include "SMESH_SMESH.hxx"

#include <vector>
#include <string>
#include <list>

#include <iostream>
#include <fstream>

#if defined WIN32
#pragma warning ( disable: 4251 )
#endif

namespace SMESHHOMARDImpl
{

class SMESH_EXPORT HOMARD_Boundary
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

private:
  std::string                   _Name;
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

class SMESH_EXPORT HOMARD_Cas
{
public:
  HOMARD_Cas();
  ~HOMARD_Cas();

  // Generalites
  std::string                   GetDumpPython() const;

  // Caracteristiques
  int                           SetDirName( const char* NomDir );
  std::string                   GetDirName() const;

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

  void                          AddIteration( const char* NomIteration );

private:
  std::string                   _NomDir;
  int                           _Etat;

  std::vector<double>           _Boite;         // cf HomardQTCommun pour structure du vecteur
  std::list<std::string>        _ListGroup;
  std::list<std::string>        _ListBoundaryGroup;

  typedef std::string           IterName;
  typedef std::list<IterName>   IterNames;
  IterNames                     _ListIter;
};

class SMESH_EXPORT HomardDriver
{
public:
  HomardDriver(const std::string siter, const std::string siterp1);
  ~HomardDriver();
  //
  void        TexteInit( const std::string DirCompute, const std::string LogFile, const std::string Langue );
  void        TexteInfo( int TypeBila, int NumeIter );
  void        TexteMajCoords( int NumeIter );
  void        CreeFichierDonn();
  void        TexteAdap();
  void        CreeFichier();
  void        TexteMaillage( const std::string NomMesh, const std::string MeshFile, int apres );
  void        TexteMaillageHOMARD( const std::string Dir, const std::string liter, int apres );
  void        TexteConfRaffDera( int ConfType );

  void        TexteBoundaryOption( int BoundaryOption );
  void        TexteBoundaryCAOGr( const std::string GroupName );
  void        TexteBoundaryDi( const std::string MeshName, const std::string MeshFile );
  void        TexteBoundaryDiGr( const std::string GroupName );
  void        TexteBoundaryAn( const std::string NameBoundary, int NumeBoundary, int BoundaryType, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7 );
  void        TexteBoundaryAnGr( const std::string NameBoundary, int NumeBoundary, const std::string GroupName );

  void        TexteAdvanced( int NivMax, double DiamMin, int AdapInit, int LevelOutput );
  void        TexteInfoCompute( int MessInfo );
  //
  int         ExecuteHomard();

public:
  int         _modeHOMARD;
  std::string _HOMARD_Exec;
  std::string _NomDir;
  std::string _NomFichierConfBase;
  std::string _NomFichierConf;
  std::string _NomFichierDonn;
  std::string _siter;
  std::string _siterp1;
  std::string _Texte;
  bool _bLu;
};

class HOMARD_Iteration;

class SMESH_EXPORT HOMARD_Gen
{
public :
  HOMARD_Gen();
  ~HOMARD_Gen();
};

class SMESH_EXPORT HOMARD_Iteration
{
public:
  HOMARD_Iteration();
  ~HOMARD_Iteration();

  // Generalites
  void                          SetName( const char* Name );
  std::string                   GetName() const;

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

  void                          SetLogFile( const char* LogFile );
  std::string                   GetLogFile() const;

  void                          SetFileInfo( const char* FileInfo );
  std::string                   GetFileInfo() const;

  // Divers
  void                          SetInfoCompute( int MessInfo );
  int                           GetInfoCompute() const;

private:
  std::string                   _Name;
  int                           _Etat;
  int                           _NumIter;
  std::string                   _NomMesh;
  std::string                   _MeshFile;
  std::string                   _LogFile;
  std::string                   _NomDir;
  std::string                   _FileInfo;
  int                           _MessInfo;
};

} // namespace SMESHHOMARDImpl

#endif
