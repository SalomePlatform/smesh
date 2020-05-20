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

//  File   : HOMARDGUI.cxx
//  Author : Gerald NICOLAS, EDF
//  Module : HOMARD

#include "HOMARDGUI.h"

// SALOME Includes
#include "Utils_ORB_INIT.hxx"
#include "Utils_SINGLETON.hxx"
#include "SALOME_LifeCycleCORBA.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_MessageBox.h"
#include "SUIT_Session.h"
#include "SUIT_ViewWindow.h"
#include "SUIT_ViewManager.h"
#include <SUIT_Desktop.h>

#include "CAM_Module.h"
#include "OB_Browser.h"

#include "SALOME_ListIO.hxx"

#include "SalomeApp_Application.h"
#include "SalomeApp_DataModel.h"
#include "SalomeApp_Study.h"
#include "LightApp_SelectionMgr.h"
#include "LightApp_Selection.h"
#include <LightApp_Preferences.h>
#include "SalomeApp_Module.h"
#include "SALOMEconfig.h"
#include <SALOME_LifeCycleCORBA.hxx>

#include <utilities.h>


// QT Includes
#include <QMenu>
#include "MonCreateCase.h"
#include "MonCreateIteration.h"
#include "MonPursueIteration.h"
#include "MonCreateYACS.h"
#include "MonEditBoundaryCAO.h"
#include "MonEditBoundaryAn.h"
#include "MonEditBoundaryDi.h"
#include "MonEditCase.h"
#include "MonEditHypothesis.h"
#include "MonEditIteration.h"
#include "MonEditYACS.h"
#include "MonEditZone.h"
#include "MonMeshInfo.h"
#include "MonIterInfo.h"
#include "MonEditFile.h"
#include "HomardQtCommun.h"

// BOOST Includes
#include <boost/shared_ptr.hpp>

//Pour le _CAST
#include "SALOMEDS_Study.hxx"
#include "HOMARDGUI_Utils.h"

using namespace std;

static CORBA::ORB_var _orb;

//=======================================================================
// function : HOMARDGUI()
// purpose  : Constructor
//=======================================================================
HOMARDGUI::HOMARDGUI(const QString&) :
           SalomeApp_Module( "HOMARD" ) // default name
{
}
//=======================================================================
// function : ~HOMARDGUI()
// purpose  : Destructor
//=======================================================================
HOMARDGUI::~HOMARDGUI()
{
}

//=======================================================================
// function : InitHOMARDGen
// launch HOMARD component and return a handle
//=======================================================================
HOMARD::HOMARD_Gen_var HOMARDGUI::InitHOMARDGen(SalomeApp_Application* app)
{
  Engines::EngineComponent_var comp = app->lcc()->FindOrLoad_Component( "FactoryServer","HOMARD" );
  HOMARD::HOMARD_Gen_var clr = HOMARD::HOMARD_Gen::_narrow(comp);
  ASSERT(!CORBA::is_nil(clr));
  return clr;
}

//=======================================================================
// Module's initialization
void HOMARDGUI::initialize( CAM_Application* app )
//=======================================================================
{
  SalomeApp_Module::initialize( app );
  InitHOMARDGen(dynamic_cast<SalomeApp_Application*>( app ));
  anId = 0;
  createActions();
  createMenus();
  recupPreferences();
}

//================================================
// function : createHOMARDAction
// create an item in status bar and Homard menu
//================================================
void HOMARDGUI::createHOMARDAction( const int id, const QString& po_id, const QString& icon_id, const int key, const bool toggle  )
{
//   MESSAGE("createHOMARDAction");
  QIcon icon;
  QWidget* parent = application()->desktop();
  SUIT_ResourceMgr* resMgr = application()->resourceMgr();
  QPixmap pix;
  if ( icon_id.length() )
     pix = resMgr->loadPixmap( "HOMARD", tr( icon_id .toLatin1().data()) );
  else
     pix = resMgr->loadPixmap( "HOMARD", tr( QString( "ICO_%1" ).arg( po_id ).toLatin1().data()), false );
  if ( !pix.isNull() )
      icon = QIcon( pix );

  QString tooltip    = tr(QString( "HOM_TOP_%1" ).arg( po_id ).toLatin1().data()),
          menu       = tr(QString( "HOM_MEN_%1" ).arg( po_id ).toLatin1().data()),
          status_bar = tr(QString( "HOM_STB_%1" ).arg( po_id ).toLatin1().data());

  createAction( id, tooltip, icon, menu, status_bar, key, parent, toggle, this, SLOT( OnGUIEvent() )  );
}

