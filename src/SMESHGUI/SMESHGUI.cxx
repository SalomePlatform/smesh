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
#include "SMESHGUI_InitMeshDlg.h"
#include "SMESHGUI_AddSubMeshDlg.h"
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI_ClippingDlg.h"
#include "SMESHGUI_GroupDlg.h"
#include "SMESHGUI_RemoveNodesDlg.h"
#include "SMESHGUI_RemoveElementsDlg.h"
#include "SMESHGUI_MeshInfosDlg.h"
#include "SMESHGUI_StandardMeshInfosDlg.h"
#include "SMESHGUI_Preferences_ColorDlg.h"
#include "SMESHGUI_Preferences_ScalarBarDlg.h"
#include "SMESHGUI_Preferences_SelectionDlg.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_MoveNodesDlg.h"
#include "SMESHGUI_AddMeshElementDlg.h"
#include "SMESHGUI_EditHypothesesDlg.h"
#include "SMESHGUI_CreateHypothesesDlg.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_FilterLibraryDlg.h"
#include "SMESHGUI_SingleEditDlg.h"
#include "SMESHGUI_MultiEditDlg.h"
#include "SMESHGUI_GroupOpDlg.h"
#include "SMESHGUI_DeleteGroupDlg.h"
#include "SMESHGUI_SmoothingDlg.h"
#include "SMESHGUI_RenumberingDlg.h"
#include "SMESHGUI_ExtrusionDlg.h"
#include "SMESHGUI_RevolutionDlg.h"
#include "SMESHGUI_TranslationDlg.h"
#include "SMESHGUI_RotationDlg.h"
#include "SMESHGUI_SymmetryDlg.h"
#include "SMESHGUI_SewingDlg.h"
#include "SMESHGUI_MergeNodesDlg.h"
#include "SMESHGUI_MeshPatternDlg.h"
#include "SMESHGUI_PrecisionDlg.h"

#include "VTKViewer_ViewFrame.h"
#include "VTKViewer_InteractorStyleSALOME.h"
#include "VTKViewer_RenderWindowInteractor.h"

#include "SMESH_Actor.h"
#include "SMESH_Object.h"

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

#include "SALOME_NamingService.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_InteractiveObject.hxx"

#include "SALOMEGUI_Desktop.h"
#include "SALOMEGUI_NameDlg.h"
#include "SALOMEGUI_ImportOperation.h"
#include "SALOMEGUI_QtCatchCorbaException.hxx"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_SMESHGenUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

// QT Includes
#define	 INCLUDE_MENUITEM_DEF
#include <qapplication.h>
#include <qlineedit.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfont.h>
#include <qstring.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qspinbox.h>
#include <qlist.h>
#include <qwidget.h>
#include <qevent.h>
#include <qradiobutton.h>

#include <boost/shared_ptr.hpp>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkActorCollection.h>
#include <vtkScalarBarActor.h>

#include "utilities.h"

using namespace std;

namespace{
  // Decalarations
  //=============================================================
  void ImportMeshesFromFile(QAD_Desktop * parent, 
			    SMESH::SMESH_Gen_ptr theComponentMesh,
			    int theCommandID);

  void ExportMeshToFile(QAD_Desktop * parent, int theCommandID);

  void SetViewMode(int theCommandID);

  void Control( int theCommandID );

  void SetDisplaySettings();


