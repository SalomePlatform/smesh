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

#include "SALOME_InteractiveObject.hxx"
#include "VTKViewer_Filter.h"

class QAD_Study;
class QAD_StudyFrame;
class SALOME_Selection;
class VTKViewer_ViewFrame;
class VTKViewer_InteractorStyleSALOME;

#include <CORBA.h>

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS)

#include <boost/shared_ptr.hpp>
#include "SMESH_Object.h"

class SALOME_Actor;
class SMESH_Actor;

namespace SMESH{

  //----------------------------------------------------------------------------
  typedef pair<int,string> TKeyOfVisualObj;
  
  TVisualObjPtr GetVisualObj(int theStudyId, 
			     const char* theEntry);


  //----------------------------------------------------------------------------
  QAD_StudyFrame* GetActiveStudyFrame();

  VTKViewer_ViewFrame* FindVtkViewFrame(QAD_Study* theStudy,
					QAD_StudyFrame* theStudyFrame);

  VTKViewer_ViewFrame* GetVtkViewFrame(QAD_StudyFrame* theStudyFrame);

  VTKViewer_ViewFrame* GetCurrentVtkView();

  void RepaintViewFrame(VTKViewer_ViewFrame* theFrame);

  void RenderViewFrame(VTKViewer_ViewFrame* theFrame);

  //----------------------------------------------------------------------------
  SMESH_Actor* FindActorByEntry(QAD_StudyFrame *theStudyFrame, 
				const char* theEntry);
  SMESH_Actor* FindActorByEntry(const char* theEntry);

  SMESH_Actor* FindActorByObject(CORBA::Object_ptr theObject);


  //----------------------------------------------------------------------------
  SMESH_Actor* CreateActor(SALOMEDS::Study_ptr theStudy,
			   const char* theEntry,
			   int theIsClear = false);

  void DisplayActor(QAD_StudyFrame *theStudyFrame, 
		    SMESH_Actor* theActor);
  void RemoveActor(QAD_StudyFrame *theStudyFrame, 
		   SMESH_Actor* theActor);


  //----------------------------------------------------------------------------
  enum EDisplaing {eDisplayAll, eDisplay, eDisplayOnly, eErase, eEraseAll};
  void UpdateView(QAD_StudyFrame *theStudyFrame, 
		  EDisplaing theAction, 
		  const char* theEntry = "");
  void UpdateView(EDisplaing theAction, 
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

  void UpdateSelectionProp();


  //----------------------------------------------------------------------------
  VTKViewer_InteractorStyleSALOME* 
    GetInteractorStyle(QAD_StudyFrame *theStudyFrame = GetActiveStudyFrame());

  void SetFilter(const Handle(VTKViewer_Filter)& theFilter,
		 VTKViewer_InteractorStyleSALOME* theStyle = GetInteractorStyle());

  Handle(VTKViewer_Filter) 
    GetFilter(int theId, VTKViewer_InteractorStyleSALOME* theStyle = GetInteractorStyle());

  bool IsFilterPresent(int theId, 
		       VTKViewer_InteractorStyleSALOME* theStyle = GetInteractorStyle());

  void RemoveFilter(int theId, 
		    VTKViewer_InteractorStyleSALOME* theStyle = GetInteractorStyle());

  bool IsValid(SALOME_Actor* theActor, int theCellId,
	       VTKViewer_InteractorStyleSALOME* theStyle = GetInteractorStyle());

  //----------------------------------------------------------------------------
  int GetNameOfSelectedNodes(SALOME_Selection *theSel, 
			     const Handle(SALOME_InteractiveObject)& theIO, 
			     QString& theName);

  int GetNameOfSelectedNodes(SALOME_Selection *theSel, 
			     QString& aName);

  int GetNameOfSelectedElements(SALOME_Selection *theSel, 
				const Handle(SALOME_InteractiveObject)& theIO, 
				QString& theName);

  int GetNameOfSelectedElements(SALOME_Selection *theSel, 
				QString& aName);

  int GetSelected(SALOME_Selection* theSel, 
		  TColStd_IndexedMapOfInteger& theMap, 
		  const bool theIsElement = true );

  int GetEdgeNodes(SALOME_Selection* theSel, int& theId1, int& theId2);

  void SetControlsPrecision( const long theVal );

};


#endif




























