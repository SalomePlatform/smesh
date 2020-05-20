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

#include "HOMARD_Gen_i.hxx"
#include "HOMARD_Cas_i.hxx"
#include "HOMARD_Hypothesis_i.hxx"
#include "HOMARD_Iteration_i.hxx"
#include "HOMARD_Boundary_i.hxx"
#include "HOMARD_Zone_i.hxx"
#include "HOMARD_YACS_i.hxx"
#include "HomardDriver.hxx"
#include "HOMARD_DriverTools.hxx"
#include "HomardMedCommun.h"
#include "YACSDriver.hxx"
#include "HOMARD.hxx"

#include "FrontTrack.hxx"

#include "HOMARD_version.h"

#include "utilities.h"
#include "Basics_Utils.hxx"
#include "Basics_DirUtils.hxx"
#include "Utils_SINGLETON.hxx"
#include "Utils_CorbaException.hxx"
#include "SALOMEDS_Tool.hxx"
#include "SALOME_LifeCycleCORBA.hxx"
#include "SALOMEconfig.h"
#include <SMESH_Gen_i.hxx>
#include CORBA_CLIENT_HEADER(SALOME_ModuleCatalog)
#include CORBA_CLIENT_HEADER(SMESH_Gen)

#include <cmath>
#include <stdlib.h>
#include <sys/stat.h>
#ifndef WIN32
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

#ifdef WIN32
#include <direct.h>
#endif

using  namespace std;

//=============================================================================
//functions
//=============================================================================
std::string RemoveTabulation( std::string theScript )
{
  std::string::size_type aPos = 0;
  while( aPos < theScript.length() )
  {
    aPos = theScript.find( "\n\t", aPos );
    if( aPos == std::string::npos )
      break;
    theScript.replace( aPos, 2, "\n" );
    aPos++;
  }
  return theScript;
}
//=============================================================================
/*!
 *  standard constructor
 */
//=============================================================================
HOMARD_Gen_i::HOMARD_Gen_i( CORBA::ORB_ptr orb,
                            PortableServer::POA_ptr poa,
                            PortableServer::ObjectId * contId,
                            const char *instanceName,
                            const char *interfaceName) :
Engines_Component_i(orb, poa, contId, instanceName, interfaceName)
{
  MESSAGE("constructor");
  _thisObj = this;
  _id = _poa->activate_object(_thisObj);

  myHomard = new ::HOMARD_Gen();
  _NS = SINGLETON_<SALOME_NamingService>::Instance();
  ASSERT(SINGLETON_<SALOME_NamingService>::IsAlreadyExisting());
  _NS->init_orb(_orb);

  myStudy = SALOMEDS::Study::_duplicate( SMESH_Gen_i::getStudyServant() );

  _tag_gene = 0 ;
  _tag_boun = 0 ;
  _tag_hypo = 0 ;
  _tag_yacs = 0 ;
  _tag_zone = 0 ;

  SetPreferences( ) ;
}
//=================================
/*!
 *  standard destructor
 */
//================================
HOMARD_Gen_i::~HOMARD_Gen_i()
{
}
//=============================================================================
//=============================================================================
// Utilitaires pour l'étude
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::UpdateStudy()
{
  ASSERT(!CORBA::is_nil(myStudy));
  SALOMEDS::StudyBuilder_var myBuilder = myStudy->NewBuilder();

  // Create SComponent labelled 'homard' if it doesn't already exit
  SALOMEDS::SComponent_var homardFather = myStudy->FindComponent(ComponentDataType());
  if (CORBA::is_nil(homardFather))
  {
    myBuilder->NewCommand();
    MESSAGE("Add Component HOMARD");

    bool aLocked = myStudy->GetProperties()->IsLocked();
    if (aLocked) myStudy->GetProperties()->SetLocked(false);

    homardFather = myBuilder->NewComponent(ComponentDataType());
    SALOMEDS::GenericAttribute_var anAttr = myBuilder->FindOrCreateAttribute(homardFather,"AttributeName");
    SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(anAttr);
    CORBA::Object_var objVarN = _NS->Resolve("/Kernel/ModulCatalog");
    SALOME_ModuleCatalog::ModuleCatalog_var Catalogue =
                SALOME_ModuleCatalog::ModuleCatalog::_narrow(objVarN);
    SALOME_ModuleCatalog::Acomponent_var Comp = Catalogue->GetComponent(ComponentDataType());
    if (!Comp->_is_nil())
    {
      aName->SetValue(ComponentDataType());
    }

    anAttr = myBuilder->FindOrCreateAttribute(homardFather,"AttributePixMap");
    SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap("HOMARD_2.png");
    myBuilder->DefineComponentInstance(homardFather, HOMARD_Gen::_this());

    if (aLocked) myStudy->GetProperties()->SetLocked(true);
    myBuilder->CommitCommand();
  }
}

//=============================================================================
//=============================================================================
// Utilitaires pour l'iteration
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::SetEtatIter(const char* nomIter, const CORBA::Long Etat)
//=====================================================================================
{
  MESSAGE( "SetEtatIter : affectation de l'etat " << Etat << " a l'iteration " << nomIter );
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

  myIteration->SetState(Etat);

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  SALOMEDS::SObject_var aIterSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));

  std::string icone ;
  if ( Etat <= 0 )
    icone = "iter0.png" ;
  else if ( Etat == 2 )
    icone = "iter_calculee.png" ;
  else
    icone = "iter_non_calculee.png" ;
  PublishInStudyAttr(aStudyBuilder, aIterSO, NULL , NULL, icone.c_str(), NULL) ;

  aStudyBuilder->CommitCommand();

}
//=============================================================================
//=============================================================================
//
//=============================================================================
//=============================================================================
// Destruction des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteBoundary(const char* BoundaryName)
{
  MESSAGE ( "DeleteBoundary : BoundaryName = " << BoundaryName );
  HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  if (CORBA::is_nil(myBoundary))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid boundary";
    throw SALOME::SALOME_Exception(es);
    return 1 ;
  };

// On verifie que la frontiere n'est plus utilisee
  HOMARD::listeCases* maListe = GetAllCasesName();
  int numberOfCases = maListe->length();
  MESSAGE ( ".. Nombre de cas = " << numberOfCases );
  std::string CaseName ;
  HOMARD::ListBoundaryGroupType* ListBoundaryGroupType ;
  int numberOfitems ;
  HOMARD::HOMARD_Cas_var myCase ;
  for (int NumeCas = 0; NumeCas< numberOfCases; NumeCas++)
  {
    CaseName = std::string((*maListe)[NumeCas]);
    MESSAGE ( "... Examen du cas = " << CaseName.c_str() );
    myCase = myStudyContext._mesCas[CaseName];
    ASSERT(!CORBA::is_nil(myCase));
    ListBoundaryGroupType = myCase->GetBoundaryGroup();
    numberOfitems = ListBoundaryGroupType->length();
    MESSAGE ( "... number of string for Boundary+Group = " << numberOfitems);
    for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
    {
      if ( std::string((*ListBoundaryGroupType)[NumBoundary]) == BoundaryName )
      {
        SALOME::ExceptionStruct es;
        es.type = SALOME::BAD_PARAM;
        es.text = "This boundary is used in a case and cannot be deleted.";
        throw SALOME::SALOME_Exception(es);
        return 2 ;
      };
    };
  }

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesBoundarys.erase(BoundaryName);
  SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(BoundaryName, ComponentDataType());
  SALOMEDS::SObject_var aSO =listSO[0];
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  myStudy->NewBuilder()->RemoveObjectWithChildren(aSO);

  return 0 ;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteCase(const char* nomCas, CORBA::Long Option)
{
  // Pour detruire un cas
  MESSAGE ( "DeleteCase : nomCas = " << nomCas << ", avec option = " << Option );
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case context";
    throw SALOME::SALOME_Exception(es);
    return 1;
  };
  // On commence par detruire toutes les iterations en partant de l'initiale et y compris elle
  CORBA::String_var nomIter = myCase->GetIter0Name();
  CORBA::Long Option1 = 0 ;
  if ( DeleteIterationOption(nomIter, Option1, Option) != 0 )
  {
    return 2;
  };

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesCas.erase(nomCas);
  SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(nomCas, ComponentDataType());
  SALOMEDS::SObject_var aSO =listSO[0];
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  myStudy->NewBuilder()->RemoveObjectWithChildren(aSO);

  return 0 ;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteHypo(const char* nomHypo)
{
  MESSAGE ( "DeleteHypo : nomHypo = " << nomHypo );
  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  if (CORBA::is_nil(myHypo))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid hypothesis";
    throw SALOME::SALOME_Exception(es);
    return 1 ;
  };

// On verifie que l'hypothese n'est plus utilisee
  HOMARD::listeIters* maListeIter = myHypo->GetIterations();
  int numberOfIter = maListeIter->length();
  if ( numberOfIter > 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This hypothesis is used in an iteration and cannot be deleted.";
    throw SALOME::SALOME_Exception(es);
    return 2 ;
  };

  // suppression du lien avec les zones eventuelles
  HOMARD::listeZonesHypo* maListe = myHypo->GetZones();
  int numberOfZones = maListe->length();
  MESSAGE ( ".. Nombre de zones = " << numberOfZones );
  for (int NumeZone = 0; NumeZone< numberOfZones; NumeZone++)
  {
    std::string ZoneName = std::string((*maListe)[NumeZone]);
    MESSAGE ( ".. suppression du lien avec la zone = " << ZoneName.c_str() );
    DissociateHypoZone(nomHypo, ZoneName.c_str()) ;
    NumeZone += 1 ;
  }

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesHypotheses.erase(nomHypo);
  SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(nomHypo, ComponentDataType());
  SALOMEDS::SObject_var aSO =listSO[0];
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  myStudy->NewBuilder()->RemoveObjectWithChildren(aSO);

  return 0 ;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteIteration(const char* nomIter, CORBA::Long Option)
{
  //  Option = 0 : On ne supprime pas le fichier du maillage associe
  //  Option = 1 : On supprime le fichier du maillage associe
  // Pour detruire une iteration courante
  MESSAGE ( "DeleteIteration : nomIter = " << nomIter << ", avec option = " << Option );
  CORBA::Long Option1 = 1 ;
  return DeleteIterationOption(nomIter, Option1, Option);
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteIterationOption(const char* nomIter, CORBA::Long Option1, CORBA::Long Option2)
{
  //  Option1 = 0 : On autorise la destruction de l'iteration 0
  //  Option1 = 1 : On interdit la destruction de l'iteration 0

  //  Option2 = 0 : On ne supprime pas le fichier du maillage associe
  //  Option2 = 1 : On supprime le fichier du maillage associe
  MESSAGE ( "DeleteIterationOption : nomIter = " << nomIter << ", avec options = " << Option1<< ", " << Option2 );
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iteration";
    throw SALOME::SALOME_Exception(es);
    return 1 ;
  };

  int numero = myIteration->GetNumber();
  MESSAGE ( "DeleteIterationOption : numero = " << numero );
  if ( numero == 0 && Option1 == 1 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration cannot be deleted.";
    throw SALOME::SALOME_Exception(es);
    return 2 ;
  };

  // On detruit recursivement toutes les filles
  HOMARD::listeIterFilles* maListe = myIteration->GetIterations();
  int numberOfIter = maListe->length();
  for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++)
  {
    std::string nomIterFille = std::string((*maListe)[NumeIter]);
    MESSAGE ( ".. appel recursif de DeleteIterationOption pour nomIter = " << nomIterFille.c_str() );
    DeleteIterationOption(nomIterFille.c_str(), Option1, Option2);
  }

  // On arrive ici pour une iteration sans fille
  MESSAGE ( "Destruction effective de " << nomIter );
  // On commence par invalider l'iteration pour faire le menage des dependances
  // et eventuellement du maillage associe
  int option ;
  if ( numero == 0 ) { option = 0 ; }
  else               { option = Option2 ; }
  InvalideIterOption(nomIter, option) ;

  // Retrait dans la descendance de l'iteration parent
  if ( numero > 0 )
  {
    std::string nomIterationParent = myIteration->GetIterParentName();
    MESSAGE ( "Retrait dans la descendance de nomIterationParent " << nomIterationParent );
    HOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterationParent];
    if (CORBA::is_nil(myIterationParent))
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
      return 3 ;
    };
    myIterationParent->UnLinkNextIteration(nomIter);
  }

  // suppression du lien avec l'hypothese
  if ( numero > 0 )
  {
    std::string nomHypo = myIteration->GetHypoName();
    HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
    ASSERT(!CORBA::is_nil(myHypo));
    myHypo->UnLinkIteration(nomIter);
  }

  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesIterations.erase(nomIter);
  SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(nomIter, ComponentDataType());
  SALOMEDS::SObject_var aSO =listSO[0];
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  myStudy->NewBuilder()->RemoveObjectWithChildren(aSO);
  // on peut aussi faire RemoveObject
//   MESSAGE ( "Au final" );
//   HOMARD::listeIterations* Liste = GetAllIterationsName() ;
//   numberOfIter = Liste->length();
//   for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++)
//   {
//       std::string nomIterFille = std::string((*Liste)[NumeIter]);
//       MESSAGE ( ".. nomIter = " << nomIterFille.c_str() );
//   }

  return 0 ;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteYACS(const char* nomYACS, CORBA::Long Option)
{
  //  Option = 0 : On ne supprime pas le fichier du schema associe
  //  Option = 1 : On supprime le fichier du schema associe
  MESSAGE ( "DeleteYACS : nomYACS = " << nomYACS << ", avec option = " << Option );
  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[nomYACS];
  if (CORBA::is_nil(myYACS))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid schema YACS";
    throw SALOME::SALOME_Exception(es);
    return 1 ;
  };
  // Suppression eventuelle du fichier XML
  if ( Option == 1 )
  {
    std::string nomFichier = myYACS->GetXMLFile();
    std::string commande = "rm -rf " + nomFichier ;
    MESSAGE ( "commande = " << commande );
    if ((system(commande.c_str())) != 0)
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "The xml file for the schema YACS cannot be removed." ;
      throw SALOME::SALOME_Exception(es);
      return 2 ;
    }
  }
  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesYACSs.erase(nomYACS);
  SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(nomYACS, ComponentDataType());
  SALOMEDS::SObject_var aSO =listSO[0];
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  myStudy->NewBuilder()->RemoveObjectWithChildren(aSO);

  return 0 ;
}
//=============================================================================
CORBA::Long HOMARD_Gen_i::DeleteZone(const char* nomZone)
{
  MESSAGE ( "DeleteZone : nomZone = " << nomZone );
  HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[nomZone];
  if (CORBA::is_nil(myZone))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid zone";
    throw SALOME::SALOME_Exception(es);
    return 1 ;
  };

// On verifie que la zone n'est plus utilisee
  HOMARD::listeHypo* maListe = myZone->GetHypo();
  int numberOfHypo = maListe->length();
  MESSAGE ( ".. Nombre d'hypotheses = " << numberOfHypo );
  if ( numberOfHypo > 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This zone is used in a hypothesis and cannot be deleted.";
    throw SALOME::SALOME_Exception(es);
    return 2 ;
  };
//
  // comme on a un _var comme pointeur CORBA, on ne se preoccupe pas du delete
  myStudyContext._mesZones.erase(nomZone);
  SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(nomZone, ComponentDataType());
  SALOMEDS::SObject_var aSO =listSO[0];
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  myStudy->NewBuilder()->RemoveObjectWithChildren(aSO);

  return 0 ;
}
//=============================================================================
//=============================================================================
//
//=============================================================================
//=============================================================================
// Invalidation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::InvalideBoundary(const char* BoundaryName)
{
  MESSAGE( "InvalideBoundary : BoundaryName = " << BoundaryName  );
  HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  if (CORBA::is_nil(myBoundary))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid boundary";
    throw SALOME::SALOME_Exception(es);
    return ;
  }
  else
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "No change is allowed in a boundary. Ask for evolution.";
    throw SALOME::SALOME_Exception(es);
    return ;
  };
}
//=============================================================================
void HOMARD_Gen_i::InvalideHypo(const char* nomHypo)
{
  MESSAGE( "InvalideHypo : nomHypo    = " << nomHypo  );
  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  if (CORBA::is_nil(myHypo))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid hypothesis";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

  HOMARD::listeIters* maListe = myHypo->GetIterations();
  int numberOfIter = maListe->length();
  for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++)
  {
      std::string nomIter = std::string((*maListe)[NumeIter]);
      MESSAGE( ".. nomIter = " << nomIter );
      InvalideIter(nomIter.c_str());
  }
}
//=============================================================================
void HOMARD_Gen_i::InvalideIter(const char* nomIter)
{
  MESSAGE("InvalideIter : nomIter = " << nomIter);
  // Pour invalider totalement une iteration courante
  CORBA::Long Option = 1 ;
  return InvalideIterOption(nomIter, Option);
}
//=============================================================================
void HOMARD_Gen_i::InvalideIterOption(const char* nomIter, CORBA::Long Option)
{
  //  Option = 0 : On ne supprime pas le fichier du maillage associe
  //  Option = 1 : On supprime le fichier du maillage associe
  MESSAGE ( "InvalideIterOption : nomIter = " << nomIter << ", avec option = " << Option );
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

  HOMARD::listeIterFilles* maListe = myIteration->GetIterations();
  int numberOfIter = maListe->length();
  for (int NumeIter = 0; NumeIter< numberOfIter; NumeIter++)
  {
      std::string nomIterFille = std::string((*maListe)[NumeIter]);
      MESSAGE ( ".. appel recursif de InvalideIter pour nomIter = " << nomIterFille.c_str() );
      InvalideIter(nomIterFille.c_str());
  }

  // On arrive ici pour une iteration sans fille
  MESSAGE ( "Invalidation effective de " << nomIter );
  SALOMEDS::SObject_var aIterSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));
  SALOMEDS::ChildIterator_var  aIter = myStudy->NewChildIterator(aIterSO);
  for (; aIter->More(); aIter->Next())
  {
      SALOMEDS::SObject_var so = aIter->Value();
      SALOMEDS::GenericAttribute_var anAttr;
      if (!so->FindAttribute(anAttr, "AttributeComment")) continue;
      SALOMEDS::AttributeComment_var aCommentAttr = SALOMEDS::AttributeComment::_narrow(anAttr);
      std::string value (aCommentAttr->Value());
      if(value == std::string("IterationHomard")) continue;
      if(value == std::string("HypoHomard")) continue;
      SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
      aStudyBuilder->RemoveObject(so);
  }

  int etat = myIteration->GetState();
  if ( etat > 0 )
  {
    SetEtatIter(nomIter,1);
    const char * nomCas = myIteration->GetCaseName();
    HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
    if (CORBA::is_nil(myCase))
    {
        SALOME::ExceptionStruct es;
        es.type = SALOME::BAD_PARAM;
        es.text = "Invalid case context";
        throw SALOME::SALOME_Exception(es);
        return ;
    };
    std::string nomDir     = myIteration->GetDirName();
    std::string nomFichier = myIteration->GetMeshFile();
    std::string commande = "rm -rf " + std::string(nomDir);
    if ( Option == 1 ) { commande = commande + ";rm -rf " + std::string(nomFichier) ; }
    MESSAGE ( "commande = " << commande );
    if ((system(commande.c_str())) != 0)
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "The directory for the calculation cannot be cleared." ;
      throw SALOME::SALOME_Exception(es);
      return ;
    }
  // Suppression du maillage publie dans SMESH
    std::string MeshName = myIteration->GetMeshName() ;
    DeleteResultInSmesh(nomFichier, MeshName) ;
  };

}
//=============================================================================
void HOMARD_Gen_i::InvalideIterInfo(const char* nomIter)
{
  MESSAGE("InvalideIterInfo : nomIter = " << nomIter);
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iteration";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

  SALOMEDS::SObject_var aIterSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));
  SALOMEDS::ChildIterator_var  aIter = myStudy->NewChildIterator(aIterSO);
  for (; aIter->More(); aIter->Next())
  {
      SALOMEDS::SObject_var so = aIter->Value();
      SALOMEDS::GenericAttribute_var anAttr;
      if (!so->FindAttribute(anAttr, "AttributeComment")) continue;
      SALOMEDS::AttributeComment_var aCommentAttr = SALOMEDS::AttributeComment::_narrow(anAttr);
      std::string value (aCommentAttr->Value());
/*      MESSAGE("... value = " << value);*/
      if( (value == std::string("logInfo")) || ( value == std::string("SummaryInfo")) )
      {
        SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
        aStudyBuilder->RemoveObject(so);
      }
  }

  const char * nomCas = myIteration->GetCaseName();
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid case context";
      throw SALOME::SALOME_Exception(es);
      return ;
  };
  const char* nomDir   = myIteration->GetDirName();
  std::string commande = "rm -f " + std::string(nomDir) + "/info* " ;
  commande += std::string(nomDir) + "/Liste.*info" ;
/*  MESSAGE ( "commande = " << commande );*/
  if ((system(commande.c_str())) != 0)
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory for the calculation cannot be cleared." ;
    throw SALOME::SALOME_Exception(es);
    return ;
  }
}
//=============================================================================
void HOMARD_Gen_i::InvalideYACS(const char* YACSName)
{
  MESSAGE( "InvalideYACS : YACSName    = " << YACSName );
  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[YACSName];
  if (CORBA::is_nil(myYACS))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid schema YACS";
      throw SALOME::SALOME_Exception(es);
      return ;
  };
  //
  SALOMEDS::SObject_var aYACSSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myYACS)));
  SALOMEDS::ChildIterator_var  aYACS = myStudy->NewChildIterator(aYACSSO);
  for (; aYACS->More(); aYACS->Next())
  {
    SALOMEDS::SObject_var so = aYACS->Value();
    SALOMEDS::GenericAttribute_var anAttr;
    if (!so->FindAttribute(anAttr, "AttributeComment")) continue;
    SALOMEDS::AttributeComment_var aCommentAttr = SALOMEDS::AttributeComment::_narrow(anAttr);
    std::string value (aCommentAttr->Value());
    if( value == std::string("xml") )
    {
      SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
      aStudyBuilder->RemoveObject(so);
    }
  }
  std::string nomFichier = myYACS->GetXMLFile();
  std::string commande = "rm -rf " + std::string(nomFichier) ;
  MESSAGE ( "commande = " << commande );
  if ((system(commande.c_str())) != 0)
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The XML file for the schema YACS cannot be removed." ;
    throw SALOME::SALOME_Exception(es);
    return ;
  }
}
//=============================================================================
void HOMARD_Gen_i::InvalideZone(const char* ZoneName)
{
  MESSAGE( "InvalideZone : ZoneName    = " << ZoneName );
  HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[ZoneName];
  if (CORBA::is_nil(myZone))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid zone";
      throw SALOME::SALOME_Exception(es);
      return ;
  };
  HOMARD::listeHypo* maListe = myZone->GetHypo();
  int numberOfHypo = maListe->length();
  MESSAGE( ".. numberOfHypo = " << numberOfHypo );
  for (int NumeHypo = 0; NumeHypo< numberOfHypo; NumeHypo++)
  {
      std::string nomHypo = std::string((*maListe)[NumeHypo]);
      MESSAGE( ".. nomHypo = " << nomHypo );
      InvalideHypo(nomHypo.c_str());
  }
}
//=============================================================================
//=============================================================================
//
//=============================================================================
//=============================================================================
// Association de lien entre des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::AssociateCaseIter(const char* nomCas, const char* nomIter, const char* labelIter)
{
  MESSAGE( "AssociateCaseIter : " << nomCas << ", " << nomIter << ", "  << labelIter );

  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case";
    throw SALOME::SALOME_Exception(es);
    return ;
  };

  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  if (CORBA::is_nil(myIteration))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid iteration";
    throw SALOME::SALOME_Exception(es);
    return ;
  };

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  SALOMEDS::SObject_var aCasSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myCase)));
  if (CORBA::is_nil(aCasSO))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case";
    throw SALOME::SALOME_Exception(es);
    return ;
  };

  aStudyBuilder->NewCommand();
  SALOMEDS::SObject_var newStudyIter = aStudyBuilder->NewObject(aCasSO);
  PublishInStudyAttr(aStudyBuilder, newStudyIter, nomIter , labelIter,
                     "iter_non_calculee.png", _orb->object_to_string(myIteration)) ;
  aStudyBuilder->CommitCommand();

  myCase->AddIteration(nomIter);
  myIteration->SetCaseName(nomCas);
}
//=====================================================================================
void HOMARD_Gen_i::AssociateHypoZone(const char* nomHypothesis, const char* ZoneName, CORBA::Long TypeUse)
{
  MESSAGE ( "AssociateHypoZone : nomHypo = " << nomHypothesis << ", ZoneName= " << ZoneName << ", TypeUse = " << TypeUse);

  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypothesis];
  ASSERT(!CORBA::is_nil(myHypo));
  SALOMEDS::SObject_var aHypoSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myHypo)));
  ASSERT(!CORBA::is_nil(aHypoSO));

  HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[ZoneName];
  ASSERT(!CORBA::is_nil(myZone));
  SALOMEDS::SObject_var aZoneSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myZone)));
  ASSERT(!CORBA::is_nil(aZoneSO));

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();

  aStudyBuilder->NewCommand();

  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aHypoSO);
  aStudyBuilder->Addreference(aSubSO, aZoneSO);

  aStudyBuilder->CommitCommand();

  myZone->AddHypo(nomHypothesis);
  myHypo->AddZone0(ZoneName, TypeUse);
};
//=============================================================================
void HOMARD_Gen_i::AssociateIterHypo(const char* nomIter, const char* nomHypo)
{
  MESSAGE("AssociateIterHypo : nomHypo = " << nomHypo << " nomIter = " << nomIter);

  // Verification de l'existence de l'hypothese
  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  ASSERT(!CORBA::is_nil(myHypo));
  SALOMEDS::SObject_var aHypoSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myHypo)));
  ASSERT(!CORBA::is_nil(aHypoSO));

  // Verification de l'existence de l'iteration
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[nomIter];
  ASSERT(!CORBA::is_nil(myIteration));
  SALOMEDS::SObject_var aIterSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));
  ASSERT(!CORBA::is_nil(aIterSO));

  // Gestion de l'arbre d'etudes
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  aStudyBuilder->NewCommand();
  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aIterSO);
  aStudyBuilder->Addreference(aSubSO, aHypoSO);
  aStudyBuilder->CommitCommand();

  // Liens reciproques
  myIteration->SetHypoName(nomHypo);
  myHypo->LinkIteration(nomIter);

  // On stocke les noms des champ a interpoler pour le futur controle de la donnée des pas de temps
  myIteration->SupprFieldInterps() ;
  HOMARD::listeFieldInterpsHypo* ListField = myHypo->GetFieldInterps();
  int numberOfFieldsx2 = ListField->length();
  for (int iaux = 0; iaux< numberOfFieldsx2; iaux++)
  {
    std::string FieldName = std::string((*ListField)[iaux]) ;
    myIteration->SetFieldInterp(FieldName.c_str()) ;
    iaux++ ;
  }
};
//=============================================================================
//=============================================================================
//
//=============================================================================
//=============================================================================
// Dissociation de lien entre des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::DissociateHypoZone(const char* nomHypothesis, const char* ZoneName)
{
  MESSAGE ( "DissociateHypoZone : ZoneName= " << ZoneName << ", nomHypo = " << nomHypothesis);

  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypothesis];
  ASSERT(!CORBA::is_nil(myHypo));
  SALOMEDS::SObject_var aHypoSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myHypo)));
  ASSERT(!CORBA::is_nil(aHypoSO));

  HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[ZoneName];
  ASSERT(!CORBA::is_nil(myZone));
  SALOMEDS::SObject_var aZoneSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myZone)));
  ASSERT(!CORBA::is_nil(aZoneSO));

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();

  SALOMEDS::ChildIterator_var it = myStudy->NewChildIterator(aHypoSO);
  for (; it->More(); it->Next())
  {
    SALOMEDS::SObject_var aHypObj = it->Value();
    SALOMEDS::SObject_var ptrObj;
    if (aHypObj->ReferencedObject(ptrObj))
    {
      if (std::string(ptrObj->GetName()) == std::string(aZoneSO->GetName()))
      {
        aStudyBuilder->NewCommand();
        aStudyBuilder->RemoveObject(aHypObj);
        aStudyBuilder->CommitCommand();
        break;
      }
    }
  }

  myZone->SupprHypo(nomHypothesis);
  myHypo->SupprZone(ZoneName);
};
//=============================================================================
//=============================================================================
//

