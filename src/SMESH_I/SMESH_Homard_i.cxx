// Copyright (C) 2011-2024  CEA, EDF
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

#include "SMESH_Homard_i.hxx"
#include "SMESH_Homard.hxx"

#include "SMESH_Gen_i.hxx"
#include "SMESH_PythonDump.hxx"

#include "SMESH_File.hxx"

#include "utilities.h"
#include "Basics_Utils.hxx"
#include "Basics_DirUtils.hxx"
#include "Utils_SINGLETON.hxx"
#include "Utils_CorbaException.hxx"
#include "SALOMEDS_Tool.hxx"
#include "SALOME_LifeCycleCORBA.hxx"
#include "SALOMEconfig.h"

// Have to be included before std headers
#include <Python.h>
#include <structmember.h>

#include <vector>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <sys/stat.h>
#include <algorithm>

#include <med.h>

#ifdef WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>
#include <vector>
#include <stdio.h>

// C'est le ASSERT de SALOMELocalTrace/utilities.h dans KERNEL
#ifndef VERIFICATION
#define VERIFICATION(condition) \
        if (!(condition)){INTERRUPTION("CONDITION "<<#condition<<" NOT VERIFIED")}
#endif /* VERIFICATION */

// La gestion des repertoires
#ifndef CHDIR
  #ifdef WIN32
    #define CHDIR _chdir
  #else
    #define CHDIR chdir
  #endif
#endif

using namespace std;

SMESHHOMARD::HOMARD_Gen_ptr SMESH_Gen_i::CreateHOMARD_ADAPT()
{
  if (getenv("HOMARD_ROOT_DIR") == NULL) {
    THROW_SALOME_CORBA_EXCEPTION("HOMARD_ROOT_DIR is not defined", SALOME::INTERNAL_ERROR);
  }
  else {
    std::string homard_exec = getenv("HOMARD_ROOT_DIR");
    homard_exec += "/bin/salome/homard";
    if (!SMESH_File(homard_exec).exists())
      THROW_SALOME_CORBA_EXCEPTION("HOMARD module is not built", SALOME::INTERNAL_ERROR);
  }
  SMESHHOMARD_I::HOMARD_Gen_i* aHomardGen = new SMESHHOMARD_I::HOMARD_Gen_i();
  SMESHHOMARD::HOMARD_Gen_var anObj = aHomardGen->_this();
  return anObj._retn();
}

namespace SMESHHOMARD_I
{

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Boundary_i::HOMARD_Boundary_i()
  : SALOME::GenericObj_i(SMESH_Gen_i::GetPOA())
{
  MESSAGE("Default constructor, not for use");
  ASSERT(0);
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Boundary_i::HOMARD_Boundary_i(SMESHHOMARD::HOMARD_Gen_var engine)
  : SALOME::GenericObj_i(SMESH_Gen_i::GetPOA())
{
  MESSAGE("HOMARD_Boundary_i");
  _gen_i = engine;
  myHomardBoundary = new SMESHHOMARDImpl::HOMARD_Boundary();
  ASSERT(myHomardBoundary);
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Boundary_i::~HOMARD_Boundary_i()
{
}
//=============================================================================
//=============================================================================
// Generalites
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetName(const char* Name)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetName(Name);
}
//=============================================================================
char* HOMARD_Boundary_i::GetName()
{
  ASSERT(myHomardBoundary);
  return CORBA::string_dup(myHomardBoundary->GetName().c_str());
}
//=============================================================================
char* HOMARD_Boundary_i::GetDumpPython()
{
  ASSERT(myHomardBoundary);
  return CORBA::string_dup(myHomardBoundary->GetDumpPython().c_str());
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Boundary_i::SetType(CORBA::Long Type)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetType(Type);
}
//=============================================================================
CORBA::Long HOMARD_Boundary_i::GetType()
{
  ASSERT(myHomardBoundary);
  return  CORBA::Long(myHomardBoundary->GetType());
}
//=============================================================================
void HOMARD_Boundary_i::SetMeshName(const char* MeshName)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetMeshName(MeshName);
}
//=============================================================================
char* HOMARD_Boundary_i::GetMeshName()
{
  ASSERT(myHomardBoundary);
  return CORBA::string_dup(myHomardBoundary->GetMeshName().c_str());
}
//=============================================================================
void HOMARD_Boundary_i::SetDataFile(const char* DataFile)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetDataFile(DataFile);
}
//=============================================================================
char* HOMARD_Boundary_i::GetDataFile()
{
  ASSERT(myHomardBoundary);
  return CORBA::string_dup(myHomardBoundary->GetDataFile().c_str());
}
//=============================================================================
void HOMARD_Boundary_i::SetCylinder(double X0, double X1, double X2, double X3, double X4, double X5, double X6)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetCylinder(X0, X1, X2, X3, X4, X5, X6);
}
//=============================================================================
void HOMARD_Boundary_i::SetSphere(double Xcentre, double Ycentre, double ZCentre, double rayon)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetSphere(Xcentre, Ycentre, ZCentre, rayon);
}
//=============================================================================
void HOMARD_Boundary_i::SetConeR(double Xcentre1, double Ycentre1, double Zcentre1, double Rayon1, double Xcentre2, double Ycentre2, double Zcentre2, double Rayon2)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetConeR(Xcentre1, Ycentre1, Zcentre1, Rayon1, Xcentre2, Ycentre2, Zcentre2, Rayon2);
}
//=============================================================================
void HOMARD_Boundary_i::SetConeA(double Xaxe, double Yaxe, double Zaxe, double Angle, double Xcentre, double Ycentre, double Zcentre)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetConeA(Xaxe, Yaxe, Zaxe, Angle, Xcentre, Ycentre, Zcentre);
}
//=============================================================================
void HOMARD_Boundary_i::SetTorus(double X0, double X1, double X2, double X3, double X4, double X5, double X6, double X7)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetTorus(X0, X1, X2, X3, X4, X5, X6, X7);
}
//=============================================================================
SMESHHOMARD::double_array* HOMARD_Boundary_i::GetCoords()
{
  ASSERT(myHomardBoundary);
  SMESHHOMARD::double_array_var aResult = new SMESHHOMARD::double_array();
  std::vector<double> mesCoor = myHomardBoundary->GetCoords();
  aResult->length(mesCoor .size());
  std::vector<double>::const_iterator it;
  int i = 0;
  for (it = mesCoor.begin(); it != mesCoor.end(); it++)
    aResult[i++] = (*it);
  return aResult._retn();
}
//=============================================================================
void HOMARD_Boundary_i::SetLimit(double Xincr, double Yincr, double Zincr)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->SetLimit(Xincr, Yincr, Zincr);
}
//=============================================================================
SMESHHOMARD::double_array* HOMARD_Boundary_i::GetLimit()
{
  ASSERT(myHomardBoundary);
  SMESHHOMARD::double_array_var aResult = new SMESHHOMARD::double_array();
  std::vector<double> mesCoor = myHomardBoundary->GetLimit();
  aResult->length(mesCoor .size());
  std::vector<double>::const_iterator it;
  int i = 0;
  for (it = mesCoor.begin(); it != mesCoor.end(); it++)
    aResult[i++] = (*it);
  return aResult._retn();
}
//=============================================================================
void HOMARD_Boundary_i::AddGroup(const char* Group)
{
  ASSERT(myHomardBoundary);
  myHomardBoundary->AddGroup(Group);
}
//=============================================================================
void HOMARD_Boundary_i::SetGroups(const SMESHHOMARD::ListGroupType& ListGroup)
{
  ASSERT(myHomardBoundary);
  std::list<std::string> ListString;
  for (unsigned int i = 0; i < ListGroup.length(); i++) {
    ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardBoundary->SetGroups(ListString);
}
//=============================================================================
SMESHHOMARD::ListGroupType*  HOMARD_Boundary_i::GetGroups()
{
  ASSERT(myHomardBoundary);
  const std::list<std::string>& ListString = myHomardBoundary->GetGroups();
  SMESHHOMARD::ListGroupType_var aResult = new SMESHHOMARD::ListGroupType;
  aResult->length(ListString.size());
  std::list<std::string>::const_iterator it;
  int i = 0;
  for (it = ListString.begin(); it != ListString.end(); it++)
  {
    aResult[i++] = CORBA::string_dup((*it).c_str());
  }
  return aResult._retn();
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Cas_i::HOMARD_Cas_i()
  : SALOME::GenericObj_i(SMESH_Gen_i::GetPOA())
{
  MESSAGE("Default constructor, not for use");
  ASSERT(0);
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Cas_i::HOMARD_Cas_i(SMESHHOMARD::HOMARD_Gen_var engine)
  : SALOME::GenericObj_i(SMESH_Gen_i::GetPOA())
{
  MESSAGE("HOMARD_Cas_i");
  _gen_i = engine;
  myHomardCas = new SMESHHOMARDImpl::HOMARD_Cas();
  ASSERT(myHomardCas);
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Cas_i::~HOMARD_Cas_i()
{
}
//=============================================================================
char* HOMARD_Cas_i::GetDumpPython()
{
  ASSERT(myHomardCas);
  return CORBA::string_dup(myHomardCas->GetDumpPython().c_str());
}
//=============================================================================
//=============================================================================
// Caracteristiques
//=============================================================================
//=============================================================================
void HOMARD_Cas_i::SetDirName(const char* NomDir)
{
  ASSERT(myHomardCas);
  int codret;
  // A. recuperation du nom; on ne fait rien si c'est le meme
  char* oldrep = GetDirName();
  if (strcmp(oldrep,NomDir) == 0) return;
  MESSAGE ("SetDirName : passage de oldrep = "<< oldrep << " a NomDir = "<<NomDir);
  // C. Changement/creation du repertoire
  codret = myHomardCas->SetDirName(NomDir);
  if (codret != 0) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text;
    if (codret == 1)
      text = "The directory for the case cannot be modified because some iterations are already defined.";
    else
      text = "The directory for the case cannot be reached.";
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }

  // D. En cas de reprise, deplacement du point de depart
  HOMARD_Gen_i* aGenImpl = SMESH::DownCast<HOMARD_Gen_i*>(_gen_i);
  HOMARD_Iteration_i* Iter0 = aGenImpl->GetIteration(0);
  int state = Iter0->GetNumber();
  if (state != 0) { // GetState()
    MESSAGE ("etat : " << state);
    // D.1. Nom local du repertoire de l'iteration de depart dans le repertoire actuel du cas
    char* DirNameIter = Iter0->GetDirNameLoc();
    MESSAGE ("SetDirName : nom actuel pour le repertoire de l iteration, DirNameIter = "<< DirNameIter);
    // D.2. Recherche d'un nom local pour l'iteration de depart dans le futur repertoire du cas
    char* nomDirIter = aGenImpl->CreateDirNameIter(NomDir, 0);
    MESSAGE ("SetDirName : nom futur pour le repertoire de l iteration, nomDirIter = "<< nomDirIter);
    // D.3. Creation du futur repertoire local pour l'iteration de depart
    std::string nomDirIterTotal;
    nomDirIterTotal = std::string(NomDir) + "/" + std::string(nomDirIter);
#ifndef WIN32
    if (mkdir(nomDirIterTotal.c_str(), S_IRWXU|S_IRGRP|S_IXGRP) != 0)
#else
    if (_mkdir(nomDirIterTotal.c_str()) != 0)
#endif
    {
      MESSAGE ("nomDirIterTotal : " << nomDirIterTotal);
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "The directory for the starting iteration cannot be created.";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    // D.4. Deplacement du contenu du repertoire
    std::string oldnomDirIterTotal;
    oldnomDirIterTotal = std::string(oldrep) + "/" + std::string(DirNameIter);
    std::string commande = "mv " + std::string(oldnomDirIterTotal) + "/*" + " " + std::string(nomDirIterTotal);
    codret = system(commande.c_str());
    if (codret != 0)
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "The starting point for the case cannot be moved into the new directory.";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    commande = "rm -rf " + std::string(oldnomDirIterTotal);
    codret = system(commande.c_str());
    if (codret != 0)
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "The starting point for the case cannot be deleted.";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    // D.5. Memorisation du nom du repertoire de l'iteration
    Iter0->SetDirNameLoc(nomDirIter);
  }
}
//=============================================================================
char* HOMARD_Cas_i::GetDirName()
{
  ASSERT(myHomardCas);
  return CORBA::string_dup(myHomardCas->GetDirName().c_str());
}
//=============================================================================
void HOMARD_Cas_i::SetBoundingBox(const SMESHHOMARD::extrema& LesExtrema)
{
  ASSERT(myHomardCas);
  std::vector<double> VExtrema;
  ASSERT(LesExtrema.length() == 10);
  VExtrema.resize(LesExtrema.length());
  for (int i = 0; i < (int)LesExtrema.length(); i++) {
    VExtrema[i] = LesExtrema[i];
  }
  myHomardCas->SetBoundingBox(VExtrema);
}
//=============================================================================
SMESHHOMARD::extrema* HOMARD_Cas_i::GetBoundingBox()
{
  ASSERT(myHomardCas);
  SMESHHOMARD::extrema_var aResult = new SMESHHOMARD::extrema();
  std::vector<double> LesExtremes = myHomardCas->GetBoundingBox();
  ASSERT(LesExtremes.size() == 10);
  aResult->length(10);
  for (unsigned int i = 0; i < LesExtremes.size(); i++) {
    aResult[i] = LesExtremes[i];
  }
  return aResult._retn();
}

//=============================================================================
void HOMARD_Cas_i::AddGroup(const char* Group)
{
  ASSERT(myHomardCas);
  myHomardCas->AddGroup(Group);
}
//=============================================================================
void HOMARD_Cas_i::SetGroups(const SMESHHOMARD::ListGroupType& ListGroup)
{
  ASSERT(myHomardCas);
  std::list<std::string> ListString;
  for (unsigned int i = 0; i < ListGroup.length(); i++)
  {
    ListString.push_back(std::string(ListGroup[i]));
  }
  myHomardCas->SetGroups(ListString);
}
//=============================================================================
SMESHHOMARD::ListGroupType* HOMARD_Cas_i::GetGroups()
{
  ASSERT(myHomardCas);
  const std::list<std::string>& ListString = myHomardCas->GetGroups();
  SMESHHOMARD::ListGroupType_var aResult = new SMESHHOMARD::ListGroupType();
  aResult->length(ListString.size());
  std::list<std::string>::const_iterator it;
  int i = 0;
  for (it = ListString.begin(); it != ListString.end(); it++) {
    aResult[i++] = CORBA::string_dup((*it).c_str());
  }
  return aResult._retn();
}

//=============================================================================
void HOMARD_Cas_i::AddBoundary(const char* BoundaryName)
{
  MESSAGE ("HOMARD_Cas_i::AddBoundary : BoundaryName = "<< BoundaryName);
  const char * Group = "";
  AddBoundaryGroup(BoundaryName, Group);
}
//=============================================================================
void HOMARD_Cas_i::AddBoundaryGroup(const char* BoundaryName, const char* Group)
{
  MESSAGE ("HOMARD_Cas_i::AddBoundaryGroup : BoundaryName = "<< BoundaryName << ", Group = " << Group);
  ASSERT(myHomardCas);
  // A. Préalables
  // A.1. Caractéristiques de la frontière à ajouter
  SMESHHOMARD::HOMARD_Boundary_ptr myBoundary = _gen_i->GetBoundary(BoundaryName);
  ASSERT(!CORBA::is_nil(myBoundary));
  int BoundaryType = myBoundary->GetType();
  MESSAGE (". BoundaryType = " << BoundaryType);
  // A.2. La liste des frontiere+groupes
  const std::list<std::string>& ListBoundaryGroup = myHomardCas->GetBoundaryGroup();
  std::list<std::string>::const_iterator it;
  // B. Controles
  const char * boun;
  int erreur = 0;
  while (erreur == 0)
  {
    // B.1. Si on ajoute une frontière CAO, elle doit être la seule frontière
    if (BoundaryType == -1)
    {
      for (it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++)
      {
        boun = (*it).c_str();
        MESSAGE ("..  Frontiere enregistrée : "<< boun);
        if (*it != BoundaryName)
        { erreur = 1;
          break; }
        // On saute le nom du groupe
        it++;
      }
    }
    if (erreur != 0) { break; }
    // B.2. Si on ajoute une frontière non CAO, il ne doit pas y avoir de frontière CAO
    if (BoundaryType != -1)
    {
      for (it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++)
      {
        boun = (*it).c_str();
        MESSAGE ("..  Frontiere enregistrée : "<< boun);
        SMESHHOMARD::HOMARD_Boundary_ptr myBoundary_0 = _gen_i->GetBoundary(boun);
        int BoundaryType_0 = myBoundary_0->GetType();
        MESSAGE (".. BoundaryType_0 = " << BoundaryType_0);
        if (BoundaryType_0 == -1)
        { erreur = 2;
          break; }
        // On saute le nom du groupe
        it++;
      }
      if (erreur != 0) { break; }
    }
    // B.3. Si on ajoute une frontière discrète, il ne doit pas y avoir d'autre frontière discrète
    if (BoundaryType == 0)
    {
      for (it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++)
      {
        boun = (*it).c_str();
        MESSAGE ("..  Frontiere enregistrée : "<< boun);
        if (boun != BoundaryName)
        {
          SMESHHOMARD::HOMARD_Boundary_ptr myBoundary_0 = _gen_i->GetBoundary(boun);
          int BoundaryType_0 = myBoundary_0->GetType();
          MESSAGE (".. BoundaryType_0 = " << BoundaryType_0);
          if (BoundaryType_0 == 0)
          { erreur = 3;
            break; }
        }
        // On saute le nom du groupe
        it++;
      }
      if (erreur != 0) { break; }
    }
    // B.4. Pour une nouvelle frontiere, publication dans l'arbre d'etudes sous le cas
    for (it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++) {
      MESSAGE ("..  Frontiere : "<< *it);
      // On saute le nom du groupe
      it++;
    }
    // B.5. Le groupe est-il deja enregistre pour une frontiere de ce cas ?
    for (it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++) {
      boun = (*it).c_str();
      it++;
      MESSAGE ("..  Groupe enregistré : "<< *it);
      if (*it == Group) {
        erreur = 5;
        break;
      }
    }
    if (erreur != 0) { break; }
    //
    break;
  }
  // F. Si aucune erreur, enregistrement du couple (frontiere,groupe) dans la reference du cas
  //    Sinon, arrêt
  if (erreur == 0) {
    myHomardCas->AddBoundaryGroup(BoundaryName, Group);
  }
  else {
    std::stringstream ss;
    ss << erreur;
    std::string str = ss.str();
    std::string texte;
    texte = "Erreur numéro " + str + " pour la frontière à enregistrer : " + std::string(BoundaryName);
    if (erreur == 1) { texte += "\nIl existe déjà la frontière "; }
    else if (erreur == 2) { texte += "\nIl existe déjà la frontière CAO "; }
    else if (erreur == 3) { texte += "\nIl existe déjà une frontière discrète : "; }
    else if (erreur == 5) { texte += "\nLe groupe " + std::string(Group) + " est déjà enregistré pour la frontière "; }
    texte += std::string(boun);
    //
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    
    if (SALOME::VerbosityActivated())
      texte += "\nInvalid AddBoundaryGroup";

    INFOS(texte);
    es.text = CORBA::string_dup(texte.c_str());
    throw SALOME::SALOME_Exception(es);
  }
}
//=============================================================================
SMESHHOMARD::ListBoundaryGroupType* HOMARD_Cas_i::GetBoundaryGroup()
{
  MESSAGE ("GetBoundaryGroup");
  ASSERT(myHomardCas);
  const std::list<std::string>& ListBoundaryGroup = myHomardCas->GetBoundaryGroup();
  SMESHHOMARD::ListBoundaryGroupType_var aResult = new SMESHHOMARD::ListBoundaryGroupType();
  aResult->length(ListBoundaryGroup.size());
  std::list<std::string>::const_iterator it;
  int i = 0;
  for (it = ListBoundaryGroup.begin(); it != ListBoundaryGroup.end(); it++)
  {
    aResult[i++] = CORBA::string_dup((*it).c_str());
  }
  return aResult._retn();
}
//=============================================================================
void HOMARD_Cas_i::SupprBoundaryGroup()
{
  MESSAGE ("SupprBoundaryGroup");
  ASSERT(myHomardCas);
  myHomardCas->SupprBoundaryGroup();
}

void HOMARD_Cas_i::AddIteration(const char* NomIteration)
{
  ASSERT(myHomardCas);
  myHomardCas->AddIteration(NomIteration);
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Iteration_i::HOMARD_Iteration_i()
{
  MESSAGE("Default constructor, not for use");
  ASSERT(0);
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Iteration_i::HOMARD_Iteration_i(SMESHHOMARD::HOMARD_Gen_var engine)
{
  MESSAGE("constructor");
  _gen_i = engine;
  myHomardIteration = new SMESHHOMARDImpl::HOMARD_Iteration();
  ASSERT(myHomardIteration);
}
//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Iteration_i::~HOMARD_Iteration_i()
{
}
//=============================================================================
void HOMARD_Iteration_i::SetName(const char* Name)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetName(Name);
}
//=============================================================================
char* HOMARD_Iteration_i::GetName()
{
  ASSERT(myHomardIteration);
  return CORBA::string_dup(myHomardIteration->GetName().c_str());
}
//=============================================================================
void HOMARD_Iteration_i::SetDirNameLoc(const char* NomDir)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetDirNameLoc(NomDir);
}
//=============================================================================
char* HOMARD_Iteration_i::GetDirNameLoc()
{
  ASSERT(myHomardIteration);
  return CORBA::string_dup(myHomardIteration->GetDirNameLoc().c_str());
}
//=============================================================================
char* HOMARD_Iteration_i::GetDirName()
{
  ASSERT(myHomardIteration);
  SMESHHOMARD::HOMARD_Cas_ptr caseiter = _gen_i->GetCase();
  std::string dirnamecase = caseiter->GetDirName();
  std::string dirname = dirnamecase + "/" +  GetDirNameLoc();
  return CORBA::string_dup(dirname.c_str());
}
//=============================================================================
void HOMARD_Iteration_i::SetNumber(CORBA::Long NumIter)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetNumber(NumIter);
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetNumber()
{
  ASSERT(myHomardIteration);
  return myHomardIteration->GetNumber();
}
//=============================================================================
void HOMARD_Iteration_i::SetState(CORBA::Long Etat)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetState(Etat);
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetState()
{
  ASSERT(myHomardIteration);
  return myHomardIteration->GetState();
}
//=============================================================================
void HOMARD_Iteration_i::SetMeshName(const char* NomMesh)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetMeshName(NomMesh);
}
//=============================================================================
char* HOMARD_Iteration_i::GetMeshName()
{
  ASSERT(myHomardIteration);
  return CORBA::string_dup(myHomardIteration->GetMeshName().c_str());
}
//=============================================================================
void HOMARD_Iteration_i::SetMeshFile(const char* MeshFile)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetMeshFile(MeshFile);
}
//=============================================================================
char* HOMARD_Iteration_i::GetMeshFile()
{
  ASSERT(myHomardIteration);
  return CORBA::string_dup(myHomardIteration->GetMeshFile().c_str());
}
//=============================================================================
void HOMARD_Iteration_i::SetLogFile(const char* LogFile)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetLogFile(LogFile);
}
//=============================================================================
char* HOMARD_Iteration_i::GetLogFile()
{
  ASSERT(myHomardIteration);
  return CORBA::string_dup(myHomardIteration->GetLogFile().c_str());
}
//=============================================================================
void HOMARD_Iteration_i::SetFileInfo(const char* FileInfo)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetFileInfo(FileInfo);
}
//=============================================================================
char* HOMARD_Iteration_i::GetFileInfo()
{
  ASSERT(myHomardIteration);
  return CORBA::string_dup(myHomardIteration->GetFileInfo().c_str());
}
//=============================================================================
void HOMARD_Iteration_i::SetInfoCompute(CORBA::Long MessInfo)
{
  ASSERT(myHomardIteration);
  myHomardIteration->SetInfoCompute(MessInfo);
}
//=============================================================================
CORBA::Long HOMARD_Iteration_i::GetInfoCompute()
{
  ASSERT(myHomardIteration);
  return myHomardIteration->GetInfoCompute();
}

