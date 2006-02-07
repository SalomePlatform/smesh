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


#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_Filter.h"

#include <vtkRenderer.h>
#include <vtkActorCollection.h>

#include <TColStd_IndexedMapOfInteger.hxx>

#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_Study.h>

#include "LightApp_SelectionMgr.h"

#include "SVTK_Selector.h"
#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"

#include "utilities.h"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SMESH_Gen)
#include CORBA_CLIENT_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(SMESH_Group)
#include CORBA_CLIENT_HEADER(SMESH_Hypothesis)

#include "SMESHGUI.h"
#include "SMESH_Actor.h"
#include "SMESH_ObjectDef.h"

#include <SalomeApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Study.h>

#include <SALOMEDSClient_Study.hxx>
#include <SALOMEDSClient_SObject.hxx>

#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>

#include <set>
using namespace std;

namespace SMESH{

  typedef map<TKeyOfVisualObj,TVisualObjPtr> TVisualObjCont;
  static TVisualObjCont VISUAL_OBJ_CONT;

  TVisualObjPtr GetVisualObj(int theStudyId, const char* theEntry){
    TVisualObjPtr aVisualObj;
    try{
      TVisualObjCont::key_type aKey(theStudyId,theEntry);
      TVisualObjCont::iterator anIter = VISUAL_OBJ_CONT.find(aKey);
      if(anIter != VISUAL_OBJ_CONT.end()){
	aVisualObj = anIter->second;
      }else{
        SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SMESHGUI::activeStudy()->application() );
	_PTR(Study) aStudy = SMESHGUI::activeStudy()->studyDS();
	_PTR(SObject) aSObj = aStudy->FindObjectID(theEntry);
	if(aSObj){
	  _PTR(GenericAttribute) anAttr;
	  if(aSObj->FindAttribute(anAttr,"AttributeIOR")){
	    _PTR(AttributeIOR) anIOR = anAttr;
	    CORBA::String_var aVal = anIOR->Value().c_str();
	    CORBA::Object_var anObj = app->orb()->string_to_object( aVal.in() );
	    if(!CORBA::is_nil(anObj)){
	      //Try narrow to SMESH_Mesh interafce
	      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow(anObj);
	      if(!aMesh->_is_nil()){
		aVisualObj.reset(new SMESH_MeshObj(aMesh));
		aVisualObj->Update();
		TVisualObjCont::value_type aValue(aKey,aVisualObj);
		VISUAL_OBJ_CONT.insert(aValue);
		return aVisualObj;
	      }
	      //Try narrow to SMESH_Group interafce
	      SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObj);
	      if(!aGroup->_is_nil()){
		_PTR(SObject) aFatherSObj = aSObj->GetFather();
		if(!aFatherSObj) return aVisualObj;
		aFatherSObj = aFatherSObj->GetFather();
		if(!aFatherSObj) return aVisualObj;
		CORBA::String_var anEntry = aFatherSObj->GetID().c_str();
		TVisualObjPtr aVisObj = GetVisualObj(theStudyId,anEntry.in());
		if(SMESH_MeshObj* aMeshObj = dynamic_cast<SMESH_MeshObj*>(aVisObj.get())){
		  aVisualObj.reset(new SMESH_GroupObj(aGroup,aMeshObj));
		  aVisualObj->Update();
		  TVisualObjCont::value_type aValue(aKey,aVisualObj);
		  VISUAL_OBJ_CONT.insert(aValue);
		  return aVisualObj;
		}
	      }
	      //Try narrow to SMESH_subMesh interafce
	      SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(anObj);
	      if(!aSubMesh->_is_nil()){
		_PTR(SObject) aFatherSObj = aSObj->GetFather();
		if(!aFatherSObj) return aVisualObj;
		aFatherSObj = aFatherSObj->GetFather();
		if(!aFatherSObj) return aVisualObj;
		CORBA::String_var anEntry = aFatherSObj->GetID().c_str();
		TVisualObjPtr aVisObj = GetVisualObj(theStudyId,anEntry.in());
		if(SMESH_MeshObj* aMeshObj = dynamic_cast<SMESH_MeshObj*>(aVisObj.get())){
		  aVisualObj.reset(new SMESH_subMeshObj(aSubMesh,aMeshObj));
		  aVisualObj->Update();
		  TVisualObjCont::value_type aValue(aKey,aVisualObj);
		  VISUAL_OBJ_CONT.insert(aValue);
		  return aVisualObj;
		}
	      }
	    }
	  }
	}
      }
    }catch(...){
      INFOS("GetMeshObj - There is no SMESH_Mesh object for the SALOMEDS::Strudy and Entry!!!");
    }
    return aVisualObj;
  }


  SVTK_ViewWindow*
  GetViewWindow(const SalomeApp_Module* theModule)
  {
    if (SalomeApp_Application* anApp = theModule->getApp())
      return dynamic_cast<SVTK_ViewWindow*>(anApp->desktop()->activeWindow());
    return NULL;
  }

  SVTK_ViewWindow* FindVtkViewWindow( SUIT_ViewManager* theMgr,
					   SUIT_ViewWindow* theWindow )
  {
    if( !theMgr )
      return NULL;

    QPtrVector<SUIT_ViewWindow> views = theMgr->getViews();
    if( views.containsRef( theWindow ) )
      return GetVtkViewWindow( theWindow );
    else
      return NULL;
  }


  SVTK_ViewWindow* GetVtkViewWindow(SUIT_ViewWindow* theWindow){
    return dynamic_cast<SVTK_ViewWindow*>(theWindow);
  }


