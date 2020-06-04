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

//  SMESH SMESHGUI : GUI for the adaptation in the SMESH component
//  File   : SMESHGUI_AdaptDlg.cxx
//  Author : Gerald NICOLAS, EDF

//  SMESH includes
#include "SMESHGUI.h"
#include "SMESHGUI_AdaptDlg.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Study.h>

// HOMARD includes
#include "MonCreateCase.h"

// SALOME KERNEL includes
#include "utilities.h"
#include <SALOME_LifeCycleCORBA.hxx>

//=================================================================================
// function : SMESHGUI_AdaptDlg()
// purpose  :
//=================================================================================
SMESHGUI_AdaptDlg::SMESHGUI_AdaptDlg( SMESHGUI* theModule,
                                      int theCommandID,
                                      SMESH::SMESH_Mesh_ptr theMesh )
  : mySMESHGUI( theModule )
{
  bool ok = action( theCommandID ) ;
}

//=================================================================================
// function : ~SMESHGUI_AdaptDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_AdaptDlg::~SMESHGUI_AdaptDlg()
{
}

/*!
  * \brief Pilote les actions d'adaption de maillage
  * \param
  * \return bool OK/notOK
*/
bool SMESHGUI_AdaptDlg::action (int theCommandID)
//=======================================================================
{
  std::cout  << "SMESHGUI_AdaptDlg::action avec theCommandID : " << theCommandID << std::endl;

// Preferences
  recupPreferences();

// Menus and actions
  bool ok =  OnGUIEvent (theCommandID) ;
  if ( ! ok ) INFOS("Erreur");

  return ok ;
}

/*!
  * \brief Gets the preferences for the adaptation
  * \param
  * \return
  *
  * Pour chaque valeur, le defaut est la valeur definie dans ADAPT_Gen
  * . Si la recuperation dans config/salome s'est bien passee a la creation de ADAPT_Gen
  *   ces valeurs sont les valeurs definies.
  * . Si cela ne s'est pas bien passe, ce sont les valeurs par defaut de ADAPT_Gen
*/
void SMESHGUI_AdaptDlg::recupPreferences()
{
  INFOS("Début de recupPreferences")
//
// A. Declarations
//
  SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
  SALOME_LifeCycleCORBA* ls = new SALOME_LifeCycleCORBA(app->namingService());
  Engines::EngineComponent_var comp = ls->FindOrLoad_Component("FactoryServer", "SMESH");
  ADAPT::ADAPT_Gen_var adaptGen = ADAPT::ADAPT_Gen::_narrow(comp);
  if (!CORBA::is_nil(adaptGen))
    adaptGen->UpdateStudy();

  SUIT_ResourceMgr* resMgr = mySMESHGUI->getApp()->resourceMgr();

// B. Les valeurs
// B.1. La langue

  _LanguageShort = resMgr->stringValue("language", "language", "en" );
  INFOS ("Enregistrement de LanguageShort = " << _LanguageShort.toStdString().c_str() );
//   adaptGen->SetLanguageShort(_LanguageShort.toStdString().c_str());

// B.2. Les publications
  bool publish_mesh ;

  publish_mesh = resMgr->booleanValue("HOMARD", "publish_mesh_in", false );
  if ( publish_mesh ) { _PublisMeshIN = 1 ; }
  else                { _PublisMeshIN = 0 ; }

  publish_mesh = resMgr->booleanValue("HOMARD", "publish_mesh_out", false );
  if ( publish_mesh ) { _PublisMeshOUT = 1 ; }
  else                { _PublisMeshOUT = 0 ; }

  INFOS ("Enregistrement de PublisMeshIN = " << _PublisMeshIN<<", PublisMeshOUT = "<< _PublisMeshOUT);
//   adaptGen->SetPublisMesh(_PublisMeshIN, _PublisMeshOUT);

// B.3. Les maximum pour YACS

  _YACSMaxIter = resMgr->integerValue("HOMARD", "yacs_max_iter", 0 );
  _YACSMaxNode = resMgr->integerValue("HOMARD", "yacs_max_node", 0 );
  _YACSMaxElem = resMgr->integerValue("HOMARD", "yacs_max_elem", 0 );

  INFOS ("Enregistrement de YACSMaxIter = " << _YACSMaxIter<<", YACSMaxNode = "<< _YACSMaxNode<<", YACSMaxElem = "<< _YACSMaxElem);
//   adaptGen->SetYACSMaximum(_YACSMaxIter, _YACSMaxNode, _YACSMaxElem);

// B.4. La convergence pour YACS

  QString QString_v = resMgr->stringValue ( "HOMARD", "yacs_type_test", "None" );
  if ( ( QString_v == "VTest > VRef" ) || ( QString_v == "VTest &gt; VRef" ) )      { _YACSTypeTest = 1 ; }
  else if ( ( QString_v == "VTest < VRef" ) || ( QString_v == "VTest &lt; VRef" ) ) { _YACSTypeTest = 2 ; }
  else                                                                              { _YACSTypeTest = 0 ; }

  INFOS ("Enregistrement de YACSTypeTest = " << _YACSTypeTest);
//   adaptGen->SetYACSConvergenceType(_YACSTypeTest);

  INFOS("Fin de recupPreferences")
}