//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Gen_i::HOMARD_Gen_i() : SALOME::GenericObj_i(SMESH_Gen_i::GetPOA()),
                               myIteration0(NULL),
                               myIteration1(NULL),
                               _ConfType(0),
                               _KeepMedOUT(true),
                               _PublishMeshOUT(false),
                               _KeepWorkingFiles(false),
                               _LogInFile(false),
                               _RemoveLogOnSuccess(false),
                               _VerboseLevel(0),
                               _MeshNameOUT(""),
                               _MeshFileOUT(""),
                               _LogFile(""),
                               _CaseOnMedFile(true),
                               _SmeshMesh(SMESH::SMESH_Mesh::_nil()),
                               _TmpMeshFile("")
{
  MESSAGE("constructor de HOMARD_Gen_i");
  myHomard = new SMESHHOMARDImpl::HOMARD_Gen;
}

//=============================================================================
/*!
 *  standard destructor
 */
//=============================================================================
HOMARD_Gen_i::~HOMARD_Gen_i()
{
  MESSAGE ("HOMARD_Gen_i::~HOMARD_Gen_i()");
}

//=============================================================================
//=============================================================================
// Destruction des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteBoundary(const char* BoundaryName)
{
  MESSAGE ("DeleteBoundary : BoundaryName = " << BoundaryName);
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = _mesBoundarys[BoundaryName];
  if (CORBA::is_nil(myBoundary)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid boundary";
    throw SALOME::SALOME_Exception(es);
  }

  // Boundaries should be deleted only after all cases deletion!!!

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  _mesBoundarys.erase(BoundaryName);

  return 0;
}