//=============================================================================
//=============================================================================
// Recuperation des listes
//=============================================================================
//=============================================================================
HOMARD::listeBoundarys* HOMARD_Gen_i::GetAllBoundarysName()
{
  MESSAGE("GetAllBoundarysName");
  IsValidStudy () ;

  HOMARD::listeBoundarys_var ret = new HOMARD::listeBoundarys;
  ret->length(myStudyContext._mesBoundarys.size());
  std::map<std::string, HOMARD::HOMARD_Boundary_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesBoundarys.begin();
  it != myStudyContext._mesBoundarys.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
HOMARD::listeCases* HOMARD_Gen_i::GetAllCasesName()
{
  MESSAGE("GetAllCasesName");
  IsValidStudy () ;

  HOMARD::listeCases_var ret = new HOMARD::listeCases;
  ret->length(myStudyContext._mesCas.size());
  std::map<std::string, HOMARD::HOMARD_Cas_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesCas.begin();
  it != myStudyContext._mesCas.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
HOMARD::listeHypotheses* HOMARD_Gen_i::GetAllHypothesesName()
{
  MESSAGE("GetAllHypothesesName");
  IsValidStudy () ;

  HOMARD::listeHypotheses_var ret = new HOMARD::listeHypotheses;
  ret->length(myStudyContext._mesHypotheses.size());
  std::map<std::string, HOMARD::HOMARD_Hypothesis_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesHypotheses.begin();
  it != myStudyContext._mesHypotheses.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
HOMARD::listeIterations* HOMARD_Gen_i::GetAllIterationsName()
{
  MESSAGE("GetAllIterationsName");
  IsValidStudy () ;

  HOMARD::listeIterations_var ret = new HOMARD::listeIterations;
  ret->length(myStudyContext._mesIterations.size());
  std::map<std::string, HOMARD::HOMARD_Iteration_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesIterations.begin();
  it != myStudyContext._mesIterations.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
HOMARD::listeYACSs* HOMARD_Gen_i::GetAllYACSsName()
{
  MESSAGE("GetAllYACSsName");
  IsValidStudy () ;

  HOMARD::listeYACSs_var ret = new HOMARD::listeYACSs;
  ret->length(myStudyContext._mesYACSs.size());
  std::map<std::string, HOMARD::HOMARD_YACS_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesYACSs.begin();
  it != myStudyContext._mesYACSs.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
HOMARD::listeZones* HOMARD_Gen_i::GetAllZonesName()
{
  MESSAGE("GetAllZonesName");
  IsValidStudy () ;

  HOMARD::listeZones_var ret = new HOMARD::listeZones;
  ret->length(myStudyContext._mesZones.size());
  std::map<std::string, HOMARD::HOMARD_Zone_var>::const_iterator it;
  int i = 0;
  for (it = myStudyContext._mesZones.begin();
  it != myStudyContext._mesZones.end(); it++)
  {
    ret[i++] = CORBA::string_dup((*it).first.c_str());
  }

  return ret._retn();
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Recuperation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::GetBoundary(const char* nomBoundary)
{
  HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[nomBoundary];
  ASSERT(!CORBA::is_nil(myBoundary));
  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::GetCase(const char* nomCas)
{
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));
  return HOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
HOMARD::HOMARD_Hypothesis_ptr HOMARD_Gen_i::GetHypothesis(const char* nomHypothesis)
{
  HOMARD::HOMARD_Hypothesis_var myHypothesis = myStudyContext._mesHypotheses[nomHypothesis];
  ASSERT(!CORBA::is_nil(myHypothesis));
  return HOMARD::HOMARD_Hypothesis::_duplicate(myHypothesis);
}
//=============================================================================
HOMARD::HOMARD_Iteration_ptr  HOMARD_Gen_i::GetIteration(const char* NomIterationation)
{
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIterationation];
  ASSERT(!CORBA::is_nil(myIteration));
  return HOMARD::HOMARD_Iteration::_duplicate(myIteration);
}
//=============================================================================
HOMARD::HOMARD_YACS_ptr HOMARD_Gen_i::GetYACS(const char* nomYACS)
{
  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[nomYACS];
  ASSERT(!CORBA::is_nil(myYACS));
  return HOMARD::HOMARD_YACS::_duplicate(myYACS);
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::GetZone(const char* ZoneName)
{
  HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[ZoneName];
  ASSERT(!CORBA::is_nil(myZone));
  return HOMARD::HOMARD_Zone::_duplicate(myZone);
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Informations
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::MeshInfo(const char* nomCas, const char* MeshName, const char* MeshFile, const char* DirName, CORBA::Long Qual, CORBA::Long Diam, CORBA::Long Conn, CORBA::Long Tail, CORBA::Long Inte)
{
  INFOS ( "MeshInfo : nomCas = " << nomCas << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile  );
  INFOS ( "Qual = " << Qual << ", Diam = " << Diam << ", Conn = " << Conn << ", Tail = " << Tail << ", Inte = " << Inte  );
  IsValidStudy () ;

// Creation du cas
  int option = 1 ;
  if ( _PublisMeshIN != 0 ) option = 2 ;
  HOMARD::HOMARD_Cas_ptr myCase = CreateCase0(nomCas, MeshName, MeshFile, 1, 0, option) ;
  myCase->SetDirName(DirName) ;
// Analyse
  myCase->MeshInfo(Qual, Diam, Conn, Tail, Inte) ;

  return ;
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Recuperation des structures par le contexte
//=============================================================================
//=============================================================================
HOMARD::HOMARD_Iteration_ptr HOMARD_Gen_i::LastIteration(const char* nomCas)
{
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));
//
  HOMARD::HOMARD_Iteration_var myIteration = myCase->LastIteration();
  ASSERT(!CORBA::is_nil(myIteration));
//
  return HOMARD::HOMARD_Iteration::_duplicate(myIteration);
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Nouvelles structures
//=============================================================================
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::newCase()
{
  HOMARD::HOMARD_Gen_var engine = POA_HOMARD::HOMARD_Gen::_this();
  HOMARD_Cas_i* aServant = new HOMARD_Cas_i(_orb, engine);
  HOMARD::HOMARD_Cas_var aCase = HOMARD::HOMARD_Cas::_narrow(aServant->_this());
  return aCase._retn();
}
//=============================================================================
HOMARD::HOMARD_Hypothesis_ptr HOMARD_Gen_i::newHypothesis()
{
  HOMARD::HOMARD_Gen_var engine = POA_HOMARD::HOMARD_Gen::_this();
  HOMARD_Hypothesis_i* aServant = new HOMARD_Hypothesis_i(_orb, engine);
  HOMARD::HOMARD_Hypothesis_var aHypo = HOMARD::HOMARD_Hypothesis::_narrow(aServant->_this());
  return aHypo._retn();
}
//=============================================================================
HOMARD::HOMARD_Iteration_ptr HOMARD_Gen_i::newIteration()
{
  HOMARD::HOMARD_Gen_var engine = POA_HOMARD::HOMARD_Gen::_this();
  HOMARD_Iteration_i* aServant = new HOMARD_Iteration_i(_orb, engine);
  HOMARD::HOMARD_Iteration_var aIter = HOMARD::HOMARD_Iteration::_narrow(aServant->_this());
  return aIter._retn();
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::newBoundary()
{
  HOMARD::HOMARD_Gen_var engine = POA_HOMARD::HOMARD_Gen::_this();
  HOMARD_Boundary_i* aServant = new HOMARD_Boundary_i(_orb, engine);
  HOMARD::HOMARD_Boundary_var aBoundary = HOMARD::HOMARD_Boundary::_narrow(aServant->_this());
  return aBoundary._retn();
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::newZone()
{
  HOMARD::HOMARD_Gen_var engine = POA_HOMARD::HOMARD_Gen::_this();
  HOMARD_Zone_i* aServant = new HOMARD_Zone_i(_orb, engine);
  HOMARD::HOMARD_Zone_var aZone = HOMARD::HOMARD_Zone::_narrow(aServant->_this());
  return aZone._retn();
}
//=============================================================================
HOMARD::HOMARD_YACS_ptr HOMARD_Gen_i::newYACS()
{
  HOMARD::HOMARD_Gen_var engine = POA_HOMARD::HOMARD_Gen::_this();
  HOMARD_YACS_i* aServant = new HOMARD_YACS_i(_orb, engine);
  HOMARD::HOMARD_YACS_var aYACS = HOMARD::HOMARD_YACS::_narrow(aServant->_this());
  return aYACS._retn();
}
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
// Creation des structures identifiees par leurs noms
//=============================================================================
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCase(const char* nomCas, const char* MeshName, const char* MeshFile)
//
// Creation d'un cas initial
// nomCas : nom du cas a creer
// MeshName, MeshFile : nom et fichier du maillage correspondant
//
{
  INFOS ( "CreateCase : nomCas = " << nomCas << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile );

  int option = 1 ;
  if ( _PublisMeshIN != 0 ) option = 2 ;
  HOMARD::HOMARD_Cas_ptr myCase = CreateCase0(nomCas, MeshName, MeshFile, 0, 0, option) ;

// Valeurs par defaut des filtrages
  myCase->SetPyram(0);

  return HOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCaseFromIteration(const char* nomCas, const char* DirNameStart)
//
// nomCas : nom du cas a creer
// DirNameStart : nom du répertoire contenant l'iteration de reprise
//
{
  INFOS ( "CreateCaseFromIteration : nomCas = " << nomCas << ", DirNameStart = " << DirNameStart );
  std::string nomDirWork = getenv("PWD") ;
  int codret ;

  // A. Decodage du point de reprise
  // A.1. Controle du répertoire de depart de l'iteration
  codret = CHDIR(DirNameStart) ;
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the iteration does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  // A.2. Reperage des fichiers du répertoire de reprise
  std::string file_configuration = "" ;
  std::string file_maillage_homard = "" ;
  int bilan ;
#ifndef WIN32
  DIR *dp;
  struct dirent *dirp;
  dp  = opendir(DirNameStart);
  while ( (dirp = readdir(dp)) != NULL )
  {
    std::string file_name(dirp->d_name);
//     MESSAGE ( file_name );
    bilan = file_name.find("HOMARD.Configuration.") ;
    if ( bilan != string::npos ) { file_configuration = file_name ; }
    bilan = file_name.find("maill.") ;
    if ( bilan != string::npos )
    {
      bilan = file_name.find(".hom.med") ;
      if ( bilan != string::npos ) { file_maillage_homard = file_name ; }
    }
  }
  closedir(dp);
#else
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  hFind = FindFirstFile(DirNameStart, &ffd);
  if (INVALID_HANDLE_VALUE != hFind) {
    while (FindNextFile(hFind, &ffd) != 0) {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
      std::string file_name(ffd.cFileName);
      bilan = file_name.find("HOMARD.Configuration.") ;
      if ( bilan != string::npos ) { file_configuration = file_name ; }
      bilan = file_name.find("maill.") ;
      if ( bilan != string::npos )
      {
        bilan = file_name.find(".hom.med") ;
        if ( bilan != string::npos ) { file_maillage_homard = file_name ; }
      }
    }
    FindClose(hFind);
  }
#endif
  MESSAGE ( "==> file_configuration   : " << file_configuration ) ;
  MESSAGE ( "==> file_maillage_homard : " << file_maillage_homard ) ;
  // A.3. Controle
  if ( ( file_configuration == "" ) || ( file_maillage_homard == "" ) )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text ;
    if ( file_configuration == "" ) { text = "The configuration file cannot be found." ; }
    else                            { text = "The HOMARD mesh file cannot be found." ; }
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }

  // B. Lecture du fichier de configuration
  // ATTENTION : on doit veiller a la coherence entre HomardDriver et CreateCaseFromIteration
  int NumeIter ;
  int TypeExt = 0 ;
  int TypeConf = 0 ;
  int Pyram = 0 ;
  char* MeshName ;
  char* MeshFile ;
  // le constructeur de ifstream permet d'ouvrir un fichier en lecture
  std::ifstream fichier( file_configuration.c_str() );
  if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
  {
    std::string ligne; // variable contenant chaque ligne lue
    std::string mot_cle;
    std::string argument;
    int decalage;
    // cette boucle sur les lignes s'arrête dès qu'une erreur de lecture survient
    while ( std::getline( fichier, ligne ) )
    {
      // B.1. Pour la ligne courante, on identifie le premier mot : le mot-cle
      std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
      ligne_bis >> mot_cle ;
      // B.2. Des valeurs entieres : le second bloc de la ligne
      if ( mot_cle == "NumeIter" )
      {
        ligne_bis >> NumeIter ;
        NumeIter += 1 ;
      }
      // B.3. Des valeurs caracteres brutes : le second bloc de la ligne est la valeur
      else if ( ( mot_cle == "TypeConf" ) || ( mot_cle == "TypeElem" ) )
      {
        ligne_bis >> argument ;

        if ( mot_cle == "TypeConf" )
        {
          if      ( argument == "conforme" )                { TypeConf = 1 ; }
          else if ( argument == "non_conforme_1_noeud" )    { TypeConf = 2 ; }
          else if ( argument == "non_conforme_1_arete" )    { TypeConf = 3 ; }
          else if ( argument == "non_conforme_indicateur" ) { TypeConf = 4 ; }
        }
        else if ( mot_cle == "TypeElem" )
        {
          if ( argument == "ignore_pyra" ) { Pyram = 1 ; }
          else if ( argument == "HOMARD" ) { Pyram = 0 ; }
        }
      }
      // B.4. Des valeurs caracteres : le deuxieme bloc de la ligne peut etre encadre par des quotes :
      //                               il faut les supprimer
      else if ( ( mot_cle == "CCNoMNP1" ) || ( mot_cle == "CCMaiNP1" ) )
      {
        ligne_bis >> argument ;
        if ( argument[0] == '"' ) { decalage = 1 ; }
        else                      { decalage = 0 ; }
        size_t size = argument.size() + 1 - 2*decalage ;

        if ( mot_cle == "CCNoMNP1" )
        {
          MeshName = new char[ size ];
          strncpy( MeshName, argument.c_str()+decalage, size );
          MeshName[size-1] = '\0' ;
        }
        else if ( mot_cle == "CCMaiNP1" )
        {
          MeshFile = new char[ size ];
          strncpy( MeshFile, argument.c_str()+decalage, size );
          MeshFile[size-1] = '\0' ;
        }
      }
    }
    MESSAGE ( "==> TypeConf   : " << TypeConf ) ;
    MESSAGE ( "==> MeshName   : " << MeshName ) ;
    MESSAGE ( "==> MeshFile   : " << MeshFile ) ;
    MESSAGE ( "==> NumeIter   : " << NumeIter ) ;
    MESSAGE ( "==> Pyram      : " << Pyram ) ;
  }
  else
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "The configuration file cannot be read." ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }

  // C. Creation effective du cas

  int option = 1 ;
  if ( _PublisMeshIN != 0 ) option = 2 ;
  HOMARD::HOMARD_Cas_ptr myCase = CreateCase0(nomCas, MeshName, MeshFile, 1, NumeIter, option) ;

  // D. Parametrages lus dans le fichier de configuration

  myCase->SetConfType (TypeConf) ;
  myCase->SetExtType (TypeExt) ;
  myCase->SetPyram (Pyram) ;

  // E. Copie du fichier de maillage homard
  // E.1. Répertoire associe au cas
  char* nomDirCase = myCase->GetDirName() ;
  // E.2. Répertoire associe a l'iteration de ce cas
  char* IterName ;
  IterName = myCase->GetIter0Name() ;
  HOMARD::HOMARD_Iteration_var Iter = GetIteration(IterName) ;
  char* nomDirIter = CreateDirNameIter(nomDirCase, 0 );
  Iter->SetDirNameLoc(nomDirIter);
  std::string nomDirIterTotal ;
  nomDirIterTotal = std::string(nomDirCase) + "/" + std::string(nomDirIter) ;
#ifndef WIN32
  if (mkdir(nomDirIterTotal.c_str(), S_IRWXU|S_IRGRP|S_IXGRP) != 0)
#else
  if (_mkdir(nomDirIterTotal.c_str()) != 0)
#endif
  {
    MESSAGE ( "nomDirIterTotal : " << nomDirIterTotal ) ;
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "The directory for the computation cannot be created." ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }
  // E.3. Copie du maillage HOMARD au format MED
  codret = CHDIR(DirNameStart) ;
  std::string commande = "cp " + file_maillage_homard + " " + nomDirIterTotal ;
  MESSAGE ( "commande : " << commande ) ;
  codret = system(commande.c_str()) ;
  MESSAGE ( "codret : " << codret ) ;
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The starting point for the case cannot be copied into the working directory.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  // F. Menage

  delete[] MeshName ;
  delete[] MeshFile ;

  CHDIR(nomDirWork.c_str());
  return HOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCaseFromCaseLastIteration(const char* nomCas, const char* DirNameStart)
//
// nomCas : nom du cas a creer
// DirNameStart : nom du répertoire du cas contenant l'iteration de reprise
//
{
  INFOS ( "CreateCaseFromCaseLastIteration : nomCas = " << nomCas << ", DirNameStart = " << DirNameStart );

  std::string DirNameStartIter = CreateCase1(DirNameStart, -1) ;

  DirNameStartIter = string(DirNameStart) + "/" + DirNameStartIter ;
  HOMARD::HOMARD_Cas_ptr myCase = CreateCaseFromIteration(nomCas, DirNameStartIter.c_str()) ;

  return HOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCaseFromCaseIteration(const char* nomCas, const char* DirNameStart, CORBA::Long Number)
//
// nomCas : nom du cas a creer
// DirNameStart : nom du répertoire du cas contenant l'iteration de reprise
// Number : numero de l'iteration de depart
//
{
  INFOS ( "CreateCaseFromCaseIteration : nomCas = " << nomCas << ", DirNameStart = " << DirNameStart << ", Number = " << Number );
  if ( Number < 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The number of iteration must be positive.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  std::string DirNameStartIter = CreateCase1(DirNameStart, Number) ;

  DirNameStartIter = string(DirNameStart) + "/" + DirNameStartIter ;
  HOMARD::HOMARD_Cas_ptr myCase = CreateCaseFromIteration(nomCas, DirNameStartIter.c_str()) ;

  return HOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
std::string HOMARD_Gen_i::CreateCase1(const char* DirNameStart, CORBA::Long Number)
//
// Retourne le nom du répertoire ou se trouve l'iteration voulue.
// DirNameStart : nom du répertoire du cas contenant l'iteration de reprise
// Number : numero de l'iteration de depart ou -1 si on cherche la derniere
//
{
  MESSAGE ( "CreateCase1 : DirNameStart = " << DirNameStart << ", Number = " << Number );
  std::string nomDirWork = getenv("PWD") ;
  std::string DirNameStartIter ;
  int codret ;
  int NumeIterMax = -1 ;

  // A.1. Controle du répertoire de depart du cas
  codret = CHDIR(DirNameStart) ;
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the case for the pursuit does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  // A.2. Reperage des sous-répertoire du répertoire de reprise
  bool existe = false ;
#ifndef WIN32
  DIR *dp;
  struct dirent *dirp;
  dp  = opendir(DirNameStart);
  while ( (dirp = readdir(dp)) != NULL ) {
    std::string DirName_1(dirp->d_name);
#else
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  hFind = FindFirstFile(DirNameStart, &ffd);
  if (INVALID_HANDLE_VALUE != hFind) {
    while (FindNextFile(hFind, &ffd) != 0) {
      std::string DirName_1 = "";
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        DirName_1 = std::string(ffd.cFileName);
      }
#endif
    if ( ( DirName_1 != "." ) && ( DirName_1 != ".." ) )
    {
      if ( CHDIR(DirName_1.c_str()) == 0 )
      {
//      On cherche le fichier de configuration dans ce sous-répertoire
        codret = CHDIR(DirNameStart);
#ifndef WIN32
        DIR *dp_1;
        struct dirent *dirp_1;
        dp_1  = opendir(DirName_1.c_str()) ;
        while ( (dirp_1 = readdir(dp_1)) != NULL )
        {
          std::string file_name_1(dirp_1->d_name);
#else
        HANDLE hFind1 = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA ffd1;
        hFind1 = FindFirstFile(DirName_1.c_str(), &ffd1);
        while (FindNextFile(hFind1, &ffd1) != 0)
        {
          if (ffd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; //skip directories
          std::string file_name_1(ffd1.cFileName);
#endif
          int bilan = file_name_1.find("HOMARD.Configuration.") ;
          if ( bilan != string::npos )
          {
  // Decodage du fichier pour trouver le numero d'iteration
            CHDIR(DirName_1.c_str()) ;

            std::ifstream fichier( file_name_1.c_str() );
            if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
            {
              int NumeIter ;
              std::string ligne; // variable contenant chaque ligne lue
              std::string mot_cle;
              // cette boucle sur les lignes s'arrête dès qu'une erreur de lecture survient
              while ( std::getline( fichier, ligne ) )
              {
                // B.1. Pour la ligne courante, on identifie le premier mot : le mot-cle
                std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
                ligne_bis >> mot_cle ;
                if ( mot_cle == "NumeIter" )
                {
                  ligne_bis >> NumeIter ;
                  NumeIter += 1 ;
//                   MESSAGE ( "==> NumeIter   : " << NumeIter ) ;
                  if ( Number == - 1 )
                  {
                    if ( NumeIter >= NumeIterMax )
                    {
                      NumeIterMax = NumeIter ;
                      DirNameStartIter = DirName_1 ;
                    }
                  }
                  else
                  {
                    if ( NumeIter == Number )
                    {
                      DirNameStartIter = DirName_1 ;
                      existe = true ;
                      break ;
                    }
                  }
                }
              }
            }
            else
            {
              SALOME::ExceptionStruct es;
              es.type = SALOME::BAD_PARAM;
              std::string text = "The configuration file cannot be read." ;
              es.text = CORBA::string_dup(text.c_str());
              throw SALOME::SALOME_Exception(es);
            }
            CHDIR(DirNameStart) ;
          }
          if ( existe ) { break ; }
        }
#ifndef WIN32
        closedir(dp_1);
#else
        FindClose(hFind1);
#endif
        if ( existe ) { break ; }
     }
    }
  }
#ifndef WIN32
  closedir(dp);
#else
  FindClose(hFind);
#endif
  CHDIR(nomDirWork.c_str());

  if ( ( Number >= 0 && ( !existe ) ) || ( Number < 0 && ( NumeIterMax == -1 ) ) )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the iteration does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  return DirNameStartIter ;
}
//=============================================================================
HOMARD::HOMARD_Cas_ptr HOMARD_Gen_i::CreateCase0(const char* nomCas, const char* MeshName, const char* MeshFile, CORBA::Long MeshOption, CORBA::Long NumeIter, CORBA::Long Option)
//
// nomCas : nom du cas a creer
// MeshName, MeshFile : nom et fichier du maillage correspondant
// MeshOption : 0 : le maillage fourni est obligatoirement present ==> erreur si absent
//              1 : le maillage fourni peut ne pas exister ==> on continue si absent
//             -1 : le maillage n'est pas fourni
// NumeIter : numero de l'iteration correspondante : 0, pour un depart, n>0 pour une poursuite
// Option : multiple de nombres premiers
//         1 : aucune option
//        x2 : publication du maillage dans SMESH
{
  MESSAGE ( "CreateCase0 : nomCas = " << nomCas );
  MESSAGE ( "CreateCase0 : MeshName = " << MeshName << ", MeshFile = " << MeshFile << ", MeshOption = " << MeshOption );
  MESSAGE ( "CreateCase0 : NumeIter = " << NumeIter << ", Option = " << Option );
//
  // A. Controles
  // A.1. L'etude
  IsValidStudy () ;

  // A.2. Controle du nom :
  if ((myStudyContext._mesCas).find(nomCas)!=(myStudyContext._mesCas).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This case has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  // A.3. Controle du fichier du maillage
  int existeMeshFile ;
  if ( MeshOption >= 0 )
  {
    existeMeshFile = MEDFileExist ( MeshFile ) ;
    MESSAGE ( "CreateCase0 : existeMeshFile = " << existeMeshFile );
    if ( ( existeMeshFile == 0 ) && ( MeshOption == 0 ) )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "The mesh file does not exist.";
      throw SALOME::SALOME_Exception(es);
      return 0;
    }
  }
  else { existeMeshFile = 0 ; }

  // B. Creation de l'objet cas et publication
//   MESSAGE ( "CreateCase0 : Creation de l'objet" );
  HOMARD::HOMARD_Cas_var myCase = newCase();
  myCase->SetName(nomCas);
  SALOMEDS::SObject_var aSO;
  SALOMEDS::SObject_var aResultSO=PublishInStudy(aSO, myCase, nomCas);
  myStudyContext._mesCas[nomCas] = myCase;

  // C. Caracteristiques du maillage
  if ( existeMeshFile != 0 )
  {
  // Les valeurs extremes des coordonnées
//     MESSAGE ( "CreateCase0 : Les valeurs extremes des coordonnées" );
    std::vector<double> LesExtremes =GetBoundingBoxInMedFile(MeshFile) ;
    HOMARD::extrema_var aSeq = new HOMARD::extrema() ;
    if (LesExtremes.size()!=10) { return 0; }
    aSeq->length(10) ;
    for (int i =0 ; i< LesExtremes.size() ; i++)
        aSeq[i]=LesExtremes[i] ;
    myCase->SetBoundingBox(aSeq) ;
  // Les groupes
//     MESSAGE ( "CreateCase0 : Les groupes" );
    std::set<std::string> LesGroupes  =GetListeGroupesInMedFile(MeshFile) ;
    HOMARD::ListGroupType_var aSeqGroupe = new HOMARD::ListGroupType ;
    aSeqGroupe->length(LesGroupes.size());
    std::set<std::string>::const_iterator it ;
    int i = 0 ;
    for (it=LesGroupes.begin() ; it != LesGroupes.end() ; it++)
      aSeqGroupe[i++]=(*it).c_str() ;
    myCase->SetGroups(aSeqGroupe) ;
  }

  // D. L'iteration initiale du cas
  MESSAGE ( "CreateCase0 : iteration initiale du cas" );
  // D.1. Recherche d'un nom : par defaut, on prend le nom du maillage correspondant.
  // Si ce nom d'iteration existe deja, on incremente avec 0, 1, 2, etc.
  int monNum = 0;
  std::string NomIteration = std::string(MeshName) ;
  while ( (myStudyContext._mesIterations).find(NomIteration) != (myStudyContext._mesIterations.end()) )
  {
    std::ostringstream nom;
    nom << MeshName << monNum;
    NomIteration = nom.str();
    monNum += 1;
  }
  MESSAGE ( "CreateCas0 : ==> NomIteration = " << NomIteration );

  // D.2. Creation de l'iteration
  HOMARD::HOMARD_Iteration_var anIter = newIteration();
  myStudyContext._mesIterations[NomIteration] = anIter;
  anIter->SetName(NomIteration.c_str());
  AssociateCaseIter (nomCas, NomIteration.c_str(), "IterationHomard");

  // D.4. Maillage correspondant
  if ( existeMeshFile != 0 )
  {
    anIter->SetMeshFile(MeshFile);
    if ( Option % 2 == 0 ) { PublishResultInSmesh(MeshFile, 0); }
  }
  anIter->SetMeshName(MeshName);

  // D.5. Numero d'iteration
  anIter->SetNumber(NumeIter);

  // D.6. Etat
  SetEtatIter(NomIteration.c_str(), -NumeIter);
//

  return HOMARD::HOMARD_Cas::_duplicate(myCase);
}
//=============================================================================
HOMARD::HOMARD_Hypothesis_ptr HOMARD_Gen_i::CreateHypothesis(const char* nomHypothesis)
{
  INFOS ( "CreateHypothesis : nomHypothesis = " << nomHypothesis );
  IsValidStudy () ;

  // A. Controle du nom :
  if ((myStudyContext._mesHypotheses).find(nomHypothesis) != (myStudyContext._mesHypotheses).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This hypothesis has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  }

  // B. Creation de l'objet
  HOMARD::HOMARD_Hypothesis_var myHypothesis = newHypothesis();
  if (CORBA::is_nil(myHypothesis))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Unable to create the hypothesis";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  myHypothesis->SetName(nomHypothesis);

  // C. Enregistrement
  myStudyContext._mesHypotheses[nomHypothesis] = myHypothesis;

  SALOMEDS::SObject_var aSO;
  SALOMEDS::SObject_var aResultSO=PublishInStudy(aSO, myHypothesis, nomHypothesis);

  // D. Valeurs par defaut des options avancees
  myHypothesis->SetNivMax(-1);
  myHypothesis->SetDiamMin(-1.0);
  myHypothesis->SetAdapInit(0);
  myHypothesis->SetExtraOutput(1);

  return HOMARD::HOMARD_Hypothesis::_duplicate(myHypothesis);
}

//=============================================================================
HOMARD::HOMARD_Iteration_ptr HOMARD_Gen_i::CreateIteration(const char* NomIteration, const char* nomIterParent)
//=============================================================================
{
  INFOS ("CreateIteration : NomIteration  = " << NomIteration << ", nomIterParent = " << nomIterParent);
  IsValidStudy () ;

  HOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterParent];
  if (CORBA::is_nil(myIterationParent))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The parent iteration is not defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  const char* nomCas = myIterationParent->GetCaseName();
  MESSAGE ("CreateIteration : nomCas = " << nomCas);
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  if (CORBA::is_nil(myCase))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid case context";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  const char* nomDirCase = myCase->GetDirName();

  // Controle du nom :
  if ((myStudyContext._mesIterations).find(NomIteration)!=(myStudyContext._mesIterations).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

   HOMARD::HOMARD_Iteration_var myIteration = newIteration();
   if (CORBA::is_nil(myIteration))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Unable to create the iteration";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  myStudyContext._mesIterations[std::string(NomIteration)] = myIteration;
// Nom de l'iteration et du maillage
  myIteration->SetName(NomIteration);
  myIteration->SetMeshName(NomIteration);
  myIteration->SetState(1);

  int numero = myIterationParent->GetNumber() + 1;
  myIteration->SetNumber(numero);

// Nombre d'iterations deja connues pour le cas, permettant
// la creation d'un sous-répertoire unique
  int nbitercase = myCase->GetNumberofIter();
  char* nomDirIter = CreateDirNameIter(nomDirCase, nbitercase );
  myIteration->SetDirNameLoc(nomDirIter);

// Le nom du fichier du maillage MED est indice par le nombre d'iterations du cas.
// Si on a une chaine unique depuis le depart, ce nombre est le meme que le
// numero d'iteration dans la sucession : maill.01.med, maill.02.med, etc... C'est la
// situation la plus frequente.
// Si on a plusieurs branches, donc des iterations du meme niveau d'adaptation, utiliser
// le nombre d'iterations du cas permet d'eviter les collisions.
  int jaux ;
  if      ( nbitercase <    100 ) { jaux = 2 ; }
  else if ( nbitercase <   1000 ) { jaux = 3 ; }
  else if ( nbitercase <  10000 ) { jaux = 4 ; }
  else if ( nbitercase < 100000 ) { jaux = 5 ; }
  else                            { jaux = 9 ; }
  std::ostringstream iaux ;
  iaux << std::setw(jaux) << std::setfill('0') << nbitercase ;
  std::stringstream MeshFile;
  MeshFile << nomDirCase << "/maill." << iaux.str() << ".med";
  myIteration->SetMeshFile(MeshFile.str().c_str());

// Association avec le cas
  std::string label = "IterationHomard_" + std::string(nomIterParent);
  AssociateCaseIter(nomCas, NomIteration, label.c_str());
// Lien avec l'iteration precedente
  myIterationParent->LinkNextIteration(NomIteration);
  myIteration->SetIterParentName(nomIterParent);
  // Gestion de l'arbre d'etudes
  SALOMEDS::SObject_var aIterSOParent = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIterationParent)));
  SALOMEDS::SObject_var aIterSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  aStudyBuilder->NewCommand();
  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aIterSO);
  aStudyBuilder->Addreference(aSubSO, aIterSOParent);
  aStudyBuilder->CommitCommand();

  return HOMARD::HOMARD_Iteration::_duplicate(myIteration);
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundary(const char* BoundaryName, CORBA::Long BoundaryType)
{
  MESSAGE ("CreateBoundary : BoundaryName  = " << BoundaryName << ", BoundaryType = " << BoundaryType);
  IsValidStudy () ;

  // Controle du nom :
  if ((myStudyContext._mesBoundarys).find(BoundaryName)!=(myStudyContext._mesBoundarys).end())
  {
    MESSAGE ("CreateBoundary : la frontiere " << BoundaryName << " existe deja");
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This boundary has already been defined";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  HOMARD::HOMARD_Boundary_var myBoundary = newBoundary();
  myBoundary->SetName(BoundaryName);
  myBoundary->SetType(BoundaryType);

  myStudyContext._mesBoundarys[BoundaryName] = myBoundary;

  SALOMEDS::SObject_var aSO;
  SALOMEDS::SObject_var aResultSO=PublishInStudy(aSO, myBoundary, BoundaryName);

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryCAO(const char* BoundaryName, const char* CAOFile)
{
  INFOS ("CreateBoundaryCAO : BoundaryName  = " << BoundaryName << ", CAOFile = " << CAOFile );
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, -1);
  myBoundary->SetDataFile( CAOFile ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryDi(const char* BoundaryName, const char* MeshName, const char* MeshFile)
{
  INFOS ("CreateBoundaryDi : BoundaryName  = " << BoundaryName << ", MeshName = " << MeshName << ", MeshFile = " << MeshFile );
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 0);
  myBoundary->SetDataFile( MeshFile ) ;
  myBoundary->SetMeshName( MeshName ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary);
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryCylinder(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double Rayon)
{
  INFOS ("CreateBoundaryCylinder : BoundaryName  = " << BoundaryName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe) ;
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector." ;
    error = 2 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 1) ;
  myBoundary->SetCylinder( Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, Rayon ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary) ;
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundarySphere(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Rayon)
{
  INFOS ("CreateBoundarySphere : BoundaryName  = " << BoundaryName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 2) ;
  myBoundary->SetSphere( Xcentre, Ycentre, Zcentre, Rayon ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary) ;
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryConeA(const char* BoundaryName,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe, CORBA::Double Angle,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre)
{
  INFOS ("CreateBoundaryConeA : BoundaryName  = " << BoundaryName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Angle <= 0.0 || Angle >= 90.0 )
  { es.text = "The angle must be included higher than 0 degree and lower than 90 degrees." ;
    error = 1 ; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe) ;
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector." ;
    error = 2 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 3) ;
  myBoundary->SetConeA( Xaxe, Yaxe, Zaxe, Angle, Xcentre, Ycentre, Zcentre ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary) ;
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryConeR(const char* BoundaryName,
                                      CORBA::Double Xcentre1, CORBA::Double Ycentre1, CORBA::Double Zcentre1, CORBA::Double Rayon1,
                                      CORBA::Double Xcentre2, CORBA::Double Ycentre2, CORBA::Double Zcentre2, CORBA::Double Rayon2)
{
  INFOS ("CreateBoundaryConeR : BoundaryName  = " << BoundaryName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon1 < 0.0 || Rayon2 < 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  double daux = fabs(Rayon2-Rayon1) ;
  if ( daux < 0.0000001 )
  { es.text = "The radius must be different." ;
    error = 2 ; }
  daux = fabs(Xcentre2-Xcentre1) + fabs(Ycentre2-Ycentre1) + fabs(Zcentre2-Zcentre1) ;
  if ( daux < 0.0000001 )
  { es.text = "The centers must be different." ;
    error = 3 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 4) ;
  myBoundary->SetConeR( Xcentre1, Ycentre1, Zcentre1, Rayon1, Xcentre2, Ycentre2, Zcentre2, Rayon2 ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary) ;
}
//=============================================================================
HOMARD::HOMARD_Boundary_ptr HOMARD_Gen_i::CreateBoundaryTorus(const char* BoundaryName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double RayonRev, CORBA::Double RayonPri)
{
  INFOS ("CreateBoundaryTorus : BoundaryName  = " << BoundaryName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( ( RayonRev <= 0.0 ) || ( RayonPri <= 0.0 ) )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe) ;
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector." ;
    error = 2 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Boundary_var myBoundary = CreateBoundary(BoundaryName, 5) ;
  myBoundary->SetTorus( Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, RayonRev, RayonPri ) ;

  return HOMARD::HOMARD_Boundary::_duplicate(myBoundary) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZone(const char* ZoneName, CORBA::Long ZoneType)
{
  MESSAGE ("CreateZone : ZoneName  = " << ZoneName << ", ZoneType = " << ZoneType);
  IsValidStudy () ;

  // Controle du nom :
  if ((myStudyContext._mesZones).find(ZoneName)!=(myStudyContext._mesZones).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This zone has already been defined";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

  HOMARD::HOMARD_Zone_var myZone = newZone();
  myZone->SetName(ZoneName);
  myZone->SetType(ZoneType);

  myStudyContext._mesZones[ZoneName] = myZone;

  SALOMEDS::SObject_var aSO;
  SALOMEDS::SObject_var aResultSO=PublishInStudy(aSO, myZone, ZoneName);

  return HOMARD::HOMARD_Zone::_duplicate(myZone);
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZoneBox(const char* ZoneName,
                                      CORBA::Double Xmini, CORBA::Double Xmaxi,
                                      CORBA::Double Ymini, CORBA::Double Ymaxi,
                                      CORBA::Double Zmini, CORBA::Double Zmaxi)
{
  INFOS ("CreateZoneBox : ZoneName  = " << ZoneName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Xmini > Xmaxi )
  { es.text = "The X coordinates are not coherent." ;
    error = 1 ; }
  if ( Ymini > Ymaxi )
  { es.text = "The Y coordinates are not coherent." ;
    error = 2 ; }
  if ( Zmini > Zmaxi )
  { es.text = "The Z coordinates are not coherent." ;
    error = 3 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 2) ;
  myZone->SetBox ( Xmini, Xmaxi, Ymini, Ymaxi, Zmini, Zmaxi) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZoneSphere(const char* ZoneName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre, CORBA::Double Rayon)
{
  INFOS ("CreateZoneSphere : ZoneName  = " << ZoneName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 4) ;
  myZone->SetSphere( Xcentre, Ycentre, Zcentre, Rayon ) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZoneCylinder(const char* ZoneName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double Rayon, CORBA::Double Haut)
{
  INFOS ("CreateZoneCylinder : ZoneName  = " << ZoneName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe) ;
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector." ;
    error = 2 ; }
  if ( Haut <= 0.0 )
  { es.text = "The height must be positive." ;
    error = 3 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 5) ;
  myZone->SetCylinder( Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, Rayon, Haut ) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZonePipe(const char* ZoneName,
                                      CORBA::Double Xcentre, CORBA::Double Ycentre, CORBA::Double Zcentre,
                                      CORBA::Double Xaxe, CORBA::Double Yaxe, CORBA::Double Zaxe,
                                      CORBA::Double Rayon, CORBA::Double Haut, CORBA::Double Rayonint)
{
  INFOS ("CreateZonePipe : ZoneName  = " << ZoneName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 || Rayonint <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  double daux = fabs(Xaxe) + fabs(Yaxe) + fabs(Zaxe) ;
  if ( daux < 0.0000001 )
  { es.text = "The axis must be a non 0 vector." ;
    error = 2 ; }
  if ( Haut <= 0.0 )
  { es.text = "The height must be positive." ;
    error = 3 ; }
  if ( Rayon <= Rayonint )
  { es.text = "The external radius must be higher than the internal radius." ;
    error = 4 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 7) ;
  myZone->SetPipe( Xcentre, Ycentre, Zcentre, Xaxe, Yaxe, Zaxe, Rayon, Haut, Rayonint ) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZoneBox2D(const char* ZoneName,
                                      CORBA::Double Umini, CORBA::Double Umaxi,
                                      CORBA::Double Vmini, CORBA::Double Vmaxi,
                                      CORBA::Long Orient)
{
  INFOS ("CreateZoneBox2D : ZoneName  = " << ZoneName ) ;
//   MESSAGE ("Umini = " << Umini << ", Umaxi =" << Umaxi ) ;
//   MESSAGE ("Vmini = " << Vmini << ", Vmaxi =" << Vmaxi ) ;
//   MESSAGE ("Orient = " << Orient ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Umini > Umaxi )
  { es.text = "The first coordinates are not coherent." ;
    error = 1 ; }
  if ( Vmini > Vmaxi )
  { es.text = "The second coordinates are not coherent." ;
    error = 2 ; }
  if ( Orient < 1 || Orient > 3 )
  { es.text = "The orientation must be 1, 2 or 3." ;
    error = 3 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  double Xmini, Xmaxi ;
  double Ymini, Ymaxi ;
  double Zmini, Zmaxi ;
  if ( Orient == 1 )
  { Xmini = Umini ;
    Xmaxi = Umaxi ;
    Ymini = Vmini ;
    Ymaxi = Vmaxi ;
    Zmini = 0. ;
    Zmaxi = 0. ; }
  else if ( Orient == 2 )
  { Xmini = 0. ;
    Xmaxi = 0. ;
    Ymini = Umini ;
    Ymaxi = Umaxi ;
    Zmini = Vmini ;
    Zmaxi = Vmaxi ; }
  else if ( Orient == 3 )
  { Xmini = Vmini ;
    Xmaxi = Vmaxi ;
    Ymini = 0. ;
    Ymaxi = 0. ;
    Zmini = Umini ;
    Zmaxi = Umaxi ; }
  else { VERIFICATION( (Orient>=1) && (Orient<=3) ) ; }

  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 10+Orient) ;
  myZone->SetBox ( Xmini, Xmaxi, Ymini, Ymaxi, Zmini, Zmaxi) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZoneDisk(const char* ZoneName,
                                      CORBA::Double Ucentre, CORBA::Double Vcentre,
                                      CORBA::Double Rayon,
                                      CORBA::Long Orient)
{
  INFOS ("CreateZoneDisk : ZoneName  = " << ZoneName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  if ( Orient < 1 || Orient > 3 )
  { es.text = "The orientation must be 1, 2 or 3." ;
    error = 3 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  double Xcentre ;
  double Ycentre ;
  double Zcentre ;
  if ( Orient == 1 )
  { Xcentre = Ucentre ;
    Ycentre = Vcentre ;
    Zcentre = 0. ; }
  else if ( Orient == 2 )
  { Xcentre = 0. ;
    Ycentre = Ucentre ;
    Zcentre = Vcentre ; }
  else if ( Orient == 3 )
  { Xcentre = Vcentre ;
    Ycentre = 0. ;
    Zcentre = Ucentre ; }
  else { VERIFICATION( (Orient>=1) && (Orient<=3) ) ; }

  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 30+Orient) ;
  myZone->SetCylinder( Xcentre, Ycentre, Zcentre, 0., 0., 1., Rayon, 1. ) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
HOMARD::HOMARD_Zone_ptr HOMARD_Gen_i::CreateZoneDiskWithHole(const char* ZoneName,
                                      CORBA::Double Ucentre, CORBA::Double Vcentre,
                                      CORBA::Double Rayon, CORBA::Double Rayonint,
                                      CORBA::Long Orient)
{
  INFOS ("CreateZoneDiskWithHole : ZoneName  = " << ZoneName ) ;
//
  SALOME::ExceptionStruct es;
  int error = 0 ;
  if ( Rayon <= 0.0 || Rayonint <= 0.0 )
  { es.text = "The radius must be positive." ;
    error = 1 ; }
  if ( Orient < 1 || Orient > 3 )
  { es.text = "The orientation must be 1, 2 or 3." ;
    error = 3 ; }
  if ( Rayon <= Rayonint )
  { es.text = "The external radius must be higher than the internal radius." ;
    error = 4 ; }
  if ( error != 0 )
  {
    es.type = SALOME::BAD_PARAM;
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
//
  double Xcentre ;
  double Ycentre ;
  double Zcentre ;
  if ( Orient == 1 )
  { Xcentre = Ucentre ;
    Ycentre = Vcentre ;
    Zcentre = 0. ; }
  else if ( Orient == 2 )
  { Xcentre = 0. ;
    Ycentre = Ucentre ;
    Zcentre = Vcentre ; }
  else if ( Orient == 3 )
  { Xcentre = Vcentre ;
    Ycentre = 0. ;
    Zcentre = Ucentre ; }
  else { VERIFICATION( (Orient>=1) && (Orient<=3) ) ; }

  HOMARD::HOMARD_Zone_var myZone = CreateZone(ZoneName, 60+Orient) ;
  myZone->SetPipe( Xcentre, Ycentre, Zcentre, 0., 0., 1., Rayon, 1., Rayonint ) ;

  return HOMARD::HOMARD_Zone::_duplicate(myZone) ;
}
//=============================================================================
//=============================================================================




//=============================================================================
//=============================================================================
// Traitement d'une iteration
// etatMenage = 1 : destruction du répertoire d'execution
// modeHOMARD  = 1 : adaptation
//            != 1 : information avec les options modeHOMARD
// Option1 >0 : appel depuis python
//         <0 : appel depuis GUI
// Option2 : multiple de nombres premiers
//         1 : aucune option
//        x2 : publication du maillage dans SMESH
//=============================================================================
CORBA::Long HOMARD_Gen_i::Compute(const char* NomIteration, CORBA::Long etatMenage, CORBA::Long modeHOMARD, CORBA::Long Option1, CORBA::Long Option2)
{
  INFOS ( "Compute : traitement de " << NomIteration << ", avec modeHOMARD = " << modeHOMARD << ", Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  int codret = 0;

  // A.1. L'objet iteration
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIteration];
  ASSERT(!CORBA::is_nil(myIteration));

  // A.2. Controle de la possibilite d'agir
  // A.2.1. Etat de l'iteration
  int etat = myIteration->GetState();
  MESSAGE ( "etat = "<<etat );
  // A.2.2. On ne calcule pas l'iteration initiale, ni une iteration deja calculee
  if ( modeHOMARD == 1 )
  {
    if ( etat <= 0 )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration is the first of the case and cannot be computed.";
      throw SALOME::SALOME_Exception(es);
      return 1 ;
    }
    else if ( ( etat == 2 ) & ( modeHOMARD == 1 ) )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration is already computed.";
      throw SALOME::SALOME_Exception(es);
      return 1 ;
    }
  }
  // A.2.3. On n'analyse pas une iteration non calculee
  else
  {
    if ( etat == 1 )
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration is not computed.";
      throw SALOME::SALOME_Exception(es);
      return 1 ;
    }
  }

  // A.3. Numero de l'iteration
  //     siterp1 : numero de l'iteration a traiter
  //     Si adaptation :
  //        siter   : numero de l'iteration parent, ou 0 si deja au debut mais cela ne servira pas !
  //     Ou si information :
  //        siter = siterp1
  int NumeIter = myIteration->GetNumber();
  std::string siterp1 ;
  std::stringstream saux1 ;
  saux1 << NumeIter ;
  siterp1 = saux1.str() ;
  if (NumeIter < 10) { siterp1 = "0" + siterp1 ; }

  std::string siter ;
  if ( modeHOMARD==1 )
  {
    std::stringstream saux0 ;
    int iaux = max(0, NumeIter-1) ;
    saux0 << iaux ;
    siter = saux0.str() ;
    if (NumeIter < 11) { siter = "0" + siter ; }
  }
  else
  { siter = siterp1 ; }

  // A.4. Le cas
  const char* nomCas = myIteration->GetCaseName();
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));

  // B. Les répertoires
  // B.1. Le répertoire courant
  std::string nomDirWork = getenv("PWD") ;
  // B.2. Le sous-répertoire de l'iteration a traiter
  char* DirCompute = ComputeDirManagement(myCase, myIteration, etatMenage);
  MESSAGE( ". DirCompute = " << DirCompute );

  // C. Le fichier des messages
  // C.1. Le deroulement de l'execution de HOMARD
  std::string LogFile = DirCompute ;
  LogFile += "/Liste" ;
  if ( modeHOMARD == 1 ) { LogFile += "." + siter + ".vers." + siterp1 ; }
  LogFile += ".log" ;
  MESSAGE (". LogFile = " << LogFile);
  if ( modeHOMARD == 1 ) { myIteration->SetLogFile(LogFile.c_str()); }
  // C.2. Le bilan de l'analyse du maillage
  std::string FileInfo = DirCompute ;
  FileInfo += "/" ;
  if ( modeHOMARD == 1 ) { FileInfo += "apad" ; }
  else
  { if ( NumeIter == 0 ) { FileInfo += "info_av" ; }
    else                 { FileInfo += "info_ap" ; }
  }
  FileInfo += "." + siterp1 + ".bilan" ;
  myIteration->SetFileInfo(FileInfo.c_str());

   // D. On passe dans le répertoire de l'iteration a calculer
  MESSAGE ( ". On passe dans DirCompute = " << DirCompute );
  CHDIR(DirCompute);

  // E. Les données de l'exécution HOMARD
  // E.1. L'objet du texte du fichier de configuration
  HomardDriver* myDriver = new HomardDriver(siter, siterp1);
  myDriver->TexteInit(DirCompute, LogFile, _Langue);

  // E.2. Le maillage associe a l'iteration
  const char* NomMesh = myIteration->GetMeshName();
  MESSAGE ( ". NomMesh = " << NomMesh );
  const char* MeshFile = myIteration->GetMeshFile();
  MESSAGE ( ". MeshFile = " << MeshFile );

  // E.3. Les données du traitement HOMARD
  int iaux ;
  if ( modeHOMARD == 1 )
  {
    iaux = 1 ;
    myDriver->TexteMaillageHOMARD( DirCompute, siterp1, iaux ) ;
    myDriver->TexteMaillage(NomMesh, MeshFile, 1);
    codret = ComputeAdap(myCase, myIteration, etatMenage, myDriver, Option1, Option2) ;
  }
  else
  {
    InvalideIterInfo(NomIteration);
    myDriver->TexteInfo( modeHOMARD, NumeIter ) ;
    iaux = 0 ;
    myDriver->TexteMaillageHOMARD( DirCompute, siterp1, iaux ) ;
    myDriver->TexteMaillage(NomMesh, MeshFile, 0);
    myDriver->CreeFichierDonn();
  }

  // E.4. Ajout des informations liees a l'eventuel suivi de frontiere
  int BoundaryOption = DriverTexteBoundary(myCase, myDriver) ;

  // E.5. Ecriture du texte dans le fichier
  MESSAGE ( ". Ecriture du texte dans le fichier de configuration ; codret = "<<codret );
  if (codret == 0)
  { myDriver->CreeFichier(); }

// G. Execution
//
  int codretexec = 1789 ;
  if (codret == 0)
  {
    codretexec = myDriver->ExecuteHomard(Option1);
//
    MESSAGE ( "Erreur en executant HOMARD : " << codretexec );
    // En mode adaptation, on ajuste l'etat de l'iteration
    if ( modeHOMARD == 1 )
    {
      if (codretexec == 0) { SetEtatIter(NomIteration,2); }
      else                 { SetEtatIter(NomIteration,1); }
      // GERALD -- QMESSAGE BOX
    }
  }

  // H. Gestion des resultats
  if (codret == 0)
  {
    std::string Commentaire ;
    // H.1. Le fichier des messages, dans tous les cas
    Commentaire = "log" ;
    if ( modeHOMARD == 1 ) { Commentaire += " " + siterp1 ; }
    else                   { Commentaire += "Info" ; }
    PublishFileUnderIteration(NomIteration, LogFile.c_str(), Commentaire.c_str());

    // H.2. Si tout s'est bien passe :
    if (codretexec == 0)
    {
    // H.2.1. Le fichier de bilan
      Commentaire = "Summary" ;
      if ( modeHOMARD == 1 ) { Commentaire += " " + siterp1 ; }
      else                   { Commentaire += "Info" ; }
      PublishFileUnderIteration(NomIteration, FileInfo.c_str(), Commentaire.c_str());
    // H.2.2. Le fichier de  maillage obtenu
      if ( modeHOMARD == 1 )
      {
        std::stringstream saux0 ;
        Commentaire = "Mesh" ;
        Commentaire += " " + siterp1 ;
        PublishFileUnderIteration(NomIteration, MeshFile, Commentaire.c_str());
        if ( Option2 % 2 == 0 ) { PublishResultInSmesh(MeshFile, 1); }
      }
    }
  // H.3 Message d'erreur
    if (codretexec != 0)
    {
      std::string text = "" ;
      // Message d'erreur en cas de probleme en adaptation
      if ( modeHOMARD == 1 )
      {
        text = "Error during the adaptation.\n" ;
        bool stopvu = false ;
        std::ifstream fichier( LogFile.c_str() );
        if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
        {
          std::string ligne; // variable contenant chaque ligne lue
          while ( std::getline( fichier, ligne ) )
          {
//             INFOS(ligne);
            if ( stopvu )
            { text += ligne+ "\n"; }
            else
            {
              int position = ligne.find( "===== HOMARD ===== STOP =====" ) ;
              if ( position > 0 ) { stopvu = true ; }
            }
          }
        }
      }
      text += "\n\nSee the file " + LogFile + "\n" ;
      INFOS ( text ) ;
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
//
      // En mode information, on force le succes pour pouvoir consulter le fichier log
      if ( modeHOMARD != 1 ) { codretexec = 0 ; }
    }
  }

  // I. Menage et retour dans le répertoire du cas
  if (codret == 0)
  {
    delete myDriver;
    MESSAGE ( ". On retourne dans nomDirWork = " << nomDirWork );

    CHDIR(nomDirWork.c_str());
  }

  // J. Suivi de la frontière CAO
//   std::cout << "- codret : " << codret << std::endl;
//   std::cout << "- modeHOMARD : " << modeHOMARD << std::endl;
//   std::cout << "- BoundaryOption : " << BoundaryOption << std::endl;
//   std::cout << "- codretexec : " << codretexec << std::endl;
  if (codret == 0)
  {
    if ( ( modeHOMARD == 1 ) && ( BoundaryOption % 5 == 0 ) && (codretexec == 0) )
    {
      INFOS ( "Suivi de frontière CAO" );
      codret = ComputeCAO(myCase, myIteration, Option1, Option2) ;
    }
  }

  return codretexec ;
}
//=============================================================================
// Calcul d'une iteration : partie spécifique à l'adaptation
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeAdap(HOMARD::HOMARD_Cas_var myCase, HOMARD::HOMARD_Iteration_var myIteration, CORBA::Long etatMenage, HomardDriver* myDriver, CORBA::Long Option1, CORBA::Long Option2)
{
  MESSAGE ( "ComputeAdap avec Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  // A.1. Bases
  int codret = 0;
  // Numero de l'iteration
  int NumeIter = myIteration->GetNumber();
  std::stringstream saux0 ;
  saux0 << NumeIter-1 ;
  std::string siter = saux0.str() ;
  if (NumeIter < 11) { siter = "0" + siter ; }

  // A.2. On verifie qu il y a une hypothese (erreur improbable);
  const char* nomHypo = myIteration->GetHypoName();
  if (std::string(nomHypo) == std::string(""))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "This iteration does not have any associated hypothesis.";
      throw SALOME::SALOME_Exception(es);
      return 2;
  };
  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  ASSERT(!CORBA::is_nil(myHypo));

  // B. L'iteration parent
  const char* nomIterationParent = myIteration->GetIterParentName();
  HOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterationParent];
  ASSERT(!CORBA::is_nil(myIterationParent));
  // Si l'iteration parent n'est pas calculee, on le fait (recursivite amont)
  if ( myIterationParent->GetState() == 1 )
  {
    int iaux = 1 ;
    int codret = Compute(nomIterationParent, etatMenage, iaux, Option1, Option2);
    if (codret != 0)
    {
      // GERALD -- QMESSAGE BOX
      VERIFICATION("Pb au calcul de l'iteration precedente" == 0);
    }
  };

  // C. Le sous-répertoire de l'iteration precedente
  char* DirComputePa = ComputeDirPaManagement(myCase, myIteration);
  MESSAGE( ". DirComputePa = " << DirComputePa );

  // D. Les données de l'adaptation HOMARD
  // D.1. Le type de conformite
  int ConfType = myCase->GetConfType();
  MESSAGE ( ". ConfType = " << ConfType );

  // D.1. Le type externe
  int ExtType = myCase->GetExtType();
  MESSAGE ( ". ExtType = " << ExtType );

  // D.3. Le maillage de depart
  const char* NomMeshParent = myIterationParent->GetMeshName();
  MESSAGE ( ". NomMeshParent = " << NomMeshParent );
  const char* MeshFileParent = myIterationParent->GetMeshFile();
  MESSAGE ( ". MeshFileParent = " << MeshFileParent );

  // D.4. Le maillage associe a l'iteration
  const char* MeshFile = myIteration->GetMeshFile();
  MESSAGE ( ". MeshFile = " << MeshFile );
  FILE *file = fopen(MeshFile,"r");
  if (file != NULL)
  {
    fclose(file);
    if (etatMenage == 0)
    {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      std::string text = "MeshFile : " + std::string(MeshFile) + " already exists ";
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);
      return 4;
    }
    else
    {
      std::string commande = "rm -f " + std::string(MeshFile);
      codret = system(commande.c_str());
      if (codret != 0)
      {
        SALOME::ExceptionStruct es;
        es.type = SALOME::BAD_PARAM;
        es.text = "The mesh file cannot be deleted.";
        throw SALOME::SALOME_Exception(es);
        return 5;
      }
    }
  }

  // D.5. Les types de raffinement et de deraffinement
  // Les appels corba sont lourds, il vaut mieux les grouper
  HOMARD::listeTypes* ListTypes = myHypo->GetAdapRefinUnRef();
  ASSERT(ListTypes->length() == 3);
  int TypeAdap = (*ListTypes)[0];
  int TypeRaff = (*ListTypes)[1];
  int TypeDera = (*ListTypes)[2];
//   MESSAGE ( ". TypeAdap = " << TypeAdap << ", TypeRaff = " << TypeRaff << ", TypeDera = " << TypeDera  );

  // E. Texte du fichier de configuration
  // E.1. Incontournables du texte
  myDriver->TexteAdap(ExtType);
  int iaux = 0 ;
  myDriver->TexteMaillageHOMARD( DirComputePa, siter, iaux ) ;
  myDriver->TexteMaillage(NomMeshParent, MeshFileParent, 0);
  myDriver->TexteConfRaffDera(ConfType, TypeAdap, TypeRaff, TypeDera);

  // E.2. Ajout des informations liees aux zones eventuelles
  if ( TypeAdap == 0 )
  { DriverTexteZone(myHypo, myDriver) ; }

  // E.3. Ajout des informations liees aux champs eventuels
  if ( TypeAdap == 1 )
  { DriverTexteField(myIteration, myHypo, myDriver) ; }

  // E.4. Ajout des informations liees au filtrage eventuel par les groupes
  HOMARD::ListGroupType* listeGroupes = myHypo->GetGroups();
  int numberOfGroups = listeGroupes->length();
  MESSAGE( ". Filtrage par " << numberOfGroups << " groupes");
  if (numberOfGroups > 0)
  {
    for (int NumGroup = 0; NumGroup< numberOfGroups; NumGroup++)
    {
      std::string GroupName = std::string((*listeGroupes)[NumGroup]);
      MESSAGE( "... GroupName = " << GroupName );
      myDriver->TexteGroup(GroupName);
    }
  }

  // E.5. Ajout des informations liees a l'eventuelle interpolation des champs
  DriverTexteFieldInterp(myIteration, myHypo, myDriver) ;

  // E.6. Ajout des options avancees
  int Pyram = myCase->GetPyram();
  MESSAGE ( ". Pyram = " << Pyram );
  int NivMax = myHypo->GetNivMax();
  MESSAGE ( ". NivMax = " << NivMax );
  double DiamMin = myHypo->GetDiamMin() ;
  MESSAGE ( ". DiamMin = " << DiamMin );
  int AdapInit = myHypo->GetAdapInit();
  MESSAGE ( ". AdapInit = " << AdapInit );
  int ExtraOutput = myHypo->GetExtraOutput();
  MESSAGE ( ". ExtraOutput = " << ExtraOutput );
  myDriver->TexteAdvanced(Pyram, NivMax, DiamMin, AdapInit, ExtraOutput);

  // E.7. Ajout des informations sur le deroulement de l'execution
  int MessInfo = myIteration->GetInfoCompute();
  MESSAGE ( ". MessInfo = " << MessInfo );
  myDriver->TexteInfoCompute(MessInfo);

  return codret ;
}
//=============================================================================
// Calcul d'une iteration : partie spécifique au suivi de frontière CAO
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeCAO(HOMARD::HOMARD_Cas_var myCase, HOMARD::HOMARD_Iteration_var myIteration, CORBA::Long Option1, CORBA::Long Option2)
{
  MESSAGE ( "ComputeCAO avec Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  // A.1. Bases
  int codret = 0;
  // A.2. Le sous-répertoire de l'iteration en cours de traitement
  char* DirCompute = myIteration->GetDirName();
  // A.3. Le maillage résultat de l'iteration en cours de traitement
  char* MeshFile = myIteration->GetMeshFile();

  // B. Les données pour FrontTrack
  // B.1. Le maillage à modifier
  const std::string theInputMedFile = MeshFile;
  MESSAGE ( ". theInputMedFile  = " << theInputMedFile );

  // B.2. Le maillage après modification : fichier identique
  const std::string theOutputMedFile = MeshFile ;
  MESSAGE ( ". theOutputMedFile = " << theInputMedFile );

  // B.3. La liste des fichiers contenant les numéros des noeuds à bouger
  std::vector< std::string > theInputNodeFiles ;
  MESSAGE ( ". DirCompute = " << DirCompute );
  int bilan ;
  int icpt = 0 ;
#ifndef WIN32
  DIR *dp;
  struct dirent *dirp;
  dp  = opendir(DirCompute);
  while ( (dirp = readdir(dp)) != NULL )
  {
    std::string file_name(dirp->d_name);
    bilan = file_name.find("fr") ;
    if ( bilan != string::npos )
    {
      std::stringstream filename_total ;
      filename_total << DirCompute << "/" << file_name ;
      theInputNodeFiles.push_back(filename_total.str()) ;
      icpt += 1 ;
    }
  }
#else
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  hFind = FindFirstFile(DirNameStart, &ffd);
  if (INVALID_HANDLE_VALUE != hFind) {
    while (FindNextFile(hFind, &ffd) != 0) {
      std::string file_name(ffd.cFileName);
      bilan = file_name.find("fr") ;
      if ( bilan != string::npos )
      {
        std::stringstream filename_total ;
        filename_total << DirCompute << "/" << file_name ;
        theInputNodeFiles.push_back(filename_total.str()) ;
        icpt += 1 ;
      }
    }
    FindClose(hFind);
  }
#endif
  for ( int i = 0; i < icpt; i++ )
  { MESSAGE ( ". theInputNodeFiles["<< i << "] = " << theInputNodeFiles[i] ); }

  // B.4. Le fichier de la CAO
  HOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  std::string BoundaryName = std::string((*ListBoundaryGroupType)[0]);
  MESSAGE ( ". BoundaryName = " << BoundaryName );
  HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  const std::string theXaoFileName = myBoundary->GetDataFile();
  MESSAGE ( ". theXaoFileName = " << theXaoFileName );

  // B.5. Parallélisme
  bool theIsParallel = false;

  // C. Lancement des projections
  MESSAGE ( ". Lancement des projections" );
  FrontTrack* myFrontTrack = new FrontTrack();
  myFrontTrack->track(theInputMedFile, theOutputMedFile, theInputNodeFiles, theXaoFileName, theIsParallel);

  // D. Transfert des coordonnées modifiées dans le fichier historique de HOMARD
  //    On lance une exécution spéciale de HOMARD en attendant de savoir le faire avec MEDCoupling
  MESSAGE ( ". Transfert des coordonnées" );
  codret = ComputeCAObis(myIteration, Option1, Option2) ;

  return codret ;
}
//=============================================================================
//=============================================================================
// Transfert des coordonnées en suivi de frontière CAO
// Option1 >0 : appel depuis python
//         <0 : appel depuis GUI
// Option2 : multiple de nombres premiers
//         1 : aucune option
//        x2 : publication du maillage dans SMESH
//=============================================================================
CORBA::Long HOMARD_Gen_i::ComputeCAObis(HOMARD::HOMARD_Iteration_var myIteration, CORBA::Long Option1, CORBA::Long Option2)
{
  MESSAGE ( "ComputeCAObis, avec Option1 = " << Option1 << ", Option2 = " << Option2 );

  // A. Prealable
  int codret = 0;

  // A.1. Controle de la possibilite d'agir
  // A.1.1. Etat de l'iteration
  int etat = myIteration->GetState();
  MESSAGE ( "etat = "<<etat );
  // A.1.2. L'iteration doit être calculee
  if ( etat == 1 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This iteration is not computed.";
    throw SALOME::SALOME_Exception(es);
    return 1 ;
  }
  // A.2. Numero de l'iteration
  //     siterp1 : numero de l'iteration a traiter
  int NumeIter = myIteration->GetNumber();
  std::string siterp1 ;
  std::stringstream saux1 ;
  saux1 << NumeIter ;
  siterp1 = saux1.str() ;
  if (NumeIter < 10) { siterp1 = "0" + siterp1 ; }
  MESSAGE ( "siterp1 = "<<siterp1 );

  // A.3. Le cas
  const char* CaseName = myIteration->GetCaseName();
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[CaseName];
  ASSERT(!CORBA::is_nil(myCase));

  // A.4. Le sous-répertoire de l'iteration a traiter
  char* DirCompute = myIteration->GetDirName();
  MESSAGE( ". DirCompute = " << DirCompute );

  // C. Le fichier des messages
  std::string LogFile = DirCompute ;
  LogFile += "/Liste." + siterp1 + ".maj_coords.log" ;
  MESSAGE (". LogFile = " << LogFile);
  myIteration->SetFileInfo(LogFile.c_str());

   // D. On passe dans le répertoire de l'iteration a calculer
  MESSAGE ( ". On passe dans DirCompute = " << DirCompute );
  CHDIR(DirCompute);

  // E. Les données de l'exécution HOMARD
  // E.1. L'objet du texte du fichier de configuration
  HomardDriver* myDriver = new HomardDriver("", siterp1);
  myDriver->TexteInit(DirCompute, LogFile, _Langue);

  // E.2. Le maillage associe a l'iteration
  const char* NomMesh = myIteration->GetMeshName();
  MESSAGE ( ". NomMesh = " << NomMesh );
  const char* MeshFile = myIteration->GetMeshFile();
  MESSAGE ( ". MeshFile = " << MeshFile );

  // E.3. Les données du traitement HOMARD
  int iaux ;
  myDriver->TexteMajCoords( NumeIter ) ;
  iaux = 0 ;
  myDriver->TexteMaillageHOMARD( DirCompute, siterp1, iaux ) ;
  myDriver->TexteMaillage(NomMesh, MeshFile, 0);
//
  // E.4. Ecriture du texte dans le fichier
  MESSAGE ( ". Ecriture du texte dans le fichier de configuration ; codret = "<<codret );
  if (codret == 0)
  { myDriver->CreeFichier(); }

// F. Execution
//
  int codretexec = 1789 ;
  if (codret == 0)
  {
    codretexec = myDriver->ExecuteHomard(Option1);
    MESSAGE ( "Erreur en executant HOMARD : " << codretexec );
  }

  // G. Gestion des resultats
  if (codret == 0)
  {
    // G.1. Le fichier des messages, dans tous les cas
    const char* NomIteration = myIteration->GetName();
    std::string Commentaire = "logmaj_coords" ;
    PublishFileUnderIteration(NomIteration, LogFile.c_str(), Commentaire.c_str());
    // G.2 Message d'erreur
    if (codretexec != 0)
    {
      std::string text = "\n\nSee the file " + LogFile + "\n" ;
      INFOS ( text ) ;
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = CORBA::string_dup(text.c_str());
      throw SALOME::SALOME_Exception(es);

      // On force le succes pour pouvoir consulter le fichier log
      codretexec = 0 ;
    }
  }

  // H. Menage et retour dans le répertoire du cas
  if (codret == 0) { delete myDriver; }

  return codret ;
}
//=============================================================================
// Creation d'un nom de sous-répertoire pour l'iteration au sein d'un répertoire parent
//  nomrep : nom du répertoire parent
//  num : le nom du sous-répertoire est sous la forme 'In', n est >= num
//=============================================================================
char* HOMARD_Gen_i::CreateDirNameIter(const char* nomrep, CORBA::Long num )
{
  MESSAGE ( "CreateDirNameIter : nomrep ="<< nomrep << ", num = "<<num);
  // On verifie que le répertoire parent existe
  int codret = CHDIR(nomrep) ;
  if ( codret != 0 )
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "The directory of the case does not exist.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  std::string nomDirActuel = getenv("PWD") ;
  std::string DirName ;
  // On boucle sur tous les noms possibles jusqu'a trouver un nom correspondant a un répertoire inconnu
  bool a_chercher = true ;
  while ( a_chercher )
  {
    // On passe dans le répertoire parent

    CHDIR(nomrep);
    // On recherche un nom sous la forme Iabc, avec abc representant le numero
    int jaux ;
    if      ( num <    100 ) { jaux = 2 ; }
    else if ( num <   1000 ) { jaux = 3 ; }
    else if ( num <  10000 ) { jaux = 4 ; }
    else if ( num < 100000 ) { jaux = 5 ; }
    else                     { jaux = 9 ; }
    std::ostringstream iaux ;
    iaux << std::setw(jaux) << std::setfill('0') << num ;
    std::ostringstream DirNameA ;
    DirNameA << "I" << iaux.str();
    // Si on ne pas peut entrer dans le répertoire, on doit verifier
    // que c'est bien un probleme d'absence
    if ( CHDIR(DirNameA.str().c_str()) != 0 )
    {
      bool existe = false ;
#ifndef WIN32
      DIR *dp;
      struct dirent *dirp;
      dp  = opendir(nomrep);
      while ( (dirp = readdir(dp)) != NULL )
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
        if ( file_name == DirNameA.str() ) { existe = true ; }
      }
#ifndef WIN32
      closedir(dp);
#else
      FindClose(hFind);
#endif
      if ( !existe )
      {
        DirName = DirNameA.str() ;
        a_chercher = false ;
        break ;
      }
    }
    num += 1;
  }

  MESSAGE ( "==> DirName = " << DirName);
  MESSAGE ( ". On retourne dans nomDirActuel = " << nomDirActuel );
  CHDIR(nomDirActuel.c_str());
  return CORBA::string_dup( DirName.c_str() );
}
//=============================================================================
// Calcul d'une iteration : gestion du répertoire de calcul
//        Si le sous-répertoire existe :
//         etatMenage =  0 : on sort en erreur si le répertoire n'est pas vide
//         etatMenage =  1 : on fait le menage du répertoire
//         etatMenage = -1 : on ne fait rien
//=============================================================================
char* HOMARD_Gen_i::ComputeDirManagement(HOMARD::HOMARD_Cas_var myCase, HOMARD::HOMARD_Iteration_var myIteration, CORBA::Long etatMenage)
{
  MESSAGE ( "ComputeDirManagement : répertoires pour le calcul" );
  // B.2. Le répertoire du cas
  const char* nomDirCase = myCase->GetDirName();
  MESSAGE ( ". nomDirCase = " << nomDirCase );

  // B.3. Le sous-répertoire de l'iteration a calculer, puis le répertoire complet a creer
  // B.3.1. Le nom du sous-répertoire
  const char* nomDirIt = myIteration->GetDirNameLoc();

  // B.3.2. Le nom complet du sous-répertoire
  std::stringstream DirCompute ;
  DirCompute << nomDirCase << "/" << nomDirIt;
  MESSAGE (". DirCompute = " << DirCompute.str() );

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
    if (etatMenage == 1)
    {
      MESSAGE (". Menage du répertoire DirCompute = " << DirCompute.str());
      std::string commande = "rm -rf " + DirCompute.str()+"/*" ;
      int codret = system(commande.c_str());
      if (codret != 0)
      {
        // GERALD -- QMESSAGE BOX
        std::cerr << ". Menage du répertoire de calcul" << DirCompute.str() << std::endl;
        VERIFICATION("Pb au menage du répertoire de calcul" == 0);
      }
    }
//  On n'a pas demande de faire le menage de son contenu : on sort en erreur :
    else
    {
      if (etatMenage == 0)
      {
#ifndef WIN32
        DIR *dp;
        struct dirent *dirp;
        dp  = opendir(DirCompute.str().c_str());
        bool result = true;
        while ((dirp = readdir(dp)) != NULL && result )
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
        if ( result == false)
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

  return CORBA::string_dup( DirCompute.str().c_str() );
}
//=============================================================================
// Calcul d'une iteration : gestion du répertoire de calcul de l'iteration parent
//=============================================================================
char* HOMARD_Gen_i::ComputeDirPaManagement(HOMARD::HOMARD_Cas_var myCase, HOMARD::HOMARD_Iteration_var myIteration)
{
  MESSAGE ( "ComputeDirPaManagement : répertoires pour le calcul" );
  // Le répertoire du cas
  const char* nomDirCase = myCase->GetDirName();
  MESSAGE ( ". nomDirCase = " << nomDirCase );

  // Le sous-répertoire de l'iteration precedente

  const char* nomIterationParent = myIteration->GetIterParentName();
  HOMARD::HOMARD_Iteration_var myIterationParent = myStudyContext._mesIterations[nomIterationParent];
  const char* nomDirItPa = myIterationParent->GetDirNameLoc();
  std::stringstream DirComputePa ;
  DirComputePa << nomDirCase << "/" << nomDirItPa;
  MESSAGE( ". nomDirItPa = " << nomDirItPa);
  MESSAGE( ". DirComputePa = " << DirComputePa.str() );

  return CORBA::string_dup( DirComputePa.str().c_str() );
}
//=============================================================================
// Calcul d'une iteration : ecriture des zones dans le fichier de configuration
//=============================================================================
void HOMARD_Gen_i::DriverTexteZone(HOMARD::HOMARD_Hypothesis_var myHypo, HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteZone" );
  HOMARD::listeZonesHypo* ListZone = myHypo->GetZones();
  int numberOfZonesx2 = ListZone->length();
  int NumZone ;

  for (int iaux = 0; iaux< numberOfZonesx2; iaux++)
  {
    std::string ZoneName = std::string((*ListZone)[iaux]);
    MESSAGE ( "... ZoneName = " << ZoneName);
    HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[ZoneName];
    ASSERT(!CORBA::is_nil(myZone));

    int ZoneType = myZone->GetType();
    std::string TypeUsestr = std::string((*ListZone)[iaux+1]);
    int TypeUse = atoi( TypeUsestr.c_str() );
    MESSAGE ( "... ZoneType = " << ZoneType << ", TypeUse = "<<TypeUse);
    NumZone = iaux/2 + 1 ;
    HOMARD::double_array* zone = myZone->GetCoords();
    if ( ZoneType == 2 || ( ZoneType>=11 && ZoneType <=13 ) ) // Cas d un parallelepipede ou d'un rectangle
    { myDriver->TexteZone(NumZone, ZoneType, TypeUse, (*zone)[0], (*zone)[1], (*zone)[2], (*zone)[3], (*zone)[4], (*zone)[5], 0., 0., 0.); }
    else if ( ZoneType == 4 ) // Cas d une sphere
    { myDriver->TexteZone(NumZone, ZoneType, TypeUse, (*zone)[0], (*zone)[1], (*zone)[2], (*zone)[3], 0., 0., 0., 0., 0.); }
    else if ( ZoneType == 5 || ( ZoneType>=31 && ZoneType <=33 ) ) // Cas d un cylindre ou d'un disque
    { myDriver->TexteZone(NumZone, ZoneType, TypeUse, (*zone)[0], (*zone)[1], (*zone)[2], (*zone)[3], (*zone)[4], (*zone)[5], (*zone)[6], (*zone)[7], 0.); }
    else if ( ZoneType == 7 || ( ZoneType>=61 && ZoneType <=63 ) ) // Cas d un tuyau ou disque perce
    { myDriver->TexteZone(NumZone, ZoneType, TypeUse, (*zone)[0], (*zone)[1], (*zone)[2], (*zone)[3], (*zone)[4], (*zone)[5], (*zone)[6], (*zone)[7], (*zone)[8]); }
    else { VERIFICATION("ZoneType est incorrect." == 0) ; }
    iaux += 1 ;
  }
  return ;
}
//=============================================================================
// Calcul d'une iteration : ecriture des champs dans le fichier de configuration
//=============================================================================
void HOMARD_Gen_i::DriverTexteField(HOMARD::HOMARD_Iteration_var myIteration, HOMARD::HOMARD_Hypothesis_var myHypo, HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteField" );
//  Le fichier du champ
  char* FieldFile = myIteration->GetFieldFile();
  MESSAGE ( ". FieldFile = " << FieldFile );
  if (strlen(FieldFile) == 0)
  {
    // GERALD -- QMESSAGE BOX
    std::cerr << "Le fichier du champ n'a pas ete fourni." << std::endl;
    VERIFICATION("The file for the field is not given." == 0);
  }
//  Les caracteristiques d'instants du champ de pilotage
  int TimeStep = myIteration->GetTimeStep();
  MESSAGE( ". TimeStep = " << TimeStep );
  int Rank = myIteration->GetRank();
  MESSAGE( ". Rank = " << Rank );
//  Les informations sur les champs
  HOMARD::InfosHypo* aInfosHypo = myHypo->GetField();
//  Le nom
  const char* FieldName = aInfosHypo->FieldName;
//  Les seuils
  int TypeThR = aInfosHypo->TypeThR;
  double ThreshR = aInfosHypo->ThreshR;
  int TypeThC = aInfosHypo->TypeThC;
  double ThreshC = aInfosHypo->ThreshC;
//  Saut entre mailles ou non ?
  int UsField = aInfosHypo->UsField;
  MESSAGE( ". UsField = " << UsField );
//  L'usage des composantes
  int UsCmpI = aInfosHypo->UsCmpI;
  MESSAGE( ". UsCmpI = " << UsCmpI );
//
  myDriver->TexteField(FieldName, FieldFile, TimeStep, Rank, TypeThR, ThreshR, TypeThC, ThreshC, UsField, UsCmpI);
//
//  Les composantes
  HOMARD::listeComposantsHypo* mescompo = myHypo->GetComps();
  int numberOfCompos = mescompo->length();
  MESSAGE( ". numberOfCompos = " << numberOfCompos );
  for (int NumeComp = 0; NumeComp< numberOfCompos; NumeComp++)
  {
    std::string nomCompo = std::string((*mescompo)[NumeComp]);
    MESSAGE( "... nomCompo = " << nomCompo );
    myDriver->TexteCompo(NumeComp, nomCompo);
  }
  return ;
}
//=============================================================================
// Calcul d'une iteration : ecriture des frontieres dans le fichier de configuration
// On ecrit dans l'ordre :
//    1. la definition des frontieres
//    2. les liens avec les groupes
//    3. un entier resumant le type de comportement pour les frontieres
//=============================================================================
int HOMARD_Gen_i::DriverTexteBoundary(HOMARD::HOMARD_Cas_var myCase, HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteBoundary" );
  // 1. Recuperation des frontieres
  std::list<std::string>  ListeBoundaryTraitees ;
  HOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  int numberOfitems = ListBoundaryGroupType->length();
  MESSAGE ( "... number of string for Boundary+Group = " << numberOfitems);
  int BoundaryOption = 1 ;
  // 2. Parcours des frontieres pour ecrire leur description
  int NumBoundaryAnalytical = 0 ;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ( "... BoundaryName = " << BoundaryName);
    // 2.1. La frontiere a-t-elle deja ete ecrite ?
    //      Cela arrive quand elle est liee a plusieurs groupes. Il ne faut l'ecrire que la premiere fois
    int A_faire = 1 ;
    std::list<std::string>::const_iterator it = ListeBoundaryTraitees.begin();
    while (it != ListeBoundaryTraitees.end())
    {
      MESSAGE ( "..... BoundaryNameTraitee = " << *it);
      if ( BoundaryName == *it ) { A_faire = 0 ; }
      it++;
    }
    // 2.2. Ecriture de la frontiere
    if ( A_faire == 1 )
    {
      // 2.2.1. Caracteristiques de la frontiere
      HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
      ASSERT(!CORBA::is_nil(myBoundary));
      int BoundaryType = myBoundary->GetType();
      MESSAGE ( "... BoundaryType = " << BoundaryType );
      // 2.2.2. Ecriture selon le type
      // 2.2.2.1. Cas d une frontiere CAO
      if (BoundaryType == -1)
      {
//         const char* CAOFile = myBoundary->GetDataFile() ;
//         MESSAGE ( ". CAOFile = " << CAOFile );
        if ( BoundaryOption % 5 != 0 ) { BoundaryOption = BoundaryOption*5 ; }
      }
      // 2.2.2.2. Cas d une frontiere discrete
      else if (BoundaryType == 0)
      {
        const char* MeshName = myBoundary->GetMeshName() ;
        MESSAGE ( ". MeshName = " << MeshName );
        const char* MeshFile = myBoundary->GetDataFile() ;
        MESSAGE ( ". MeshFile = " << MeshFile );
        myDriver->TexteBoundaryDi( MeshName, MeshFile);
        if ( BoundaryOption % 2 != 0 ) { BoundaryOption = BoundaryOption*2 ; }
      }
      // 2.2.2.3. Cas d une frontiere analytique
      else
      {
        NumBoundaryAnalytical++ ;
        HOMARD::double_array* coor = myBoundary->GetCoords();
        if (BoundaryType == 1) // Cas d un cylindre
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], 0.);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3 ; }
        }
        else if (BoundaryType == 2) // Cas d une sphere
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], 0., 0., 0., 0.);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3 ; }
        }
        else if (BoundaryType == 3) // Cas d un cone defini par un axe et un angle
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], 0.);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3 ; }
        }
        else if (BoundaryType == 4) // Cas d un cone defini par les 2 rayons
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], (*coor)[7]);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3 ; }
        }
        else if (BoundaryType == 5) // Cas d un tore
        {
          myDriver->TexteBoundaryAn(BoundaryName, NumBoundaryAnalytical, BoundaryType, (*coor)[0], (*coor)[1], (*coor)[2], (*coor)[3], (*coor)[4], (*coor)[5], (*coor)[6], (*coor)[7]);
          if ( BoundaryOption % 3 != 0 ) { BoundaryOption = BoundaryOption*3 ; }
        }
      }
      // 2.2.3. Memorisation du traitement
      ListeBoundaryTraitees.push_back( BoundaryName );
    }
  }
  // 3. Parcours des frontieres pour ecrire les liens avec les groupes
  NumBoundaryAnalytical = 0 ;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ( "... BoundaryName = " << BoundaryName);
    HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
    ASSERT(!CORBA::is_nil(myBoundary));
    int BoundaryType = myBoundary->GetType();
    MESSAGE ( "... BoundaryType = " << BoundaryType );
    // 3.1. Recuperation du nom du groupe
    std::string GroupName = std::string((*ListBoundaryGroupType)[NumBoundary+1]);
    MESSAGE ( "... GroupName = " << GroupName);
    // 3.2. Cas d une frontiere CAO
    if ( BoundaryType == -1 )
    {
      if ( GroupName.size() > 0 ) { myDriver->TexteBoundaryCAOGr ( GroupName ) ; }
    }
    // 3.3. Cas d une frontiere discrete
    else if ( BoundaryType == 0 )
    {
      if ( GroupName.size() > 0 ) { myDriver->TexteBoundaryDiGr ( GroupName ) ; }
    }
    // 3.4. Cas d une frontiere analytique
    else
    {
      NumBoundaryAnalytical++ ;
      myDriver->TexteBoundaryAnGr ( BoundaryName, NumBoundaryAnalytical, GroupName ) ;
    }
  }
  // 4. Ecriture de l'option finale
  myDriver->TexteBoundaryOption(BoundaryOption);
//
  return BoundaryOption ;
}
//=============================================================================
// Calcul d'une iteration : ecriture des interpolations dans le fichier de configuration
//=============================================================================
void HOMARD_Gen_i::DriverTexteFieldInterp(HOMARD::HOMARD_Iteration_var myIteration, HOMARD::HOMARD_Hypothesis_var myHypo, HomardDriver* myDriver)
{
  MESSAGE ( "... DriverTexteFieldInterp" );
  int TypeFieldInterp = myHypo->GetTypeFieldInterp();
  MESSAGE ( "... TypeFieldInterp = " << TypeFieldInterp);
  if (TypeFieldInterp != 0)
  {
//  Le fichier des champs
    char* FieldFile = myIteration->GetFieldFile();
    MESSAGE ( ". FieldFile = " << FieldFile );
    if (strlen(FieldFile) == 0)
    {
      // GERALD -- QMESSAGE BOX
      VERIFICATION("The file for the field is not given." == 0);
    }
  //
    const char* MeshFile = myIteration->GetMeshFile();
    myDriver->TexteFieldInterp(FieldFile, MeshFile);

  // Les champs
  // Interpolation de tous les champs
    if ( TypeFieldInterp == 1 )
    {
      myDriver->TexteFieldInterpAll();
    }
  // Interpolation de certains champs
    else if (TypeFieldInterp == 2)
    {
      // Les champs et leurs instants pour l'iteration
      HOMARD::listeFieldInterpTSRsIter* ListFieldTSR = myIteration->GetFieldInterpsTimeStepRank();
      int numberOfFieldsx3 = ListFieldTSR->length();
      MESSAGE( ". pour iteration, numberOfFields = " << numberOfFieldsx3/3 );
      // Les champs pour l'hypothese
      HOMARD::listeFieldInterpsHypo* ListField = myHypo->GetFieldInterps();
      int numberOfFieldsx2 = ListField->length();
      MESSAGE( ". pour hypothese, numberOfFields = " << numberOfFieldsx2/2 );
      // On parcourt tous les champs de  l'hypothese
      int NumField = 0 ;
      for (int iaux = 0; iaux< numberOfFieldsx2; iaux++)
      {
        // Le nom du champ
        std::string FieldName = std::string((*ListField)[iaux]) ;
        // Le type d'interpolation
        std::string TypeInterpstr = std::string((*ListField)[iaux+1]) ;
        MESSAGE( "... FieldName = " << FieldName << ", TypeInterp = " << TypeInterpstr );
        // On cherche ?? savoir si des instants ont ??t?? pr??cis??s pour cette it??ration
        int tsrvu = 0;
        for (int jaux = 0; jaux< numberOfFieldsx3; jaux++)
        {
        // Le nom du champ
          std::string FieldName2 = std::string((*ListFieldTSR)[jaux]) ;
          MESSAGE( "..... FieldName2 = " << FieldName2 );
        // Quand c'est le bon champ, on ecrit le pas de temps
          if ( FieldName == FieldName2 )
          {
            tsrvu = 1 ;
            // Le pas de temps
            std::string TimeStepstr = std::string((*ListFieldTSR)[jaux+1]) ;
            // Le numero d'ordre
            std::string Rankstr = std::string((*ListFieldTSR)[jaux+2]) ;
            MESSAGE( "..... TimeStepstr = " << TimeStepstr <<", Rankstr = "<<Rankstr );
            NumField += 1 ;
            int TimeStep = atoi( TimeStepstr.c_str() );
            int Rank = atoi( Rankstr.c_str() );
            myDriver->TexteFieldInterpNameType(NumField, FieldName, TypeInterpstr, TimeStep, Rank);
          }
          jaux += 2 ;
        }
        // Si aucun instant n'a été défini
        if ( tsrvu == 0 )
        {
          NumField += 1 ;
          myDriver->TexteFieldInterpNameType(NumField, FieldName, TypeInterpstr, -1, -1);
        }
        iaux++ ;
      }
    }
  }
  return ;
}
//===========================================================================
//===========================================================================


//===========================================================================
//===========================================================================
// Publications
//===========================================================================
//===========================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PublishInStudy(SALOMEDS::SObject_ptr theSObject,
                                                   CORBA::Object_ptr theObject,
                                                   const char* theName)
{
  MESSAGE("PublishInStudy pour " << theName);
  SALOMEDS::SObject_var aResultSO;
  if (CORBA::is_nil(myStudy))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };

// Recuperation de l'objet correspondant, en essayant chacun des types possibles
// Rq : Iteration est publiee ailleurs
  HOMARD::HOMARD_Boundary_var   aBoundary = HOMARD::HOMARD_Boundary::_narrow(theObject);
  HOMARD::HOMARD_Cas_var        aCase  = HOMARD::HOMARD_Cas::_narrow(theObject);
  HOMARD::HOMARD_Hypothesis_var aHypo = HOMARD::HOMARD_Hypothesis::_narrow(theObject);
  HOMARD::HOMARD_YACS_var       aYACS = HOMARD::HOMARD_YACS::_narrow(theObject);
  HOMARD::HOMARD_Zone_var       aZone = HOMARD::HOMARD_Zone::_narrow(theObject);

  UpdateStudy();

// Controle de la non publication d'un objet de meme nom
   if ( (!aBoundary->_is_nil()) || (!aHypo->_is_nil()) || (!aYACS->_is_nil()) || (!aZone->_is_nil()) )
  {
    SALOMEDS::Study::ListOfSObject_var listSO = myStudy->FindObjectByName(theName, ComponentDataType());
    if (listSO->length() >= 1)
    {
      MESSAGE("This name "<<theName<<" is already used "<<listSO->length()<<" time(s)");
      aResultSO = listSO[0];
      return aResultSO._retn();
    }
  }

  // Caracteristiques de l'etude
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  aStudyBuilder->NewCommand();
  if(!aBoundary->_is_nil())
    aResultSO = PublishBoundaryInStudy(aStudyBuilder, aBoundary, theName);
  else if(!aCase->_is_nil())
    aResultSO = PublishCaseInStudy(aStudyBuilder, aCase, theName);
  else if(!aHypo->_is_nil())
    aResultSO = PublishHypotheseInStudy(aStudyBuilder, aHypo, theName);
  else if(!aYACS->_is_nil())
    aResultSO = PublishYACSInStudy(aStudyBuilder, aYACS, theName);
  else if(!aZone->_is_nil())
    aResultSO = PublishZoneInStudy(aStudyBuilder, aZone, theName);

  aStudyBuilder->CommitCommand();

  return aResultSO._retn();
};
//=============================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PublishBoundaryInStudy( SALOMEDS::StudyBuilder_var aStudyBuilder,
                   HOMARD::HOMARD_Boundary_ptr theObject, const char* theName)
{
  MESSAGE("PublishBoundaryStudy pour "<<theName);
  SALOMEDS::SObject_var aResultSO;

  // Caracteristique de la Boundary
  HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[theName];

  // On recupere le module pere dans l etude
  SALOMEDS::SComponent_var       theFatherHomard = myStudy->FindComponent(ComponentDataType());
  if (theFatherHomard->_is_nil())
  {
    MESSAGE("theFatherHomard->_is_nil()");
    return aResultSO._retn();
  }

  // On ajoute la categorie des boundarys dans l etude si necessaire
  if ( _tag_boun == 0 )
  {
    _tag_gene += 1 ;
    _tag_boun = _tag_gene ;
  }
  MESSAGE("PublishBoundaryInStudy _tag_gene = "<<_tag_gene << ", _tag_boun = "<<_tag_boun );
  SALOMEDS::SObject_var aSObject;
  if (!theFatherHomard->FindSubObject(_tag_boun, aSObject))
  {
    MESSAGE("Ajout de la categorie des boundarys");
    aSObject = aStudyBuilder->NewObjectToTag(theFatherHomard, _tag_boun);
    PublishInStudyAttr(aStudyBuilder, aSObject, "Boundaries", "BoundList", "zone_icone_2.png", NULL ) ;
  }
  else { MESSAGE("La categorie des boundarys existe deja."); }

  CORBA::Long BoundaryType = myBoundary->GetType();
//   MESSAGE("BoundaryType : "<<BoundaryType);
  std::string icone ;
  std::string value ;
  switch (BoundaryType)
  {
    case -1 :
    { value = "BoundaryCAOHomard" ;
      icone = "geometry.png" ;
      break;
    }
    case 0 :
    { value = "BoundaryDiHomard" ;
      icone = "mesh_tree_mesh.png" ;
      break;
    }
    case 1 :
    { value = "BoundaryAnHomard" ;
      icone = "cylinderpointvector_2.png" ;
      break;
    }
    case 2 :
    { value = "BoundaryAnHomard" ;
      icone = "spherepoint_2.png" ;
      break;
    }
    case 3 :
    { value = "BoundaryAnHomard" ;
      icone = "conepointvector.png" ;
      break;
    }
    case 4 :
    { value = "BoundaryAnHomard" ;
      icone = "conedxyz.png" ;
      break;
    }
    case 5 :
    { value = "BoundaryAnHomard" ;
      icone = "toruspointvector.png" ;
      break;
    }
  }
  aResultSO = aStudyBuilder->NewObject(aSObject);
  PublishInStudyAttr(aStudyBuilder, aResultSO, theName, value.c_str(), icone.c_str(), _orb->object_to_string(theObject));
  return aResultSO._retn();
}
//=============================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PublishCaseInStudy(SALOMEDS::StudyBuilder_var aStudyBuilder,
                                                       HOMARD::HOMARD_Cas_ptr theObject, const char* theName)
{
  MESSAGE("PublishCaseInStudy pour "<<theName);
  SALOMEDS::SObject_var aResultSO;

  if (CORBA::is_nil(theObject)) {
    MESSAGE("HOMARD_Gen_i::theObject->_is_nil()");
    return aResultSO._retn();
  }
  if (myStudy->_is_nil()) {
    MESSAGE("HOMARD_Gen_i::myStudy->_is_nil()");
    return aResultSO._retn();
  }

  // On recupere le module pere dans l etude
  SALOMEDS::SComponent_var theFatherHomard = myStudy->FindComponent(ComponentDataType());
  if (theFatherHomard->_is_nil())
  {
    MESSAGE("theFatherHomard->_is_nil()");
    return aResultSO._retn();
  }

  _tag_gene += 1 ;
  MESSAGE("PublishCaseInStudy _tag_gene = "<<_tag_gene );
  aResultSO = aStudyBuilder->NewObject(theFatherHomard);
  PublishInStudyAttr(aStudyBuilder, aResultSO, theName, "CasHomard", "cas_calcule.png",
                     _orb->object_to_string(theObject) ) ;

  return aResultSO._retn();
}
//=============================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PublishHypotheseInStudy(SALOMEDS::StudyBuilder_var aStudyBuilder,
                   HOMARD::HOMARD_Hypothesis_ptr theObject, const char* theName)
{
  MESSAGE("PublishHypotheseInStudy pour "<<theName);
  SALOMEDS::SObject_var aResultSO;

  // On recupere le module pere dans l etude
  // On ajoute la categorie des hypotheses dans l etude si necessaire
  SALOMEDS::SComponent_var theFatherHomard = myStudy->FindComponent(ComponentDataType());
  if (theFatherHomard->_is_nil())
  {
    MESSAGE("theFatherHomard->_is_nil()");
    return aResultSO._retn();
  }

  // On ajoute la categorie des hypotheses dans l etude si necessaire
  SALOMEDS::SObject_var aSObject;
  if ( _tag_hypo == 0 )
  {
    _tag_gene += 1 ;
    _tag_hypo = _tag_gene ;
  }
  MESSAGE("PublishHypotheseInStudy _tag_gene = "<<_tag_gene << ", _tag_hypo = "<<_tag_hypo );
  if (!theFatherHomard->FindSubObject(_tag_hypo, aSObject))
  {
    MESSAGE("Ajout de la categorie des hypotheses");
    aSObject = aStudyBuilder->NewObjectToTag(theFatherHomard, _tag_hypo);
    PublishInStudyAttr(aStudyBuilder, aSObject, "Hypothesis", "HypoList", "hypotheses.png", NULL);
  }
  else { MESSAGE("La categorie des hypotheses existe deja."); }

// Creation du resultat dans l'etude
  aResultSO = aStudyBuilder->NewObject(aSObject);
  PublishInStudyAttr(aStudyBuilder, aResultSO, theName, "HypoHomard", NULL, _orb->object_to_string(theObject) ) ;

  return aResultSO._retn();
}
//=============================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PublishYACSInStudy(SALOMEDS::StudyBuilder_var aStudyBuilder,
                   HOMARD::HOMARD_YACS_ptr theObject, const char* theName)
{
  MESSAGE("PublishYACSInStudy pour "<<theName);
  SALOMEDS::SObject_var aResultSO;

  // On recupere le module pere dans l etude
  // On ajoute la categorie des schemas YACS dans l etude si necessaire
  SALOMEDS::SComponent_var theFatherHomard = myStudy->FindComponent(ComponentDataType());
  if (theFatherHomard->_is_nil())
  {
    MESSAGE("theFatherHomard->_is_nil()");
    return aResultSO._retn();
  }
  // On ajoute la categorie des schemas YACS dans l etude si necessaire
  if ( _tag_yacs == 0 )
  {
    _tag_gene += 1 ;
    _tag_yacs = _tag_gene ;
  }
  MESSAGE("PublishZoneStudy _tag_gene = "<<_tag_gene << ", _tag_yacs = "<<_tag_yacs );
  SALOMEDS::SObject_var aSObject;
  if (!theFatherHomard->FindSubObject(_tag_yacs, aSObject))
  {
    MESSAGE("Ajout de la categorie des schemas YACS");
    aSObject = aStudyBuilder->NewObjectToTag(theFatherHomard, _tag_yacs);
    PublishInStudyAttr(aStudyBuilder, aSObject, "YACS", "YACSList", "full_view.png", NULL);
  }
  else { MESSAGE("La categorie des schemas YACS existe deja."); }

// Creation du resultat dans l'etude
  aResultSO = aStudyBuilder->NewObject(aSObject);
  PublishInStudyAttr(aStudyBuilder, aResultSO, theName, "YACSHomard", "schema.png", _orb->object_to_string(theObject) ) ;

  return aResultSO._retn();
}