//================================================
// function : createAction
// constructs Homard menu
// calls createHOMARDAction for each item
//================================================
void HOMARDGUI::createActions(){
//
  createHOMARDAction( 1101, "NEW_CASE",         "cas_calcule.png"        );
  createHOMARDAction( 1102, "PURSUE_ITERATION", "iter_poursuite.png"     );
  createHOMARDAction( 1103, "NEW_ITERATION",    "iter_next.png"          );
  createHOMARDAction( 1111, "COMPUTE",          "mesh_compute.png"       );
  createHOMARDAction( 1112, "COMPUTE_PUBLISH",  "mesh_compute.png"       );
  createHOMARDAction( 1121, "MESH_INFO",        "advanced_mesh_info.png" );
  createHOMARDAction( 1131, "MESH_PUBLICATION", "mesh_tree_mesh.png"     );
//
  createHOMARDAction( 1201, "EDIT",             "loop.png"         );
  createHOMARDAction( 1211, "DELETE",           "delete.png"       );
//
  createHOMARDAction( 1301, "MESH_INFO",        "advanced_mesh_info.png" );
  createHOMARDAction( 1302, "EDIT_MESS_FILE",   "texte.png"              );
//
  createHOMARDAction( 1401, "YACS",             "table_view.png" );
//
}

//================================================
// function : createPreferences
//================================================
void HOMARDGUI::createPreferences()
{
  MESSAGE("createPreferences")

  int Onglet, Bloc, Pref ;
  // 1. Generalites
  Onglet = addPreference( tr( "PREF_TAB_GENERAL" ) );
//   Onglet = addPreference( tr( "PREF_TAB_SETTINGS" ) ) ;

  Bloc = addPreference( tr( "PREF_PUBLICATION" ), Onglet );
  setPreferenceProperty( Bloc, "columns", 1 );

  Pref = addPreference( tr( "PREF_PUBLICATION_MAILLAGE_IN" ), Bloc, LightApp_Preferences::Bool, "HOMARD", "publish_mesh_in" );

  Pref = addPreference( tr( "PREF_PUBLICATION_MAILLAGE_OUT" ), Bloc, LightApp_Preferences::Bool, "HOMARD", "publish_mesh_out" );

  // 2. YACS
  Onglet = addPreference( tr( "PREF_YACS" ) ) ;

  Bloc = addPreference( tr( "PREF_YACS_MAX" ), Onglet );
  setPreferenceProperty( Bloc, "columns", 1 );

  Pref = addPreference( tr( "PREF_YACS_MAX_ITER" ), Bloc, LightApp_Preferences::IntSpin, "HOMARD", "yacs_max_iter" );
  setPreferenceProperty( Pref, "min",  0 );
  setPreferenceProperty( Pref, "max",  100000000 );
  setPreferenceProperty( Pref, "step", 1 );

  Pref = addPreference( tr( "PREF_YACS_MAX_NODE" ), Bloc, LightApp_Preferences::IntSpin, "HOMARD", "yacs_max_node" );
  setPreferenceProperty( Pref, "min",  0 );
  setPreferenceProperty( Pref, "max",  100000000 );
  setPreferenceProperty( Pref, "step", 1000 );

  Pref = addPreference( tr( "PREF_YACS_MAX_ELEM" ), Bloc, LightApp_Preferences::IntSpin, "HOMARD", "yacs_max_elem" );
  setPreferenceProperty( Pref, "min",  0 );
  setPreferenceProperty( Pref, "max",  100000000 );
  setPreferenceProperty( Pref, "step", 1000 );

  Bloc = addPreference( tr( "PREF_YACS_CONVERGENCE" ), Onglet );
  setPreferenceProperty( Bloc, "columns", 1 );

  Pref = addPreference( tr( "PREF_YACS_TYPE_TEST" ), Bloc, LightApp_Preferences::Selector, "HOMARD", "yacs_type_test" );
  QStringList aListOfTypeTest;
  aListOfTypeTest << "None";
  aListOfTypeTest << "VTest > VRef";
  aListOfTypeTest << "VTest < VRef";
  setPreferenceProperty( Pref, "strings", aListOfTypeTest );
}


