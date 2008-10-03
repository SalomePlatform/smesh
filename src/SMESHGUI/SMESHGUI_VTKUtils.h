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

#include "SMESH_SMESHGUI.hxx"

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
  typedef std::pair<int,std::string> TKeyOfVisualObj;
  
SMESHGUI_EXPORT
  TVisualObjPtr GetVisualObj(int theStudyId, 
			     const char* theEntry);
SMESHGUI_EXPORT
  void OnVisuException(); // PAL16631

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  SVTK_ViewWindow* GetViewWindow(const SalomeApp_Module* theModule = NULL,
				 bool createIfNotFound = false);
SMESHGUI_EXPORT
  SVTK_ViewWindow* FindVtkViewWindow(SUIT_ViewManager*, SUIT_ViewWindow*);
SMESHGUI_EXPORT
  SVTK_ViewWindow* GetVtkViewWindow(SUIT_ViewWindow*);

SMESHGUI_EXPORT
  SVTK_ViewWindow* GetCurrentVtkView();

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  void RepaintCurrentView();
SMESHGUI_EXPORT
  void RepaintViewWindow(SVTK_ViewWindow*);
SMESHGUI_EXPORT
  void RenderViewWindow(SVTK_ViewWindow*);
SMESHGUI_EXPORT
  void FitAll();

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  SMESH_Actor* FindActorByEntry (SUIT_ViewWindow*, const char* theEntry);
SMESHGUI_EXPORT
  SMESH_Actor* FindActorByEntry (const char* theEntry);

SMESHGUI_EXPORT
  SMESH_Actor* FindActorByObject (CORBA::Object_ptr theObject);

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  SMESH_Actor* CreateActor (_PTR(Study), const char*, int = false);
SMESHGUI_EXPORT
  void DisplayActor (SUIT_ViewWindow*, SMESH_Actor* theActor);
SMESHGUI_EXPORT
  void RemoveActor  (SUIT_ViewWindow*, SMESH_Actor* theActor);
SMESHGUI_EXPORT
  void RemoveVisuData(int studyID);

  //----------------------------------------------------------------------------
  enum EDisplaing {eDisplayAll, eDisplay, eDisplayOnly, eErase, eEraseAll};
SMESHGUI_EXPORT
  bool UpdateView (SUIT_ViewWindow*,
		   EDisplaing theAction,
		   const char* theEntry = "" );
SMESHGUI_EXPORT		   
  bool UpdateView (EDisplaing theAction,
		   const char* theEntry = "");

SMESHGUI_EXPORT
  void UpdateView();

SMESHGUI_EXPORT
  bool Update(const Handle(SALOME_InteractiveObject)& theIO,
	      bool theDisplay);


  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  void SetPointRepresentation(bool theIsVisible);

SMESHGUI_EXPORT
  void SetPickable(SMESH_Actor* theActor = NULL);

SMESHGUI_EXPORT
  void UpdateSelectionProp( SMESHGUI* );


  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  SVTK_Selector* GetSelector (SUIT_ViewWindow* = GetActiveWindow());

SMESHGUI_EXPORT
  void SetFilter (const Handle(VTKViewer_Filter)& theFilter,
		  SVTK_Selector* theSelector = GetSelector());
SMESHGUI_EXPORT
  Handle(VTKViewer_Filter) GetFilter (int theId, SVTK_Selector* theSelector = GetSelector());
SMESHGUI_EXPORT
  bool IsFilterPresent (int theId, SVTK_Selector* theSelector = GetSelector());
SMESHGUI_EXPORT
  void RemoveFilter (int theId, SVTK_Selector* theSelector = GetSelector());

SMESHGUI_EXPORT
  void RemoveFilters (SVTK_Selector* theSelector = GetSelector());

SMESHGUI_EXPORT
  bool IsValid (SALOME_Actor* theActor, int theCellId,
		SVTK_Selector* theSelector = GetSelector());

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  int GetNameOfSelectedNodes(SVTK_Selector* theSelector,
			     const Handle(SALOME_InteractiveObject)& theIO,
			     QString& theName);
SMESHGUI_EXPORT
  int GetNameOfSelectedElements(SVTK_Selector* theSelector,
				const Handle(SALOME_InteractiveObject)& theIO,
				QString& theName);
SMESHGUI_EXPORT
  int GetEdgeNodes(SVTK_Selector* theSelector,
		   const TVisualObjPtr& theVisualObj,
		   int& theId1,
		   int& theId2);

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  int GetNameOfSelectedNodes (LightApp_SelectionMgr*,
			      const Handle(SALOME_InteractiveObject)& theIO,
			      QString& theName);
SMESHGUI_EXPORT
  int GetNameOfSelectedNodes (LightApp_SelectionMgr*,
			      QString& aName);
SMESHGUI_EXPORT
  int GetNameOfSelectedElements (LightApp_SelectionMgr*,
				 const Handle(SALOME_InteractiveObject)& theIO,
				 QString& theName);
SMESHGUI_EXPORT
  int GetNameOfSelectedElements (LightApp_SelectionMgr*,
				 QString& aName);
SMESHGUI_EXPORT
  int GetSelected (LightApp_SelectionMgr*,
		   TColStd_IndexedMapOfInteger& theMap,
		   const bool theIsElement = true );

SMESHGUI_EXPORT
  int GetEdgeNodes (LightApp_SelectionMgr*, int& theId1, int& theId2);

SMESHGUI_EXPORT
  void SetControlsPrecision (const long theVal);
};

#endif
