//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_VTKUtils.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_VTKUtils.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_Filter.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_ObjectDef.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ViewManager.h>
#include <SUIT_ResourceMgr.h>

#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>

#include <SVTK_Selector.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Study.h>

// SALOME KERNEL includes
#include <utilities.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(SMESH_Group)

// VTK includes
#include <vtkRenderer.h>
#include <vtkActorCollection.h>
#include <vtkUnstructuredGrid.h>

// OCCT includes
#include <TColStd_IndexedMapOfInteger.hxx>
#include <Standard_ErrorHandler.hxx>

namespace SMESH
{
  typedef std::map<TKeyOfVisualObj,TVisualObjPtr> TVisualObjCont;
  static TVisualObjCont VISUAL_OBJ_CONT;

  //=============================================================================
  /*!
   * \brief Allocate some memory at construction and release it at destruction.
   * Is used to be able to continue working after mesh generation or visualization
   * break due to lack of memory
   */
  //=============================================================================

  struct MemoryReserve
  {
    char* myBuf;
    MemoryReserve(): myBuf( new char[1024*1024*1] ){} // 1M
    void Free() { if (myBuf) { delete [] myBuf; myBuf = 0; }}
    ~MemoryReserve() { Free(); }
  };
  static MemoryReserve* theVISU_MemoryReserve = new MemoryReserve;

  //================================================================================
  /*!
   * \brief Remove VisualObj and its actor from all views
   */
  //================================================================================

