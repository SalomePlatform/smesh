// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_VTKUtils.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_VTKUtils.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Filter.h"
#include "SMESHGUI_Utils.h"
#include "SMDS_Mesh.hxx"
#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_CellLabelActor.h"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_NodeLabelActor.h"
#include "SMESH_ObjectDef.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ViewManager.h>
#include <SUIT_ResourceMgr.h>

#include <SALOME_ListIO.hxx>

#include <SVTK_Selector.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

#include <VTKViewer_Algorithm.h>

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
#include <vtkMath.h>
#include <vtkRenderer.h>
#include <vtkActorCollection.h>
#include <vtkUnstructuredGrid.h>

// OCCT includes
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

  void RemoveVisualObjectWithActors( const char* theEntry, bool fromAllViews )
  {
    SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>(SUIT_Session::session()->activeApplication());
    if(!app)
      return;
    SalomeApp_Study* aStudy  = dynamic_cast<SalomeApp_Study*>(app->activeStudy());
    if(!aStudy)
      return;
    ViewManagerList aList;

    if(fromAllViews) {
      app->viewManagers(SVTK_Viewer::Type() , aList);
    } else {
      SUIT_ViewManager* aVM = app->getViewManager(SVTK_Viewer::Type(), true);
      if(aVM)
        aList.append(aVM);
    }    
    bool actorRemoved = false;
    ViewManagerList::ConstIterator it = aList.begin();
    SUIT_ViewManager* aViewManager = 0;
    for( ; it!=aList.end();it++) {
      aViewManager = *it;
      QVector<SUIT_ViewWindow*> views = aViewManager->getViews();
      for ( int iV = 0; iV < views.count(); ++iV ) {
        if ( SMESH_Actor* actor = FindActorByEntry( views[iV], theEntry)) {
          if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(views[iV])) {
            vtkWnd->RemoveActor(actor);
            actorRemoved = true;
          }
          actor->Delete();
        }
      }
    }
    
    if (aViewManager ) {
      int aStudyId = aViewManager->study()->id();
      TVisualObjCont::key_type aKey(aStudyId,theEntry);
      TVisualObjCont::iterator anIter = VISUAL_OBJ_CONT.find(aKey);
      if(anIter != VISUAL_OBJ_CONT.end()) {
        // for unknown reason, object destructor is not called, so clear object manually
        anIter->second->GetUnstructuredGrid()->SetCells(0,0,0,0,0);
        anIter->second->GetUnstructuredGrid()->SetPoints(0);
      }
      VISUAL_OBJ_CONT.erase(aKey);
    }

    if(actorRemoved)
      aStudy->setVisibilityState(theEntry, Qtx::HiddenState);
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
            VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
            vtkActorCollection *actors = aCopy.GetActors();
            for (int i = 0; i < actors->GetNumberOfItems(); ++i ) {
              // size of actors changes inside the loop
              if (SMESH_Actor *actor = dynamic_cast<SMESH_Actor*>(actors->GetItemAsObject(i)))
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
      anIter->second->GetUnstructuredGrid()->SetCells(0,0,0,0,0);
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
            VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
            vtkActorCollection *actors = aCopy.GetActors();
            for (int i = 0; i < actors->GetNumberOfItems(); ++i ) {
              // size of actors changes inside the loop
              if(SMESH_Actor *actor = dynamic_cast<SMESH_Actor*>(actors->GetItemAsObject(i)))
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
    for ( ; anIter != VISUAL_OBJ_CONT.end(); ) {
      int curId = anIter->first.first;
      if ( curId == studyID ) {
        // for unknown reason, object destructor is not called, so clear object manually
        anIter->second->GetUnstructuredGrid()->SetCells(0,0,0,0,0);
        anIter->second->GetUnstructuredGrid()->SetPoints(0);
        VISUAL_OBJ_CONT.erase( anIter++ ); // anIter++ returns a copy of self before incrementing
      }
      else {
        anIter++;
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
      OCC_CATCH_SIGNALS;
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

  TVisualObjPtr GetVisualObj(int theStudyId, const char* theEntry, bool nulData){
    TVisualObjPtr aVisualObj;
    TVisualObjCont::key_type aKey(theStudyId,theEntry);
    try{
      OCC_CATCH_SIGNALS;
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
        OCC_CATCH_SIGNALS;
        if (nulData)
          objModified = aVisualObj->NulData();
        else
          objModified = aVisualObj->Update();
      }
      catch (...) {
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::GetVisualObj()" );
        RemoveVisualObjectWithActors( theEntry ); // remove this object
        OnVisuException();
        aVisualObj.reset();
      }
    }

    if ( objModified ) {
      // PAL16631. Mesurements showed that to show aVisualObj in SHADING(default) mode,
      // ~5 times more memory is used than it occupies.
      // Warn the user if there is less free memory than 30 sizes of a grid
      // TODO: estimate memory usage in other modes and take current mode into account
      int freeMB = SMDS_Mesh::CheckMemory(true);
      int usedMB = aVisualObj->GetUnstructuredGrid()->GetActualMemorySize() / 1024;
      //MESSAGE("SMESHGUI_VTKUtils::GetVisualObj(), freeMB=" << freeMB << ", usedMB=" <<usedMB);
      if ( freeMB > 0 && usedMB * 5 > freeMB ) {
        bool continu = false;
        if ( usedMB * 3 > freeMB )
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
        OCC_CATCH_SIGNALS;
        wnd->getRenderer()->Render();
        wnd->Repaint(false);
      }
      catch (...) {
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::RepaintCurrentView()" );
        OnVisuException();
      }
    }
  }

  void RepaintViewWindow(SVTK_ViewWindow* theWindow)
  {
    try {
      OCC_CATCH_SIGNALS;
      theWindow->getRenderer()->Render();
      theWindow->Repaint();
    }
    catch (...) {
      MESSAGE ( "Exception in SMESHGUI_VTKUtils::RepaintViewWindow(SVTK_ViewWindow*)" );
      OnVisuException();
    }
  }

  void RenderViewWindow(SVTK_ViewWindow* theWindow)
  {
    try {
      OCC_CATCH_SIGNALS;
      theWindow->getRenderer()->Render();
      theWindow->Repaint();
    }
    catch (...) {
      MESSAGE ( "Exception in SMESHGUI_VTKUtils::RenderViewWindow(SVTK_ViewWindow*)" );
      OnVisuException();
    }
  }

  void FitAll(){
    if(SVTK_ViewWindow* wnd = GetCurrentVtkView() ){
      try {
        OCC_CATCH_SIGNALS;
        wnd->onFitAll();
        wnd->Repaint();
      }
      catch (...) {
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::FitAll()" );
        OnVisuException();
      }
    }
  }


  SMESH_Actor* FindActorByEntry(SUIT_ViewWindow *theWindow,
                                const char* theEntry)
  {
    if(SVTK_ViewWindow* aViewWindow = GetVtkViewWindow(theWindow)){
      vtkRenderer *aRenderer = aViewWindow->getRenderer();
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
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
        if(!CORBA::is_nil(aGroup) && anActor)
        {
          QColor c;
          int deltaF, deltaV;
          SMESH::GetColor( "SMESH", "fill_color", c, deltaF, "0,170,255|-100"  );
          SMESH::GetColor( "SMESH", "volume_color", c, deltaV, "255,0,170|-100"  );
          c = SMESH::GetColor( "SMESH", "default_grp_color", c );
          SALOMEDS::Color aColor = aGroup->GetColor();
          if( !( aColor.R > 0 || aColor.G > 0 || aColor.B > 0 ))
          {
            aColor.R = c.redF();
            aColor.G = c.greenF();
            aColor.B = c.blueF();
            aGroup->SetColor( aColor );
          }
          if( aGroup->GetType() == SMESH::NODE )
            anActor->SetNodeColor( aColor.R, aColor.G, aColor.B );
          else if( aGroup->GetType() == SMESH::EDGE )
            anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B );
          else if( aGroup->GetType() == SMESH::ELEM0D )
            anActor->Set0DColor( aColor.R, aColor.G, aColor.B );
          else if( aGroup->GetType() == SMESH::BALL )
            anActor->SetBallColor( aColor.R, aColor.G, aColor.B );
          else if( aGroup->GetType() == SMESH::VOLUME )
            anActor->SetVolumeColor( aColor.R, aColor.G, aColor.B, deltaV );
          else
            anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B, deltaF );
        }
      }
    }
    //MESSAGE("CreateActor " << anActor);
    if( anActor )
      if( SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI() )
        aSMESHGUI->addActorAsObserver( anActor );
    return anActor;
  }


  void DisplayActor( SUIT_ViewWindow *theWnd, SMESH_Actor* theActor){
    if(SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(theWnd)){
      try {
        OCC_CATCH_SIGNALS;
        //MESSAGE("DisplayActor " << theActor);
        vtkWnd->AddActor(theActor);
        vtkWnd->Repaint();
      }
      catch (...) {
        MESSAGE ( "Exception in SMESHGUI_VTKUtils::DisplayActor()" );
        OnVisuException();
      }
    }
  }


  void RemoveActor( SUIT_ViewWindow *theWnd, SMESH_Actor* theActor)
  {
    if ( SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(theWnd)) {
      //MESSAGE("RemoveActor " << theActor);
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
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor())
        if(dynamic_cast<SMESH_Actor*>(anAct))
          return false;
    }
    return true;
  }

  bool UpdateView(SUIT_ViewWindow *theWnd, EDisplaing theAction, const char* theEntry)
  {
    //MESSAGE("UpdateView");
    bool OK = false;
    SVTK_ViewWindow* aViewWnd = GetVtkViewWindow(theWnd);
    if (!aViewWnd)
      return OK;

    SVTK_ViewWindow* vtkWnd = GetVtkViewWindow(theWnd);
    if (!vtkWnd)
      return OK;

    SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>( vtkWnd->getViewManager()->study() );
    
    if (!aStudy)
      return OK;

    {
      OK = true;
      vtkRenderer *aRenderer = aViewWnd->getRenderer();
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
      aCollection->InitTraversal();

      switch (theAction) {
      case eDisplayAll: {
        while (vtkActor *anAct = aCollection->GetNextActor()) {
          if (SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)) {
            //MESSAGE("--- display " << anActor);
            anActor->SetVisibility(true);

            if(anActor->hasIO()){
              Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
              if(anIO->hasEntry()){
                aStudy->setVisibilityState(anIO->getEntry(), Qtx::ShownState);
              }
            }
          }
        }
        break;
      }
      case eDisplayOnly:
      case eEraseAll: {
        //MESSAGE("---case eDisplayOnly");
        while (vtkActor *anAct = aCollection->GetNextActor()) {
          if (SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)) {
            //MESSAGE("--- erase " << anActor);
            anActor->SetVisibility(false);
          }
        }
        aStudy->setVisibilityStateForAll(Qtx::HiddenState);
      }
      default: {
        if (SMESH_Actor *anActor = FindActorByEntry(theWnd,theEntry)) {
          switch (theAction) {
            case eDisplay:
            case eDisplayOnly:
              //MESSAGE("--- display " << anActor);
              anActor->Update();
              anActor->SetVisibility(true);
              if (theAction == eDisplayOnly) aRenderer->ResetCameraClippingRange();
              aStudy->setVisibilityState(theEntry, Qtx::ShownState);
              break;
            case eErase:
              //MESSAGE("--- erase " << anActor);
              anActor->SetVisibility(false);
              aStudy->setVisibilityState(theEntry, Qtx::HiddenState);
              break;
            default:;
          }
        } else {
          switch (theAction) {
          case eDisplay:
          case eDisplayOnly:
            {
              //MESSAGE("---");
              SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>(theWnd->getViewManager()->study());
              _PTR(Study) aDocument = aStudy->studyDS();
              // Pass non-visual objects (hypotheses, etc.), return true in this case
              CORBA::Long anId = aDocument->StudyId();
              TVisualObjPtr aVisualObj;
              if ( (aVisualObj = GetVisualObj(anId,theEntry)) && aVisualObj->IsValid())
              {
                if ((anActor = CreateActor(aDocument,theEntry,true))) {
                  bool needFitAll = noSmeshActors(theWnd); // fit for the first object only
                  DisplayActor(theWnd,anActor);
                  aStudy->setVisibilityState(theEntry, Qtx::ShownState);
                  // FitAll(); - PAL16770(Display of a group performs an automatic fit all)
                  if (needFitAll) FitAll();
                } else {
                  OK = false;
                }
              }
              break;
            }
          default:;
          }
        }
      }
      }
    }
    return OK;
  }


  bool UpdateView(EDisplaing theAction, const char* theEntry) {
    //MESSAGE("UpdateView");
    SalomeApp_Study* aStudy = dynamic_cast< SalomeApp_Study* >( GetActiveStudy() );
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( aStudy->application() );
    if ( SUIT_ViewManager* vm = app->activeViewManager() )
    {
      SUIT_ViewWindow *aWnd = vm->getActiveView();
      return UpdateView(aWnd,theAction,theEntry);
    }
    return false;
  }

  void UpdateView(){
    if ( SVTK_ViewWindow* aWnd = SMESH::GetCurrentVtkView()) {
      LightApp_SelectionMgr* mgr = SMESHGUI::selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      if( selected.Extent() == 0){
        vtkRenderer* aRenderer = aWnd->getRenderer();
        VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
        vtkActorCollection *aCollection = aCopy.GetActors();
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
    //MESSAGE("Update");
    _PTR(Study) aStudy = GetActiveStudyDocument();
    CORBA::Long anId = aStudy->StudyId();
    if ( TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,theIO->getEntry())) {
      if ( theDisplay )
        UpdateView(SMESH::eDisplay,theIO->getEntry());
      return true;
    }
    return false;
  }

  bool UpdateNulData(const Handle(SALOME_InteractiveObject)& theIO, bool theDisplay)
  {
    //MESSAGE("UpdateNulData");
    _PTR(Study) aStudy = GetActiveStudyDocument();
    CORBA::Long anId = aStudy->StudyId();
    if ( TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,theIO->getEntry(), true)) {
      if ( theDisplay )
        UpdateView(SMESH::eDisplay,theIO->getEntry());
      return true;
    }
    return false;
  }

  void UpdateSelectionProp( SMESHGUI* theModule )
  {
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

    QColor
      aHiColor = mgr->colorValue( "SMESH", "selection_object_color", Qt::white ),
      aSelColor = mgr->colorValue( "SMESH", "selection_element_color", Qt::yellow ),
      aPreColor = mgr->colorValue( "SMESH", "highlight_color", Qt::cyan );

    int aElem0DSize = mgr->integerValue("SMESH", "elem0d_size", 5);
    // int aBallSize   = mgr->integerValue("SMESH", "ball_elem_size", 5);
    int aLineWidth  = mgr->integerValue("SMESH", "element_width", 1);
    int maxSize = aElem0DSize;
    if (aElem0DSize > maxSize) maxSize = aElem0DSize;
    if (aLineWidth > maxSize) maxSize = aLineWidth;
    //  if (aBallSize > maxSize) maxSize = aBallSize;

    double
      SP1 = mgr->doubleValue( "SMESH", "selection_precision_node", 0.025 ),
      SP2 = mgr->doubleValue( "SMESH", "selection_precision_element", 0.001 ),
      SP3 = mgr->doubleValue( "SMESH", "selection_precision_object", 0.025 );

    for ( int i=0, n=views.count(); i<n; i++ )
    {
      // update VTK viewer properties
      if ( SVTK_ViewWindow* aVtkView = GetVtkViewWindow( views[i] ))
      {
        // mesh element selection
        aVtkView->SetSelectionProp(aSelColor.red()/255.,
                                   aSelColor.green()/255.,
                                   aSelColor.blue()/255.);
        // tolerances
        aVtkView->SetSelectionTolerance(SP1, SP2, SP3);

        // pre-selection
        aVtkView->SetPreselectionProp(aPreColor.red()/255.,
                                      aPreColor.green()/255.,
                                      aPreColor.blue()/255.);
        // update actors
        vtkRenderer* aRenderer = aVtkView->getRenderer();
        VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
        vtkActorCollection *aCollection = aCopy.GetActors();
        aCollection->InitTraversal();
        while ( vtkActor *anAct = aCollection->GetNextActor() ) {
          if ( SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct) ) {
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


  void UpdateFontProp( SMESHGUI* theModule )
  {
    if ( !theModule ) return;

    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( theModule->application() );
    if ( !app ) return;

    SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( theModule );
    if ( !mgr ) return;
    //
    double anRGBNd[3] = {1,1,1};
    SMESH::GetColor( "SMESH", "numbering_node_color", anRGBNd[0], anRGBNd[1], anRGBNd[2], QColor( 255, 255, 255 ) );
    int aSizeNd = 10;
    SMESH::LabelFont aFamilyNd = SMESH::FntTimes;
    bool aBoldNd    = true;
    bool anItalicNd = false;
    bool aShadowNd  = false;

    if ( mgr->hasValue( "SMESH", "numbering_node_font" ) ) {
      QFont f = mgr->fontValue( "SMESH", "numbering_node_font" );
      if ( f.family()      == "Arial" )   aFamilyNd = SMESH::FntArial;
      else if ( f.family() == "Courier" ) aFamilyNd = SMESH::FntCourier;
      else if ( f.family() == "Times" )   aFamilyNd = SMESH::FntTimes;
      aBoldNd    = f.bold();
      anItalicNd = f.italic();
      aShadowNd  = f.overline();
      aSizeNd    = f.pointSize();
    }
    //
    double anRGBEl[3] = {0,1,0};
    SMESH::GetColor( "SMESH", "numbering_elem_color", anRGBEl[0], anRGBEl[1], anRGBEl[2], QColor( 0, 255, 0 ) );
    int aSizeEl = 12;
    SMESH::LabelFont aFamilyEl = SMESH::FntTimes;
    bool aBoldEl    = true;
    bool anItalicEl = false;
    bool aShadowEl  = false;

    if ( mgr->hasValue( "SMESH", "numbering_elem_font" ) ) {
      QFont f = mgr->fontValue( "SMESH", "numbering_elem_font" );

      if ( f.family()      == "Arial" )   aFamilyEl = SMESH::FntArial;
      else if ( f.family() == "Courier" ) aFamilyEl = SMESH::FntCourier;
      else if ( f.family() == "Times" )   aFamilyEl = SMESH::FntTimes;    
      aBoldEl    = f.bold();
      anItalicEl = f.italic();
      aShadowEl  = f.overline();
      aSizeEl    = f.pointSize();
    }
    //
    ViewManagerList vmList;
    app->viewManagers( SVTK_Viewer::Type(), vmList );
    foreach ( SUIT_ViewManager* vm, vmList ) {
      QVector<SUIT_ViewWindow*> views = vm->getViews();
      foreach ( SUIT_ViewWindow* vw, views ) {
        // update VTK viewer properties
        if ( SVTK_ViewWindow* aVtkView = GetVtkViewWindow( vw ) ) {
          // update actors
          vtkRenderer* aRenderer = aVtkView->getRenderer();
          VTK::ActorCollectionCopy aCopy( aRenderer->GetActors() );
          vtkActorCollection* aCollection = aCopy.GetActors();
          aCollection->InitTraversal();
          while ( vtkActor* anAct = aCollection->GetNextActor() ) {
            if ( SMESH_NodeLabelActor* anActor = dynamic_cast< SMESH_NodeLabelActor* >( anAct ) ) {
              anActor->SetFontProperties( aFamilyNd, aSizeNd, aBoldNd, anItalicNd, aShadowNd, anRGBNd[0], anRGBNd[1], anRGBNd[2] );
            }
            else if ( SMESH_CellLabelActor* anActor = dynamic_cast< SMESH_CellLabelActor* >( anAct ) ) {
              anActor->SetFontProperties( aFamilyEl, aSizeEl, aBoldEl, anItalicEl, aShadowEl, anRGBEl[0], anRGBEl[1], anRGBEl[2] );
            }
          }
          aVtkView->Repaint( false ); 
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
  void SetPointRepresentation(bool theIsVisible)
  {
    if ( SVTK_ViewWindow* aViewWindow = GetCurrentVtkView() ) {
      vtkRenderer *aRenderer = aViewWindow->getRenderer();
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
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


  void SetPickable(SMESH_Actor* theActor)
  {
    if ( SVTK_ViewWindow* aWnd = GetCurrentVtkView() ) {
      int anIsAllPickable = (theActor == NULL);
      vtkRenderer *aRenderer = aWnd->getRenderer();
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
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
  int GetNameOfSelectedNodes(SVTK_Selector*                          theSelector,
                             const Handle(SALOME_InteractiveObject)& theIO,
                             QString&                                theName)
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

  int GetNameOfSelectedNodes(LightApp_SelectionMgr *theMgr, QString& theName)
  {
    theName = "";
    SALOME_ListIO selected; theMgr->selectedObjects( selected );
    if(selected.Extent() == 1){
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      return GetNameOfSelectedNodes(theMgr,anIO,theName);
    }
    return -1;
  }


  int GetNameOfSelectedElements(LightApp_SelectionMgr *                 theMgr,
                                const Handle(SALOME_InteractiveObject)& theIO,
                                QString&                                theName)
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
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
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

  //----------------------------------------------------------------------------
  // internal function
  void ComputeBoundsParam( double theBounds[6],
                           double theDirection[3],
                           double theMinPnt[3],
                           double& theMaxBoundPrj,
                           double& theMinBoundPrj )
  {
    //Enlarge bounds in order to avoid conflicts of precision
    for(int i = 0; i < 6; i += 2){
      static double EPS = 1.0E-3;
      double aDelta = (theBounds[i+1] - theBounds[i])*EPS;
      theBounds[i] -= aDelta;
      theBounds[i+1] += aDelta;
    }

    double aBoundPoints[8][3] = { {theBounds[0],theBounds[2],theBounds[4]},
                                  {theBounds[1],theBounds[2],theBounds[4]},
                                  {theBounds[0],theBounds[3],theBounds[4]},
                                  {theBounds[1],theBounds[3],theBounds[4]},
                                  {theBounds[0],theBounds[2],theBounds[5]},
                                  {theBounds[1],theBounds[2],theBounds[5]},
                                  {theBounds[0],theBounds[3],theBounds[5]},
                                  {theBounds[1],theBounds[3],theBounds[5]}};

    int aMaxId = 0;
    theMaxBoundPrj = vtkMath::Dot(theDirection,aBoundPoints[aMaxId]);
    theMinBoundPrj = theMaxBoundPrj;
    for(int i = 1; i < 8; i++){
      double aTmp = vtkMath::Dot(theDirection,aBoundPoints[i]);
      if(theMaxBoundPrj < aTmp){
        theMaxBoundPrj = aTmp;
        aMaxId = i;
      }
      if(theMinBoundPrj > aTmp){
        theMinBoundPrj = aTmp;
      }
    }
    double *aMinPnt = aBoundPoints[aMaxId];
    theMinPnt[0] = aMinPnt[0];
    theMinPnt[1] = aMinPnt[1];
    theMinPnt[2] = aMinPnt[2];
  }

  // internal function
  void DistanceToPosition( double theBounds[6],
                           double theDirection[3],
                           double theDist,
                           double thePos[3] )
  {
    double aMaxBoundPrj, aMinBoundPrj, aMinPnt[3];
    ComputeBoundsParam(theBounds,theDirection,aMinPnt,aMaxBoundPrj,aMinBoundPrj);
    double aLength = (aMaxBoundPrj-aMinBoundPrj)*theDist;
    thePos[0] = aMinPnt[0]-theDirection[0]*aLength;
    thePos[1] = aMinPnt[1]-theDirection[1]*aLength;
    thePos[2] = aMinPnt[2]-theDirection[2]*aLength;
  }

  // internal function (currently unused, left just in case)
  void PositionToDistance( double theBounds[6],
                           double theDirection[3],
                           double thePos[3],
                           double& theDist )
  {
    double aMaxBoundPrj, aMinBoundPrj, aMinPnt[3];
    ComputeBoundsParam(theBounds,theDirection,aMinPnt,aMaxBoundPrj,aMinBoundPrj);
    double aPrj = vtkMath::Dot(theDirection,thePos);
    theDist = (aPrj-aMinBoundPrj)/(aMaxBoundPrj-aMinBoundPrj);
  }

  bool ComputeClippingPlaneParameters( std::list<vtkActor*> theActorList,
                                       double theNormal[3],
                                       double theDist,
                                       double theBounds[6],
                                       double theOrigin[3] )
  {
    bool anIsOk = false;
    anIsOk = ComputeBounds( theActorList, theBounds );


    if( !anIsOk )
      return false;

    DistanceToPosition( theBounds, theNormal, theDist, theOrigin );
    return true;
  }

  bool ComputeBounds( std::list<vtkActor*> theActorList,
                      double theBounds[6])
  {
    bool anIsOk = false;
    theBounds[0] = theBounds[2] = theBounds[4] = VTK_DOUBLE_MAX;
    theBounds[1] = theBounds[3] = theBounds[5] = -VTK_DOUBLE_MAX;
    std::list<vtkActor*>::iterator anIter = theActorList.begin();
    for( ; anIter != theActorList.end(); anIter++ ) {
      if( vtkActor* aVTKActor = *anIter ) {
        if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) ) {
          double aBounds[6];
          anActor->GetUnstructuredGrid()->GetBounds( aBounds );
          theBounds[0] = std::min( theBounds[0], aBounds[0] );
          theBounds[1] = std::max( theBounds[1], aBounds[1] );
          theBounds[2] = std::min( theBounds[2], aBounds[2] );
          theBounds[3] = std::max( theBounds[3], aBounds[3] );
          theBounds[4] = std::min( theBounds[4], aBounds[4] );
          theBounds[5] = std::max( theBounds[5], aBounds[5] );
          anIsOk = true;
        }
      }
    }
    return anIsOk;
  }

#ifndef DISABLE_PLOT2DVIEWER
  //================================================================================
  /*!
   * \brief Find all SMESH_Actor's in the View Window.
   * If actor constains Plot2d_Histogram object remove it from each Plot2d Viewer.
   */
  //================================================================================

  void ClearPlot2Viewers( SUIT_ViewWindow* theWindow )
  {
    if ( SVTK_ViewWindow* aViewWindow = GetVtkViewWindow(theWindow) ) {
      vtkRenderer *aRenderer = aViewWindow->getRenderer();
      VTK::ActorCollectionCopy aCopy(aRenderer->GetActors());
      vtkActorCollection *aCollection = aCopy.GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor()){
        if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
          if(anActor->hasIO() && anActor->GetPlot2Histogram() ){
            ProcessIn2DViewers(anActor,RemoveFrom2dViewer);
          }
        }
      }
    }
  }

#endif

} // end of namespace SMESH