//=============================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PublishZoneInStudy(SALOMEDS::StudyBuilder_var aStudyBuilder,
                   HOMARD::HOMARD_Zone_ptr theObject, const char* theName)
{
  MESSAGE("PublishZoneStudy pour "<<theName);
  SALOMEDS::SObject_var aResultSO;
  if (CORBA::is_nil(theObject))
  {
    MESSAGE("PublishZoneInStudy : theObject->_is_nil()");
    return aResultSO._retn();
  }
  if (myStudy->_is_nil())
  {
    MESSAGE("PublishZoneInStudy : myStudy->_is_nil()");
    return aResultSO._retn();
  }
  SALOMEDS::SComponent_var theFatherHomard = myStudy->FindComponent(ComponentDataType());
  if (theFatherHomard->_is_nil())
  {
    MESSAGE("PublishZoneInStudy : theFatherHomard->_is_nil()");
    return aResultSO._retn();
  }

  // Caracteristique de la zone
  HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[theName];
  CORBA::Long ZoneType = myZone->GetType();

  // On ajoute la categorie des zones dans l etude si necessaire
  if ( _tag_zone == 0 )
  {
    _tag_gene += 1 ;
    _tag_zone = _tag_gene ;
  }
  MESSAGE("PublishZoneStudy _tag_gene = "<<_tag_gene << ", _tag_zone = "<<_tag_zone );
  SALOMEDS::SObject_var aSObject;
  if (!theFatherHomard->FindSubObject(_tag_zone, aSObject))
  {
    MESSAGE("Ajout de la categorie des zones");
    aSObject = aStudyBuilder->NewObjectToTag(theFatherHomard, _tag_zone);
    PublishInStudyAttr(aStudyBuilder, aSObject, "Zones", "ZoneList", "zone_icone_2.png", NULL ) ;
  }
  else { MESSAGE("La categorie des zones existe deja."); }

  aResultSO = aStudyBuilder->NewObject(aSObject);
  std::string icone ;
  switch (ZoneType)
  {
    case 11 :
    { }
    case 12 :
    { }
    case 13 :
    { icone = "boxdxy_2.png" ;
      break ;
    }
    case 2 :
    { icone = "boxdxyz_2.png" ;
      break ;
    }
    case 31 :
    { }
    case 32 :
    { }
    case 33 :
    { icone = "disk_2.png" ;
      break ;
     }
    case 4 :
    { icone = "spherepoint_2.png" ;
      break ;
    }
    case 5 :
    { icone = "cylinderpointvector_2.png" ;
      break ;
    }
    case 61 :
    { }
    case 62 :
    { }
    case 63 :
    { icone = "diskwithhole_2.png" ;
      break ;
     }
    case 7 :
    { icone = "pipe_2.png" ;
      break ;
    }
  }
  PublishInStudyAttr(aStudyBuilder, aResultSO, theName, "ZoneHomard", icone.c_str(), _orb->object_to_string(theObject) ) ;

  return aResultSO._retn();
}
//===========================================================================
void HOMARD_Gen_i::PublishInStudyAttr(SALOMEDS::StudyBuilder_var aStudyBuilder,
                                      SALOMEDS::SObject_var aResultSO,
                                      const char* name, const char* comment, const char* icone, const char* ior)
{
  MESSAGE("PublishInStudyAttr pour name = "<<name<<", comment = "<<comment);
//   MESSAGE("icone = "<<icone);
//   MESSAGE("ior   = "<<ior);
  SALOMEDS::GenericAttribute_var anAttr ;
//  Ajout du nom
  if ( name != NULL )
  {
    anAttr = aStudyBuilder->FindOrCreateAttribute(aResultSO, "AttributeName");
    SALOMEDS::AttributeName_var aNameAttrib = SALOMEDS::AttributeName::_narrow(anAttr);
    aNameAttrib->SetValue(name);
  }

//  Ajout du commentaire
  if ( comment != NULL )
  {
    anAttr = aStudyBuilder->FindOrCreateAttribute(aResultSO, "AttributeComment");
    SALOMEDS::AttributeComment_var aCommentAttrib = SALOMEDS::AttributeComment::_narrow(anAttr);
    aCommentAttrib->SetValue(comment);
  }

//  Ajout de l'icone
  if ( icone != NULL  )
  {
    anAttr = aStudyBuilder->FindOrCreateAttribute(aResultSO, "AttributePixMap");
    SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow(anAttr);
    aPixmap->SetPixMap(icone);
  }

//  Ajout de l ior
  if ( ior != NULL  )
  {
    anAttr = aStudyBuilder->FindOrCreateAttribute(aResultSO, "AttributeIOR");
    SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR->SetValue(ior);
  }
};

