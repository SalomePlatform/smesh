// SMESH HOMARD : implementation of SMESHHOMARD idl descriptions
//
// Copyright (C) 2011-2022  CEA/DEN, EDF R&D
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

#include "SMESH_Homard.hxx"

#include <Utils_SALOME_Exception.hxx>
#include <utilities.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

// La gestion des repertoires
#ifndef CHDIR
  #ifdef WIN32
    #define CHDIR _chdir
  #else
    #define CHDIR chdir
  #endif
#endif

namespace SMESHHOMARDImpl
{

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================
HOMARD_Boundary::HOMARD_Boundary():
  _Name( "" ),_Type( 1 ),
  _Xmin( 0 ), _Xmax( 0 ), _Ymin( 0 ), _Ymax( 0 ), _Zmin( 0 ), _Zmax( 0 ),
  _Xaxe( 0 ), _Yaxe( 0 ), _Zaxe( 0 ),
  _Xcentre( 0 ), _Ycentre( 0 ), _Zcentre( 0 ), _rayon( 0 ),
  _Xincr( 0 ), _Yincr( 0 ), _Zincr( 0 )
{
  MESSAGE("HOMARD_Boundary");
}

//=============================================================================
HOMARD_Boundary::~HOMARD_Boundary()
{
  MESSAGE("~HOMARD_Boundary");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Boundary::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_Boundary::GetName() const
{
  return _Name;
}
//=============================================================================
std::string HOMARD_Boundary::GetDumpPython() const
{
  std::ostringstream aScript;
  switch (_Type) {
    case -1:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryCAO(\"" << _Name << "\", ";
      aScript << "\"" << _DataFile << "\")\n";
      break ;
    }
    case 0:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryDi(\"" << _Name << "\", ";
      aScript << "\"" << _MeshName << "\", ";
      aScript << "\"" << _DataFile << "\")\n";
      break ;
    }
    case 1:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryCylinder(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _rayon << ")\n";
      break ;
    }
    case 2:
    {
      aScript << _Name << " = smeshhomard.CreateBoundarySphere(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _rayon << ")\n";
      break ;
    }
    case 3:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryConeA(\"" << _Name << "\", ";
      aScript << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _Angle << ", " << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ")\n";
      break ;
    }
    case 4:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryConeR(\"" << _Name << "\", ";
      aScript << _Xcentre1 << ", " << _Ycentre1 << ", " << _Zcentre1 << ", " << _Rayon1 << ", " << _Xcentre2 << ", " << _Ycentre2 << ", " << _Zcentre2 << ", " << _Rayon2 << ")\n";
      break ;
    }
    case 5:
    {
      aScript << _Name << " = smeshhomard.CreateBoundaryTorus(\"" << _Name << "\", ";
      aScript << _Xcentre << ", " << _Ycentre << ", " << _Zcentre << ", " << _Xaxe << ", " << _Yaxe << ", " << _Zaxe << ", " << _Rayon1 << ", " << _Rayon2 << ")\n";
      break ;
    }
  }

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Boundary::SetType( int Type )
{
  _Type = Type;
}
//=============================================================================
int HOMARD_Boundary::GetType() const
{
  return _Type;
}
//=============================================================================
void HOMARD_Boundary::SetMeshName( const char* MeshName )
{
  _MeshName = std::string( MeshName );
}
//=============================================================================
std::string HOMARD_Boundary::GetMeshName() const
{
  return _MeshName;
}
//=============================================================================
void HOMARD_Boundary::SetDataFile( const char* DataFile )
{
  _DataFile = std::string( DataFile );
}
//=============================================================================
std::string HOMARD_Boundary::GetDataFile() const
{
  return _DataFile;
}
//=======================================================================================
void HOMARD_Boundary::SetCylinder( double X0, double X1, double X2,
                                   double X3, double X4, double X5, double X6 )
{
  _Xcentre = X0; _Ycentre = X1; _Zcentre = X2;
  _Xaxe = X3; _Yaxe = X4; _Zaxe = X5;
  _rayon = X6;
}
//======================================================================
void HOMARD_Boundary::SetSphere( double X0, double X1, double X2, double X3 )
{
  _Xcentre = X0; _Ycentre = X1; _Zcentre = X2;
  _rayon = X3;
}
//======================================================================
void HOMARD_Boundary::SetConeR( double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1,
                                double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2)
{
  _Xcentre1 = Xcentre1; _Ycentre1 = Ycentre1; _Zcentre1 = Zcentre1;
  _Rayon1 = Rayon1;
  _Xcentre2 = Xcentre2; _Ycentre2 = Ycentre2; _Zcentre2 = Zcentre2;
  _Rayon2 = Rayon2;
}
//======================================================================
void HOMARD_Boundary::SetConeA( double Xaxe, double Yaxe, double Zaxe, double Angle,
                                double Xcentre, double Ycentre, double Zcentre)
{
  _Xaxe = Xaxe; _Yaxe = Yaxe; _Zaxe = Zaxe;
  _Angle = Angle;
  _Xcentre = Xcentre; _Ycentre = Ycentre; _Zcentre = Zcentre;
}
//=======================================================================================
void HOMARD_Boundary::SetTorus( double X0, double X1, double X2,
                                double X3, double X4, double X5, double X6, double X7 )
{
  _Xcentre = X0; _Ycentre = X1; _Zcentre = X2;
  _Xaxe = X3; _Yaxe = X4; _Zaxe = X5;
  _Rayon1 = X6;
  _Rayon2 = X7;
}
//=======================================================================================
std::vector<double> HOMARD_Boundary::GetCoords() const
{
  std::vector<double> mesCoor;
  switch (_Type)
  {
    //  Cylindre
    case 1:
    {
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      mesCoor.push_back( _Xaxe );
      mesCoor.push_back( _Yaxe );
      mesCoor.push_back( _Zaxe );
      mesCoor.push_back( _rayon );
      break ;
    }
    //  Sphere
    case 2:
    {
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      mesCoor.push_back( _rayon );
      break ;
    }
    //  Cone defini par un axe et un angle
    case 3:
    {
      mesCoor.push_back( _Xaxe );
      mesCoor.push_back( _Yaxe );
      mesCoor.push_back( _Zaxe );
      mesCoor.push_back( _Angle );
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      break ;
    }
    //  Cone defini par les 2 rayons
    case 4:
    {
      mesCoor.push_back( _Xcentre1 );
      mesCoor.push_back( _Ycentre1 );
      mesCoor.push_back( _Zcentre1 );
      mesCoor.push_back( _Rayon1 );
      mesCoor.push_back( _Xcentre2 );
      mesCoor.push_back( _Ycentre2 );
      mesCoor.push_back( _Zcentre2 );
      mesCoor.push_back( _Rayon2 );
      break ;
    }
    //  Tore
    case 5:
    {
      mesCoor.push_back( _Xcentre );
      mesCoor.push_back( _Ycentre );
      mesCoor.push_back( _Zcentre );
      mesCoor.push_back( _Xaxe );
      mesCoor.push_back( _Yaxe );
      mesCoor.push_back( _Zaxe );
      mesCoor.push_back( _Rayon1 );
      mesCoor.push_back( _Rayon2 );
      break ;
    }
    default:
      break ;
  }
  return mesCoor;
}
//======================================================================
void HOMARD_Boundary::SetLimit( double X0, double X1, double X2 )
{
  _Xincr = X0; _Yincr = X1; _Zincr = X2;
}
//=======================================================================================
std::vector<double> HOMARD_Boundary::GetLimit() const
{
  std::vector<double> mesLimit;
  mesLimit.push_back( _Xincr );
  mesLimit.push_back( _Yincr );
  mesLimit.push_back( _Zincr );
  return mesLimit;
}
//=============================================================================
void HOMARD_Boundary::AddGroup( const char* Group)
{
  _ListGroupSelected.push_back(Group);
}
//=============================================================================
void HOMARD_Boundary::SetGroups( const std::list<std::string>& ListGroup )
{
  _ListGroupSelected.clear();
  std::list<std::string>::const_iterator it = ListGroup.begin();
  while(it != ListGroup.end())
    _ListGroupSelected.push_back((*it++));
}
//=============================================================================
const std::list<std::string>& HOMARD_Boundary::GetGroups() const
{
  return _ListGroupSelected;
}
//=============================================================================

//=============================================================================
/*!
 *  default constructor:
 *  Par defaut, l'adaptation est conforme, sans suivi de frontiere
 */
//=============================================================================
HOMARD_Cas::HOMARD_Cas():
  _NomDir("/tmp")
{
  MESSAGE("HOMARD_Cas");
}
//=============================================================================
HOMARD_Cas::~HOMARD_Cas()
//=============================================================================
{
  MESSAGE("~HOMARD_Cas");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
std::string HOMARD_Cas::GetDumpPython() const
{
  std::ostringstream aScript;
  // Suivi de frontieres
  std::list<std::string>::const_iterator it = _ListBoundaryGroup.begin();
  while (it != _ListBoundaryGroup.end()) {
    aScript << "smeshhomard.AddBoundaryGroup(\"" << *it << "\", \"";
    it++;
    aScript << *it << "\")\n";
    it++;
  }

  return aScript.str();
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
int HOMARD_Cas::SetDirName( const char* NomDir )
{
  int erreur = 0 ;
  // On vérifie qu'aucun calcul n'a eu lieu pour ce cas
  if ( _ListIter.size() > 1 ) { erreur = 1 ; }
  // Creation
  if ( CHDIR(NomDir) == 0 ) {
    _NomDir = std::string( NomDir );
  }
  else {
#ifndef WIN32
    if ( mkdir(NomDir, S_IRWXU|S_IRGRP|S_IXGRP) == 0 )
#else
    if ( _mkdir(NomDir) == 0 )
#endif
    {
      if ( CHDIR(NomDir) == 0 ) { _NomDir = std::string( NomDir ); }
      else                      { erreur = 2 ; }
    }
    else { erreur = 2 ; }
  }
  return erreur;
}
//=============================================================================
std::string HOMARD_Cas::GetDirName() const
{
  return _NomDir;
}
//
// La boite englobante
//
//=============================================================================
void HOMARD_Cas::SetBoundingBox( const std::vector<double>& extremas )
{
  _Boite.clear();
  _Boite.resize( extremas.size() );
  for ( unsigned int i = 0; i < extremas.size(); i++ )
    _Boite[i] = extremas[i];
}
//=============================================================================
const std::vector<double>& HOMARD_Cas::GetBoundingBox() const
{
  return _Boite;
}
//
// Les groupes
//
//=============================================================================
void HOMARD_Cas::AddGroup( const char* Group )
{
  _ListGroup.push_back(Group);
}
//=============================================================================
void HOMARD_Cas::SetGroups( const std::list<std::string>& ListGroup )
{
  _ListGroup.clear();
  std::list<std::string>::const_iterator it = ListGroup.begin();
  while(it != ListGroup.end())
  {
    _ListGroup.push_back((*it++));
  }
}
//=============================================================================
const std::list<std::string>& HOMARD_Cas::GetGroups() const
{
  return _ListGroup;
}
//=============================================================================
void HOMARD_Cas::SupprGroups()
{
  _ListGroup.clear();
}
//
// Les frontieres
//
//=============================================================================
void HOMARD_Cas::AddBoundary( const char* Boundary )
{
//   MESSAGE ( ". HOMARD_Cas::AddBoundary : Boundary = " << Boundary );
  const char* Group = "";
  AddBoundaryGroup( Boundary, Group );
}
//=============================================================================
void HOMARD_Cas::AddBoundaryGroup( const char* Boundary, const char* Group )
{
//   MESSAGE ( ". HOMARD_Cas::AddBoundaryGroup : Boundary = " << Boundary );
//   MESSAGE ( ". HOMARD_Cas::AddBoundaryGroup : Group = " << Group );
  _ListBoundaryGroup.push_back( Boundary );
  _ListBoundaryGroup.push_back( Group    );
}
//=============================================================================
const std::list<std::string>& HOMARD_Cas::GetBoundaryGroup() const
{
  return _ListBoundaryGroup;
}
//=============================================================================
void HOMARD_Cas::SupprBoundaryGroup()
{
  _ListBoundaryGroup.clear();
}
//=============================================================================
//=============================================================================
// Liens avec les autres structures
//=============================================================================
//=============================================================================
void HOMARD_Cas::AddIteration( const char* NomIteration )
{
  _ListIter.push_back( std::string( NomIteration ) );
}

//=============================================================================
//=============================================================================
HomardDriver::HomardDriver(const std::string siter, const std::string siterp1):
  _HOMARD_Exec( "" ), _NomDir( "" ), _NomFichierConfBase( "HOMARD.Configuration" ),
  _NomFichierConf( "" ), _NomFichierDonn( "" ), _siter( "" ), _siterp1( "" ),
  _Texte( "" ), _bLu( false )
{
  MESSAGE("siter = "<<siter<<", siterp1 = "<<siterp1);
  // Le repertoire ou se trouve l'executable HOMARD
  std::string dir ;
  // TODO?
  if ( getenv("HOMARD_ROOT_DIR") != NULL ) { dir = getenv("HOMARD_ROOT_DIR") ; }
  dir += "/bin/salome";
  MESSAGE("dir ="<<dir);
  // L'executable HOMARD
  std::string executable = "homard";
  MESSAGE("executable ="<<executable);
  // Memorisation du nom complet de l'executable HOMARD
  _HOMARD_Exec = dir + "/" + executable ;
  MESSAGE("==> _HOMARD_Exec ="<<_HOMARD_Exec) ;
  //
  _siter = siter ;
  _siterp1 = siterp1 ;
}
//=============================================================================
//=============================================================================
HomardDriver::~HomardDriver()
{
}
//===============================================================================
// A. Generalites
//===============================================================================
void HomardDriver::TexteInit( const std::string DirCompute, const std::string LogFile, const std::string Langue )
{
  MESSAGE("TexteInit, DirCompute ="<<DirCompute<<", LogFile ="<<LogFile);
//
  _Texte  = "ListeStd \"" + LogFile + "\"\n" ;
  _Texte += "RepeTrav \"" + DirCompute + "\"\n" ;
  _Texte += "RepeInfo \"" + DirCompute + "\"\n" ;
  _Texte += "Langue \"" + Langue + "\"\n" ;
//
}
//===============================================================================
void HomardDriver::TexteAdap()
{
  MESSAGE("TexteAdap");

  _Texte += "Action   homa\n";
  _Texte += "CCAssoci med\n";
  _Texte += "ModeHOMA 1\n";
  _Texte += "NumeIter " + _siter + "\n";
  _modeHOMARD = 1;
}
//===============================================================================
void HomardDriver::TexteInfo( int TypeBila, int NumeIter )
{
  MESSAGE("TexteInfo: TypeBila ="<<TypeBila<<", NumeIter ="<<NumeIter);
//
  _Texte += "ModeHOMA 2\n" ;
  std::stringstream saux1 ;
  saux1 << TypeBila ;
  std::string saux2 = saux1.str() ;
  _Texte += "TypeBila " + saux2 + "\n" ;
  if ( NumeIter ==  0 )
  {
    _Texte += "NumeIter 0\n" ;
    _Texte += "Action   info_av\n" ;
    _Texte += "CCAssoci med\n" ;
  }
  else
  {
    _Texte += "NumeIter " + _siter + "\n" ;
    _Texte += "Action   info_ap\n" ;
    _Texte += "CCAssoci homard\n" ;
  }
  _modeHOMARD = 2 ;
//
}
//===============================================================================
void HomardDriver::TexteMajCoords( int NumeIter )
{
  MESSAGE("TexteMajCoords: NumeIter ="<<NumeIter);
//
  _Texte += "ModeHOMA 5\n" ;
  _Texte += "NumeIter " + _siterp1 + "\n" ;
  _Texte += "Action   homa\n" ;
  _Texte += "CCAssoci med\n" ;
  _Texte += "EcriFiHO N_SANS_FRONTIERE\n" ;
  _modeHOMARD = 5 ;
//
}
//===============================================================================
// B. Les maillages en entree et en sortie
//===============================================================================
void HomardDriver::TexteMaillage( const std::string NomMesh, const std::string MeshFile, int apres )
{
  MESSAGE("TexteMaillage, NomMesh  = "<<NomMesh);
  MESSAGE("TexteMaillage, MeshFile = "<<MeshFile);
  MESSAGE("TexteMaillage, apres = "<<apres);
  std::string saux ;
  saux = "P1" ;
  if ( apres < 1 ) { saux = "__" ; }

  _Texte += "# Maillages Med " + saux + "\n" ;
  _Texte += "CCNoMN" + saux + " \"" + NomMesh  + "\"\n" ;
  _Texte += "CCMaiN" + saux + " \"" + MeshFile + "\"\n" ;
}

//===============================================================================
void HomardDriver::TexteMaillageHOMARD( const std::string Dir, const std::string liter, int apres )
{
  MESSAGE("TexteMaillageHOMARD, Dir ="<<Dir<<", liter ="<<liter<<", apres ="<<apres);
  std::string saux ;
  if ( apres < 1 ) { saux = "__" ; }
  else             { saux = "P1" ; }

  _Texte += "# Maillage HOMARD " + liter + "\n" ;
  _Texte += "HOMaiN" + saux + " Mai" + liter   + " \"" + Dir + "/maill." + liter   + ".hom.med\"\n" ;
}

//===============================================================================
// C. Le pilotage de l'adaptation
//===============================================================================
void HomardDriver::TexteConfRaffDera( int ConfType )
{
  MESSAGE("TexteConfRaffDera, ConfType = " << ConfType);
  //
  // Type de conformite
  //
  std::string saux;
  switch (ConfType)
  {
    case -2: //
    {
      saux = "NON_CONFORME_1_ARETE" ;
      break;
    }
    case -1: //
    {
      saux = "CONFORME_BOITES" ;
      break;
    }
    case 0: //
    {
      saux = "CONFORME" ;
      break;
    }
    case 1: //
    {
      saux = "NON_CONFORME" ;
      break;
    }
    case 2: //
    {
      saux = "NON_CONFORME_1_NOEUD" ;
      break;
    }
    case 3: //
    {
      saux = "NON_CONFORME_INDICATEUR" ;
      break;
    }
  }
  _Texte += "# Type de conformite\nTypeConf " + saux + "\n" ;
  //
  // Type de raffinement/deraffinement
  //
  saux = "TypeRaff uniforme\n" ;
  saux += "TypeDera non" ;
  _Texte += "# Type de raffinement/deraffinement\n" + saux + "\n" ;
}
//===============================================================================
// D. Les frontieres
//===============================================================================
void HomardDriver::TexteBoundaryOption( int BoundaryOption )
{
  MESSAGE("TexteBoundaryOption, BoundaryOption = "<<BoundaryOption);
//
// Type de suivi de frontiere
//
  std::stringstream saux1 ;
  saux1 << BoundaryOption ;
  std::string saux = saux1.str() ;
  _Texte += "SuivFron " + saux + "\n" ;
//
}//===============================================================================
void HomardDriver::TexteBoundaryCAOGr(  const std::string GroupName )
{
  MESSAGE("TexteBoundaryCAOGr, GroupName  = "<<GroupName);
//
  _Texte += "GrFroCAO \"" + GroupName + "\"\n" ;
//
}

//===============================================================================
void HomardDriver::TexteBoundaryDi(  const std::string MeshName, const std::string MeshFile )
{
  MESSAGE("TexteBoundaryDi, MeshName  = "<<MeshName);
  MESSAGE("TexteBoundaryDi, MeshFile  = "<<MeshFile);
//
  _Texte += "#\n# Frontiere discrete\n" ;
  _Texte += "CCNoMFro \"" + MeshName + "\"\n" ;
  _Texte += "CCFronti \"" + MeshFile + "\"\n" ;
//
}
//===============================================================================
void HomardDriver::TexteBoundaryDiGr(  const std::string GroupName )
{
  MESSAGE("TexteBoundaryDiGr, GroupName  = "<<GroupName);
//
  _Texte += "CCGroFro \"" + GroupName + "\"\n" ;
//
}
//===============================================================================
void HomardDriver::TexteBoundaryAn( const std::string NameBoundary, int NumeBoundary, int BoundaryType, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7 )
{
  MESSAGE("TexteBoundaryAn, NameBoundary = "<<NameBoundary);
//   MESSAGE("TexteBoundaryAn, NumeBoundary = "<<NumeBoundary);
  MESSAGE("TexteBoundaryAn, BoundaryType = "<<BoundaryType);
//   MESSAGE("TexteBoundaryAn, coor         = "<< x0<<","<<x1<< ","<< x2<< ","<< x3<<","<<x4<<","<<x5<<","<<x6","<<x7);
//
  std::string saux, saux2 ;
//
// Commentaires
//
  std::stringstream saux1 ;
  saux1 << NumeBoundary ;
  saux2 = saux1.str() ;
  saux = "#\n# Frontiere numero " + saux2 + "\n" ;
  if ( BoundaryType == 1 )
  { saux += "# Cylindre\n" ; }
  if ( BoundaryType == 2 )
  { saux += "# Sphere\n" ; }
  if ( BoundaryType == 3 || BoundaryType == 4 )
  { saux += "# Cone\n" ; }
  if ( BoundaryType == 5 )
  { saux += "# Tore\n" ; }
//
// Le nom de la frontiere
//
  { std::stringstream saux1 ;
    saux1 << NumeBoundary ;
    saux += "FANom " + saux1.str() + " \"" + NameBoundary + "\"\n" ;
  }
//
// Type de frontiere
//
  { std::stringstream saux1 ;
    saux1 << NumeBoundary << " " << BoundaryType ;
    saux += "FAType " + saux1.str() + "\n" ;
  }
//
// Cas du cylindre
//
  if ( BoundaryType == 1 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux2 = saux1.str() ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FAXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FARayon " + saux1.str() + "\n" ;
    }
 }
//
// Cas de la sphere
//
  else if ( BoundaryType == 2 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FARayon " + saux1.str() + "\n" ;
    }
  }
//
// Cas du cone defini par un axe et un angle
//
  if ( BoundaryType == 3 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux += "FAXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FAAngle " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
 }
//
// Cas du cone defini par les 2 rayons
//
  if ( BoundaryType == 4 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FARayon " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAXCen2 " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAYCen2 " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FAZCen2 " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x7 ;
      saux += "FARayon2 " + saux1.str() + "\n" ;
    }
 }
//
// Cas du tore
//
  if ( BoundaryType == 5 )
  {
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x0 ;
      saux2 = saux1.str() ;
      saux += "FAXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x1 ;
      saux += "FAYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x2 ;
      saux += "FAZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x3 ;
      saux += "FAXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x4 ;
      saux += "FAYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x5 ;
      saux += "FAZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x6 ;
      saux += "FARayon  " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeBoundary << " " << x7 ;
      saux += "FARayon2 " + saux1.str() + "\n" ;
    }
 }
//
  _Texte += saux + "#\n" ;
//
}
//===============================================================================
void HomardDriver::TexteBoundaryAnGr( const std::string NameBoundary, int NumeBoundary, const std::string GroupName )
{
  MESSAGE("TexteBoundaryAnGr, NameBoundary  = "<<NameBoundary);
//   MESSAGE("TexteBoundaryAnGr, NumeBoundary  = "<<NumeBoundary);
//   MESSAGE("TexteBoundaryAnGr, GroupName  = "<<GroupName);
//
// Commentaires
//
  std::string saux, saux2 ;
  std::stringstream saux1 ;
  saux1 << NumeBoundary ;
  saux2 = saux1.str() ;
  saux = "#\n# Lien Frontiere/Groupe numero " + saux2 + "\n" ;
//
  saux += "FGNomFro " + saux2 + " \"" + NameBoundary + "\"\n" ;
  saux += "FGNomGro " + saux2 + " \"" + GroupName + "\"\n" ;
//
  _Texte += saux + "#\n" ;
//
}
//===============================================================================
// F. Les options avancees
//===============================================================================
void HomardDriver::TexteAdvanced( int NivMax, double DiamMin, int AdapInit, int ExtraOutput )
{
  MESSAGE("TexteAdvanced, NivMax ="<<NivMax<<", DiamMin ="<<DiamMin<<
          ", AdapInit ="<<AdapInit<<", ExtraOutput ="<<ExtraOutput);

  if ( NivMax > 0 )
  {
    _Texte += "# Niveaux extremes\n" ;
    { std::stringstream saux1 ;
      saux1 << NivMax ;
      _Texte += "NiveauMa " + saux1.str() + "\n" ;
    }
  }
  if ( DiamMin > 0 )
  {
    _Texte += "# Diametre minimal\n" ;
    { std::stringstream saux1 ;
      saux1 << DiamMin ;
      _Texte += "DiametMi " + saux1.str()  + "\n" ;
    }
  }
  if ( AdapInit != 0 )
  {
    if ( AdapInit > 0 )
    { _Texte += "# Raffinement" ; }
    else
    { _Texte += "# Deraffinement" ; }
    _Texte += " des regions sans indicateur\n" ;
    { std::stringstream saux1 ;
      saux1 << AdapInit ;
      _Texte += "AdapInit " + saux1.str() + "\n" ;
    }
  }
  if ( ExtraOutput % 2 == 0 )
  {
    _Texte += "# Sortie des niveaux de raffinement\n" ;
    _Texte += "NCNiveau NIVEAU\n" ;
  }
  if ( ExtraOutput % 3 == 0 )
  {
    _Texte += "# Sortie des qualités des mailles\n" ;
    _Texte += "NCQualit QUAL\n" ;
  }
  if ( ExtraOutput % 5 == 0 )
  {
    _Texte += "# Sortie des diamètres des mailles\n" ;
    _Texte += "NCDiamet DIAM\n" ;
  }
  if ( ExtraOutput % 7 == 0 )
  {
    _Texte += "# Sortie des parents des mailles\n" ;
    _Texte += "NCParent PARENT\n" ;
  }
  if ( ExtraOutput % 11 == 0 )
  {
    _Texte += "# Volumes voisins par recollement\n" ;
    _Texte += "NCVoisRc Voisin-Recollement\n" ;
  }
}
//===============================================================================
// G. Les messages
//===============================================================================
void HomardDriver::TexteInfoCompute( int MessInfo )
{
  MESSAGE("TexteAdvanced, MessInfo ="<<MessInfo);

  if ( MessInfo != 0 )
  {
     _Texte += "# Messages d'informations\n" ;
    { std::stringstream saux1 ;
      saux1 << MessInfo ;
      _Texte += "MessInfo " + saux1.str()  + "\n" ;
    }
   }
}
//===============================================================================
void HomardDriver::CreeFichier( )
{
//
  if ( _modeHOMARD == 1 )
  { _NomFichierConf = _NomFichierConfBase + "." + _siter + ".vers." + _siterp1 ; }
  else if ( _modeHOMARD == 2 )
  { _NomFichierConf = _NomFichierConfBase + "." + _siter + ".info" ; }
  else if ( _modeHOMARD == 5 )
  { _NomFichierConf = _NomFichierConfBase + ".majc" ; }
//
  std::ofstream Fic(_NomFichierConf.c_str(), std::ios::out ) ;
  if (Fic.is_open() == true) { Fic << _Texte << std::endl ; }
  Fic.close() ;
//
}
//===============================================================================
// Creation du fichier de donnees pour l'information
//===============================================================================
void HomardDriver::CreeFichierDonn( )
{
//
  MESSAGE("CreeFichierDonn");
  _NomFichierDonn = "info.donn" ;
//
  std::string data ;
  data  = "0\n" ;
  data += "0\n" ;
  data += "q\n" ;
  std::ofstream Fic(_NomFichierDonn.c_str(), std::ios::out ) ;
  if (Fic.is_open() == true) { Fic << data << std::endl ; }
  Fic.close() ;
//
}
//===============================================================================
int HomardDriver::ExecuteHomard()
{
  MESSAGE("ExecuteHomard");
  std::string commande ;
  int codret ;
  // Copie des Fichiers HOMARD
  commande = "cp " + _NomFichierConf + " " + _NomFichierConfBase ;
  codret = system(commande.c_str()) ;

// Execution de HOMARD
  if ( codret == 0)
  {
    commande = _HOMARD_Exec.c_str() ;
    if ( _NomFichierDonn != "" ) { commande += " < " + _NomFichierDonn ; }
    codret = system(commande.c_str());
    if ( codret != 0) { MESSAGE ( "Erreur en executant HOMARD : " << codret ); };
    _NomFichierDonn = "" ;
  };
  return codret ;
}