//================================================
// function : createMenus
//================================================
void HOMARDGUI::createMenus()
{
  MESSAGE("createMenus")
//
  int HOMARD_Id  = createMenu( tr( "HOM_MEN_HOMARD" ),  -1,  5, 10 );
  createMenu( 1101, HOMARD_Id, -1 ); //Create_Case
  createMenu( 1102, HOMARD_Id, -1 ); //Pursue_Iteration
  createMenu( separator(), HOMARD_Id,-1);
  createMenu( 1103, HOMARD_Id, -1 ); //Create_Iteration
  createMenu( 1111, HOMARD_Id, -1 ); //Compute
  createMenu( 1112, HOMARD_Id, -1 ); //Compute and publish
//
  HOMARD_Id  = createMenu( tr( "HOM_MEN_MODIFICATION" ),  -1,  5, 10 );
  createMenu( 1201, HOMARD_Id, -1 ); //Edit
  createMenu( 1211, HOMARD_Id, -1 ); //Delete
//
  HOMARD_Id  = createMenu( tr( "HOM_MEN_INFORMATION" ),  -1,  5, 10 );
  createMenu( 1301, HOMARD_Id, -1 ); //Information sur un maillage
  createMenu( 1131, HOMARD_Id, -1 ); //Mesh publication
  createMenu( separator(), HOMARD_Id,-1);
  createMenu( 1302, HOMARD_Id, -1 ); //EditAsciiFile pour le fichier listeStd ou bilan
  createMenu( separator(), HOMARD_Id,-1);
  createMenu( 1201, HOMARD_Id, -1 ); //Edit
  createMenu( separator(), HOMARD_Id,-1);
//
  HOMARD_Id  = createMenu( tr( "HOM_MEN_YACS" ),  -1,  5, 10 );
  createMenu( 1401, HOMARD_Id, -1 ); // Création d'un schéma YACS
  createMenu( separator(), HOMARD_Id,-1);
}