  void RemoveVisualObjectWithActors( const char* theEntry )
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      ( SUIT_Session::session()->activeApplication() );
    SUIT_ViewManager* aViewManager =
      app ? app->getViewManager(SVTK_Viewer::Type(), true) : 0;
    if ( aViewManager ) {
      QVector<SUIT_ViewWindow*> views = aViewManager->getViews();
      for ( int iV = 0; iV < views.count(); ++iV ) {
        if ( SMESH_Actor* actor = FindActorByEntry( views[iV], theEntry)) {
          if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(views[iV]))
            vtkWnd->RemoveActor(actor);
          actor->Delete();
        }
      }
      int aStudyId = aViewManager->study()->id();
      TVisualObjCont::key_type aKey(aStudyId,theEntry);
      TVisualObjCont::iterator anIter = VISUAL_OBJ_CONT.find(aKey);
      if(anIter != VISUAL_OBJ_CONT.end()) {
        // for unknown reason, object destructor is not called, so clear object manually
        anIter->second->GetUnstructuredGrid()->SetCells(0,0,0);
        anIter->second->GetUnstructuredGrid()->SetPoints(0);
      }
      VISUAL_OBJ_CONT.erase(aKey);
    }
  }
  //================================================================================
  /*!
   * \brief Remove all VisualObjs and their actors from all views
   */
  //================================================================================

  void RemoveAllObjectsWithActors()
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      ( SUIT_Session::session()->activeApplication() );
    if (!app) return;
    ViewManagerList viewMgrs = app->viewManagers();
    for ( int iM = 0; iM < viewMgrs.count(); ++iM ) {
      SUIT_ViewManager* aViewManager = viewMgrs.at( iM );
      if ( aViewManager && aViewManager->getType() == SVTK_Viewer::Type()) {
        QVector<SUIT_ViewWindow*> views = aViewManager->getViews();
        for ( int iV = 0; iV < views.count(); ++iV ) {
          if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(views[iV])) {
            vtkRenderer *aRenderer = vtkWnd->getRenderer();
            vtkActorCollection *actors = aRenderer->GetActors();
            for (int i = 0; i < actors->GetNumberOfItems(); ++i ) {
              // size of actors changes inside the loop
              while (SMESH_Actor *actor = dynamic_cast<SMESH_Actor*>(actors->GetItemAsObject(i)))
              {
                vtkWnd->RemoveActor(actor);
                actor->Delete();
              }
            }
          }
        }
      }
    }
    TVisualObjCont::iterator anIter = VISUAL_OBJ_CONT.begin();
    for ( ; anIter != VISUAL_OBJ_CONT.end(); ++anIter ) {
      // for unknown reason, object destructor is not called, so clear object manually
      anIter->second->GetUnstructuredGrid()->SetCells(0,0,0);
      anIter->second->GetUnstructuredGrid()->SetPoints(0);
    }
    VISUAL_OBJ_CONT.clear();
  }

  //================================================================================
  /*!
   * \brief Remove all VisualObjs of a study
   */
  //================================================================================

  void RemoveVisuData(int studyID)
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>
      ( SUIT_Session::session()->activeApplication() );
    if (!app) return;
    ViewManagerList viewMgrs = app->viewManagers();
    for ( int iM = 0; iM < viewMgrs.count(); ++iM ) {
      SUIT_ViewManager* aViewManager = viewMgrs.at( iM );
      if ( aViewManager && aViewManager->getType() == SVTK_Viewer::Type() &&
           aViewManager->study()->id() == studyID ) {
        QVector<SUIT_ViewWindow*> views = aViewManager->getViews();
        for ( int iV = 0; iV < views.count(); ++iV ) {
          if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(views[iV])) {
            vtkRenderer *aRenderer = vtkWnd->getRenderer();
            vtkActorCollection *actors = aRenderer->GetActors();
            for (int i = 0; i < actors->GetNumberOfItems(); ++i ) {
              // size of actors changes inside the loop
              while(SMESH_Actor *actor = dynamic_cast<SMESH_Actor*>(actors->GetItemAsObject(i)))
              {
                vtkWnd->RemoveActor(actor);
                actor->Delete();
              }
            }
          }
        }
      }
    }
    TVisualObjCont::iterator anIter = VISUAL_OBJ_CONT.begin();
    for ( ; anIter != VISUAL_OBJ_CONT.end(); ++anIter ) {
      int curId = anIter->first.first;
      if ( curId == studyID ) {
        // for unknown reason, object destructor is not called, so clear object manually
        anIter->second->GetUnstructuredGrid()->SetCells(0,0,0);
        anIter->second->GetUnstructuredGrid()->SetPoints(0);
        VISUAL_OBJ_CONT.erase( anIter-- );  // dercement occures before erase()
      }
    }
  }

  //================================================================================
  /*!
   * \brief Notify the user on problems during visualization
   */
  //================================================================================

  void OnVisuException()
  {
    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      // PAL16774 (Crash after display of many groups). Salome sometimes crashes just
      // after or at showing this message, so we do an additional check of available memory
//       char* buf = new char[100*1024];
//       delete [] buf;
      SUIT_MessageBox::warning(SMESHGUI::desktop(), QObject::tr("SMESH_WRN_WARNING"),
			       QObject::tr("SMESH_VISU_PROBLEM"));
    } catch (...) {
      // no more memory at all: last resort
      MESSAGE_BEGIN ( "SMESHGUI_VTKUtils::OnVisuException(), exception even at showing a message!!!" <<
    		      std::endl << "Try to remove all visual data..." );
      if (theVISU_MemoryReserve) {
        delete theVISU_MemoryReserve;
        theVISU_MemoryReserve = 0;
      }
      RemoveAllObjectsWithActors();
      SUIT_MessageBox::warning(SMESHGUI::desktop(), QObject::tr("SMESH_WRN_WARNING"),
			       QObject::tr("SMESH_VISU_PROBLEM_CLEAR"));
      MESSAGE_END ( "...done" );
    }
  }
  //================================================================================
  /*!
   * \brief Returns an updated visual object
   */
  //================================================================================

  TVisualObjPtr GetVisualObj(int theStudyId, const char* theEntry){
    TVisualObjPtr aVisualObj;
    TVisualObjCont::key_type aKey(theStudyId,theEntry);
    try{
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      TVisualObjCont::iterator anIter = VISUAL_OBJ_CONT.find(aKey);
      if(anIter != VISUAL_OBJ_CONT.end()){
	aVisualObj = anIter->second;
      }else{
        SalomeApp_Application* app =
          dynamic_cast<SalomeApp_Application*>( SMESHGUI::activeStudy()->application() );
	_PTR(Study) aStudy = SMESHGUI::activeStudy()->studyDS();
	_PTR(SObject) aSObj = aStudy->FindObjectID(theEntry);
	if(aSObj){
	  _PTR(GenericAttribute) anAttr;
	  if(aSObj->FindAttribute(anAttr,"AttributeIOR")){
	    _PTR(AttributeIOR) anIOR = anAttr;
	    CORBA::String_var aVal = anIOR->Value().c_str();
	    CORBA::Object_var anObj = app->orb()->string_to_object( aVal.in() );
	    if(!CORBA::is_nil(anObj)){
	      //Try narrow to SMESH_Mesh interface
	      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow(anObj);
	      if(!aMesh->_is_nil()){
		aVisualObj.reset(new SMESH_MeshObj(aMesh));
		TVisualObjCont::value_type aValue(aKey,aVisualObj);
		VISUAL_OBJ_CONT.insert(aValue);
	      }
	      //Try narrow to SMESH_Group interface
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
		  TVisualObjCont::value_type aValue(aKey,aVisualObj);
		  VISUAL_OBJ_CONT.insert(aValue);
		}
	      }
	      //Try narrow to SMESH_subMesh interface
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
		  TVisualObjCont::value_type aValue(aKey,aVisualObj);
		  VISUAL_OBJ_CONT.insert(aValue);
		}
	      }
	    }
	  }
	}
      }
    }catch(...){
      INFOS("GetMeshObj - There is no SMESH_Mesh object for the SALOMEDS::Strudy and Entry!!!");
      return TVisualObjPtr();
    }
    // Update object
    bool objModified = false;
    if ( aVisualObj ) {
      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        objModified = aVisualObj->Update();
      }
      catch (...) {
#ifdef _DEBUG_
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::GetVisualObj()" );
#endif
        RemoveVisualObjectWithActors( theEntry ); // remove this object
        OnVisuException();
        aVisualObj.reset();
      }
    }

    if ( objModified ) {
      // PAL16631. Mesurements showed that to show aVisualObj in SHADING(default) mode,
      // ~10 times more memory is used than it occupies.
      // Warn the user if there is less free memory than 30 sizes of a grid
      // TODO: estimate memory usage in other modes and take current mode into account
      int freeMB = SMDS_Mesh::CheckMemory(true);
      int usedMB = aVisualObj->GetUnstructuredGrid()->GetActualMemorySize() / 1024;
      if ( freeMB > 0 && usedMB * 30 > freeMB ) {
#ifdef _DEBUG_
        MESSAGE ( "SMESHGUI_VTKUtils::GetVisualObj(), freeMB=" << freeMB
               << ", usedMB=" << usedMB );
#endif
        bool continu = false;
        if ( usedMB * 10 > freeMB )
          // even dont try to show
          SUIT_MessageBox::warning(SMESHGUI::desktop(), QObject::tr("SMESH_WRN_WARNING"),
				   QObject::tr("SMESH_NO_MESH_VISUALIZATION"));
        else
          // there is a chance to succeed
          continu = SUIT_MessageBox::warning
            (SMESHGUI::desktop(),
             QObject::tr("SMESH_WRN_WARNING"),
             QObject::tr("SMESH_CONTINUE_MESH_VISUALIZATION"),
             SUIT_MessageBox::Yes | SUIT_MessageBox::No, 
	     SUIT_MessageBox::Yes ) == SUIT_MessageBox::Yes;
        if ( !continu ) {
          // remove the corresponding actors from all views
          RemoveVisualObjectWithActors( theEntry );
          aVisualObj.reset();
        }
      }
    }

    return aVisualObj;
  }


  /*! Return active view window, if it instantiates SVTK_ViewWindow class,
   *  overwise find or create corresponding view window, make it active and return it.
   *  \note Active VVTK_ViewWindow can be returned, because it inherits SVTK_ViewWindow.
   */
  SVTK_ViewWindow* GetViewWindow (const SalomeApp_Module* theModule,
                                  bool createIfNotFound)
  {
    SalomeApp_Application* anApp;
    if (theModule)
      anApp = theModule->getApp();
    else
      anApp = dynamic_cast<SalomeApp_Application*>
        (SUIT_Session::session()->activeApplication());

    if (anApp) {
      if (SVTK_ViewWindow* aView = dynamic_cast<SVTK_ViewWindow*>(anApp->desktop()->activeWindow()))
	return aView;

      SUIT_ViewManager* aViewManager =
        anApp->getViewManager(SVTK_Viewer::Type(), createIfNotFound);
      if (aViewManager) {
        if (SUIT_ViewWindow* aViewWindow = aViewManager->getActiveView()) {
          if (SVTK_ViewWindow* aView = dynamic_cast<SVTK_ViewWindow*>(aViewWindow)) {
            aViewWindow->raise();
            aViewWindow->setFocus();
            return aView;
          }
        }
      }
    }
    return NULL;
  }

  SVTK_ViewWindow* FindVtkViewWindow (SUIT_ViewManager* theMgr,
                                      SUIT_ViewWindow * theWindow)
  {
    if( !theMgr )
      return NULL;

    QVector<SUIT_ViewWindow*> views = theMgr->getViews();
    if( views.contains( theWindow ) )
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


  void RepaintCurrentView()
  {
    if (SVTK_ViewWindow* wnd = GetCurrentVtkView())
    {
      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        wnd->getRenderer()->Render();
        wnd->Repaint(false);
      }
      catch (...) {
#ifdef _DEBUG_
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::RepaintCurrentView()" );
#endif
        OnVisuException();
      }
    }
  }

  void RepaintViewWindow(SVTK_ViewWindow* theWindow)
  {
    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      theWindow->getRenderer()->Render();
      theWindow->Repaint();
    }
    catch (...) {
#ifdef _DEBUG_
      MESSAGE ( "Exception in SMESHGUI_VTKUtils::RepaintViewWindow(SVTK_ViewWindow*)" );
#endif
      OnVisuException();
    }
  }

  void RenderViewWindow(SVTK_ViewWindow* theWindow)
  {
    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      theWindow->getRenderer()->Render();
      theWindow->Repaint();
    }
    catch (...) {
#ifdef _DEBUG_
      MESSAGE ( "Exception in SMESHGUI_VTKUtils::RenderViewWindow(SVTK_ViewWindow*)" );
#endif
      OnVisuException();
    }
  }

  void FitAll(){
    if(SVTK_ViewWindow* wnd = GetCurrentVtkView() ){
      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        wnd->onFitAll();
        wnd->Repaint();
      }
      catch (...) {
#ifdef _DEBUG_
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::FitAll()" );
#endif
        OnVisuException();
      }
    }
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

	SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( SMESH::SObjectToObject( aSObj ));
	if(!CORBA::is_nil(aGroup))
	{
	  SALOMEDS::Color aColor = aGroup->GetColor();
	  if( !( aColor.R > 0 || aColor.G > 0 || aColor.B > 0 ) )
	  {
	    int r = 0, g = 0, b = 0;
	    SMESH::GetColor( "SMESH", "fill_color", r, g, b, QColor( 0, 170, 255 ) );
	    aColor.R = (float)r / 255.0;
	    aColor.G = (float)g / 255.0;
	    aColor.B = (float)b / 255.0;
	    aGroup->SetColor( aColor );
	  }
	  if( aGroup->GetType() == SMESH::NODE )
	    anActor->SetNodeColor( aColor.R, aColor.G, aColor.B );
	  else if( aGroup->GetType() == SMESH::EDGE )
	    anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B );
	  else
	    anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B );
	}
      }
    }
    return anActor;
  }


  void DisplayActor( SUIT_ViewWindow *theWnd, SMESH_Actor* theActor){
    if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(theWnd)){
      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        vtkWnd->AddActor(theActor);
        vtkWnd->Repaint();
      }
      catch (...) {
#ifdef _DEBUG_
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::DisplayActor()" );
#endif
        OnVisuException();
      }
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

  //================================================================================
  /*!
   * \brief Return true if there are no SMESH actors in a view
   */
  //================================================================================

  bool noSmeshActors(SUIT_ViewWindow *theWnd)
  {
    if(SVTK_ViewWindow* aViewWindow = GetVtkViewWindow(theWnd)) {
      vtkRenderer *aRenderer = aViewWindow->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor())
	if(dynamic_cast<SMESH_Actor*>(anAct))
          return false;
    }
    return true;
  }

  bool UpdateView(SUIT_ViewWindow *theWnd, EDisplaing theAction, const char* theEntry)
  {
    bool OK = false;
    SVTK_ViewWindow* aViewWnd = GetVtkViewWindow(theWnd);
    if (!aViewWnd)
      return OK;

    {
      OK = true;
      vtkRenderer *aRenderer = aViewWnd->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();

      switch (theAction) {
      case eDisplayAll: {
	while (vtkActor *anAct = aCollection->GetNextActor()) {
	  if (SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)) {
	    anActor->SetVisibility(true);
	  }
	}
	break;
      }
      case eDisplayOnly:
      case eEraseAll: {
	while (vtkActor *anAct = aCollection->GetNextActor()) {
	  if (SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)) {
	    anActor->SetVisibility(false);
	  }
	}
      }
      default: {
	if (SMESH_Actor *anActor = FindActorByEntry(theWnd,theEntry)) {
	  switch (theAction) {
	    case eDisplay:
	    case eDisplayOnly:
	      anActor->SetVisibility(true);
	      if (theAction == eDisplayOnly) aRenderer->ResetCameraClippingRange();
	      break;
	    case eErase:
	      anActor->SetVisibility(false);
	      break;
	  }
	} else {
	  switch (theAction) {
	  case eDisplay:
	  case eDisplayOnly:
            {
              SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>(theWnd->getViewManager()->study());
              _PTR(Study) aDocument = aStudy->studyDS();
              // Pass non-visual objects (hypotheses, etc.), return true in this case
              CORBA::Long anId = aDocument->StudyId();
              if (TVisualObjPtr aVisualObj = GetVisualObj(anId,theEntry))
              {
                if ((anActor = CreateActor(aDocument,theEntry,true))) {
                  bool needFitAll = noSmeshActors(theWnd); // fit for the first object only
                  DisplayActor(theWnd,anActor);
                  // FitAll(); - PAL16770(Display of a group performs an automatic fit all)
                  if (needFitAll) FitAll();
                } else {
                  OK = false;
                }
              }
              break;
            }
	  }
	}
      }
      }
    }
    return OK;
  }


  bool UpdateView(EDisplaing theAction, const char* theEntry){
    SalomeApp_Study* aStudy = dynamic_cast< SalomeApp_Study* >( GetActiveStudy() );
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( aStudy->application() );
    SUIT_ViewWindow *aWnd = app->activeViewManager()->getActiveView();
    return UpdateView(aWnd,theAction,theEntry);
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
	      if (!Update(anActor->getIO(),anActor->GetVisibility()))
                break; // avoid multiple warinings if visu failed
	  }
	}
      }else{
	SALOME_ListIteratorOfListIO anIter( selected );
	for( ; anIter.More(); anIter.Next()){
	  Handle(SALOME_InteractiveObject) anIO = anIter.Value();
	  if ( !Update(anIO,true) )
            break; // avoid multiple warinings if visu failed
	}
      }
      RepaintCurrentView();
    }
  }


  bool Update(const Handle(SALOME_InteractiveObject)& theIO, bool theDisplay)
  {
    _PTR(Study) aStudy = GetActiveStudyDocument();
    CORBA::Long anId = aStudy->StudyId();
    if ( TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,theIO->getEntry())) {
      if ( theDisplay )
        UpdateView(SMESH::eDisplay,theIO->getEntry());
      return true;
    }
    return false;
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

    QVector<SUIT_ViewWindow*> views = vm->getViews();

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
           SP2 = mgr->doubleValue( "SMESH", "selection_precision_element", 0.001 ),
	   SP3 = mgr->doubleValue( "SMESH", "selection_precision_object", 0.025 );

    for ( int i=0, n=views.count(); i<n; i++ ){
      // update VTK viewer properties
      if(SVTK_ViewWindow* aVtkView = GetVtkViewWindow( views[i] )){
	// mesh element selection
	aVtkView->SetSelectionProp(aSelColor.red()/255.,
				   aSelColor.green()/255.,
				   aSelColor.blue()/255.,
				   SW );
	// tolerances
	aVtkView->SetSelectionTolerance(SP1, SP2, SP3);

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
    for ( int id = SMESH::NodeFilter; theSelector && id < SMESH::LastFilter; id++ )
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
    for( ; anIter != anIdContainer.end(); anIter++)
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
	typedef std::set<int> TIdContainer;
	TIdContainer anIdContainer;
	for( int i = 1; i <= aMapIndex.Extent(); i++)
	  anIdContainer.insert(aMapIndex(i));
	TIdContainer::const_iterator anIter = anIdContainer.begin();
	for( ; anIter != anIdContainer.end(); anIter++){
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
} // end of namespace SMESH
