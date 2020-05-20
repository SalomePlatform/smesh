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

// Pilote l'ecriture du fichier de configuration pour lancer l'execution de HOMARD

#include <cstring>
#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/stat.h>

#include "HomardDriver.hxx"
#include "Utils_SALOME_Exception.hxx"
#include "utilities.h"

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
  if ( getenv("HOMARD_REP_EXE_PRIVATE") != NULL ) { dir = getenv("HOMARD_REP_EXE_PRIVATE") ; }
  else                                            { dir = getenv("HOMARD_REP_EXE") ; }
  MESSAGE("dir ="<<dir);
// L'executable HOMARD
  std::string executable ;
  if ( getenv("HOMARD_EXE_PRIVATE") != NULL ) { executable = getenv("HOMARD_EXE_PRIVATE") ; }
  else                                        { executable = getenv("HOMARD_EXE") ; }
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
void HomardDriver::TexteAdap( int ExtType )
{
  MESSAGE("TexteAdap");
//
  _Texte += "Action   homa\n" ;
  if ( ExtType ==  0 )      { _Texte += "CCAssoci med\n" ; }
  else if ( ExtType ==  1 ) { _Texte += "CCAssoci saturne\n" ; }
  else                      { _Texte += "CCAssoci saturne_2d\n" ; }
  _Texte += "ModeHOMA 1\n" ;
  _Texte += "NumeIter " + _siter + "\n" ;
  _modeHOMARD = 1 ;
//
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
void HomardDriver::TexteConfRaffDera( int ConfType, int TypeAdap, int TypeRaff, int TypeDera )
{
  MESSAGE("TexteConfRaffDera, ConfType ="<<ConfType);
  MESSAGE("TexteConfRaffDera, TypeAdap ="<<TypeAdap<<", TypeRaff ="<<TypeRaff<<", TypeDera ="<<TypeDera);
//
// Type de conformite
//
  std::string saux ;
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
  if ( TypeAdap == -1 )
  {
    if ( TypeRaff == 1 )
    {
      saux = "TypeRaff uniforme\n" ;
    }
    else
    {
      saux = "TypeRaff non\n" ;
    }
    if ( TypeDera == 1 )
    {
      saux += "TypeDera uniforme" ;
    }
    else
    {
      saux += "TypeDera non" ;
    }
  }
  else
  {
    if ( TypeRaff == 1 )
    {
      saux = "TypeRaff libre\n" ;
    }
    else
    {
      saux = "TypeRaff non\n" ;
    }
    if ( TypeDera == 1 )
    {
      saux += "TypeDera libre" ;
    }
    else
    {
      saux += "TypeDera non" ;
    }
  }
  _Texte += "# Type de raffinement/deraffinement\n" + saux + "\n" ;
//
//   MESSAGE("A la fin de HomardDriver::TexteConfRaffDera, _Texte ="<<_Texte);
}
//===============================================================================
void HomardDriver::TexteCompo( int NumeComp, const std::string NomCompo)
{
  MESSAGE("TexteCompo, NumeComp = "<<NumeComp<<", NomCompo = "<<NomCompo);
  _Texte +="CCCoChaI \"" + NomCompo + "\"\n" ;
}
//===============================================================================
void HomardDriver::TexteZone( int NumeZone, int ZoneType, int TypeUse, double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8 )
{
  MESSAGE("TexteZone, NumeZone = "<<NumeZone<<", ZoneType = "<<ZoneType<<", TypeUse = "<<TypeUse);
  MESSAGE("TexteZone, coor = "<< x0<<","<<x1<< ","<< x2<< ","<< x3<<","<<x4<<","<<x5<<","<<x6<<","<<x7<<","<<x8);
//
  std::string saux, saux2 ;
//
// Type de zones
// On convertit le type de zone au sens du module HOMARD dans Salome, ZoneType, dans le
// type au sens de l'executable HOMARD, ZoneTypeHOMARD
// Attention a mettre le bon signe a ZoneTypeHOMARD :
//    >0 signifie que l'on raffinera les mailles contenues dans la zone,
//    <0 signifie que l'on deraffinera
//
  int ZoneTypeHOMARD ;
  if ( ZoneType >= 11 && ZoneType <= 13 ) { ZoneTypeHOMARD = 1 ; }
  else if ( ZoneType >= 31 && ZoneType <= 33 ) { ZoneTypeHOMARD = 3 ; }
  else if ( ZoneType >= 61 && ZoneType <= 63 ) { ZoneTypeHOMARD = 6 ; }
  else { ZoneTypeHOMARD = ZoneType ; }
//
  if ( TypeUse < 0 ) { ZoneTypeHOMARD = -ZoneTypeHOMARD ; }
//
  std::stringstream saux1 ;
  saux1 << NumeZone ;
  saux = "#\n# Zone numero " + saux1.str() + "\n" ;
//
  { std::stringstream saux1 ;
    saux1 << NumeZone << " " << ZoneTypeHOMARD ;
    saux += "ZoRaType " + saux1.str() + "\n" ;
  }
//
// Cas du rectangle
//
  if ( ZoneType == 11 ) // Z est constant X Homard <=> X Salome
//                                        Y Homard <=> Y Salome
  {
    saux += "#Rectangle\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
  }
//
  else if ( ZoneType == 12 ) // X est constant X Homard <=> Y Salome
//                                             Y Homard <=> Z Salome
  {
    saux += "#Rectangle\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
  }
//
  else if ( ZoneType == 13 ) // Y est constant X Homard <=> X Salome
//                                             Y Homard <=> Z Salome
  {
    saux += "#Rectangle\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
  }
//
// Cas du parallelepipede
//
  else if ( ZoneType == 2 )
  {
    saux += "# Boite\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaYmax " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaZmin " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaZmax " + saux1.str() + "\n" ;
    }
  }
//
// Cas du disque
//
  else if ( ZoneType == 31 || ZoneType == 61 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
      if ( ZoneType == 61 ) { saux += "ZoRaRayE " + saux2 + "\n" ; }
      else                  { saux += "ZoRaRayo " + saux2 + "\n" ; }
    }
    if ( ZoneType == 61 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
  else if ( ZoneType == 32 || ZoneType == 62 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
      if ( ZoneType == 62 ) { saux += "ZoRaRayE " + saux2 + "\n" ; }
      else                  { saux += "ZoRaRayo " + saux2 + "\n" ; }
    }
    if ( ZoneType == 62 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
  else if ( ZoneType == 33 || ZoneType == 63 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
      if ( ZoneType == 63 ) { saux += "ZoRaRayE " + saux2 + "\n" ; }
      else                  { saux += "ZoRaRayo " + saux2 + "\n" ; }
    }
    if ( ZoneType == 63 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
//
// Cas de la sphere
//
  else if ( ZoneType == 4 )
  {
    saux += "# Sphere\n" ;
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaYCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaZCen " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaRayo " + saux1.str() + "\n" ;
    }
  }
//
// Cas du cylindre ou du tuyau
//
  else if ( ZoneType == 5 || ZoneType == 7 )
  {
    if ( ZoneType == 5 ) { saux += "# Cylindre\n" ; }
    else                 { saux += "# Tuyau\n" ; }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x0 ;
      saux += "ZoRaXBas " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x1 ;
      saux += "ZoRaYBas " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x2 ;
      saux += "ZoRaZBas " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x3 ;
      saux += "ZoRaXAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x4 ;
      saux += "ZoRaYAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x5 ;
      saux += "ZoRaZAxe " + saux1.str() + "\n" ;
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x6 ;
      saux2 = saux1.str() ;
     if ( ZoneType == 5 ) { saux += "ZoRaRayo " + saux2 + "\n" ; }
     else                 { saux += "ZoRaRayE " + saux2 + "\n" ; }
    }
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x7 ;
      saux += "ZoRaHaut " + saux1.str() + "\n" ;
    }
    if ( ZoneType == 7 )
    { std::stringstream saux1 ;
      saux1 << NumeZone << " " << x8 ;
      saux += "ZoRaRayI " + saux1.str() + "\n" ;
    }
  }
//
  _Texte += saux + "#\n" ;
//
//   MESSAGE("A la fin de HomardDriver::TexteZone, _Texte ="<<_Texte);
}
//===============================================================================
void HomardDriver::TexteField( const std::string FieldName, const std::string FieldFile, int TimeStep, int Rank,
               int TypeThR, double ThreshR, int TypeThC, double ThreshC,
               int UsField, int UsCmpI )
{
  MESSAGE("TexteField, FieldName = "<<FieldName<<", FieldFile = "<<FieldFile);
  MESSAGE("TexteField, TimeStep = "<<TimeStep<<", Rank = "<<Rank);

  std::string saux, saux2 ;
//
//
  _Texte += "# Champ d'indicateurs\n" ;
  _Texte += "CCIndica \"" + FieldFile  + "\"\n" ;
  _Texte += "CCNoChaI \"" + FieldName  + "\"\n" ;

// Cas ou on prend le dernier pas de temps
  if ( TimeStep == -2 )
  { _Texte += "CCNumPTI Last\n" ; }
// Cas avec pas de temps
  else if ( TimeStep >= 0 )
  {
    {
      std::stringstream saux1 ;
      saux1 << TimeStep ;
      saux2 = saux1.str() ;
      _Texte += "CCNumPTI " + saux2  + "\n" ;
    }
    if ( Rank >= 0 )
    {
      std::stringstream saux1 ;
      saux1 << Rank ;
      saux2 = saux1.str() ;
      _Texte += "CCNumOrI " + saux2  + "\n" ;
    }
  }
//
  saux = " " ;
  if ( TypeThR == 1 )
  { saux = "Hau" ; }
  if ( TypeThR == 2 )
  { saux = "HRe" ; }
  if ( TypeThR == 3 )
  { saux = "HPE" ; }
  if ( TypeThR == 4 )
  { saux = "HMS" ; }
  if ( saux != " " )
  {
    std::stringstream saux1 ;
    saux1 << ThreshR ;
    _Texte += "Seuil" + saux + " " + saux1.str()  + "\n" ;
  }
//
  saux = " " ;
  if ( TypeThC == 1 )
  { saux = "Bas" ; }
  if ( TypeThC == 2 )
  { saux = "BRe" ; }
  if ( TypeThC == 3 )
  { saux = "BPE" ; }
  if ( TypeThC == 4 )
  { saux = "BMS" ; }
  if ( saux != " " )
  {
    std::stringstream saux1 ;
    saux1 << ThreshC ;
    _Texte += "Seuil" + saux + " " + saux1.str()  + "\n" ;
  }
//
  saux = " " ;
  if ( UsField == 0 )
  { saux = "MAILLE" ; }
  if ( UsField == 1 )
  { saux = "SAUT" ; }
  if ( saux != " " )
  {
    _Texte += "CCModeFI " + saux  + "\n" ;
  }
//
  saux = " " ;
  if ( UsCmpI == 0 )
  { saux = "L2" ; }
  if ( UsCmpI == 1 )
  { saux = "INFINI" ; }
  if ( UsCmpI == 2 )
  { saux = "RELATIF" ; }
  if ( saux != " " )
  {
    _Texte += "CCUsCmpI " + saux  + "\n" ;
  }
}
//===============================================================================
void HomardDriver::TexteGroup( const std::string GroupName )
{
  MESSAGE("TexteGroup, GroupName = "<<GroupName);
//
  _Texte += "CCGroAda \"" + GroupName  + "\"\n" ;
//
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
// E. Les interpolations
//===============================================================================
// Les fichiers d'entree et de sortie des champs a interpoler
void HomardDriver::TexteFieldInterp( const std::string FieldFile, const std::string MeshFile )
{
  MESSAGE("TexteFieldInterp, FieldFile = "<<FieldFile<<", MeshFile = "<<MeshFile);
//
  _Texte += "#\n# Interpolations des champs\n" ;
//
// Fichier en entree
  _Texte += "CCSolN__ \"" + FieldFile + "\"\n" ;
// Fichier en sortie
  _Texte += "CCSolNP1 \"" + MeshFile  + "\"\n" ;
//
//  std::cerr << "A la fin de TexteFieldInterp _Texte ="<<_Texte << std::endl;
}
//===============================================================================
// Tous les champs sont a interpoler
void HomardDriver::TexteFieldInterpAll( )
{
  MESSAGE("TexteFieldInterpAll");
//
  _Texte += "CCChaTou oui\n" ;
}
//===============================================================================
// Ecrit les caracteristiques de chaque interpolation sous la forme :
//   CCChaNom 1 "DEPL"     ! Nom du 1er champ a interpoler
//   CCChaTIn 1 0          ! Mode d'interpolation : automatique
//   CCChaNom 2 "VOLUME"   ! Nom du 2nd champ a interpoler
//   CCChaTIn 2 1          ! Mode d'interpolation : une variable extensive
//   CCChaPdT 2 14         ! Pas de temps 14
//   CCChaNuO 2 14         ! Numero d'ordre 14
//   etc.
//
// NumeChamp : numero d'ordre du champ a interpoler
// FieldName : nom du champ
// TypeInterp : type d'interpolation
// TimeStep : pas de temps retenu (>0 si pas de precision)
// Rank : numero d'ordre retenu
//
void HomardDriver::TexteFieldInterpNameType( int NumeChamp, const std::string FieldName, const std::string TypeInterp, int TimeStep, int Rank)
{
  MESSAGE("TexteFieldInterpNameType, NumeChamp = "<<NumeChamp<<", FieldName = "<<FieldName<<", TypeInterp = "<<TypeInterp);
  MESSAGE("TexteFieldInterpNameType, TimeStep = "<<TimeStep<<", Rank = "<<Rank);
// Numero d'ordre du champ a interpoler
  std::stringstream saux1 ;
  saux1 << NumeChamp ;
  std::string saux = saux1.str() ;
// Nom du champ
  _Texte +="CCChaNom " + saux + " \"" + FieldName + "\"\n" ;
// Type d'interpolation pour le champ
  _Texte +="CCChaTIn " + saux + " " + TypeInterp + "\n" ;
//
  if ( TimeStep >= 0 )
  {
    {
      std::stringstream saux1 ;
      saux1 << TimeStep ;
      _Texte += "CCChaPdT " + saux + " " + saux1.str()  + "\n" ;
    }
    {
      std::stringstream saux1 ;
      saux1 << Rank ;
      _Texte += "CCChaNuO " + saux + " " + saux1.str()  + "\n" ;
    }
  }
}
//===============================================================================
// F. Les options avancees
//===============================================================================
void HomardDriver::TexteAdvanced( int Pyram, int NivMax, double DiamMin, int AdapInit, int ExtraOutput )
{
  MESSAGE("TexteAdvanced, Pyram ="<<Pyram<<", NivMax ="<<NivMax<<", DiamMin ="<<DiamMin<<", AdapInit ="<<AdapInit<<", ExtraOutput ="<<ExtraOutput);

  if ( Pyram > 0 )
  {
    _Texte += "# Autorisation de pyramides dans le maillage initial\n" ;
    _Texte += "TypeElem ignore_pyra\n" ;
  }
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
int HomardDriver::ExecuteHomard(int option)
{
  MESSAGE("ExecuteHomard, avec option = "<<option);
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