//================================================
// function : recupPreferences
// Pour chaque valeur, le defaut est la valeur definie dans HOMARD_Gen
// . Si la recuperation dans config/salome s'est bien passee a la creation de HOMARD_Gen,
//   ces valeurs sont les valeurs definies.
// . Si cela ne s'est pas bien passe, ce sont les valeurs par defaut de HOMARD_Gen
//================================================
void HOMARDGUI::recupPreferences()
{
  MESSAGE("recupPreferences")
//
// A. Declarations
//
  SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( application() );
  HOMARD::HOMARD_Gen_var homardGen = HOMARDGUI::InitHOMARDGen(app);
  int defaut_i ;
  std::string defaut_s ;
  QString QString_v ;
//
// B. Les valeurs
// B.1. La langue
//
  defaut_s = homardGen->GetLanguageShort();
  SUIT_ResourceMgr* resMgr = getApp()->resourceMgr();
  _LanguageShort = resMgr->stringValue("language", "language", QString(defaut_s.c_str()) );
//
// B.2. Les publications
  bool publish_mesh ;
//
  _PublisMeshIN = homardGen->GetPublisMeshIN();
  if ( _PublisMeshIN == 1 ) { publish_mesh = true ;  }
  else                      { publish_mesh = false ; }
  publish_mesh = resMgr->booleanValue("HOMARD", "publish_mesh_in", publish_mesh );
  if ( publish_mesh ) { _PublisMeshIN = 1 ; }
  else                { _PublisMeshIN = 0 ; }
//
  _PublisMeshOUT = homardGen->GetPublisMeshOUT();
  if ( _PublisMeshOUT == 1 ) { publish_mesh = true ;  }
  else                       { publish_mesh = false ; }
  publish_mesh = resMgr->booleanValue("HOMARD", "publish_mesh_out", publish_mesh );
  if ( publish_mesh ) { _PublisMeshOUT = 1 ; }
  else                { _PublisMeshOUT = 0 ; }
//
// B.3. Les maximum pour YACS
//
  defaut_i = homardGen->GetYACSMaxIter();
  _YACSMaxIter = resMgr->integerValue("HOMARD", "yacs_max_iter", defaut_i );
//
  defaut_i = homardGen->GetYACSMaxNode();
  _YACSMaxNode = resMgr->integerValue("HOMARD", "yacs_max_node", defaut_i );
//
  defaut_i = homardGen->GetYACSMaxElem();
  _YACSMaxElem = resMgr->integerValue("HOMARD", "yacs_max_elem", defaut_i );
//
// B.4. La convergence pour YACS
//
  defaut_i = homardGen->GetYACSConvergenceType();
  if ( defaut_i == 1 )      { QString_v = tr("VTest > VRef") ; }
  else if ( defaut_i == 2 ) { QString_v = tr("VTest < VRef") ; }
  else                      { QString_v = tr("None") ; }
  QString_v = resMgr->stringValue ( "HOMARD", "yacs_type_test", QString_v );
  if ( ( QString_v == "VTest > VRef" ) || ( QString_v == "VTest &gt; VRef" ) )      { _YACSTypeTest = 1 ; }
  else if ( ( QString_v == "VTest < VRef" ) || ( QString_v == "VTest &lt; VRef" ) ) { _YACSTypeTest = 2 ; }
  else                                                                              { _YACSTypeTest = 0 ; }
//
// C. Enregistrement dans l'objet general
//
  MESSAGE ("Enregistrement de LanguageShort = " << _LanguageShort.toStdString().c_str() );
  MESSAGE ("Enregistrement de PublisMeshIN = " << _PublisMeshIN<<", PublisMeshOUT = "<< _PublisMeshOUT);
  MESSAGE ("Enregistrement de YACSMaxIter = " << _YACSMaxIter<<", YACSMaxNode = "<< _YACSMaxNode<<", YACSMaxElem = "<< _YACSMaxElem);
  MESSAGE ("Enregistrement de YACSTypeTest = " << _YACSTypeTest);
//
  homardGen->SetLanguageShort(_LanguageShort.toStdString().c_str());
  homardGen->SetPublisMesh(_PublisMeshIN, _PublisMeshOUT);
  homardGen->SetYACSMaximum(_YACSMaxIter, _YACSMaxNode, _YACSMaxElem);
//
  homardGen->SetYACSConvergenceType(_YACSTypeTest);
}

//================================================
void HOMARDGUI::OnGUIEvent()
//================================================
{
  MESSAGE("OnGUIEvent()")
  setOrb();
  const QObject* obj = sender();
  if ( !obj || !obj->inherits( "QAction" ) ) { return; }
  int id = actionId((QAction*)obj);
  if ( id != -1 ) { bool ret = OnGUIEvent( id ); }
  MESSAGE("Fin de OnGUIEvent()");
}