//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteIteration(int numIter)
{
  MESSAGE ("DeleteIteration : numIter = " << numIter);

  if (numIter == 0) {
    if (myIteration0 != NULL) {
      delete myIteration0;
      myIteration0 = NULL;
    }
  }
  else {
    if (myIteration1 != NULL) {
      // Invalide Iteration
      if (myIteration1->GetState() > 0) {
        myIteration1->SetState(1);
        if (!_KeepWorkingFiles) {
          std::string nomDir = myIteration1->GetDirName();
          std::string commande = "rm -rf " + nomDir;
          if (numIter > 0 && !_KeepMedOUT) {
            // Remove associated mesh file
            std::string nomFichier = myIteration1->GetMeshFile();
            commande = commande + ";rm -rf " + nomFichier;
          }
          MESSAGE ("commande = " << commande);
          if ((system(commande.c_str())) != 0) {
            SALOME::ExceptionStruct es;
            es.type = SALOME::BAD_PARAM;
            es.text = "The directory for the calculation cannot be cleared.";
            throw SALOME::SALOME_Exception(es);
          }
        }
      }

      delete myIteration1;
      myIteration1 = NULL;
    }
  }

  return 0;
}

//=============================================================================
//=============================================================================
// Invalidation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::InvalideBoundary(const char* BoundaryName)
{
  MESSAGE("InvalideBoundary : BoundaryName = " << BoundaryName);
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = _mesBoundarys[BoundaryName];
  if (CORBA::is_nil(myBoundary)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid boundary";
    throw SALOME::SALOME_Exception(es);
  }
  else {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "No change is allowed in a boundary. Ask for evolution.";
    throw SALOME::SALOME_Exception(es);
  }
}

//=============================================================================
//=============================================================================
// Association de lien entre des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::AssociateCaseIter(int numIter, const char* labelIter)
{
  MESSAGE("AssociateCaseIter : " << numIter << ", "  << labelIter);

  if (CORBA::is_nil(myCase)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case";
    throw SALOME::SALOME_Exception(es);
  }

  if (numIter == 0) {
    if (myIteration0 == NULL) {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
    }

    HOMARD_Cas_i* aCaseImpl = SMESH::DownCast<HOMARD_Cas_i*>(myCase);
    aCaseImpl->AddIteration(myIteration0->GetName());
  }
  else {
    if (myIteration1 == NULL) {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
    }

    HOMARD_Cas_i* aCaseImpl = SMESH::DownCast<HOMARD_Cas_i*>(myCase);
    aCaseImpl->AddIteration(myIteration1->GetName());
  }
}

//=============================================================================
//=============================================================================
// Recuperation des listes
//=============================================================================
//=============================================================================
SMESHHOMARD::listeBoundarys* HOMARD_Gen_i::GetAllBoundarysName()
{
  MESSAGE("GetAllBoundarysName");

  SMESHHOMARD::listeBoundarys_var ret = new SMESHHOMARD::listeBoundarys;
  ret->length(_mesBoundarys.size());
  std::map<std::string, SMESHHOMARD::HOMARD_Boundary_var>::const_iterator it;
  int i = 0;
  for (it = _mesBoundarys.begin();
       it != _mesBoundarys.end(); it++) {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}

//=============================================================================
//=============================================================================
// Recuperation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::GetBoundary(const char* nomBoundary)
{
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = _mesBoundarys[nomBoundary];
  ASSERT(!CORBA::is_nil(myBoundary));
  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::GetCase()
{
  ASSERT(!CORBA::is_nil(myCase));
  return SMESHHOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
HOMARD_Iteration_i* HOMARD_Gen_i::GetIteration(CORBA::Long numIter)
{
  if (numIter == 0) {
    ASSERT(myIteration0);
    return myIteration0;
  }

  ASSERT(myIteration1);
  return myIteration1;
}

//=============================================================================
//=============================================================================
// Nouvelles structures
//=============================================================================
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::newCase()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Cas_i* aServant = new HOMARD_Cas_i(engine);
  SMESHHOMARD::HOMARD_Cas_var aCase = SMESHHOMARD::HOMARD_Cas::_narrow(aServant->_this());
  return aCase._retn();
}
//=============================================================================
HOMARD_Iteration_i* HOMARD_Gen_i::newIteration()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Iteration_i* aServant = new HOMARD_Iteration_i(engine);
  return aServant;
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::newBoundary()
{
  SMESHHOMARD::HOMARD_Gen_var engine = POA_SMESHHOMARD::HOMARD_Gen::_this();
  HOMARD_Boundary_i* aServant = new HOMARD_Boundary_i(engine);
  SMESHHOMARD::HOMARD_Boundary_var aBoundary =
    SMESHHOMARD::HOMARD_Boundary::_narrow(aServant->_this());
  return aBoundary._retn();
}

//=============================================================================
// Creation of a case
// MeshName : name of the mesh
// smeshMesh : correspondent mesh
// theWorkingDir : path to working directory
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCaseOnMesh (const char* MeshName,
                                                            SMESH::SMESH_Mesh_ptr smeshMesh,
                                                            const char* theWorkingDir)
{
  INFOS("CreateCaseOnMesh");

  // A. Controles
  // A.1. Controle du nom :
  if (!myCase->_is_nil()) {
    DeleteCase();
  }

  // A.2. Controle du objet maillage
  if (CORBA::is_nil(smeshMesh)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The mesh object is null.";
    throw SALOME::SALOME_Exception(es);
  }
  MESSAGE("CreateCaseOnMesh : smeshMesh is not nil");

  // A.3. Write mesh object in a temporary file in the working directory
  std::string aTmpMeshFile = theWorkingDir;
  aTmpMeshFile = theWorkingDir;
  aTmpMeshFile += std::string("/") + std::string(MeshName) + "_saved_from_SMESH";
  _TmpMeshFile = aTmpMeshFile + ".med";
  SMESH_File aFile (_TmpMeshFile, false);
  for (int ii = 1; aFile.exists(); ii++) {
    _TmpMeshFile = aTmpMeshFile + std::string("_") + std::to_string(ii) + ".med";
    aFile = SMESH_File(_TmpMeshFile, false);
  }
  const char* MeshFile = _TmpMeshFile.c_str();
  bool toOverwrite = true;
  bool toFindOutDim = true;

  // Prevent dump of ExportMED
  {
    SMESH::TPythonDump pDump; // do not delete this line of code
    smeshMesh->ExportMED(MeshFile, false, -1, toOverwrite, toFindOutDim);
  }

  // A.4. Controle du fichier du maillage
  med_idt medIdt = MEDfileOpen(MeshFile, MED_ACC_RDONLY);
  bool existeMeshFile = medIdt >= 0;
  if (existeMeshFile) MEDfileClose(medIdt);
  MESSAGE("CreateCaseOnMesh : existeMeshFile = " << existeMeshFile);
  if (!existeMeshFile) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The mesh file does not exist.";
    throw SALOME::SALOME_Exception(es);
  }

  // B. Creation de l'objet cas
  myCase = newCase();
  _CaseOnMedFile = false;
  _SmeshMesh = SMESH::SMESH_Mesh::_duplicate(smeshMesh);

  // C. Caracteristiques du maillage
  if (existeMeshFile) {
    // Les valeurs extremes des coordonnées
    //MESSAGE ("CreateCaseOnMesh : Les valeurs extremes des coordonnées");
    std::vector<double> LesExtremes = GetBoundingBoxInMedFile(MeshFile);
    SMESHHOMARD::extrema_var aSeq = new SMESHHOMARD::extrema();
    if (LesExtremes.size() != 10) { return 0; }
    aSeq->length(10);
    for (unsigned int i = 0; i < LesExtremes.size(); i++)
      aSeq[i] = LesExtremes[i];
    myCase->SetBoundingBox(aSeq);
    // Les groupes
    //MESSAGE ("CreateCaseOnMesh : Les groupes");
    std::set<std::string> LesGroupes = GetListeGroupesInMedFile(MeshFile);
    SMESHHOMARD::ListGroupType_var aSeqGroupe = new SMESHHOMARD::ListGroupType;
    aSeqGroupe->length(LesGroupes.size());
    std::set<std::string>::const_iterator it;
    int i = 0;
    for (it = LesGroupes.begin(); it != LesGroupes.end(); it++)
      aSeqGroupe[i++] = (*it).c_str();
    myCase->SetGroups(aSeqGroupe);
  }

  // D. L'iteration initiale du cas
  MESSAGE ("CreateCaseOnMesh : iteration initiale du cas");
  // D.1. Recherche d'un nom : par defaut, on prend le nom du maillage correspondant.
  // Si ce nom d'iteration existe deja, on incremente avec 0, 1, 2, etc.
  MESSAGE("CreateCaseOnMesh : ==> NomIteration = " << MeshName);

  // D.2. Creation de l'iteration 0
  myIteration0 = newIteration();
  myIteration0->SetName(MeshName);
  AssociateCaseIter(0, "IterationHomard");

  // D.4. Maillage correspondant
  if (existeMeshFile) {
    myIteration0->SetMeshFile(MeshFile);
  }
  myIteration0->SetMeshName(MeshName);
  myIteration0->SetNumber(0);
  myIteration0->SetState(0);

  // Only after full initialization of Iteration0
  myCase->SetDirName(theWorkingDir);

  return SMESHHOMARD::HOMARD_Cas::_duplicate(myCase);
}

//=============================================================================
// Creation of a case
// MeshName : name of the mesh
// MeshFile : med file
// theWorkingDir : path to working directory
//=============================================================================
SMESHHOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCase(const char* MeshName,
                                                     const char* MeshFile,
                                                     const char* theWorkingDir)
{
  INFOS("CreateCase : MeshName = " << MeshName << ", MeshFile = " << MeshFile);

  // A. Controles
  // A.1. Controle du nom :
  if (!myCase->_is_nil()) {
    DeleteCase();
  }

  // A.3. Controle du fichier du maillage
  med_idt medIdt = MEDfileOpen(MeshFile, MED_ACC_RDONLY);
  bool existeMeshFile = medIdt >= 0;
  if (existeMeshFile) MEDfileClose(medIdt);
  MESSAGE("CreateCase : existeMeshFile = " << existeMeshFile);
  if (!existeMeshFile) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The mesh file does not exist.";
    throw SALOME::SALOME_Exception(es);
  }

  // B. Creation de l'objet cas
  myCase = newCase();
  _CaseOnMedFile = true;

  // C. Caracteristiques du maillage
  if (existeMeshFile) {
    // Les valeurs extremes des coordonnées
    //MESSAGE ("CreateCase : Les valeurs extremes des coordonnées");
    std::vector<double> LesExtremes = GetBoundingBoxInMedFile(MeshFile);
    SMESHHOMARD::extrema_var aSeq = new SMESHHOMARD::extrema();
    if (LesExtremes.size() != 10) { return 0; }
    aSeq->length(10);
    for (unsigned int i = 0; i < LesExtremes.size(); i++)
      aSeq[i] = LesExtremes[i];
    myCase->SetBoundingBox(aSeq);
    // Les groupes
    //MESSAGE ("CreateCase : Les groupes");
    std::set<std::string> LesGroupes = GetListeGroupesInMedFile(MeshFile);
    SMESHHOMARD::ListGroupType_var aSeqGroupe = new SMESHHOMARD::ListGroupType;
    aSeqGroupe->length(LesGroupes.size());
    std::set<std::string>::const_iterator it;
    int i = 0;
    for (it = LesGroupes.begin(); it != LesGroupes.end(); it++)
      aSeqGroupe[i++] = (*it).c_str();
    myCase->SetGroups(aSeqGroupe);
  }

  // D. L'iteration initiale du cas
  MESSAGE ("CreateCase : iteration initiale du cas");
  // D.1. Recherche d'un nom : par defaut, on prend le nom du maillage correspondant.
  // Si ce nom d'iteration existe deja, on incremente avec 0, 1, 2, etc.
  MESSAGE("CreateCas : ==> NomIteration = " << MeshName);

  // D.2. Creation de l'iteration
  myIteration0 = newIteration();
  myIteration0->SetName(MeshName);
  AssociateCaseIter(0, "IterationHomard");

  // D.4. Maillage correspondant
  if (existeMeshFile) {
    myIteration0->SetMeshFile(MeshFile);
  }
  myIteration0->SetMeshName(MeshName);
  myIteration0->SetNumber(0);
  myIteration0->SetState(0);

  // Only after full initialization of Iteration0
  myCase->SetDirName(theWorkingDir);

  return SMESHHOMARD::HOMARD_Cas::_duplicate(myCase);
}