//=====================================================================================
void HOMARD_Gen_i::PublishBoundaryUnderCase(const char* CaseName, const char* BoundaryName)
{
  MESSAGE ( "PublishBoundaryUnderCase : CaseName = " << CaseName << ", BoundaryName= " << BoundaryName );

  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[CaseName];
  ASSERT(!CORBA::is_nil(myCase));
  SALOMEDS::SObject_var aCaseSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myCase)));
  ASSERT(!CORBA::is_nil(aCaseSO));

  HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
  ASSERT(!CORBA::is_nil(myBoundary));
  SALOMEDS::SObject_var aBoundarySO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myBoundary)));
  ASSERT(!CORBA::is_nil(aBoundarySO));

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();

  aStudyBuilder->NewCommand();

  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aCaseSO);
  aStudyBuilder->Addreference(aSubSO, aBoundarySO);
//   aStudyBuilder->RemoveReference(aSubSO);

  aStudyBuilder->CommitCommand();

};
//=====================================================================================
void HOMARD_Gen_i::PublishCaseUnderYACS(const char* nomYACS, const char* CaseName)
{
  MESSAGE ( "PublishCaseUnderYACS : nomYACS = " << nomYACS << ", CaseName= " << CaseName );

  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[nomYACS];
  ASSERT(!CORBA::is_nil(myYACS));
  SALOMEDS::SObject_var aYACSSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myYACS)));
  ASSERT(!CORBA::is_nil(aYACSSO));

  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[CaseName];
  ASSERT(!CORBA::is_nil(myCase));
  SALOMEDS::SObject_var aCaseSO = SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myCase)));
  ASSERT(!CORBA::is_nil(aCaseSO));

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();

  aStudyBuilder->NewCommand();

  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aYACSSO);
  aStudyBuilder->Addreference(aSubSO, aCaseSO);

  aStudyBuilder->CommitCommand();

};
//=============================================================================
void HOMARD_Gen_i::PublishResultInSmesh(const char* NomFich, CORBA::Long Option)
//  Option = 0 : fichier issu d'une importation
//  Option = 1 : fichier issu d'une execution HOMARD
{
  MESSAGE( "PublishResultInSmesh " << NomFich << ", avec Option = " << Option);
  if (CORBA::is_nil(myStudy))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
    return ;
  };