//=======================================================================
// Method OnGUIEvent pour Homard
//=======================================================================
bool HOMARDGUI::OnGUIEvent (int theCommandID)
{
  MESSAGE("OnGUIEvent avec theCommandID = "<<theCommandID);
// A. Controles
  SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( application() );
  if ( !app ) return false;

  SalomeApp_Study* stud = dynamic_cast<SalomeApp_Study*> ( app->activeStudy() );
  if ( !stud )
  {
    MESSAGE ( "FAILED to cast active study to SalomeApp_Study" );
    return false;
  }

  SUIT_Desktop* parent = application()->desktop();

  HOMARD::HOMARD_Gen_var homardGen = HOMARDGUI::InitHOMARDGen(app);

  if (!CORBA::is_nil(homardGen))
    homardGen->UpdateStudy();

  getApp()->updateObjectBrowser();

// B. Choix selon les commandes
  SCRUTE(theCommandID);
  switch (theCommandID)
  {
    case 1101: // Creation d un Cas
    {
      MESSAGE("command " << theCommandID << " activated");
      MonCreateCase *aDlg = new MonCreateCase( true,
                            HOMARD::HOMARD_Gen::_duplicate(homardGen) ) ;
      aDlg->show();
      break;
    }

    case 1102: // Poursuite d une iteration
    {
      MESSAGE("command " << theCommandID << " activated");
      MonPursueIteration *aDlg = new MonPursueIteration( true,
                                HOMARD::HOMARD_Gen::_duplicate(homardGen) ) ;
      aDlg->show();
      break;
    }

    case 1103: // Creation d une Iteration
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      MonCreateIteration *IterDlg = new MonCreateIteration( parent, true,
                                     HOMARD::HOMARD_Gen::_duplicate(homardGen), _ObjectName ) ;
      IterDlg->show();
      break;
    }

    case 1111: // Compute une iteration
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      try { homardGen->Compute(_ObjectName.toStdString().c_str(), 0, 1, -1, 1); }
      catch( SALOME::SALOME_Exception& S_ex )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr(CORBA::string_dup(S_ex.details.text)) );
        getApp()->updateObjectBrowser();
        return false;
      }
      break;
    }

    case 1112: // Compute une iteration et publication
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      try { homardGen->Compute(_ObjectName.toStdString().c_str(), 0, 1, -1, 2); }
      catch( SALOME::SALOME_Exception& S_ex )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr(CORBA::string_dup(S_ex.details.text)) );
        getApp()->updateObjectBrowser();
        return false;
      }
      break;
    }

    case 1121: // Information sur le maillage de l'iteration
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      MonIterInfo *IterDlg = new MonIterInfo( parent, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), _ObjectName ) ;
      IterDlg->show();
      break;
    }

    case 1131: // Publication du maillage de l'iteration
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      homardGen->PublishMeshIterInSmesh(_ObjectName.toStdString().c_str());
      break;
    }

    case 1132: // Publication du maillage de l'iteration a partir du fichier
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      homardGen->PublishResultInSmesh(_ObjectName.toStdString().c_str(), 1);
      break;
    }

    case 1201: // Edition d'un objet
    {
      MESSAGE("command " << theCommandID << " activated");
      QString nomObjet = HOMARD_QT_COMMUN::SelectionArbreEtude(QString(""), 1);
      if (nomObjet == QString("")) break;
      _PTR(SObject) obj = chercheMonObjet();
      if (obj)
      {
        // Edition d'une frontiere CAO
        if (HOMARD_UTILS::isBoundaryCAO(obj))
        {
          MonEditBoundaryCAO *aDlg = new MonEditBoundaryCAO(0, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), QString(""), _ObjectName ) ;
          aDlg->show();
        }
        // Edition d'une frontiere discrete
        else if (HOMARD_UTILS::isBoundaryDi(obj))
        {
          MonEditBoundaryDi *aDlg = new MonEditBoundaryDi(0, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), QString(""), _ObjectName ) ;
          aDlg->show();
        }
        // Edition d'une frontiere analytique
        else if (HOMARD_UTILS::isBoundaryAn(obj))
        {
          MonEditBoundaryAn *aDlg = new MonEditBoundaryAn(0, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), QString(""), _ObjectName ) ;
          aDlg->show();
        }
        // Edition d'un cas
        else if (HOMARD_UTILS::isCase(obj))
        {
          MonEditCase *aDlg = new MonEditCase(true, HOMARD::HOMARD_Gen::_duplicate(homardGen), _ObjectName ) ;
          aDlg->show();
        }
        // Edition d'une hypothese
        else if (HOMARD_UTILS::isHypo(obj))
        {
          MonEditHypothesis *aDlg = new MonEditHypothesis(0, true, HOMARD::HOMARD_Gen::_duplicate(homardGen),  _ObjectName, QString(""), QString("")) ;
          aDlg->show();
        }
        // Edition d'une iteration
        else if (HOMARD_UTILS::isIter(obj))
        {
          MonEditIteration *aDlg = new MonEditIteration(parent, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), QString(""), _ObjectName ) ;
          aDlg->show();
        }
        // Edition d'un schema YACS
        else if (HOMARD_UTILS::isYACS(obj))
        {
          MESSAGE("appel de MonEditYACS");
          MonEditYACS *aDlg = new MonEditYACS(true, HOMARD::HOMARD_Gen::_duplicate(homardGen), _ObjectName) ;
          aDlg->show();
        }
        // Edition d'une zone
        else if (HOMARD_UTILS::isZone(obj))
        {
          MonEditZone *aDlg = new MonEditZone(0, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), QString(""), _ObjectName ) ;
          aDlg->show();
        }
      }
      break;
    }

    case 1211: // Suppression d'un objet
    {
      MESSAGE("command " << theCommandID << " activated");
      QString nomObjet = HOMARD_QT_COMMUN::SelectionArbreEtude(QString(""), 1);
      if (nomObjet == QString("")) break;
      _PTR(SObject) obj = chercheMonObjet();
      if (obj)
      {
        // Suppression d'une frontiere
        if ( HOMARD_UTILS::isBoundaryCAO(obj) || HOMARD_UTILS::isBoundaryDi(obj) || HOMARD_UTILS::isBoundaryAn(obj) )
        {
          try
          { homardGen->DeleteBoundary(_ObjectName.toStdString().c_str()); }
          catch( SALOME::SALOME_Exception& S_ex )
          {
            QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                      QObject::tr(CORBA::string_dup(S_ex.details.text)) );
            getApp()->updateObjectBrowser();
            return false;
          }
        }
        // Suppression d'un cas
        else if (HOMARD_UTILS::isCase(obj))
        {
          try
          { homardGen->DeleteCase(_ObjectName.toStdString().c_str(), 1); }
          catch( SALOME::SALOME_Exception& S_ex )
          {
            QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                      QObject::tr(CORBA::string_dup(S_ex.details.text)) );
            getApp()->updateObjectBrowser();
            return false;
          }
        }
        // Suppression d'une hypothese
        else if (HOMARD_UTILS::isHypo(obj))
        {
          try
          { homardGen->DeleteHypo(_ObjectName.toStdString().c_str()); }
          catch( SALOME::SALOME_Exception& S_ex )
          {
            QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                      QObject::tr(CORBA::string_dup(S_ex.details.text)) );
            getApp()->updateObjectBrowser();
            return false;
          }
        }
        // Suppression d'une iteration
        else if (HOMARD_UTILS::isIter(obj))
        {
          try
          { homardGen->DeleteIteration(_ObjectName.toStdString().c_str(), 1); }
          catch( SALOME::SALOME_Exception& S_ex )
          {
            QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                      QObject::tr(CORBA::string_dup(S_ex.details.text)) );
            getApp()->updateObjectBrowser();
            return false;
          }
        }
        // Suppression d'un schema YACS
        else if (HOMARD_UTILS::isYACS(obj))
        {
          try
          { homardGen->DeleteYACS(_ObjectName.toStdString().c_str(), 1); }
          catch( SALOME::SALOME_Exception& S_ex )
          {
            QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                      QObject::tr(CORBA::string_dup(S_ex.details.text)) );
            getApp()->updateObjectBrowser();
            return false;
          }
        }
        // Suppression d'une zone
        else if (HOMARD_UTILS::isZone(obj))
        {
          try
          { homardGen->DeleteZone(_ObjectName.toStdString().c_str()); }
          catch( SALOME::SALOME_Exception& S_ex )
          {
            QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                      QObject::tr(CORBA::string_dup(S_ex.details.text)) );
            getApp()->updateObjectBrowser();
            return false;
          }
        }
      }
      break;
    }

    case 1301: // Information sur un maillage
    {
      MESSAGE("etape 1301")
      MESSAGE("command " << theCommandID << " activated");
      MonMeshInfo *aDlg = new MonMeshInfo( parent, true, HOMARD::HOMARD_Gen::_duplicate(homardGen) ) ;
      aDlg->show();
      break;
    }

    case 1302: // Affichage de fichier texte
    {
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      _PTR(SObject) obj = chercheMonObjet();
      if ( (obj) && ( HOMARD_UTILS::isFileType(obj,QString("log")) || HOMARD_UTILS::isFileType(obj,QString("Summary")) || HOMARD_UTILS::isFileType(obj,QString("xml")) ) )
      {
          MonEditFile *aDlg = new MonEditFile( 0, true, HOMARD::HOMARD_Gen::_duplicate(homardGen), _ObjectName, 0 ) ;
          if ( aDlg->_codret == 0 ) { aDlg->show(); }
      }
      break;
    }

    case 1401: // Création d'un schema YACS
    {
      MESSAGE("etape 1401")
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      MonCreateYACS *aDlg = new MonCreateYACS( true, HOMARD::HOMARD_Gen::_duplicate(homardGen), _ObjectName ) ;
      aDlg->show();
      break;
    }

    case 1402: // Ecriture d'un schéma YACS
    {
      MESSAGE("etape 1402")
      MESSAGE("command " << theCommandID << " activated avec objet " << _ObjectName.toStdString().c_str() );
      try { homardGen->YACSWrite(_ObjectName.toStdString().c_str()); }
      catch( SALOME::SALOME_Exception& S_ex )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr(CORBA::string_dup(S_ex.details.text)) );
        getApp()->updateObjectBrowser();
        return false;
      }
      break;
    }

  }
  getApp()->updateObjectBrowser();
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