//=============================================================================
// Create Iteration1
//=============================================================================
HOMARD_Iteration_i* HOMARD_Gen_i::CreateIteration()
{
  if (myIteration0 == NULL) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The parent iteration is not defined.";
    throw SALOME::SALOME_Exception(es);
  }

  if (CORBA::is_nil(myCase)) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case context";
    throw SALOME::SALOME_Exception(es);
  }
  const char* nomDirCase = myCase->GetDirName();

  if (myIteration1 != NULL) {
    DeleteIteration(1);
  }

  myIteration1 = newIteration();
  if (myIteration1 == NULL) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Unable to create the iteration 1";
    throw SALOME::SALOME_Exception(es);
  }

  // Nom de l'iteration et du maillage
  myIteration1->SetName("Iter_1");
  myIteration1->SetMeshName("Iter_1");
  myIteration1->SetState(1);
  myIteration1->SetNumber(1);

  int nbitercase = 1;
  char* nomDirIter = CreateDirNameIter(nomDirCase, nbitercase);
  myIteration1->SetDirNameLoc(nomDirIter);

  // Le nom du fichier du maillage MED est indice par le nombre d'iterations du cas.
  // Si on a une chaine unique depuis le depart, ce nombre est le meme que le
  // numero d'iteration dans la sucession : maill.01.med, maill.02.med, etc... C'est la
  // situation la plus frequente.
  // Si on a plusieurs branches, donc des iterations du meme niveau d'adaptation, utiliser
  // le nombre d'iterations du cas permet d'eviter les collisions.
  int jaux;
  if      (nbitercase <    100) { jaux = 2; }
  else if (nbitercase <   1000) { jaux = 3; }
  else if (nbitercase <  10000) { jaux = 4; }
  else if (nbitercase < 100000) { jaux = 5; }
  else                          { jaux = 9; }
  std::ostringstream iaux;
  iaux << std::setw(jaux) << std::setfill('0') << nbitercase;
  std::stringstream MeshFile;
  MeshFile << nomDirCase << "/maill." << iaux.str() << ".med";
  myIteration1->SetMeshFile(MeshFile.str().c_str());

  // Association avec le cas
  std::string nomIterParent = myIteration0->GetName();
  std::string label = "IterationHomard_" + nomIterParent;
  AssociateCaseIter(1, label.c_str());

  return myIteration1;
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundary(const char* BoundaryName,
                                                              CORBA::Long BoundaryType)
{
  MESSAGE ("CreateBoundary : BoundaryName  = " << BoundaryName <<
           ", BoundaryType = " << BoundaryType);

  // Controle du nom :
  if ((_mesBoundarys).find(BoundaryName) != (_mesBoundarys).end()) {
    MESSAGE ("CreateBoundary : la frontiere " << BoundaryName << " existe deja");
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This boundary has already been defined";
    throw SALOME::SALOME_Exception(es);
  }

  SMESHHOMARD::HOMARD_Boundary_var myBoundary = newBoundary();
  myBoundary->SetName(BoundaryName);
  myBoundary->SetType(BoundaryType);

  _mesBoundarys[BoundaryName] = myBoundary;

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryCAO(const char* BoundaryName, const char* CAOFile)
{
  MESSAGE ("CreateBoundaryCAO : BoundaryName  = " << BoundaryName << ", CAOFile = " << CAOFile);
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, -1);
  myBoundary->SetDataFile(CAOFile);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryDi(const char* BoundaryName, const char* MeshName, const char* MeshFile)
{
  MESSAGE ("CreateBoundaryDi : BoundaryName  = " << BoundaryName << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile);
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 0);
  myBoundary->SetDataFile(MeshFile);
  myBoundary->SetMeshName(MeshName);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryCylinder(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double Rayon)
{
  MESSAGE ("CreateBoundaryCylinder : BoundaryName  = " << BoundaryName);
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if (Rayon <= 0.0)
  { es.text = "The radius must be positive.";
    error = 1; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe);
  if (daux < 0.0000001)
  { es.text = "The axis must be a non 0 vector.";
    error = 2; }
  if (error != 0)
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 1);
  myBoundary->SetCylinder(Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, Rayon);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundarySphere(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Rayon)
{
  MESSAGE ("CreateBoundarySphere : BoundaryName  = " << BoundaryName);
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if (Rayon <= 0.0)
  { es.text = "The radius must be positive.";
    error = 1; }
  if (error != 0)
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 2);
  myBoundary->SetSphere(Xcentre, Ycentre, Zcentre, Rayon);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryConeA(const char* BoundaryName,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe, CORBA::Double Angle,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre)
{
  MESSAGE ("CreateBoundaryConeA : BoundaryName  = " << BoundaryName);
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if (Angle <= 0.0 || Angle >= 90.0)
  { es.text = "The angle must be included higher than 0 degree and lower than 90 degrees.";
    error = 1; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe);
  if (daux < 0.0000001)
  { es.text = "The axis must be a non 0 vector.";
    error = 2; }
  if (error != 0)
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 3);
  myBoundary->SetConeA(Xaxe, Yaxe, Zaxe, Angle, Xcentre, Ycentre, Zcentre);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryConeR(const char* BoundaryName,
                                      CORBA::Double Xcentre1, CORBA::Double Ycentre1, CORBA::Double Zcentre1, CORBA::Double Rayon1,
                                      CORBA::Double Xcentre2, CORBA::Double Ycentre2, CORBA::Double Zcentre2, CORBA::Double Rayon2)
{
  MESSAGE ("CreateBoundaryConeR : BoundaryName  = " << BoundaryName);
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if (Rayon1 < 0.0 || Rayon2 < 0.0)
  { es.text = "The radius must be positive.";
    error = 1; }
  double daux = fabs(Rayon2-Rayon1);
  if (daux < 0.0000001)
  { es.text = "The radius must be different.";
    error = 2; }
  daux = fabs(Xcentre2-Xcentre1) + fabs(Ycentre2-Ycentre1) + fabs(Zcentre2-Zcentre1);
  if (daux < 0.0000001)
  { es.text = "The centers must be different.";
    error = 3; }
  if (error != 0)
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 4);
  myBoundary->SetConeR(Xcentre1, Ycentre1, Zcentre1, Rayon1, Xcentre2, Ycentre2, Zcentre2, Rayon2);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
SMESHHOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryTorus(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double RayonRev, CORBA::Double RayonPri)
{
  MESSAGE ("CreateBoundaryTorus : BoundaryName  = " << BoundaryName);
//
  SALOME::ExceptionStruct es;
  int error = 0;
  if ((RayonRev <= 0.0) || (RayonPri <= 0.0))
  { es.text = "The radius must be positive.";
    error = 1; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe);
  if (daux < 0.0000001)
  { es.text = "The axis must be a non 0 vector.";
    error = 2; }
  if (error != 0)
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 5);
  myBoundary->SetTorus(Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, RayonRev, RayonPri);

  return SMESHHOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}

