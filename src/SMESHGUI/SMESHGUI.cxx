//  SMESH SMESHGUI : GUI for SMESH component
//
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
//
//  File   : SMESHGUI.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI.h"
#include "SMESHGUI_AddHypothesisDlg.h"
#include "SMESHGUI_AddAlgorithmDlg.h"
#include "SMESHGUI_InitMeshDlg.h"
#include "SMESHGUI_AddSubMeshDlg.h"
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI_GroupDlg.h"
#include "SMESHGUI_RemoveNodesDlg.h"
#include "SMESHGUI_RemoveElementsDlg.h"
#include "SMESHGUI_MeshInfosDlg.h"
#include "SMESHGUI_Preferences_ColorDlg.h"
#include "SMESHGUI_Preferences_ScalarBarDlg.h"
#include "SMESHGUI_Preferences_SelectionDlg.h"
#include "SMESHGUI_aParameterDlg.h"
#include "SMESHGUI_ComputeScalarValue.h"
#include "SMESHGUI_MoveNodesDlg.h"
#include "SMESHGUI_OrientationElementsDlg.h"
#include "SMESHGUI_DiagonalInversionDlg.h"
#include "SMESHGUI_AddFaceDlg.h"
#include "SMESHGUI_AddEdgeDlg.h"
#include "SMESHGUI_AddVolumeDlg.h"
#include "SMESHGUI_EditHypothesesDlg.h"
#include "SMESHGUI_CreateHypothesesDlg.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_XmlHandler.h"

#include "SMESH_Actor.h"
#include "SMESH_Object.h"

// SALOME Includes
#include "Utils_ORB_INIT.hxx"
#include "Utils_SINGLETON.hxx"

#include "QAD.h"
#include "QAD_Tools.h"
#include "QAD_Config.h"
#include "QAD_Settings.h"
#include "QAD_RightFrame.h"
#include "QAD_MessageBox.h"
#include "QAD_Resource.h"
#include "QAD_FileDlg.h"
#include "QAD_Desktop.h"
#include "QAD_ResourceMgr.h"
#include "QAD_WaitCursor.h"

#include "OCCViewer_ViewPort.h"
#include "OCCViewer_ViewPort3d.h"
#include "OCCViewer_Viewer3d.h"

#include "GEOM_InteractiveObject.hxx"

#include "SALOME_NamingService.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_InteractiveObject.hxx"

#include "SALOMEGUI_Desktop.h"
#include "SALOMEGUI_NameDlg.h"
#include "SALOMEGUI_ImportOperation.h"
#include "SALOMEGUI_QtCatchCorbaException.hxx"

#include "utilities.h"
#include <dlfcn.h>

#include "SMDS_Mesh.hxx"
#include "SMESHDS_Document.hxx"
#include "Document_Reader.h"
#include "Document_Writer.h"
#include "Mesh_Reader.h"
#include "Mesh_Writer.h"

#include "DriverDAT_R_SMESHDS_Document.h"
#include "DriverMED_R_SMESHDS_Document.h"
#include "DriverUNV_R_SMESHDS_Document.h"
#include "DriverDAT_W_SMESHDS_Document.h"
#include "DriverMED_W_SMESHDS_Document.h"
#include "DriverUNV_W_SMESHDS_Document.h"
#include "DriverDAT_R_SMESHDS_Mesh.h"
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverUNV_R_SMESHDS_Mesh.h"
#include "DriverDAT_W_SMESHDS_Mesh.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverUNV_W_SMESHDS_Mesh.h"

// QT Includes
#define	 INCLUDE_MENUITEM_DEF
#include <qapplication.h>
#include <qlineedit.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfont.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qspinbox.h>
#include <qlist.h>
#include <qwidget.h>
#include <qevent.h>
#include <qradiobutton.h>

// VTK Includes
#include "VTKViewer_Common.h"
#include "VTKViewer_ViewFrame.h"
#include "VTKViewer_InteractorStyleSALOME.h"

#include <vtkScalarBarActor.h>
#include <vtkLegendBoxActor.h>
#include <vtkFeatureEdges.h>
#include <vtkDoubleArray.h>
#include <vtkTextProperty.h>

// Open CASCADE Includes
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <boost/shared_ptr.hpp>

static SMESHGUI *smeshGUI = 0;
static CORBA::ORB_var _orb;

using namespace std;

typedef pair<int,string> TKeyOfVisualObj;
typedef boost::shared_ptr<SMESH_VisualObj> TVisualObjPtr;
typedef map<TKeyOfVisualObj,TVisualObjPtr> TVisualObjCont;
static TVisualObjCont VISUALOBJCONT;


