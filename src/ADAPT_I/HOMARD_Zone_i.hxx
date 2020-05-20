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

// Remarques :
// L'ordre de description des fonctions est le meme dans tous les fichiers
// HOMARD_aaaa.idl, HOMARD_aaaa.hxx, HOMARD_aaaa.cxx, HOMARD_aaaa_i.hxx, HOMARD_aaaa_i.cxx :
// 1. Les generalites : Name, Delete, DumpPython, Dump, Restore
// 2. Les caracteristiques
// 3. Le lien avec les autres structures
//
// Quand les 2 fonctions Setxxx et Getxxx sont presentes, Setxxx est decrit en premier
//

#ifndef _HOMARD_Zone_I_HXX_
#define _HOMARD_Zone_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(HOMARD_Gen)
#include CORBA_SERVER_HEADER(HOMARD_Zone)

#include "HOMARD_i.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"

#include <string>

class HOMARD_Zone;

class HOMARDENGINE_EXPORT HOMARD_Zone_i:
  public virtual Engines_Component_i,
  public virtual POA_HOMARD::HOMARD_Zone,
  public virtual PortableServer::ServantBase
{
public:
  HOMARD_Zone_i( CORBA::ORB_ptr orb, HOMARD::HOMARD_Gen_var gen_i );
  HOMARD_Zone_i();

  virtual ~HOMARD_Zone_i();

// Generalites
  void                   SetName( const char* Name );
  char*                  GetName();

  CORBA::Long            Delete();

  char*                  GetDumpPython();

  std::string            Dump() const;
  bool                   Restore( const std::string& stream );

// Caracteristiques
  void                   SetType( CORBA::Long Type );
  CORBA::Long            GetType();

  void                   SetBox( double Xmini, double Xmaxi,
                                 double Ymini, double Ymaxi,
                                 double Zmini, double Zmaxi );

  void                   SetCylinder( double Xcentre, double Ycentre, double ZCentre,
                                      double Xaxis, double Yaxis, double Zaxis,
                                      double Rayon, double Haut );

  void                   SetPipe( double Xcentre, double Ycentre, double ZCentre,
                                  double Xaxis, double Yaxis, double Zaxis,
                                  double Rayon, double Haut, double Rayonint );

  void                   SetSphere( double Xcentre, double Ycentre, double ZCentre,
                                    double Rayon );

  HOMARD::double_array*  GetCoords();

  void                   SetLimit( double Xincr, double Yincr, double Zincr);
  HOMARD::double_array*  GetLimit();

// Liens avec les autres structures
  void                   AddHypo( const char *NomHypo );
  HOMARD::listeHypo*     GetHypo();
  void                   SupprHypo( const char *NomHypo );

private:
  ::HOMARD_Zone*         myHomardZone;

  CORBA::ORB_ptr         _orb;
  HOMARD::HOMARD_Gen_var _gen_i;
};

#endif
