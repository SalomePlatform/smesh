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

# ifndef __YACSDRIVER_H__
# define __YACSDRIVER_H__

#include "HOMARD.hxx"

#include <iostream>
#include <fstream>

class HOMARDIMPL_EXPORT YACSDriver
{
public:
  YACSDriver(const std::string XMLFile, const std::string DirName, const std::string LangueShort) ;
  ~YACSDriver() ;
  //
  void        TexteAdd( const std::string ligne ) ;
  void        Texte_DataInit_MeshFile( const std::string Meshfile ) ;
  void        Texte_Alternance_Calcul_HOMARD_Calcul( const std::string FileName ) ;
  void        Texte_Iter_1_Case_Options( const std::string pythonCas ) ;
  std::string Texte_Iter_1_Boundary( int BoundaryType, const std::string pythonStructure, const std::string methode, const std::string BoundaryName, const std::string MeshName, const std::string MeshFile ) ;
  std::string Texte_Iter_1_Zone( int ZoneType, const std::string pythonStructure, const std::string methode, const std::string ZoneName ) ;
  std::string Texte_Iter_1_control() ;
  std::string Texte_control( const std::string noeud_1, const std::string noeud_2, int option ) ;
  std::string Texte_inport( const std::string inport_type, const std::string inport_nom ) ;
  void        Texte_HOMARD_Exec_DirName( ) ;
  void        Texte_HOMARD_Exec_MeshName( const std::string MeshName ) ;
  void        Texte_python_1( const std::string pythonTexte, int indice, const std::string concept ) ;
  void        Texte_python_2( const std::string pythonTexte, const std::string mot_cle, const std::string concept ) ;
  void        TexteParametre( const std::string node, const std::string port, const std::string type_value, const std::string value ) ;
  void        TexteAnalyse_Test_Convergence( int MaxIter, int MaxNode, int MaxElem ) ;
  void        TexteAddParametres( ) ;
  void        CreeFichier() ;
  //

public:
  std::string _XMLFile ;
  std::string _DirName ;
  std::string _Texte ;
  std::string _Texte_parametres ;
  std::string _noeud_1 ;
  std::string _LangueShort ;
  bool _bLu;

private :
  std::string GetStringInTexte( const std::string Texte, const std::string String, int option ) ;
};

# endif         /* # ifndef __YACSDRIVER_H__ */
