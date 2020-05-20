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

// Pilote l'ecriture du fichier xml pour lancer un schema YACS

#include <cstring>
#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/stat.h>

#include "YACSDriver.hxx"
#include "HOMARD.hxx"
#include "Utils_SALOME_Exception.hxx"
#include "utilities.h"

//=============================================================================
//=============================================================================
YACSDriver::YACSDriver(const std::string XMLFile, const std::string DirName, const std::string LangueShort):
  _XMLFile( "" ), _DirName( "" ),
  _Texte( "" ),
  _Texte_parametres( "" ),
  _noeud_1( "CreateCase" ),
  _LangueShort( "" ),
  _bLu( false )
{
  MESSAGE("XMLFile = "<<XMLFile<<", DirName ="<<DirName);
  _XMLFile = XMLFile;
  _DirName = DirName;
  _LangueShort = LangueShort;
}
//=============================================================================
//=============================================================================
YACSDriver::~YACSDriver()
{
}
//===============================================================================
// Ajout d'une ligne simple
//===============================================================================
void YACSDriver::TexteAdd( const std::string ligne )
{
//   MESSAGE("TexteAdd, ligne ="<<ligne);
//
  _Texte += ligne + "\n" ;
//
}
//===============================================================================
// Nom du fichier du maillage initial
//===============================================================================
void YACSDriver::Texte_DataInit_MeshFile( const std::string Meshfile )
{
  MESSAGE("TexteInitMeshfile, Meshfile ="<<Meshfile);
//
  _Texte += "         <value><string>" ;
  _Texte += Meshfile ;
  _Texte += "</string></value>\n" ;
//
}
//===============================================================================
// Le repertoire de calcul
// Le script de lancement
//===============================================================================
void YACSDriver::Texte_Alternance_Calcul_HOMARD_Calcul( const std::string FileName )
{
  MESSAGE("Texte_Alternance_Calcul_HOMARD_Calcul, FileName : "<<FileName);
//
  int position = FileName.find_last_of( '/' ) ;
  std::string nomfic = FileName.substr( position+1 ) ;
  position = nomfic.find_last_of( '.' ) ;;
  nomfic = nomfic.substr( 0, position ) ;
  MESSAGE("nomfic : "<<nomfic) ;
//
  _Texte += "rep_calc = \"" + _DirName + "\"\n" ;
  _Texte += "rep_script = os.path.dirname(\"" + FileName + "\")\n" ;
  _Texte += "sys.path.append(rep_script)\n" ;
  _Texte += "from " + nomfic + " import Script\n" ;
//
}
//===============================================================================
// Les options du cas
//===============================================================================
void YACSDriver::Texte_Iter_1_Case_Options( const std::string pythonTexte )
{
  MESSAGE("Texte_Iter_1_Case_Options, pythonTexte\n"<<pythonTexte);
//
  _Texte += "DirName = \"" + _DirName + "/HOMARD\"\n" ;
  _Texte += "Case.SetDirName(DirName)\n" ;

  Texte_python_1( pythonTexte, 1, "Case" ) ;
//
}
//===============================================================================
// La description des zones
// ZoneType : le type de la zone
// pythonStructure : le python correspondant a la zone
// methode : methode associee a la creation de la zone
// ZoneName : nom de la zone
//===============================================================================
std::string YACSDriver::Texte_Iter_1_Zone( int ZoneType, const std::string pythonStructure, const std::string methode, const std::string ZoneName )
{
  MESSAGE("Texte_Iter_1_Zone, ZoneType = "<<ZoneType<<", pythonStructure = "<<pythonStructure);
  MESSAGE("methode = "<<methode<<", ZoneName = "<<ZoneName );
//
// 1. Le nom du noeud
  std::string noeud_2 = methode + "_" + ZoneName ;
  std::string node ;
  if ( _LangueShort == "fr" ) { node = "Boucle_de_convergence.Alternance_Calcul_HOMARD" ; }
  else                        { node = "Convergence_Loop.Alternation_Computation_HOMARD" ; }
  node += ".Adaptation.p0_Adaptation_HOMARD.HOMARD_Initialisation.p1_Iter_1." ;
  node += noeud_2 ;
// 2. Texte de controle : le noeud precedent est _noeud_1, le noeud courant noeud_2.
//                        A la fin, on bascule le courant dans le precedent
  std::string texte_control = Texte_control (_noeud_1, noeud_2, 1) ;
  _noeud_1 = noeud_2 ;
// 3. Definition du service
  std::string motcle ;
  _Texte += "                           <service name=\"" + noeud_2 + "\">\n" ;
  INFOS("_LangueShort = "<<_LangueShort );
  if ( _LangueShort == "fr" ) { motcle = "Etude_Initialisation" ; }
  else                        { motcle = "Study_Initialisation" ; }
  INFOS("motcle = "<<motcle );
  _Texte += "                              <node>" + motcle + ".UpdateStudy</node>\n" ;
  _Texte += "                              <method>" + methode + "</method>\n" ;
// 4. Les inports
// 4.1. Le nom de la zone
  _Texte += Texte_inport( "string", "ZoneName" ) ;
   TexteParametre( node, "ZoneName", "string", ZoneName ) ;
// 4.2. Les valeurs numeriques
//      ATTENTION : les noms doivent etre les memes que dans Gen.xml, donc HOMARD_Gen.idl
// 4.2.1. Decodage des valeurs
// La chaine pythonStructure est de ce genre :
//   CreateZoneBox( "Zone_1", 0.144, 0.216, 0.096, 0.1464, 0.076, 0.12)
  std::string ligne = pythonStructure ;
// On commence par ne garder que ce qui suit la premiere virgule
  ligne = GetStringInTexte( ligne, ",", 1 );
// On boucle pour isoler toutes les chaines dans les virgules
  std::string lignebis ;
  std::string x0, x1, x2, x3, x4, x5, x6, x7, x8  ;
  int iaux = 0  ;
  while ( ligne != lignebis )
  {
    lignebis = GetStringInTexte ( ligne, ",", 0 ) ;
//     MESSAGE("lignebis = "<<lignebis );
    if      ( iaux == 0 ) { x0 = lignebis ; }
    else if ( iaux == 1 ) { x1 = lignebis ; }
    else if ( iaux == 2 ) { x2 = lignebis ; }
    else if ( iaux == 3 ) { x3 = lignebis ; }
    else if ( iaux == 4 ) { x4 = lignebis ; }
    else if ( iaux == 5 ) { x5 = lignebis ; }
    else if ( iaux == 6 ) { x6 = lignebis ; }
    else if ( iaux == 7 ) { x7 = lignebis ; }
    ligne = GetStringInTexte( ligne, ",", 1 );
    iaux += 1 ;
  }
// La derniere valeur est toujours mise dans x8
  x8 = GetStringInTexte ( ligne, ")", 0 ) ;
  MESSAGE("coor = "<< x0<<","<<x1<< ","<< x2<< ","<< x3<<","<<x4<<","<<x5<<","<<x6<<","<<x7<<","<<x8);
// 4.2.2. Cas du parallelepipede (2)
  if ( ZoneType == 2 )
  {
    _Texte += Texte_inport( "double", "Xmini" ) ;
    _Texte += Texte_inport( "double", "Xmaxi" ) ;
    _Texte += Texte_inport( "double", "Ymini" ) ;
    _Texte += Texte_inport( "double", "Ymaxi" ) ;
    _Texte += Texte_inport( "double", "Zmini" ) ;
    _Texte += Texte_inport( "double", "Zmaxi" ) ;
    TexteParametre( node, "Xmini", "double", x0 ) ;
    TexteParametre( node, "Xmaxi", "double", x1 ) ;
    TexteParametre( node, "Ymini", "double", x2 ) ;
    TexteParametre( node, "Ymaxi", "double", x3 ) ;
    TexteParametre( node, "Zmini", "double", x4 ) ;
    TexteParametre( node, "Zmaxi", "double", x8 ) ;
  }
//
// 4.2.3. Cas du rectangle (11, 12, 13)
  else if ( ( ZoneType > 10 ) && ( ZoneType < 14 ) )
  {
    _Texte += Texte_inport( "double", "Umini" ) ;
    _Texte += Texte_inport( "double", "Umaxi" ) ;
    _Texte += Texte_inport( "double", "Vmini" ) ;
    _Texte += Texte_inport( "double", "Vmaxi" ) ;
    _Texte += Texte_inport( "long", "Orient" ) ;
    TexteParametre( node, "Umini", "double", x0 ) ;
    TexteParametre( node, "Umaxi", "double", x1 ) ;
    TexteParametre( node, "Vmini", "double", x2 ) ;
    TexteParametre( node, "Vmaxi", "double", x3 ) ;
    TexteParametre( node, "Orient", "int", x8 ) ;
  }
//
// 4.2.4. Cas du disque (31, 32, 33) ou du disque perce (61, 62, 63)
  else if ( ( ( ZoneType > 30 ) && ( ZoneType < 34 ) ) || ( ( ZoneType > 60 ) && ( ZoneType < 64 ) ) )
  {
    _Texte += Texte_inport( "double", "Ucentre" ) ;
    _Texte += Texte_inport( "double", "Vcentre" ) ;
    _Texte += Texte_inport( "double", "Radius" ) ;
    TexteParametre( node, "Ucentre", "double", x0 ) ;
    TexteParametre( node, "Vcentre", "double", x1 ) ;
    TexteParametre( node, "Radius", "double", x2 ) ;
    if ( ZoneType > 60 )
    {
      _Texte += Texte_inport( "double", "InternalRadius" ) ;
      TexteParametre( node, "InternalRadius", "double", x3 ) ;
    }
    _Texte += Texte_inport( "long", "Orient" ) ;
    TexteParametre( node, "Orient", "int", x8 ) ;
  }
//
// 4.2.5. Cas de la sphere (4)
  else if ( ZoneType == 4 )
  {
    _Texte += Texte_inport( "double", "Xcentre" ) ;
    _Texte += Texte_inport( "double", "Ycentre" ) ;
    _Texte += Texte_inport( "double", "Zcentre" ) ;
    _Texte += Texte_inport( "double", "Radius" ) ;
    TexteParametre( node, "Xcentre", "double", x0 ) ;
    TexteParametre( node, "Ycentre", "double", x1 ) ;
    TexteParametre( node, "Zcentre", "double", x2 ) ;
    TexteParametre( node, "Radius", "double", x8 ) ;
  }
//
// 4.2.6. Cas du cylindre (5) ou du tuyau (7)
  else if ( ZoneType == 5 || ZoneType == 7 )
  {
    _Texte += Texte_inport( "double", "Xcentre" ) ;
    _Texte += Texte_inport( "double", "Ycentre" ) ;
    _Texte += Texte_inport( "double", "Zcentre" ) ;
    _Texte += Texte_inport( "double", "Xaxis" ) ;
    _Texte += Texte_inport( "double", "Yaxis" ) ;
    _Texte += Texte_inport( "double", "Zaxis" ) ;
    _Texte += Texte_inport( "double", "Radius" ) ;
    _Texte += Texte_inport( "double", "Height" ) ;
    TexteParametre( node, "Xcentre", "double", x0 ) ;
    TexteParametre( node, "Ycentre", "double", x1 ) ;
    TexteParametre( node, "Zcentre", "double", x2 ) ;
    TexteParametre( node, "Xaxis", "double", x3 ) ;
    TexteParametre( node, "Yaxis", "double", x4 ) ;
    TexteParametre( node, "Zaxis", "double", x5 ) ;
    TexteParametre( node, "Radius", "double", x6 ) ;
    if ( ZoneType == 5 )
    {
      TexteParametre( node, "Height", "double", x8 ) ;
    }
    else
    {
      _Texte += Texte_inport( "double", "InternalRadius" ) ;
      TexteParametre( node, "Height", "double", x7 ) ;
      TexteParametre( node, "InternalRadius", "double", x8 ) ;
    }
  }
//
// 4.2.7. Erreur
  else
  { VERIFICATION("Type de zone inconnu." == 0); }

//
// 5. La fin
  _Texte += "                              <outport name=\"return\" type=\"HOMARD_Zone\"/>\n" ;
  _Texte += "                           </service>\n" ;
//
  return texte_control ;
//
}
//===============================================================================
// La description des frontieres
// BoundaryType : le type de la frontiere
// pythonStructure : le python correspondant a la frontiere
// methode : methode associee a la creation de la frontiere
// BoundaryName : nom de la frontiere
// MeshName : nom du maillage dans le cas d'une frontiere discrete
// MeshFile : nom du fichier du maillage dans le cas d'une frontiere discrete
//===============================================================================
std::string YACSDriver::Texte_Iter_1_Boundary( int BoundaryType, const std::string pythonStructure, const std::string methode, const std::string BoundaryName, const std::string MeshName, const std::string MeshFile )
{
  MESSAGE("Texte_Iter_1_Boundary, BoundaryType = "<<BoundaryType<<", pythonStructure = "<<pythonStructure);
  MESSAGE("methode = "<<methode<<", BoundaryName = "<<BoundaryName );
  if (BoundaryType == 0) { MESSAGE("MeshName = "<<MeshName<<", MeshFile = "<<MeshFile ); }
//
// 1. Le nom du noeud
  std::string noeud_2 = methode + "_" + BoundaryName ;
  std::string node ;
  if ( _LangueShort == "fr" ) { node = "Boucle_de_convergence.Alternance_Calcul_HOMARD" ; }
  else                        { node = "Convergence_Loop.Alternation_Computation_HOMARD" ; }
  node += ".Adaptation.p0_Adaptation_HOMARD.HOMARD_Initialisation.p1_Iter_1." ;
  node += noeud_2 ;
// 2. Texte de controle : le noeud precedent est _noeud_1, le noeud courant noeud_2.
//                        A la fin, on bascule le courant dans le precedent
  std::string texte_control = Texte_control (_noeud_1, noeud_2, 1) ;
  _noeud_1 = noeud_2 ;
// 3. Definition du service
  std::string motcle ;
  _Texte += "                           <service name=\"" + noeud_2 + "\">\n" ;
  if ( _LangueShort == "fr" ) { motcle = "Etude_Initialisation" ; }
  else                        { motcle = "Study_Initialisation" ; }
  _Texte += "                              <node>" + motcle + ".UpdateStudy</node>\n" ;
  _Texte += "                              <method>" + methode + "</method>\n" ;
// 4. Les inports
//    ATTENTION : les noms doivent etre les memes que dans Gen.xml, donc HOMARD_Gen.idl
// 4.1. Le nom de la frontiere
  _Texte += Texte_inport( "string", "BoundaryName" ) ;
  TexteParametre( node, "BoundaryName", "string", BoundaryName ) ;
// 4.2. Cas d une frontiere discrete
  if (BoundaryType == 0)
  {
    _Texte += Texte_inport( "string", "MeshName" ) ;
    TexteParametre( node, "MeshName", "string", MeshName ) ;
    _Texte += Texte_inport( "string", "FileName" ) ;
    TexteParametre( node, "FileName", "string", MeshFile ) ;
  }
// 4.3. Cas d'une frontiere analytique : les valeurs numeriques
  else
  {
// 4.3.1. Decodage des valeurs
// La chaine pythonStructure est de ce genre :
//   CreateBoundaryCylinder('cyl_2', 17.5, -2.5, -12.5, -100., -75., -25., 50.)
//   CreateBoundaryDi("intersection", "PIQUAGE", "/scratch/D68518/Salome/script/sfr_2d_piquage.fr.med")
    std::string ligne = pythonStructure ;
// On commence par ne garder que ce qui suit la premiere virgule
    ligne = GetStringInTexte( ligne, ",", 1 );
// On boucle pour isoler toutes les chaines dans les virgules
    std::string lignebis ;
    std::string x0, x1, x2, x3, x4, x5, x6, x7  ;
    int iaux = 0  ;
    while ( ligne != lignebis )
    {
      lignebis = GetStringInTexte ( ligne, ",", 0 ) ;
//     MESSAGE("lignebis = "<<lignebis );
      if      ( iaux == 0 ) { x0 = lignebis ; }
      else if ( iaux == 1 ) { x1 = lignebis ; }
      else if ( iaux == 2 ) { x2 = lignebis ; }
      else if ( iaux == 3 ) { x3 = lignebis ; }
      else if ( iaux == 4 ) { x4 = lignebis ; }
      else if ( iaux == 5 ) { x5 = lignebis ; }
      else if ( iaux == 6 ) { x6 = lignebis ; }
      ligne = GetStringInTexte( ligne, ",", 1 );
      iaux += 1 ;
    }
// La derniere valeur est toujours mise dans x7
    x7 = GetStringInTexte ( ligne, ")", 0 ) ;
    MESSAGE("Valeurs = "<< x0<<", "<<x1<< ", "<< x2<< ", "<< x3<<", "<<x4<<", "<<x5<<", "<<x6<<", x7"<<x7);
//
// 4.3.2. Cas du cylindre (1)
    if ( BoundaryType == 1 )
    {
      _Texte += Texte_inport( "double", "Xcentre" ) ;
      _Texte += Texte_inport( "double", "Ycentre" ) ;
      _Texte += Texte_inport( "double", "Zcentre" ) ;
      _Texte += Texte_inport( "double", "Xaxis" ) ;
      _Texte += Texte_inport( "double", "Yaxis" ) ;
      _Texte += Texte_inport( "double", "Zaxis" ) ;
      _Texte += Texte_inport( "double", "Radius" ) ;
      TexteParametre( node, "Xcentre", "double", x0 ) ;
      TexteParametre( node, "Ycentre", "double", x1 ) ;
      TexteParametre( node, "Zcentre", "double", x2 ) ;
      TexteParametre( node, "Xaxis", "double", x3 ) ;
      TexteParametre( node, "Yaxis", "double", x4 ) ;
      TexteParametre( node, "Zaxis", "double", x5 ) ;
      TexteParametre( node, "Radius", "double", x7 ) ;
    }
//
// 4.3.3. Cas de la sphere (2)
    else if ( BoundaryType == 2 )
    {
      _Texte += Texte_inport( "double", "Xcentre" ) ;
      _Texte += Texte_inport( "double", "Ycentre" ) ;
      _Texte += Texte_inport( "double", "Zcentre" ) ;
      _Texte += Texte_inport( "double", "Radius" ) ;
      TexteParametre( node, "Xcentre", "double", x0 ) ;
      TexteParametre( node, "Ycentre", "double", x1 ) ;
      TexteParametre( node, "Zcentre", "double", x2 ) ;
      TexteParametre( node, "Radius", "double", x7 ) ;
    }
//
// 4.3.4. Cas d un cone defini par un axe et un angle
    else if ( BoundaryType == 3 )
    {
      _Texte += Texte_inport( "double", "Xaxis" ) ;
      _Texte += Texte_inport( "double", "Yaxis" ) ;
      _Texte += Texte_inport( "double", "Zaxis" ) ;
      _Texte += Texte_inport( "double", "Angle" ) ;
      _Texte += Texte_inport( "double", "Xcentre" ) ;
      _Texte += Texte_inport( "double", "Ycentre" ) ;
      _Texte += Texte_inport( "double", "Zcentre" ) ;
      TexteParametre( node, "Xaxis", "double", x0 ) ;
      TexteParametre( node, "Yaxis", "double", x1 ) ;
      TexteParametre( node, "Zaxis", "double", x2 ) ;
      TexteParametre( node, "Angle", "double", x3 ) ;
      TexteParametre( node, "Xcentre", "double", x4 ) ;
      TexteParametre( node, "Ycentre", "double", x5 ) ;
      TexteParametre( node, "Zcentre", "double", x7 ) ;
    }
//
// 4.3.5. Cas d un cone defini par les 2 rayons
    else if ( BoundaryType == 4 )
    {
      _Texte += Texte_inport( "double", "Xcentre1" ) ;
      _Texte += Texte_inport( "double", "Ycentre1" ) ;
      _Texte += Texte_inport( "double", "Zcentre1" ) ;
      _Texte += Texte_inport( "double", "Radius1" ) ;
      _Texte += Texte_inport( "double", "Xcentre2" ) ;
      _Texte += Texte_inport( "double", "Ycentre2" ) ;
      _Texte += Texte_inport( "double", "Zcentre2" ) ;
      _Texte += Texte_inport( "double", "Radius2" ) ;
      TexteParametre( node, "Xcentre1", "double", x0 ) ;
      TexteParametre( node, "Ycentre1", "double", x1 ) ;
      TexteParametre( node, "Zcentre1", "double", x2 ) ;
      TexteParametre( node, "Radius1", "double", x3 ) ;
      TexteParametre( node, "Xcentre2", "double", x4 ) ;
      TexteParametre( node, "Ycentre2", "double", x5 ) ;
      TexteParametre( node, "Zcentre2", "double", x6 ) ;
      TexteParametre( node, "Radius2", "double", x7 ) ;
    }
// 4.3.6. Cas du tore (5)
    else if ( BoundaryType == 5 )
    {
      _Texte += Texte_inport( "double", "Xcentre" ) ;
      _Texte += Texte_inport( "double", "Ycentre" ) ;
      _Texte += Texte_inport( "double", "Zcentre" ) ;
      _Texte += Texte_inport( "double", "Xaxis" ) ;
      _Texte += Texte_inport( "double", "Yaxis" ) ;
      _Texte += Texte_inport( "double", "Zaxis" ) ;
      _Texte += Texte_inport( "double", "RRev" ) ;
      _Texte += Texte_inport( "double", "RPri" ) ;
      TexteParametre( node, "Xcentre", "double", x0 ) ;
      TexteParametre( node, "Ycentre", "double", x1 ) ;
      TexteParametre( node, "Zcentre", "double", x2 ) ;
      TexteParametre( node, "Xaxis", "double", x3 ) ;
      TexteParametre( node, "Yaxis", "double", x4 ) ;
      TexteParametre( node, "Zaxis", "double", x5 ) ;
      TexteParametre( node, "RRev", "double", x6 ) ;
      TexteParametre( node, "RPri", "double", x7 ) ;
    }
//
// 4.3.7. Erreur
    else
    { VERIFICATION("Type de frontiere inconnu." == 0); }
  }
//
// 5. La fin
  _Texte += "                              <outport name=\"return\" type=\"HOMARD_Boundary\"/>\n" ;
  _Texte += "                           </service>\n" ;
//
  return texte_control ;
//
}
//===============================================================================
// Fin du controle des enchainements de noeud dans le noeud Iter_1
//===============================================================================
  std::string YACSDriver::Texte_Iter_1_control()
{
  MESSAGE("Texte_Iter_1_control");
//
  std::string texte ;
  texte  = Texte_control (_noeud_1, "CreateHypothesis", 1) ;
  texte += Texte_control ("CreateHypothesis", "Case_Options", 0) ;
//
  return texte ;
//
}
//===============================================================================
// Controle des enchainements de noeuds
// noeud_1 : noeud de depart
// noeud_2 : noeud d'arrivee
// option : 0 : sans caractere de saut de ligne a la fin
//          1 : avec caractere de saut de ligne a la fin
//===============================================================================
  std::string YACSDriver::Texte_control( const std::string noeud_1, const std::string noeud_2, int option )
{
  MESSAGE("Texte_control, noeud_1 = "<<noeud_1<<", noeud_2 = "<<noeud_2<<", option = "<<option);
//
  std::string texte ;
  texte  = "                           <control> " ;
  texte += "<fromnode>" + noeud_1 + "</fromnode>" ;
  texte += " <tonode>"  + noeud_2 + "</tonode>" ;
  texte += " </control>" ;
  if ( option == 1 ) { texte += "\n" ; }

  return texte ;
//
}
//===============================================================================
// Inport
// inport_type : type de la donnee a importer
// inport_nom : nom de la donnee a importer
//===============================================================================
  std::string YACSDriver::Texte_inport( const std::string inport_type, const std::string inport_nom )
{
//   MESSAGE("Texte_inport, inport_type = "<<inport_type<<", inport_nom = "<<inport_nom);
//
  std::string texte ;
  texte  = "                              <inport " ;
  texte += "name=\"" + inport_nom + "\" " ;
  texte += "type=\"" + inport_type + "\"" ;
  texte += "/>\n" ;

  return texte ;
//
}
//===============================================================================
// Le repertoire d'execution
//===============================================================================
void YACSDriver::Texte_HOMARD_Exec_DirName( )
{
  MESSAGE("Texte_HOMARD_Exec_DirName");
//
  _Texte += "DirName = \"" + _DirName + "\"\n" ;
//
}
//===============================================================================
// Le nom du maillage
//===============================================================================
void YACSDriver::Texte_HOMARD_Exec_MeshName( const std::string MeshName )
{
  MESSAGE("Texte_HOMARD_Exec_MeshName pour "<<MeshName);
//
  _Texte += "MeshName = \"" + MeshName + "\"\n" ;
//
}
//===============================================================================
// Manipulation des instructions python - 1
// pythonTexte : le texte des instructions python a manipuler
// indice : numero de la premiere ligne voulue
// concept : nom du concept a inserer
//===============================================================================
void YACSDriver::Texte_python_1( const std::string pythonTexte, int indice, const std::string concept )
{
  MESSAGE("Texte_python_1, pythonTexte\n"<<pythonTexte);
  MESSAGE("indice = "<<indice<<", concept = "<<concept);
//
// Conversion de type
  std::istringstream tout (pythonTexte) ;
//   MESSAGE("\ntout :"<<tout);
  std::string ligne; // variable contenant chaque ligne de python
  std::string ligne_bis ; // variable contenant la portion de ligne de python apres '.'
  int cptr = 0 ;
  indice -= 1 ;
  while ( std::getline( tout, ligne ) )
  {
    if ( cptr > indice )
    {
      int position = ligne.find_first_of( "." ) ;
      MESSAGE("\nposition : "<< position);
      if ( position > 0 )
      {
        ligne_bis = ligne.substr( position );
        MESSAGE("\nligne_bis : "<< ligne_bis);
        _Texte += concept + ligne_bis + "\n" ;
      }
    }
    cptr += 1 ;
  }
//
}
//===============================================================================
// Manipulation des instructions python - 2
// pythonTexte : le texte des instructions python a manipuler
// mot_cle : mot-cle dans les lignes a inserer
// concept : nom du concept a inserer
//===============================================================================
void YACSDriver::Texte_python_2( const std::string pythonTexte, const std::string mot_cle, const std::string concept )
{
  MESSAGE("Texte_python_2, pythonTexte\n"<<pythonTexte);
  MESSAGE("mot_cle = "<<mot_cle<<", concept = "<<concept);
//
// Conversion de type
  std::istringstream tout (pythonTexte) ;
//   MESSAGE("\ntout :"<<tout);
  std::string ligne; // variable contenant chaque ligne de python
  std::string ligne_bis ; // variable contenant la portion de ligne de python apres '.'
  while ( std::getline( tout, ligne ) )
  {
    int reperage = ligne.find( mot_cle ) ;
    if ( reperage > 0 )
    {
      int position = ligne.find_first_of( "." ) ;
//       MESSAGE("\nposition : "<< position);
      if ( position > 0 )
      {
        ligne_bis = ligne.substr( position );
//         MESSAGE("\nligne_bis : "<< ligne_bis);
        _Texte += concept + ligne_bis + "\n" ;
      }
    }
  }
//
}
//===============================================================================
// Parametres de tests de convergence
//===============================================================================
void YACSDriver::TexteAnalyse_Test_Convergence( int MaxIter, int MaxNode, int MaxElem )
{
//
//   MESSAGE("TexteAnalyse_Test_Convergence");
  std::string chaine ;
  std::ostringstream oss1;
  oss1 << MaxIter;
  chaine = oss1.str();
  _Texte += "MaxIter = " + chaine + "\n" ;
  std::ostringstream oss2;
  oss2 << MaxNode;
  chaine = oss2.str();
  _Texte += "MaxNode = " + chaine + "\n" ;
  std::ostringstream oss3;
  oss3 << MaxElem;
  chaine = oss3.str();
  _Texte += "MaxElem = " + chaine + "\n" ;
//
}
//===============================================================================
// Creation d'un parametre
//===============================================================================
void YACSDriver::TexteParametre( const std::string node, const std::string port, const std::string type_value, const std::string value )
{
//
//   MESSAGE("TexteParametre");
  _Texte_parametres += "   <parameter>\n" ;
  _Texte_parametres += "      <tonode>" + node + "</tonode>" ;
  _Texte_parametres += "<toport>" + port + "</toport>\n" ;
  _Texte_parametres += "      <value><" + type_value + ">" + value + "</" + type_value + "></value>\n" ;
  _Texte_parametres += "   </parameter>\n" ;
//
}
//===============================================================================
// Ajout des parametres
//===============================================================================
void YACSDriver::TexteAddParametres( )
{
//
  MESSAGE("TexteAddParametres");
  TexteAdd(_Texte_parametres) ;
//
}
//===============================================================================
void YACSDriver::CreeFichier( )
{
//
  MESSAGE("CreeFichier sur le fichier "<<_XMLFile);
  std::ofstream Fic(_XMLFile.c_str(), std::ios::out ) ;
  if (Fic.is_open() == true) { Fic << _Texte << std::endl ; }
  Fic.close() ;
//
}
//===============================================================================
// REMARQUE : on devrait utiliser le GetStringInTexte de HOMARD_Gen_i mais je ne sais pas
//            comment l'appeler. ALors je clone.
// Recuperation de la chaine de caracteres par rapport l'apparition d'un texte
// ligne : la ligne a manipuler
// texte : le texte a reperer
// option : 0 : la chaine avant le texte
//          1 : la chaine apres le texte
// Si le texte est absent, on retourne la chaine totale
//===============================================================================
std::string YACSDriver::GetStringInTexte( const std::string ligne, const std::string texte, int option )
{
//   MESSAGE("GetStringInTexte, recherche de '"<<texte<<"' dans '"<<ligne<<"'"<<", option = "<<option);
//
  std::string chaine = ligne ;
  int position = ligne.find_first_of( texte ) ;
  if ( position > 0 )
  {
    if ( option == 0 ) { chaine = ligne.substr( 0, position ) ; }
    else               { chaine = ligne.substr( position+1 ) ; }
  }
// Conversion de type
  return chaine ;
//
}
