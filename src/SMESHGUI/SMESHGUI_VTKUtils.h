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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org

#ifndef SMESHGUI_VTKUtils_HeaderFile
#define SMESHGUI_VTKUtils_HeaderFile

class QString;
class vtkRenderer;
class TColStd_IndexedMapOfInteger;

#include "SALOMEDSClient_definitions.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "VTKViewer_Filter.h"

class SALOMEDSClient_Study;

class SUIT_Study;
class SUIT_ViewWindow;
class SUIT_ViewManager;

class SVTK_InteractorStyle;
class SVTK_ViewWindow;
class SVTK_Selector;

class LightApp_SelectionMgr;
class SMESHGUI;

#include <CORBA.h>

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS)

#include <boost/shared_ptr.hpp>
#include "SMESH_Object.h"

class SMESH_Actor;
class SALOME_Actor;
class SVTK_ViewWindow;
class SalomeApp_Module;

namespace SMESH{

  //----------------------------------------------------------------------------
  typedef pair<int,string> TKeyOfVisualObj;
  
  TVisualObjPtr GetVisualObj(int theStudyId, 
			     const char* theEntry);

  //----------------------------------------------------------------------------
  SVTK_ViewWindow* GetViewWindow(const SalomeApp_Module* theModule);

  SUIT_ViewWindow* GetActiveWindow();

  SVTK_ViewWindow* FindVtkViewWindow( SUIT_ViewManager*,
				      SUIT_ViewWindow* );

  SVTK_ViewWindow* GetVtkViewWindow( SUIT_ViewWindow* );

  SVTK_ViewWindow* GetCurrentVtkView();

  void RepaintViewWindow(SVTK_ViewWindow*);
  void RenderViewWindow(SVTK_ViewWindow*);

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

  void Update(const Handle(SALOME_InteractiveObject)& theIO,
	      bool theDisplay);


  //----------------------------------------------------------------------------
  void FitAll();

  void RepaintCurrentView();

  vtkRenderer* GetCurrentRenderer();


  //----------------------------------------------------------------------------
  void SetPointRepresentation(bool theIsVisible);

  void SetPickable(SMESH_Actor* theActor = NULL);

  void UpdateSelectionProp( SMESHGUI* );


  //----------------------------------------------------------------------------
  SVTK_InteractorStyle* 
    GetInteractorStyle (SUIT_ViewWindow* = GetActiveWindow());

  void SetFilter (const Handle(VTKViewer_Filter)& theFilter,
		  SVTK_InteractorStyle* theStyle = GetInteractorStyle());

  Handle(VTKViewer_Filter) 
    GetFilter (int theId, SVTK_InteractorStyle* theStyle = GetInteractorStyle());

  bool IsFilterPresent (int theId, 
			SVTK_InteractorStyle* theStyle = GetInteractorStyle());

  void RemoveFilter (int theId, 
		     SVTK_InteractorStyle* theStyle = GetInteractorStyle());

  void RemoveFilters (SVTK_InteractorStyle* theStyle = GetInteractorStyle());

  bool IsValid (SALOME_Actor* theActor, int theCellId,
		SVTK_InteractorStyle* theStyle = GetInteractorStyle());

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
