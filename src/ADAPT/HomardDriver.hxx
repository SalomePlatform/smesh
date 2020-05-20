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

# ifndef _HOMARDDRIVER_HXX_
# define _HOMARDDRIVER_HXX_

#include "HOMARD.hxx"

#include <iostream>
#include <fstream>

class HOMARDIMPL_EXPORT HomardDriver
{
public:
  HomardDriver(const std::string siter, const std::string siterp1);
  ~HomardDriver();
  //
  void        TexteInit( const std::string DirCompute, const std::string LogFile, const std::string Langue );
  void        TexteInfo( int TypeBila, int NumeIter );
  void        TexteMajCoords( int NumeIter );
  void        CreeFichierDonn();
  void        TexteAdap( int ExtType );
  void        CreeFichier();
  void        TexteMaillage( const std::string NomMesh, const std::string MeshFile, int apres );
  void        TexteMaillageHOMARD( const std::string Dir, const std::string liter, int apres );
  void        TexteConfRaffDera( int ConfType, int TypeAdap, int TypeRaff, int TypeDera );
  void        TexteZone( int NumeZone, int ZoneType, int TypeUse, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8 );
  void        TexteGroup( const std::string GroupName );
  void        TexteField( const std::string FieldName, const std::string FieldFile,
                          int TimeStep, int Rank,
                          int TypeThR, double ThreshR, int TypeThC, double ThreshC,
                          int UsField, int UsCmpI );
  void        TexteCompo( int NumeComp, const std::string NomCompo);

  void        TexteBoundaryOption( int BoundaryOption );
  void        TexteBoundaryCAOGr( const std::string GroupName );
  void        TexteBoundaryDi( const std::string MeshName, const std::string MeshFile );
  void        TexteBoundaryDiGr( const std::string GroupName );
  void        TexteBoundaryAn( const std::string NameBoundary, int NumeBoundary, int BoundaryType, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7 );
  void        TexteBoundaryAnGr( const std::string NameBoundary, int NumeBoundary, const std::string GroupName );

  void        TexteFieldInterp( const std::string FieldFile, const std::string MeshFile );
  void        TexteFieldInterpAll();
  void        TexteFieldInterpNameType( int NumeChamp, const std::string FieldName, const std::string TypeInterp, int TimeStep, int Rank );
  void        TexteAdvanced( int Pyram, int NivMax, double DiamMin, int AdapInit, int LevelOutput );
  void        TexteInfoCompute( int MessInfo );
  //
  int         ExecuteHomard(int option);
  //

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
  int         _TimeStep;
  int         _Rank;
  bool _bLu;

};

# endif         /* # ifndef _HOMARDDRIVER_HXX_ */
