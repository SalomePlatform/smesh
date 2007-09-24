//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com

#ifndef SMESHGUI_VTKUtils_HeaderFile
#define SMESHGUI_VTKUtils_HeaderFile

#include "SALOMEDSClient_definitions.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "VTKViewer_Filter.h"

#include "SMESH_Object.h"
#include "SMESHGUI_Utils.h"

#include <CORBA.h>

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS)

#include <boost/shared_ptr.hpp>

class QString;

class TColStd_IndexedMapOfInteger;

class SALOMEDSClient_Study;

class SUIT_Study;
class SUIT_ViewWindow;
class SUIT_ViewManager;

class SVTK_ViewWindow;
class SVTK_Selector;

class LightApp_SelectionMgr;
class SalomeApp_Module;

class SMESHGUI;
class SMESH_Actor;
class SALOME_Actor;

namespace SMESH {

  //----------------------------------------------------------------------------
  typedef pair<int,string> TKeyOfVisualObj;

  TVisualObjPtr GetVisualObj(int theStudyId,
			     const char* theEntry);
  void OnVisuException(); // PAL16631

  //----------------------------------------------------------------------------
  SVTK_ViewWindow* GetViewWindow(const SalomeApp_Module* theModule = NULL,
				 bool createIfNotFound = false);

  SVTK_ViewWindow* FindVtkViewWindow(SUIT_ViewManager*, SUIT_ViewWindow*);

  SVTK_ViewWindow* GetVtkViewWindow(SUIT_ViewWindow*);

  SVTK_ViewWindow* GetCurrentVtkView();

  //----------------------------------------------------------------------------
  void RepaintCurrentView();
  void RepaintViewWindow(SVTK_ViewWindow*);
  void RenderViewWindow(SVTK_ViewWindow*);

  void FitAll();

  //----------------------------------------------------------------------------
  SMESH_Actor* FindActorByEntry (SUIT_ViewWindow*, const char* theEntry);
  SMESH_Actor* FindActorByEntry (const char* theEntry);

  SMESH_Actor* FindActorByObject (CORBA::Object_ptr theObject);

  //----------------------------------------------------------------------------
  SMESH_Actor* CreateActor (_PTR(Study), const char*, int = false);
  void DisplayActor (SUIT_ViewWindow*, SMESH_Actor* theActor);
  void RemoveActor  (SUIT_ViewWindow*, SMESH_Actor* theActor);

  //----------------------------------------------------------------------------
  enum EDisplaing {eDisplayAll, eDisplay, eDisplayOnly, eErase, eEraseAll};
  void UpdateView (SUIT_ViewWindow*,
		   EDisplaing theAction,
		   const char* theEntry = "");
  void UpdateView (EDisplaing theAction,
		   const char* theEntry = "");

  void UpdateView();

  bool Update(const Handle(SALOME_InteractiveObject)& theIO,
	      bool theDisplay);


  //----------------------------------------------------------------------------
  void SetPointRepresentation(bool theIsVisible);

  void SetPickable(SMESH_Actor* theActor = NULL);

  void UpdateSelectionProp( SMESHGUI* );


  //----------------------------------------------------------------------------
  SVTK_Selector* GetSelector (SUIT_ViewWindow* = GetActiveWindow());

  void SetFilter (const Handle(VTKViewer_Filter)& theFilter,
		  SVTK_Selector* theSelector = GetSelector());

  Handle(VTKViewer_Filter) GetFilter (int theId, SVTK_Selector* theSelector = GetSelector());

  bool IsFilterPresent (int theId, SVTK_Selector* theSelector = GetSelector());

  void RemoveFilter (int theId, SVTK_Selector* theSelector = GetSelector());

  void RemoveFilters (SVTK_Selector* theSelector = GetSelector());

  bool IsValid (SALOME_Actor* theActor, int theCellId,
		SVTK_Selector* theSelector = GetSelector());

  //----------------------------------------------------------------------------
  int GetNameOfSelectedNodes(SVTK_Selector* theSelector,
			     const Handle(SALOME_InteractiveObject)& theIO,
			     QString& theName);

  int GetNameOfSelectedElements(SVTK_Selector* theSelector,
				const Handle(SALOME_InteractiveObject)& theIO,
				QString& theName);

  int GetEdgeNodes(SVTK_Selector* theSelector,
		   const TVisualObjPtr& theVisualObj,
		   int& theId1,
		   int& theId2);

  //----------------------------------------------------------------------------
  int GetNameOfSelectedNodes (LightApp_SelectionMgr*,
			      const Handle(SALOME_InteractiveObject)& theIO,
			      QString& theName);

  int GetNameOfSelectedNodes (LightApp_SelectionMgr*,
			      QString& aName);

  int GetNameOfSelectedElements (LightApp_SelectionMgr*,
				 const Handle(SALOME_InteractiveObject)& theIO,
				 QString& theName);

  int GetNameOfSelectedElements (LightApp_SelectionMgr*,
				 QString& aName);

  int GetSelected (LightApp_SelectionMgr*,
		   TColStd_IndexedMapOfInteger& theMap,
		   const bool theIsElement = true );

  int GetEdgeNodes (LightApp_SelectionMgr*, int& theId1, int& theId2);

  void SetControlsPrecision (const long theVal);
};

#endif