/*!
  * \brief Launches the GUI for the adaptation
  * \param theCommandID - the integer that references the operation
  * \return bool OK/notOK
*/
bool SMESHGUI_AdaptDlg::OnGUIEvent (int theCommandID)
{
  std::cout  << "OnGUIEvent avec theCommandID : " << theCommandID << std::endl;
// A. Controles
  SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
  if ( !app ) return false;

  SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*> ( app->activeStudy() );
  if ( !aStudy )
  {
    INFOS ( "FAILED to cast active study to SalomeApp_Study" );
    return false;
  }

  SUIT_Desktop* parent = SUIT_Session::session()->activeApplication()->desktop();

  SALOME_LifeCycleCORBA* ls = new SALOME_LifeCycleCORBA(app->namingService());
  Engines::EngineComponent_var comp = ls->FindOrLoad_Component("FactoryServer", "SMESH");
  ADAPT::ADAPT_Gen_var adaptGen = ADAPT::ADAPT_Gen::_narrow(comp);
  if (!CORBA::is_nil(adaptGen))
    adaptGen->UpdateStudy();

  mySMESHGUI->getApp()->updateObjectBrowser();
//
// B. Choix selon les commandes
  bool ok = true ;
  SCRUTE(theCommandID);
  switch (theCommandID)
  {
    case 8000: // Raffinement uniforme
    {
      INFOS("Raffinement uniforme");
//       A faire
      break;
    }
    case 8011: // Création d'un Cas
    {
      INFOS("Création d'un Cas");
      MonCreateCase *aDlg = new MonCreateCase( true,
                            ADAPT::ADAPT_Gen::_duplicate(adaptGen) ) ;
      aDlg->show();
      break;
    }
    case 8012: // Création d'un cas en poursuite d'une itération
    {
      INFOS("Création d'un cas en poursuite d'une itération");
//       MonPursueIteration *aDlg = new MonPursueIteration( true,
//                                 ADAPT::ADAPT_Gen::_duplicate(adaptGen) ) ;
//       aDlg->show();
//       break;
    }
    case 8013: // Création d'une itération
    {
      INFOS("Création d'une itération" );
//       MonCreateIteration *IterDlg = new MonCreateIteration( parent, true,
//                                      ADAPT::ADAPT_Gen::_duplicate(adaptGen), _ObjectName ) ;
//       IterDlg->show();
      break;
    }
    case 8014: // Calcule une itération
    {
      INFOS("Calcule une itération" );
//       try { adaptGen->Compute(_ObjectName.toStdString().c_str(), 0, 1, -1, 1); }
//       catch( SALOME::SALOME_Exception& S_ex )
//       {
//         QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
//                                   QObject::tr(CORBA::string_dup(S_ex.details.text)) );
//         mySMESHGUI->getApp()->updateObjectBrowser();
//         ok = false;
//       }
      break;
    }
    case 8015: // Calcule une itération et publication
    {
      INFOS("Calcule une itération et publication");
//       try { adaptGen->Compute(_ObjectName.toStdString().c_str(), 0, 1, -1, 2); }
//       catch( SALOME::SALOME_Exception& S_ex )
//       {
//         QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
//                                   QObject::tr(CORBA::string_dup(S_ex.details.text)) );
//         mySMESHGUI->getApp()->updateObjectBrowser();
//         ok = false;
//       }
      break;
    }
    case 8020: // Adaptation avec MG-Adpat
    {
      INFOS("Interface avec MG-Adapt" );
//       A faire
      break;
    }

  }
  mySMESHGUI->getApp()->updateObjectBrowser();
  return ok;
}