TVisualObjPtr GetVisualObj(int theStudyId, const char* theEntry){
  TVisualObjPtr aVisualObj;
  try{
    TVisualObjCont::key_type aKey(theStudyId,theEntry);
    TVisualObjCont::iterator anIter = VISUALOBJCONT.find(aKey);
    if(anIter != VISUALOBJCONT.end()){
      aVisualObj = anIter->second;
    }else{
      SALOMEDS::Study_var aStudy = QAD_Application::getDesktop()->getActiveStudy()->getStudyDocument();
      SALOMEDS::SObject_var aSObj = aStudy->FindObjectID(theEntry);
      if(!CORBA::is_nil(aSObj)){
	SALOMEDS::GenericAttribute_var anAttr;
	if(aSObj->FindAttribute(anAttr,"AttributeIOR")){
	  SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	  CORBA::String_var aVal = anIOR->Value();
	  CORBA::Object_var anObj =_orb->string_to_object(aVal.in());
	  if(!CORBA::is_nil(anObj)){
	    //Try narrow to SMESH_Mesh interafce
	    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow(anObj);
	    if(!aMesh->_is_nil()){
	      aVisualObj.reset(new SMESH_MeshObj(aMesh));
	      aVisualObj->Update();
	      TVisualObjCont::value_type aValue(aKey,aVisualObj); 
	      VISUALOBJCONT.insert(aValue);
	      return aVisualObj;
	    }
	    //Try narrow to SMESH_Group interafce
	    SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_narrow(anObj);
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
		VISUALOBJCONT.insert(aValue);
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
		VISUALOBJCONT.insert(aValue);
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


VTKViewer_ViewFrame* GetVtkViewFrame(QAD_StudyFrame* theStudyFrame){
  QAD_ViewFrame* aViewFrame = theStudyFrame->getRightFrame()->getViewFrame();
  return dynamic_cast<VTKViewer_ViewFrame*>(aViewFrame);
}

void UpdateSelectionProp() {
  QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
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

VTKViewer_ViewFrame* GetCurrentVtkView(){
  QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
  QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
  return GetVtkViewFrame(aStudyFrame);
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
  QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
  QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
  return FindActorByEntry(aStudyFrame,theEntry);
}


SMESH_Actor* CreateActor(SALOMEDS::Study_ptr theStudy,
			 const char* theEntry,
			 int theIsClear = false)
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
	SMESH::FilterManager_ptr aFilterMgr = smeshGUI->GetFilterMgr();
	anActor = SMESH_Actor::New(aVisualObj,aFilterMgr,theEntry,aNameVal.c_str(),theIsClear);
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
  if(VTKViewer_ViewFrame* aViewFrame = GetCurrentVtkView()){
    aViewFrame->onViewFitAll();
    aViewFrame->Repaint();
  }
}

vtkRenderer* GetCurrentRenderer(){
  if(VTKViewer_ViewFrame* aViewFrame = GetCurrentVtkView())
    return aViewFrame->getRenderer();
  return NULL;
}

void RepaintCurrentView(){
  if(VTKViewer_ViewFrame* aViewFrame = GetCurrentVtkView())
    aViewFrame->Repaint();
}


enum EDisplaing {eDisplayAll, eDisplay, eDisplayOnly, eErase, eEraseAll};
void UpdateView(QAD_StudyFrame *theStudyFrame, EDisplaing theAction,
		const char* theEntry = "")
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
	      if(anActor = CreateActor(aDocument,theEntry,true)) {
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


void UpdateView(EDisplaing theAction, const char* theEntry = ""){
  QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
  QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
  UpdateView(aStudyFrame,theAction,theEntry);
}

static bool IsReferencedObject( SALOMEDS::SObject_ptr SO ) 
{
  if ( !SO->_is_nil() ) {
    SALOMEDS::GenericAttribute_var anAttr;
    if ( SO->FindAttribute( anAttr, "AttributeTarget" ) ) {
      SALOMEDS::AttributeTarget_var aTarget = SALOMEDS::AttributeTarget::_narrow( anAttr );
      SALOMEDS::Study::ListOfSObject_var aList = aTarget->Get();
      if ( aList->length() > 0 ) {
	return true;
      }
    }
  }
  return false;
}

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


//=============================================================================
/*!
 *
 */
//=============================================================================
class CustomItem:public QCustomMenuItem
{
 public:
  CustomItem(const QString & s, const QFont & f):string(s), font(f)
  {
  };
  ~CustomItem()
  {
  }

  void paint(QPainter * p, const QColorGroup & /*cg */ , bool /*act */ ,
             bool /*enabled */ , int x, int y, int w, int h)
  {
    p->setFont(font);
    p->drawText(x, y, w, h,
                AlignHCenter | AlignVCenter | ShowPrefix | DontClip, string);
  }

  QSize sizeHint()
  {
    return QFontMetrics(font).
      size(AlignHCenter | AlignVCenter | ShowPrefix | DontClip, string);
  }
 private:
  QString string;
  QFont font;
};

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::SMESHGUI( const QString& theName, QObject* theParent ) :
  SALOMEGUI( theName, theParent )
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::~SMESHGUI()
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI* SMESHGUI::GetSMESHGUI()
{
  if ( !smeshGUI )
    GetOrCreateSMESHGUI( QAD_Application::getDesktop() );
  return smeshGUI;
}

extern "C"
{
  Standard_EXPORT SALOMEGUI* GetComponentGUI() {
    return SMESHGUI::GetSMESHGUI();
  }
}
//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI* SMESHGUI::GetOrCreateSMESHGUI( QAD_Desktop* desktop )
{
  if( !smeshGUI ) {
    setOrb();
    smeshGUI = new SMESHGUI;
    smeshGUI->myActiveDialogBox = 0 ;
    smeshGUI->myState = -1 ;
    smeshGUI->myDesktop = desktop ; 

    /* get smesh engine */
    Engines::Component_var comp = desktop->getEngine("FactoryServer", "SMESH");
    smeshGUI->myComponentMesh = SMESH::SMESH_Gen::_narrow(comp);
    
    /* get geom engine */
    Engines::Component_var comp1 = desktop->getEngine("FactoryServer", "GEOM");    
    smeshGUI->myComponentGeom = GEOM::GEOM_Gen::_narrow(comp1);
      
    /* set current study */
    smeshGUI->myActiveStudy = desktop->getActiveStudy();
    smeshGUI->myStudyId     = smeshGUI->myActiveStudy->getStudyId();
    smeshGUI->myStudy       = smeshGUI->myActiveStudy->getStudyDocument();
    smeshGUI->myComponentGeom->GetCurrentStudy( smeshGUI->myStudyId );
    smeshGUI->myComponentMesh->SetCurrentStudy( smeshGUI->myStudy );
    //    smeshGUI->myComponentGeom->NbLabels();
    
    /* create service object */
    smeshGUI->myStudyAPI = SMESHGUI_StudyAPI(smeshGUI->myStudy, smeshGUI->myComponentMesh);

    smeshGUI->myDocument = new SMESHDS_Document(1);//NBU

    smeshGUI->mySimulationActors = vtkActorCollection::New();
    smeshGUI->mySimulationActors2D = vtkActor2DCollection::New();

    /*filter manager*/
    smeshGUI->myFilterMgr = smeshGUI->myComponentMesh->CreateFilterManager();

    /* load resources for all available meshers */
    smeshGUI->InitAvailableHypotheses();
  } else {
    /* study may have changed - set current study */
    smeshGUI->myActiveStudy = desktop->getActiveStudy();
    smeshGUI->myStudyId     = smeshGUI->myActiveStudy->getStudyId();
    smeshGUI->myStudy       = smeshGUI->myActiveStudy->getStudyDocument();
    smeshGUI->myComponentGeom->GetCurrentStudy( smeshGUI->myStudyId );
    smeshGUI->myComponentMesh->SetCurrentStudy( smeshGUI->myStudy );
    
    // Set active study to Study API
    smeshGUI->myStudyAPI.Update( smeshGUI->myStudy );
  }
  
  /* Automatic Update flag */
  smeshGUI->myAutomaticUpdate = ( QAD_CONFIG->getSetting( "SMESH:AutomaticUpdate" ).compare( "true" ) == 0 );

  return smeshGUI;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetState(int aState)
{
  this->myState = aState;
  return;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ResetState()
{
  this->myState = -1;
  return;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalDeactivateDialog()
{
  emit this->SignalDeactivateActiveDialog();
  return;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalCloseAllDialogs()
{
  emit SignalCloseAllDialogs();
  return;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QDialog *SMESHGUI::GetActiveDialogBox()
{
  return this->myActiveDialogBox;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetActiveDialogBox(QDialog * aDlg)
{
  this->myActiveDialogBox = (QDialog *) aDlg;
  return;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QAD_Study *SMESHGUI::GetActiveStudy()
{
  return this->myActiveStudy;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SALOMEDS::Study_ptr SMESHGUI::GetStudy()
{
  return SALOMEDS::Study::_narrow(myStudy);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI_StudyAPI SMESHGUI::GetStudyAPI()
{
  return myStudyAPI;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QAD_Desktop *SMESHGUI::GetDesktop()
{
  return this->myDesktop;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::ActiveStudyChanged(QAD_Desktop* parent)
{
  MESSAGE("SMESHGUI::activeStudyChanged init.");
  QAD_Study* prevStudy = 0;
  if ( smeshGUI )
    prevStudy = smeshGUI->myActiveStudy;
  /* Create or retrieve an object SMESHGUI */
  MESSAGE("Active study changed : prev study =" << prevStudy);
  SMESHGUI::GetOrCreateSMESHGUI( parent );
  MESSAGE("Active study changed : active study =" << smeshGUI->myActiveStudy);
  if ( prevStudy != smeshGUI->myActiveStudy ) {
    smeshGUI->EmitSignalCloseAllDialogs();
    MESSAGE("Active study changed : SMESHGUI nullified" << endl);
    //smeshGUI = 0;
    ::UpdateSelectionProp();
  }
  MESSAGE("SMESHGUI::activeStudyChanged done.");
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::DefineDlgPosition(QWidget * aDlg, int &x, int &y)
{
  /* Here the position is on the bottom right corner - 10 */
  // aDlg->resize(QSize().expandedTo(aDlg->minimumSizeHint()));
  aDlg->adjustSize();
  QAD_Desktop *PP = QAD_Application::getDesktop();
  x = abs(PP->x() + PP->size().width() - aDlg->size().width() - 10);
  y = abs(PP->y() + PP->size().height() - aDlg->size().height() - 10);
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EraseSimulationActors()
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK)
  {							//VTK
    vtkRenderer *theRenderer =
      ((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
       getRightFrame()->getViewFrame())->getRenderer();
    vtkRenderWindow *renWin = theRenderer->GetRenderWindow();

    if (mySimulationActors != NULL)
    {
      mySimulationActors->InitTraversal();
      vtkActor *ac = mySimulationActors->GetNextActor();
      while (!(ac == NULL))
      {
        theRenderer->RemoveActor(ac);
        ac = mySimulationActors->GetNextActor();
      }
      mySimulationActors->RemoveAllItems();
    }
    if (mySimulationActors2D != NULL)
    {
      mySimulationActors2D->InitTraversal();
      vtkActor2D *ac = mySimulationActors2D->GetNextActor2D();
      while (!(ac == NULL))
      {
        theRenderer->RemoveActor2D(ac);
        ac = mySimulationActors2D->GetNextActor2D();
      }
      mySimulationActors2D->RemoveAllItems();
    }
    SetPointRepresentation(false);
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESHGUI::InitMesh( GEOM::GEOM_Shape_ptr aShape,
					  QString              NameMesh )
{
  try {
    if ( !myComponentMesh->_is_nil() && !aShape->_is_nil() ) {
      SMESH::SMESH_Mesh_var aMesh = myComponentMesh->CreateMesh( aShape );
      if ( !aMesh->_is_nil() ) {
	SALOMEDS::SObject_var SM = smeshGUI->myStudy->FindObjectIOR( _orb->object_to_string( aMesh ) );
	myStudyAPI.SetName( SM, NameMesh );
	return SMESH::SMESH_Mesh::_narrow(aMesh);
      }
    }
  }
  catch( const SALOME::SALOME_Exception& S_ex ) {
    QtCatchCorbaException( S_ex );
  }
  return SMESH::SMESH_Mesh::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_subMesh_ptr SMESHGUI::AddSubMesh( SMESH::SMESH_Mesh_ptr aMesh,
					       GEOM::GEOM_Shape_ptr  aShape,
					       QString               NameMesh )
{
  SMESH::SMESH_subMesh_var aSubMesh;
  try {
    if ( !aMesh->_is_nil() && !aShape->_is_nil() )
      // create sub mesh
      aSubMesh = SMESH::SMESH_subMesh::_duplicate( aMesh->GetSubMesh( aShape, NameMesh.latin1() ) ); //VSR: published automatically
  }
  catch( const SALOME::SALOME_Exception& S_ex ) {
    QtCatchCorbaException( S_ex );
    aSubMesh = SMESH::SMESH_subMesh::_nil();
  }
  return aSubMesh._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Group_ptr SMESHGUI::AddGroup( SMESH::SMESH_Mesh_ptr    aMesh,
					   SMESH::ElementType       aType,
					   QString                  aName )
{
  SMESH::SMESH_Group_var aGroup;
  try {
    if ( !aMesh->_is_nil() )
      // create group
      aGroup = SMESH::SMESH_Group::_duplicate( aMesh->CreateGroup( aType, strdup(aName) ) ); //VSR: published automatically
  }
  catch( const SALOME::SALOME_Exception& S_ex ) {
    QtCatchCorbaException( S_ex );
    aGroup = SMESH::SMESH_Group::_nil();
  }
  myActiveStudy->updateObjBrowser( true );
  return aGroup._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================

static void addMap( const map<string, HypothesisData*>& theMap,
                   map<string, HypothesisData*>&       toMap)
{
  map<string, HypothesisData*>::const_iterator it;
  for ( it = theMap.begin(); it != theMap.end(); it++ )
    toMap.insert( *it );
}

    
//=============================================================================
/*!
 *  InitAvailableHypotheses (read from resource XML file)
 */
//=============================================================================
void SMESHGUI::InitAvailableHypotheses ()
{
  QAD_WaitCursor wc;
  if ((myHypothesesMap.begin() == myHypothesesMap.end()) &&
      (myAlgorithmsMap.begin() == myAlgorithmsMap.end()))
  {
    // Resource manager
    QAD_ResourceMgr* resMgr = QAD_Desktop::createResourceManager();
    if (!resMgr) return;

    // Find name of a resource XML file ("SMESH_Meshers.xml");
    QString HypsXml;
    char* cenv = getenv("SMESH_MeshersList");
    if (cenv)
      HypsXml.sprintf("%s", cenv);

    QStringList HypsXmlList = QStringList::split( ":", HypsXml, false );
    if (HypsXmlList.count() == 0)
    {
      QAD_MessageBox::error1(QAD_Application::getDesktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("MESHERS_FILE_NO_VARIABLE"),
                             tr("SMESH_BUT_OK"));
      return;
    }

    // loop on files in HypsXml
    QString aNoAccessFiles;
    for ( int i = 0; i < HypsXmlList.count(); i++ ) {
      QString HypsXml = HypsXmlList[ i ];

      // Find full path to the resource XML file
      QString xmlFile = HypsXml + ".xml";
      xmlFile = QAD_Tools::addSlash(resMgr->findFile(xmlFile, HypsXml)) + xmlFile;

      QFile file (QAD_Tools::unix2win(xmlFile));
      if (file.exists() && file.open(IO_ReadOnly))
      {
        file.close();

        SMESHGUI_XmlHandler* myXmlHandler = new SMESHGUI_XmlHandler();
        ASSERT(myXmlHandler);

        QXmlInputSource source (file);
        QXmlSimpleReader reader;
        reader.setContentHandler(myXmlHandler);
        reader.setErrorHandler(myXmlHandler);
        bool ok = reader.parse(source);
        file.close();
        if (ok)
        {
          addMap( myXmlHandler->myHypothesesMap, myHypothesesMap );
          addMap( myXmlHandler->myAlgorithmsMap, myAlgorithmsMap );
        }
        else
        {
          QAD_MessageBox::error1(myDesktop, 
				 tr("INF_PARSE_ERROR"),
				 tr(myXmlHandler->errorProtocol()),
				 tr("SMESH_BUT_OK"));
        }
      }
      else
      {
        if (aNoAccessFiles.isEmpty())
          aNoAccessFiles = xmlFile;
        else
          aNoAccessFiles += ", " + xmlFile;
      }
    } // end loop


    if (!aNoAccessFiles.isEmpty())
    {
      QString aMess = tr("MESHERS_FILE_CANT_OPEN") + " " + aNoAccessFiles + "\n";
      aMess += tr("MESHERS_FILE_CHECK_VARIABLE");
      wc.stop();
      QAD_MessageBox::warn1(QAD_Application::getDesktop(),
                            tr("SMESH_WRN_WARNING"),
                            aMess,
                            tr("SMESH_BUT_OK"));
      wc.start();
    }
  }
}

//=============================================================================
/*!
 *  GetAvailableHypotheses (read from resource XML file)
 */
//=============================================================================
QStringList SMESHGUI::GetAvailableHypotheses (const bool isAlgo)
{
  QStringList aHypList;

  // Init list of available hypotheses, if needed
  InitAvailableHypotheses();

  // fill list of hypotheses/algorithms
  map<string, HypothesisData*>::iterator anIter;
  if (isAlgo)
  {
    anIter = myAlgorithmsMap.begin();
    for (; anIter != myAlgorithmsMap.end(); anIter++)
    {
      aHypList.append(((*anIter).first).c_str());
    }
  }
  else
  {
    anIter = myHypothesesMap.begin();
    for (; anIter != myHypothesesMap.end(); anIter++)
    {
      aHypList.append(((*anIter).first).c_str());
    }
  }

  return aHypList;
}

//=============================================================================
/*!
 *  GetHypothesisData
 */
//=============================================================================
HypothesisData* SMESHGUI::GetHypothesisData (const char* aHypType)
{
  HypothesisData* aHypData = 0;

  // Init list of available hypotheses, if needed
  InitAvailableHypotheses();

  if (myHypothesesMap.find(aHypType) == myHypothesesMap.end())
  {
    if (myAlgorithmsMap.find(aHypType) != myAlgorithmsMap.end())
    {
      aHypData = myAlgorithmsMap[aHypType];
    }
  }
  else
  {
    aHypData = myHypothesesMap[aHypType];
  }
  return aHypData;
}

//=============================================================================
/*! 
 *  Get a Hypothesis Creator from a Plugin Library
 */
//=============================================================================
SMESHGUI_GenericHypothesisCreator* SMESHGUI::GetHypothesisCreator(const QString& aHypType)
{
  char* sHypType = (char*)aHypType.latin1();
  MESSAGE("Get HypothesisCreator for " << sHypType);

  SMESHGUI_GenericHypothesisCreator* aCreator = 0;

  // check, if creator for this hypothesis type already exists
  if (myHypCreatorMap.find(sHypType) != myHypCreatorMap.end())
  {
    aCreator = myHypCreatorMap[sHypType];
  }
  else
  {
    // 1. Init list of available hypotheses, if needed
    InitAvailableHypotheses();

    // 2. Get names of plugin libraries
    HypothesisData* aHypData = GetHypothesisData(sHypType);
    if (!aHypData)
    {
      return aCreator;
    }
    QString aClientLibName = aHypData->ClientLibName;
    QString aServerLibName = aHypData->ServerLibName;

    // 3. Load Client Plugin Library
    try
    {
      // load plugin library
      MESSAGE("Loading client meshers plugin library ...");
      void* libHandle = dlopen (aClientLibName, RTLD_LAZY);
      if (!libHandle)
      {
        // report any error, if occured
        const char* anError = dlerror();
        MESSAGE(anError);
      }
      else
      {
        // get method, returning hypothesis creator
        MESSAGE("Find GetHypothesisCreator() method ...");
        typedef SMESHGUI_GenericHypothesisCreator* (*GetHypothesisCreator) \
          (QString aHypType, QString aServerLibName, SMESHGUI* aSMESHGUI);
        GetHypothesisCreator procHandle =
          (GetHypothesisCreator)dlsym( libHandle, "GetHypothesisCreator" );
        if (!procHandle)
        {
          MESSAGE("bad hypothesis client plugin library");
          dlclose(libHandle);
        }
        else
        {
          // get hypothesis creator
          MESSAGE("Get Hypothesis Creator for " << aHypType);
          aCreator = procHandle(aHypType, aServerLibName, smeshGUI);
          if (!aCreator)
          {
            MESSAGE("no such a hypothesis in this plugin");
          }
          else
          {
            // map hypothesis creator to a hypothesis name
            myHypCreatorMap[sHypType] = aCreator;
          }
        }
      }
    }
    catch (const SALOME::SALOME_Exception& S_ex)
    {
      QtCatchCorbaException(S_ex);
    }
  }

  return aCreator;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Hypothesis_ptr SMESHGUI::CreateHypothesis (const QString& aHypType,
							const QString& aHypName,
                                                        const bool     isAlgo)
{
  MESSAGE("Create " << aHypType << " with name " << aHypName);

  SMESH::SMESH_Hypothesis_var Hyp;

  HypothesisData* aHypData = GetHypothesisData((char*)aHypType.latin1());
  QString aServLib = aHypData->ServerLibName;

  try
  {
    Hyp = myComponentMesh->CreateHypothesis(aHypType, aServLib);
    if (!Hyp->_is_nil())
    {
      SALOMEDS::SObject_var SHyp =
        smeshGUI->myStudy->FindObjectIOR( _orb->object_to_string( Hyp ) );
      if (!SHyp->_is_nil())
      {
	if ( !aHypName.isEmpty() )
	  myStudyAPI.SetName( SHyp, aHypName );
        myActiveStudy->updateObjBrowser(true);
	return Hyp._retn();
      }
    }
  }
  catch (const SALOME::SALOME_Exception & S_ex)
  {
    QtCatchCorbaException(S_ex);
  }

  return SMESH::SMESH_Hypothesis::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::AddHypothesisOnMesh(SMESH::SMESH_Mesh_ptr aMesh,
				   SMESH::SMESH_Hypothesis_ptr aHyp)
{
  int res = SMESH::HYP_UNKNOWN_FATAL; 
  QAD_WaitCursor wc;

  if ( !aMesh->_is_nil() ) {
    SALOMEDS::SObject_var SM = myStudyAPI.FindObject( aMesh );
    GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh(SM);
    try {
      res = aMesh->AddHypothesis( aShape, aHyp );
      if ( res < SMESH::HYP_UNKNOWN_FATAL ) {
	SALOMEDS::SObject_var SH = myStudyAPI.FindObject(aHyp);
	if ( !SM->_is_nil() && !SH->_is_nil() ) {
	  //myStudyAPI.SetHypothesis(SM, SH); // VSR: published automatically by engine
	  myStudyAPI.ModifiedMesh(SM, false);
	}
      }
      if ( res >= SMESH::HYP_UNKNOWN_FATAL ) {
	wc.stop();
	QAD_MessageBox::error1(QAD_Application::getDesktop(),
			      tr("SMESH_ERROR"),
			      tr(QString("SMESH_HYP_%1").arg(res)),
			      tr("SMESH_BUT_OK"));
	wc.start();
      }
      else if ( res > SMESH::HYP_OK ) {
	wc.stop();
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"),
			      tr(QString("SMESH_HYP_%1").arg(res)),
			      tr("SMESH_BUT_OK"));
	wc.start();
      }
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      wc.stop();
      QtCatchCorbaException( S_ex );
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
  }
  return res < SMESH::HYP_UNKNOWN_FATAL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::RemoveHypothesisOrAlgorithmOnMesh
               (const Handle(SALOME_InteractiveObject)& IObject)
{
  int res = SMESH::HYP_UNKNOWN_FATAL;
  QAD_WaitCursor wc;

  if (IObject->hasReference())
  {
    try {
      SMESH::SMESH_Hypothesis_var anHyp;
      SALOMEDS::SObject_var SO_Hypothesis =
	smeshGUI->myStudy->FindObjectID(IObject->getEntry());
      SALOMEDS::GenericAttribute_var anAttr;
      SALOMEDS::AttributeIOR_var anIOR;

      if (!SO_Hypothesis->_is_nil()) {
	if (SO_Hypothesis->FindAttribute(anAttr, "AttributeIOR")) {
	  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	  anHyp = SMESH::SMESH_Hypothesis::_narrow
	    (_orb->string_to_object(anIOR->Value()));
	}

	SALOMEDS::SObject_var SO_Applied_Hypothesis =
	  smeshGUI->myStudy->FindObjectID(IObject->getReference());
	if (!SO_Applied_Hypothesis->_is_nil()) {
	  SALOMEDS::SObject_var MorSM =
	    smeshGUI->myStudyAPI.GetMeshOrSubmesh(SO_Applied_Hypothesis);
	  if (!MorSM->_is_nil()) {
	    GEOM::GEOM_Shape_var aShape =
	      smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh(MorSM);
	    if (!aShape->_is_nil() && MorSM->FindAttribute(anAttr, "AttributeIOR")) {
              anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
              SMESH::SMESH_Mesh_var aMesh =
                SMESH::SMESH_Mesh::_narrow(_orb->string_to_object(anIOR->Value()));
              SMESH::SMESH_subMesh_var aSubMesh =
                SMESH::SMESH_subMesh::_narrow(_orb->string_to_object(anIOR->Value()));

	      if (!aSubMesh->_is_nil())
                aMesh = aSubMesh->GetFather();

              if (!aMesh->_is_nil())
              {
                res = aMesh->RemoveHypothesis(aShape, anHyp);
		if ( res < SMESH::HYP_UNKNOWN_FATAL )
		  smeshGUI->myStudyAPI.ModifiedMesh(MorSM, false);
                if ( res > SMESH::HYP_OK ) {
		  wc.stop();
                  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
                                        tr("SMESH_WRN_WARNING"),
                                        tr(QString("SMESH_HYP_%1").arg(res)),
                                        tr("SMESH_BUT_OK"));
		  wc.start();
		}
              }
            }
          }
        }
      }
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      wc.stop();
      QtCatchCorbaException( S_ex );
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
  }
  else if (IObject->hasEntry())
  {
    MESSAGE("IObject entry " << IObject->getEntry());
  }
  return res < SMESH::HYP_UNKNOWN_FATAL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::RemoveHypothesisOrAlgorithmOnMesh(SALOMEDS::SObject_ptr MorSM,
	SMESH::SMESH_Hypothesis_ptr anHyp)
{
  SALOMEDS::SObject_var AHR, aRef;
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeIOR_var anIOR;
  int res = SMESH::HYP_UNKNOWN_FATAL;
  QAD_WaitCursor wc;

  if (!MorSM->_is_nil()) {
    try {
      GEOM::GEOM_Shape_var aShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh(MorSM);
      if (!aShape->_is_nil()) {
	if (MorSM->FindAttribute(anAttr, "AttributeIOR")) {
	  anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	  SMESH::SMESH_Mesh_var aMesh =
	    SMESH::SMESH_Mesh::_narrow(_orb->string_to_object(anIOR->Value()));
	  SMESH::SMESH_subMesh_var aSubMesh =
	    SMESH::SMESH_subMesh::_narrow(_orb->string_to_object(anIOR->Value()));
	  
	  if ( !aSubMesh->_is_nil() )
	    aMesh = aSubMesh->GetFather();
	  
	  if (!aMesh->_is_nil()) {
	    res = aMesh->RemoveHypothesis(aShape, anHyp);
	    if ( res < SMESH::HYP_UNKNOWN_FATAL )
	      smeshGUI->myStudyAPI.ModifiedMesh(MorSM, false);
	    if ( res > SMESH::HYP_OK ) {
	      wc.stop();
	      QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				    tr("SMESH_WRN_WARNING"),
				    tr(QString("SMESH_HYP_%1").arg(res)),
				    tr("SMESH_BUT_OK"));
	      wc.start();
	    }
	  }
	}
      }
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      wc.stop();
      QtCatchCorbaException( S_ex );
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
  }
  return res < SMESH::HYP_UNKNOWN_FATAL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::AddAlgorithmOnMesh( SMESH::SMESH_Mesh_ptr       aMesh,
				   SMESH::SMESH_Hypothesis_ptr aHyp )
{
  int res = SMESH::HYP_UNKNOWN_FATAL;
  QAD_WaitCursor wc;

  if ( !aMesh->_is_nil() ) {
    SALOMEDS::SObject_var SM = myStudyAPI.FindObject( aMesh );
    GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh( SM );
    try {
      res = aMesh->AddHypothesis( aShape, aHyp );
      if ( res < SMESH::HYP_UNKNOWN_FATAL ) {
	//SALOMEDS::SObject_var SH = myStudyAPI.FindObject( aHyp );
	//if ( !SM->_is_nil() && !SH->_is_nil() ) {
	  //myStudyAPI.SetAlgorithms(SM, SH);
	myStudyAPI.ModifiedMesh( SM, false );
	//}
      }
      if ( res >= SMESH::HYP_UNKNOWN_FATAL ) {
	wc.stop();
	QAD_MessageBox::error1(QAD_Application::getDesktop(),
			      tr("SMESH_ERROR"),
			      tr(QString("SMESH_HYP_%1").arg(res)),
			      tr("SMESH_BUT_OK"));
	wc.start();
      }
      else if ( res > SMESH::HYP_OK ) {
	wc.stop();
	QAD_MessageBox::warn1( QAD_Application::getDesktop(),
			       tr( "SMESH_WRN_WARNING" ),
			       tr(QString("SMESH_HYP_%1").arg(res)),
			       tr( "SMESH_BUT_OK" ) );
	wc.start();
      }
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      wc.stop();
      QtCatchCorbaException( S_ex );
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
  }
  return res < SMESH::HYP_UNKNOWN_FATAL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::AddHypothesisOnSubMesh( SMESH::SMESH_subMesh_ptr    aSubMesh,
				       SMESH::SMESH_Hypothesis_ptr aHyp )
{
  int res = SMESH::HYP_UNKNOWN_FATAL;
  QAD_WaitCursor wc;

  if ( !aSubMesh->_is_nil() ) {
    try {
      SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
      SALOMEDS::SObject_var SsubM = myStudyAPI.FindObject( aSubMesh );
      GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh( SsubM );
      if ( !aMesh->_is_nil() && !SsubM->_is_nil() && !aShape->_is_nil() ) {
	res = aMesh->AddHypothesis( aShape, aHyp );
	if ( res < SMESH::HYP_UNKNOWN_FATAL )  {
	  //SALOMEDS::SObject_var SH = myStudyAPI.FindObject(aHyp);
	  //			if (!SsubM->_is_nil() && !SH->_is_nil())
	  //			{
	  //				myStudyAPI.SetHypothesis(SsubM, SH);
	  myStudyAPI.ModifiedMesh( SsubM, false );
	  //			}
	}
	if ( res >= SMESH::HYP_UNKNOWN_FATAL ) {
	  wc.stop();
	  QAD_MessageBox::error1(QAD_Application::getDesktop(),
				 tr("SMESH_ERROR"),
				 tr(QString("SMESH_HYP_%1").arg(res)),
				 tr("SMESH_BUT_OK"));
	  wc.start();
	}
	else if ( res > SMESH::HYP_OK ) {
	  wc.stop();
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				tr( "SMESH_WRN_WARNING" ),
				tr(QString("SMESH_HYP_%1").arg(res)),
				tr( "SMESH_BUT_OK" ) );
	  wc.start();
	}
      }
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      wc.stop();
      QtCatchCorbaException( S_ex );
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
  }
  return res < SMESH::HYP_UNKNOWN_FATAL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::AddAlgorithmOnSubMesh( SMESH::SMESH_subMesh_ptr    aSubMesh,
 				      SMESH::SMESH_Hypothesis_ptr aHyp )
{
  int res = SMESH::HYP_UNKNOWN_FATAL;
  QAD_WaitCursor wc;

  if ( !aSubMesh->_is_nil() ) {
    try {
      SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
      SALOMEDS::SObject_var SsubM = myStudyAPI.FindObject(aSubMesh);
      GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh(SsubM);
      if ( !aMesh->_is_nil() && !SsubM->_is_nil() && !aShape->_is_nil() ) {
	res = aMesh->AddHypothesis( aShape, aHyp );
	if ( res < SMESH::HYP_UNKNOWN_FATAL )  {
	  //SALOMEDS::SObject_var SH = myStudyAPI.FindObject(aHyp);
	  //if (!SsubM->_is_nil() && !SH->_is_nil()) {
	  //myStudyAPI.SetAlgorithms(SsubM, SH);
	  myStudyAPI.ModifiedMesh( SsubM, false );
	  //}
	}
	if ( res >= SMESH::HYP_UNKNOWN_FATAL ) {
	  wc.stop();
	  QAD_MessageBox::error1(QAD_Application::getDesktop(),
				 tr("SMESH_ERROR"),
				 tr(QString("SMESH_HYP_%1").arg(res)),
				 tr("SMESH_BUT_OK"));
	  wc.start();
	}
	else if ( res > SMESH::HYP_OK ) {
	  wc.stop();
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				tr( "SMESH_WRN_WARNING" ),
				tr(QString("SMESH_HYP_%1").arg(res)),
				tr( "SMESH_BUT_OK" ) );
	  wc.start();
	}
      }
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      wc.stop();
      QtCatchCorbaException( S_ex );
      res = SMESH::HYP_UNKNOWN_FATAL;
    }
  }
  return res < SMESH::HYP_UNKNOWN_FATAL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedIObjects(SALOME_Selection * Sel, QString & aName)
{
	int nbSel = Sel->IObjectCount();
	if (nbSel == 1)
	{
		Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
		aName = IObject->getName();
	}
	else
	{
		aName = tr("SMESH_OBJECTS_SELECTED").arg(nbSel);
	}
	return nbSel;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedNodes(SALOME_Selection * Sel, QString & aName){
  aName = "";
  if(Sel->IObjectCount() == 1){
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    if(IObject->hasEntry()){
      if(SMESH_Actor *anActor = ::FindActorByEntry(IObject->getEntry())){
	TColStd_MapOfInteger MapIndex;
	Sel->GetIndex(IObject, MapIndex);
	TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
	for(; ite.More(); ite.Next()){
	  aName += QString(" %1").arg(anActor->GetNodeObjId(ite.Key()));
	}
	return MapIndex.Extent();
      }
    }
  }
  return -1;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedElements(SALOME_Selection * Sel, QString & aName){
  aName = "";
  if(Sel->IObjectCount() == 1){
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    if(IObject->hasEntry()){
      if(SMESH_Actor *anActor = ::FindActorByEntry(IObject->getEntry())){
	TColStd_MapOfInteger MapIndex;
	Sel->GetIndex(IObject, MapIndex);
	TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
	std::vector<int> aList;
	for(; ite.More(); ite.Next()){
	  int aVtkId = ite.Key();
	  int anObjId = anActor->GetElemObjId(aVtkId);
	  std::vector<int>::iterator found = find(aList.begin(), aList.end(), anObjId);
	  //	  MESSAGE("GetNameOfSelectedElements(): VTK Id = " << aVtkId << ", OBJECT Id = " << anObjId);
	  if (found == aList.end()) {
	    aList.push_back(anObjId);
	    aName += QString(" %1").arg(anObjId);
	  }
	}
	return aList.size();
      }
    }
  }
  return -1;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedEdges(SALOME_Selection * Sel, QString & aName){
  aName = "";
  if(Sel->IObjectCount() == 1){
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    if(IObject->hasEntry()){
      if(SMESH_Actor *anActor = ::FindActorByEntry(IObject->getEntry())){
	TColStd_MapOfInteger MapIndex;
	Sel->GetIndex(IObject, MapIndex);
	TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
	for(; ite.More(); ite.Next()){
	  aName += QString(" %1").arg(ite.Key());
	}
	return MapIndex.Extent();
      }
    }
  }
  return -1;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Actor *SMESHGUI::FindActorByEntry(QString theEntry,
					Standard_Boolean & theResult, 
					bool onlyInActiveView)
{
  theResult = false;
  int aNbStudyFrames = myActiveStudy->getStudyFramesCount();
  for (int i = 0; i < aNbStudyFrames; i++){
    QAD_StudyFrame *aStudyFrame = myActiveStudy->getStudyFrame(i);
    if(SMESH_Actor *anActor = ::FindActorByEntry(aStudyFrame,theEntry.latin1())){
      theResult = true;
      return anActor;
    }
  }
  return NULL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Actor* SMESHGUI::FindActor(CORBA::Object_ptr theObj,
				 Standard_Boolean & theResult, 
				 bool theOnlyInActiveView)
{
  theResult = false;
  SMESH_Actor* aRes = NULL;
  if ( !CORBA::is_nil( theObj ) ) {
    SALOMEDS::SObject_var aSObject = SALOMEDS::SObject::_narrow( myStudy->FindObjectIOR( _orb->object_to_string( theObj ) ) );
    if( !aSObject->_is_nil()) {
      CORBA::String_var anEntry = aSObject->GetID();
      aRes = FindActorByEntry( anEntry.in(), theResult, theOnlyInActiveView );
    }
  }
  return aRes;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESHGUI::ConvertIOinMesh(const Handle(SALOME_InteractiveObject) & IO,
						Standard_Boolean & testResult)
{
  SMESH::SMESH_Mesh_var aMesh;
  testResult = false;

  /* case SObject */
  if (IO->hasEntry())
  {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID(IO->getEntry());
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    if (!obj->_is_nil())
    {
      if (obj->FindAttribute(anAttr, "AttributeIOR"))
      {
        anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
        aMesh =
          SMESH::SMESH_Mesh::_narrow(_orb->string_to_object(anIOR->Value()));
        if (!aMesh->_is_nil())
        {
          testResult = true;
          return SMESH::SMESH_Mesh::_duplicate(aMesh);
        }
      }
    }
  }
  return SMESH::SMESH_Mesh::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_subMesh_ptr SMESHGUI::ConvertIOinSubMesh(const Handle(SALOME_InteractiveObject) & IO,
						      Standard_Boolean & testResult)
{
  SMESH::SMESH_subMesh_var aSubMesh;
  testResult = false;

  /* case SObject */
  if (IO->hasEntry())
  {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID(IO->getEntry());
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    if (!obj->_is_nil())
    {
      if (obj->FindAttribute(anAttr, "AttributeIOR"))
      {
        anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
        aSubMesh = SMESH::SMESH_subMesh::_narrow(_orb->string_to_object(anIOR->Value()));
        if (!aSubMesh->_is_nil())
        {
          testResult = true;
          return SMESH::SMESH_subMesh::_duplicate(aSubMesh);
        }
      }
    }
  }
  return SMESH::SMESH_subMesh::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Hypothesis_ptr SMESHGUI::ConvertIOinSMESHHypothesis(const Handle(SALOME_InteractiveObject) & IO,
								 Standard_Boolean & testResult)
{
  SMESH::SMESH_Hypothesis_var aHyp;
  testResult = false;

  /* case SObject */
  if (IO->hasEntry())
  {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID(IO->getEntry());
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    if (!obj->_is_nil())
    {
      if (obj->FindAttribute(anAttr, "AttributeIOR"))
      {
        anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
        aHyp = SMESH::SMESH_Hypothesis::_narrow(_orb->string_to_object(anIOR->Value()));
        if (!aHyp->_is_nil())
        {
          testResult = true;
          return SMESH::SMESH_Hypothesis::_duplicate(aHyp);
        }
      }
    }
  }
  return SMESH::SMESH_Hypothesis::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Group_ptr SMESHGUI::ConvertIOinSMESHGroup(const Handle(SALOME_InteractiveObject) & IO,
						       Standard_Boolean & testResult)
{
  SMESH::SMESH_Group_var aGroup;
  testResult = false;

  /* case SObject */
  if (IO->hasEntry()) {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID(IO->getEntry());
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    if (!obj->_is_nil()) {
      if (obj->FindAttribute(anAttr, "AttributeIOR")) {
        anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
        aGroup = SMESH::SMESH_Group::_narrow(_orb->string_to_object(anIOR->Value()));
        if (!aGroup->_is_nil()) {
          testResult = true;
          return SMESH::SMESH_Group::_duplicate(aGroup);
        }
      }
    }
  }
  return SMESH::SMESH_Group::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
GEOM::GEOM_Shape_ptr SMESHGUI::ConvertIOinGEOMShape(const Handle(SALOME_InteractiveObject) & IO,
						    Standard_Boolean & testResult)
{
  GEOM::GEOM_Shape_var aShape;
  testResult = false;

  /* case SObject */
  if (IO->hasEntry()) {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID(IO->getEntry());
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    if (!obj->_is_nil()) {
      if (obj->FindAttribute(anAttr, "AttributeIOR")) {
	anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	aShape = myComponentGeom->GetIORFromString(anIOR->Value());
	if (!aShape->_is_nil())	{
	  testResult = true;
	  return GEOM::GEOM_Shape::_duplicate(aShape);
	}
      }
    }
  }
  return GEOM::GEOM_Shape::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetViewMode(int commandId){
  SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
  if(Sel->IObjectCount() >= 1){
    SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
    for(; It.More(); It.Next()){
      Handle(SALOME_InteractiveObject) IObject = It.Value();
      if(IObject->hasEntry()){
	if(SMESH_Actor *anActor = ::FindActorByEntry(IObject->getEntry())){
	  switch(commandId){
	  case 211:
	    ChangeRepresentation(anActor,0);
	    break;
	  case 212:
	    ChangeRepresentation(anActor,1);
	    break;
	  case 213:
	    ChangeRepresentation(anActor,2);
	    break;
	  case 215:
	    ChangeRepresentation(anActor,5);
	    break;
	  case 1132:
	    ChangeRepresentation(anActor,3);
	    break;
	  }
	}
      }
    }
    if(commandId == 1133)
      ChangeRepresentation(NULL,4);
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ChangeRepresentation(SMESH_Actor * theActor, int type){
  switch (type){
  case 0:{
    //theActor->UnShrink();
    theActor->SetRepresentation(SMESH_Actor::eEdge);
    break;
  }
  case 1:{
    //theActor->UnShrink();
    theActor->SetRepresentation(SMESH_Actor::eSurface);
    break;
  }
  case 2:{
    theActor->IsShrunk() ? theActor->UnShrink() : theActor->SetShrink();
    //theActor->SetRepresentation(SMESH_Actor::eSurface);
    break;
  }
  case 5:{
    //theActor->UnShrink();
    theActor->SetRepresentation(SMESH_Actor::ePoint);
    break;
  }
  case 3:{
    float color[3];
    theActor->GetSufaceColor(color[0], color[1], color[2]);
    int c0 = int (color[0] * 255);
    int c1 = int (color[1] * 255);
    int c2 = int (color[2] * 255);
    QColor c(c0, c1, c2);

    float edgecolor[3];
    theActor->GetEdgeColor(edgecolor[0], edgecolor[1], edgecolor[2]);
    c0 = int (edgecolor[0] * 255);
    c1 = int (edgecolor[1] * 255);
    c2 = int (edgecolor[2] * 255);
    QColor e(c0, c1, c2);

    float backfacecolor[3];
    theActor->GetBackSufaceColor(backfacecolor[0], backfacecolor[1], backfacecolor[2]);
    c0 = int (backfacecolor[0] * 255);
    c1 = int (backfacecolor[1] * 255);
    c2 = int (backfacecolor[2] * 255);
    QColor b(c0, c1, c2);

    float nodecolor[3];
    theActor->GetNodeColor(nodecolor[0], nodecolor[1], nodecolor[2]);
    c0 = int (nodecolor[0] * 255);
    c1 = int (nodecolor[1] * 255);
    c2 = int (nodecolor[2] * 255);
    QColor n(c0, c1, c2);

    int Edgewidth = (int)theActor->GetLineWidth();
    if(Edgewidth == 0)
      Edgewidth = 1;
    int intValue = int(theActor->GetNodeSize());
    float Shrink = theActor->GetShrinkFactor();

    SMESHGUI_Preferences_ColorDlg *aDlg = new SMESHGUI_Preferences_ColorDlg(QAD_Application::getDesktop(),"");
    aDlg->SetColor(1, c);
    aDlg->SetColor(2, e);
    aDlg->SetColor(3, n);
    aDlg->SetColor(4, b);
    aDlg->SetIntValue(1, Edgewidth);
    aDlg->SetIntValue(2, intValue);
    aDlg->SetIntValue(3, int(Shrink*100.));
    if(aDlg->exec()){
      QColor color = aDlg->GetColor(1);
      QColor edgecolor = aDlg->GetColor(2);
      QColor nodecolor = aDlg->GetColor(3);
      QColor backfacecolor = aDlg->GetColor(4);
      /* actor color and backface color */
      theActor->SetSufaceColor(float (color.red()) / 255.,
			       float (color.green()) / 255., 
			       float (color.blue()) / 255.);
      theActor->SetBackSufaceColor(float (backfacecolor.red()) / 255., 
				   float (backfacecolor.green()) / 255.,
				   float (backfacecolor.blue()) / 255.);

      /* edge color */
      theActor->SetEdgeColor(float (edgecolor.red()) / 255.,
			     float (edgecolor.green()) / 255.,
			     float (edgecolor.blue()) / 255.);
      
      /* Shrink factor and size edges */
      theActor->SetShrinkFactor(aDlg->GetIntValue(3) / 100.);
      theActor->SetLineWidth(aDlg->GetIntValue(1));
      
      /* Nodes color and size */
      theActor->SetNodeColor(float (nodecolor.red()) / 255.,
			     float (nodecolor.green()) / 255.,
			     float (nodecolor.blue()) / 255.);
      theActor->SetNodeSize(aDlg->GetIntValue(2));

      delete aDlg;
    }
    break;
  }
  case 4:{
    EmitSignalDeactivateDialog();
    SMESHGUI_TransparencyDlg *aDlg = new SMESHGUI_TransparencyDlg(QAD_Application::getDesktop(),"",false);
    break;
  }
  }
  UpdateView();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::UpdateView(){
  ::RepaintCurrentView();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayActor(SMESH_Actor * theActor, bool theVisibility){
  theActor->SetVisibility(theVisibility);
  ::DisplayActor(myActiveStudy->getActiveStudyFrame(),theActor);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EraseActor(SMESH_Actor * theActor){
  theActor->SetVisibility(false);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::AddActorInSelection(SMESH_Actor * ac)
{
	SALOME_Selection *Sel =
		SALOME_Selection::Selection(myActiveStudy->getSelection());
	Sel->ClearIObjects();
	if (ac->hasIO())
		return Sel->AddIObject(ac->getIO());
	else
		return -1;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QString SMESHGUI::CheckHomogeneousSelection()
{
	SALOME_Selection *Sel =
		SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	/* copy the list */
	SALOME_ListIO List;
	SALOME_ListIteratorOfListIO Itinit(Sel->StoredIObjects());
	for (; Itinit.More(); Itinit.Next())
	{
		List.Append(Itinit.Value());
	}

	QString RefType = CheckTypeObject(Sel->firstIObject());
	SALOME_ListIteratorOfListIO It(List);
	for (; It.More(); It.Next())
	{
		Handle(SALOME_InteractiveObject) IObject = It.Value();
		QString Type = CheckTypeObject(IObject);
		if (Type.compare(RefType) != 0)
			return "Heterogeneous Selection";
	}

	Sel->ClearIObjects();
	SALOME_ListIteratorOfListIO It1(List);
	for (; It1.More(); It1.Next())
	{
		int res = Sel->AddIObject(It1.Value());
		if (res == -1)
			myActiveStudy->highlight(It1.Value(), false);
		if (res == 0)
			myActiveStudy->highlight(It1.Value(), true);
	}
	return RefType;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QString SMESHGUI::CheckTypeObject(const Handle(SALOME_InteractiveObject) & IO)
{
  SALOMEDS::SObject_var sobj = smeshGUI->myActiveStudy->getStudyDocument()->FindObjectID(IO->getEntry());
  if (!sobj->_is_nil())	{
    SALOMEDS::SComponent_var scomp = sobj->GetFatherComponent();
    if (strcmp(scomp->GetID(), IO->getEntry()) == 0)
      {						// component is selected
	return "Component";
      }
  }

  SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
  Sel->ClearIObjects();

  Handle(SMESH_TypeFilter) aHypFilter = new SMESH_TypeFilter(HYPOTHESIS);
  Handle(SMESH_TypeFilter) anAlgoFilter = new SMESH_TypeFilter(ALGORITHM);
  Handle(SMESH_TypeFilter) aMeshFilter = new SMESH_TypeFilter(MESH);
  Handle(SMESH_TypeFilter) aSubMeshFilter = new SMESH_TypeFilter(SUBMESH);
  Handle(SMESH_TypeFilter) aGroupFilter = new SMESH_TypeFilter(GROUP);
  Handle(SMESH_TypeFilter) aMeshOrSubMeshFilter = new SMESH_TypeFilter(MESHorSUBMESH);
  Handle(SMESH_TypeFilter) aSubMeshVextexFilter = new SMESH_TypeFilter(SUBMESH_VERTEX);
  Handle(SMESH_TypeFilter) aSubMeshEdgeFilter = new SMESH_TypeFilter(SUBMESH_EDGE);
  Handle(SMESH_TypeFilter) aSubMeshFaceFilter =	new SMESH_TypeFilter(SUBMESH_FACE);
  Handle(SMESH_TypeFilter) aSubMeshSolidFilter = new SMESH_TypeFilter(SUBMESH_SOLID);
  Handle(SMESH_TypeFilter) aSubMeshCompoundFilter = new SMESH_TypeFilter(SUBMESH_COMPOUND);

  Sel->AddFilter(aHypFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "Hypothesis";
  }

  Sel->ClearFilters();
  Sel->AddFilter(anAlgoFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "Algorithm";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aMeshFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "Mesh";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "SubMesh";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aGroupFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "Group";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshVextexFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "SubMeshVertex";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshEdgeFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "SubMeshEdge";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshFaceFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "SubMeshFace";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshSolidFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "SubMeshSolid";
  }

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshCompoundFilter);
  if (Sel->AddIObject(IO) != -1) {
    Sel->ClearFilters();
    return "SubMeshCompound";
  }

  Sel->ClearFilters();
  Sel->AddIObject(IO);
  return "NoType";
}


static int isStudyLocked(const SALOMEDS::Study_var& theStudy){
  return theStudy->GetProperties()->IsLocked();
}

static int checkLock(const SALOMEDS::Study_var& theStudy) {
  if (isStudyLocked(theStudy)) {
    QAD_MessageBox::warn1 ( (QWidget*)QAD_Application::getDesktop(),
			   QObject::tr("WRN_WARNING"), 
			   QObject::tr("WRN_STUDY_LOCKED"),
			   QObject::tr("BUT_OK") );
    return true;
  }
  return false;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnGUIEvent(int theCommandID, QAD_Desktop * parent)
{
  SALOMEDS::Study_var aStudy = smeshGUI->myActiveStudy->getStudyDocument(); //Document OCAF de l'etude active
  //  QAD_Viewer3d* v3d;
  OCCViewer_Viewer3d *v3d;

  Handle(AIS_InteractiveContext) ic;
  vtkRenderer *Renderer;
  vtkRenderWindow *RenWin;

  if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_OCC) {
    v3d = ((OCCViewer_ViewFrame *) smeshGUI->myActiveStudy->
	   getActiveStudyFrame()->getRightFrame()->getViewFrame())->getViewer();
    ic = v3d->getAISContext();
  }
  else if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
    Renderer = ((VTKViewer_ViewFrame *) smeshGUI->myActiveStudy->
		getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
    RenWin = Renderer->GetRenderWindow();
  }

  switch (theCommandID)	{
  case 33:					// DELETE
    if(checkLock(aStudy)) break;
    smeshGUI->OnEditDelete();
    break;
    
  case 113:					// IMPORT
  case 112:
  case 111:
    {
      if(checkLock(aStudy)) break;
      //Import_Document(parent, theCommandID);	//NBU
      Import_Mesh(parent,theCommandID);
      break;
    }

  case 122:					// EXPORT MED
  case 121:
  case 123:
    {
      Export_Mesh(parent, theCommandID);
      break;
    }

  case 200:					// SCALAR BAR
    {
      SALOME_Selection *Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
      if( Sel && Sel->IObjectCount() ) {
	Handle(SALOME_InteractiveObject) anIO = Sel->firstIObject();
	if( anIO->hasEntry() ) {
	  if( SMESH_Actor* anActor = ::FindActorByEntry( anIO->getEntry() ) ) {
	    anActor->SetControlMode( SMESH_Actor::eNone );
	  }
	}
      }
      break;
    }
  case 201:
    {
      SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
      SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties( parent, Sel );
      break;
    }

  case 1133:					// DISPLAY MODE : WireFrame, Surface, Shrink
  case 1132:
  case 215:
  case 213:
  case 212:
  case 211:
    {
      smeshGUI->SetViewMode(theCommandID);
      break;
    }

  case 214:					// UPDATE
    {
      if(checkLock(aStudy)) break;
      smeshGUI->Update();
      break;
    }

  case 300:					// ERASE
  case 301:					// DISPLAY
  case 302:					// DISPLAY ONLY
    {
      EDisplaing anAction;
      switch(theCommandID){
      case 300:	anAction = eErase; break;
      case 301:	anAction = eDisplay; break;
      case 302:	anAction = eDisplayOnly; break;
      }
      
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
	for (; It.More(); It.Next()) {
	  Handle(SALOME_InteractiveObject) IOS = It.Value();
	  if (IOS->hasEntry()) {
	    ::UpdateView(anAction,IOS->getEntry());
	  }
	}
      }
      Sel->ClearIObjects();
      break;
    }

  case 400:					// NODES
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_NodesDlg *aDlg = new SMESHGUI_NodesDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), 
			      tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }

  case 2151:  // FILTER
  {
    if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK )
    {
      smeshGUI->EmitSignalDeactivateDialog();
      SMESHGUI_FilterDlg *aDlg = new SMESHGUI_FilterDlg( parent, SMESH::EDGE );
    }
    break;
  }
    
  case 405:					// MOVE NODE
    {
      if(checkLock(aStudy)) break;
      smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SMESHGUI_MoveNodesDlg *aDlg = new SMESHGUI_MoveNodesDlg(parent, "", Sel);
      break;
    }
    
  case 701:					// COMPUTE MESH 
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	int nbSel = Sel->IObjectCount();
	if (nbSel != 1){
	  break;
	}

	SMESH::SMESH_Mesh_var aMesh;
	SMESH::SMESH_subMesh_var aSubMesh;
	Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	if (IObject->hasEntry()){
	  SALOMEDS::SObject_var aMeshSObj = smeshGUI->myStudy->FindObjectID(IObject->getEntry());
	  GEOM::GEOM_Shape_var aShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh( aMeshSObj );
	  if ( aShape->_is_nil() ) {
	    // imported mesh
	    break;
	  }
	  SALOMEDS::GenericAttribute_var anAttr;
	  if(!aMeshSObj->_is_nil() && aMeshSObj->FindAttribute(anAttr, "AttributeIOR")){
	    SALOMEDS::AttributeIOR_var anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    MESSAGE("SMESHGUI::OnGUIEvent - Compute mesh : IOR = "<< anIOR->Value());
	    CORBA::Object_var anObj;
	    try{
	      anObj = _orb->string_to_object(anIOR->Value());
	      if (CORBA::is_nil(anObj)){
		MESSAGE("SMESHGUI::OnGUIEvent - Compute mesh : nil object");
	      }
	    }catch(CORBA::COMM_FAILURE & ex){
	      INFOS("SMESHGUI::OnGUIEvent - Compute mesh : exception (1)");
	    }
	    aMesh = SMESH::SMESH_Mesh::_narrow(anObj);
	    aSubMesh = SMESH::SMESH_subMesh::_narrow(anObj);
	    if (!aMesh->_is_nil()){
	      GEOM::GEOM_Shape_var refShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh(aMeshSObj);
	      if (!refShape->_is_nil()) {
		if(!smeshGUI->myComponentMesh->IsReadyToCompute(aMesh,refShape)){
		  QAD_MessageBox::warn1(QAD_Application::getDesktop(), 
					tr("SMESH_WRN_WARNING"),
					tr("SMESH_WRN_MISSING_PARAMETERS"),
					tr("SMESH_BUT_OK"));
		  break;
		}
		try{
		  if (smeshGUI->myComponentMesh->Compute(aMesh,refShape))
                    smeshGUI->myStudyAPI.ModifiedMesh(aMeshSObj,true);
		  // TO Do : change icon of all submeshes
		}
		catch(const SALOME::SALOME_Exception & S_ex){
		  QtCatchCorbaException(S_ex);
		}
	      }
	    }else if(!aSubMesh->_is_nil()){
	      aMesh = aSubMesh->GetFather();
	      GEOM::GEOM_Shape_var refShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh(aMeshSObj);
	      if(!refShape->_is_nil()){
		bool compute = smeshGUI->myComponentMesh->IsReadyToCompute(aMesh,refShape);
		if(!compute){
		  QAD_MessageBox::warn1(QAD_Application::getDesktop(), 
					tr("SMESH_WRN_WARNING"),
					tr("SMESH_WRN_MISSING_PARAMETERS"),
					tr("SMESH_BUT_OK"));
		  break;
		}
		try{
		  if ( smeshGUI->myComponentMesh->Compute(aMesh,refShape) )
                    smeshGUI->myStudyAPI.ModifiedMesh(aMeshSObj,true);
		  // TO Do : change icon of all submeshes
		}catch(const SALOME::SALOME_Exception & S_ex){
		  QtCatchCorbaException(S_ex);
		}
	      }
	    }
	  }
	}
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	TVisualObjPtr aVisualObj = GetVisualObj(anId,IObject->getEntry());
	if(smeshGUI->myAutomaticUpdate && aVisualObj){
	  aVisualObj->Update();
	  SMESH_Actor* anActor = ::FindActorByEntry(IObject->getEntry());
	  if(!anActor){
	    anActor = ::CreateActor(smeshGUI->myStudy,IObject->getEntry());
	    if(anActor){
	      ::DisplayActor(smeshGUI->myActiveStudy->getActiveStudyFrame(),anActor); //apo
	      ::FitAll();
	    }
	  }
	}
      }else{
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), 
			      tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }

  case 702:					// ADD SUB MESH 
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	SMESHGUI_AddSubMeshDlg *aDlg = new SMESHGUI_AddSubMeshDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
    
  case 703:					// INIT MESH 
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SMESHGUI_InitMeshDlg *aDlg = new SMESHGUI_InitMeshDlg(parent, "", Sel);
      break;
    }

  case 704:					// EDIT Hypothesis 
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg(parent, "", Sel);
      break;
    }

  case 705:					//  EDIT Global Hypothesis
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg(parent, "", Sel);
      break;
    }

  case 706:					//  EDIT Local Hypothesis
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg(parent, "", Sel);
      break;
    }

  case 406:					// ORIENTATION ELEMENTS
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      smeshGUI->myDesktop->SetSelectionMode(FaceSelection, true);
      SMESHGUI_OrientationElementsDlg *aDlg = new SMESHGUI_OrientationElementsDlg(parent, "", Sel);
      break;
    }

  case 407:					// DIAGONAL INVERSION
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      Sel->ClearIObjects();
      smeshGUI->myDesktop->SetSelectionMode(EdgeOfCellSelection, true);
      SMESHGUI_DiagonalInversionDlg *aDlg = new SMESHGUI_DiagonalInversionDlg(parent, "", Sel);
      break;
    }

  case 801:                                     // CREATE GROUP
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();
      if (nbSel == 1) {
	// check if mesh is selected
	Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	if (IObject->hasEntry()) {
	  SALOMEDS::SObject_var aMeshSObj = smeshGUI->myStudy->FindObjectID(IObject->getEntry());
	  if (!aMeshSObj->_is_nil()) {
	    CORBA::Object_var anObj = aMeshSObj->GetObject();
	    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow(anObj);
	    SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(anObj);
	    if (aMesh->_is_nil() && !aSubMesh->_is_nil()) {
	      aMesh = aSubMesh->GetFather();
	    }
	    if (!aMesh->_is_nil()) {
	      SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg(parent, "", Sel, aMesh);
	      aDlg->show();
	    }
	  }
	}
      }
      break;
    }

  case 802:                                     // CONSTRUCT GROUP
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();
      if (nbSel == 1) {
	// check if submesh is selected
	Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	if (IObject->hasEntry()) {
	  SALOMEDS::SObject_var aSObj = smeshGUI->myStudy->FindObjectID(IObject->getEntry());
	  if(!aSObj->_is_nil()) {
	    SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(aSObj->GetObject());
	    if (!aSubMesh->_is_nil()) {
	      try {
		SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
		// get submesh elements list by types
		SMESH::long_array_var aNodes = aSubMesh->GetElementsByType(SMESH::NODE);
		SMESH::long_array_var aEdges = aSubMesh->GetElementsByType(SMESH::EDGE);
		SMESH::long_array_var aFaces = aSubMesh->GetElementsByType(SMESH::FACE);
		SMESH::long_array_var aVolumes = aSubMesh->GetElementsByType(SMESH::VOLUME);
		// create group for each type o elements
		QString aName = IObject->getName();
		MESSAGE("SMESHGUI::OnGUIEvent - Construct group on submesh : "<<aName);
		MESSAGE(" Number of elements : nodes "<<aNodes->length() << ", edges " << aEdges->length()
			<< ", faces " << aFaces->length() << ", volumes " << aVolumes->length());
		if (aNodes->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = smeshGUI->AddGroup(aMesh, SMESH::NODE, aName);
		  aGroup->Add(aNodes);
		}
		if (aEdges->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = smeshGUI->AddGroup(aMesh, SMESH::EDGE, aName);
		  aGroup->Add(aEdges);
		}
		if (aFaces->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = smeshGUI->AddGroup(aMesh, SMESH::FACE, aName);
		  aGroup->Add(aFaces);
		}
		if (aVolumes->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = smeshGUI->AddGroup(aMesh, SMESH::VOLUME, aName);
		  aGroup->Add(aVolumes);
		}
		smeshGUI->myActiveStudy->updateObjBrowser(true);
	      }catch(const SALOME::SALOME_Exception & S_ex){
		QtCatchCorbaException(S_ex);
	      }
	    }
	  }
	}
      }
      break;
    }

  case 803:                                     // EDIT GROUP
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();
      if (nbSel == 1) {
	// check if group is selected
	Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	if (IObject->hasEntry()) {
	  SALOMEDS::SObject_var aSObj = smeshGUI->myStudy->FindObjectID(IObject->getEntry());
	  if(!aSObj->_is_nil()) {
	    SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_narrow(aSObj->GetObject());
	    if (!aGroup->_is_nil()) {
	      SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg(parent, "", Sel, aGroup);
	      aDlg->show();
	    }
	  }
	}
      }
      break;
    }

  case 804:                                     // Add elements to group
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myState == 800) {
	SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) smeshGUI->myActiveDialogBox;
	if (aDlg) aDlg->onAdd();
      }
      break;
    }

  case 805:                                     // Remove elements from group
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myState == 800) {
	SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) smeshGUI->myActiveDialogBox;
	if (aDlg) aDlg->onRemove();
      }
      break;
    }

  case 900:					// MESH INFOS
    {
      smeshGUI->EmitSignalDeactivateDialog();
      SMESHGUI_MeshInfosDlg *aDlg = new SMESHGUI_MeshInfosDlg(parent, "", false);
      break;
    }

  case 1001:					// AUTOMATIC UPDATE PREFERENCES
    {
      parent->menuBar()->setItemChecked(1001, !parent->menuBar()->isItemChecked(1001));
      if (parent->menuBar()->isItemChecked(1001)) {
	QAD_CONFIG->addSetting("SMESH:AutomaticUpdate", "true");
	smeshGUI->myAutomaticUpdate = true;
      }
      else {
	QAD_CONFIG->addSetting("SMESH:AutomaticUpdate", "false");
	smeshGUI->myAutomaticUpdate = false;
      }
      break;
    }

  case 1003:					// MESH PREFERENCES
    {
      smeshGUI->SetDisplaySettings();
      break;
    }

  case 1005:
    {
      SMESHGUI_Preferences_ScalarBarDlg::ScalarBarPreferences( parent );
      break;
    }

  case 1006: 
    {
      SMESHGUI_Preferences_SelectionDlg* aDlg = 
	new SMESHGUI_Preferences_SelectionDlg(parent);

      QColor aColor;
      QString SCr, SCg, SCb;
      SCr = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectColorRed");
      SCg = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectColorGreen");
      SCb = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectColorBlue");
      if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
	aColor = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
      else aColor = Qt::cyan;
      aDlg->SetColor(1, aColor);

      SCr = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectColorRed");
      SCg = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectColorGreen");
      SCb = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectColorBlue");
      if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
	aColor = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
      else aColor = Qt::yellow;
      aDlg->SetColor(2, aColor);

      SCr = QAD_CONFIG->getSetting("SMESH:SettingsSelectColorRed");
      SCg = QAD_CONFIG->getSetting("SMESH:SettingsSelectColorGreen");
      SCb = QAD_CONFIG->getSetting("SMESH:SettingsSelectColorBlue");
      if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
	aColor = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
      else aColor = Qt::white;
      aDlg->SetColor(3, aColor);

      QString SW1 = QAD_CONFIG->getSetting("SMESH:SettingsPreSelectWidth");
      if (SW1.isEmpty()) SW1 = "5";
      aDlg->SetWidth(1, SW1.toInt());

      QString SW2 = QAD_CONFIG->getSetting("SMESH:SettingsItemSelectWidth");
      if (SW2.isEmpty()) SW2 = "5";
      aDlg->SetWidth(2, SW2.toInt());

      QString SP1 = QAD_CONFIG->getSetting("SMESH:SettingsNodeSelectTol");
      if (SP1.isEmpty()) SP1 = "0.025";
      aDlg->SetPrecision(1, SP1.toDouble());

      QString SP2 = QAD_CONFIG->getSetting("SMESH:SettingsElementsSelectTol");
      if (SP2.isEmpty()) SP2 = "0.001";
      aDlg->SetPrecision(2, SP2.toDouble());

      if (aDlg->exec()) {
	QColor aPreColor = aDlg->GetColor(1);
	QAD_CONFIG->addSetting("SMESH:SettingsPreSelectColorRed", aPreColor.red());
	QAD_CONFIG->addSetting("SMESH:SettingsPreSelectColorGreen", aPreColor.green());
	QAD_CONFIG->addSetting("SMESH:SettingsPreSelectColorBlue", aPreColor.blue());

	QColor aSelColor = aDlg->GetColor(2);
	QAD_CONFIG->addSetting("SMESH:SettingsItemSelectColorRed", aSelColor.red());
	QAD_CONFIG->addSetting("SMESH:SettingsItemSelectColorGreen", aSelColor.green());
	QAD_CONFIG->addSetting("SMESH:SettingsItemSelectColorBlue", aSelColor.blue());

	QColor aHiColor = aDlg->GetColor(3);
	QAD_CONFIG->addSetting("SMESH:SettingsSelectColorRed", aHiColor.red());
	QAD_CONFIG->addSetting("SMESH:SettingsSelectColorGreen", aHiColor.green());
	QAD_CONFIG->addSetting("SMESH:SettingsSelectColorBlue", aHiColor.blue());

	int aPreWidth = aDlg->GetWidth(1);
	QAD_CONFIG->addSetting("SMESH:SettingsPreSelectWidth", aPreWidth);
	int aSelWidth = aDlg->GetWidth(2);
	QAD_CONFIG->addSetting("SMESH:SettingsItemSelectWidth", aSelWidth);

	double aTolNodes = aDlg->GetPrecision(1);
	QAD_CONFIG->addSetting("SMESH:SettingsNodeSelectTol", aTolNodes);
	double aTolItems = aDlg->GetPrecision(2);
	QAD_CONFIG->addSetting("SMESH:SettingsElementsSelectTol", aTolItems);

	// update current study settings
	::UpdateSelectionProp();

	QAD_StudyFrame* studyFrame = smeshGUI->myActiveStudy->getActiveStudyFrame();
	if (studyFrame->getTypeView() == VIEW_VTK) {
	  VTKViewer_ViewFrame* aViewFrame = GetVtkViewFrame(studyFrame);
	  // update VTK viewer properties
	  VTKViewer_RenderWindowInteractor* anInteractor = aViewFrame->getRWInteractor();
	  if (anInteractor) {
	    anInteractor->SetSelectionProp(aSelColor.red()/255., aSelColor.green()/255., 
					   aSelColor.blue()/255., aSelWidth);
	    anInteractor->SetSelectionTolerance(aTolNodes, aTolItems);
	    VTKViewer_InteractorStyleSALOME* aStyle = anInteractor->GetInteractorStyleSALOME();
	    if (aStyle)
	      aStyle->setPreselectionProp(aPreColor.red()/255., aPreColor.green()/255., 
					  aPreColor.blue()/255., aPreWidth);
	  }
	  // update actors
	  vtkRenderer* aRenderer = aViewFrame->getRenderer();
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

      break;
    }

  case 1100:					// EDIT HYPOTHESIS
    {
      if(checkLock(aStudy)) break;
      SALOME_Selection *Sel =
        SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();

      if (nbSel == 1) {
        Standard_Boolean res;
        SMESH::SMESH_Hypothesis_var Hyp =
          smeshGUI->ConvertIOinSMESHHypothesis(Sel->firstIObject(), res);

        /* Look for all mesh objects that have this hypothesis affected in order to flag as ModifiedMesh */
        /* At end below '...->updateObjBrowser(true)' will change icon of mesh objects                   */
        /* Warning : however by internal mechanism all subMeshes icons are changed !                     */
        if ( res )
        {
          char* sName = Hyp->GetName();
          SMESHGUI_GenericHypothesisCreator* aCreator = smeshGUI->GetHypothesisCreator(sName);
          if (aCreator)
          {
            aCreator->EditHypothesis(Hyp);
          }
          else
          {
            // report error
          }
        }
      }
      break;
    }

  case 1101:					// RENAME
    {
      if(checkLock(aStudy)) break;
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
      for (; It.More(); It.Next()) {
	Handle(SALOME_InteractiveObject) IObject = It.Value();

	SALOMEDS::SObject_var obj = smeshGUI->myStudy->FindObjectID(IObject->getEntry());
	SALOMEDS::GenericAttribute_var anAttr;
	SALOMEDS::AttributeName_var aName;
	if (!obj->_is_nil()) {
	  if (obj->FindAttribute(anAttr, "AttributeName")) {
	    aName = SALOMEDS::AttributeName::_narrow(anAttr);
	    QString newName = QString(aName->Value());
	    newName = SALOMEGUI_NameDlg::getName(QAD_Application::getDesktop(), newName);
	    if (!newName.isEmpty()) {
	      smeshGUI->myActiveStudy->renameIObject(IObject, newName);
	    }
	  }
	}
      }
      break;
    }
    
  case 1102:					// REMOVE HYPOTHESIS / ALGORITHMS
    {
      if(checkLock(aStudy)) break;
      QAD_WaitCursor wc;
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
      for (int i = 0; It.More(); It.Next(), i++) {
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	smeshGUI->RemoveHypothesisOrAlgorithmOnMesh(IObject);
      }
      Sel->ClearIObjects();
      smeshGUI->myActiveStudy->updateObjBrowser(true);
      break;
    }

  case 401:					// GEOM::EDGE
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_AddEdgeDlg *aDlg = new SMESHGUI_AddEdgeDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4021:					// TRIANGLE
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_AddFaceDlg *aDlg = new SMESHGUI_AddFaceDlg(parent, "", Sel, 3);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4022:					// QUAD
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_AddFaceDlg *aDlg = new SMESHGUI_AddFaceDlg(parent, "", Sel, 4);
      }
      else
	{
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }
  case 4031:					// TETRA
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_AddVolumeDlg *aDlg = new SMESHGUI_AddVolumeDlg(parent, "", Sel, 4);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4032:					// HEXA
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_AddVolumeDlg *aDlg = new SMESHGUI_AddVolumeDlg(parent, "", Sel, 8);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }

  case 4041:					// REMOVES NODES
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(NodeSelection, true);
	smeshGUI->ViewNodes();
	SMESHGUI_RemoveNodesDlg *aDlg = new SMESHGUI_RemoveNodesDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4042:					// REMOVES ELEMENTS
    {
      if(checkLock(aStudy)) break;
      if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	smeshGUI->EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(CellSelection, true);
	SMESHGUI_RemoveElementsDlg *aDlg = new SMESHGUI_RemoveElementsDlg(parent, "", Sel);
      }
      else
	{
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }

  case 5000: // HYPOTHESIS
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SMESHGUI_CreateHypothesesDlg *aDlg =
        new SMESHGUI_CreateHypothesesDlg (parent, "", FALSE, false);
      break;
    }
  case 5010: // ALGO
    {
      if(checkLock(aStudy)) break;
      smeshGUI->EmitSignalDeactivateDialog();
      SMESHGUI_CreateHypothesesDlg *aDlg =
        new SMESHGUI_CreateHypothesesDlg (parent, "", FALSE, true);
      break;
    }

  case 6016:					// CONTROLS 
  case 6015:
  case 6014:
  case 6013:
  case 6012:
  case 6011:
  case 6001:
  case 6003:
  case 6004:    
    if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      if ( Sel->IObjectCount() == 1 && Sel->firstIObject()->hasEntry() ) {
	SALOMEDS::SObject_var SO = smeshGUI->myStudy->FindObjectID( Sel->firstIObject()->getEntry() );
	if ( !SO->_is_nil() ) {
	  CORBA::Object_var aObject = SO->GetObject();
	  SMESH::SMESH_Mesh_var    aMesh    = SMESH::SMESH_Mesh::_narrow( aObject );
	  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( aObject );
	  SMESH::SMESH_Group_var   aGroup   = SMESH::SMESH_Group::_narrow( aObject );
	  if ( !aMesh->_is_nil() || !aSubMesh->_is_nil() || !aGroup->_is_nil() ) {
	    smeshGUI->Control( theCommandID );
	    break;
	  }
	}
      }
      QAD_MessageBox::warn1(smeshGUI->GetDesktop(), 
			    tr( "SMESH_WRN_WARNING" ),
			    tr( "SMESH_BAD_SELECTION" ), 
			    tr( "SMESH_BUT_OK" ) );
      break;
    }
    else {
      QAD_MessageBox::warn1(smeshGUI->GetDesktop(), 
			    tr( "SMESH_WRN_WARNING" ),
			    tr( "NOT_A_VTK_VIEWER" ), 
			    tr( "SMESH_BUT_OK" ) );
    }
    break;
  case 9010:
    {
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      if (Sel->IObjectCount() == 1)	{
	Handle(SALOME_InteractiveObject) anIObject = Sel->firstIObject();
	if(anIObject->hasEntry())
	  if(SMESH_Actor *anActor = ::FindActorByEntry(anIObject->getEntry())){
	    anActor->SetPointsLabeled( !anActor->GetPointsLabeled() );
	  }
      }
      break;
    }
  case 9011:
    {
      SALOME_Selection *Sel = SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
      if (Sel->IObjectCount() == 1)	{
	Handle(SALOME_InteractiveObject) anIObject = Sel->firstIObject();
	if(anIObject->hasEntry())
	  if(SMESH_Actor *anActor = ::FindActorByEntry(anIObject->getEntry())){
	    anActor->SetCellsLabeled( !anActor->GetCellsLabeled() );
	  }
      }
      break;
    }
  case 10001:				// DISPLAY MODE PREFERENCE
    {
      // Wireframe
      parent->menuBar()->setItemChecked(10001, true);
      parent->menuBar()->setItemChecked(10002, false);
      parent->menuBar()->setItemChecked(10004, false);
      parent->menuBar()->setItemEnabled(10003, true);
      QAD_CONFIG->addSetting("SMESH:DisplayMode", "Wireframe");
      break;
    }
  case 10002:
    {
      parent->menuBar()->setItemChecked(10002, true);
      parent->menuBar()->setItemChecked(10001, false);
      parent->menuBar()->setItemChecked(10004, false);
      parent->menuBar()->setItemEnabled(10003, true);
      QAD_CONFIG->addSetting("SMESH:DisplayMode", "Shading");
      break;
    }
  case 10003:
    {
      parent->menuBar()->setItemChecked(10003,!parent->menuBar()->isItemChecked(10003));
      QAD_CONFIG->addSetting("SMESH:Shrink", parent->menuBar()->isItemChecked(10003) ? "yes" : "no");
      break;
    }
  case 10004:
    {
      parent->menuBar()->setItemChecked(10001, false);
      parent->menuBar()->setItemChecked(10004, true);
      parent->menuBar()->setItemChecked(10002, false);
      parent->menuBar()->setItemEnabled(10003, false);
      QAD_CONFIG->addSetting("SMESH:DisplayMode", "Nodes");
      break;
    }
    
  }

  smeshGUI->myActiveStudy->updateObjBrowser(true);
  return true;
}

//=============================================================================
/*! function : GetMeshesUsingAlgoOrHypothesis()
 *  purpose  : return a list of Study objects (mesh kind) that have 'AlgoOrHyp' affected.
 *           : However is supposed here that father of father of an hypothesis is a Mesh Object.
 */
//=============================================================================      
SALOMEDS::Study::ListOfSObject *
	SMESHGUI::GetMeshesUsingAlgoOrHypothesis(SMESH::
	SMESH_Hypothesis_ptr AlgoOrHyp)
{
  SALOMEDS::Study::ListOfSObject_var listSOmesh =
    new SALOMEDS::Study::ListOfSObject;
  listSOmesh->length(0);
  unsigned int index = 0;
  if (!AlgoOrHyp->_is_nil()) {
    SALOMEDS::SObject_var SO_Hypothesis =
      smeshGUI->GetStudyAPI().FindObject(AlgoOrHyp);
    if (!SO_Hypothesis->_is_nil()) {
      SALOMEDS::Study::ListOfSObject_var listSO =
	smeshGUI->myStudy->FindDependances(SO_Hypothesis);
      MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): dependency number ="<<listSO->length());
      for (unsigned int i = 0; i < listSO->length(); i++) {
	SALOMEDS::SObject_ptr SO = listSO[i];
	if (!SO->_is_nil()) { 
	  SALOMEDS::SObject_var aFather = SO->GetFather();
	  if (!aFather->_is_nil()) {
	    SALOMEDS::SObject_var SOfatherFather = aFather->GetFather();
	    if (!SOfatherFather->_is_nil()) {
	      MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): dependency added to list");
	      index++;
	      listSOmesh->length(index);
	      listSOmesh[index - 1] = SOfatherFather;
	    }
	  }
	}
      }
    }
  }
  MESSAGE("SMESHGUI::GetMeshesUsingAlgoOrHypothesis(): completed");
  return listSOmesh._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Import_Mesh(QAD_Desktop * parent, int theCommandID){
  QString filter;
  string myExtension;
  
  if(theCommandID == 113){
    filter = tr("MED files (*.med)");
  }else if (theCommandID == 112){
    filter = tr("IDEAS files (*.unv)");
  }else if (theCommandID == 111){
    filter = tr("DAT files (*.dat)");
  }
  QString filename = QAD_FileDlg::getFileName(parent,
					      "",
					      filter,
					      tr("Import mesh"),
					      true);
  if(!filename.isEmpty()){
    QAD_WaitCursor wc;
    SMESH::mesh_array_var aMeshes;
    try {
      SMESH::DriverMED_ReadStatus res;
      aMeshes = smeshGUI->myComponentMesh->CreateMeshesFromMED(filename.latin1(),
                                                               res);
      if ( res > SMESH::DRS_OK ) {
	wc.stop();
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"),
			      tr(QString("SMESH_DRS_%1").arg(res)),
			      tr("SMESH_BUT_OK"));
	wc.start();

      }
    }
    catch (const SALOME::SALOME_Exception& S_ex)
    {
      wc.stop();
      QtCatchCorbaException(S_ex);
      wc.start();
    }
    for ( int i = 0, n = aMeshes->length(); i < n; i++ ) {
      SALOMEDS::SObject_var aMeshSO = smeshGUI->myStudyAPI.FindObject( aMeshes[i] );
      if ( !aMeshSO->_is_nil() ) {
	SALOMEDS::StudyBuilder_var aBuilder = smeshGUI->myStudy->NewBuilder();
	SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow( aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" ) );
	aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" );
      }
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Export_Mesh(QAD_Desktop * parent, int theCommandID)
{
  SALOME_Selection *Sel =
    SALOME_Selection::Selection(smeshGUI->myActiveStudy->getSelection());
  int nbSel = Sel->IObjectCount();
  if ( !nbSel )
    return;
  Standard_Boolean res;
  Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
  SMESH::SMESH_Mesh_var aMesh = smeshGUI->ConvertIOinMesh(IObject, res);
  if ( res && ( nbSel == 1 || theCommandID == 122 ) ) {// MED export supports multiple meshes
    QString aFilter, aTitle = tr("Export mesh");
    switch ( theCommandID ) {
    case 122:
      aFilter = tr("MED files (*.med)");
      break;
    case 121:
      aFilter = tr("DAT files (*.dat)");
      break;
    case 123:
      aFilter = tr("IDEAS files (*.unv)");
      break;
    default:
      break;
    }
    
    QString filename = QAD_FileDlg::getFileName(parent, "", aFilter, aTitle, false);
    
    if ( !filename.isEmpty() ) {
      // Check whether the file already exists and delete it if yes
      QFile aFile( filename );
      if ( aFile.exists() )
	aFile.remove();
      
      QAD_WaitCursor wc;
      for ( SALOME_ListIteratorOfListIO it( Sel->StoredIObjects() ); it.More(); it.Next() ) {
	aMesh = smeshGUI->ConvertIOinMesh( it.Value(), res );
	if ( res ) {
	  switch ( theCommandID ) {
	  case 122:
	    aMesh->ExportMED( filename.latin1(), true ); // currently, automatic groups are always created
	    break;
	  case 121:
	    aMesh->ExportDAT( filename.latin1() );
	    break;
	  case 123:
	    aMesh->ExportUNV( filename.latin1() );
	    break;
	  default:
	    break;
	  }
	}
      }
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnMousePress(QMouseEvent * pe, QAD_Desktop * parent,
			    QAD_StudyFrame * studyFrame)
{
  return false;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnMouseMove(QMouseEvent * pe, QAD_Desktop * parent,
			   QAD_StudyFrame * studyFrame)
{
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnKeyPress(QKeyEvent * pe, QAD_Desktop * parent,
			  QAD_StudyFrame * studyFrame)
{
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::SetSettings(QAD_Desktop * parent)
{
  MESSAGE("SMESHGUI::SetSettings.");
  SMESHGUI::GetOrCreateSMESHGUI(parent);

  /* Display mode */
  QString DisplayMode = "Shading";
  if ( QAD_CONFIG->hasSetting("SMESH:DisplayMode") )
    DisplayMode = QAD_CONFIG->getSetting("SMESH:DisplayMode");
  else
    QAD_CONFIG->addSetting("SMESH:DisplayMode", "Shading");

  bool Shrink = false;
  if ( QAD_CONFIG->hasSetting("SMESH:Shrink") )
    Shrink = QAD_CONFIG->getSetting("SMESH:Shrink") == "yes";

  if (DisplayMode == "Wireframe") {
    // wireframe
    parent->menuBar()->setItemChecked(10004, false);
    parent->menuBar()->setItemChecked(10002, false);
    parent->menuBar()->setItemChecked(10001, true);
    parent->menuBar()->setItemEnabled(10003, true);
  }
  else if (DisplayMode == "Nodes") {
    // poins
    parent->menuBar()->setItemChecked(10004, true);
    parent->menuBar()->setItemChecked(10002, false);
    parent->menuBar()->setItemChecked(10001, false);
    parent->menuBar()->setItemEnabled(10003, false);
  }
  else {
    // default is shading
    parent->menuBar()->setItemChecked(10004, false);
    parent->menuBar()->setItemChecked(10002, true);
    parent->menuBar()->setItemChecked(10001, false);
    parent->menuBar()->setItemEnabled(10003, true);
  }
  parent->menuBar()->setItemChecked(10003, Shrink);

  /* Automatic Update */
  QString AutoUpdate = QAD_CONFIG->getSetting("SMESH:AutomaticUpdate");
  if (AutoUpdate.compare("true") == 0) {
    parent->menuBar()->setItemChecked(1001, true);
    smeshGUI->myAutomaticUpdate = true;
  }
  else {
    parent->menuBar()->setItemChecked(1001, false);
    smeshGUI->myAutomaticUpdate = false;
  }

  /* Selection */
  ::UpdateSelectionProp();

  /* menus disable */
  parent->menuBar()->setItemEnabled(111, false);	// IMPORT DAT
  parent->menuBar()->setItemEnabled(112, false);	// IMPORT UNV

  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DefinePopup(QString & theContext, QString & theParent, QString & theObject)
{
  // NRI : Temporary added
  //  if ( smeshGUI->myStudy->GetProperties()->IsLocked() ) {
  //    theObject = "NothingSelected";
  //    theContext = "NothingSelected";
  //  }
  // NRI

  SALOME_Selection *Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount();
  switch ( nbSel ) {
  case 0:
    theObject = "NothingSelected";
    theContext = "NothingSelected";
    break;
  case 1:
    if ( smeshGUI->myState == 800 && Sel->HasIndex( Sel->firstIObject() ) )
      theObject = "Elements";
    else
      theObject = smeshGUI->CheckTypeObject( Sel->firstIObject() );
    theContext = "";
    break;
  default:
    theObject = smeshGUI->CheckHomogeneousSelection();
    theContext = "";
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::CustomPopup(QAD_Desktop* parent, QPopupMenu* popup, const QString& theContext,
			   const QString& theParent, const QString& theObject)
{
  // get active study frame
  QAD_StudyFrame* studyFrame = smeshGUI->myActiveStudy->getActiveStudyFrame();
  
  // get parent component which selected object(s) belongs to
  QString parentComp = ( (SALOMEGUI_Desktop*)parent )->getComponentFromSelection();

  // get selection
  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount();

  if ( nbSel == 0 ) {
    popup->clear();
  }
  else if ( nbSel == 1 ) {
    if ( parentComp != parent->getActiveComponent() ) {
      // object not belongs to SMESH module - remove all commands except common Display/Erase...
      while ( 1 ) {
	int id = popup->idAt( 0 );
	if ( id <= QAD_TopLabel_Popup_ID )
	  popup->removeItemAt( 0 );
	else
	  break;
      }
    }
    else {
      // get selected interactive object 
      Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
      SALOMEDS::SObject_var SO = smeshGUI->myStudy->FindObjectID( IObject->getEntry() );
      // find popup menu's TopLabel item
      int topItem = popup->indexOf( QAD_TopLabel_Popup_ID );
      if ( topItem >= 0 ) {
	if ( theParent == "Viewer" ) {
	  // set bold font for popup menu's TopLabel item (Viewer popup)
	  QFont fnt = QApplication::font(); fnt.setBold( TRUE );
	  popup->removeItem( QAD_TopLabel_Popup_ID );
	  popup->insertItem( new CustomItem( QString( IObject->getName() ), fnt ), QAD_TopLabel_Popup_ID, topItem );
	}
	else if ( theParent == "ObjectBrowser" ) {
	  // remove popup menu's TopLabel item (Object Browser popup)
	  popup->removeItem( QAD_TopLabel_Popup_ID );
	}
      }

      // remove "Display only" command for component object
      if ( theObject.compare( "Component" ) == 0 ) {
	popup->removeItem( QAD_DisplayOnly_Popup_ID );
      }
      else if ( theObject == "Hypothesis" || theObject == "Algorithm" ) {
	// remove Display/Erase commands
	popup->removeItem( QAD_Display_Popup_ID );
	popup->removeItem( QAD_DisplayOnly_Popup_ID );
	popup->removeItem( QAD_Erase_Popup_ID );
	// remove "Unassign ..." command if hyp/algo is not assigned
	if ( SO->_is_nil() || !IObject->hasReference()/* !IsReferencedObject( SO ) */)
	  popup->removeItem( 1102 );
      }
      else if ( theObject == "Mesh" || theObject == "SubMesh" || theObject == "Group" ) {
	// get actor
	GEOM::GEOM_Shape_var aShape = GetSMESHGUI()->GetStudyAPI().GetShapeOnMeshOrSubMesh( SO );
	if ( aShape->_is_nil() ) {
	  // imported mesh
	  popup->removeItem( 701 ); // Compute
	  popup->removeItem( 705 ); // Edit hypothesis
	  popup->removeItem( 706 ); // ...
	}
	SMESH_Actor* ac = ::FindActorByEntry(IObject->getEntry());
	// if object has actor
	if ( ac && studyFrame->getTypeView() == VIEW_VTK ) {
	  VTKViewer_RenderWindowInteractor* myRenderInter =
	    ( ( VTKViewer_ViewFrame* ) studyFrame->getRightFrame()->getViewFrame() )->getRWInteractor();
	  if ( myRenderInter->isVisible( IObject ) ) {
	    popup->removeItem( QAD_Display_Popup_ID );
	    popup->setItemChecked( 9010, ac->GetPointsLabeled() ); // Numbering / Display Nodes #
	    popup->setItemChecked( 9011, ac->GetCellsLabeled() );  // Numbering / Display Elements #
	    TVisualObjPtr aVisualObj = ac->GetObject();
	    int aNbEdges = aVisualObj->GetNbEntities(SMESH::EDGE);
	    int aNbFaces = aVisualObj->GetNbEntities(SMESH::FACE);
	    int aNbVolumes = aVisualObj->GetNbEntities(SMESH::VOLUME);
	    QMenuItem* mi = popup->findItem( 1131 );
	    if ( mi && mi->popup() ) {
	      int  prType = ac->GetRepresentation();
	      // Display Mode / Wireframe
	      if(!aNbFaces && !aNbVolumes && !aNbEdges){
		mi->popup()->removeItem( 211 );
	      }else{
		mi->popup()->setItemChecked( 211, prType == SMESH_Actor::eEdge );
	      }
	      // Display Mode / Shading
	      if(!aNbFaces && !aNbVolumes){
		mi->popup()->removeItem( 212 );
	      }else{
		mi->popup()->setItemChecked( 212, prType == SMESH_Actor::eSurface );
	      }
	      // Display Mode / Points
	      mi->popup()->setItemChecked( 215, prType == SMESH_Actor::ePoint );  
	      // Display Mode / Shrink
	      bool isShrunk = ac->IsShrunk();
	      bool isShrunkable = ac->IsShrunkable();
	      mi->popup()->setItemChecked( 213, isShrunk );   
	      mi->popup()->setItemEnabled( 213, prType != SMESH_Actor::ePoint && isShrunkable);
	    }
	    // Scalar Bar
	    mi = popup->findItem( 2000 );
	    if ( mi && mi->popup() ) {
	      SMESH_Actor::eControl cMode = ac->GetControlMode();
	      switch ( cMode ) {
	      case SMESH_Actor::eLengthEdges:
		mi->popup()->setItemChecked( 6001, true ); break;
	      case SMESH_Actor::eFreeBorders:
		mi->popup()->setItemChecked( 6003, true );
		mi->popup()->removeItem( 201 );
		break;
	      case SMESH_Actor::eMultiConnection:
		mi->popup()->setItemChecked( 6004, true ); break;
	      case SMESH_Actor::eArea:
		mi->popup()->setItemChecked( 6011, true ); break;
	      case SMESH_Actor::eTaper:
		mi->popup()->setItemChecked( 6012, true ); break;
	      case SMESH_Actor::eAspectRatio:
		mi->popup()->setItemChecked( 6013, true ); break;
	      case SMESH_Actor::eMinimumAngle:
		mi->popup()->setItemChecked( 6014, true ); break;
	      case SMESH_Actor::eWarping:
		mi->popup()->setItemChecked( 6015, true ); break;
	      case SMESH_Actor::eSkew:
		mi->popup()->setItemChecked( 6016, true ); break;
	      case SMESH_Actor::eNone:
	      default:
		mi->popup()->removeItem( 200 );
		mi->popup()->removeItem( 201 );
		break;
	      }
	      TVisualObjPtr aVisualObj = ac->GetObject();
	      SMESH::ElementType aType;
	      if(!aNbEdges){
		mi->popup()->removeItem( 6001 );
		mi->popup()->removeItem( 6003 );
		mi->popup()->removeItem( 6004 );
	      }
	      if(!aNbFaces){
		mi->popup()->removeItem( 6011 );
		mi->popup()->removeItem( 6012 );
		mi->popup()->removeItem( 6013 );
		mi->popup()->removeItem( 6014 );
		mi->popup()->removeItem( 6015 );
		mi->popup()->removeItem( 6016 );
	      }
	      if(!aNbFaces && !aNbEdges)
		popup->removeItem( 2000 );                         // Scalar Bar
	    }
	  }
	  else {
	    popup->removeItem( QAD_Erase_Popup_ID );
	    popup->removeItem( 114 );                              // Numbering popup menu
	    popup->removeItem( 1131 );                             // Display mode
	    popup->removeItem( 1132 );                             // Color / size
	    popup->removeItem( 1133 );                             // Transparency
	    popup->removeItem( 2000 );                             // Scalar Bar
	  }
	}
	else {
	  // object doesn't have actor
	  CORBA::Object_var anObject = SO->GetObject();
	  bool bDisplay = false;
	  if ( !CORBA::is_nil( anObject ) ) {
	    SMESH::SMESH_Mesh_var aMeshObj = SMESH::SMESH_Mesh::_narrow( anObject );
	    if ( !aMeshObj->_is_nil() && ( aMeshObj->NbNodes() > 0 || aMeshObj->NbFaces() > 0  || aMeshObj->NbVolumes() > 0 ) )
	      bDisplay = true;
	    SMESH::SMESH_subMesh_var aSubMeshObj = SMESH::SMESH_subMesh::_narrow( anObject );
	    if ( !aSubMeshObj->_is_nil() && ( aSubMeshObj->GetNumberOfNodes() > 0 || aSubMeshObj->GetNumberOfElements() > 0 ) )
	      bDisplay = true;
	    SMESH::SMESH_Group_var aGroupObj = SMESH::SMESH_Group::_narrow( anObject );
	    if ( !aGroupObj->_is_nil() && aGroupObj->Size() > 0 )
	      bDisplay = true;
	  }
	  if ( !bDisplay ) {
	    popup->removeItem( QAD_Display_Popup_ID );
	    popup->removeItem( QAD_DisplayOnly_Popup_ID );
	  }
	  popup->removeItem( QAD_Erase_Popup_ID );
	  popup->removeItem( 114 );                              // Numbering popup menu
	  popup->removeItem( 1131 );                             // Display mode
	  popup->removeItem( 1132 );                             // Color / size
	  popup->removeItem( 1133 );                             // Transparency
	  popup->removeItem( 2000 );                             // Scalar Bar
	}
      }
      else {
	// another SMESH object
	popup->removeItem( QAD_Display_Popup_ID );
	popup->removeItem( QAD_DisplayOnly_Popup_ID );
	popup->removeItem( QAD_Erase_Popup_ID );
      }
    }
  }
  else {
    // multiple selection
    if ( parentComp != parent->getActiveComponent() ) {
      // object not belongs to SMESH module - remove all commands except common Display/Erase...
      while ( 1 )	{
	int id = popup->idAt( 0 );
	if ( id <= QAD_TopLabel_Popup_ID )
	  popup->removeItemAt( 0 );
	else
	  break;
      }
      if ( parentComp.isNull() ) {
	// objects from different components are selected
	popup->removeItem( QAD_DisplayOnly_Popup_ID );
	popup->removeItem( QAD_Display_Popup_ID );
	popup->removeItem( QAD_Erase_Popup_ID );
      }
    }
    else {
      QString type = smeshGUI->CheckHomogeneousSelection();
      if ( type != "Heterogeneous Selection" ) {
	int topItem = popup->indexOf( QAD_TopLabel_Popup_ID );
	if ( topItem >= 0 ) {
	  // set bold font for popup menu's TopLabel item
	  QFont fnt = QApplication::font(); fnt.setBold( TRUE );
	  popup->removeItem( QAD_TopLabel_Popup_ID );
	  popup->insertItem( new CustomItem( QString("%1 ").arg( nbSel ) + type + "(s) ", fnt ), QAD_TopLabel_Popup_ID, topItem );
	}
      }
    }
  }
  return false;
}

//=============================================================================
/*! Method:  BuildPresentation(const Handle(SALOME_InteractiveObject)& theIO)
 *  Purpose: ensures that the actor for the given <theIO> exists in the active VTK view
 */
//=============================================================================
void SMESHGUI::BuildPresentation(const Handle(SALOME_InteractiveObject) & theIO)
{
  if(theIO->hasEntry()){
    QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
    QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
    ::UpdateView(aStudyFrame,eDisplay,theIO->getEntry());
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::setOrb()
{
	try
	{
		ORB_INIT & init = *SINGLETON_ < ORB_INIT >::Instance();
		ASSERT(SINGLETON_ < ORB_INIT >::IsAlreadyExisting());
		_orb = init(0, 0);
	} catch(...)
	{
		INFOS("internal error : orb not found");
		_orb = 0;
	}
	ASSERT(!CORBA::is_nil(_orb));
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Actor *SMESHGUI::ReadScript(SMESH::SMESH_Mesh_ptr theMesh){}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Dump(SMESH_Actor * Mactor)
{
	vtkUnstructuredGrid *ugrid = Mactor->GetUnstructuredGrid();
	vtkPoints *Pts = ugrid->GetPoints();
	int nbPts = Pts->GetNumberOfPoints();
	int nbCells = ugrid->GetNumberOfCells();

	FILE *In;
	int i, j;
	In = fopen("/tmp/dumpMesh", "w+");
	fprintf(In, "%d %d\n", nbPts, nbCells);
	for (int i = 0; i < nbPts; i++)
	{
		float *p = ugrid->GetPoint(i);
		fprintf(In, "%d %e %e %e\n", i, p[0], p[1], p[2]);
	}

	for (int i = 0; i < nbCells; i++)
	{
		fprintf(In, "%d %d", i, ugrid->GetCell(i)->GetCellType());
		vtkIdList *Id = ugrid->GetCell(i)->GetPointIds();
		for (j = 0; j < Id->GetNumberOfIds(); j++)
		{
			fprintf(In, " %d", Id->GetId(j));
		}
		fprintf(In, "\n");
	}
	fclose(In);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveNodes(SMESH::SMESH_Mesh_ptr theMesh,
			   const TColStd_MapOfInteger & MapIndex)
{
  QAD_WaitCursor wc;
  try{
    SALOMEDS::SObject_var aSobj = myStudyAPI.FindObject(theMesh);
    CORBA::String_var anEntry = aSobj->GetID();
    if(SMESH_Actor* anActor = ::FindActorByEntry(anEntry.in())){
      SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
      anArrayOfIdeces->length(MapIndex.Extent());
      TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
      for(int i = 0; ite.More(); ite.Next(), i++){
	anArrayOfIdeces[i] = anActor->GetNodeObjId(ite.Key());
      }
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->RemoveNodes(anArrayOfIdeces);
      if(myAutomaticUpdate){
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	if(TVisualObjPtr aVisualObj = GetVisualObj(anId,anEntry.in())){
	  aVisualObj->Update(true);
	}
      }
    }
  }catch(SALOME::SALOME_Exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveElements(SMESH::SMESH_Mesh_ptr theMesh,
			      const TColStd_MapOfInteger & MapIndex)
{
  QAD_WaitCursor wc;
  try{
    SALOMEDS::SObject_var aSobj = myStudyAPI.FindObject(theMesh);
    CORBA::String_var anEntry = aSobj->GetID();
    if(SMESH_Actor* anActor = ::FindActorByEntry(anEntry.in())){
      SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
      anArrayOfIdeces->length(MapIndex.Extent());
      TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
      for(int i = 0; ite.More(); ite.Next(), i++){
	anArrayOfIdeces[i] = anActor->GetElemObjId(ite.Key());
      }
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->RemoveElements(anArrayOfIdeces);
      if(myAutomaticUpdate){
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	if(TVisualObjPtr aVisualObj = GetVisualObj(anId,anEntry.in())){
	  aVisualObj->Update(true);
	}
      }
    }
  }catch(SALOME::SALOME_Exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::OrientationElements(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex)
{
  MESSAGE("OrientationElements - not implemeted yet!");
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DiagonalInversion(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex)
{
  MESSAGE("OrientationElements - not implemeted yet!");
}

//=====================================================================================
// EXPORTED METHODS
//=====================================================================================
void SMESHGUI::SupportedViewType(int *buffer, int bufferSize)
{
  if (!buffer || !bufferSize)
    return;
  buffer[0] = (int)VIEW_VTK;
}

void SMESHGUI::Deactivate()
{
  if ( SMESHGUI::GetSMESHGUI() ) {
    SMESHGUI::GetSMESHGUI()->EmitSignalCloseAllDialogs();
  }
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetPickable(SMESH_Actor* theActor){
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


void SMESHGUI::ViewNodes(){
  EraseSimulationActors();
  ::SetPointRepresentation(true);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Control( int theCommandID ){
  SALOME_Selection *Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
  if(Sel){
    Handle(SALOME_InteractiveObject) anIO = Sel->firstIObject();
    if(!anIO.IsNull()){
      QString aTitle;
      SMESH_Actor::eControl aControl = SMESH_Actor::eNone;
      if(SMESH_Actor *anActor = ::FindActorByEntry(anIO->getEntry())){
	switch ( theCommandID ){
	case 6001:
	  aTitle = tr( "LENGTH_EDGES" );
	  aControl = SMESH_Actor::eLengthEdges;
	  break;
	case 6003:
	  aTitle = tr( "FREE_BORDERS" );
	  aControl = SMESH_Actor::eFreeBorders;
	  break;
	case 6004:
	  aTitle = tr( "MULTI_BORDERS" );
	  aControl = SMESH_Actor::eMultiConnection;
	  break;
	case 6011:
	  aTitle = tr( "AREA_ELEMENTS" );
	  aControl = SMESH_Actor::eArea;
	  break;
	case 6012:
	  aTitle = tr( "TAPER_ELEMENTS" );
	  aControl = SMESH_Actor::eTaper;
	  break;
	case 6013:
	  aTitle = tr( "ASPECTRATIO_ELEMENTS" );
	  aControl = SMESH_Actor::eAspectRatio;
	  break;
	case 6014:
	  aTitle = tr( "MINIMUMANGLE_ELEMENTS" );
	  aControl = SMESH_Actor::eMinimumAngle;
	  break;
	case 6015:
	  aTitle = tr( "WARP_ELEMENTS" );
	  aControl = SMESH_Actor::eWarping;
	  break;
	case 6016:
	  aTitle = tr( "SKEW_ELEMENTS" );
	  aControl = SMESH_Actor::eSkew;
	  break;
	}
	anActor->SetControlMode(aControl);
	anActor->GetScalarBarActor()->SetTitle(aTitle.latin1());
      }
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetSettingsScalarBar(vtkScalarBarActor * theScalarBar,
				    QString Bold, 
				    QString Italic,
				    QString Shadow, 
				    QString FontFamily,
				    QString Orientation, 
				    float Width, 
				    float Height, 
				    int NbColors, 
				    int NbLabels)
{
  if (Bold.isNull() || Bold.isEmpty() || (Bold.compare("true") == 0))
    theScalarBar->BoldOn();
  else
    theScalarBar->BoldOff();
  
  if (Italic.isNull() || Italic.isEmpty() || (Italic.compare("true") == 0))
    theScalarBar->ItalicOn();
  else
    theScalarBar->ItalicOff();
  
  if (Shadow.isNull() || Shadow.isEmpty() || (Shadow.compare("true") == 0))
    theScalarBar->ShadowOn();
  else
    theScalarBar->ShadowOff();
  
  if (FontFamily.compare("Arial") == 0)
    theScalarBar->SetFontFamilyToArial();
  else if (FontFamily.compare("Courier") == 0)
    theScalarBar->SetFontFamilyToCourier();
  else if (FontFamily.compare("Times") == 0)
    theScalarBar->SetFontFamilyToTimes();
  else
    theScalarBar->SetFontFamilyToArial();
  
  if (Orientation.isNull() || Orientation.isEmpty() ||
      (Orientation.compare("Vertical") == 0))
    theScalarBar->SetOrientationToVertical();
  else
    theScalarBar->SetOrientationToHorizontal();
  
  theScalarBar->SetWidth((Width == 0) ? 0.17 : Width);
  theScalarBar->SetHeight((Height == 0) ? 0.8 : Height);
  
  theScalarBar->SetNumberOfLabels((NbLabels == 0) ? 5 : NbLabels);
  theScalarBar->SetMaximumNumberOfColors((NbColors == 0) ? 64 : NbColors);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetDisplaySettings()
{
  EmitSignalDeactivateDialog();
  SMESHGUI_Preferences_ColorDlg *aDlg =
    new SMESHGUI_Preferences_ColorDlg(QAD_Application::getDesktop(), "");

  QColor color;
  QString SCr, SCg, SCb;
  SCr = QAD_CONFIG->getSetting("SMESH:SettingsFillColorRed");
  SCg = QAD_CONFIG->getSetting("SMESH:SettingsFillColorGreen");
  SCb = QAD_CONFIG->getSetting("SMESH:SettingsFillColorBlue");
  if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
    color = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
  else color = QColor(0, 170, 255);
  aDlg->SetColor(1, color);

  SCr = QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorRed");
  SCg = QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorGreen");
  SCb = QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorBlue");
  if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
    color = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
  else color = QColor(0, 170, 255);
  aDlg->SetColor(2, color);

  SCr = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorRed");
  SCg = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorGreen");
  SCb = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorBlue");
  if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
    color = QColor(SCr.toInt(), SCg.toInt(), SCb.toInt());
  else color = Qt::red;
  aDlg->SetColor(3, color);

  QString SBr = QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorRed");
  QString SBg = QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorGreen");
  QString SBb = QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorBlue");
  if (!SCr.isEmpty() && !SCg.isEmpty() && !SCb.isEmpty())
    color = QColor(SBr.toInt(), SBg.toInt(), SBb.toInt());
  else color = Qt::blue;
  aDlg->SetColor(4, color);

  QString intValue = QAD_CONFIG->getSetting("SMESH:SettingsWidth");
  if (intValue.isEmpty()) intValue = "1";
  aDlg->SetIntValue(1, intValue.toInt());
  intValue = QAD_CONFIG->getSetting("SMESH:SettingsNodesSize");
  if (intValue.isEmpty()) intValue = "3";
  aDlg->SetIntValue(2, intValue.toInt());
  intValue = QAD_CONFIG->getSetting("SMESH:SettingsShrinkCoeff");
  if (intValue.isEmpty()) intValue = "75";
  aDlg->SetIntValue(3, intValue.toInt());

  if (aDlg->exec()) {
    QColor colorFill = aDlg->GetColor(1);
    QAD_CONFIG->addSetting("SMESH:SettingsFillColorRed", colorFill.red());
    QAD_CONFIG->addSetting("SMESH:SettingsFillColorGreen", colorFill.green());
    QAD_CONFIG->addSetting("SMESH:SettingsFillColorBlue", colorFill.blue());

    QColor colorOutline = aDlg->GetColor(2);
    QAD_CONFIG->addSetting("SMESH:SettingsOutlineColorRed", colorOutline.red());
    QAD_CONFIG->addSetting("SMESH:SettingsOutlineColorGreen", colorOutline.green());
    QAD_CONFIG->addSetting("SMESH:SettingsOutlineColorBlue", colorOutline.blue());

    QColor colorNode = aDlg->GetColor(3);
    QAD_CONFIG->addSetting("SMESH:SettingsNodeColorRed", colorNode.red());
    QAD_CONFIG->addSetting("SMESH:SettingsNodeColorGreen", colorNode.green());
    QAD_CONFIG->addSetting("SMESH:SettingsNodeColorBlue", colorNode.blue());

    QColor colorBackFace = aDlg->GetColor(4);
    QAD_CONFIG->addSetting("SMESH:SettingsBackFaceColorRed", colorBackFace.red());
    QAD_CONFIG->addSetting("SMESH:SettingsBackFaceColorGreen", colorBackFace.green());
    QAD_CONFIG->addSetting("SMESH:SettingsBackFaceColorBlue", colorBackFace.blue());

    int width = aDlg->GetIntValue(1);
    QAD_CONFIG->addSetting("SMESH:SettingsWidth", width);

    int nodes_size = aDlg->GetIntValue(2);
    QAD_CONFIG->addSetting("SMESH:SettingsNodesSize", nodes_size);

    int shrink_coeff = aDlg->GetIntValue(3);
    QAD_CONFIG->addSetting("SMESH:SettingsShrinkCoeff", shrink_coeff);
  }

  delete aDlg;
}

//=======================================================================
// function : Parameter()
// purpose  : return a parameter (double) from a dialog box
//
//  aValue     : is a double used as a default value displayed
//  aLabel     : is the title for aValue1
//  aTitle     : is the main title
//  bottom     : maximum value to be entered
//  top        : minimum value to be entered
//  decimals   : number of decimals
//=======================================================================
double SMESHGUI::Parameter(Standard_Boolean & res,
	const double aValue,
	const char *aLabel,
	const char *aTitle,
	const double bottom, const double top, const int decimals)
{
	SMESHGUI_aParameterDlg *Dialog =
		new SMESHGUI_aParameterDlg(QAD_Application::getDesktop(),
		aTitle,
		aLabel,
		bottom, top, decimals,
		TRUE);
	Dialog->setValue(aValue);
	double X = 0.0;
	res = (Dialog->exec() == QDialog::Accepted);
	if (res)
		X = Dialog->getDblValue();
	return X;
}

//=======================================================================
// function : Parameter()
// purpose  : return a parameter (int) from a dialog box
//
//  aValue     : is a int used as a default value displayed
//  aLabel     : is the title for aValue1
//  aTitle     : is the main title
//  bottom     : maximum value to be entered
//  top        : minimum value to be entered
//=======================================================================
int SMESHGUI::Parameter(Standard_Boolean & res,
	const int aValue,
	const char *aLabel, const char *aTitle, const int bottom, const int top)
{
	SMESHGUI_aParameterDlg *Dialog =
		new SMESHGUI_aParameterDlg(QAD_Application::getDesktop(),
		aTitle,
		aLabel,
		bottom, top,
		TRUE);
	Dialog->setValue(aValue);
	int X = 0;
	res = (Dialog->exec() == QDialog::Accepted);
	if (res)
		X = Dialog->getIntValue();
	return X;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayEdges(SMESH_Actor * ac, bool visibility){}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::InitActor(SMESH::SMESH_Mesh_ptr aMesh){}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Update(){
  if(VTKViewer_ViewFrame* aViewFrame = GetCurrentVtkView()){
    SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
    if(Sel->IObjectCount() == 0){
      vtkRenderer* aRenderer = aViewFrame->getRenderer();
      vtkActorCollection *aCollection = aRenderer->GetActors();
      aCollection->InitTraversal();
      while(vtkActor *anAct = aCollection->GetNextActor()){
	if(SMESH_Actor *anActor = dynamic_cast<SMESH_Actor*>(anAct)){
	  if(anActor->hasIO()){
	    Update(anActor->getIO());
	  }
	}
      }
    }else{
      SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
      for(; It.More(); It.Next()){
	Handle(SALOME_InteractiveObject) IO = It.Value();
	Update(IO);
      }
    }
    ::RepaintCurrentView();
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Update(const Handle(SALOME_InteractiveObject) & theIO){
  CORBA::Long anId = smeshGUI->myStudy->StudyId();
  TVisualObjPtr aVisualObj = ::GetVisualObj(anId,theIO->getEntry());
  aVisualObj->Update();
  ::UpdateView(eDisplay,theIO->getEntry());
}

//=============================================================================
/*!
 *
 */
//=============================================================================
vtkActor *SMESHGUI::SimulationMoveNode(SMESH_Actor * Mactor, int idnode)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return NULL;

	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();

	vtkUnstructuredGrid *ugrid =
		vtkUnstructuredGrid::SafeDownCast(Mactor->GetUnstructuredGrid());
	vtkIdList *IdCells = vtkIdList::New();
	ugrid->GetPointCells(idnode, IdCells);

	vtkPoints *Pts = vtkPoints::New();
	Pts = ugrid->GetPoints();

	vtkUnstructuredGrid *ugridSimulation = vtkUnstructuredGrid::New();
	ugridSimulation->SetPoints(Pts);
	vtkIdList *IdPts = vtkIdList::New();
	for (int j = 0; j < IdCells->GetNumberOfIds(); j++)
	{
		ugrid->GetCellPoints(IdCells->GetId(j), IdPts);
		ugridSimulation->InsertNextCell(ugrid->GetCellType(IdCells->GetId(j)),
			IdPts);
	}

	vtkProperty *prop = vtkProperty::New();
	prop->SetColor(1., 0., 0.);
	prop->SetRepresentationToWireframe();

	int Edgewidth = (int)Mactor->GetLineWidth();
	if (Edgewidth == 0)
		Edgewidth = 1;
	prop->SetLineWidth(Edgewidth + 1);

	vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
	Mapper->SetInput(ugridSimulation);
	vtkActor *ac = vtkActor::New();
	ac->SetMapper(Mapper);
	ac->SetProperty(prop);

	ac->GetMapper()->SetResolveCoincidentTopologyToShiftZBuffer();
	ac->GetMapper()->SetResolveCoincidentTopologyZShift(0.02);

	mySimulationActors->AddItem(ac);
	theRenderer->AddActor(ac);

	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();

	return ac;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationNode(SMESH::SMESH_Mesh_ptr aMesh, float x,
	float y, float z)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	Standard_Boolean result;
	SMESH_Actor *ac = FindActor(aMesh, result, true);

	if (result)
	{
		vtkUnstructuredGrid *ugrid =
			vtkUnstructuredGrid::SafeDownCast(ac->GetUnstructuredGrid());
		vtkUnstructuredGrid *newugrid = vtkUnstructuredGrid::New();

		vtkPoints *Pts = ugrid->GetPoints();
		vtkPoints *newPts = vtkPoints::New();
		int nbPts = Pts->GetNumberOfPoints();
		for (int i = 0; i < nbPts; i++)
		{
			newPts->InsertPoint(i, Pts->GetPoint(i));
		}

		newugrid->SetPoints(newPts);
		newugrid->GetPoints()->InsertNextPoint(x, y, z);

		vtkMaskPoints *verts = vtkMaskPoints::New();
		verts->SetInput(newugrid);
		verts->SetGenerateVertices(1);
		verts->SetOnRatio(1);

		vtkPolyDataMapper *vertMapper = vtkPolyDataMapper::New();
		vertMapper->SetInput(verts->GetOutput());
		vertMapper->ScalarVisibilityOff();

		vtkActor *node = vtkActor::New();
		node->SetMapper(vertMapper);

		QString SCr = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorRed");
		QString SCg = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorGreen");
		QString SCb = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorBlue");
		QColor nodecolor(SCr.toInt(), SCg.toInt(), SCb.toInt());
		if (!nodecolor.isValid())
//       nodecolor = QColor(0.,1.,0.);
			nodecolor = QColor(0, 1, 0);

		node->GetProperty()->SetColor(float (nodecolor.red()) / 255.,
			float (nodecolor.green()) / 255., float (nodecolor.blue()) / 255.);

		int intValue =
			QAD_CONFIG->getSetting("SMESH:SettingsNodesSize").toInt();
		if (intValue < 1)
			intValue == 1;

		node->GetProperty()->SetPointSize(intValue);

		node->VisibilityOn();

		mySimulationActors->AddItem(node);
		theRenderer->AddActor(node);
	}

	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationMoveNode(vtkActor * ac, int idnode, float x,
	float y, float z)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	float *pt = ac->GetMapper()->GetInput()->GetPoint(idnode);
	pt[0] = x;
	pt[1] = y;
	pt[2] = z;

	ac->GetMapper()->ImmediateModeRenderingOn();
	ac->GetProperty()->SetRepresentationToWireframe();

	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();

	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::MoveNode(SMESH::SMESH_Mesh_ptr aMesh, int idnode, float x,
	float y, float z)
{
	Standard_Boolean result;

	SMESH_Actor *MeshActor = FindActor(aMesh, result, true);
	if (result)
	{
	}
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ScalarVisibilityOff(){
  if(vtkRenderer *aRenderer = ::GetCurrentRenderer()){
    vtkActorCollection *actorList = aRenderer->GetActors();
    actorList->InitTraversal();
    while(vtkActor *ac = actorList->GetNextActor()){
      if(SMESH_Actor *anActor = SMESH_Actor::SafeDownCast(ac)){
	anActor->GetMapper()->ScalarVisibilityOff();
      }
    }
    ::RepaintCurrentView();
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationEdge(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	Standard_Boolean result;
	SMESH_Actor *ac = FindActor(aMesh, result, true);
	if (result)
	{
		TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
		int idNodes[2];
		int pts[2];
		int i = 0;
		vtkIdList *Ids = vtkIdList::New();
		for (; ite.More(); ite.Next())
		{
			idNodes[i] = ite.Key();
			i++;
		}

		Ids->InsertId(0, idNodes[0]);
		pts[0] = idNodes[0];
		Ids->InsertId(1, idNodes[1]);
		pts[1] = idNodes[1];

		vtkUnstructuredGrid *ugrid =
			vtkUnstructuredGrid::SafeDownCast(ac->GetUnstructuredGrid());
		vtkUnstructuredGrid *newugrid = vtkUnstructuredGrid::New();
		newugrid->SetPoints(ugrid->GetPoints());
		newugrid->InsertNextCell(VTK_LINE, 2, pts);

		vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
		Mapper->SetInput(newugrid);
		Mapper->Update();

		vtkActor *edge = vtkActor::New();
		edge->SetMapper(Mapper);
		edge->SetProperty(ac->GetProperty());
		edge->SetBackfaceProperty(ac->GetBackfaceProperty());
		edge->VisibilityOn();
		mySimulationActors->AddItem(edge);
		theRenderer->AddActor(edge);
	}

	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationTriangle(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex, bool reverse)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	Standard_Boolean result;
	SMESH_Actor *ac = FindActor(aMesh, result, true);
	if (result)
	{
		TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
		int idNodes[3];
		int pts[3];
		int i = 0;
		vtkIdList *Ids = vtkIdList::New();
		for (; ite.More(); ite.Next())
		{
			idNodes[i] = ite.Key();
			i++;
		}

		if (reverse)
		{
			Ids->InsertId(0, idNodes[2]);
			pts[0] = idNodes[2];
			Ids->InsertId(1, idNodes[1]);
			pts[1] = idNodes[1];
			Ids->InsertId(2, idNodes[0]);
			pts[2] = idNodes[0];
		}
		else
		{
			Ids->InsertId(0, idNodes[0]);
			pts[0] = idNodes[0];
			Ids->InsertId(1, idNodes[1]);
			pts[1] = idNodes[1];
			Ids->InsertId(2, idNodes[2]);
			pts[2] = idNodes[2];
		}
		vtkUnstructuredGrid *ugrid =
			vtkUnstructuredGrid::SafeDownCast(ac->GetUnstructuredGrid());
		vtkUnstructuredGrid *newugrid = vtkUnstructuredGrid::New();
		newugrid->SetPoints(ugrid->GetPoints());
		newugrid->InsertNextCell(VTK_TRIANGLE, 3, pts);

		vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
		Mapper->SetInput(newugrid);
		Mapper->Update();

		vtkActor *tri = vtkActor::New();
		tri->SetMapper(Mapper);
		tri->SetProperty(ac->GetProperty());
		tri->SetBackfaceProperty(ac->GetBackfaceProperty());
		tri->VisibilityOn();
		mySimulationActors->AddItem(tri);
		theRenderer->AddActor(tri);
	}

	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationQuadrangle(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex, bool reverse)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	Standard_Boolean result;
	SMESH_Actor *ac = FindActor(aMesh, result, true);

	vtkUnstructuredGrid *ugrid =
		vtkUnstructuredGrid::SafeDownCast(ac->GetUnstructuredGrid());

	if (result)
	{
		TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
		int i = 0;
		int idNodes[4];
		vtkIdList *Ids = vtkIdList::New();
		for (; ite.More(); ite.Next())
		{
			idNodes[i] = ite.Key();
			i++;
		}

		float *p0 = ugrid->GetPoint(idNodes[0]);
		float *p1 = ugrid->GetPoint(idNodes[1]);
		float *p2 = ugrid->GetPoint(idNodes[2]);
		float *p3 = ugrid->GetPoint(idNodes[3]);

		gp_Pnt P0(p0[0], p0[1], p0[2]);

		gp_Vec V1(P0, gp_Pnt(p1[0], p1[1], p1[2]));
		gp_Vec V2(P0, gp_Pnt(p2[0], p2[1], p2[2]));
		gp_Vec V3(P0, gp_Pnt(p3[0], p3[1], p3[2]));

		gp_Vec Cross1 = V1 ^ V2;
		gp_Vec Cross2 = V2 ^ V3;

		int tmp;
		if (Cross1.Dot(Cross2) < 0)
		{
			V1 = gp_Vec(P0, gp_Pnt(p2[0], p2[1], p2[2]));
			V2 = gp_Vec(P0, gp_Pnt(p1[0], p1[1], p1[2]));
			Cross1 = V1 ^ V2;
			Cross2 = V2 ^ V3;

			if (Cross1.Dot(Cross2) < 0)
			{
				tmp = idNodes[2];
				idNodes[2] = idNodes[3];
				idNodes[3] = tmp;
			}
			else
			{
				tmp = idNodes[1];
				idNodes[1] = idNodes[2];
				idNodes[2] = tmp;
			}
		}

		if (reverse)
		{
			Ids->InsertId(0, idNodes[3]);
			Ids->InsertId(1, idNodes[2]);
			Ids->InsertId(2, idNodes[1]);
			Ids->InsertId(3, idNodes[0]);
		}
		else
		{
			Ids->InsertId(0, idNodes[0]);
			Ids->InsertId(1, idNodes[1]);
			Ids->InsertId(2, idNodes[2]);
			Ids->InsertId(3, idNodes[3]);
		}

		//    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->GetUnstructuredGrid() );
		vtkUnstructuredGrid *newugrid = vtkUnstructuredGrid::New();
		newugrid->SetPoints(ugrid->GetPoints());
		newugrid->InsertNextCell(VTK_QUAD, Ids);

		vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
		Mapper->SetInput(newugrid);
		Mapper->Update();

		vtkActor *quad = vtkActor::New();
		quad->SetMapper(Mapper);
		quad->SetProperty(ac->GetProperty());
		quad->SetBackfaceProperty(ac->GetBackfaceProperty());
		quad->VisibilityOn();
		mySimulationActors->AddItem(quad);
		theRenderer->AddActor(quad);
	}
	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationTetra(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	Standard_Boolean result;
	SMESH_Actor *ac = FindActor(aMesh, result, true);
	if (result)
	{
		TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
		int i = 0;
		int idNodes[4];
		vtkIdList *Ids = vtkIdList::New();
		for (; ite.More(); ite.Next())
		{
			idNodes[i] = ite.Key();
			i++;
		}

		Ids->InsertId(0, idNodes[0]);
		Ids->InsertId(1, idNodes[1]);
		Ids->InsertId(2, idNodes[2]);
		Ids->InsertId(3, idNodes[3]);

		vtkUnstructuredGrid *ugrid =
			vtkUnstructuredGrid::SafeDownCast(ac->GetUnstructuredGrid());
		vtkUnstructuredGrid *newugrid = vtkUnstructuredGrid::New();
		newugrid->SetPoints(ugrid->GetPoints());
		newugrid->InsertNextCell(VTK_TETRA, Ids);

		vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
		Mapper->SetInput(newugrid);
		Mapper->Update();

		vtkActor *tetra = vtkActor::New();
		tetra->SetMapper(Mapper);
		tetra->SetProperty(ac->GetProperty());
		tetra->SetBackfaceProperty(ac->GetBackfaceProperty());
		tetra->VisibilityOn();
		mySimulationActors->AddItem(tetra);
		theRenderer->AddActor(tetra);
	}
	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationHexa(SMESH::SMESH_Mesh_ptr aMesh,
	const TColStd_MapOfInteger & MapIndex)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors = vtkActorCollection::New();
	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();
	Standard_Boolean result;
	SMESH_Actor *ac = FindActor(aMesh, result, true);
	if (result)
	{
		TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
		int i = 0;
		int idNodes[8];
		vtkIdList *Ids = vtkIdList::New();
		for (; ite.More(); ite.Next())
		{
			idNodes[i] = ite.Key();
			i++;
		}

		Ids->InsertId(0, idNodes[0]);
		Ids->InsertId(1, idNodes[1]);
		Ids->InsertId(2, idNodes[2]);
		Ids->InsertId(3, idNodes[3]);
		Ids->InsertId(4, idNodes[4]);
		Ids->InsertId(5, idNodes[5]);
		Ids->InsertId(6, idNodes[6]);
		Ids->InsertId(7, idNodes[7]);

		vtkUnstructuredGrid *ugrid =
			vtkUnstructuredGrid::SafeDownCast(ac->GetUnstructuredGrid());
		vtkUnstructuredGrid *newugrid = vtkUnstructuredGrid::New();
		newugrid->SetPoints(ugrid->GetPoints());
		newugrid->InsertNextCell(VTK_HEXAHEDRON, Ids);

		vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
		Mapper->SetInput(newugrid);
		Mapper->Update();

		vtkActor *hexa = vtkActor::New();
		hexa->SetMapper(Mapper);
		hexa->SetProperty(ac->GetProperty());
		hexa->SetBackfaceProperty(ac->GetBackfaceProperty());
		hexa->VisibilityOn();
		mySimulationActors->AddItem(hexa);
		theRenderer->AddActor(hexa);
	}
	vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
	renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddFace(SMESH::SMESH_Mesh_ptr theMesh,
		       const TColStd_MapOfInteger & MapIndex, 
		       bool reverse)
{
  QAD_WaitCursor wc;
  try{
    SALOMEDS::SObject_var aSobj = myStudyAPI.FindObject(theMesh);
    CORBA::String_var anEntry = aSobj->GetID();
    if(SMESH_Actor* anActor = ::FindActorByEntry(anEntry.in())){
      SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
      anArrayOfIdeces->length(MapIndex.Extent());
      TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
      for(int i = 0; ite.More(); ite.Next(), i++){
	anArrayOfIdeces[i] = anActor->GetNodeObjId(ite.Key());
      }
      int tmp;
      if(MapIndex.Extent() == 4){
	int idNodes[4];
	TColStd_MapIteratorOfMapOfInteger ite1(MapIndex);
	for(int i = 0; ite1.More(); ite1.Next(), i++){
	  idNodes[i] = ite1.Key();
	}
	vtkUnstructuredGrid *ugrid = anActor->GetUnstructuredGrid();
	float *p0 = ugrid->GetPoint(idNodes[0]);
	float *p1 = ugrid->GetPoint(idNodes[1]);
	float *p2 = ugrid->GetPoint(idNodes[2]);
	float *p3 = ugrid->GetPoint(idNodes[3]);
	
	gp_Pnt P0(p0[0], p0[1], p0[2]);
	
	gp_Vec V1(P0, gp_Pnt(p1[0], p1[1], p1[2]));
	gp_Vec V2(P0, gp_Pnt(p2[0], p2[1], p2[2]));
	gp_Vec V3(P0, gp_Pnt(p3[0], p3[1], p3[2]));
	
	gp_Vec Cross1 = V1 ^ V2;
	gp_Vec Cross2 = V2 ^ V3;
	
	if(Cross1.Dot(Cross2) < 0){
	  V1 = gp_Vec(P0, gp_Pnt(p2[0], p2[1], p2[2]));
	  V2 = gp_Vec(P0, gp_Pnt(p1[0], p1[1], p1[2]));
	  Cross1 = V1 ^ V2;
	  Cross2 = V2 ^ V3;
	  
	  if(Cross1.Dot(Cross2) < 0){
	    tmp = anArrayOfIdeces[2];
	    anArrayOfIdeces[2] = anArrayOfIdeces[3];
	    anArrayOfIdeces[3] = tmp;
	  }else{
	    tmp = anArrayOfIdeces[1];
	    anArrayOfIdeces[1] = anArrayOfIdeces[2];
	    anArrayOfIdeces[2] = tmp;
	  }
	}
      }
      if(reverse){
	for(int i = 0; i < (MapIndex.Extent() / 2); i++){
	  tmp = anArrayOfIdeces[i];
	  anArrayOfIdeces[i] = anArrayOfIdeces[MapIndex.Extent() - i - 1];
	  anArrayOfIdeces[MapIndex.Extent() - i - 1] = tmp;
	}
      }
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->AddFace(anArrayOfIdeces);
      if(myAutomaticUpdate){
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	if(TVisualObjPtr aVisualObj = GetVisualObj(anId,anEntry.in())){
	  aVisualObj->Update(true);
	}
	AddActorInSelection(anActor);
      }
    }
  }catch(SALOME::SALOME_Exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddVolume(SMESH::SMESH_Mesh_ptr theMesh,
			 const TColStd_MapOfInteger & MapIndex)
{
  QAD_WaitCursor wc;
  try{
    SALOMEDS::SObject_var aSobj = myStudyAPI.FindObject(theMesh);
    CORBA::String_var anEntry = aSobj->GetID();
    if(SMESH_Actor* anActor = ::FindActorByEntry(anEntry.in())){
      SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
      anArrayOfIdeces->length(MapIndex.Extent());
      TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
      for(int i = 0; ite.More(); ite.Next(), i++){
	anArrayOfIdeces[i] = anActor->GetNodeObjId(ite.Key());
      }
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->AddVolume(anArrayOfIdeces);
      if(myAutomaticUpdate){
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	if(TVisualObjPtr aVisualObj = GetVisualObj(anId,anEntry.in())){
	  aVisualObj->Update(true);
	}
      }
    }
  }catch(SALOME::SALOME_Exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
}
//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddEdge(SMESH::SMESH_Mesh_ptr theMesh,
		       const TColStd_MapOfInteger & MapIndex)
{
  QAD_WaitCursor wc;
  try{
    SALOMEDS::SObject_var aSobj = myStudyAPI.FindObject(theMesh);
    CORBA::String_var anEntry = aSobj->GetID();
    if(SMESH_Actor* anActor = ::FindActorByEntry(anEntry.in())){
      SMESH::long_array_var anArrayOfIdeces = new SMESH::long_array;
      anArrayOfIdeces->length(MapIndex.Extent());
      TColStd_MapIteratorOfMapOfInteger ite(MapIndex);
      for(int i = 0; ite.More(); ite.Next(), i++){
	anArrayOfIdeces[i] = anActor->GetNodeObjId(ite.Key());
      }
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->AddEdge(anArrayOfIdeces);
      if(myAutomaticUpdate){
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	if(TVisualObjPtr aVisualObj = GetVisualObj(anId,anEntry.in())){
	  aVisualObj->Update(true);
	}
      }
    }
  }catch(SALOME::SALOME_Exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
}
//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddNode(SMESH::SMESH_Mesh_ptr theMesh, 
		       float x, float y, float z)
{
  QAD_WaitCursor wc;
  try{
    SALOMEDS::SObject_var aSobj = myStudyAPI.FindObject(theMesh);
    CORBA::String_var anEntry = aSobj->GetID();
    if(SMESH_Actor* anActor = ::FindActorByEntry(anEntry.in())){
      SMESH::SMESH_MeshEditor_var aMeshEditor = theMesh->GetMeshEditor();
      aMeshEditor->AddNode(x, y, z);
      if(myAutomaticUpdate){
	CORBA::Long anId = smeshGUI->myStudy->StudyId();
	if(TVisualObjPtr aVisualObj = GetVisualObj(anId,anEntry.in())){
	  aVisualObj->Update(true);
	}
      }
    }
  }catch(SALOME::SALOME_Exception& exc) {
    INFOS("Follow exception was cought:\n\t"<<exc.details.text);
  }catch(const std::exception& exc){
    INFOS("Follow exception was cought:\n\t"<<exc.what());
  }catch(...){
    INFOS("Unknown exception was cought !!!");
  }
}
//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayEdgesConnectivityLegendBox(vtkActor * ac)
{
	if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK)
		return;

	EraseSimulationActors();
	mySimulationActors2D = vtkActor2DCollection::New();

	vtkRenderer *theRenderer =
		((VTKViewer_ViewFrame *) myActiveStudy->getActiveStudyFrame()->
		getRightFrame()->getViewFrame())->getRenderer();

	vtkGlyphSource2D *gs2 = vtkGlyphSource2D::New();
	gs2->SetGlyphTypeToTriangle();
	gs2->FilledOff();

	vtkLegendBoxActor *legend = vtkLegendBoxActor::New();
	legend->GetPositionCoordinate()->SetValue(0.025, 0.025, 0.);
	legend->GetPosition2Coordinate()->SetValue(0.3, 0.3, 0.);	//relative to Position

	legend->SetNumberOfEntries(4);
	legend->SetEntryString(0, tr("SMESH_BOUNDARYEDGES"));
	legend->SetEntrySymbol(0, gs2->GetOutput());
	legend->SetEntryColor(0, ac->GetMapper()->GetLookupTable()->GetColor(0.));
	legend->SetEntryString(1, tr("SMESH_MANIFOLDEDGES"));
	legend->SetEntrySymbol(1, gs2->GetOutput());
	legend->SetEntryColor(1,
		ac->GetMapper()->GetLookupTable()->GetColor(0.666667));
	legend->SetEntryString(2, tr("SMESH_NONMANIFOLDEDGES"));
	legend->SetEntrySymbol(2, gs2->GetOutput());
	legend->SetEntryColor(2,
		ac->GetMapper()->GetLookupTable()->GetColor(0.222222));
	legend->SetEntryString(3, tr("SMESH_FEATUREEDGES"));
	legend->SetEntrySymbol(3, gs2->GetOutput());
	legend->SetEntryColor(3,
		ac->GetMapper()->GetLookupTable()->GetColor(0.444444));
	legend->SetPadding(5);
	//  legend->GetProperty()->SetColor();

	mySimulationActors2D->AddItem(legend);
	theRenderer->AddActor2D(legend);

	// Update the view
	myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame()->
		Repaint();
}

//===============================================================================
// function : OnEditDelete()
// purpose  :
//===============================================================================
void SMESHGUI::OnEditDelete()
{
  if (QAD_MessageBox::warn2
      (QAD_Application::getDesktop(),
       tr("SMESH_WRN_WARNING"),
       tr("SMESH_REALLY_DELETE"),
       tr("SMESH_BUT_YES"), tr("SMESH_BUT_NO"), 1, 0, 0) != 1)
    return;

  int nbSf = myActiveStudy->getStudyFramesCount();

  Standard_Boolean found;
  SALOMEDS::Study_var aStudy = myActiveStudy->getStudyDocument();
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeIOR_var anIOR;

  SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
  SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
  for(; It.More(); It.Next()){
    Handle(SALOME_InteractiveObject) IObject = It.Value();
    if(IObject->hasEntry()){
      SALOMEDS::SObject_var SO = myStudy->FindObjectID(IObject->getEntry());

      /* Erase child graphical objects */
      SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(SO);
      for(; it->More(); it->Next()){
        SALOMEDS::SObject_var CSO = it->Value();
        if(CSO->FindAttribute(anAttr, "AttributeIOR")){
          anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	  
          for(int i = 0; i < nbSf; i++){
            QAD_StudyFrame *sf = myActiveStudy->getStudyFrame(i);
	    CORBA::String_var anEntry = CSO->GetID();
	    if(SMESH_Actor* anActor = ::FindActorByEntry(sf,anEntry.in())){
	      RemoveActor(sf,anActor);
	    }
          }
        }
      }

      /* Erase main graphical object */
      for(int i = 0; i < nbSf; i++){
	QAD_StudyFrame *sf = myActiveStudy->getStudyFrame(i);
	if(SMESH_Actor* anActor = ::FindActorByEntry(sf,IObject->getEntry())){
	  RemoveActor(sf,anActor);
	}
      }

      // Remove object(s) from data structures
      SALOMEDS::SObject_var obj = myStudy->FindObjectID(IObject->getEntry());
      if(!obj->_is_nil()){
	SMESH::SMESH_Group_var     aGroup = SMESH::SMESH_Group  ::_narrow(obj->GetObject());
	SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(obj->GetObject());

	if ( !aGroup->_is_nil() ) {                          // DELETE GROUP
	  SMESH::SMESH_Mesh_var aMesh = aGroup->GetMesh();
	  aMesh->RemoveGroup( aGroup );
	}
	else if ( !aSubMesh->_is_nil() ) {                   // DELETE SUBMESH
	  SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
	  aMesh->RemoveSubMesh( aSubMesh );
	}
	else {// default action: remove SObject from the study
	  // san - it's no use opening a transaction here until UNDO/REDO is provided in SMESH
	  //QAD_Operation *op = new SALOMEGUI_ImportOperation(myActiveStudy);
	  //op->start();
	  aStudyBuilder->RemoveObjectWithChildren( obj );
	  //op->finish();
	}
      }

    } /* IObject->hasEntry() */
  } /* more/next */

  /* Clear any previous selection */
  Sel->ClearIObjects();
  myActiveStudy->updateObjBrowser();
}

//=======================================================================
// name    : SMESHGUI::GetFilterMgr
// Purpose : Get filter manager
//=======================================================================
SMESH::FilterManager_ptr SMESHGUI::GetFilterMgr()
{
   return myFilterMgr;
}
