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

#include <vtkRenderer.h>
#include <vtkActorCollection.h>

#include <TColStd_IndexedMapOfInteger.hxx>

#include "QAD_Config.h"
#include "QAD_Desktop.h"
#include "QAD_Study.h"
#include "QAD_Settings.h"
#include "QAD_RightFrame.h"

#include "SALOME_Selection.h"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "VTKViewer_ViewFrame.h"
#include "VTKViewer_RenderWindow.h"
#include "VTKViewer_InteractorStyleSALOME.h"
#include "VTKViewer_RenderWindowInteractor.h"

#include "utilities.h"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SMESH_Gen)
#include CORBA_CLIENT_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(SMESH_Group)
#include CORBA_CLIENT_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

#include "SMESH_Actor.h"
#include "SMESH_ObjectDef.h"

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
	SALOMEDS::Study_var aStudy = QAD_Application::getDesktop()->getActiveStudy()->getStudyDocument();
	SALOMEDS::SObject_var aSObj = aStudy->FindObjectID(theEntry);
	if(!CORBA::is_nil(aSObj)){
	  SALOMEDS::GenericAttribute_var anAttr;
	  if(aSObj->FindAttribute(anAttr,"AttributeIOR")){
	    SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    CORBA::String_var aVal = anIOR->Value();
	    CORBA::Object_var anObj = aStudy->ConvertIORToObject(aVal.in());
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
		SALOMEDS::SObject_var aFatherSObj = aSObj->GetFather();
		if(aFatherSObj->_is_nil()) return aVisualObj;
		aFatherSObj = aFatherSObj->GetFather();
		if(aFatherSObj->_is_nil()) return aVisualObj;
		CORBA::String_var anEntry = aFatherSObj->GetID();
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
		SALOMEDS::SObject_var aFatherSObj = aSObj->GetFather();
		if(aFatherSObj->_is_nil()) return aVisualObj;
		aFatherSObj = aFatherSObj->GetFather();
		if(aFatherSObj->_is_nil()) return aVisualObj;
		CORBA::String_var anEntry = aFatherSObj->GetID();
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


  VTKViewer_ViewFrame* FindVtkViewFrame(QAD_Study* theStudy,
					QAD_StudyFrame* theStudyFrame)
  {
    QList<QAD_Study>& aStudies = 
      QAD_Application::getDesktop()->getActiveApp()->getStudies();
    if(aStudies.containsRef(theStudy)){
      const QList<QAD_StudyFrame>& aStudyFrames = theStudy->getStudyFrames();
      if(aStudyFrames.containsRef(theStudyFrame)){
	return GetVtkViewFrame(theStudyFrame);
      }
    }
    return NULL;
  }


  VTKViewer_ViewFrame* GetVtkViewFrame(QAD_StudyFrame* theStudyFrame){
    QAD_ViewFrame* aViewFrame = theStudyFrame->getRightFrame()->getViewFrame();
    return dynamic_cast<VTKViewer_ViewFrame*>(aViewFrame);
  }


  VTKViewer_ViewFrame* GetCurrentVtkView(){
    return GetVtkViewFrame(GetActiveStudy()->getActiveStudyFrame());
  }


  void RepaintViewFrame(VTKViewer_ViewFrame* theFrame)
  {
    theFrame->Repaint();
  }


  void RenderViewFrame(VTKViewer_ViewFrame* theFrame)
  {
    theFrame->getRW()->getRenderWindow()->Render();
    theFrame->Repaint();
  }


  SMESH_Actor* FindActorByEntry(QAD_StudyFrame *theStudyFrame, 
				const char* theEntry)
  {
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(theStudyFrame)){
      vtkRenderer *aRenderer = aViewFrame->getRenderer();
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
    return FindActorByEntry(GetActiveStudy()->getActiveStudyFrame(),theEntry);
  }
  
  
  SMESH_Actor* FindActorByObject(CORBA::Object_ptr theObject){
    if(!CORBA::is_nil(theObject)){
      SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
      CORBA::String_var anIOR = aStudy->ConvertObjectToIOR(theObject);
      SALOMEDS::SObject_var aSObject = aStudy->FindObjectIOR(anIOR.in());
      if(!aSObject->_is_nil()){
	CORBA::String_var anEntry = aSObject->GetID();
	return FindActorByEntry(anEntry.in());
      }
    }
    return NULL;
  }


  SMESH_Actor* CreateActor(SALOMEDS::Study_ptr theStudy,
			   const char* theEntry,
			   int theIsClear)
  {
    SMESH_Actor *anActor = NULL;
    CORBA::Long anId = theStudy->StudyId();
    if(TVisualObjPtr aVisualObj = GetVisualObj(anId,theEntry)){
      SALOMEDS::SObject_var aSObj = theStudy->FindObjectID(theEntry);
      if(!aSObj->_is_nil()){
	SALOMEDS::GenericAttribute_var anAttr;
	if(aSObj->FindAttribute(anAttr,"AttributeName")){
	  SALOMEDS::AttributeName_var aName = SALOMEDS::AttributeName::_narrow(anAttr);
	  CORBA::String_var aVal = aName->Value();
	  string aNameVal = aVal.in();
	  anActor = SMESH_Actor::New(aVisualObj,theEntry,aNameVal.c_str(),theIsClear);
	}
      }
    }
    return anActor;
  }


  void DisplayActor(QAD_StudyFrame *theStudyFrame, SMESH_Actor* theActor){
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(theStudyFrame)){
      aViewFrame->AddActor(theActor);
      aViewFrame->Repaint();
    }
  }


  void RemoveActor(QAD_StudyFrame *theStudyFrame, SMESH_Actor* theActor){
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(theStudyFrame)){
      aViewFrame->RemoveActor(theActor);
      aViewFrame->Repaint();
    }
  }
  

  void FitAll(){
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(GetActiveStudy()->getActiveStudyFrame())){
      aViewFrame->onViewFitAll();
      aViewFrame->Repaint();
    }
  }
  
  vtkRenderer* GetCurrentRenderer(){
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(GetActiveStudy()->getActiveStudyFrame()))
      return aViewFrame->getRenderer();
    return NULL;
  }

  void RepaintCurrentView(){
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(GetActiveStudy()->getActiveStudyFrame())){
      aViewFrame->getRW()->getRenderWindow()->Render();
      //aViewFrame->Repaint();
    }
  }
  
  
  void UpdateView(QAD_StudyFrame *theStudyFrame, EDisplaing theAction, const char* theEntry)
  {
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(theStudyFrame)){
      vtkRenderer *aRenderer = aViewFrame->getRenderer();
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
	if(SMESH_Actor *anActor = FindActorByEntry(theStudyFrame,theEntry)){
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
	    QAD_Study* aStudy = theStudyFrame->getStudy();
	    SALOMEDS::Study_var aDocument = aStudy->getStudyDocument();
	    if((anActor = CreateActor(aDocument,theEntry,true))) {
	      DisplayActor(theStudyFrame,anActor);
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
    QAD_Study* aStudy = GetActiveStudy();
    QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
    UpdateView(aStudyFrame,theAction,theEntry);
  }
  
  void UpdateView(){
    if(VTKViewer_ViewFrame* aViewFrame = SMESH::GetCurrentVtkView()){
      SALOME_Selection *aSel = SALOME_Selection::Selection(GetActiveStudy()->getSelection());
      if(aSel->IObjectCount() == 0){
	vtkRenderer* aRenderer = aViewFrame->getRenderer();
	vtkActorCollection *aCollection = aRenderer->GetActors();
	aCollection->InitTraversal();
	while(vtkActor *anAct = aCollection->GetNextActor()){
	  if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	    if(anActor->hasIO())
	      Update(anActor->getIO(),anActor->GetVisibility());
	  }
	}
      }else{
	SALOME_ListIteratorOfListIO anIter(aSel->StoredIObjects());
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
    SALOMEDS::Study_var aStudy = GetActiveStudyDocument();
    CORBA::Long anId = aStudy->StudyId();
    TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,theIO->getEntry());
    aVisualObj->Update();
    if ( theDisplay )
      UpdateView(SMESH::eDisplay,theIO->getEntry());
  }


  void UpdateSelectionProp() {
    QAD_Study* aStudy = GetActiveStudy();
    QList<QAD_StudyFrame> aFrameList = aStudy->getStudyFrames();
    
    QString SCr, SCg, SCb;
    SCr = QAD_CONFIG->getSetting("SMESH:SettingsSelectColorRed");
    SCg = QAD_CONFIG->getSetting("SMESH:SettingsSelectColorGreen");
    SCb = QAD_CONFIG->getSetting("SMESH:SettingsSelectColorBlue");
    QColor aHiColor = Qt::white;
    if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
      aHiColor = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
    
    SCr = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectColorRed");
    SCg = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectColorGreen");
    SCb = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectColorBlue");
    QColor aSelColor = Qt::yellow;
    if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
      aSelColor = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
    QString SW = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectWidth");
    if (SW.isEmpty()) SW = "5";
    
    SCr = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectColorRed");
    SCg = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectColorGreen");
    SCb = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectColorBlue");
    QColor aPreColor = Qt::cyan;
    if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
      aPreColor = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
    QString PW = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectWidth");
    if (PW.isEmpty()) PW = "5";
    
    QString SP1 = QAD_CONFIG->getSetting("SMESH:SettingsNodeSelectTol");
    if (SP1.isEmpty()) SP1 = "0.025";
    QString SP2 = QAD_CONFIG->getSetting("SMESH:SettingsElementsSelectTol");
    if (SP2.isEmpty()) SP2 = "0.001";
    
    for (QAD_StudyFrame* aStudyFrame = aFrameList.first(); aStudyFrame; aStudyFrame = aFrameList.next()) {
      if (aStudyFrame->getTypeView() == VIEW_VTK) {
	VTKViewer_ViewFrame* aVtkViewFrame = GetVtkViewFrame(aStudyFrame);
	if (!aVtkViewFrame) continue;
	// update VTK viewer properties
	VTKViewer_RenderWindowInteractor* anInteractor = aVtkViewFrame->getRWInteractor();
	if (anInteractor) {
	  // mesh element selection
	  anInteractor->SetSelectionProp(aSelColor.red()/255., aSelColor.green()/255., 
					 aSelColor.blue()/255., SW.toInt());
	  
	  // tolerances
	  anInteractor->SetSelectionTolerance(SP1.toDouble(), SP2.toDouble());
	  
	  // pre-selection
	  VTKViewer_InteractorStyleSALOME* aStyle = anInteractor->GetInteractorStyleSALOME();
	  if (aStyle) {
	    aStyle->setPreselectionProp(aPreColor.red()/255., aPreColor.green()/255., 
					aPreColor.blue()/255., PW.toInt());
	  }
	}
	// update actors
	vtkRenderer* aRenderer = aVtkViewFrame->getRenderer();
	vtkActorCollection *aCollection = aRenderer->GetActors();
	aCollection->InitTraversal();
	while(vtkActor *anAct = aCollection->GetNextActor()){
	  if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	    anActor->SetHighlightColor(aHiColor.red()/255., aHiColor.green()/255., 
				       aHiColor.blue()/255.);
	    anActor->SetPreHighlightColor(aPreColor.red()/255., aPreColor.green()/255., 
					  aPreColor.blue()/255.);
	  }
	}
      }
    }
  }

  
  //----------------------------------------------------------------------------
  VTKViewer_InteractorStyleSALOME* GetInteractorStyle(QAD_StudyFrame *theStudyFrame){
    if(VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(theStudyFrame)){
      if(VTKViewer_RenderWindowInteractor* anInteractor = aViewFrame->getRWInteractor()){
	return anInteractor->GetInteractorStyleSALOME();
      }
    }
    return NULL;
  }

  void SetFilter(const Handle(VTKViewer_Filter)& theFilter,
		 VTKViewer_InteractorStyleSALOME* theStyle)
  {
    theStyle->SetFilter(theFilter);
  }

  Handle(VTKViewer_Filter) GetFilter(int theId, VTKViewer_InteractorStyleSALOME* theStyle)
  {
    return theStyle->GetFilter(theId);
  }

  bool IsFilterPresent(int theId, VTKViewer_InteractorStyleSALOME* theStyle)
  {
    return theStyle->IsFilterPresent(theId);
  }

  void RemoveFilter(int theId, VTKViewer_InteractorStyleSALOME* theStyle){
    theStyle->RemoveFilter(theId);
  }

  bool IsValid(SALOME_Actor* theActor, int theCellId,
	       VTKViewer_InteractorStyleSALOME* theStyle)
  {
    return theStyle->IsValid(theActor,theCellId);
  }


  //----------------------------------------------------------------------------
  void SetPointRepresentation(bool theIsVisible){
    if(VTKViewer_ViewFrame* aViewFrame = GetCurrentVtkView()){
      vtkRenderer *aRenderer = aViewFrame->getRenderer();
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
    if(VTKViewer_ViewFrame* aViewFrame = GetCurrentVtkView()){
      int anIsAllPickable = (theActor == NULL);
      vtkRenderer *aRenderer = aViewFrame->getRenderer();
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


  int GetNameOfSelectedNodes(SALOME_Selection *theSel, 
			     const Handle(SALOME_InteractiveObject)& theIO, 
			     QString& theName)
  {
    theName = "";
    if(theIO->hasEntry()){
      if(FindActorByEntry(theIO->getEntry())){
	TColStd_IndexedMapOfInteger aMapIndex;
	theSel->GetIndex(theIO,aMapIndex);
	for(int i = 1; i <= aMapIndex.Extent(); i++){
	  theName += QString(" %1").arg(aMapIndex(i));
	}
	return aMapIndex.Extent();
      }
    }
    return -1;
  }
  

  int GetNameOfSelectedNodes(SALOME_Selection *theSel, QString& theName){
    theName = "";
    if(theSel->IObjectCount() == 1){
      Handle(SALOME_InteractiveObject) anIO = theSel->firstIObject();
      return GetNameOfSelectedNodes(theSel,anIO,theName);
    }
    return -1;
  }
  

  int GetNameOfSelectedElements(SALOME_Selection *theSel, 
				const Handle(SALOME_InteractiveObject)& theIO, 
				QString& theName)
  {
    theName = "";
    if(theIO->hasEntry()){
      if(FindActorByEntry(theIO->getEntry())){
	TColStd_IndexedMapOfInteger aMapIndex;
	theSel->GetIndex(theIO,aMapIndex);
	typedef std::set<int> TIdContainer;
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


  int GetNameOfSelectedElements(SALOME_Selection *theSel, QString& theName)
  {
    theName = "";
    if(theSel->IObjectCount() == 1){
      Handle(SALOME_InteractiveObject) anIO = theSel->firstIObject();
      return GetNameOfSelectedElements(theSel,anIO,theName);
    }
    return -1;
  }

  
  int GetSelected(SALOME_Selection*            theSel, 
		  TColStd_IndexedMapOfInteger& theMap, 
		  const bool                   theIsElement)
  {
    theMap.Clear();

    if ( theSel->IObjectCount() == 1 )
    {
      Handle(SALOME_InteractiveObject) anIO = theSel->firstIObject();
      if ( anIO->hasEntry() ) {
	theSel->GetIndex( anIO, theMap );
      }
    }
    return theMap.Extent();
  }


  int GetEdgeNodes( SALOME_Selection* theSel, int& theId1, int& theId2 )
  {
    if ( theSel->IObjectCount() != 1 )
      return -1;

    Handle(SALOME_InteractiveObject) anIO = theSel->firstIObject();
    if ( anIO.IsNull() || !anIO->hasEntry() )
      return -1;

    SMESH_Actor *anActor = SMESH::FindActorByEntry( anIO->getEntry() );
    if ( anActor == 0 )
      return -1;

    TColStd_IndexedMapOfInteger aMapIndex;
    theSel->GetIndex( anIO, aMapIndex );
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
    if( VTKViewer_ViewFrame* aViewFrame = SMESH::GetCurrentVtkView() )
    {
      vtkRenderer *aRenderer = aViewFrame->getRenderer();
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

