//=============================================================================
//=============================================================================
CORBA::Long HOMARD_Gen_i::Compute()
{
  INFOS ("Compute");
  // A. Prealable
  int codret = 0;

  // A.0. Create Iteration 1
  CreateIteration();
  myIteration1->SetInfoCompute(_VerboseLevel);
  myIteration1->SetMeshName(_MeshNameOUT.c_str());
  myIteration1->SetMeshFile(_MeshFileOUT.c_str());
  if (_LogInFile) myIteration1->SetLogFile(_LogFile.c_str());

  // A.1. L'objet iteration
  ASSERT(myIteration1);

  // A.2. Controle de la possibilite d'agir
  // A.2.1. Etat de l'iteration
  int etat = myIteration1->GetState();
  MESSAGE ("etat = " << etat);
  // A.2.2. On ne calcule pas l'iteration initiale, ni une iteration deja calculee
  if (etat == 2) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration is already computed.";
    throw SALOME::SALOME_Exception(es);
  }

  // A.3. Numero de l'iteration
  //     siterp1 : numero de l'iteration a traiter
  //     Si adaptation :
  //        siter: numero de l'iteration parent, ou 0 si deja au debut mais cela ne servira pas !
  //     Ou si information :
  //        siter = siterp1
  int NumeIter = myIteration1->GetNumber();
  std::string siterp1;
  std::stringstream saux1;
  saux1 << NumeIter;
  siterp1 = saux1.str();
  if (NumeIter < 10) { siterp1 = "0" + siterp1; }

  std::string siter;
  std::stringstream saux0;
  int iaux = max(0, NumeIter-1);
  saux0 << iaux;
  siter = saux0.str();
  if (NumeIter < 11) { siter = "0" + siter; }

  // A.4. Le cas
  ASSERT(!CORBA::is_nil(myCase));

  // B. Les répertoires
  // B.1. Le répertoire courant
  std::string nomDirWork = getenv("PWD");
  // B.2. Le sous-répertoire de l'iteration a traiter
  char* DirCompute = ComputeDirManagement();
  MESSAGE(". DirCompute = " << DirCompute);

  // C. Le fichier des messages
  // C.1. Le deroulement de l'execution de HOMARD
  std::string LogFile = myIteration1->GetLogFile();
  if (LogFile.empty()) {
    LogFile = DirCompute;
    LogFile += "/Liste." + siter + ".vers." + siterp1 + ".log";
    myIteration1->SetLogFile(LogFile.c_str());
  }
  MESSAGE (". LogFile = " << LogFile);
  // C.2. Le bilan de l'analyse du maillage
  std::string FileInfo = DirCompute;
  FileInfo += "/apad." + siterp1 + ".bilan";
  myIteration1->SetFileInfo(FileInfo.c_str());

   // D. On passe dans le répertoire de l'iteration a calculer
  MESSAGE (". On passe dans DirCompute = " << DirCompute);
  CHDIR(DirCompute);

  // E. Les données de l'exécution HOMARD
  // E.1. L'objet du texte du fichier de configuration
  SMESHHOMARDImpl::HomardDriver* myDriver = new SMESHHOMARDImpl::HomardDriver(siter, siterp1);
  myDriver->TexteInit(DirCompute, LogFile, "English");

  // E.2. Le maillage associe a l'iteration
  const char* NomMesh = myIteration1->GetMeshName();
  MESSAGE (". NomMesh = " << NomMesh);
  const char* MeshFile = myIteration1->GetMeshFile();
  MESSAGE (". MeshFile = " << MeshFile);

  // E.3. Les données du traitement HOMARD
  iaux = 1;
  myDriver->TexteMaillageHOMARD(DirCompute, siterp1, iaux);
  myDriver->TexteMaillage(NomMesh, MeshFile, 1);
  codret = ComputeAdap(myDriver);

  // E.4. Ajout des informations liees a l'eventuel suivi de frontiere
  int BoundaryOption = DriverTexteBoundary(myDriver);

  // E.5. Ecriture du texte dans le fichier
  MESSAGE (". Ecriture du texte dans le fichier de configuration; codret = "<<codret);
  if (codret == 0)
  { myDriver->CreeFichier(); }

  // G. Execution
  //
  int codretexec = 1789;
  if (codret == 0) {
    codretexec = myDriver->ExecuteHomard();
    //
    MESSAGE ("Erreur en executant HOMARD : " << codretexec);
    // En mode adaptation, on ajuste l'etat de l'iteration
    if (codretexec == 0) { myIteration1->SetState(2); }
    else                 { myIteration1->SetState(1); }
  }

  // H. Gestion des resultats
  if (codret == 0) {
    std::string Commentaire;
    // H.1. Le fichier des messages, dans tous les cas
    Commentaire = "log";
    Commentaire += " " + siterp1;

    // H.2. Si tout s'est bien passe :
    if (codretexec == 0) {
      // H.2.1. Le fichier de bilan
      Commentaire = "Summary";
      Commentaire += " " + siterp1;
      // H.2.2. Le fichier de  maillage obtenu
      std::stringstream saux0;
      Commentaire = "Mesh";
      Commentaire += " " + siterp1;
      if (_PublishMeshOUT) PublishResultInSmesh(MeshFile);
    }
    // H.3 Message d'erreur
    if (codretexec != 0) {
      std::string text = "";
      // Message d'erreur en cas de probleme en adaptation
      text = "Error during the adaptation.\n";
      bool stopvu = false;
      std::ifstream fichier(LogFile.c_str());
      if (fichier) { // ce test échoue si le fichier n'est pas ouvert
        std::string ligne; // variable contenant chaque ligne lue
        while (std::getline(fichier, ligne)) {
          //INFOS(ligne);
          if (stopvu) { text += ligne+ "\n"; }
          else {
            int position = ligne.find("===== HOMARD ===== STOP =====");
            if (position > 0) { stopvu = true; }
          }
        }
      }
      text += "\n\nSee the file " + LogFile + "\n";
      INFOS (text);
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
  }

  // I. Menage et retour dans le répertoire du cas
  if (codret == 0) {
    delete myDriver;
    MESSAGE (". On retourne dans nomDirWork = " << nomDirWork);
    CHDIR(nomDirWork.c_str());
  }

  // J. Suivi de la frontière CAO
  if (codret == 0) {
    if ((BoundaryOption % 5 == 0) && (codretexec == 0)) {
      MESSAGE ("Suivi de frontière CAO");
      codret = ComputeCAO();
    }
  }

  if (codretexec == 0) {
    // Python Dump
    PythonDump();

    // Delete log file, if required
    if (myIteration1 != NULL) {
      MESSAGE("myIteration1->GetLogFile() = " << myIteration1->GetLogFile());
      if (_LogInFile && _RemoveLogOnSuccess) {
        // Remove log file on success
        SMESH_File(myIteration1->GetLogFile(), false).remove();
      }
    }

    // Clean all data
    DeleteCase();
  }

  return codretexec;
}

void HOMARD_Gen_i::DeleteCase()
{
  MESSAGE ("DeleteCase");
  if (myCase->_is_nil()) return;

  // Delete all boundaries
  _mesBoundarys.clear();

  // Delete iterations
  DeleteIteration(1);
  DeleteIteration(0);

  // Delete case
  myCase = SMESHHOMARD::HOMARD_Cas::_nil();

  // Delete tmp mesh file
  if (!_CaseOnMedFile && !_TmpMeshFile.empty()) {
    SMESH_File aFile (_TmpMeshFile, false);
    if (aFile.exists()) aFile.remove();
    _TmpMeshFile = "";
  }
  _SmeshMesh = SMESH::SMESH_Mesh::_nil();
}