// Le module SMESH est-il actif ?
  SALOMEDS::SObject_var aSmeshSO = myStudy->FindComponent("SMESH");
//
  if (!CORBA::is_nil(aSmeshSO))
  {
// On verifie que le fichier n est pas deja publie
    SALOMEDS::ChildIterator_var aIter = myStudy->NewChildIterator(aSmeshSO);
    for (; aIter->More(); aIter->Next())
    {
      SALOMEDS::SObject_var  aSO = aIter->Value();
      SALOMEDS::GenericAttribute_var aGAttr;
      if (aSO->FindAttribute(aGAttr,"AttributeExternalFileDef"))
      {
        SALOMEDS::AttributeExternalFileDef_var anAttr = SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
        CORBA::String_var value=anAttr->Value();
        if (strcmp((const char*)value,NomFich) == 0)
        {
          MESSAGE ( "PublishResultInSmesh : le fichier " << NomFich << " est deja publie." );
          // Pour un fichier importe, on ne republie pas
          if ( Option == 0 ) { return; }
          // Pour un fichier calcule, on commence par faire la depublication
          else
          {
            MESSAGE ( "PublishResultInSmesh : depublication" );
            SALOMEDS::AttributeName_var anAttr2 = SALOMEDS::AttributeName::_narrow(aGAttr);
            CORBA::String_var value2=anAttr2->Value();
            std::string MeshName = string(value2) ;
            MESSAGE ( "PublishResultInSmesh : depublication de " << MeshName );
            DeleteResultInSmesh(NomFich, MeshName) ;
          }
        }
      }
    }
  }