// Module's engine IOR
//=============================================================================
QString HOMARDGUI::engineIOR() const
//=============================================================================
{
  CORBA::String_var anIOR = getApp()->orb()->object_to_string( InitHOMARDGen(getApp()) );
  return QString( anIOR.in() );
}

// Module's activation
//=============================================================================
bool HOMARDGUI::activateModule( SUIT_Study* theStudy )
//=============================================================================
{
  bool bOk = SalomeApp_Module::activateModule( theStudy );

  setMenuShown( true );
  setToolShown( true );

  return bOk;
}

// Module's deactivation
//=============================================================================
bool HOMARDGUI::deactivateModule( SUIT_Study* theStudy )
//=============================================================================
{
  setMenuShown( false );
  setToolShown( false );

  return SalomeApp_Module::deactivateModule( theStudy );
}

// Default windows
//=============================================================================
void HOMARDGUI::windows( QMap<int, int>& theMap ) const
//=============================================================================
{
  theMap.clear();
  theMap.insert( SalomeApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
#ifndef DISABLE_PYCONSOLE
  theMap.insert( SalomeApp_Application::WT_PyConsole,     Qt::BottomDockWidgetArea );
#endif
}

//=============================================================================
void HOMARDGUI::setOrb()
//=============================================================================
{
  try
  {
     ORB_INIT &init = *SINGLETON_<ORB_INIT>::Instance();
     ASSERT(SINGLETON_<ORB_INIT>::IsAlreadyExisting());
     _orb = init( 0 , 0 );
  }
  catch (...)
  {
     INFOS("internal error : orb not found");
     _orb = 0;
  }
	    ASSERT(! CORBA::is_nil(_orb));
}
//========================================
_PTR(SObject) HOMARDGUI::chercheMonObjet()
//========================================
{
  SALOMEDSClient_SObject* aSO = NULL;
  _PTR(SObject) obj;
  SALOME_ListIO lst;
  getApp()->selectionMgr()->selectedObjects( lst );
  if (  lst.Extent() == 1 )
  {
    Handle(SALOME_InteractiveObject) io = lst.First();
    SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( application()->activeStudy() );
    _PTR(Study) study = appStudy->studyDS();
    _PTR(SObject) obj = study->FindObjectID( io->getEntry() );
    _ObjectName = QString( obj->GetName().c_str() );
    return obj;
  }
  else { return _PTR(SObject)(aSO); }
}
//=============================================================================
void HOMARDGUI::contextMenuPopup( const QString& client, QMenu* menu, QString& title )
//=============================================================================
{
  MESSAGE("Debut de contextMenuPopup");
  _PTR(SObject) obj = chercheMonObjet();
  if ( obj )
  {
    title = QString( obj->GetName().c_str() );
    _ObjectName = title;
    SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
//
    QPixmap pix ;
    bool DeleteObject = false ;
    bool EditObject = false ;
//
    if ( HOMARD_UTILS::isBoundaryCAO(obj) )
    {
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isBoundaryAn(obj) )
    {
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isBoundaryDi(obj) )
    {
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isCase(obj) )
    {
      pix = resMgr->loadPixmap( "HOMARD", "table_view.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_YACS").toLatin1().data()), this, SLOT(YACSCreate()));
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isHypo(obj) )
    {
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isIter(obj) )
    {
      pix = resMgr->loadPixmap( "HOMARD", "iter_next.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_NEW_ITERATION").toLatin1().data()), this, SLOT(NextIter()));
      pix = resMgr->loadPixmap( "HOMARD", "mesh_compute.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_COMPUTE").toLatin1().data()), this, SLOT(LanceCalcul0()));
      pix = resMgr->loadPixmap( "HOMARD", "mesh_compute.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_COMPUTE_PUBLISH").toLatin1().data()), this, SLOT(LanceCalcul1()));
      pix = resMgr->loadPixmap( "HOMARD", "advanced_mesh_info.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_MESH_INFO").toLatin1().data()), this, SLOT(IterInfo()));
      pix = resMgr->loadPixmap( "HOMARD", "mesh_tree_mesh.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_MESH_PUBLICATION").toLatin1().data()), this, SLOT(MeshPublish0()));
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isYACS(obj) )
    {
      pix = resMgr->loadPixmap( "HOMARD", "write.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_WRITE").toLatin1().data()), this, SLOT(YACSWrite()));
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isZone(obj) )
    {
      EditObject = true ;
      DeleteObject = true ;
    }
    else if ( HOMARD_UTILS::isFileType(obj,QString("log")) || HOMARD_UTILS::isFileType(obj,QString("Summary")) || HOMARD_UTILS::isFileType(obj,QString("xml")) )
    {
      pix = resMgr->loadPixmap( "HOMARD", "texte.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_EDIT_MESS_FILE").toLatin1().data()), this, SLOT(EditAsciiFile()));
    }
    else if ( HOMARD_UTILS::isFileType(obj,QString("Mesh")) )
    {
      pix = resMgr->loadPixmap( "HOMARD", "mesh_tree_mesh.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_MESH_PUBLICATION").toLatin1().data()), this, SLOT(MeshPublish1()));
    }