/*  SUIT_ViewWindow* GetActiveWindow()
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
    if( !app )
      return NULL;
    SUIT_ViewManager* mgr = app->activeViewManager();
    if( mgr )
      return mgr->getActiveView();
    else
      return NULL;
  }*/

  SVTK_ViewWindow* GetCurrentVtkView(){
    return GetVtkViewWindow( GetActiveWindow() );
  }

  void RepaintViewWindow(SVTK_ViewWindow* theWindow)
  {
    theWindow->Repaint();
  }

  void RenderViewWindow(SVTK_ViewWindow* theWindow)
  {
    theWindow->getRenderer()->Render();
    theWindow->Repaint();
  }

  SMESH_Actor* FindActorByEntry(SUIT_ViewWindow *theWindow,
				const char* theEntry)
  {
    if(SVTK_ViewWindow* aViewWindow = GetVtkViewWindow(theWindow)){
      vtkRenderer *aRenderer = aViewWindow->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor()){
	if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	  if(anActor->hasIO()){
	    Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
	    if(anIO->hasEntry() && strcmp(anIO->getEntry(),theEntry) == 0){
	      return anActor;
	    }
	  }
	}
      }
    }
    return NULL;
  }


  SMESH_Actor* FindActorByEntry(const char* theEntry){
    return FindActorByEntry(GetActiveWindow(),theEntry);
  }


  SMESH_Actor* FindActorByObject(CORBA::Object_ptr theObject){
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
    if( !app )
      return NULL;

    if(!CORBA::is_nil(theObject)){
      _PTR(Study) aStudy = GetActiveStudyDocument();
      CORBA::String_var anIOR = app->orb()->object_to_string( theObject );
      _PTR(SObject) aSObject = aStudy->FindObjectIOR(anIOR.in());
      if(aSObject){
	CORBA::String_var anEntry = aSObject->GetID().c_str();
	return FindActorByEntry(anEntry.in());
      }
    }
    return NULL;
  }


  SMESH_Actor* CreateActor(_PTR(Study) theStudy,
			   const char* theEntry,
			   int theIsClear)
  {
    SMESH_Actor *anActor = NULL;
    CORBA::Long anId = theStudy->StudyId();
    if(TVisualObjPtr aVisualObj = GetVisualObj(anId,theEntry)){
      _PTR(SObject) aSObj = theStudy->FindObjectID(theEntry);
      if(aSObj){
	_PTR(GenericAttribute) anAttr;
	if(aSObj->FindAttribute(anAttr,"AttributeName")){
	  _PTR(AttributeName) aName = anAttr;
	  std::string aNameVal = aName->Value();
	  anActor = SMESH_Actor::New(aVisualObj,theEntry,aNameVal.c_str(),theIsClear);
	}
      }
    }
    return anActor;
  }


  void DisplayActor( SUIT_ViewWindow *theWnd, SMESH_Actor* theActor){
    if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(theWnd)){
      vtkWnd->AddActor(theActor);
      vtkWnd->Repaint();
    }
  }


  void RemoveActor( SUIT_ViewWindow *theWnd, SMESH_Actor* theActor){
    if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(theWnd)){
      vtkWnd->RemoveActor(theActor);
      if(theActor->hasIO()){
	Handle(SALOME_InteractiveObject) anIO = theActor->getIO();
	if(anIO->hasEntry()){
	  std::string anEntry = anIO->getEntry();
	  SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>( vtkWnd->getViewManager()->study() );
	  int aStudyId = aStudy->id();
	  TVisualObjCont::key_type aKey(aStudyId,anEntry);
	  VISUAL_OBJ_CONT.erase(aKey);
	}
      }
      theActor->Delete();
      vtkWnd->Repaint();
    }
  }


  void FitAll(){
    if(SVTK_ViewWindow* wnd = GetCurrentVtkView() ){
      wnd->onFitAll();
      wnd->Repaint();
    }
  }

  vtkRenderer* GetCurrentRenderer(){
    if(SVTK_ViewWindow* wnd = GetCurrentVtkView() )
      return wnd->getRenderer();
    return NULL;
  }

  void RepaintCurrentView(){
    if(SVTK_ViewWindow* wnd = GetCurrentVtkView() )
      {
	wnd->getRenderer()->Render();
	wnd->Repaint(false);
      }
  }

  void UpdateView(SUIT_ViewWindow *theWnd, EDisplaing theAction, const char* theEntry)
  {
    if(SVTK_ViewWindow* aViewWnd = GetVtkViewWindow(theWnd)){
      vtkRenderer *aRenderer = aViewWnd->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();
      switch(theAction){
      case eDisplayAll: {
	while(vtkActor *anAct = aCollection->GetNextActor()){
	  if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	    anActor->SetVisibility(true);
	  }
	}
	break;
      }
      case eDisplayOnly:
      case eEraseAll: {
	while(vtkActor *anAct = aCollection->GetNextActor()){
	  if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	    anActor->SetVisibility(false);
	  }
	}
      }
      default: {
	if(SMESH_Actor *anActor = FindActorByEntry(theWnd,theEntry)){
	  switch(theAction) {
	    case eDisplay:
	    case eDisplayOnly:
	      anActor->SetVisibility(true);
	      break;
	    case eErase:
	      anActor->SetVisibility(false);
	      break;
	  }
	} else {
	  switch(theAction){
	  case eDisplay:
	  case eDisplayOnly:{
	    SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>( theWnd->getViewManager()->study() );
	    _PTR(Study) aDocument = aStudy->studyDS();
	    if((anActor = CreateActor(aDocument,theEntry,true))) {
	      DisplayActor(theWnd,anActor);
	      FitAll();
	    }
	    break;
	  }
	  }
	}
      }
      }
    }
  }


  void UpdateView(EDisplaing theAction, const char* theEntry){
    SalomeApp_Study* aStudy = dynamic_cast< SalomeApp_Study* >( GetActiveStudy() );
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( aStudy->application() );
    SUIT_ViewWindow *aWnd = app->activeViewManager()->getActiveView();
    UpdateView(aWnd,theAction,theEntry);
  }

  void UpdateView(){
    if(SVTK_ViewWindow* aWnd = SMESH::GetCurrentVtkView()){
      LightApp_SelectionMgr* mgr = SMESHGUI::selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      if( selected.Extent() == 0){
	vtkRenderer* aRenderer = aWnd->getRenderer();
	vtkActorCollection *aCollection = aRenderer->GetActors();
	aCollection->InitTraversal();
	while(vtkActor *anAct = aCollection->GetNextActor()){
	  if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	    if(anActor->hasIO())
	      Update(anActor->getIO(),anActor->GetVisibility());
	  }
	}
      }else{
	SALOME_ListIteratorOfListIO anIter( selected );
	for(; anIter.More(); anIter.Next()){
	  Handle(SALOME_InteractiveObject) anIO = anIter.Value();
	  Update(anIO,true);
	}
      }
      RepaintCurrentView();
    }
  }


  void Update(const Handle(SALOME_InteractiveObject)& theIO,
	      bool theDisplay)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    CORBA::Long anId = aStudy->StudyId();
    TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,theIO->getEntry());
    if( aVisualObj )
      aVisualObj->Update();
    if ( theDisplay )
      UpdateView(SMESH::eDisplay,theIO->getEntry());
  }


  void UpdateSelectionProp( SMESHGUI* theModule ) {
    if( !theModule )
      return;

    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( theModule->application() );
    if( !app )
    {
      MESSAGE( "UpdateSelectionProp: Application is null" );
      return;
    }

    SUIT_ViewManager* vm = app->activeViewManager();
    if( !vm )
    {
      MESSAGE( "UpdateSelectionProp: View manager is null" );
      return;
    }

    QPtrVector<SUIT_ViewWindow> views = vm->getViews();

    SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( theModule );
    if( !mgr )
    {
      MESSAGE( "UpdateSelectionProp: Resource manager is null" );
      return;
    }

    QColor aHiColor = mgr->colorValue( "SMESH", "selection_object_color", Qt::white ),
           aSelColor = mgr->colorValue( "SMESH", "selection_element_color", Qt::yellow ),
	   aPreColor = mgr->colorValue( "SMESH", "highlight_color", Qt::cyan );

    int SW = mgr->integerValue( "SMESH", "selection_width", 5 ),
        PW = mgr->integerValue( "SMESH", "highlight_width", 5 );

    double SP1 = mgr->doubleValue( "SMESH", "selection_precision_node", 0.025 ),
           SP2 = mgr->doubleValue( "SMESH", "selection_precision_element", 0.001 );

    for ( int i=0, n=views.count(); i<n; i++ ){
      // update VTK viewer properties
      if(SVTK_ViewWindow* aVtkView = GetVtkViewWindow( views[i] )){
	// mesh element selection
	aVtkView->SetSelectionProp(aSelColor.red()/255., 
				   aSelColor.green()/255.,
				   aSelColor.blue()/255., 
				   SW );
	// tolerances
	aVtkView->SetSelectionTolerance(SP1, SP2);

	// pre-selection
	aVtkView->SetPreselectionProp(aPreColor.red()/255., 
				      aPreColor.green()/255.,
				      aPreColor.blue()/255., 
				      PW);
	// update actors
	vtkRenderer* aRenderer = aVtkView->getRenderer();
	vtkActorCollection *aCollection = aRenderer->GetActors();
	aCollection->InitTraversal();
	while(vtkActor *anAct = aCollection->GetNextActor()){
	  if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	    anActor->SetHighlightColor(aHiColor.red()/255., 
				       aHiColor.green()/255.,
				       aHiColor.blue()/255.);
	    anActor->SetPreHighlightColor(aPreColor.red()/255., 
					  aPreColor.green()/255.,
					  aPreColor.blue()/255.);
	  }
	}
      }
    }
  }


  //----------------------------------------------------------------------------
  SVTK_Selector* 
  GetSelector(SUIT_ViewWindow *theWindow)
  {
    if(SVTK_ViewWindow* aWnd = GetVtkViewWindow(theWindow))
      return aWnd->GetSelector();

    return NULL;
  }

  void SetFilter(const Handle(VTKViewer_Filter)& theFilter,
		 SVTK_Selector* theSelector)
  {
    if (theSelector)
      theSelector->SetFilter(theFilter);
  }

  Handle(VTKViewer_Filter) GetFilter(int theId, SVTK_Selector* theSelector)
  {
    return theSelector->GetFilter(theId);
  }

  bool IsFilterPresent(int theId, SVTK_Selector* theSelector)
  {
    return theSelector->IsFilterPresent(theId);
  }

  void RemoveFilter(int theId, SVTK_Selector* theSelector)
  {
    theSelector->RemoveFilter(theId);
  }

  void RemoveFilters(SVTK_Selector* theSelector)
  {
    for ( int id = SMESHGUI_NodeFilter; theSelector && id < SMESHGUI_LastFilter; id++ )
      theSelector->RemoveFilter( id );
  }

  bool IsValid(SALOME_Actor* theActor, int theCellId,
	       SVTK_Selector* theSelector)
  {
    return theSelector->IsValid(theActor,theCellId);
  }


  //----------------------------------------------------------------------------
  void SetPointRepresentation(bool theIsVisible){
    if(SVTK_ViewWindow* aViewWindow = GetCurrentVtkView()){
      vtkRenderer *aRenderer = aViewWindow->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor()){
	if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	  if(anActor->GetVisibility()){
	    anActor->SetPointRepresentation(theIsVisible);
	  }
	}
      }
      RepaintCurrentView();
    }
  }


  void SetPickable(SMESH_Actor* theActor){
    if(SVTK_ViewWindow* aWnd = GetCurrentVtkView()){
      int anIsAllPickable = (theActor == NULL);
      vtkRenderer *aRenderer = aWnd->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor()){
	if(SALOME_Actor *anActor = dynamic_cast<SALOME_Actor*>(anAct)){
	  if(anActor->GetVisibility()){
	    anActor->SetPickable(anIsAllPickable);
	  }
	}
      }
      if(theActor)
	theActor->SetPickable(!anIsAllPickable);
      RepaintCurrentView();
    }
  }


  //----------------------------------------------------------------------------
  int GetNameOfSelectedNodes(SVTK_Selector* theSelector,
			     const Handle(SALOME_InteractiveObject)& theIO,
			     QString& theName)
  {
    theName = "";
    TColStd_IndexedMapOfInteger aMapIndex;
    theSelector->GetIndex(theIO,aMapIndex);

    for(int i = 1; i <= aMapIndex.Extent(); i++)
      theName += QString(" %1").arg(aMapIndex(i));

    return aMapIndex.Extent();
  }

  int GetNameOfSelectedElements(SVTK_Selector* theSelector,
				const Handle(SALOME_InteractiveObject)& theIO,
				QString& theName)
  {
    theName = "";
    TColStd_IndexedMapOfInteger aMapIndex;
    theSelector->GetIndex(theIO,aMapIndex);

    typedef std::set<int> TIdContainer;
    TIdContainer anIdContainer;
    for( int i = 1; i <= aMapIndex.Extent(); i++)
      anIdContainer.insert(aMapIndex(i));

    TIdContainer::const_iterator anIter = anIdContainer.begin();
    for(; anIter != anIdContainer.end(); anIter++)
      theName += QString(" %1").arg(*anIter);

    return aMapIndex.Extent();
  }


  int GetEdgeNodes(SVTK_Selector* theSelector,
		   const TVisualObjPtr& theVisualObject,
		   int& theId1,
		   int& theId2)
  {
    const SALOME_ListIO& selected = theSelector->StoredIObjects();

    if ( selected.Extent() != 1 )
      return -1;

    Handle(SALOME_InteractiveObject) anIO = selected.First();
    if ( anIO.IsNull() || !anIO->hasEntry() )
      return -1;

    TColStd_IndexedMapOfInteger aMapIndex;
    theSelector->GetIndex( anIO, aMapIndex );
    if ( aMapIndex.Extent() != 2 )
      return -1;

    int anObjId = -1, anEdgeNum = -1;
    for ( int i = 1; i <= aMapIndex.Extent(); i++ ) {
      int aVal = aMapIndex( i );
      if ( aVal > 0 )
	anObjId = aVal;
      else
	anEdgeNum = abs( aVal ) - 1;
    }

    if ( anObjId == -1 || anEdgeNum == -1 )
      return -1;

    return theVisualObject->GetEdgeNodes( anObjId, anEdgeNum, theId1, theId2 ) ? 1 : -1;
  }

  //----------------------------------------------------------------------------
  int GetNameOfSelectedNodes(LightApp_SelectionMgr *theMgr,
			     const Handle(SALOME_InteractiveObject)& theIO,
			     QString& theName)
  {
    theName = "";
    if(theIO->hasEntry()){
      if(FindActorByEntry(theIO->getEntry())){
	TColStd_IndexedMapOfInteger aMapIndex;
	theMgr->GetIndexes(theIO,aMapIndex);
	for(int i = 1; i <= aMapIndex.Extent(); i++){
	  theName += QString(" %1").arg(aMapIndex(i));
	}
	return aMapIndex.Extent();
      }
    }
    return -1;
  }

  int GetNameOfSelectedNodes(LightApp_SelectionMgr *theMgr, QString& theName){
    theName = "";
    SALOME_ListIO selected; theMgr->selectedObjects( selected );
    if(selected.Extent() == 1){
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      return GetNameOfSelectedNodes(theMgr,anIO,theName);
    }
    return -1;
  }


  int GetNameOfSelectedElements(LightApp_SelectionMgr *theMgr,
				const Handle(SALOME_InteractiveObject)& theIO,
				QString& theName)
  {
    theName = "";
    if(theIO->hasEntry()){
      if(FindActorByEntry(theIO->getEntry())){
	TColStd_IndexedMapOfInteger aMapIndex;
	theMgr->GetIndexes(theIO,aMapIndex);
	typedef set<int> TIdContainer;
	TIdContainer anIdContainer;
	for( int i = 1; i <= aMapIndex.Extent(); i++)
	  anIdContainer.insert(aMapIndex(i));
	TIdContainer::const_iterator anIter = anIdContainer.begin();
	for(; anIter != anIdContainer.end(); anIter++){
	  theName += QString(" %1").arg(*anIter);
	}
	return aMapIndex.Extent();
      }
    }
    return -1;
  }


  int GetNameOfSelectedElements(LightApp_SelectionMgr *theMgr, QString& theName)
  {
    theName = "";
    SALOME_ListIO selected; theMgr->selectedObjects( selected );

    if( selected.Extent() == 1){
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      return GetNameOfSelectedElements(theMgr,anIO,theName);
    }
    return -1;
  }

  int GetSelected(LightApp_SelectionMgr*       theMgr,
		  TColStd_IndexedMapOfInteger& theMap,
		  const bool                   theIsElement)
  {
    theMap.Clear();
    SALOME_ListIO selected; theMgr->selectedObjects( selected );

    if ( selected.Extent() == 1 )
    {
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      if ( anIO->hasEntry() ) {
	theMgr->GetIndexes( anIO, theMap );
      }
    }
    return theMap.Extent();
  }


  int GetEdgeNodes( LightApp_SelectionMgr* theMgr, int& theId1, int& theId2 )
  {
    SALOME_ListIO selected; theMgr->selectedObjects( selected );

    if ( selected.Extent() != 1 )
      return -1;

    Handle(SALOME_InteractiveObject) anIO = selected.First();
    if ( anIO.IsNull() || !anIO->hasEntry() )
      return -1;

    SMESH_Actor *anActor = SMESH::FindActorByEntry( anIO->getEntry() );
    if ( anActor == 0 )
      return -1;

    TColStd_IndexedMapOfInteger aMapIndex;
    theMgr->GetIndexes( anIO, aMapIndex );
    if ( aMapIndex.Extent() != 2 )
      return -1;

    int anObjId = -1, anEdgeNum = -1;
    for ( int i = 1; i <= aMapIndex.Extent(); i++ ) {
      int aVal = aMapIndex( i );
      if ( aVal > 0 )
	anObjId = aVal;
      else
	anEdgeNum = abs( aVal );
    }

    if ( anObjId == -1 || anEdgeNum == -1 )
      return -1;

    return anActor->GetObject()->GetEdgeNodes( anObjId, anEdgeNum, theId1, theId2 ) ? 1 : -1;
  }

  void SetControlsPrecision( const long theVal )
  {
    if( SVTK_ViewWindow* aWnd = SMESH::GetCurrentVtkView() )
    {
      vtkRenderer *aRenderer = aWnd->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();

      while ( vtkActor *anAct = aCollection->GetNextActor())
      {
        if ( SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>( anAct ) )
        {
          anActor->SetControlsPrecision( theVal );
          anActor->SetControlMode( anActor->GetControlMode() );
        }
      }

    }
  }
}