// On enregistre le fichier
  MESSAGE( "Enregistrement du fichier");
  SALOME_LifeCycleCORBA* myLCC = new SALOME_LifeCycleCORBA(_NS);
  SMESH::SMESH_Gen_var aSmeshEngine = SMESH::SMESH_Gen::_narrow(myLCC->FindOrLoad_Component("FactoryServer","SMESH"));
  ASSERT(!CORBA::is_nil(aSmeshEngine));
  aSmeshEngine->UpdateStudy();
  SMESH::DriverMED_ReadStatus theStatus;
  //aSmeshEngine->CreateMeshesFromMED(NomFich, theStatus);

// On met a jour les attributs AttributeExternalFileDef et AttributePixMap
  SMESH::mesh_array* mesMaillages=aSmeshEngine->CreateMeshesFromMED(NomFich, theStatus);
  for (int i = 0; i < mesMaillages->length();  i++)
  {
    MESSAGE( ". Mise a jour des attributs du maillage");
    SMESH::SMESH_Mesh_var monMaillage= (*mesMaillages)[i];
    SALOMEDS::SObject_var aSO=SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(monMaillage)));
    SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
    SALOMEDS::GenericAttribute_var aGAttr = aStudyBuilder->FindOrCreateAttribute(aSO, "AttributeExternalFileDef");
    SALOMEDS::AttributeExternalFileDef_var anAttr = SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
    anAttr->SetValue(NomFich);
    SALOMEDS::GenericAttribute_var aPixMap = aStudyBuilder->FindOrCreateAttribute(aSO, "AttributePixMap" );
    SALOMEDS::AttributePixMap_var anAttr2 = SALOMEDS::AttributePixMap::_narrow(aPixMap);
    const char* icone ;
    if ( Option == 0 ) { icone = "mesh_tree_importedmesh.png" ; }
    else               { icone = "mesh_tree_mesh.png" ; }
    anAttr2->SetPixMap( icone );
  }

}
//=============================================================================
void HOMARD_Gen_i::DeleteResultInSmesh(std::string NomFich, std::string MeshName)
{
  MESSAGE ("DeleteResultInSmesh pour le maillage " << MeshName << " dans le fichier " << NomFich );
  if (CORBA::is_nil(myStudy))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid study context";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

// Le module SMESH est-il actif ?
  SALOMEDS::SObject_var aSmeshSO = myStudy->FindComponent("SMESH");
//
  if (CORBA::is_nil(aSmeshSO))
  {
      return ;
  };
// On verifie que le fichier est deja publie
  SALOMEDS::StudyBuilder_var myBuilder = myStudy->NewBuilder();
  SALOMEDS::ChildIterator_var aIter = myStudy->NewChildIterator(aSmeshSO);
  for (; aIter->More(); aIter->Next())
  {
     SALOMEDS::SObject_var  aSO = aIter->Value();
     SALOMEDS::GenericAttribute_var aGAttr;
     if (aSO->FindAttribute(aGAttr,"AttributeExternalFileDef"))
     {
       SALOMEDS::AttributeExternalFileDef_var anAttr = SALOMEDS::AttributeExternalFileDef::_narrow(aGAttr);
       CORBA::String_var value=anAttr->Value();
       if (strcmp((const char*)value,NomFich.c_str()) == 0)
       {
         if (aSO->FindAttribute(aGAttr,"AttributeName"))
         {
           SALOMEDS::AttributeName_var anAttr2 = SALOMEDS::AttributeName::_narrow(aGAttr);
           CORBA::String_var value2=anAttr2->Value();
           if (strcmp((const char*)value2,MeshName.c_str()) == 0)
           {
             myBuilder->RemoveObjectWithChildren( aSO ) ;
           }
         }
       }
     }
  }
  return ;
}
//=============================================================================
void HOMARD_Gen_i::PublishMeshIterInSmesh(const char* NomIter)
{
  MESSAGE( "PublishMeshIterInSmesh " << NomIter);
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIter];

  SALOMEDS::SObject_var aIterSO=SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));
  if (CORBA::is_nil(myIteration))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iterationStudy Object";
      throw SALOME::SALOME_Exception(es);
      return ;
  };
  const char* MeshFile = myIteration->GetMeshFile() ;
  const char* MeshName = myIteration->GetMeshName() ;
  CORBA::Long Option = -1 ;
  int etat = myIteration->GetState();
// Iteration initiale
  if ( etat <= 0 )      { Option = 0 ; }
// ou iteration calculee
  else if ( etat == 2 ) { Option = 1 ; }
// Publication effective apres menage eventuel
  if ( Option >= 0 )
  {
    DeleteResultInSmesh(MeshFile, MeshName) ;
    PublishResultInSmesh(MeshFile, Option) ;
  }

}
//=============================================================================
void HOMARD_Gen_i::PublishFileUnderIteration(const char* NomIter, const char* NomFich, const char* Commentaire)
{
//   MESSAGE ("PublishFileUnderIteration pour l'iteration " << NomIter << " du fichier " << NomFich << " avec le commentaire " << Commentaire );
  HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[NomIter];

  SALOMEDS::SObject_var aIterSO=SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myIteration)));
  if (CORBA::is_nil(myIteration))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid iterationStudy Object";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();

  aStudyBuilder->NewCommand();

  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aIterSO);