//=============================================================================
// Calcul d'une iteration : partie spécifique à l'adaptation
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeAdap(SMESHHOMARDImpl::HomardDriver* myDriver)
{
  MESSAGE ("ComputeAdap");

  // A. Prealable
  // A.1. Bases
  int codret = 0;
  // Numero de l'iteration
  int NumeIter = 1;
  std::stringstream saux0;
  saux0 << NumeIter-1;
  std::string siter = saux0.str();
  if (NumeIter < 11) { siter = "0" + siter; }

  // B. L'iteration parent
  ASSERT(myIteration0);

  // C. Le sous-répertoire de l'iteration precedente
  char* DirComputePa = ComputeDirPaManagement();
  MESSAGE(". DirComputePa = " << DirComputePa);

  // D. Les données de l'adaptation HOMARD
  // D.1. Le type de conformite
  MESSAGE (". ConfType = " << _ConfType);

  // D.3. Le maillage de depart
  const char* NomMeshParent = myIteration0->GetMeshName();
  MESSAGE (". NomMeshParent = " << NomMeshParent);
  const char* MeshFileParent = myIteration0->GetMeshFile();
  MESSAGE (". MeshFileParent = " << MeshFileParent);

  // D.4. Le maillage associe a l'iteration
  const char* MeshFile = myIteration1->GetMeshFile();
  MESSAGE (". MeshFile = " << MeshFile);
  FILE *file = fopen(MeshFile,"r");
  if (file != NULL) {
    fclose(file);
    // CleanOption = 0 : report an error if output mesh file exists
    // CleanOption = 1 : destruction du répertoire d'execution
    int CleanOption = 1;
    if (CleanOption == 0) {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "MeshFile : " + std::string(MeshFile) + " already exists ";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
    }
    else {
      std::string commande = "rm -f " + std::string(MeshFile);
      codret = system(commande.c_str());
      if (codret != 0) {
        SALOME::ExceptionStruct es;
        es.type = SALOME::BAD_PARAM;
        es.text = "The mesh file cannot be deleted.";
        throw SALOME::SALOME_Exception(es);
      }
    }
  }

  // E. Texte du fichier de configuration
  // E.1. Incontournables du texte
  myDriver->TexteAdap();
  int iaux = 0;
  myDriver->TexteMaillageHOMARD(DirComputePa, siter, iaux);
  myDriver->TexteMaillage(NomMeshParent, MeshFileParent, 0);
  myDriver->TexteConfRaffDera(_ConfType);

  // E.6. Ajout des options avancees
  //myDriver->TexteAdvanced(NivMax, DiamMin, AdapInit, ExtraOutput);
  myDriver->TexteAdvanced(-1, -1.0, 0, 1);

  // E.7. Ajout des informations sur le deroulement de l'execution
  int MessInfo = myIteration1->GetInfoCompute();
  MESSAGE (". MessInfo = " << MessInfo);
  myDriver->TexteInfoCompute(MessInfo);

  return codret;
}
//=============================================================================
// Calcul d'une iteration : partie spécifique au suivi de frontière CAO
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeCAO()
{
  MESSAGE ("ComputeCAO");

  // A. Prealable
  // A.1. Bases
  int codret = 0;
#ifndef DISABLE_HOMARD_ADAPT
  // A.2. Le sous-répertoire de l'iteration en cours de traitement
  char* DirCompute = myIteration1->GetDirName();
  // A.3. Le maillage résultat de l'iteration en cours de traitement
  char* MeshFile = myIteration1->GetMeshFile();

  // B. Les données pour FrontTrack
  // B.1. Le maillage à modifier
  const std::string theInputMedFile = MeshFile;
  MESSAGE (". theInputMedFile  = " << theInputMedFile);

  // B.2. Le maillage après modification : fichier identique
  const std::string theOutputMedFile = MeshFile;
  MESSAGE (". theOutputMedFile = " << theInputMedFile);

  // B.3. La liste des fichiers contenant les numéros des noeuds à bouger
  std::vector< std::string > theInputNodeFiles;
  MESSAGE (". DirCompute = " << DirCompute);
  std::basic_string<char>::size_type bilan;
  int icpt = 0;
#ifndef WIN32
  DIR *dp;
  struct dirent *dirp;
  dp  = opendir(DirCompute);
  while ((dirp = readdir(dp)) != NULL)
  {
    std::string file_name(dirp->d_name);
    bilan = file_name.find("fr");
    if (bilan != string::npos)
    {
      std::stringstream filename_total;
      filename_total << DirCompute << "/" << file_name;
      theInputNodeFiles.push_back(filename_total.str());
      icpt += 1;
    }
  }
#else
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  hFind = FindFirstFile(DirNameStart, &ffd);
  if (INVALID_HANDLE_VALUE != hFind) {
    while (FindNextFile(hFind, &ffd) != 0) {
      std::string file_name(ffd.cFileName);
      bilan = file_name.find("fr");
      if (bilan != string::npos)
      {
        std::stringstream filename_total;
        filename_total << DirCompute << "/" << file_name;
        theInputNodeFiles.push_back(filename_total.str());
        icpt += 1;
      }
    }
    FindClose(hFind);
  }
#endif
  for (int i = 0; i < icpt; i++)
  { MESSAGE (". theInputNodeFiles["<< i << "] = " << theInputNodeFiles[i]); }

  // B.4. Le fichier de la CAO
  SMESHHOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  std::string BoundaryName = std::string((*ListBoundaryGroupType)[0]);
  MESSAGE (". BoundaryName = " << BoundaryName);
  SMESHHOMARD::HOMARD_Boundary_var myBoundary = _mesBoundarys[BoundaryName];
  const std::string theXaoFileName = myBoundary->GetDataFile();
  MESSAGE (". theXaoFileName = " << theXaoFileName);

  // C. Lancement des projections
  MESSAGE (". Lancement des projections");

  assert(Py_IsInitialized());
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
  PyRun_SimpleString("from FrontTrack import FrontTrack");
  // FrontTrack().track( fic_med_brut, fic_med_new, l_fr, xao_file )
  std::string pyCommand ("FrontTrack().track( \"");
  pyCommand += theInputMedFile + "\", \"" + theOutputMedFile + "\", [";
  for (int i = 0; i < icpt; i++) {
    if (i > 0) pyCommand += ", ";
    pyCommand += "\"";
    pyCommand += theInputNodeFiles[i];
    pyCommand += "\"";
  }
  pyCommand += "], \"" + theXaoFileName + "\", False )";
  MESSAGE (". Lancement des projections: pyCommand = " << pyCommand);
  PyRun_SimpleString(pyCommand.c_str());
  PyGILState_Release(gstate);

  // D. Transfert des coordonnées modifiées dans le fichier historique de HOMARD
  //    On lance une exécution spéciale de HOMARD en attendant
  //    de savoir le faire avec MEDCoupling
  MESSAGE (". Transfert des coordonnées");
  codret = ComputeCAObis();
#endif
  return codret;
}
//=============================================================================
//=============================================================================
// Transfert des coordonnées en suivi de frontière CAO
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeCAObis()
{
  MESSAGE ("ComputeCAObis");

  // A. Prealable
  int codret = 0;

  // A.1. Controle de la possibilite d'agir
  // A.1.1. Etat de l'iteration
  int etat = myIteration1->GetState();
  MESSAGE ("etat = " << etat);
  // A.1.2. L'iteration doit être calculee
  if (etat == 1) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration is not computed.";
    throw SALOME::SALOME_Exception(es);
    return 1;
  }
  // A.2. Numero de l'iteration
  //     siterp1 : numero de l'iteration a traiter
  int NumeIter = myIteration1->GetNumber();
  std::string siterp1;
  std::stringstream saux1;
  saux1 << NumeIter;
  siterp1 = saux1.str();
  if (NumeIter < 10) { siterp1 = "0" + siterp1; }
  MESSAGE ("siterp1 = "<<siterp1);

  // A.3. Le cas
  ASSERT(!CORBA::is_nil(myCase));

  // A.4. Le sous-répertoire de l'iteration a traiter
  char* DirCompute = myIteration1->GetDirName();
  MESSAGE(". DirCompute = " << DirCompute);

  // C. Le fichier des messages
  std::string LogFile = DirCompute;
  LogFile += "/Liste." + siterp1 + ".maj_coords.log";
  MESSAGE (". LogFile = " << LogFile);
  myIteration1->SetFileInfo(LogFile.c_str());

  // D. On passe dans le répertoire de l'iteration a calculer
  MESSAGE (". On passe dans DirCompute = " << DirCompute);
  CHDIR(DirCompute);

  // E. Les données de l'exécution HOMARD
  // E.1. L'objet du texte du fichier de configuration
  SMESHHOMARDImpl::HomardDriver* myDriver = new SMESHHOMARDImpl::HomardDriver("", siterp1);
  myDriver->TexteInit(DirCompute, LogFile, "English");

  // E.2. Le maillage associe a l'iteration
  const char* NomMesh = myIteration1->GetMeshName();
  MESSAGE (". NomMesh = " << NomMesh);
  const char* MeshFile = myIteration1->GetMeshFile();
  MESSAGE (". MeshFile = " << MeshFile);

  // E.3. Les données du traitement HOMARD
  myDriver->TexteMajCoords(NumeIter);
  int iaux = 0;
  myDriver->TexteMaillageHOMARD(DirCompute, siterp1, iaux);
  myDriver->TexteMaillage(NomMesh, MeshFile, 0);
  //
  // E.4. Ecriture du texte dans le fichier
  MESSAGE (". Ecriture du texte dans le fichier de configuration; codret = "<<codret);
  if (codret == 0) myDriver->CreeFichier();

  // F. Execution
  //
  int codretexec = 1789;
  if (codret == 0) {
    codretexec = myDriver->ExecuteHomard();
    MESSAGE ("Erreur en executant HOMARD : " << codretexec);
  }

  // G. Gestion des resultats
  if (codret == 0) {
    // G.1. Le fichier des messages, dans tous les cas
    std::string Commentaire = "logmaj_coords";
    // G.2 Message d'erreur
    if (codretexec != 0) {
      std::string text = "\n\nSee the file " + LogFile + "\n";
      INFOS (text);
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);

      // On force le succes pour pouvoir consulter le fichier log
      codretexec = 0;
    }
  }

  // H. Menage et retour dans le répertoire du cas
  if (codret == 0) { delete myDriver; }

  return codret;
}
//=============================================================================
// Creation d'un nom de sous-répertoire pour l'iteration au sein d'un répertoire parent
//  nomrep : nom du répertoire parent
//  num : le nom du sous-répertoire est sous la forme 'In', n est >= num
//=============================================================================
char* HOMARD_Gen_i::CreateDirNameIter(const char* nomrep, CORBA::Long num)
{
#ifndef DISABLE_HOMARD_ADAPT 
  MESSAGE ("CreateDirNameIter : nomrep ="<< nomrep << ", num = "<<num);
  // On verifie que le répertoire parent existe
  int codret = CHDIR(nomrep);
  if (codret != 0) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the case does not exist.";
    throw SALOME::SALOME_Exception(es);
  }
  std::string nomDirActuel = getenv("PWD");
  std::string DirName;
  // On boucle sur tous les noms possibles jusqu'a trouver un nom correspondant a un répertoire inconnu
  bool a_chercher = true;
  while (a_chercher) {
    // On passe dans le répertoire parent

    CHDIR(nomrep);
    // On recherche un nom sous la forme Iabc, avec abc representant le numero
    int jaux;
    if      (num <    100) { jaux = 2; }
    else if (num <   1000) { jaux = 3; }
    else if (num <  10000) { jaux = 4; }
    else if (num < 100000) { jaux = 5; }
    else                     { jaux = 9; }
    std::ostringstream iaux;
    iaux << std::setw(jaux) << std::setfill('0') << num;
    std::ostringstream DirNameA;
    DirNameA << "I" << iaux.str();
    // Si on ne pas peut entrer dans le répertoire, on doit verifier
    // que c'est bien un probleme d'absence
    if (CHDIR(DirNameA.str().c_str()) != 0)
    {
      bool existe = false;
#ifndef WIN32
      DIR *dp;
      struct dirent *dirp;
      dp  = opendir(nomrep);
      while ((dirp = readdir(dp)) != NULL)
      {
        std::string file_name(dirp->d_name);
#else
      HANDLE hFind = INVALID_HANDLE_VALUE;
      WIN32_FIND_DATA ffd;
      hFind = FindFirstFile(nomrep, &ffd);
      if (INVALID_HANDLE_VALUE != hFind) {
        while (FindNextFile(hFind, &ffd) != 0) {
         if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
         std::string file_name(ffd.cFileName);
#endif
        if (file_name == DirNameA.str()) { existe = true; }
      }
#ifndef WIN32
      closedir(dp);
#else
      FindClose(hFind);
#endif
      if (!existe)
      {
        DirName = DirNameA.str();
        a_chercher = false;
        break;
      }
    }
    num += 1;
  }

  MESSAGE ("==> DirName = " << DirName);
  MESSAGE (". On retourne dans nomDirActuel = " << nomDirActuel);
  CHDIR(nomDirActuel.c_str());
  return CORBA::string_dup(DirName.c_str());
#else
	return "";
#endif
}
//=============================================================================
// Calcul d'une iteration : gestion du répertoire de calcul
//=============================================================================
char* HOMARD_Gen_i::ComputeDirManagement()
{
#ifndef DISABLE_HOMARD_ADAPT
  MESSAGE ("ComputeDirManagement : répertoires pour le calcul");

  //Si le sous-répertoire existe :
  //  CleanOption =  0 : on sort en erreur si le répertoire n'est pas vide
  //  CleanOption =  1 : on fait le menage du répertoire
  //  CleanOption = -1 : on ne fait rien
  int CleanOption = 0;

  // B.2. Le répertoire du cas
  const char* nomDirCase = myCase->GetDirName();
  MESSAGE (". nomDirCase = " << nomDirCase);

  // B.3. Le sous-répertoire de l'iteration a calculer, puis le répertoire complet a creer
  // B.3.1. Le nom du sous-répertoire
  const char* nomDirIt = myIteration1->GetDirNameLoc();

  // B.3.2. Le nom complet du sous-répertoire
  std::stringstream DirCompute;
  DirCompute << nomDirCase << "/" << nomDirIt;
  MESSAGE (". DirCompute = " << DirCompute.str());

  // B.3.3. Si le sous-répertoire n'existe pas, on le cree
  if (CHDIR(DirCompute.str().c_str()) != 0)
  {
#ifndef WIN32
    if (mkdir(DirCompute.str().c_str(), S_IRWXU|S_IRGRP|S_IXGRP) != 0)
#else
    if (_mkdir(DirCompute.str().c_str()) != 0)
#endif
    {
       // GERALD -- QMESSAGE BOX
       std::cerr << "Pb Creation du répertoire DirCompute = " << DirCompute.str() << std::endl;
       VERIFICATION("Pb a la creation du répertoire" == 0);
    }
  }
  else
  {
    //  Le répertoire existe
    //  On demande de faire le menage de son contenu :
    if (CleanOption == 1) {
      MESSAGE (". Menage du répertoire DirCompute = " << DirCompute.str());
      std::string commande = "rm -rf " + DirCompute.str()+"/*";
      int codret = system(commande.c_str());
      if (codret != 0)
      {
        // GERALD -- QMESSAGE BOX
        std::cerr << ". Menage du répertoire de calcul" << DirCompute.str() << std::endl;
        VERIFICATION("Pb au menage du répertoire de calcul" == 0);
      }
    }
    //  On n'a pas demande de faire le menage de son contenu : on sort en erreur :
    else {
      if (CleanOption == 0) {
#ifndef WIN32
        DIR *dp;
        struct dirent *dirp;
        dp  = opendir(DirCompute.str().c_str());
        bool result = true;
        while ((dirp = readdir(dp)) != NULL && result)
        {
          std::string file_name(dirp->d_name);
          result = file_name.empty() || file_name == "." || file_name == ".."; //if any file - break and return false
        }
        closedir(dp);
#else
       HANDLE hFind = INVALID_HANDLE_VALUE;
       WIN32_FIND_DATA ffd;
       hFind = FindFirstFile(DirCompute.str().c_str(), &ffd);
       bool result = true;
       if (INVALID_HANDLE_VALUE != hFind) {
         while (FindNextFile(hFind, &ffd) != 0) {
          if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
          std::string file_name(ffd.cFileName);
          result = file_name.empty() || file_name == "." || file_name == ".."; //if any file - break and return false
         }
       }
       FindClose(hFind);
#endif
        if (result == false)
        {
          SALOME::ExceptionStruct es;
          es.type = SALOME::BAD_PARAM;
          std::string text = "Directory : " + DirCompute.str() + " is not empty";
          es.text = CORBA::string_dup(text.c_str());
          throw SALOME::SALOME_Exception(es);
          VERIFICATION("Directory is not empty" == 0);
        }
      }
    }
  }

  return CORBA::string_dup(DirCompute.str().c_str());
#else
	return "";
#endif
}
//=============================================================================
// Calcul d'une iteration : gestion du répertoire de calcul de l'iteration parent
//=============================================================================
char* HOMARD_Gen_i::ComputeDirPaManagement()
{
  MESSAGE ("ComputeDirPaManagement : répertoires pour le calcul");
  // Le répertoire du cas
  const char* nomDirCase = myCase->GetDirName();
  MESSAGE (". nomDirCase = " << nomDirCase);

  // Le sous-répertoire de l'iteration precedente

  const char* nomDirItPa = myIteration0->GetDirNameLoc();
  std::stringstream DirComputePa;
  DirComputePa << nomDirCase << "/" << nomDirItPa;
  MESSAGE(". nomDirItPa = " << nomDirItPa);
  MESSAGE(". DirComputePa = " << DirComputePa.str());

  return CORBA::string_dup(DirComputePa.str().c_str());
}

//=============================================================================
// Calcul d'une iteration : ecriture des frontieres dans le fichier de configuration
// On ecrit dans l'ordre :
//    1. la definition des frontieres
//    2. les liens avec les groupes
//    3. un entier resumant le type de comportement pour les frontieres
//=============================================================================
int HOMARD_Gen_i::DriverTexteBoundary(SMESHHOMARDImpl::HomardDriver* myDriver)
{
  MESSAGE ("... DriverTexteBoundary");
  // 1. Recuperation des frontieres
  std::list<std::string>  ListeBoundaryTraitees;
  SMESHHOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  int numberOfitems = ListBoundaryGroupType->length();
  MESSAGE ("... number of string for Boundary+Group = " << numberOfitems);
  int BoundaryOption = 1;
  // 2. Parcours des frontieres pour ecrire leur description
  int NumBoundaryAnalytical = 0;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ("... BoundaryName = " << BoundaryName);
    // 2.1. La frontiere a-t-elle deja ete ecrite ?
    //      Cela arrive quand elle est liee a plusieurs groupes. Il ne faut l'ecrire que la premiere fois
    int A_faire = 1;
    std::list<std::string>::const_iterator it = ListeBoundaryTraitees.begin();
    while (it != ListeBoundaryTraitees.end())
    {
      MESSAGE ("..... BoundaryNameTraitee = " << *it);
      if (BoundaryName == *it) { A_faire = 0; }
      it++;
    }
    // 2.2. Ecriture de la frontiere
    if (A_faire == 1)
    {
      // 2.2.1. Caracteristiques de la frontiere
      SMESHHOMARD::HOMARD_Boundary_var myBoundary = _mesBoundarys[BoundaryName];
      ASSERT(!CORBA::is_nil(myBoundary));
      int BoundaryType = myBoundary->GetType();
      MESSAGE ("... BoundaryType = " << BoundaryType);
      // 2.2.2. Ecriture selon le type
      // 2.2.2.1. Cas d une frontiere CAO
      if (BoundaryType == -1)
      {
//         const char* CAOFile = myBoundary->GetDataFile();
//         MESSAGE (". CAOFile = " << CAOFile);
        if (BoundaryOption % 5 != 0) { BoundaryOption = BoundaryOption*5; }
      }
      // 2.2.2.2. Cas d une frontiere discrete
      else if (BoundaryType == 0)
      {
        const char* MeshName = myBoundary->GetMeshName();
        MESSAGE (". MeshName = " << MeshName);
        const char* MeshFile = myBoundary->GetDataFile();
        MESSAGE (". MeshFile = " << MeshFile);
        myDriver->TexteBoundaryDi(MeshName, MeshFile);
        if (BoundaryOption % 2 != 0) { BoundaryOption = BoundaryOption*2; }
      }
      // 2.2.2.3. Cas d une frontiere analytique
      else
      {
        NumBoundaryAnalytical++;
        SMESHHOMARD::double_array* coor = myBoundary->GetCoords();
        if (BoundaryType == 1) // Cas d un cylindre
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], 0.);
          if (BoundaryOption % 3 != 0) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 2) // Cas d une sphere
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], 0., 0., 0., 0.);
          if (BoundaryOption % 3 != 0) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 3) // Cas d un cone defini par un axe et un angle
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], 0.);
          if (BoundaryOption % 3 != 0) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 4) // Cas d un cone defini par les 2 rayons
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], (*coor)[7]);
          if (BoundaryOption % 3 != 0) { BoundaryOption = BoundaryOption*3; }
        }
        else if (BoundaryType == 5) // Cas d un tore
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], (*coor)[7]);
          if (BoundaryOption % 3 != 0) { BoundaryOption = BoundaryOption*3; }
        }
      }
      // 2.2.3. Memorisation du traitement
      ListeBoundaryTraitees.push_back(BoundaryName);
    }
  }
  // 3. Parcours des frontieres pour ecrire les liens avec les groupes
  NumBoundaryAnalytical = 0;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ("... BoundaryName = " << BoundaryName);
    SMESHHOMARD::HOMARD_Boundary_var myBoundary = _mesBoundarys[BoundaryName];
    ASSERT(!CORBA::is_nil(myBoundary));
    int BoundaryType = myBoundary->GetType();
    MESSAGE ("... BoundaryType = " << BoundaryType);
    // 3.1. Recuperation du nom du groupe
    std::string GroupName = std::string((*ListBoundaryGroupType)[NumBoundary+1]);
    MESSAGE ("... GroupName = " << GroupName);
    // 3.2. Cas d une frontiere CAO
    if (BoundaryType == -1)
    {
      if (GroupName.size() > 0) { myDriver->TexteBoundaryCAOGr (GroupName); }
    }
    // 3.3. Cas d une frontiere discrete
    else if (BoundaryType == 0)
    {
      if (GroupName.size() > 0) { myDriver->TexteBoundaryDiGr (GroupName); }
    }
    // 3.4. Cas d une frontiere analytique
    else
    {
      NumBoundaryAnalytical++;
      myDriver->TexteBoundaryAnGr (BoundaryName, NumBoundaryAnalytical, GroupName);
    }
  }
  // 4. Ecriture de l'option finale
  myDriver->TexteBoundaryOption(BoundaryOption);