  // Definitions
  //=============================================================
  void ImportMeshesFromFile(QAD_Desktop * parent, 
			    SMESH::SMESH_Gen_ptr theComponentMesh,
			    int theCommandID)
  {
    QString filter;
    string myExtension;
  
    if(theCommandID == 113){
      filter = QObject::tr("MED files (*.med)");
    }else if (theCommandID == 112){
      filter = QObject::tr("IDEAS files (*.unv)");
    }else if (theCommandID == 111){
      filter = QObject::tr("DAT files (*.dat)");
    }
    QString filename = QAD_FileDlg::getFileName(parent,
						"",
						filter,
						QObject::tr("Import mesh"),
						true);
    if(!filename.isEmpty()) {
      QAD_WaitCursor wc;
      SALOMEDS::Study_var aStudy = SMESH::GetActiveStudyDocument();

      try {
	SMESH::mesh_array_var aMeshes = new SMESH::mesh_array;
	switch ( theCommandID ) {
	case 112:
	  {
	    aMeshes->length( 1 );
	    aMeshes[0] = theComponentMesh->CreateMeshesFromUNV(filename.latin1());
	    break;
	  }
	case 113:
	  {
	    SMESH::DriverMED_ReadStatus res;
	    aMeshes = theComponentMesh->CreateMeshesFromMED(filename.latin1(),res);
	    if ( res != SMESH::DRS_OK ) {
	      wc.stop();
	      QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				    QObject::tr("SMESH_WRN_WARNING"),
				    QObject::tr(QString("SMESH_DRS_%1").arg(res)),
				    QObject::tr("SMESH_BUT_OK"));
	      aMeshes->length( 0 );
	      wc.start();
	    }
	    break;
	  }
	}

	bool isEmpty = false;
	for ( int i = 0, iEnd = aMeshes->length(); i < iEnd; i++ ) {
	  SALOMEDS::SObject_var aMeshSO = SMESH::FindSObject( aMeshes[i] );
	  if ( !aMeshSO->_is_nil() ) {
	    SALOMEDS::StudyBuilder_var aBuilder = aStudy->NewBuilder();
	    SALOMEDS::AttributePixMap_var aPixmap = SALOMEDS::AttributePixMap::_narrow( aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" ) );
	    aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_IMPORTED");
	    if ( theCommandID == 112 ) // mesh names aren't taken from the file for UNV import
	      SMESH::SetName( aMeshSO, QFileInfo(filename).fileName() );
	  } else
	    isEmpty = true;
	}

	if ( isEmpty ) {
	  wc.stop();
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				QObject::tr("SMESH_WRN_WARNING"),
				QObject::tr("SMESH_DRS_EMPTY"),
				QObject::tr("SMESH_BUT_OK"));
	  wc.start();
	}
      }
      catch (const SALOME::SALOME_Exception& S_ex){
	wc.stop();
	QtCatchCorbaException(S_ex);
	wc.start();
      }
    }
  }


  void ExportMeshToFile(QAD_Desktop * parent, int theCommandID)
  {
    SALOME_Selection *aSel = SALOME_Selection::Selection(SMESH::GetActiveStudy()->getSelection());
    if(aSel->IObjectCount()){
      Handle(SALOME_InteractiveObject) anIObject = aSel->firstIObject();
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
      if ( !aMesh->_is_nil() ) {
	QString aFilter, aTitle = QObject::tr("Export mesh");
	switch ( theCommandID ) {
	case 122:
	  aFilter = QObject::tr("MED files (*.med)");
	  break;
	case 121:
	  aFilter = QObject::tr("DAT files (*.dat)");
	  break;
	case 123: {
	  if(aMesh->NbPyramids()){
	    int aRet = QAD_MessageBox::warn2(QAD_Application::getDesktop(),
					     QObject::tr("SMESH_WRN_WARNING"),
					     QObject::tr("SMESH_EXPORT_UNV").arg(anIObject->getName()),
					     QObject::tr("SMESH_BUT_YES"),
					     QObject::tr("SMESH_BUT_NO"),
					     0,1,0);
	    if(aRet)
	      return;
	  }
	  aFilter = QObject::tr("IDEAS files (*.unv)");
	  break;
	default:
	  return;
	}}
	
	QString aFilename = QAD_FileDlg::getFileName(parent, "", aFilter, aTitle, false);
	if ( !aFilename.isEmpty() ) {
	  // Check whether the file already exists and delete it if yes
	  QFile aFile( aFilename );
	  if ( aFile.exists() )
	    aFile.remove();
	  QAD_WaitCursor wc;
	  switch ( theCommandID ) {
	  case 122:
	    aMesh->ExportMED( aFilename.latin1(), true ); // currently, automatic groups are always created
	    break;
	  case 121:
	    aMesh->ExportDAT( aFilename.latin1() );
	    break;
	  case 123:
	    aMesh->ExportUNV( aFilename.latin1() );
	    break;
	  default:
	    break;
	  }
	}
      }
    }
  }  
  
  void SetViewMode(int theCommandID){
    SALOME_Selection *Sel = SALOME_Selection::Selection(SMESH::GetActiveStudy()->getSelection());
    if(Sel->IObjectCount() >= 1){
      switch(theCommandID){
      case 1134:{
	SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
	SMESHGUI_ClippingDlg *aDlg = 
	  new SMESHGUI_ClippingDlg(QAD_Application::getDesktop(),"",false);
	return;
      }
      case 1133:{
	SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
	SMESHGUI_TransparencyDlg *aDlg = 
	  new SMESHGUI_TransparencyDlg(QAD_Application::getDesktop(),"",false);
	return;
      }}
      SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
      for(; It.More(); It.Next()){
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	if(IObject->hasEntry()){
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
	    switch(theCommandID){
	    case 211:
	      anActor->SetRepresentation(SMESH_Actor::eEdge);
	      break;
	    case 212:
	      anActor->SetRepresentation(SMESH_Actor::eSurface);
	      break;
	    case 213:
	      if(anActor->IsShrunk()) 
		anActor->UnShrink();
	      else
		anActor->SetShrink();
	      break;
	    case 215:
	      anActor->SetRepresentation(SMESH_Actor::ePoint);
	      break;
	    case 1132:{
	      float color[3];
	      anActor->GetSufaceColor(color[0], color[1], color[2]);
	      int c0 = int (color[0] * 255);
	      int c1 = int (color[1] * 255);
	      int c2 = int (color[2] * 255);
	      QColor c(c0, c1, c2);
	      
	      float edgecolor[3];
	      anActor->GetEdgeColor(edgecolor[0], edgecolor[1], edgecolor[2]);
	      c0 = int (edgecolor[0] * 255);
	      c1 = int (edgecolor[1] * 255);
	      c2 = int (edgecolor[2] * 255);
	      QColor e(c0, c1, c2);
	      
	      float backfacecolor[3];
	      anActor->GetBackSufaceColor(backfacecolor[0], backfacecolor[1], backfacecolor[2]);
	      c0 = int (backfacecolor[0] * 255);
	      c1 = int (backfacecolor[1] * 255);
	      c2 = int (backfacecolor[2] * 255);
	      QColor b(c0, c1, c2);
	      
	      float nodecolor[3];
	      anActor->GetNodeColor(nodecolor[0], nodecolor[1], nodecolor[2]);
	      c0 = int (nodecolor[0] * 255);
	      c1 = int (nodecolor[1] * 255);
	      c2 = int (nodecolor[2] * 255);
	      QColor n(c0, c1, c2);
	      
	      int Edgewidth = (int)anActor->GetLineWidth();
	      if(Edgewidth == 0)
		Edgewidth = 1;
	      int intValue = int(anActor->GetNodeSize());
	      float Shrink = anActor->GetShrinkFactor();
	      
	      SMESHGUI_Preferences_ColorDlg *aDlg = 
		new SMESHGUI_Preferences_ColorDlg(QAD_Application::getDesktop(),"");
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
		anActor->SetSufaceColor(float (color.red()) / 255.,
					 float (color.green()) / 255., 
					 float (color.blue()) / 255.);
		anActor->SetBackSufaceColor(float (backfacecolor.red()) / 255., 
					     float (backfacecolor.green()) / 255.,
					     float (backfacecolor.blue()) / 255.);
		
		/* edge color */
		anActor->SetEdgeColor(float (edgecolor.red()) / 255.,
				       float (edgecolor.green()) / 255.,
				       float (edgecolor.blue()) / 255.);
		
		/* Shrink factor and size edges */
		anActor->SetShrinkFactor(aDlg->GetIntValue(3) / 100.);
		anActor->SetLineWidth(aDlg->GetIntValue(1));
		
		/* Nodes color and size */
		anActor->SetNodeColor(float (nodecolor.red()) / 255.,
			       float (nodecolor.green()) / 255.,
				       float (nodecolor.blue()) / 255.);
		anActor->SetNodeSize(aDlg->GetIntValue(2));
		
		delete aDlg;
	      }
	      break;
	    }}
	  }
	}
      }
      SMESH::RepaintCurrentView();
    }
  }


  void SetDisplaySettings()
  {
    SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
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
  
  
  void Control( int theCommandID )
  {
    SALOME_Selection *Sel = SALOME_Selection::Selection( SMESH::GetActiveStudy()->getSelection() );
    if(Sel){
      Handle(SALOME_InteractiveObject) anIO = Sel->firstIObject();
      if(!anIO.IsNull()){
	QString aTitle;
	SMESH_Actor::eControl aControl = SMESH_Actor::eNone;
	if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIO->getEntry())){
	  switch ( theCommandID ){
	  case 6001:
	    aTitle = QObject::tr( "LENGTH_EDGES" );
	    aControl = SMESH_Actor::eLengthEdges;
	    break;
	  case 6002:
	    aTitle = QObject::tr( "FREE_EDGES" );
	    aControl = SMESH_Actor::eFreeEdges;
	    break;
	  case 6003:
	    aTitle = QObject::tr( "FREE_BORDERS" );
	    aControl = SMESH_Actor::eFreeBorders;
	    break;
	  case 6004:
	    aTitle = QObject::tr( "MULTI_BORDERS" );
	    aControl = SMESH_Actor::eMultiConnection;
	    break;
	  case 6011:
	    aTitle = QObject::tr( "AREA_ELEMENTS" );
	    aControl = SMESH_Actor::eArea;
	    break;
	  case 6012:
	    aTitle = QObject::tr( "TAPER_ELEMENTS" );
	    aControl = SMESH_Actor::eTaper;
	    break;
	  case 6013:
	    aTitle = QObject::tr( "ASPECTRATIO_ELEMENTS" );
	    aControl = SMESH_Actor::eAspectRatio;
	    break;
	  case 6014:
	    aTitle = QObject::tr( "MINIMUMANGLE_ELEMENTS" );
	    aControl = SMESH_Actor::eMinimumAngle;
	    break;
	  case 6015:
	    aTitle = QObject::tr( "WARP_ELEMENTS" );
	    aControl = SMESH_Actor::eWarping;
	    break;
	  case 6016:
	    aTitle = QObject::tr( "SKEW_ELEMENTS" );
	    aControl = SMESH_Actor::eSkew;
	    break;
	  }
	  anActor->SetControlMode(aControl);
	  anActor->GetScalarBarActor()->SetTitle(aTitle.latin1());
	}
      }
    }
  }
  

  bool CheckOIType(const Handle(SALOME_InteractiveObject) & theIO,
		   MeshObjectType                           theType,
		   const QString                            theInTypeName,
		   QString &                                theOutTypeName)
  {
    SMESH_TypeFilter aTypeFilter( theType );
    if ( aTypeFilter.IsOk( theIO )) {
      theOutTypeName = theInTypeName;
      return true;
    }
    return false;
  }
    

  QString CheckTypeObject(const Handle(SALOME_InteractiveObject) & theIO)
  {
    SALOMEDS::Study_var aStudy = SMESH::GetActiveStudyDocument();
    SALOMEDS::SObject_var aSObj = aStudy->FindObjectID(theIO->getEntry());
    if (!aSObj->_is_nil()) {
      SALOMEDS::SComponent_var aSComp = aSObj->GetFatherComponent();
      CORBA::String_var anID = aSComp->GetID();
      if (!strcmp(anID.in(),theIO->getEntry()))
	return "Component";
    }
    
    QString aTypeName;
    if (
	CheckOIType ( theIO, HYPOTHESIS,    "Hypothesis", aTypeName ) ||
	CheckOIType ( theIO, ALGORITHM,     "Algorithm",  aTypeName ) ||
	CheckOIType ( theIO, MESH,          "Mesh",       aTypeName ) ||
	CheckOIType ( theIO, SUBMESH,       "SubMesh",    aTypeName ) ||
	CheckOIType ( theIO, GROUP,         "Group",      aTypeName )
	)
      return aTypeName;
    
    return "NoType";
  }


  QString CheckHomogeneousSelection()
  {
    QAD_Study* aStudy = SMESH::GetActiveStudy();
    SALOME_Selection *aSel = SALOME_Selection::Selection(aStudy->getSelection());
    /* copy the list */
    SALOME_ListIO List;
    SALOME_ListIteratorOfListIO Itinit(aSel->StoredIObjects());
    for (; Itinit.More(); Itinit.Next())
      {
	List.Append(Itinit.Value());
      }

    QString RefType = CheckTypeObject(aSel->firstIObject());
    SALOME_ListIteratorOfListIO It(List);
    for (; It.More(); It.Next())
      {
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	QString Type = CheckTypeObject(IObject);
	if (Type.compare(RefType) != 0)
	  return "Heterogeneous Selection";
      }
    
    aSel->ClearIObjects();
    SALOME_ListIteratorOfListIO It1(List);
    for (; It1.More(); It1.Next())
      {
	int res = aSel->AddIObject(It1.Value());
	if (res == -1)
	  aStudy->highlight(It1.Value(), false);
	if (res == 0)
	  aStudy->highlight(It1.Value(), true);
      }
    return RefType;
  }

  void OnEditDelete()
  {
    // VSR 17/11/04: check if all objects selected belong to SMESH component --> start
    QString aParentComponent = ((SALOMEGUI_Desktop*)QAD_Application::getDesktop())->getComponentFromSelection();
    if ( aParentComponent != QAD_Application::getDesktop()->getActiveComponent() )  {
      QAD_MessageBox::warn1 ( QAD_Application::getDesktop(),
			      QObject::tr("ERR_ERROR"), 
			      QObject::tr("NON_SMESH_OBJECTS_SELECTED").arg(QAD_Application::getDesktop()->getComponentUserName( "SMESH" )),
			      QObject::tr("BUT_OK") );
      return;
    }
    // VSR 17/11/04: check if all objects selected belong to SMESH component <-- finish
    if (QAD_MessageBox::warn2
	(QAD_Application::getDesktop(),
	 QObject::tr("SMESH_WRN_WARNING"),
	 QObject::tr("SMESH_REALLY_DELETE"),
	 QObject::tr("SMESH_BUT_YES"), QObject::tr("SMESH_BUT_NO"), 1, 0, 0) != 1)
      return;

    QAD_Study* anActiveStudy = SMESH::GetActiveStudy();
    int nbSf = anActiveStudy->getStudyFramesCount();
    
    SALOMEDS::Study_var aStudy = SMESH::GetActiveStudyDocument();
    SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var anIOR;
    
    SALOME_Selection *Sel = SALOME_Selection::Selection(anActiveStudy->getSelection());
    SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
    for(; It.More(); It.Next()){
      Handle(SALOME_InteractiveObject) IObject = It.Value();
      if(IObject->hasEntry()){
	SALOMEDS::SObject_var SO = aStudy->FindObjectID(IObject->getEntry());
	
	/* Erase child graphical objects */
	SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(SO);
	for(; it->More(); it->Next()){
	  SALOMEDS::SObject_var CSO = it->Value();
	  if(CSO->FindAttribute(anAttr, "AttributeIOR")){
	    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    
	    for(int i = 0; i < nbSf; i++){
	      QAD_StudyFrame *sf = anActiveStudy->getStudyFrame(i);
	      CORBA::String_var anEntry = CSO->GetID();
	      if(SMESH_Actor* anActor = SMESH::FindActorByEntry(sf,anEntry.in())){
		SMESH::RemoveActor(sf,anActor);
	      }
	    }
	  }
	}
	
	/* Erase main graphical object */
	for(int i = 0; i < nbSf; i++){
	  QAD_StudyFrame *sf = anActiveStudy->getStudyFrame(i);
	  if(SMESH_Actor* anActor = SMESH::FindActorByEntry(sf,IObject->getEntry())){
	    SMESH::RemoveActor(sf,anActor);
	  }
	}
	
	// Remove object(s) from data structures
	SALOMEDS::SObject_var obj = aStudy->FindObjectID(IObject->getEntry());
	if(!obj->_is_nil()){
	  SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(obj->GetObject());
	  SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow(obj->GetObject());
	  
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
    anActiveStudy->updateObjBrowser();
  }


}

//=============================================================================
/*!
 *
 */
//=============================================================================
class CustomItem : public QCustomMenuItem
{
public:
  CustomItem(const QString& s, const QFont& f) : myString(s), myFont(f) {}
  ~CustomItem() {}

  void paint(QPainter* p, const QColorGroup& cg, bool act, bool /*enabled*/, int x, int y, int w, int h)
  {
    p->save();
    p->fillRect( x, y, w, h, act ? cg.highlight() : cg.mid() );
    p->setPen( act ? cg.highlightedText() : cg.buttonText() );
    p->setFont( myFont );
    p->drawText( x, y, w, h, AlignHCenter | AlignVCenter | ShowPrefix | DontClip | SingleLine, myString );
    p->restore();
  }

  QSize sizeHint()
  {
    return QFontMetrics( myFont ).size( AlignHCenter | AlignVCenter | ShowPrefix | DontClip | SingleLine, myString );
  }

  bool fullSpan() const
  {
    return true;
  }

private:
  QString myString;
  QFont   myFont;
};

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::SMESHGUI( const QString& theName, QObject* theParent ) :
  SALOMEGUI( theName, theParent )
{
  myActiveDialogBox = 0 ;
  myState = -1 ;

  SMESH::GetSMESHGen();
  SMESH::GetFilterManager();
  SMESH::GetPattern();

  /* load resources for all available meshers */
  SMESH::InitAvailableHypotheses();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::~SMESHGUI()
{
  SMESH::GetFilterManager()->Destroy();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI* SMESHGUI::GetSMESHGUI()
{
  static SMESHGUI aSMESHGUI;
  aSMESHGUI.myActiveStudy = SMESH::GetActiveStudy();
  SMESH::GetSMESHGen()->SetCurrentStudy( SMESH::GetActiveStudyDocument() );

  /* Automatic Update flag */
  aSMESHGUI.myAutomaticUpdate = ( QAD_CONFIG->getSetting( "SMESH:AutomaticUpdate" ).compare( "true" ) == 0 );

  return &aSMESHGUI;
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
void SMESHGUI::EmitSignalStudyFrameChanged()
{
  emit this->SignalStudyFrameChanged();
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
QAD_Desktop *SMESHGUI::GetDesktop()
{
  return QAD_Application::getDesktop();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::ActiveStudyChanged(QAD_Desktop* parent)
{
  MESSAGE("SMESHGUI::activeStudyChanged init.");
  QAD_Study* prevStudy = myActiveStudy;
  /* Create or retrieve an object SMESHGUI */
  MESSAGE("Active study changed : prev study =" << prevStudy);
  SMESHGUI::GetSMESHGUI();
  EmitSignalStudyFrameChanged();
  MESSAGE("Active study changed : active study =" << myActiveStudy);
  if ( prevStudy != myActiveStudy ) {
    EmitSignalCloseAllDialogs();
    MESSAGE("Active study changed : SMESHGUI nullified" << endl);
    //smeshGUI = 0;
    SMESH::UpdateSelectionProp();
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
static int isStudyLocked(const SALOMEDS::Study_var& theStudy){
  return theStudy->GetProperties()->IsLocked();
}

static bool checkLock(const SALOMEDS::Study_var& theStudy) {
  if (isStudyLocked(theStudy)) {
    QAD_MessageBox::warn1 ( (QWidget*)QAD_Application::getDesktop(),
			   QObject::tr("WRN_WARNING"), 
			   QObject::tr("WRN_STUDY_LOCKED"),
			   QObject::tr("BUT_OK") );
    return true;
  }
  return false;
}

//=======================================================================
//function : CheckActiveStudyLocked
//purpose  : 
//=======================================================================

bool SMESHGUI::ActiveStudyLocked()
{
  SALOMEDS::Study_var aStudy = myActiveStudy->getStudyDocument();
  return checkLock( aStudy );
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnGUIEvent(int theCommandID, QAD_Desktop * parent)
{
  SALOMEDS::Study_var aStudy = SMESH::GetActiveStudyDocument(); //Document OCAF de l'etude active

  switch (theCommandID)	{
  case 33:					// DELETE
    if(checkLock(aStudy)) break;
    ::OnEditDelete();
    break;
    
  case 113:					// IMPORT
  case 112:
  case 111:
    {
      if(checkLock(aStudy)) break;
      ::ImportMeshesFromFile(parent,SMESH::GetSMESHGen(),theCommandID);
      break;
    }

  case 122:					// EXPORT MED
  case 121:
  case 123:
    {
      ::ExportMeshToFile(parent, theCommandID);
      break;
    }

  case 200:					// SCALAR BAR
    {
      SALOME_Selection *Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
      if( Sel && Sel->IObjectCount() ) {
	Handle(SALOME_InteractiveObject) anIO = Sel->firstIObject();
	if( anIO->hasEntry() ) {
	  if( SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() ) ) {
	    anActor->SetControlMode( SMESH_Actor::eNone );
	  }
	}
      }
      break;
    }
  case 201:
    {
      SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
      SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties( parent, Sel );
      break;
    }

  case 1134: // Clipping
  case 1133: // Tranparency
  case 1132: // Colors / Size
  case 215:
  case 213:
  case 212:
  case 211:
    {
      ::SetViewMode(theCommandID);
      break;
    }

  case 214:					// UPDATE
    {
      if(checkLock(aStudy)) break;
      SMESH::UpdateView();
      break;
    }

  case 300:					// ERASE
  case 301:					// DISPLAY
  case 302:					// DISPLAY ONLY
    {
      SMESH::EDisplaing anAction;
      switch(theCommandID){
      case 300:	anAction = SMESH::eErase; break;
      case 301:	anAction = SMESH::eDisplay; break;
      case 302:	anAction = SMESH::eDisplayOnly; break;
      }
      
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
	for (; It.More(); It.Next()) {
	  Handle(SALOME_InteractiveObject) IOS = It.Value();
	  if (IOS->hasEntry()) {
	    SMESH::UpdateView(anAction,IOS->getEntry());
	  }
	}
      }
      Sel->ClearIObjects();
      break;
    }

  case 400:					// NODES
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
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
    if ( myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK )
    {
      EmitSignalDeactivateDialog();
      SMESHGUI_FilterDlg *aDlg = new SMESHGUI_FilterDlg( parent, SMESH::EDGE );
    }
    break;
  }
    
  case 406:					// MOVE NODE
    {
      if ( myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK )
      {
        QAD_MessageBox::warn1( GetDesktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }
      
      if(checkLock(aStudy)) break;
      SALOME_Selection *Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
      SMESHGUI_MoveNodesDlg *aDlg = new SMESHGUI_MoveNodesDlg( parent, Sel );
      break;
    }
    
  case 701:					// COMPUTE MESH 
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
	int nbSel = Sel->IObjectCount();
	if (nbSel != 1){
	  break;
	}

	SMESH::SMESH_Mesh_var aMesh;
	SMESH::SMESH_subMesh_var aSubMesh;
	Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	if (IObject->hasEntry()){
	  SALOMEDS::SObject_var aMeshSObj = aStudy->FindObjectID(IObject->getEntry());
	  GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh( aMeshSObj );
	  if ( aShapeObject->_is_nil() ) {
	    // imported mesh
	    break;
	  }
	  if(!aMeshSObj->_is_nil()) {
	    SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(aMeshSObj);
	    SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(aMeshSObj);
	    
	    if (!aMesh->_is_nil()){
	      GEOM::GEOM_Object_var refShapeObject = SMESH::GetShapeOnMeshOrSubMesh(aMeshSObj);
	      if (!refShapeObject->_is_nil()) {
		if(!SMESH::GetSMESHGen()->IsReadyToCompute(aMesh,refShapeObject)){
		  QAD_MessageBox::warn1(QAD_Application::getDesktop(), 
					tr("SMESH_WRN_WARNING"),
					tr("SMESH_WRN_MISSING_PARAMETERS"),
					tr("SMESH_BUT_OK"));
		  break;
		}
		try{
		  if (SMESH::GetSMESHGen()->Compute(aMesh,refShapeObject))
                    SMESH::ModifiedMesh(aMeshSObj,true);
		  // TO Do : change icon of all submeshes
                  else
                    QAD_MessageBox::warn1(QAD_Application::getDesktop(), 
                                          tr("SMESH_WRN_WARNING"),
                                          tr("SMESH_WRN_COMPUTE_FAILED"),
                                          tr("SMESH_BUT_OK"));
		}
		catch(const SALOME::SALOME_Exception & S_ex){
		  QtCatchCorbaException(S_ex);
		}
	      }
	    }else if(!aSubMesh->_is_nil()){
	      aMesh = aSubMesh->GetFather();
	      GEOM::GEOM_Object_var refShapeObject = SMESH::GetShapeOnMeshOrSubMesh(aMeshSObj);
	      if(!refShapeObject->_is_nil()){
		bool compute = SMESH::GetSMESHGen()->IsReadyToCompute(aMesh,refShapeObject);
		if(!compute){
		  QAD_MessageBox::warn1(QAD_Application::getDesktop(), 
					tr("SMESH_WRN_WARNING"),
					tr("SMESH_WRN_MISSING_PARAMETERS"),
					tr("SMESH_BUT_OK"));
		  break;
		}
		try{
		  if ( SMESH::GetSMESHGen()->Compute(aMesh,refShapeObject) )
                    SMESH::ModifiedMesh(aMeshSObj,true);
		  // TO Do : change icon of all submeshes
                  else
                    QAD_MessageBox::warn1(QAD_Application::getDesktop(), 
                                          tr("SMESH_WRN_WARNING"),
                                          tr("SMESH_WRN_COMPUTE_FAILED"),
                                          tr("SMESH_BUT_OK"));
		}catch(const SALOME::SALOME_Exception & S_ex){
		  QtCatchCorbaException(S_ex);
		}
	      }
	    }
	  }
	}
	CORBA::Long anId = aStudy->StudyId();
	TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,IObject->getEntry());
	if(myAutomaticUpdate && aVisualObj){
	  aVisualObj->Update();
	  SMESH_Actor* anActor = SMESH::FindActorByEntry(IObject->getEntry());
	  if(!anActor){
	    anActor = SMESH::CreateActor(aStudy,IObject->getEntry());
	    if(anActor){
	      SMESH::DisplayActor(myActiveStudy->getActiveStudyFrame(),anActor); //apo
	      SMESH::FitAll();
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
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
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
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      SMESHGUI_InitMeshDlg *aDlg = new SMESHGUI_InitMeshDlg(parent, "", Sel);
      break;
    }

  case 704:					// EDIT Hypothesis 
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg(parent, "", Sel);
      break;
    }

  case 705:					//  EDIT Global Hypothesis
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg(parent, "", Sel);
      break;
    }

  case 706:					//  EDIT Local Hypothesis
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg(parent, "", Sel);
      break;
    }

  case 407: // DIAGONAL INVERSION
  case 408: // Delete diagonal
    {
      if ( myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK )
      {
        QAD_MessageBox::warn1( GetDesktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;
        
      SALOME_Selection *aSel =
        SALOME_Selection::Selection( myActiveStudy->getSelection() );
      /*Standard_Boolean aRes;
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IObject);
      if ( aMesh->_is_nil() )
      {
        QAD_MessageBox::warn1(GetDesktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "SMESH_BAD_SELECTION" ), tr( "SMESH_BUT_OK" ) );
        break;
      }
      */
      EmitSignalDeactivateDialog();
      if ( theCommandID == 407 )
        new SMESHGUI_TrianglesInversionDlg( parent, aSel );
      else
        new SMESHGUI_UnionOfTwoTrianglesDlg( parent, aSel );
      break;
    }
    case 409: // Change orientation
    case 410: // Union of triangles
    case 411: // Cutting of quadrangles
    {
      if ( myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK )
      {
        QAD_MessageBox::warn1( GetDesktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      SALOME_Selection *aSel =
        SALOME_Selection::Selection( myActiveStudy->getSelection() );

      EmitSignalDeactivateDialog();
      SMESHGUI_MultiEditDlg* aDlg;
      if ( theCommandID == 409 )
        aDlg = new SMESHGUI_ChangeOrientationDlg( parent, aSel );
      else if ( theCommandID == 410 )
        aDlg = new SMESHGUI_UnionOfTrianglesDlg( parent, aSel );
      else
        aDlg = new SMESHGUI_CuttingOfQuadsDlg( parent, aSel );

      int x, y ;
      DefineDlgPosition( aDlg, x, y );
      aDlg->move( x, y );
      aDlg->show();
      break;
    }
  case 412: // Smoothing
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_SmoothingDlg *aDlg = new SMESHGUI_SmoothingDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 413: // Extrusion
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_ExtrusionDlg *aDlg = new SMESHGUI_ExtrusionDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 414: // Revolution
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_RevolutionDlg *aDlg = new SMESHGUI_RevolutionDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 415: // Pattern mapping
    {
      if ( checkLock( aStudy ) )
        break;
      if ( myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK )
      {
        EmitSignalDeactivateDialog();
        SALOME_Selection *Sel = SALOME_Selection::Selection(
          myActiveStudy->getSelection() );
        new SMESHGUI_MeshPatternDlg( parent, Sel );
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 801:                                     // CREATE GROUP
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_nil();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();
      if (nbSel == 1) {
	// check if mesh is selected
        aMesh = SMESH::GetMeshByIO(Sel->firstIObject());
      }
      SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg(parent, "", Sel, aMesh);
      aDlg->show();
      break;
    }

  case 802:                                     // CONSTRUCT GROUP
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();
      if (nbSel == 1) {
	// check if submesh is selected
	Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	if (IObject->hasEntry()) {
	  SALOMEDS::SObject_var aSObj = aStudy->FindObjectID(IObject->getEntry());
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
		if (aNodes->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::NODE, aName);
		  aGroup->Add(aNodes.inout());
		}
		if (aEdges->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::EDGE, aName);
		  aGroup->Add(aEdges.inout());
		}
		if (aFaces->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::FACE, aName);
		  aGroup->Add(aFaces.inout());
		}
		if (aVolumes->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::VOLUME, aName);
		  aGroup->Add(aVolumes.inout());
		}
		myActiveStudy->updateObjBrowser(true);
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
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      SALOME_ListIO IOs; IOs = Sel->StoredIObjects(); // list copy
      SALOME_ListIteratorOfListIO It (IOs);
      int nbSelectedGroups = 0;
      for ( ; It.More(); It.Next() )
      {
        SMESH::SMESH_Group_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_Group>(It.Value());
        if (!aGroup->_is_nil()) {
	  nbSelectedGroups++;
          SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg(parent, "", Sel, aGroup);
          aDlg->show();
	}
      }
      if (nbSelectedGroups == 0)
	{
	  SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg(parent, "", Sel, SMESH::SMESH_Group::_nil());
	  aDlg->show();
	}
      break;
    }

  case 804:                                     // Add elements to group
    {
      if(checkLock(aStudy)) break;
      if (myState == 800) {
	SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) myActiveDialogBox;
	if (aDlg) aDlg->onAdd();
      }
      break;
    }

  case 805:                                     // Remove elements from group
    {
      if(checkLock(aStudy)) break;
      if (myState == 800) {
	SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) myActiveDialogBox;
	if (aDlg) aDlg->onRemove();
      }
      break;
    }

    case 810: // Union Groups
    case 811: // Intersect groups
    case 812: // Cut groups
    {
      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(
        myActiveStudy->getSelection() );

      int aMode;
      if      ( theCommandID == 810 ) aMode = SMESHGUI_GroupOpDlg::UNION;
      else if ( theCommandID == 811 ) aMode = SMESHGUI_GroupOpDlg::INTERSECT;
      else                            aMode = SMESHGUI_GroupOpDlg::CUT;

      ( new SMESHGUI_GroupOpDlg( parent, Sel, aMode ) )->show();
      break;
    }

    case 813: // Delete groups with their contents
    {
      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel = SALOME_Selection::Selection(
        myActiveStudy->getSelection() );

      ( new SMESHGUI_DeleteGroupDlg( parent, Sel ) )->show();
      break;
    }

  case 900:					// MESH INFOS
    {
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel =
	SALOME_Selection::Selection(myActiveStudy->getSelection());
      if ( Sel->IObjectCount() > 1 ) { // a dlg for each IO
        SALOME_ListIO IOs; IOs = Sel->StoredIObjects(); // list copy
        SALOME_ListIteratorOfListIO It (IOs);
        for ( ; It.More(); It.Next() ) {
          Sel->ClearIObjects();
          Sel->AddIObject( It.Value() );
          SMESHGUI_MeshInfosDlg *aDlg = new SMESHGUI_MeshInfosDlg(parent, "", false);
        }
        // restore selection
        Sel->ClearIObjects();
        for (It.Initialize( IOs ) ; It.More(); It.Next() )
          Sel->AddIObject( It.Value() );
      }
      else
        SMESHGUI_MeshInfosDlg *aDlg = new SMESHGUI_MeshInfosDlg(parent, "", false);
      break;
    }

  case 902:					// STANDARD MESH INFOS
    {
      EmitSignalDeactivateDialog();
      SALOME_Selection *Sel =
	SALOME_Selection::Selection(myActiveStudy->getSelection());
      if ( Sel->IObjectCount() > 1 ) { // a dlg for each IO
        SALOME_ListIO IOs; IOs = Sel->StoredIObjects(); // list copy
        SALOME_ListIteratorOfListIO It (IOs);
        for ( ; It.More(); It.Next() ) {
          Sel->ClearIObjects();
          Sel->AddIObject( It.Value() );
          SMESHGUI_StandardMeshInfosDlg *aDlg = new SMESHGUI_StandardMeshInfosDlg(parent, "", false);
        }
        // restore selection
        Sel->ClearIObjects();
        for (It.Initialize( IOs ) ; It.More(); It.Next() )
          Sel->AddIObject( It.Value() );
      }
      else
        SMESHGUI_StandardMeshInfosDlg *aDlg = new SMESHGUI_StandardMeshInfosDlg(parent, "", false);
      break;
    } 
    
  case 1001:					// AUTOMATIC UPDATE PREFERENCES
    {
      parent->menuBar()->setItemChecked(1001, !parent->menuBar()->isItemChecked(1001));
      if (parent->menuBar()->isItemChecked(1001)) {
	QAD_CONFIG->addSetting("SMESH:AutomaticUpdate", "true");
	myAutomaticUpdate = true;
      }
      else {
	QAD_CONFIG->addSetting("SMESH:AutomaticUpdate", "false");
	myAutomaticUpdate = false;
      }
      break;
    }

  case 1003:					// MESH PREFERENCES
    {
      ::SetDisplaySettings();
      break;
    }

  case 1005:
    {
      SMESHGUI_Preferences_ScalarBarDlg::ScalarBarPreferences( parent );
      break;
    }

  case 1007:
    {
      ( new SMESHGUI_PrecisionDlg( parent ) )->exec();
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
	SMESH::UpdateSelectionProp();

	QAD_StudyFrame* studyFrame = myActiveStudy->getActiveStudyFrame();
	if (studyFrame->getTypeView() == VIEW_VTK) {
	  VTKViewer_ViewFrame* aViewFrame = SMESH::GetVtkViewFrame(studyFrame);
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
        SALOME_Selection::Selection(myActiveStudy->getSelection());
      int nbSel = Sel->IObjectCount();

      if (nbSel == 1) {
	SMESH::SMESH_Hypothesis_var Hyp = SMESH::IObjectToInterface<SMESH::SMESH_Hypothesis>(Sel->firstIObject());

        /* Look for all mesh objects that have this hypothesis affected in order to flag as ModifiedMesh */
        /* At end below '...->updateObjBrowser(true)' will change icon of mesh objects                   */
        /* Warning : however by internal mechanism all subMeshes icons are changed !                     */
        if ( !Hyp->_is_nil() )
        {
          char* sName = Hyp->GetName();
          SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator(sName);
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
      if ( checkLock( aStudy ) )
        break;
        
      SALOME_Selection *Sel =
        SALOME_Selection::Selection( myActiveStudy->getSelection() );
        
      SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
      for ( ; It.More(); It.Next() )
      {
        Handle(SALOME_InteractiveObject) IObject = It.Value();
        SALOMEDS::SObject_var obj = aStudy->FindObjectID( IObject->getEntry() );
        SALOMEDS::GenericAttribute_var anAttr;
        SALOMEDS::AttributeName_var aName;
        if ( !obj->_is_nil() )
        {
          if ( obj->FindAttribute(anAttr, "AttributeName") )
          {
            aName = SALOMEDS::AttributeName::_narrow( anAttr );
            QString newName = QString(aName->Value());
            newName = SALOMEGUI_NameDlg::getName( QAD_Application::getDesktop(), newName );
            if ( !newName.isEmpty() )
            {
              myActiveStudy->renameIObject( IObject, newName );

              // if current object is group update group's name
	      SMESH::SMESH_GroupBase_var aGroup =
                SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IObject);
              if (!aGroup->_is_nil() )
                aGroup->SetName( newName.latin1() );
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
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
      for (int i = 0; It.More(); It.Next(), i++) {
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	SMESH::RemoveHypothesisOrAlgorithmOnMesh(IObject);
      }
      Sel->ClearIObjects();
      myActiveStudy->updateObjBrowser(true);
      break;
    }

  case 401:					// GEOM::EDGE
  case 4021:					// TRIANGLE
  case 4022:					// QUAD
  case 4031:					// TETRA
  case 4032:					// HEXA
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
        SMDSAbs_ElementType type    = SMDSAbs_Edge;
        int                 nbNodes = 2;
        switch (theCommandID) {
        case 4021:                                      // TRIANGLE
          type = SMDSAbs_Face; nbNodes = 3; break;
        case 4022:                                      // QUAD
          type = SMDSAbs_Face; nbNodes = 4; break;
        case 4031:                                      // TETRA
          type = SMDSAbs_Volume; nbNodes = 4; break;
        case 4032:                                      // HEXA
          type = SMDSAbs_Volume; nbNodes = 8; break;
        default:;
        }
	SMESHGUI_AddMeshElementDlg *aDlg =
          new SMESHGUI_AddMeshElementDlg(parent, "", Sel, type, nbNodes);
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
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
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
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
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
  case 4051:					// RENUMBERING NODES
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_RenumberingDlg *aDlg = new SMESHGUI_RenumberingDlg(parent, "", Sel, 0);
      }
      else
	{
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }
  case 4052:					// RENUMBERING ELEMENTS
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_RenumberingDlg *aDlg = new SMESHGUI_RenumberingDlg(parent, "", Sel, 1);
      }
      else
	{
	  QAD_MessageBox::warn1(QAD_Application::getDesktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }
  case 4061:                                   // TRANSLATION
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_TranslationDlg *aDlg = new SMESHGUI_TranslationDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4062:                                   // ROTATION
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_RotationDlg *aDlg = new SMESHGUI_RotationDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4063:                                   // SYMMETRY 
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_SymmetryDlg *aDlg = new SMESHGUI_SymmetryDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4064:                                   // SEWING
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_SewingDlg *aDlg = new SMESHGUI_SewingDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4065:                                   // MERGE NODES
    {
      if(checkLock(aStudy)) break;
      if (myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	EmitSignalDeactivateDialog();
	SALOME_Selection *Sel =	SALOME_Selection::Selection(myActiveStudy->getSelection());
	SMESHGUI_MergeNodesDlg *aDlg = new SMESHGUI_MergeNodesDlg(parent, "", Sel);
      }
      else {
	QAD_MessageBox::warn1(QAD_Application::getDesktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
    

  case 5000: // HYPOTHESIS
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SMESHGUI_CreateHypothesesDlg *aDlg =
        new SMESHGUI_CreateHypothesesDlg (parent, "", FALSE, false);
      break;
    }
  case 5010: // ALGO
    {
      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SMESHGUI_CreateHypothesesDlg *aDlg =
        new SMESHGUI_CreateHypothesesDlg (parent, "", FALSE, true);
      break;
    }

  case 5105: // Library of selection filters
  {
    static QValueList<int> aTypes;
    if ( aTypes.isEmpty() )
    {
      aTypes.append( SMESH::NODE );
      aTypes.append( SMESH::EDGE );
      aTypes.append( SMESH::FACE );
      aTypes.append( SMESH::VOLUME );
    }
    new SMESHGUI_FilterLibraryDlg( parent, aTypes, SMESHGUI_FilterLibraryDlg::EDIT );
  }
  break;

  case 6016:					// CONTROLS 
  case 6015:
  case 6014:
  case 6013:
  case 6012:
  case 6011:
  case 6001:
  case 6002:
  case 6003:
  case 6004:    
    if ( myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      if ( Sel->IObjectCount() == 1 && Sel->firstIObject()->hasEntry() ) {
	SALOMEDS::SObject_var SO = aStudy->FindObjectID( Sel->firstIObject()->getEntry() );
	if ( !SO->_is_nil() ) {
	  CORBA::Object_var aObject = SO->GetObject();
	  SMESH::SMESH_Mesh_var      aMesh    = SMESH::SMESH_Mesh::_narrow( aObject );
	  SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow( aObject );
	  SMESH::SMESH_GroupBase_var aGroup   = SMESH::SMESH_GroupBase::_narrow( aObject );
	  if ( !aMesh->_is_nil() || !aSubMesh->_is_nil() || !aGroup->_is_nil() ) {
	    ::Control( theCommandID );
	    break;
	  }
	}
      }
      QAD_MessageBox::warn1(GetDesktop(), 
			    tr( "SMESH_WRN_WARNING" ),
			    tr( "SMESH_BAD_SELECTION" ), 
			    tr( "SMESH_BUT_OK" ) );
      break;
    }
    else {
      QAD_MessageBox::warn1(GetDesktop(), 
			    tr( "SMESH_WRN_WARNING" ),
			    tr( "NOT_A_VTK_VIEWER" ), 
			    tr( "SMESH_BUT_OK" ) );
    }
    break;
  case 9010:
    {
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      if (Sel->IObjectCount() == 1)	{
	Handle(SALOME_InteractiveObject) anIObject = Sel->firstIObject();
	if(anIObject->hasEntry())
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
	    anActor->SetPointsLabeled( !anActor->GetPointsLabeled() );
	  }
      }
      break;
    }
  case 9011:
    {
      SALOME_Selection *Sel = SALOME_Selection::Selection(myActiveStudy->getSelection());
      if (Sel->IObjectCount() == 1)	{
	Handle(SALOME_InteractiveObject) anIObject = Sel->firstIObject();
	if(anIObject->hasEntry())
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
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

  myActiveStudy->updateObjBrowser(true);
  return true;
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
  SMESHGUI::GetSMESHGUI();

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
    myAutomaticUpdate = true;
  }
  else {
    parent->menuBar()->setItemChecked(1001, false);
    myAutomaticUpdate = false;
  }

  /* Selection */
  SMESH::UpdateSelectionProp();

  /* menus disable */
  parent->menuBar()->setItemEnabled(111, false);	// IMPORT DAT
  //parent->menuBar()->setItemEnabled(112, false);	// IMPORT UNV

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
  //  if ( myStudy->GetProperties()->IsLocked() ) {
  //    theObject = "NothingSelected";
  //    theContext = "NothingSelected";
  //  }
  // NRI

  SALOME_Selection *Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount();
  switch ( nbSel ) {
  case 0:
    theObject = "NothingSelected";
    theContext = "NothingSelected";
    break;
  case 1:
    if ( myState == 800 && Sel->HasIndex( Sel->firstIObject() ) )
      theObject = "Elements";
    else
      theObject = ::CheckTypeObject( Sel->firstIObject() );
    theContext = "";
    break;
  default:
    theObject = ::CheckHomogeneousSelection();
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
  QAD_StudyFrame* studyFrame = myActiveStudy->getActiveStudyFrame();
  
  // get parent component which selected object(s) belongs to
  QString parentComp = ( (SALOMEGUI_Desktop*)parent )->getComponentFromSelection();

  // get selection
  SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
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
      SALOMEDS::SObject_var SO = SMESH::GetActiveStudyDocument()->FindObjectID( IObject->getEntry() );
      // find popup menu's TopLabel item
      int topItem = popup->indexOf( QAD_TopLabel_Popup_ID );
      if ( topItem >= 0 ) {
	if ( theParent == "Viewer" ) {
	  // set bold font for popup menu's TopLabel item (Viewer popup)
	  QFont fnt = popup->font(); fnt.setBold( TRUE );
	  popup->removeItem( QAD_TopLabel_Popup_ID );
	  popup->insertItem( new CustomItem( QString( IObject->getName() ), fnt ), QAD_TopLabel_Popup_ID, topItem );
	}
	else if ( theParent == "ObjectBrowser" ) {
	  // remove popup menu's TopLabel item (Object Browser popup)
	  popup->removeItem( QAD_TopLabel_Popup_ID );
	}
      }

      if ( theObject == "Hypothesis" || theObject == "Algorithm" ) {
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
	GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh( SO );
	if ( aShapeObject->_is_nil() ) {
	  // imported mesh
	  popup->removeItem( 701 ); // Compute
	  popup->removeItem( 705 ); // Edit hypothesis
	  popup->removeItem( 706 ); // ...
	}
        SMESH::SMESH_GroupOnGeom_var aGeomGroup =
          SMESH::SMESH_GroupOnGeom::_narrow( SO->GetObject() );
        if ( !aGeomGroup->_is_nil()  ) // group linked on geometry
	  popup->removeItem( 803 ); // EDIT GROUP
          
	SMESH_Actor* ac = SMESH::FindActorByEntry(IObject->getEntry());
	// if object has actor
	if ( ac && studyFrame->getTypeView() == VIEW_VTK ) {
	  VTKViewer_RenderWindowInteractor* myRenderInter = SMESH::GetCurrentVtkView()->getRWInteractor();
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
	      case SMESH_Actor::eFreeEdges:
		mi->popup()->setItemChecked( 6002, true );
		mi->popup()->removeItem( 201 );
		break;
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
	      if(!aNbEdges){
		mi->popup()->removeItem( 6001 );
		mi->popup()->removeItem( 6003 );
		mi->popup()->removeItem( 6004 );
	      }
	      if(!aNbFaces){
		mi->popup()->removeItem( 6002 );
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
	    popup->removeItem( 1134 );                             // Clipping
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
	    if ( !aSubMeshObj->_is_nil() && ( aSubMeshObj->GetNumberOfNodes(false) > 0 || aSubMeshObj->GetNumberOfElements() > 0 ) )
	      bDisplay = true;
	    SMESH::SMESH_GroupBase_var aGroupObj = SMESH::SMESH_GroupBase::_narrow( anObject );
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
	  popup->removeItem( 1134 );                             // Clipping
	  popup->removeItem( 2000 );                             // Scalar Bar
	}
      }
      else if( theObject!="Component" ) {
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
      QString type = ::CheckHomogeneousSelection();
      if ( type != "Heterogeneous Selection" ) {
	int topItem = popup->indexOf( QAD_TopLabel_Popup_ID );
	if ( topItem >= 0 ) {
	  // set bold font for popup menu's TopLabel item
	  QFont fnt = popup->font(); fnt.setBold( TRUE );
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
void SMESHGUI::BuildPresentation( const Handle(SALOME_InteractiveObject) & theIO,
                                  QAD_ViewFrame* )
{
  if(theIO->hasEntry()){
    QAD_Study* aStudy = SMESHGUI::GetSMESHGUI()->GetActiveStudy();
    QAD_StudyFrame *aStudyFrame = aStudy->getActiveStudyFrame();
    SMESH::UpdateView(aStudyFrame,SMESH::eDisplay,theIO->getEntry());
  }
}

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