//=============================================================================
//=============================================================================
HOMARD_Gen::HOMARD_Gen()
{
  MESSAGE("HOMARD_Gen");
}

//=============================================================================
//=============================================================================
HOMARD_Gen::~HOMARD_Gen()
{
  MESSAGE("~HOMARD_Gen");
}
//=============================================================================

//=============================================================================
/*!
 *  default constructor:
 */
//=============================================================================
HOMARD_Iteration::HOMARD_Iteration():
  _Name( "" ),
  _Etat( 0 ),
  _NumIter( -1 ),
  _NomMesh( "" ),
  _MeshFile( "" ),
  _LogFile( "" ),
  _NomDir( "" ),
  _FileInfo( "" ),
  _MessInfo( 1 )
{
  MESSAGE("HOMARD_Iteration");
}
//=============================================================================
/*!
 *
 */
//=============================================================================
HOMARD_Iteration::~HOMARD_Iteration()
{
  MESSAGE("~HOMARD_Iteration");
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Iteration::SetName( const char* Name )
{
  _Name = std::string( Name );
}
//=============================================================================
std::string HOMARD_Iteration::GetName() const
{
  return _Name;
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Iteration::SetDirNameLoc( const char* NomDir )
{
  _NomDir = std::string( NomDir );
}
//=============================================================================
std::string HOMARD_Iteration::GetDirNameLoc() const
{
   return _NomDir;
}
//=============================================================================
void HOMARD_Iteration::SetNumber( int NumIter )
{
  _NumIter = NumIter;
}
//=============================================================================
int HOMARD_Iteration::GetNumber() const
{
  return _NumIter;
}
//=============================================================================
void HOMARD_Iteration::SetState( int etat )
{
  _Etat = etat;
}
//=============================================================================
int HOMARD_Iteration::GetState() const
{
  return _Etat;
}
//=============================================================================
void HOMARD_Iteration::SetMeshName( const char* NomMesh )
{
  _NomMesh = std::string( NomMesh );
}
//=============================================================================
std::string HOMARD_Iteration::GetMeshName() const
{
  return _NomMesh;
}
//=============================================================================
void HOMARD_Iteration::SetMeshFile( const char* MeshFile )
{
  _MeshFile = std::string( MeshFile );
}
//=============================================================================
std::string HOMARD_Iteration::GetMeshFile() const
{
  return _MeshFile;
}
//=============================================================================
void HOMARD_Iteration::SetLogFile( const char* LogFile )
{
  _LogFile = std::string( LogFile );
}
//=============================================================================
std::string HOMARD_Iteration::GetLogFile() const
{
  return _LogFile;
}
//=============================================================================
void HOMARD_Iteration::SetFileInfo( const char* FileInfo )
{
  _FileInfo = std::string( FileInfo );
}
//=============================================================================
std::string HOMARD_Iteration::GetFileInfo() const
{
  return _FileInfo;
}
//=============================================================================
void HOMARD_Iteration::SetInfoCompute( int MessInfo )
{
  _MessInfo = MessInfo;
}
//=============================================================================
int HOMARD_Iteration::GetInfoCompute() const
{
  return _MessInfo;
}

} // namespace SMESHHOMARDImpl /end/