//
  return BoundaryOption;
}
//===========================================================================
//===========================================================================

//===========================================================================
// Publication
//===========================================================================
void HOMARD_Gen_i::PublishResultInSmesh(const char* NomFich)
{
  // Prevent dump of CreateMeshesFromMED
  SMESH::TPythonDump pDump; // do not delete this line of code

  MESSAGE("PublishResultInSmesh " << NomFich);
  if (CORBA::is_nil(SMESH_Gen_i::GetSMESHGen()->getStudyServant())) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
  }

  // Le module SMESH est-il actif ?
  SALOMEDS::SObject_var aSmeshSO =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindComponent("SMESH");
  //
  // TODO?
  // Temporary suppressed depublication of mesh with the same name of file
  /*
  if (!CORBA::is_nil(aSmeshSO)) {
    // On verifie que le fichier n est pas deja publie
    SALOMEDS::ChildIterator_var aIter =
      SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewChildIterator(aSmeshSO);
    for (; aIter->More(); aIter->Next()) {
      SALOMEDS::SObject_var aSO = aIter->Value();
      SALOMEDS::GenericAttribute_var aGAttr;
      if (aSO->FindAttribute(aGAttr, "AttributeExternalFileDef")) {
        SALOMEDS::AttributeExternalFileDef_var anAttr =
          SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
        CORBA::String_var value = anAttr->Value();
        if (strcmp((const char*)value, NomFich) == 0) {
          MESSAGE ("PublishResultInSmesh : le fichier " << NomFich << " est deja publie.");
          // Pour un fichier importe, on ne republie pas
          // Option = 0 : fichier issu d'une importation
          // Option = 1 : fichier issu d'une execution HOMARD
          if (Option == 0) { return; }
          // Pour un fichier calcule, on commence par faire la depublication
          else {
            MESSAGE("PublishResultInSmesh : depublication");
            if (aSO->FindAttribute(aGAttr, "AttributeName")) {
              SALOMEDS::AttributeName_var anAttr2 = SALOMEDS::AttributeName::_narrow(aGAttr);
              CORBA::String_var value2 = anAttr2->Value();
              std::string MeshName = string(value2);
              MESSAGE("PublishResultInSmesh : depublication de " << MeshName);
              DeleteResultInSmesh(NomFich, MeshName);
            }
          }
        }
      }
    }
  }
  */

  // On enregistre le fichier
  MESSAGE("Enregistrement du fichier");
  //
  //SMESH::SMESH_Gen_var aSmeshEngine = this->retrieveSMESHInst();
  SMESH_Gen_i* aSmeshEngine = SMESH_Gen_i::GetSMESHGen();
  //
  //ASSERT(!CORBA::is_nil(aSmeshEngine));
  aSmeshEngine->UpdateStudy();
  SMESH::DriverMED_ReadStatus theStatus;

  // On met a jour les attributs AttributeExternalFileDef et AttributePixMap
  SMESH::mesh_array* mesMaillages = aSmeshEngine->CreateMeshesFromMED(NomFich, theStatus);
  if (CORBA::is_nil(aSmeshSO)) {
    aSmeshSO = SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindComponent("SMESH");
    if (CORBA::is_nil(aSmeshSO)) return;
  }

  for (int i = 0; i < (int)mesMaillages->length(); i++) {
    MESSAGE(". Mise a jour des attributs du maillage");
    SMESH::SMESH_Mesh_var monMaillage = (*mesMaillages)[i];
    SALOMEDS::SObject_var aSO = SALOMEDS::SObject::_narrow
      (SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindObjectIOR
       (SMESH_Gen_i::GetORB()->object_to_string(monMaillage)));
    SALOMEDS::StudyBuilder_var aStudyBuilder =
      SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewBuilder();
    SALOMEDS::GenericAttribute_var aGAttr =
      aStudyBuilder->FindOrCreateAttribute(aSO, "AttributeExternalFileDef");
    SALOMEDS::AttributeExternalFileDef_var anAttr =
      SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
    anAttr->SetValue(NomFich);
    SALOMEDS::GenericAttribute_var aPixMap =
      aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePixMap");
    SALOMEDS::AttributePixMap_var anAttr2 = SALOMEDS::AttributePixMap::_narrow(aPixMap);
    anAttr2->SetPixMap("mesh_tree_mesh.png");
  }
}

//=============================================================================
void HOMARD_Gen_i::DeleteResultInSmesh(std::string NomFich, std::string MeshName)
{
  MESSAGE ("DeleteResultInSmesh pour le maillage " << MeshName << " dans le fichier " << NomFich);
  if (CORBA::is_nil(SMESH_Gen_i::GetSMESHGen()->getStudyServant())) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
  }

  // Le module SMESH est-il actif ?
  SALOMEDS::SObject_var aSmeshSO =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->FindComponent("SMESH");
  //
  if (CORBA::is_nil(aSmeshSO)) return;
  // On verifie que le fichier est deja publie
  SALOMEDS::StudyBuilder_var myBuilder =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewBuilder();
  SALOMEDS::ChildIterator_var aIter =
    SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewChildIterator(aSmeshSO);
  for (; aIter->More(); aIter->Next()) {
    SALOMEDS::SObject_var  aSO = aIter->Value();
    SALOMEDS::GenericAttribute_var aGAttr;
    if (aSO->FindAttribute(aGAttr, "AttributeExternalFileDef")) {
      SALOMEDS::AttributeExternalFileDef_var anAttr =
        SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
      CORBA::String_var value = anAttr->Value();
      if (strcmp((const char*)value, NomFich.c_str()) == 0) {
        if (aSO->FindAttribute(aGAttr, "AttributeName")) {
          SALOMEDS::AttributeName_var anAttr2 = SALOMEDS::AttributeName::_narrow(aGAttr);
          CORBA::String_var value2 = anAttr2->Value();
          if (strcmp((const char*)value2, MeshName.c_str()) == 0) {
            myBuilder->RemoveObjectWithChildren(aSO);
          }
        }
      }
    }
  }
}

void HOMARD_Gen_i::PythonDump()
{
  MESSAGE ("Begin PythonDump");
  SMESH::TPythonDump pd;

  // SMESH_Homard
  pd << "import SMESHHOMARD\n";
  //pd << "smeshhomard = " << SMESH_Gen_i::GetSMESHGen() << ".CreateHOMARD_ADAPT()\n";
  pd << "smeshhomard = " << SMESH_Gen_i::GetSMESHGen() << ".Adaptation(\"Uniform\")\n";

  // Boundaries
  if (_mesBoundarys.size() > 0) MESSAGE(". Creation of the boundaries");
  std::map<std::string, SMESHHOMARD::HOMARD_Boundary_var>::const_iterator it_boundary;
  for (it_boundary  = _mesBoundarys.begin();
       it_boundary != _mesBoundarys.end(); ++it_boundary) {
    SMESHHOMARD::HOMARD_Boundary_var maBoundary = (*it_boundary).second;
    //MESSAGE ("PythonDump  of boundary " << (*it_boundary).first <<
    //         " : " << maBoundary->GetDumpPython());
    pd << maBoundary->GetDumpPython();
  }

  // Case
  ASSERT(!myCase->_is_nil());
  MESSAGE (". Creation of the case");

  if (_CaseOnMedFile) {
    pd << "smeshhomard.CreateCase(\"" << myIteration0->GetMeshName();
    pd << "\", \"" << myIteration0->GetMeshFile();
    pd << "\", \"" << myCase->GetDirName() << "\")\n";
  }
  else {
    pd << "smeshhomard.CreateCaseOnMesh(\"" << myIteration0->GetMeshName();
    pd << "\", " << _SmeshMesh;
    pd << ".GetMesh(), \"" << myCase->GetDirName() << "\")\n";
  }

  pd << myCase->GetDumpPython();

  // Preferences
  pd << "smeshhomard.SetConfType(" << _ConfType << ")\n";
  pd << "smeshhomard.SetKeepMedOUT(" << (_KeepMedOUT ? "True" : "False") << ")\n";
  pd << "smeshhomard.SetPublishMeshOUT(" << (_PublishMeshOUT ? "True" : "False") << ")\n";
  pd << "smeshhomard.SetMeshNameOUT(\"" << _MeshNameOUT << "\")\n";
  pd << "smeshhomard.SetMeshFileOUT(\"" << _MeshFileOUT << "\")\n";

  pd << "smeshhomard.SetKeepWorkingFiles(" << (_KeepWorkingFiles ? "True" : "False") << ")\n";
  pd << "smeshhomard.SetLogInFile(" << (_LogInFile ? "True" : "False") << ")\n";
  if (_LogInFile) pd << "smeshhomard.SetLogFile(\"" << _LogFile << "\")\n";
  pd << "smeshhomard.SetRemoveLogOnSuccess(" << (_RemoveLogOnSuccess ? "True" : "False") << ")\n";
  pd << "smeshhomard.SetVerboseLevel(" << _VerboseLevel << ")\n";

  // Compute
  pd << "smeshhomard.Compute()\n";
  MESSAGE ("End PythonDump");
}