// Pour les fichiers med, on affiche une icone de maillage
// Pour les fichiers qui sont du texte, on affiche une icone de fichier texte 'texte'
// Le reperage se fait par la 1ere lettre du commentaire : I pour Iteration n
  std::string icone ;
  std::string ior = " " ;
  if ( Commentaire[0] == 'M' )
  { icone = "med.png" ; }
  else
  { icone = "texte_2.png" ; }
  PublishInStudyAttr(aStudyBuilder, aSubSO, NomFich, Commentaire, icone.c_str(), ior.c_str() ) ;

  aStudyBuilder->CommitCommand();
}
//
//=============================================================================
void HOMARD_Gen_i::PublishFileUnderYACS(const char* nomYACS, const char* NomFich, const char* Commentaire)
{
//   MESSAGE ("PublishFileUnderYACS pour le schema " << nomYACS << " du fichier " << NomFich << " avec le commentaire " << Commentaire );
  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[nomYACS];

  SALOMEDS::SObject_var aYACSSO=SALOMEDS::SObject::_narrow(myStudy->FindObjectIOR(_orb->object_to_string(myYACS)));
  if (CORBA::is_nil(myYACS))
  {
      SALOME::ExceptionStruct es;
      es.type = SALOME::BAD_PARAM;
      es.text = "Invalid YACSStudy Object";
      throw SALOME::SALOME_Exception(es);
      return ;
  };

  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();

  aStudyBuilder->NewCommand();

  SALOMEDS::SObject_var aSubSO = aStudyBuilder->NewObject(aYACSSO);
  std::string icone = "texte_2.png" ;
  std::string ior = " " ;
  PublishInStudyAttr(aStudyBuilder, aSubSO, NomFich, Commentaire, icone.c_str(), ior.c_str() ) ;

  aStudyBuilder->CommitCommand();
}
//
//=============================================================================
//=============================================================================
// YACS
//=============================================================================
//=============================================================================
//=============================================================================
// Creation d'un schema YACS
// nomCas : nom du cas a traiter
// FileName : nom du fichier contenant le script de lancement du calcul
// DirName : le répertoire de lancement des calculs du schéma
//=============================================================================
HOMARD::HOMARD_YACS_ptr HOMARD_Gen_i::CreateYACSSchema (const char* nomYACS, const char* nomCas, const char* ScriptFile, const char* DirName, const char* MeshFile)
{
  INFOS ( "CreateYACSSchema : Schema YACS " << nomYACS );
  INFOS ( ". nomCas     : " << nomCas);
  INFOS ( ". ScriptFile : " << ScriptFile);
  INFOS ( ". DirName    : " << DirName);
  INFOS ( ". MeshFile   : " << MeshFile);

  // A. Controle du nom :
  if ((myStudyContext._mesYACSs).find(nomYACS) != (myStudyContext._mesYACSs).end())
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "This schema YACS has already been defined.";
    throw SALOME::SALOME_Exception(es);
    return 0;
  }

  // B. Creation de l'objet
  HOMARD::HOMARD_YACS_var myYACS = newYACS();
  if (CORBA::is_nil(myYACS))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Unable to create the schema YACS";
    throw SALOME::SALOME_Exception(es);
    return 0;
  };
  myYACS->SetName( nomYACS ) ;

  // C. Enregistrement
  myStudyContext._mesYACSs[nomYACS] = myYACS;

  SALOMEDS::SObject_var aSO;
  SALOMEDS::SObject_var aResultSO=PublishInStudy(aSO, myYACS, nomYACS);

  PublishCaseUnderYACS(nomYACS, nomCas);

  // D. Caracterisation
  // D.1. Options
  myYACS->SetDirName( DirName ) ;
  myYACS->SetMeshFile( MeshFile ) ;
  myYACS->SetScriptFile( ScriptFile ) ;
  myYACS->SetCaseName( nomCas ) ;
  // D.2. Defaut
  int defaut_i ;
  // D.2.1. Type constant
  myYACS->SetType( 1 ) ;
  // D.2.2. Convergence
  defaut_i = GetYACSMaxIter() ;
  myYACS->SetMaxIter( defaut_i ) ;
  defaut_i = GetYACSMaxNode() ;
  myYACS->SetMaxNode( defaut_i ) ;
  defaut_i = GetYACSMaxElem() ;
  myYACS->SetMaxElem( defaut_i ) ;
  // D.3. Fichier de sauvegarde dans le répertoire du cas
  HOMARD::HOMARD_Cas_ptr caseyacs = GetCase(nomCas) ;
  std::string dirnamecase = caseyacs->GetDirName() ;
  std::string XMLFile ;
  XMLFile = dirnamecase + "/schema.xml" ;
  myYACS->SetXMLFile( XMLFile.c_str() ) ;

  return HOMARD::HOMARD_YACS::_duplicate(myYACS);
}
//=============================================================================
// Ecriture d'un schema YACS
//=============================================================================
CORBA::Long HOMARD_Gen_i::YACSWrite(const char* nomYACS)
{
  INFOS ( "YACSWrite : Ecriture de " << nomYACS );
// Le répertoire du cas
  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[nomYACS];
  ASSERT(!CORBA::is_nil(myYACS));
// Le nom du fichier du schema
  std::string XMLFile ;
  XMLFile = myYACS->GetXMLFile() ;

  int codret = YACSWriteOnFile(nomYACS, XMLFile.c_str()) ;

  return codret ;
}
//=============================================================================
// Ecriture d'un schema YACS sur un fichier donne
//=============================================================================
CORBA::Long HOMARD_Gen_i::YACSWriteOnFile(const char* nomYACS, const char* XMLFile)
{
  INFOS ( "YACSWriteOnFile : Ecriture de " << nomYACS << " sur " << XMLFile );

  // A. Prealable
  int codret = 0;

  // B. L'objet YACS
  // B.1. L'objet
  HOMARD::HOMARD_YACS_var myYACS = myStudyContext._mesYACSs[nomYACS];
  ASSERT(!CORBA::is_nil(myYACS));
  // B.2. Les caracteristiques
  std::string DirName = myYACS->GetDirName() ;
  std::string MeshFile = myYACS->GetMeshFile() ;
  std::string ScriptFile = myYACS->GetScriptFile() ;
  // B.3. Les caracteristiques de convergence
  int MaxIter = myYACS->GetMaxIter() ;
  int MaxNode = myYACS->GetMaxNode() ;
  int MaxElem = myYACS->GetMaxElem() ;

  // C. Le cas
  // C.1. L'objet cas
  const char* nomCas = myYACS->GetCaseName();
  HOMARD::HOMARD_Cas_var myCase = myStudyContext._mesCas[nomCas];
  ASSERT(!CORBA::is_nil(myCase));
  // C.2. Les instructions python associees au cas
  CORBA::String_var dumpCorbaCase = myCase->GetDumpPython();
  std::string pythonCas = dumpCorbaCase.in();
  MESSAGE ("pythonCas :\n"<<pythonCas<<"\n");

  // D. Les iterations
  // D.1. L'iteration initiale
  HOMARD::HOMARD_Iteration_var Iter0 = myCase->GetIter0() ;
  std::string Iter0Name = myCase->GetIter0Name() ;
  MESSAGE (". Iter0Name = " << Iter0Name);
  std::string MeshName = Iter0->GetMeshName();
  MESSAGE (". MeshName = " << MeshName);
  // D.2. L'iteration numero 1
  HOMARD::listeIterFilles* maListe = Iter0->GetIterations();
  int numberOfIter = maListe->length();
  if ( numberOfIter > 1 )
  {
    MESSAGE (". numberOfIter = " << numberOfIter);
    SALOME::ExceptionStruct es ;
    es.type = SALOME::BAD_PARAM;
    std::string text = "Une seule iteration est permise." ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
    return 0;
  }
  std::string Iter1Name = std::string((*maListe)[0]);
  MESSAGE ("... Iter1Name = " << Iter1Name);
  HOMARD::HOMARD_Iteration_var Iter1 = GetIteration(Iter1Name.c_str()) ;
  // D.3. Les instructions python associees a l'iteration
  CORBA::String_var dumpCorbaIter = Iter1->GetDumpPython();
  std::string pythonIter = dumpCorbaIter.in();
  MESSAGE ("pythonIter :\n"<<pythonIter<<"\n");

  // E. L'hypothese pour passer de l'iteration initiale a la suivante
  // E.1. La structure
  std::string nomHypo = Iter1->GetHypoName();
  MESSAGE (". nomHypo = " << nomHypo);
  HOMARD::HOMARD_Hypothesis_var myHypo = myStudyContext._mesHypotheses[nomHypo];
  ASSERT(!CORBA::is_nil(myHypo));
  // E.2. Les caracteristiques de l'adaptation
  HOMARD::listeTypes* ListTypes = myHypo->GetAdapRefinUnRef();
  ASSERT(ListTypes->length() == 3);
  int TypeAdap = (*ListTypes)[0];
//   int TypeRaff = (*ListTypes)[1];
//   int TypeDera = (*ListTypes)[2];
  // E.3. Les instructions python associees a l'hypothese
  CORBA::String_var dumpCorbaHypo = myHypo->GetDumpPython();
  std::string pythonHypo = dumpCorbaHypo.in();
  MESSAGE ("pythonHypo :\n"<<pythonHypo<<"\n");

  // F. Le fichier du schema de reference
  // HOMARD_ROOT_DIR : répertoire ou se trouve le module HOMARD
  std::string XMLFile_base ;
  if ( getenv("HOMARD_ROOT_DIR") != NULL ) { XMLFile_base = getenv("HOMARD_ROOT_DIR") ; }
  else
  {
    SALOME::ExceptionStruct es ;
    es.type = SALOME::BAD_PARAM;
    std::string text = "HOMARD_ROOT_DIR est inconnu." ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
    return 0;
  }
  XMLFile_base += "/share/salome/resources/homard/yacs_01." + _LangueShort + ".xml" ;
//   if ( _Langue ==
  MESSAGE("XMLFile_base ="<<XMLFile_base);

  // G. Lecture du schema de reference et insertion des données propres au fil de la rencontre des mots-cles
  YACSDriver* myDriver = new YACSDriver(XMLFile, DirName, _LangueShort);
  std::ifstream fichier( XMLFile_base.c_str() );
  if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
  {
    // G.1. Lecture du schema de reference et insertion des données propres au fil de la rencontre des mots-cles
    std::string ligne; // variable contenant chaque ligne lue
    std::string mot_cle;
    while ( std::getline( fichier, ligne ) )
    {
      // G.1.1. Pour la ligne courante, on identifie le premier mot : le mot-cle eventuel
      std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
      ligne_bis >> mot_cle ;
      // G.1.2. Le maillage initial
      if ( mot_cle == "DataInit_MeshFile" )
      { myDriver->Texte_DataInit_MeshFile(MeshFile); }
      // G.1.3. Le script de lancement
      else if ( mot_cle == "Alternance_Calcul_HOMARD_Calcul" )
      { myDriver->Texte_Alternance_Calcul_HOMARD_Calcul(ScriptFile); }
      // G.1.4. Les options du cas
      else if ( mot_cle == "Iter_1_Case_Options" )
      { myDriver->Texte_Iter_1_Case_Options(pythonCas); }
      // G.1.5. Execution de HOMARD : le répertoire du cas
      else if ( mot_cle == "HOMARD_Exec_DirName" )
      { myDriver->Texte_HOMARD_Exec_DirName(); }
      // G.1.6. Execution de HOMARD : le nom du maillage
      else if ( mot_cle == "HOMARD_Exec_MeshName" )
      {
        myDriver->Texte_HOMARD_Exec_MeshName(MeshName);
        std::string node ;
        if ( _LangueShort == "fr" ) { node = "Boucle_de_convergence.Alternance_Calcul_HOMARD" ; }
        else                        { node = "Convergence_Loop.Alternation_Computation_HOMARD" ; }
        node += ".Adaptation.p0_Adaptation_HOMARD.HOMARD_Initialisation.p1_Iter_1.CreateCase" ;
        myDriver->TexteParametre( node, "MeshName", "string", MeshName ) ;
      }
      // G.1.7. Execution de HOMARD : les options de l'hypothese
      else if ( mot_cle == "HOMARD_Exec_Hypo_Options" )
      { myDriver->Texte_python_1( pythonHypo, 3, "Hypo" ) ;  }
      // G.1.8. Execution de HOMARD : les options de l'iteration
      else if ( mot_cle == "HOMARD_Exec_Iter_Options" )
      { myDriver->Texte_python_2( pythonIter, "TimeStep", "Iter" ) ;  }
      // G.1.9. a. Creation eventuelles des zones et frontieres
      //        b. Enchainement
      else if ( mot_cle == "Iter_1" )
      {
        std::string texte_control = "" ;
        if ( TypeAdap == 0 ) { texte_control += YACSDriverTexteZone( myHypo, myDriver ) ; }
        texte_control += YACSDriverTexteBoundary( myCase, myDriver ) ;
        texte_control += myDriver->Texte_Iter_1_control() ;
        myDriver->TexteAdd(texte_control);
      }
      // G.1.10. Les tests de convergence
      else if ( mot_cle == "Analyse_Test_Convergence" )
      { myDriver->TexteAnalyse_Test_Convergence(MaxIter, MaxNode, MaxElem); }
      // G.1.11. Les parametres
      else if ( mot_cle == "PARAMETRES" )
      { myDriver->TexteAddParametres(); }
      // G.1.n. La ligne est recopiee telle quelle
      else { myDriver->TexteAdd(ligne);  }
    }
    // G.2. Ecriture du texte dans le fichier
    if ( codret == 0 )
    { myDriver->CreeFichier(); }
  }
  else
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    std::string text = "The reference file for the YACS schema cannot be read." ;
    es.text = CORBA::string_dup(text.c_str());
    throw SALOME::SALOME_Exception(es);
  }

  delete myDriver;

  // H. Publication du fichier dans l'arbre

    std::string Commentaire = "xml" ;
    PublishFileUnderYACS(nomYACS, XMLFile, Commentaire.c_str());

  return codret ;
}
//=============================================================================
// Ecriture d'un schema YACS : ecriture des zones associees a une hypothese
//=============================================================================
std::string HOMARD_Gen_i::YACSDriverTexteZone(HOMARD::HOMARD_Hypothesis_var myHypo, YACSDriver* myDriver)
{
  MESSAGE ( "YACSDriverTexteZone" );
  // A. Les zones associees a cette hypothese
  HOMARD::listeZonesHypo* ListZone = myHypo->GetZones();
  int numberOfZonesx2 = ListZone->length();

  // B. Parcours des zones
  std::string texte_control ;
  for (int iaux = 0; iaux< numberOfZonesx2; iaux++)
  {
    // 1. Reperage de la zone
    std::string ZoneName = std::string((*ListZone)[iaux]);
    MESSAGE ( "\n. ZoneName = " << ZoneName << " - " <<iaux);
    HOMARD::HOMARD_Zone_var myZone = myStudyContext._mesZones[ZoneName];
    ASSERT(!CORBA::is_nil(myZone));
    // 2. Les instructions python associees a la zone
    //    La premiere ligne est un commentaire a eliminer
    //    La seconde ligne est l'instruction a proprement parler ; on ne garde que ce qui suit le "."
    CORBA::String_var dumpCorbaZone = myZone->GetDumpPython();
    std::string pythonStructure_0 = dumpCorbaZone.in();
    MESSAGE ("pythonStructure_0 :"<<pythonStructure_0);
    std::istringstream tout (pythonStructure_0) ;
    std::string ligne ;
    std::string pythonStructure ;
    while ( std::getline( tout, ligne ) )
    { pythonStructure = GetStringInTexte ( ligne, ".", 1 ) ; }
    MESSAGE ("pythonStructure :\n"<<pythonStructure);
    // 3. Decodage du nom du service
    std::string methode = GetStringInTexte ( pythonStructure, "(", 0 ) ;
    MESSAGE ( "... methode = " << methode);
    // 4. Mise en place des instructions
    int ZoneType = myZone->GetType();
    MESSAGE ( "... ZoneType = " << ZoneType);
    std::string texte_control_0 ;
    texte_control_0 = myDriver->Texte_Iter_1_Zone(ZoneType, pythonStructure, methode, ZoneName );
    texte_control += texte_control_0 ;
    // 5. Decalage
    iaux ++ ;
  }

  return texte_control ;
}
//=============================================================================
// Ecriture d'un schema YACS : ecriture des frontieres associees au cas
//=============================================================================
std::string HOMARD_Gen_i::YACSDriverTexteBoundary(HOMARD::HOMARD_Cas_var myCase, YACSDriver* myDriver)
{
  MESSAGE ( "YACSDriverTexteBoundary" );
  // A. Les frontieres associees au cas
  HOMARD::ListBoundaryGroupType* ListBoundaryGroupType = myCase->GetBoundaryGroup();
  int numberOfitems = ListBoundaryGroupType->length();

  // B. Parcours des frontieres
  std::string texte_control ;
  std::list<std::string>  ListeBoundaryTraitees ;
  for (int NumBoundary = 0; NumBoundary< numberOfitems; NumBoundary=NumBoundary+2)
  {
    std::string BoundaryName = std::string((*ListBoundaryGroupType)[NumBoundary]);
    MESSAGE ( "... BoundaryName = " << BoundaryName);
    // Attention a n'ecrire la definition qu'une seule fois car elle peut se trouver
    // plusieurs fois dans la definition du cas, d'ou la liste ListeBoundaryTraitees
    int A_faire = 1 ;
    std::list<std::string>::const_iterator it = ListeBoundaryTraitees.begin();
    while (it != ListeBoundaryTraitees.end())
    {
      MESSAGE ( "..... BoundaryNameTraitee = " << *it);
      if ( BoundaryName == *it ) { A_faire = 0 ; }
      it++;
    }
    if ( A_faire == 1 )
    {
    // 1. Caracteristiques de la frontiere
      HOMARD::HOMARD_Boundary_var myBoundary = myStudyContext._mesBoundarys[BoundaryName];
      ASSERT(!CORBA::is_nil(myBoundary));
      // 2. Les instructions python associees a la frontiere
      //    La premiere ligne est un commentaire a eliminer
      //    La seconde ligne est l'instruction a proprement parler ; on ne garde que ce qui suit le "."
      CORBA::String_var dumpCorbaBoundary = myBoundary->GetDumpPython();
      std::string pythonStructure_0 = dumpCorbaBoundary.in();
      MESSAGE ("pythonStructure_0 :"<<pythonStructure_0);
      std::istringstream tout (pythonStructure_0) ;
      std::string ligne ;
      std::string pythonStructure ;
      while ( std::getline( tout, ligne ) )
      { pythonStructure = GetStringInTexte ( ligne, ".", 1 ) ; }
      MESSAGE ("pythonStructure :\n"<<pythonStructure);
      // 3. Decodage du nom du service
      std::string methode = GetStringInTexte ( pythonStructure, "(", 0 ) ;
      MESSAGE ( "... methode = " << methode);
      // 4. Mise en place des instructions
      int BoundaryType = myBoundary->GetType();
      MESSAGE ( "... BoundaryType = " << BoundaryType);
      const char* MeshName ;
      const char* DataFile ;
      if (BoundaryType == -1)
      {
        DataFile = myBoundary->GetDataFile() ;
        MESSAGE ( ". CAOFile = " << DataFile );
      }
      else if (BoundaryType == 0)
      {
        MeshName = myBoundary->GetMeshName() ;
        MESSAGE ( ". MeshName = " << MeshName );
        DataFile = myBoundary->GetDataFile() ;
        MESSAGE ( ". MeshFile = " << DataFile );
      }
      std::string texte_control_0 ;
      texte_control_0 = myDriver->Texte_Iter_1_Boundary(BoundaryType, pythonStructure, methode, BoundaryName, MeshName, DataFile );
      texte_control += texte_control_0 ;
      // 5. Memorisation du traitement
      ListeBoundaryTraitees.push_back( BoundaryName );
    }
  }

  return texte_control ;
}
//
//=============================================================================
//=============================================================================
// Next functions are inherited from SALOMEDS::Driver interface
//=============================================================================
//=============================================================================
SALOMEDS::TMPFile* HOMARD_Gen_i::Save(SALOMEDS::SComponent_ptr theComponent,
                                      const char* theURL,
                                      CORBA::Boolean isMultiFile)
{
  MESSAGE ("Save for theURL = "<< theURL);
  SALOMEDS::TMPFile_var aStreamFile;

  // get temporary directory name
  std::string tmpDir = isMultiFile ? std::string(theURL) : SALOMEDS_Tool::GetTmpDir();

  StudyContext& context = myStudyContext;

  // HOMARD data file name
  std::string aFileName = "";
  if (isMultiFile)
    aFileName = SALOMEDS_Tool::GetNameFromPath(Kernel_Utils::encode(SMESH_Gen_i::getStudyServant()->URL()));
  aFileName += "_HOMARD.dat";

  // initialize sequence of file names
  SALOMEDS_Tool::ListOfFiles aFileSeq;
  aFileSeq.reserve(1);
  aFileSeq.push_back(CORBA::string_dup(aFileName.c_str())) ;

  // get full path to the data file
  aFileName = tmpDir + aFileName;

  // save data
  // -> create file
  std::ofstream f(aFileName.c_str());

  // clear temporary id map
  context._idmap.clear();

  int id = 1;

  // -> save cases
  std::map<std::string, HOMARD::HOMARD_Cas_var>::const_iterator it_case;
  for (it_case = context._mesCas.begin(); it_case != context._mesCas.end(); ++it_case) {
    HOMARD::HOMARD_Cas_var aCas = it_case->second;
    PortableServer::ServantBase_var aServant = GetServant(aCas);
    HOMARD_Cas_i* aCasServant = dynamic_cast<HOMARD_Cas_i*>(aServant.in());
    if (aCasServant) {
      f << HOMARD::GetSignature(HOMARD::Case) << aCasServant->Dump() << std::endl;
      context._idmap[id++] = dynamic_cast<PortableServer::ServantBase*>(aCasServant);
    }
  }
  // -> save zones
  std::map<std::string, HOMARD::HOMARD_Zone_var>::const_iterator it_zone;
  for (it_zone = context._mesZones.begin(); it_zone != context._mesZones.end(); ++it_zone) {
    HOMARD::HOMARD_Zone_var aZone = it_zone->second;
    PortableServer::ServantBase_var aServant = GetServant(aZone);
    HOMARD_Zone_i* aZoneServant = dynamic_cast<HOMARD_Zone_i*>(aServant.in());
    if (aZoneServant) {
      f << HOMARD::GetSignature(HOMARD::Zone) << aZoneServant->Dump() << std::endl;
      context._idmap[id++] = dynamic_cast<PortableServer::ServantBase*>(aZoneServant);
    }
  }
  // -> save hypotheses
  std::map<std::string, HOMARD::HOMARD_Hypothesis_var>::const_iterator it_hypo;
  for (it_hypo = context._mesHypotheses.begin(); it_hypo != context._mesHypotheses.end(); ++it_hypo) {
    HOMARD::HOMARD_Hypothesis_var aHypo = it_hypo->second;
    PortableServer::ServantBase_var aServant = GetServant(aHypo);
    HOMARD_Hypothesis_i* aHypoServant = dynamic_cast<HOMARD_Hypothesis_i*>(aServant.in());
    if (aHypoServant) {
      f << HOMARD::GetSignature(HOMARD::Hypothesis) << aHypoServant->Dump() << std::endl;
      context._idmap[id++] = dynamic_cast<PortableServer::ServantBase*>(aHypoServant);
    }
  }
  // -> save iterations
  std::map<std::string, HOMARD::HOMARD_Iteration_var>::const_iterator it_iter;
  for (it_iter = context._mesIterations.begin(); it_iter != context._mesIterations.end(); ++it_iter) {
    HOMARD::HOMARD_Iteration_var aIter = it_iter->second;
    PortableServer::ServantBase_var aServant = GetServant(aIter);
    HOMARD_Iteration_i* aIterServant = dynamic_cast<HOMARD_Iteration_i*>(aServant.in());
    if (aIterServant) {
      f << HOMARD::GetSignature(HOMARD::Iteration) << aIterServant->Dump() << std::endl;
      context._idmap[id++] = dynamic_cast<PortableServer::ServantBase*>(aIterServant);
    }
  }
  // -> save boundaries
  std::map<std::string, HOMARD::HOMARD_Boundary_var>::const_iterator it_boundary;
  for (it_boundary = context._mesBoundarys.begin(); it_boundary != context._mesBoundarys.end(); ++it_boundary) {
    HOMARD::HOMARD_Boundary_var aBoundary = it_boundary->second;
    PortableServer::ServantBase_var aServant = GetServant(aBoundary);
    HOMARD_Boundary_i* aBoundaryServant = dynamic_cast<HOMARD_Boundary_i*>(aServant.in());
    if (aBoundaryServant) {
      f << HOMARD::GetSignature(HOMARD::Boundary) << aBoundaryServant->Dump() << std::endl;
      context._idmap[id++] = dynamic_cast<PortableServer::ServantBase*>(aBoundaryServant);
    }
  }
  // -> close file
  MESSAGE ("close file");
  f.close();

  // put temporary files to the stream
  MESSAGE ("put temporary files to the stream");
  aStreamFile = SALOMEDS_Tool::PutFilesToStream(tmpDir.c_str(), aFileSeq, isMultiFile);

  // remove temporary files
  MESSAGE ("remove temporary files");
  if (!isMultiFile) SALOMEDS_Tool::RemoveTemporaryFiles(tmpDir.c_str(), aFileSeq, true);

  // return data stream
  MESSAGE ("return data stream");
  return aStreamFile._retn();
};

//===========================================================================
SALOMEDS::TMPFile* HOMARD_Gen_i::SaveASCII(SALOMEDS::SComponent_ptr theComponent,
                                           const char* theURL,
                                           CORBA::Boolean isMultiFile)
{
  // No specific ASCII persistence
  SALOMEDS::TMPFile_var aStreamFile = Save(theComponent, theURL, isMultiFile);
  return aStreamFile._retn();
};

//===========================================================================
CORBA::Boolean HOMARD_Gen_i::Load(SALOMEDS::SComponent_ptr theComponent,
				   const SALOMEDS::TMPFile& theStream,
				   const char* theURL,
				   CORBA::Boolean isMultiFile)
{
  MESSAGE ("Load pour theURL = "<< theURL);

  // set current study
  if (myStudy->_is_nil())
    UpdateStudy();

  // get temporary directory name
  std::string tmpDir = isMultiFile ? std::string(theURL) : SALOMEDS_Tool::GetTmpDir();

  // Convert the stream into sequence of files to process
  SALOMEDS_Tool::ListOfFiles aFileSeq = SALOMEDS_Tool::PutStreamToFiles(theStream,
                                                                        tmpDir.c_str(),
                                                                        isMultiFile);
  // HOMARD data file name
  std::string aFileName = "";
  if (isMultiFile)
    aFileName = SALOMEDS_Tool::GetNameFromPath(Kernel_Utils::encode(SMESH_Gen_i::getStudyServant()->URL()));
  aFileName = tmpDir + aFileName + "_HOMARD.dat";

  StudyContext& context = myStudyContext;

  // save data
  // -> create file
  std::ifstream f(aFileName.c_str());

  // clear context
  context._mesCas.clear();
  context._mesHypotheses.clear();
  context._mesIterations.clear();
  context._mesZones.clear();
  context._mesBoundarys.clear();
  context._idmap.clear();

  int id = 1;
  std::string line;

  while (f) {
    std::getline(f, line);
    std::string bounSignature = HOMARD::GetSignature(HOMARD::Boundary);
    std::string caseSignature = HOMARD::GetSignature(HOMARD::Case);
    std::string hypoSignature = HOMARD::GetSignature(HOMARD::Hypothesis);
    std::string iterSignature = HOMARD::GetSignature(HOMARD::Iteration);
    std::string zoneSignature = HOMARD::GetSignature(HOMARD::Zone);
    std::string yacsSignature = HOMARD::GetSignature(HOMARD::YACS);
    if (line.substr(0, bounSignature.size()) == bounSignature) {
      // re-create boundary
      MESSAGE ("Recreation de la frontiere" );
      HOMARD::HOMARD_Boundary_var aBoundary = newBoundary();
      PortableServer::ServantBase_var aServant = GetServant(aBoundary);
      HOMARD_Boundary_i* aBoundaryServant = dynamic_cast<HOMARD_Boundary_i*>(aServant.in());
      if (aBoundaryServant && aBoundaryServant->Restore(line.substr(bounSignature.size()))) {
        context._mesBoundarys[aBoundary->GetName()] = aBoundary;
        context._idmap[id] = dynamic_cast<PortableServer::ServantBase*>(aBoundaryServant);
      }
    }
    else if (line.substr(0, caseSignature.size()) == caseSignature) {
      // re-create case
      MESSAGE ("Recreation du cas" );
      HOMARD::HOMARD_Cas_var aCase = newCase();
      PortableServer::ServantBase_var aServant = GetServant(aCase);
      HOMARD_Cas_i* aCaseServant = dynamic_cast<HOMARD_Cas_i*>(aServant.in());
      if (aCaseServant && aCaseServant->Restore(line.substr(caseSignature.size()))) {
        context._mesCas[aCase->GetName()] = aCase;
        context._idmap[id] = dynamic_cast<PortableServer::ServantBase*>(aCaseServant);
      }
    }
    else if (line.substr(0, hypoSignature.size()) == hypoSignature) {
      // re-create hypothesis
      MESSAGE ("Recreation de l hypothese" );
      HOMARD::HOMARD_Hypothesis_var aHypo = newHypothesis();
      PortableServer::ServantBase_var aServant = GetServant(aHypo);
      HOMARD_Hypothesis_i* aHypoServant = dynamic_cast<HOMARD_Hypothesis_i*>(aServant.in());
      if (aHypoServant && aHypoServant->Restore(line.substr(hypoSignature.size()))) {
        context._mesHypotheses[aHypo->GetName()] = aHypo;
        context._idmap[id] = dynamic_cast<PortableServer::ServantBase*>(aHypoServant);
      }
    }
    else if (line.substr(0, iterSignature.size()) == iterSignature) {
      // re-create iteration
      MESSAGE ("Recreation de l'iteration" );
      HOMARD::HOMARD_Iteration_var aIter = newIteration();
      PortableServer::ServantBase_var aServant = GetServant(aIter);
      HOMARD_Iteration_i* aIterServant = dynamic_cast<HOMARD_Iteration_i*>(aServant.in());
      if (aIterServant && aIterServant->Restore(line.substr(iterSignature.size()))) {
        context._mesIterations[aIter->GetName()] = aIter;
        context._idmap[id] = dynamic_cast<PortableServer::ServantBase*>(aIterServant);
      }
    }
    else if (line.substr(0, zoneSignature.size()) == zoneSignature) {
      MESSAGE ("Recreation de la zone" );
      // re-create zone
      HOMARD::HOMARD_Zone_var aZone = newZone();
      PortableServer::ServantBase_var aServant = GetServant(aZone);
      HOMARD_Zone_i* aZoneServant = dynamic_cast<HOMARD_Zone_i*>(aServant.in());
      if (aZoneServant && aZoneServant->Restore(line.substr(zoneSignature.size()))) {
        context._mesZones[aZone->GetName()] = aZone;
        context._idmap[id] = dynamic_cast<PortableServer::ServantBase*>(aZoneServant);
      }
    }
    else if (line.substr(0, zoneSignature.size()) == yacsSignature) {
      MESSAGE ("Recreation du schema YACS" );
      // re-create YACS
      HOMARD::HOMARD_YACS_var aYACS = newYACS();
      PortableServer::ServantBase_var aServant = GetServant(aYACS);
      HOMARD_YACS_i* aYACSServant = dynamic_cast<HOMARD_YACS_i*>(aServant.in());
      if (aYACSServant && aYACSServant->Restore(line.substr(yacsSignature.size()))) {
        context._mesYACSs[aYACS->GetName()] = aYACS;
        context._idmap[id] = dynamic_cast<PortableServer::ServantBase*>(aYACSServant);
      }
    }
    id++;
  }

  // -> close file
  f.close();

  // Remove temporary files created from the stream
  if (!isMultiFile)
    SALOMEDS_Tool::RemoveTemporaryFiles(tmpDir.c_str(), aFileSeq, true);

  return true;
};

