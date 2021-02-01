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
#include "SMESHGUI_MG_ADAPTDRIVER.h"
//~#include "MG_ADAPT_i.hxx"
//~#include "MG_ADAPT.hxx"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Study.h>

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
  action( theCommandID ) ;
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
void SMESHGUI_AdaptDlg::action (int theCommandID)
//=======================================================================
{
//   std::cout  << "SMESHGUI_AdaptDlg::action avec theCommandID : " << theCommandID << std::endl;

// Preferences
//   recupPreferences();

// Menus and actions
  bool ok =  OnGUIEvent (theCommandID) ;
  if ( ! ok ) INFOS("Erreur");

  return ;
}

// /*!
//   * \brief Gets the preferences for the adaptation
//   * \param
//   * \return
//   *
//   * Pour chaque valeur, le defaut est la valeur definie dans ADAPT_Gen
//   * . Si la recuperation dans config/salome s'est bien passee a la creation de ADAPT_Gen
//   *   ces valeurs sont les valeurs definies.
//   * . Si cela ne s'est pas bien passe, ce sont les valeurs par defaut de ADAPT_Gen
// */
// void SMESHGUI_AdaptDlg::recupPreferences()
// {
//   INFOS("Début de recupPreferences")
// //
// // A. Declarations
// //
//   SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
//   SALOME_LifeCycleCORBA* ls = new SALOME_LifeCycleCORBA(app->namingService());
//   Engines::EngineComponent_var comp = ls->FindOrLoad_Component("FactoryServer", "SMESH");
//   ADAPT::ADAPT_Gen_var adaptGen = ADAPT::ADAPT_Gen::_narrow(comp);
//   if (!CORBA::is_nil(adaptGen))
//     adaptGen->UpdateStudy();
//
//   SUIT_ResourceMgr* resMgr = mySMESHGUI->getApp()->resourceMgr();
//
// }

/*!
  * \brief Launches the GUI for the adaptation
  * \param theCommandID - the integer that references the operation
  * \return bool OK/notOK
*/
bool SMESHGUI_AdaptDlg::OnGUIEvent (int theCommandID)
{
//   std::cout  << "SMESHGUI_AdaptDlg:OnGUIEvent avec theCommandID : " << theCommandID << std::endl;
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
//   ADAPT::ADAPT_Gen_var adaptGen = ADAPT::ADAPT_Gen::_narrow(comp);
//   if (!CORBA::is_nil(adaptGen))
//     adaptGen->UpdateStudy();

  mySMESHGUI->getApp()->updateObjectBrowser();
//
// B. Choix selon les commandes
  bool ok = true ;
  SCRUTE(theCommandID);
  switch (theCommandID)
  {
    case 8020: // Adaptation avec MG-Adpat
    {
//       INFOS("Interface avec MG-Adapt" );

      SMESH::MG_ADAPT_ptr model = SMESHGUI::GetSMESHGen()->CreateMG_ADAPT();
      bool isCreation = false;
	  if (mySMESHGUI->isStudyLocked()) break;
      mySMESHGUI->EmitSignalDeactivateDialog();
      SMESHGUI_MG_ADAPTDRIVER *mgAdapt = new SMESHGUI_MG_ADAPTDRIVER(mySMESHGUI, model, isCreation);
      mgAdapt->show();
      break;
    }

  }
  mySMESHGUI->getApp()->updateObjectBrowser();
  return ok;
}