//  Ajout d'un menu d'edition pour les objets qui le proposent
    if ( EditObject )
    {
      pix = resMgr->loadPixmap( "HOMARD", "loop.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_EDIT").toLatin1().data()), this, SLOT(Edit()));
    }
//  Ajout d'un menu de destruction pour les objets qui le proposent
    if ( DeleteObject )
    {
      pix = resMgr->loadPixmap( "HOMARD", "delete.png" );
      menu->addAction(QIcon(pix), tr(QString("HOM_MEN_DELETE").toLatin1().data()), this, SLOT(Delete()));
    }
  }
}

void HOMARDGUI::NextIter()
{
  this->OnGUIEvent(1103);
}

void HOMARDGUI::LanceCalcul0()
{
  this->OnGUIEvent(1111);
}

void HOMARDGUI::LanceCalcul1()
{
  this->OnGUIEvent(1112);
}

void HOMARDGUI::IterInfo()
{
  this->OnGUIEvent(1121);
}

void HOMARDGUI::MeshPublish0()
{
  this->OnGUIEvent(1131);
}

void HOMARDGUI::MeshPublish1()
{
  this->OnGUIEvent(1132);
}

void HOMARDGUI::Edit()
{
  this->OnGUIEvent(1201);
}

void HOMARDGUI::Delete()
{
  this->OnGUIEvent(1211);
}

void HOMARDGUI::EditAsciiFile()
{
  this->OnGUIEvent(1302);
}

void HOMARDGUI::YACSCreate()
{
  this->OnGUIEvent(1401);
}
void HOMARDGUI::YACSWrite()
{
  this->OnGUIEvent(1402);
}


//
//=============================================================================
// Export the module
//=============================================================================
extern "C" {
  Standard_EXPORT CAM_Module* createModule()
  {
    return new HOMARDGUI("");
  }
}