//===========================================================================
CORBA::Boolean HOMARD_Gen_i::LoadASCII(SALOMEDS::SComponent_ptr theComponent,
					const SALOMEDS::TMPFile& theStream,
					const char* theURL,
					CORBA::Boolean isMultiFile)
{
  // No specific ASCII persistence
  return Load(theComponent, theStream, theURL, isMultiFile);
};

//===========================================================================
void HOMARD_Gen_i::Close(SALOMEDS::SComponent_ptr theComponent)
{
};

//===========================================================================
char* HOMARD_Gen_i::ComponentDataType()
{
  return CORBA::string_dup("HOMARD");
};

//===========================================================================
char* HOMARD_Gen_i::IORToLocalPersistentID(SALOMEDS::SObject_ptr theSObject,
					    const char* IORString,
					    CORBA::Boolean isMultiFile,
					    CORBA::Boolean isASCII)
{
  CORBA::String_var aString("");
  if (!CORBA::is_nil(theSObject) && strcmp(IORString, "") != 0) {
    StudyContext context = myStudyContext;
    CORBA::Object_var anObj = _orb->string_to_object(IORString);
    if (!CORBA::is_nil(anObj)) {
      PortableServer::ServantBase_var aServant = GetServant(anObj);
      PortableServer::ServantBase* aStorable = dynamic_cast<PortableServer::ServantBase*>(aServant.in());
      if (aStorable) {
	std::map<int, PortableServer::ServantBase*>::const_iterator it;
	for (it = context._idmap.begin(); it != context._idmap.end(); ++it) {
	  if (it->second == aStorable) {
	    std::stringstream os;
	    os << it->first;
	    aString = CORBA::string_dup(os.str().c_str());
	  }
	}
      }
    }
  }
  return aString._retn();
};

//===========================================================================
char* HOMARD_Gen_i::LocalPersistentIDToIOR(SALOMEDS::SObject_ptr theSObject,
					    const char* aLocalPersistentID,
					    CORBA::Boolean isMultiFile,
					    CORBA::Boolean isASCII)
{
  CORBA::String_var aString("");
  if (!CORBA::is_nil(theSObject) && strcmp(aLocalPersistentID, "") != 0) {
    StudyContext context = myStudyContext;
    int id = atoi(aLocalPersistentID);
    if (id > 0 && context._idmap.find(id) != context._idmap.end()) {
      CORBA::Object_var object = _poa->servant_to_reference(context._idmap[ id ]);
      if (!CORBA::is_nil(object)) {
	aString = _orb->object_to_string(object);
      }
    }
  }
  return aString._retn();
};

//===========================================================================
CORBA::Boolean HOMARD_Gen_i::CanPublishInStudy(CORBA::Object_ptr theIOR)
{
  if(CORBA::is_nil(myStudy))
    return false;

  HOMARD::HOMARD_Cas_var aCas = HOMARD::HOMARD_Cas::_narrow(theIOR);
  if(!aCas->_is_nil())
    return true;

  HOMARD::HOMARD_Hypothesis_var aHypo = HOMARD::HOMARD_Hypothesis::_narrow(theIOR);
  if(!aHypo->_is_nil())
    return true;

  HOMARD::HOMARD_Zone_var aZone = HOMARD::HOMARD_Zone::_narrow(theIOR);
  if(!aZone->_is_nil())
    return true;

  HOMARD::HOMARD_Boundary_var aBoundary = HOMARD::HOMARD_Boundary::_narrow(theIOR);
  if(!aBoundary->_is_nil())
    return true;

  /* Iteration is not published directly
  HOMARD::HOMARD_Iteration_var aIter = HOMARD::HOMARD_Iteration::_narrow(theIOR);
  if(!aIter->_is_nil())
    return true;
  */
  return false;
};

//===========================================================================
CORBA::Boolean HOMARD_Gen_i::CanCopy(SALOMEDS::SObject_ptr theObject)
{
  // No Copy/Paste support
  return false;
};

//===========================================================================
SALOMEDS::TMPFile* HOMARD_Gen_i::CopyFrom(SALOMEDS::SObject_ptr theObject,
					   CORBA::Long& theObjectID)
{
  // No Copy/Paste support
  SALOMEDS::TMPFile_var aStreamFile = new SALOMEDS::TMPFile(0);
  return aStreamFile._retn();
};

//===========================================================================
CORBA::Boolean  HOMARD_Gen_i::CanPaste(const char *theComponentName,
					CORBA::Long theObjectID)
{
  // No Copy/Paste support
  return false;
};

//===========================================================================
SALOMEDS::SObject_ptr HOMARD_Gen_i::PasteInto(const SALOMEDS::TMPFile& theStream,
					       CORBA::Long theObjectID,
					       SALOMEDS::SObject_ptr theSObject)
{
  // No Copy/Paste support
  SALOMEDS::SObject_var aResultSO;
  return aResultSO._retn();
};

//===========================================================================
PortableServer::ServantBase_var HOMARD_Gen_i::GetServant(CORBA::Object_ptr theObject)
{
  PortableServer::Servant aServant = 0;
  if (!CORBA::is_nil(theObject)) {
    try {
      aServant = _poa->reference_to_servant(theObject);
    }
    catch (...) {
    }
  }
  return aServant;
}

//==========================================================================
Engines::TMPFile* HOMARD_Gen_i::DumpPython(CORBA::Boolean isPublished,
                                           CORBA::Boolean isMultiFile,
                                           CORBA::Boolean& isValidScript)
{
   MESSAGE ("Entree dans DumpPython");
   isValidScript=1;

   SALOMEDS::SObject_var aSO = SMESH_Gen_i::getStudyServant()->FindComponent("HOMARD");
   if(CORBA::is_nil(aSO))
      return new Engines::TMPFile(0);

   std::string aScript = "\"\"\"\n";
   aScript += "Python script for HOMARD\n";
   aScript += "Copyright 1996, 2011, 2015 EDF\n";
   aScript += "\"\"\"\n";
   aScript += "__revision__ = \"V1.2\"\n";
   aScript += "import HOMARD\n";
   if( isMultiFile )
      aScript += "import salome\n";
   aScript += "homard = salome.lcc.FindOrLoadComponent('FactoryServer','HOMARD')\n";
   aScript += "\thomard.UpdateStudy()\n";
   MESSAGE (". Au depart \n"<<aScript);


   if (myStudyContext._mesBoundarys.size() > 0)
   {
    MESSAGE (". Ecritures des frontieres");
    aScript += "#\n# Creation of the boundaries";
    aScript +=  "\n# ==========================";
   }
   std::map<std::string, HOMARD::HOMARD_Boundary_var>::const_iterator it_boundary;
   for (it_boundary  = myStudyContext._mesBoundarys.begin();
        it_boundary != myStudyContext._mesBoundarys.end(); ++it_boundary)
   {
    HOMARD::HOMARD_Boundary_var maBoundary = (*it_boundary).second;
    CORBA::String_var dumpCorbaBoundary = maBoundary->GetDumpPython();
    std::string dumpBoundary = dumpCorbaBoundary.in();
    MESSAGE (dumpBoundary<<"\n");
    aScript += dumpBoundary;
   }


   if (myStudyContext._mesZones.size() > 0)
   {
    MESSAGE (". Ecritures des zones");
    aScript += "#\n# Creation of the zones";
    aScript +=  "\n# =====================";
   }
   std::map<std::string, HOMARD::HOMARD_Zone_var>::const_iterator it_zone;
   for ( it_zone  = myStudyContext._mesZones.begin();
         it_zone != myStudyContext._mesZones.end(); ++it_zone)
   {
    HOMARD::HOMARD_Zone_var myZone = (*it_zone).second;
    CORBA::String_var dumpCorbaZone = myZone->GetDumpPython();
    std::string dumpZone = dumpCorbaZone.in();
    MESSAGE (dumpZone<<"\n");
    aScript += dumpZone;
   }


   if (myStudyContext._mesHypotheses.size() > 0)
   {
    MESSAGE (". Ecritures des hypotheses");
    aScript += "#\n# Creation of the hypotheses";
    aScript +=  "\n# ==========================";
   }
   std::map<std::string, HOMARD::HOMARD_Hypothesis_var>::const_iterator it_hypo;
   for ( it_hypo  = myStudyContext._mesHypotheses.begin();
         it_hypo != myStudyContext._mesHypotheses.end(); it_hypo++)
   {
    HOMARD::HOMARD_Hypothesis_var monHypo = (*it_hypo).second;
    CORBA::String_var dumpCorbaHypo = monHypo->GetDumpPython();
    std::string dumpHypo = dumpCorbaHypo.in();
    MESSAGE (dumpHypo<<"\n");
    aScript += dumpHypo;
   }


   if (myStudyContext._mesCas.size() > 0)
   {
    MESSAGE (". Ecritures des cas");
    aScript += "#\n# Creation of the cases";
    aScript += "\n# =====================";
   }
   std::map<std::string, HOMARD::HOMARD_Cas_var>::const_iterator it_cas;
   for (it_cas  = myStudyContext._mesCas.begin();
        it_cas != myStudyContext._mesCas.end(); it_cas++)
        {
           std::string nomCas = (*it_cas).first;
           std::string dumpCas = std::string("\n# Creation of the case ") ;
           dumpCas +=  nomCas + std::string("\n");
           dumpCas += std::string("\t") + nomCas;
           dumpCas += std::string(" = homard.CreateCase(\"") + nomCas + std::string("\", \"");

           HOMARD::HOMARD_Cas_var myCase = (*it_cas).second;
           CORBA::String_var cIter0= myCase->GetIter0Name();
           std::string iter0 = cIter0.in();

           HOMARD::HOMARD_Iteration_var myIteration = myStudyContext._mesIterations[iter0];
           CORBA::String_var cMesh0= myIteration->GetMeshFile();
           std::string mesh0 = cMesh0.in();
           CORBA::String_var cMeshName0= myIteration->GetMeshName();
           std::string meshName0 = cMeshName0.in();
           dumpCas += meshName0 + std::string("\", \"")+ mesh0 + std::string("\")\n");
           CORBA::String_var dumpCorbaCase = myCase->GetDumpPython();
           std::string dumpCas2= dumpCorbaCase.in();

           MESSAGE (dumpCas<<dumpCas2<<"\n");
           aScript += dumpCas + dumpCas2;
        };


   if (myStudyContext._mesIterations.size() > 0)
   {
    MESSAGE (". Ecritures des iterations");
    aScript += "#\n# Creation of the iterations" ;
    aScript += "\n# ==========================";
   }
   std::map<std::string, HOMARD::HOMARD_Iteration_var>::const_iterator it_iter;
   for (it_iter  = myStudyContext._mesIterations.begin();
        it_iter != myStudyContext._mesIterations.end(); ++it_iter)
   {
    HOMARD::HOMARD_Iteration_var aIter = (*it_iter).second;
    CORBA::String_var dumpCorbaIter = aIter->GetDumpPython();
    std::string dumpIter = dumpCorbaIter.in();
    MESSAGE (dumpIter<<"\n");
    aScript += dumpIter;
   }


   if (myStudyContext._mesYACSs.size() > 0)
   {
    MESSAGE (". Ecritures des schemas YACS");
    aScript += "#\n# Creation of the schemas YACS";
    aScript +=  "\n# ============================";
   }
   std::map<std::string, HOMARD::HOMARD_YACS_var>::const_iterator it_yacs;
   for ( it_yacs  = myStudyContext._mesYACSs.begin();
         it_yacs != myStudyContext._mesYACSs.end(); ++it_yacs)
   {
    HOMARD::HOMARD_YACS_var myYACS = (*it_yacs).second;
    CORBA::String_var dumpCorbaYACS = myYACS->GetDumpPython();
    std::string dumpYACS = dumpCorbaYACS.in();
    MESSAGE (dumpYACS<<"\n");
    aScript += dumpYACS;
   }

  MESSAGE (". Ecritures finales");
  if( isMultiFile )
    aScript += "\n\tpass";
  aScript += "\n";

  if( !isMultiFile ) // remove unnecessary tabulation
    aScript = RemoveTabulation( aScript );

//   MESSAGE ("A ecrire \n"<<aScript);
  const size_t aLen = strlen(aScript.c_str());
  char* aBuffer = new char[aLen+1];
  strcpy(aBuffer, aScript.c_str());

  CORBA::Octet* anOctetBuf =  (CORBA::Octet*)aBuffer;
  Engines::TMPFile_var aStreamFile = new Engines::TMPFile(aLen+1, aLen+1, anOctetBuf, 1);

  MESSAGE ("Sortie de DumpPython");
  return aStreamFile._retn();
}


//=============================================================================
//=============================================================================
// Utilitaires
//=============================================================================
//=============================================================================
void HOMARD_Gen_i::IsValidStudy( )
{
//   MESSAGE( "IsValidStudy" );
  if (CORBA::is_nil(SMESH_Gen_i::getStudyServant()))
  {
    SALOME::ExceptionStruct es;
    es.type = SALOME::BAD_PARAM;
    es.text = "Invalid study context";
    throw SALOME::SALOME_Exception(es);
  };
  return ;
}

//=============================================================================
char* HOMARD_Gen_i::VerifieDir(const char* nomDir)
{
  std::string casename = std::string("") ;
  std::map<std::string, HOMARD::HOMARD_Cas_var>::const_iterator it;
  for (it = myStudyContext._mesCas.begin();
  it != myStudyContext._mesCas.end(); it++)
  {
   if (std::string(nomDir) == std::string(it->second->GetDirName()))
   {
     casename = std::string(it->second->GetName()) ;
     break ;
   }
  }
  return CORBA::string_dup( casename.c_str() );
}
/*//=============================================================================
void SALOMEException( std::string message )
{
  SALOME::ExceptionStruct es;
  es.type = SALOME::BAD_PARAM;
  es.text = message;
  throw SALOME::SALOME_Exception(es);
  return ;
}*/
//=============================================================================
char* HOMARD_Gen_i::getVersion()
{
#if HOMARD_DEVELOPMENT
  return CORBA::string_dup(HOMARD_VERSION_STR"dev");
#else
  return CORBA::string_dup(HOMARD_VERSION_STR);
#endif
}
//===============================================================================
// Recuperation de la chaine de caracteres par rapport a l'apparition d'un caractere
// ligne : la ligne a manipuler
// caractere : le caractere a reperer
// option : 0 : la chaine avant la premiere apparition du caractere
//          1 : la chaine apres la premiere apparition du caractere
//          2 : la chaine avant la derniere apparition du caractere
//          3 : la chaine apres la derniere apparition du caractere
// Si le caractere est absent, on retourne la chaine totale
//===============================================================================
std::string HOMARD_Gen_i::GetStringInTexte( const std::string ligne, const std::string caractere, int option )
{
//   MESSAGE("GetStringInTexte, recherche de '"<<caractere<<"' dans '"<<ligne<<"'"<<", option = "<<option);
//
  std::string chaine = ligne ;
  int position ;
  if ( option < 2 ) { position = ligne.find_first_of( caractere ) ; }
  else              { position = ligne.find_last_of( caractere ) ; }
//   MESSAGE("position = "<<position);
//   MESSAGE("a = "<<ligne.substr( 0, position ).c_str());
//   MESSAGE("b = "<<ligne.substr( position+1 ).c_str());
//
  if ( position != std::string::npos )
  {
    if ( ( option == 0 ) || ( option == 2 ) ) { chaine = ligne.substr( 0, position ) ; }
    else                                      { chaine = ligne.substr( position+1 ) ; }
  }
  return chaine ;
//
}
//=============================================================================
//=============================================================================
// Gestion des preferences
//=============================================================================
//=============================================================================
// Decodage du fichier d'arcihvage des preferences
//
void HOMARD_Gen_i::SetPreferences( )
{
  MESSAGE ( "SetPreferences" );

  std::string ligne, mot_cle, salome_version ;
  bool ok = true ;

  // A. Les valeurs par defaut ; elles doivent etre coherentes
  std::string LanguageShort = "en" ;
  int PublisMeshIN = 0 ;
  int PublisMeshOUT = 0 ;
  int YACSMaxIter = 0 ;
  int YACSMaxNode = 0 ;
  int YACSMaxElem = 0 ;
  std::string YACSTypeTestchaine = "None" ;

  // B. La version de salome
  // Cela se presente sous la forme :
  // [SALOME KERNEL] : 7.3.0
  std::string File ;
  File  = getenv("KERNEL_ROOT_DIR") ;
  File += "/bin/salome/VERSION" ;
  MESSAGE ( "File = "<<File ) ;
  std::ifstream fichier0( File.c_str() ) ;
  if ( fichier0 ) // ce test échoue si le fichier n'est pas ouvert
  {
    std::string ligne; // variable contenant chaque ligne lue
    while ( std::getline( fichier0, ligne ) )
    {
      std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
      ligne_bis >> mot_cle ;
      if ( mot_cle == "[SALOME" )
      {
        salome_version = GetStringInTexte ( ligne, " ", 3 ) ;
//         MESSAGE ( "salome_version = "<<salome_version<<"|||");
        break ;
      }
    }
  }
  else { ok = false ; }

  // B. Decodage du fichier de preferences
  if ( ok )
  {
    std::string PrefFile ;
    PrefFile  = Kernel_Utils::HomePath() ;
    PrefFile += "/.config/salome/SalomeApprc." + salome_version ;
    MESSAGE ( "PrefFile = "<<PrefFile ) ;

    std::ifstream fichier( PrefFile.c_str() );
    if ( fichier ) // ce test échoue si le fichier n'est pas ouvert
    {
      bool section_langue = false ;
      bool section_homard = false ;
      while ( std::getline( fichier, ligne ) )
      {
        std::string chaine ;
        // 1. Pour la ligne courante, on identifie le premier mot : le mot-cle eventuel
        std::istringstream ligne_bis(ligne); // variable contenant chaque ligne sous forme de flux
        ligne_bis >> mot_cle ;

        // 2. Les sections
        // 2.1. Debut d'une section
  //       MESSAGE(mot_cle);
        if ( mot_cle == "<section" )
        { /*MESSAGE ( "Debut de la section : "<< ligne);*/
          ligne_bis >> mot_cle ;
          chaine = GetStringInTexte ( mot_cle, "\"", 1 ) ;
          chaine = GetStringInTexte ( chaine,  "\"", 0 ) ;
          if ( chaine == "language" ) { section_langue = true ; }
          if ( chaine == "HOMARD" )   { section_homard = true ; }
  //         MESSAGE ( "section_langue = "<<section_langue<<", section_homard = "<<section_homard);
        }
        // 2.2. Fin d'une section
        else if ( mot_cle == "</section>" )
        { /*MESSAGE ( "Fin de la section : "<< ligne<<", section_langue = "<<section_langue<<", section_homard = "<<section_homard);*/
          section_langue = false ;
          section_homard = false ; }

        // 3. Parametres
        // 3.1. La langue
        else if ( section_langue || section_homard )
        { MESSAGE ( "a decoder : "<< ligne);
//        La valeur : entre les deux premieres quotes
          chaine = GetStringInTexte ( ligne, "\"", 1 ) ;
//           MESSAGE("chaine 1 = |"<<chaine<<"|");
          chaine = GetStringInTexte ( chaine,  "\"", 0 ) ;
//           MESSAGE("chaine = |"<<chaine<<"|");
//        Le mot_cle : entre les deux dernieres quotes
          std::string chaine2 = GetStringInTexte ( ligne, "\"", 2 ) ;
//           MESSAGE("chaine2 1 = |"<<chaine2<<"|");
          chaine2 = GetStringInTexte ( chaine2,  "\"", 3 ) ;
//           MESSAGE("chaine2 = |"<<chaine2<<"|");
          // 3.1. La langue
          if ( section_langue )
          { if ( chaine2 == "language" ) { LanguageShort = chaine ; } }
          // 3.2. HOMARD
          if ( section_homard )
          {
            std::istringstream chainebis( chaine ) ;
            // 3.2.1. Les publications
            if ( chaine2 == "publish_mesh_in" )  { chainebis >> PublisMeshIN ; }
            if ( chaine2 == "publish_mesh_out" ) { chainebis >> PublisMeshOUT ; }
            // 3.2.2. Les maximum pour YACS
            if ( chaine2 == "yacs_max_iter" ) { chainebis >> YACSMaxIter ; }
            if ( chaine2 == "yacs_max_node" ) { chainebis >> YACSMaxNode ; }
            if ( chaine2 == "yacs_max_elem" ) { chainebis >> YACSMaxElem ; }
            if ( chaine2 == "yacs_type_test" ) { YACSTypeTestchaine = chaine ; }
          }
        }
      }
    }
  }

  // C. Enregistrements
  MESSAGE ("Enregistrement de LanguageShort = " << LanguageShort );
  SetLanguageShort( LanguageShort.c_str() ) ;

  MESSAGE ("Enregistrement de PublisMeshIN = " << PublisMeshIN<<", PublisMeshOUT = "<< PublisMeshOUT);
  SetPublisMesh(PublisMeshIN, PublisMeshOUT) ;

  MESSAGE ("Enregistrement de YACSMaxIter = " << YACSMaxIter<<", YACSMaxNode = "<< YACSMaxNode<<", YACSMaxElem = "<< YACSMaxElem);
  SetYACSMaximum(YACSMaxIter, YACSMaxNode, YACSMaxElem) ;

  MESSAGE ("Enregistrement de TypeTest = " << YACSTypeTestchaine.c_str() );
  int YACSTypeTest ;
  if ( ( YACSTypeTestchaine == "VTest > VRef" ) || ( YACSTypeTestchaine == "VTest &gt; VRef" ) )      { YACSTypeTest = 1 ; }
  else if ( ( YACSTypeTestchaine == "VTest < VRef" ) || ( YACSTypeTestchaine == "VTest &lt; VRef" ) ) { YACSTypeTest = 2 ; }
  else                                                                                                { YACSTypeTest = 0 ; }
  MESSAGE ("==> TypeTest = " << YACSTypeTest );
  SetYACSConvergenceType( YACSTypeTest ) ;

  return ;
}
//===============================================================================
// Langue de SALOME
//===============================================================================
void HOMARD_Gen_i::SetLanguageShort(const char* LanguageShort)
{
//   MESSAGE ("SetLanguageShort pour LanguageShort = " << LanguageShort );
  _LangueShort = LanguageShort ;
  if ( _LangueShort == "fr" ) { _Langue = "Francais" ; }
  else                        { _Langue = "English" ; }
  return ;
}
char* HOMARD_Gen_i::GetLanguageShort()
{
//   MESSAGE ("GetLanguageShort");
  return CORBA::string_dup( _LangueShort.c_str() );
}
//===============================================================================
// Options de publications
//===============================================================================
void HOMARD_Gen_i::SetPublisMesh(CORBA::Long PublisMeshIN, CORBA::Long PublisMeshOUT)
{
  _PublisMeshIN  = PublisMeshIN  ;
  _PublisMeshOUT = PublisMeshOUT ;
  return ;
}
CORBA::Long HOMARD_Gen_i::GetPublisMeshIN()
{
  return _PublisMeshIN ;
}
CORBA::Long HOMARD_Gen_i::GetPublisMeshOUT()
{
  return _PublisMeshOUT ;
}
//===============================================================================
// YACS - test de convergence
//===============================================================================
void HOMARD_Gen_i::SetYACSMaximum(CORBA::Long YACSMaxIter, CORBA::Long YACSMaxNode, CORBA::Long YACSMaxElem)
{
  _YACSMaxIter = YACSMaxIter ;
  _YACSMaxNode = YACSMaxNode ;
  _YACSMaxElem = YACSMaxElem ;
  return ;
}
CORBA::Long HOMARD_Gen_i::GetYACSMaxIter()
{
  return _YACSMaxIter ;
}
CORBA::Long HOMARD_Gen_i::GetYACSMaxNode()
{
  return _YACSMaxNode ;
}
CORBA::Long HOMARD_Gen_i::GetYACSMaxElem()
{
  return _YACSMaxElem ;
}
void HOMARD_Gen_i::SetYACSConvergenceType(CORBA::Long YACSTypeTest)
{
  _YACSTypeTest = YACSTypeTest ;
  return ;
}
CORBA::Long HOMARD_Gen_i::GetYACSConvergenceType()
{
  return _YACSTypeTest ;
}

//=============================================================================
extern "C"
{
  HOMARDENGINE_EXPORT
  PortableServer::ObjectId* HOMARDEngine_factory(CORBA::ORB_ptr orb,
						  PortableServer::POA_ptr poa,
						  PortableServer::ObjectId* contId,
						  const char* instanceName,
						  const char* interfaceName)
  {
    MESSAGE("PortableServer::ObjectId* HOMARDEngine_factory()");
    HOMARD_Gen_i* myHOMARD_Gen = new HOMARD_Gen_i(orb, poa, contId, instanceName, interfaceName);
    return myHOMARD_Gen->getId();
  }
}