void HOMARD_Gen_i::AddBoundary(const char* BoundaryName)
{
  MESSAGE("HOMARD_Gen_i::AddBoundary : BoundaryName = " << BoundaryName);
  if (myCase->_is_nil()) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "The input mesh must be defined before boundary addition";
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  myCase->AddBoundary(BoundaryName);
}

void HOMARD_Gen_i::AddBoundaryGroup(const char* BoundaryName, const char* Group)
{
  MESSAGE("HOMARD_Gen_i::AddBoundaryGroup : BoundaryName = " <<
          BoundaryName << ", Group = " << Group);
  if (myCase->_is_nil()) {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "The input mesh must be defined before boundary group addition";
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  myCase->AddBoundaryGroup(BoundaryName, Group);
}

//===============================================================================
// Preferences
//===============================================================================
void HOMARD_Gen_i::SetConfType(CORBA::Long theConfType)
{
  _ConfType = theConfType;
}
void HOMARD_Gen_i::SetKeepMedOUT(bool theKeepMedOUT)
{
  _KeepMedOUT = theKeepMedOUT;
}
void HOMARD_Gen_i::SetPublishMeshOUT(bool thePublishMeshOUT)
{
  _PublishMeshOUT = thePublishMeshOUT;
}
void HOMARD_Gen_i::SetKeepWorkingFiles(bool theKeepWorkingFiles)
{
  _KeepWorkingFiles = theKeepWorkingFiles;
}
void HOMARD_Gen_i::SetLogInFile(bool theLogInFile)
{
  _LogInFile = theLogInFile;
}
void HOMARD_Gen_i::SetRemoveLogOnSuccess(bool theRemoveLogOnSuccess)
{
  _RemoveLogOnSuccess = theRemoveLogOnSuccess;
}
void HOMARD_Gen_i::SetVerboseLevel(CORBA::Long theVerboseLevel)
{
  _VerboseLevel = theVerboseLevel;
}
void HOMARD_Gen_i::SetMeshNameOUT(const char* theMeshNameOUT)
{
  _MeshNameOUT = theMeshNameOUT;
}
void HOMARD_Gen_i::SetMeshFileOUT(const char* theMeshFileOUT)
{
  _MeshFileOUT = theMeshFileOUT;
}
void HOMARD_Gen_i::SetLogFile(const char* theLogFile)
{
  _LogFile = theLogFile;
}

// =======================================================================
std::set<std::string> GetListeGroupesInMedFile(const char * aFile)
{
  std::set<std::string> ListeGroupes;
  med_err erreur = 0;
  med_idt medIdt;
  while (erreur == 0) {
    //  Ouverture du fichier
    medIdt = MEDfileOpen(aFile,MED_ACC_RDONLY);
    if (medIdt < 0) {
      erreur = 1;
      break;
    }
    // Caracteristiques du maillage
    char meshname[MED_NAME_SIZE+1];
    med_int spacedim,meshdim;
    med_mesh_type meshtype;
    char descriptionription[MED_COMMENT_SIZE+1];
    char dtunit[MED_SNAME_SIZE+1];
    med_sorting_type sortingtype;
    med_int nstep;
    med_axis_type axistype;
    int naxis = MEDmeshnAxis(medIdt,1);
    char *axisname=new char[naxis*MED_SNAME_SIZE+1];
    char *axisunit=new char[naxis*MED_SNAME_SIZE+1];
    erreur = MEDmeshInfo(medIdt,
                            1,
                            meshname,
                            &spacedim,
                            &meshdim,
                            &meshtype,
                            descriptionription,
                            dtunit,
                            &sortingtype,
                            &nstep,
                            &axistype,
                            axisname,
                            axisunit);
    delete[] axisname;
    delete[] axisunit;
    if (erreur < 0) { break; }
    // Nombre de familles
    med_int nfam;
    nfam = MEDnFamily(medIdt,meshname);
    if (nfam < 0) {
      erreur = 2;
      break;
    }
    // Lecture des caracteristiques des familles
    for (int i=0;i<nfam;i++) {
      // Lecture du nombre de groupes
      med_int ngro = MEDnFamilyGroup(medIdt,meshname,i+1);
      if (ngro < 0) {
        erreur = 3;
        break;
      }
      // Lecture de la famille
      else if (ngro > 0) {
        char familyname[MED_NAME_SIZE+1];
        med_int numfam;
        char* gro = (char*) malloc(MED_LNAME_SIZE*ngro+1);
        erreur = MEDfamilyInfo(medIdt,
                               meshname,
                               i+1,
                               familyname,
                               &numfam,
                               gro);
        if (erreur < 0) {
          free(gro);
          break;
        }
        // Lecture des groupes pour une famille de mailles
        if (numfam < 0) {
          for (int j=0;j<ngro;j++) {
            char str2[MED_LNAME_SIZE+1];
            strncpy(str2,gro+j*MED_LNAME_SIZE,MED_LNAME_SIZE);
            str2[MED_LNAME_SIZE] = '\0';
            ListeGroupes.insert(std::string(str2));
          }
        }
        free(gro);
      }
    }
    break;
  }
  // Fermeture du fichier
  if (medIdt > 0) MEDfileClose(medIdt);

  return ListeGroupes;
}

// =======================================================================
// Le vecteur en retour contiendra les informations suivantes :
// en position 0 et 1 Xmin, Xmax et en position 2 Dx si < 0  2D
// en position 3 et 4 Ymin, Ymax et en position 5 Dy si < 0  2D
// en position 6 et 7 Zmin, Zmax et en position 8 Dz si < 0  2D
//  9 distance max dans le maillage
// =======================================================================
std::vector<double> GetBoundingBoxInMedFile(const char * aFile)
{
  std::vector<double> LesExtremes;
  med_err erreur = 0;
  med_idt medIdt;
  while (erreur == 0) {
    //  Ouverture du fichier
    medIdt = MEDfileOpen(aFile,MED_ACC_RDONLY);
    if (medIdt < 0) {
      erreur = 1;
      break;
    }
    //Nombre de maillage : on ne peut en lire qu'un seul
    med_int numberOfMeshes = MEDnMesh(medIdt);
    if (numberOfMeshes != 1) {
      erreur = 2;
      break;
    }
    // Caracteristiques du maillage
    char meshname[MED_NAME_SIZE+1];
    med_int spacedim,meshdim;
    med_mesh_type meshtype;
    char descriptionription[MED_COMMENT_SIZE+1];
    char dtunit[MED_SNAME_SIZE+1];
    med_sorting_type sortingtype;
    med_int nstep;
    med_axis_type axistype;
    int naxis = MEDmeshnAxis(medIdt,1);
    char *axisname=new char[naxis*MED_SNAME_SIZE+1];
    char *axisunit=new char[naxis*MED_SNAME_SIZE+1];
    erreur = MEDmeshInfo(medIdt,
                            1,
                            meshname,
                            &spacedim,
                            &meshdim,
                            &meshtype,
                            descriptionription,
                            dtunit,
                            &sortingtype,
                            &nstep,
                            &axistype,
                            axisname,
                            axisunit);
    delete[] axisname;
    delete[] axisunit;
    if (erreur < 0) { break; }

    // Nombre de noeuds
    med_bool chgt,trsf;
    med_int nnoe  = MEDmeshnEntity(medIdt,
                              meshname,
                              MED_NO_DT,
                              MED_NO_IT,
                              MED_NODE,
                              MED_NO_GEOTYPE,
                              MED_COORDINATE,
                              MED_NO_CMODE,
                              &chgt,
                              &trsf);
    if (nnoe < 0) {
      erreur =  4;
      break;
    }

    // Les coordonnees
    med_float* coo = (med_float*) malloc(sizeof(med_float)*nnoe*spacedim);

    erreur = MEDmeshNodeCoordinateRd(medIdt,
                                      meshname,
                                      MED_NO_DT,
                                      MED_NO_IT,
                                      MED_NO_INTERLACE,
                                      coo);
    if (erreur < 0) {
      free(coo);
      break;
    }

    // Calcul des extremes
    med_float xmin,xmax,ymin,ymax,zmin,zmax;

    xmin = coo[0];
    xmax = coo[0];
    for (int i = 1; i < nnoe; i++) {
      xmin = std::min(xmin, coo[i]);
      xmax = std::max(xmax, coo[i]);
    }
    //
    if (spacedim > 1) {
      ymin = coo[nnoe];
      ymax = coo[nnoe];
      for (int i = nnoe + 1; i < 2*nnoe; i++) {
        ymin = std::min(ymin,coo[i]);
        ymax = std::max(ymax,coo[i]);
      }
    }
    else {
      ymin = 0;
      ymax = 0;
      zmin = 0;
      zmax = 0;
    }
    //
    if (spacedim > 2) {
      zmin = coo[2*nnoe];
      zmax = coo[2*nnoe];
      for (int i = 2*nnoe + 1; i < 3*nnoe; i++) {
        zmin = std::min(zmin,coo[i]);
        zmax = std::max(zmax,coo[i]);
      }
    }
    else {
      zmin = 0;
      zmax = 0;
    }

    MESSAGE("_______________________________________");
    MESSAGE("xmin : " << xmin << " xmax : " << xmax);
    MESSAGE("ymin : " << ymin << " ymax : " << ymax);
    MESSAGE("zmin : " << zmin << " zmax : " << zmax);
    MESSAGE("_______________________________________");
    double epsilon = 1.e-6;
    LesExtremes.push_back(xmin);
    LesExtremes.push_back(xmax);
    LesExtremes.push_back(0);
    LesExtremes.push_back(ymin);
    LesExtremes.push_back(ymax);
    LesExtremes.push_back(0);
    LesExtremes.push_back(zmin);
    LesExtremes.push_back(zmax);
    LesExtremes.push_back(0);

    double max1 = std::max (LesExtremes[1] - LesExtremes[0], LesExtremes[4] - LesExtremes[3]);
    double max2 = std::max (max1 , LesExtremes[7] - LesExtremes[6]);
    LesExtremes.push_back(max2);

    // LesExtremes[0] = Xmini du maillage
    // LesExtremes[1] = Xmaxi du maillage
    // LesExtremes[2] = increment de progression en X
    // LesExtremes[3,4,5] : idem pour Y
    // LesExtremes[6,7,8] : idem pour Z
    // LesExtremes[9] = ecart maximal entre coordonnees
    // On fait un traitement pour dans le cas d'une coordonnee constante
    // inhiber ce cas en mettant un increment negatif
    //
    double diff = LesExtremes[1] - LesExtremes[0];
    if (fabs(diff) > epsilon*max2) { LesExtremes[2] = diff/100.; }
    else                           { LesExtremes[2] = -1.; }

    diff = LesExtremes[4] - LesExtremes[3];
    if (fabs(diff) > epsilon*max2) { LesExtremes[5]=diff/100.; }
    else                           { LesExtremes[5] = -1.; }

    diff = LesExtremes[7] - LesExtremes[6];
    if (fabs(diff) > epsilon*max2) { LesExtremes[8]=diff/100.; }
    else                           { LesExtremes[8] = -1.;  }

    MESSAGE ("_______________________________________");
    MESSAGE ("xmin : " << LesExtremes[0] << " xmax : " << LesExtremes[1] << " xincr : " << LesExtremes[2]);
    MESSAGE ("ymin : " << LesExtremes[3] << " ymax : " << LesExtremes[4] << " yincr : " << LesExtremes[5]);
    MESSAGE ("zmin : " << LesExtremes[6] << " zmax : " << LesExtremes[7] << " zincr : " << LesExtremes[8]);
    MESSAGE ("dmax : " << LesExtremes[9]);
    MESSAGE ("_______________________________________");

    free(coo);
    break;
  }
  // Fermeture du fichier
  if (medIdt > 0) MEDfileClose(medIdt);

  return  LesExtremes;
}

}; // namespace SMESHHOMARD_I
