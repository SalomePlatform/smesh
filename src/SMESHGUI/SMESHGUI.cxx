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
//
//
//  File   : SMESHGUI.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI.h"
#include "SMESHGUI_AddHypothesisDlg.h"
#include "SMESHGUI_AddAlgorithmDlg.h"
#include "SMESHGUI_InitMeshDlg.h"
#include "SMESHGUI_LocalLengthDlg.h"
#include "SMESHGUI_NbSegmentsDlg.h"
#include "SMESHGUI_AddSubMeshDlg.h"
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI_RemoveNodesDlg.h"
#include "SMESHGUI_RemoveElementsDlg.h"
#include "SMESHGUI_MeshInfosDlg.h"
#include "SMESHGUI_Preferences_ColorDlg.h"
#include "SMESHGUI_MaxElementAreaDlg.h"
#include "SMESHGUI_MaxElementVolumeDlg.h"
#include "SMESHGUI_Preferences_ScalarBarDlg.h"
#include "SMESHGUI_EditScalarBarDlg.h"
#include "SMESHGUI_aParameterDlg.h"
#include "SMESHGUI_ComputeScalarValue.h"
#include "SMESHGUI_MoveNodesDlg.h"
#include "SMESHGUI_OrientationElementsDlg.h"
#include "SMESHGUI_DiagonalInversionDlg.h"
#include "SMESHGUI_EdgesConnectivityDlg.h"
#include "SMESHGUI_AddFaceDlg.h"
#include "SMESHGUI_AddEdgeDlg.h"
#include "SMESHGUI_AddVolumeDlg.h"
#include "SMESHGUI_EditHypothesesDlg.h"

#include "SMESH_Grid.h"

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
#include "SALOMEGUI_Desktop.h"
#include "SALOMEGUI_NameDlg.h"

#include "OCCViewer_ViewPort.h"
#include "OCCViewer_ViewPort3d.h"
#include "OCCViewer_Viewer3d.h"

#include "GEOM_Client.hxx"
#include "GEOM_InteractiveObject.hxx"

#include "SALOME_NamingService.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_InteractiveObject.hxx"

#include "SALOMEGUI_ImportOperation.h"
#include "SALOMEGUI_QtCatchCorbaException.hxx"
#include "utilities.h"

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
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qlist.h>
#include <qwidget.h> 
#include <qevent.h> 
#include <qradiobutton.h> 

// VTK Includes
#include "VTKViewer_Common.h"
#include "VTKViewer_ViewFrame.h"
#include <vtkLegendBoxActor.h>
#include <vtkFeatureEdges.h>
#include <vtkDoubleArray.h>


// Open CASCADE Includes
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

static GEOM_Client      ShapeReader;
static SMESHGUI*        smeshGUI = 0;
static CORBA::ORB_var   _orb;


//=============================================================================
/*!
 *
 */
//=============================================================================
class CustomItem : public QCustomMenuItem
{
public:
  CustomItem( const QString& s, const QFont& f )
    : string( s ), font( f ){};
  ~CustomItem(){}
  
  void paint( QPainter* p, const QColorGroup& /*cg*/, bool /*act*/, bool /*enabled*/, int x, int y, int w, int h )
  {
    p->setFont ( font );
    p->drawText( x, y, w, h, AlignHCenter | AlignVCenter | ShowPrefix | DontClip, string );
  }
  
  QSize sizeHint()
  {
    return QFontMetrics( font ).size( AlignHCenter | AlignVCenter | ShowPrefix | DontClip,  string );
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
SMESHGUI::SMESHGUI() :
  QObject()
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
SMESHGUI* SMESHGUI::GetSMESHGUI( )
{
  return smeshGUI;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI* SMESHGUI::GetOrCreateSMESHGUI( QAD_Desktop* desktop )
{
  if( smeshGUI == 0 ) {
    setOrb();
    smeshGUI = new SMESHGUI;
    smeshGUI->myActiveDialogBox = 0 ;
    smeshGUI->myState = -1 ;
    smeshGUI->myDesktop = desktop ; 
    smeshGUI->myActiveStudy = desktop->getActiveStudy();

    Engines::Component_var comp = desktop->getEngine("FactoryServer", "SMESH");
    smeshGUI->myComponentMesh = SMESH::SMESH_Gen::_narrow(comp);
    
    Engines::Component_var comp1 = desktop->getEngine("FactoryServer", "GEOM");    
    smeshGUI->myComponentGeom = GEOM::GEOM_Gen::_narrow(comp1);
      
    /* GetCurrentStudy */
    smeshGUI->myStudyId = smeshGUI->myActiveStudy->getStudyId();
    
    smeshGUI->myComponentGeom->GetCurrentStudy( smeshGUI->myStudyId );
    //    smeshGUI->myComponentGeom->NbLabels();
    
    smeshGUI->myStudy = smeshGUI->myActiveStudy->getStudyDocument();
    
    smeshGUI->myStudyAPI = SMESHGUI_StudyAPI(smeshGUI->myStudy, smeshGUI->myComponentMesh);

    smeshGUI->myDocument = new SMESHDS_Document(1);//NBU

    smeshGUI->mySimulationActors = vtkActorCollection::New();
    smeshGUI->mySimulationActors2D = vtkActor2DCollection::New();
  } else {
    /* study may have changed */
    smeshGUI->myActiveStudy = desktop->getActiveStudy();
    smeshGUI->myStudyAPI.Update(smeshGUI->myComponentMesh);
  }
  
  /* Automatic Update */
  if ( desktop->menuBar()->isItemChecked(1001) )
    smeshGUI->myAutomaticUpdate = true;
  else 
    smeshGUI->myAutomaticUpdate = false;

  return smeshGUI;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetState(int aState )
{  this->myState = aState ;  return ; }


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ResetState()
{  this->myState = -1 ;  return ; }


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalDeactivateDialog()
{
  emit this->SignalDeactivateActiveDialog() ;
  return ;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalCloseAllDialogs()
{
  emit this->SignalCloseAllDialogs() ;
  return ;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
QDialog* SMESHGUI::GetActiveDialogBox()
{
  return this->myActiveDialogBox ;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetActiveDialogBox(QDialog* aDlg)
{
  this->myActiveDialogBox = (QDialog*)aDlg ;
  return ;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
QAD_Study* SMESHGUI::GetActiveStudy()
{
  return this->myActiveStudy ;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
SALOMEDS::Study_ptr SMESHGUI::GetStudy()
{
  return SALOMEDS::Study::_narrow( myStudy );
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
QAD_Desktop* SMESHGUI::GetDesktop()
{
  return this->myDesktop ;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
vtkScalarBarActor* SMESHGUI::GetScalarBar()
{
  vtkRenderer* aRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkActor2DCollection* actor2DList = aRenderer->GetActors2D();
  actor2DList->InitTraversal();
  vtkActor2D* aActor2d =  actor2DList->GetNextActor2D();
  while (aActor2d != NULL) {
    if (aActor2d->IsA("vtkScalarBarActor"))
      return vtkScalarBarActor::SafeDownCast(aActor2d);
    actor2DList->GetNextActor2D();
  }
  return NULL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::activeStudyChanged( QAD_Desktop* parent )
{
  MESSAGE ("SMESHGUI::activeStudyChanged init.")
  /* Create or retrieve an object SMESHGUI */
  SMESHGUI::GetOrCreateSMESHGUI(parent); 
  if(smeshGUI != 0) {  
    smeshGUI->EmitSignalCloseAllDialogs() ;
    MESSAGE( "Active study changed : SMESHGUI nullified" << endl ) ;
    smeshGUI = 0 ;
  }

  //smeshGUI->SetSettings( parent ); //DCQ : Pb. Multi-Etudes
  MESSAGE ("SMESHGUI::activeStudyChanged done.")
  return ;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::DefineDlgPosition(QWidget* aDlg, int& x, int& y)
{
  /* Here the position is on the bottom right corner - 10 */
	aDlg->resize(QSize().expandedTo(aDlg->minimumSizeHint()));
  QAD_Desktop* PP = QAD_Application::getDesktop() ;
  x = abs ( PP->x() + PP->size().width()  - aDlg->size().width()  - 10 ) ;
  y = abs ( PP->y() + PP->size().height() - aDlg->size().height() - 10 ) ;
  return true ;  
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EraseSimulationActors()
{
  if ( myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {  //VTK
    vtkRenderer* theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
    vtkRenderWindow *renWin = theRenderer->GetRenderWindow();

    if ( mySimulationActors != NULL ) {

      mySimulationActors->InitTraversal();
      vtkActor *ac = mySimulationActors->GetNextActor();
      while(!(ac==NULL)) {
	theRenderer->RemoveActor(ac);
	ac = mySimulationActors->GetNextActor();
      }
      mySimulationActors->RemoveAllItems();
    }
    if ( mySimulationActors2D != NULL ) {
      mySimulationActors2D->InitTraversal();
      vtkActor2D *ac = mySimulationActors2D->GetNextActor2D();
      while(!(ac==NULL)) {
	theRenderer->RemoveActor2D(ac);
	ac = mySimulationActors2D->GetNextActor2D();
      }
      mySimulationActors2D->RemoveAllItems();
    }
    renWin->Render();
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESHGUI::InitMesh( GEOM::GEOM_Shape_ptr aShape, QString NameMesh )
{
  try {
    if ( !myComponentMesh->_is_nil() && !aShape->_is_nil() ) {
      SMESH::SMESH_Mesh_var aMesh = myComponentMesh->Init( myComponentGeom, myStudyId, aShape);

      if ( !aMesh->_is_nil() ) {
	SALOMEDS::SObject_var SM = myStudyAPI.AddNewMesh ( aMesh );
	myStudyAPI.SetName( SM, NameMesh );
	  
	// GEOM::GEOM_Shape has non-empty StudyShapeId only after AddInStudy operation,
	// not after loading from file, so let's use more reliable way to retrieve SObject
	Standard_CString ShapeIOR = _orb->object_to_string(aShape);
	SALOMEDS::SObject_var SObject = myStudy->FindObjectIOR(ShapeIOR);
	if ( !SObject->_is_nil() && !SM->_is_nil() ) {
	  myStudyAPI.SetShape( SM, SObject );
	}
	return SMESH::SMESH_Mesh::_narrow(aMesh);
      }
    }
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  }
  myActiveStudy->updateObjBrowser(true);
  return SMESH::SMESH_Mesh::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_subMesh_ptr SMESHGUI::AddSubMesh( SMESH::SMESH_Mesh_ptr aMesh, GEOM::GEOM_Shape_ptr aShape, QString NameMesh )
{
  try {
    SMESH::SMESH_subMesh_var aSubMesh = aMesh->GetElementsOnShape(aShape);
    SALOMEDS::SObject_var SO_Mesh = myStudyAPI.FindMesh(aMesh);
    Standard_CString ShapeIOR = _orb->object_to_string(aShape);
    SALOMEDS::SObject_var SO_GeomShape = myStudy->FindObjectIOR(ShapeIOR);
    
    if ( !SO_GeomShape->_is_nil() && !SO_Mesh->_is_nil() && !aSubMesh->_is_nil() && !aMesh->_is_nil() ) {
      SALOMEDS::SObject_var SO = myStudyAPI.AddSubMeshOnShape(SO_Mesh, SO_GeomShape, aSubMesh, aShape->ShapeType() );
      myStudyAPI.SetName(SO, NameMesh);
    
      SMESH_Actor* amesh = SMESH_Actor::New();
      Handle(SALOME_InteractiveObject) IO = new SALOME_InteractiveObject(SO->GetID(),
									 "MESH",
									 strdup(NameMesh));
      amesh->setIO( IO );
      amesh->setName( strdup(NameMesh) );
      DisplayActor(amesh, false);
      return SMESH::SMESH_subMesh::_narrow( aSubMesh );
    }
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  }
  myActiveStudy->updateObjBrowser(true);
  return SMESH::SMESH_subMesh::_nil();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Hypothesis_ptr SMESHGUI::CreateHypothesis( QString TypeHypothesis, 
							QString NameHypothesis )
{
  SMESH::SMESH_Hypothesis_var Hyp;
  try {
    Hyp = myComponentMesh->CreateHypothesis( TypeHypothesis, myStudyId );
    if ( !Hyp->_is_nil() ) {
      SALOMEDS::SObject_var SHyp = myStudyAPI.AddNewHypothesis( Hyp );
      myStudyAPI.SetName( SHyp, NameHypothesis);
    }
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  }

  return SMESH::SMESH_Hypothesis::_narrow( Hyp );
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddHypothesisOnMesh( SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp )
{
  if ( !aMesh->_is_nil() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    SALOMEDS::SObject_var SM = myStudyAPI.FindMesh( aMesh );
    GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh( SM );
    try {
      bool res = aMesh->AddHypothesis( aShape, aHyp );
      if (res) {
	SALOMEDS::SObject_var SH = myStudyAPI.FindHypothesisOrAlgorithms( aHyp );
	if ( !SM->_is_nil() && !SH->_is_nil() ) {
	  myStudyAPI.SetHypothesis( SM, SH );
	  myStudyAPI.ModifiedMesh( SM, false );
	}
	QApplication::restoreOverrideCursor();
      } else {
	QApplication::restoreOverrideCursor();
	QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				tr ("SMESH_WRN_HYPOTHESIS_ALREADYEXIST"), tr ("SMESH_BUT_YES") );
      }
    }
    catch  (const SALOME::SALOME_Exception& S_ex) {
      QtCatchCorbaException(S_ex);
    }
  }
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveHypothesisOrAlgorithmOnMesh( const Handle(SALOME_InteractiveObject)& IObject )
{
  if ( IObject->hasReference() ) {
    SMESH::SMESH_Hypothesis_var anHyp;
    SALOMEDS::SObject_var SO_Hypothesis = smeshGUI->myStudy->FindObjectID(IObject->getEntry() );
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var     anIOR;

    if ( !SO_Hypothesis->_is_nil() ) {
      QApplication::setOverrideCursor( Qt::waitCursor );
      if (SO_Hypothesis->FindAttribute(anAttr, "AttributeIOR")) {
        anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	anHyp = SMESH::SMESH_Hypothesis::_narrow( _orb->string_to_object(anIOR->Value()) );
      }

      SALOMEDS::SObject_var SO_Applied_Hypothesis = smeshGUI->myStudy->FindObjectID(IObject->getReference());
      if ( !SO_Applied_Hypothesis->_is_nil() ) {
	SALOMEDS::SObject_var MorSM = smeshGUI->myStudyAPI.GetMeshOrSubmesh( SO_Applied_Hypothesis );
	if ( !MorSM->_is_nil() ) {
	  smeshGUI->myStudyAPI.ModifiedMesh( MorSM, false );

	  GEOM::GEOM_Shape_var aShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh( MorSM );
	  if ( !aShape->_is_nil() ) {
	    if (MorSM->FindAttribute(anAttr, "AttributeIOR")) {
              anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	      SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	      if ( !aMesh->_is_nil() ) {
		bool res = aMesh->RemoveHypothesis( aShape, anHyp );
		if ( !res ) {
		  QApplication::restoreOverrideCursor();
		  QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
					  tr ("SMESH_WRN_HYPOTHESIS_NOTEXIST"), tr ("SMESH_BUT_YES") );
  		}
	      } else if ( !aSubMesh->_is_nil() ) {
		aMesh = aSubMesh->GetFather();
		if ( !aMesh->_is_nil() ) {
		  bool res = aMesh->RemoveHypothesis( aShape, anHyp );
		  if ( !res ) {
		    QApplication::restoreOverrideCursor();
  		    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
  					    tr ("SMESH_WRN_HYPOTHESIS_NOTEXIST"), tr ("SMESH_BUT_YES") );
  		  }
		}
	      }
	      if ( myAutomaticUpdate ) {
		SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
		if ( Mesh != NULL ) {
#ifdef TRACE
		  Dump( Mesh );
#endif
		  DisplayActor( Mesh );
		  DisplayEdges( Mesh ); 
		  smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
		}
	      }
	    }
	  }
	}
	smeshGUI->myStudyAPI.UnSetHypothesis( SO_Applied_Hypothesis );
      }
    }    
  } else if ( IObject->hasEntry() ) {
    MESSAGE ( "IObject entry " << IObject->getEntry() )    
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveHypothesisOrAlgorithmOnMesh( SALOMEDS::SObject_ptr MorSM, SMESH::SMESH_Hypothesis_ptr anHyp )
{
  SALOMEDS::SObject_var             AHR, aRef;
  SALOMEDS::GenericAttribute_var    anAttr;
  SALOMEDS::AttributeIOR_var        anIOR;

  if ( !MorSM->_is_nil() ) {
    GEOM::GEOM_Shape_var aShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh( MorSM );
    if ( !aShape->_is_nil() ) {
      if (MorSM->FindAttribute(anAttr, "AttributeIOR")) {
	anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	if ( !aMesh->_is_nil() ) {
	  bool res = aMesh->RemoveHypothesis( aShape, anHyp );
	  if ( !res ) {
	    QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				    tr ("SMESH_WRN_HYPOTHESIS_NOTEXIST"), tr ("SMESH_BUT_YES") );
	  }
	} else if ( !aSubMesh->_is_nil() ) {
	  aMesh = aSubMesh->GetFather();
	  if ( !aMesh->_is_nil() ) {
	    bool res = aMesh->RemoveHypothesis( aShape, anHyp );
	    if ( !res ) {
	      QApplication::restoreOverrideCursor();
	      QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				      tr ("SMESH_WRN_HYPOTHESIS_NOTEXIST"), tr ("SMESH_BUT_YES") );
	    }
	  }
	}
	if ( myAutomaticUpdate ) {
	  SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
	  if ( Mesh != NULL ) {
#ifdef TRACE
	    Dump( Mesh );
#endif
	    DisplayActor( Mesh );
	    DisplayEdges( Mesh ); 
	    // smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
	  }
	}
      }
    }

    if ( MorSM->FindSubObject (2, AHR)) {
      SALOMEDS::ChildIterator_var it = myStudy->NewChildIterator(AHR);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var Obj = it->Value();
	if ( Obj->ReferencedObject(aRef) ) {
	  if (aRef->FindAttribute(anAttr, "AttributeIOR")) {
	    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    if ( strcmp( anIOR->Value(), _orb->object_to_string(anHyp) ) == 0 ) {
	      smeshGUI->myStudyAPI.UnSetHypothesis( Obj );
	      break;
	    }
	  }
	}
      }
    }
    if ( MorSM->FindSubObject (3, AHR)) {
      SALOMEDS::ChildIterator_var it = myStudy->NewChildIterator(AHR);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var Obj = it->Value();
	if ( Obj->ReferencedObject(aRef) ) {
	  if (aRef->FindAttribute(anAttr, "AttributeIOR")) {
	    anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	    if ( strcmp( anIOR->Value(), _orb->object_to_string(anHyp) ) == 0 ) {
	      smeshGUI->myStudyAPI.UnSetAlgorithm( Obj );
	      break;
	    }
	  }
	}
      }
    }    
   smeshGUI->myStudyAPI.ModifiedMesh( MorSM, false );
  } 
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddAlgorithmOnMesh( SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp )
{
  if ( !aMesh->_is_nil() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    SALOMEDS::SObject_var SM = myStudyAPI.FindMesh( aMesh );
    GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh( SM );
    try {
      bool res = aMesh->AddHypothesis( aShape, aHyp );
      if (res) {
	SALOMEDS::SObject_var SH = myStudyAPI.FindHypothesisOrAlgorithms( aHyp );
	if ( !SM->_is_nil() && !SH->_is_nil() ) { 
	  myStudyAPI.SetAlgorithms( SM, SH );    
	  myStudyAPI.ModifiedMesh( SM, false );
	}
      } else {
	QApplication::restoreOverrideCursor();
	QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				tr ("SMESH_WRN_ALGORITHM_ALREADYEXIST"), tr ("SMESH_BUT_YES") );
      }
    }
    catch  (const SALOME::SALOME_Exception& S_ex) {
      QtCatchCorbaException(S_ex);
    }
  }
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddHypothesisOnSubMesh( SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp )
{
  if ( !aSubMesh->_is_nil() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    try {
      SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
      SALOMEDS::SObject_var SsubM = myStudyAPI.FindSubMesh( aSubMesh );
      GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh( SsubM );
      if ( !aMesh->_is_nil() ) {
	bool res = aMesh->AddHypothesis( aShape, aHyp );
	if (res) {      
	  SALOMEDS::SObject_var SH = myStudyAPI.FindHypothesisOrAlgorithms( aHyp );
	  if ( !SsubM->_is_nil() && !SH->_is_nil() ) {
	    myStudyAPI.SetHypothesis( SsubM, SH );
	    myStudyAPI.ModifiedMesh( SsubM, false );
	  }
	} else {
	  QApplication::restoreOverrideCursor();
	  QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				  tr ("SMESH_WRN_HYPOTHESIS_ALREADYEXIST"), tr ("SMESH_BUT_YES") );
	}
      }
    }
    catch  (const SALOME::SALOME_Exception& S_ex) {
      QtCatchCorbaException(S_ex);
    }
  }
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddAlgorithmOnSubMesh( SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp )
{
  if ( !aSubMesh->_is_nil() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    try {
      SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
      SALOMEDS::SObject_var SsubM = myStudyAPI.FindSubMesh( aSubMesh );
      GEOM::GEOM_Shape_var aShape = myStudyAPI.GetShapeOnMeshOrSubMesh( SsubM );
      if ( !aMesh->_is_nil() ) {
	bool res = aMesh->AddHypothesis( aShape, aHyp );
	if (res) {
	  SALOMEDS::SObject_var SH = myStudyAPI.FindHypothesisOrAlgorithms( aHyp );
	  if ( !SsubM->_is_nil() && !SH->_is_nil() ) {
	    myStudyAPI.SetAlgorithms( SsubM, SH ); 
	    myStudyAPI.ModifiedMesh( SsubM, false );
	  }
	} else {
	  QApplication::restoreOverrideCursor();
	  QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				  tr ("SMESH_WRN_ALGORITHM_ALREADYEXIST"), tr ("SMESH_BUT_YES") );
	}
      }
    }
    catch  (const SALOME::SALOME_Exception& S_ex) {
      QtCatchCorbaException(S_ex);
    }
  }
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::CreateAlgorithm( QString TypeAlgo, QString NameAlgo )
{
  SMESH::SMESH_Hypothesis_var Hyp;
  try {
    if ( TypeAlgo.compare("Regular_1D") == 0 )
      Hyp = myComponentMesh->CreateHypothesis( TypeAlgo, myStudyId );
    else if ( TypeAlgo.compare("MEFISTO_2D") == 0 )
      Hyp = myComponentMesh->CreateHypothesis( TypeAlgo, myStudyId );
    else if ( TypeAlgo.compare("Quadrangle_2D") == 0 )
      Hyp = myComponentMesh->CreateHypothesis( TypeAlgo, myStudyId );
    else if ( TypeAlgo.compare("Hexa_3D") == 0 )
      Hyp = myComponentMesh->CreateHypothesis( TypeAlgo, myStudyId );
    else if ( TypeAlgo.compare("NETGEN_3D") == 0 )
      Hyp = myComponentMesh->CreateHypothesis( TypeAlgo, myStudyId );
    
    if ( !Hyp->_is_nil() ) {
      SALOMEDS::SObject_var SHyp = myStudyAPI.AddNewAlgorithms( Hyp );
      myStudyAPI.SetName( SHyp, NameAlgo);
    }
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  }
  myActiveStudy->updateObjBrowser(true);
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::CreateLocalLength( QString TypeHypothesis, QString NameHypothesis, double Length )
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  try {
    SMESH::SMESH_Hypothesis_var Hyp = SMESH::SMESH_Hypothesis::_narrow( CreateHypothesis( TypeHypothesis, NameHypothesis ) );
    SMESH::SMESH_LocalLength_var LL = SMESH::SMESH_LocalLength::_narrow( Hyp );
    if ( !LL->_is_nil() )
      LL->SetLength( Length );
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  }
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::CreateMaxElementArea( QString TypeHypothesis, QString NameHypothesis, double MaxArea )
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  try {
    SMESH::SMESH_Hypothesis_var Hyp = SMESH::SMESH_Hypothesis::_narrow( CreateHypothesis( TypeHypothesis, NameHypothesis  ) );
    SMESH::SMESH_MaxElementArea_var MaxElArea = SMESH::SMESH_MaxElementArea::_narrow( Hyp );
    if ( !MaxElArea->_is_nil() )
      MaxElArea->SetMaxElementArea( MaxArea );
  }
  catch  (SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  } 

  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::CreateMaxElementVolume( QString TypeHypothesis, QString NameHypothesis, double MaxVolume )
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  try {
    SMESH::SMESH_Hypothesis_var Hyp = SMESH::SMESH_Hypothesis::_narrow( CreateHypothesis( TypeHypothesis, NameHypothesis  ) );
    SMESH::SMESH_MaxElementVolume_var MaxElVolume = SMESH::SMESH_MaxElementVolume::_narrow( Hyp );
    if ( !MaxElVolume->_is_nil() )
      MaxElVolume->SetMaxElementVolume( MaxVolume );
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  } 
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::CreateNbSegments( QString TypeHypothesis, QString NameHypothesis, int nbSegments )
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  try {
    SMESH::SMESH_Hypothesis_var Hyp = SMESH::SMESH_Hypothesis::_narrow( CreateHypothesis( TypeHypothesis, NameHypothesis  ) );
    SMESH::SMESH_NumberOfSegments_var NbS = SMESH::SMESH_NumberOfSegments::_narrow( Hyp );
    if ( !NbS->_is_nil() )
      NbS->SetNumberOfSegments( nbSegments );
  }
  catch  (const SALOME::SALOME_Exception& S_ex) {
    QtCatchCorbaException(S_ex);
  } 
  myActiveStudy->updateObjBrowser(true);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedIObjects( SALOME_Selection* Sel, QString& aName )
{
  int nbSel = Sel->IObjectCount() ;
  if ( nbSel == 1 ) {
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    aName = IObject->getName();
  } else {
    aName = tr( "SMESH_OBJECTS_SELECTED" ).arg( nbSel );
  }
  return nbSel;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedNodes( SALOME_Selection* Sel, QString& aName )
{
  int nbNodes = 0;
  int nbSel = Sel->IObjectCount() ;
  if ( nbSel == 1 ) {
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    if ( !IObject->hasEntry() )
      return -1;

    Standard_Boolean res;
    SMESH_Actor* ac = FindActorByEntry( IObject->getEntry(), res, true );
    if ( !res )
      return -1;

    TColStd_MapOfInteger MapIndex;
    Sel->GetIndex( IObject, MapIndex );
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    aName = " ";
    nbNodes = MapIndex.Extent();
    for ( ; ite.More(); ite.Next() ) {
      aName = aName + QString("%1").arg(ac->GetIdSMESHDSNode(ite.Key())) + " ";
    }
  } else {
    aName = "";
  }
  return nbNodes;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedElements( SALOME_Selection* Sel, QString& aName )
{
  int nbElements = 0;
  int nbSel = Sel->IObjectCount() ;
  if ( nbSel == 1 ) {
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    if ( !IObject->hasEntry() )
      return -1;

    Standard_Boolean res;
    SMESH_Actor* ac = FindActorByEntry( IObject->getEntry(), res, true );
    if ( !res )
      return -1;

    TColStd_MapOfInteger MapIndex;
    Sel->GetIndex( IObject, MapIndex );
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    aName = " ";
    nbElements = MapIndex.Extent();
    MESSAGE("GetNameOfSelectedElements(): number = " << nbElements);
    for ( ; ite.More(); ite.Next() ) {
      int idVTK = ite.Key();
      MESSAGE("GetNameOfSelectedElements(): VTK Id = " << idVTK);
      aName = aName + QString("%1").arg(ac->GetIdSMESHDSElement(idVTK)) + " ";
    }
  } else {
    aName = "";
  }
  return nbElements;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
int SMESHGUI::GetNameOfSelectedEdges( SALOME_Selection* Sel, QString& aName )
{
  int nbElements = 0;
  int nbSel = Sel->IObjectCount() ;
  if ( nbSel == 1 ) {
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    if ( !IObject->hasEntry() )
      return -1;

    Standard_Boolean res;
    SMESH_Actor* ac = FindActorByEntry( IObject->getEntry(), res, true );
    if ( !res )
      return -1;

    TColStd_MapOfInteger MapIndex;
    Sel->GetIndex( IObject, MapIndex );
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    aName = " ";
    nbElements = MapIndex.Extent();
    for ( ; ite.More(); ite.Next() ) {
      aName = aName + QString("%1").arg(ite.Key()) + " ";
    }
  } else {
    aName = "";
  }
  return nbElements;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Actor* SMESHGUI::FindActorByEntry(QString entry, 
					Standard_Boolean& testResult,
					bool onlyInActiveView)
{
  int nbSf = myActiveStudy->getStudyFramesCount();
  for ( int i = 0; i < nbSf; i++ ) {
    QAD_StudyFrame* sf = myActiveStudy->getStudyFrame(i);
    if ( sf->getTypeView() == VIEW_VTK ) {
      vtkRenderer* Renderer = ((VTKViewer_ViewFrame*)sf->getRightFrame()->getViewFrame())->getRenderer();
      vtkActorCollection* theActors = Renderer->GetActors();
      theActors->InitTraversal();
      vtkActor *ac = theActors->GetNextActor();
      while(!(ac==NULL)) {
	if ( ac->IsA("SMESH_Actor") ) {
	  SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( ac );
	  if ( anActor->hasIO() ) {
	    Handle(SALOME_InteractiveObject) IO = anActor->getIO();
	    if ( strcmp( IO->getEntry(), entry ) == 0 ) {
	      if ( onlyInActiveView ) {
		if ( sf == myActiveStudy->getActiveStudyFrame() ) {
		  testResult = true;
		  return anActor;
		}
	      } else {
		testResult = true;
		return anActor;
	      }
	    }
	  }
	}
	ac = theActors->GetNextActor();
      }
    }
  }

  MESSAGE ( " Actor Not Found " )
  testResult = false;
  return SMESH_Actor::New();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Actor* SMESHGUI::FindActor(SMESH::SMESH_Mesh_ptr aMesh, 
				 Standard_Boolean& testResult,
				 bool onlyInActiveView)
{
  SALOMEDS::SObject_var SM = myStudyAPI.FindMesh( aMesh );
  if ( SM->_is_nil() ) {
    MESSAGE ( " Actor Not Found " )
    
    testResult = false;
    return SMESH_Actor::New();
  }

  return FindActorByEntry( SM->GetID(), testResult, onlyInActiveView);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESHGUI::ConvertIOinMesh(const Handle(SALOME_InteractiveObject)& IO, 
						Standard_Boolean& testResult)
{
  SMESH::SMESH_Mesh_var aMesh;
  testResult = false ;

  /* case SObject */
  if ( IO->hasEntry() ) {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID( IO->getEntry() );
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var     anIOR;
    if ( !obj->_is_nil() ) {
       if (obj->FindAttribute(anAttr, "AttributeIOR")) {
         anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	 aMesh = SMESH::SMESH_Mesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	 if ( !aMesh->_is_nil() ) {
	   testResult = true ;
	   return SMESH::SMESH_Mesh::_duplicate( aMesh );
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
SMESH::SMESH_subMesh_ptr SMESHGUI::ConvertIOinSubMesh(const Handle(SALOME_InteractiveObject)& IO, 
						      Standard_Boolean& testResult)
{
  SMESH::SMESH_subMesh_var aSubMesh;
  testResult = false ;

  /* case SObject */
  if ( IO->hasEntry() ) {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID( IO->getEntry() );
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var     anIOR;
    if ( !obj->_is_nil() ) {
       if (obj->FindAttribute(anAttr, "AttributeIOR")) {
         anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	 aSubMesh = SMESH::SMESH_subMesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	 if ( !aSubMesh->_is_nil() ) {
	   testResult = true ;
	   return SMESH::SMESH_subMesh::_duplicate( aSubMesh );
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
SMESH::SMESH_Hypothesis_ptr SMESHGUI::ConvertIOinSMESHHypothesis( const Handle(SALOME_InteractiveObject)& IO, 
								  Standard_Boolean& testResult )
{
  SMESH::SMESH_Hypothesis_var aHyp ;
  testResult = false ;

  /* case SObject */
  if ( IO->hasEntry() ) {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID( IO->getEntry() );
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var     anIOR;
    if ( !obj->_is_nil() ) {
       if (obj->FindAttribute(anAttr, "AttributeIOR")) {
         anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	 aHyp = SMESH::SMESH_Hypothesis::_narrow( _orb->string_to_object(anIOR->Value()));
	 if ( !aHyp->_is_nil() ) {
	   testResult = true ;
	   return SMESH::SMESH_Hypothesis::_duplicate( aHyp );
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
GEOM::GEOM_Shape_ptr SMESHGUI::ConvertIOinGEOMShape( const Handle(SALOME_InteractiveObject)& IO, 
					       Standard_Boolean& testResult )
{
  GEOM::GEOM_Shape_var aShape ;
  testResult = false ;

  /* case SObject */
  if ( IO->hasEntry() ) {
    SALOMEDS::SObject_var obj = myStudy->FindObjectID( IO->getEntry() );
    SALOMEDS::GenericAttribute_var anAttr;
    SALOMEDS::AttributeIOR_var     anIOR;
    if ( !obj->_is_nil() ) {
       if (obj->FindAttribute(anAttr, "AttributeIOR")) {
         anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	 aShape = myComponentGeom->GetIORFromString(anIOR->Value()) ;
	 if ( !aShape->_is_nil() ) {
	   testResult = true ;
	   return GEOM::GEOM_Shape::_duplicate( aShape );
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
void SMESHGUI::SetViewMode(int commandId)
{
  SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount();
  if ( nbSel >= 1 ) {
    SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
    for ( ; It.More(); It.Next() ) {
      Handle(SALOME_InteractiveObject) IObject = It.Value();
      if ( IObject->hasEntry() ) {
	Standard_Boolean res;
	SMESH_Actor* ac = FindActorByEntry( IObject->getEntry(), res, true );
	if ( res ) {
	  switch (commandId) {
	  case 211: {
	    ChangeRepresentation( ac, 0 );
	    break;
	  }
	  case 212: {
	    ChangeRepresentation( ac, 1 );
	    break;
	  }
	  case 213: {
	    ChangeRepresentation( ac, 2 );
	    break;
	  }
	  case 1132: {
	    ChangeRepresentation( ac, 3 );
	    break;
	  }
	  }
	}
      }
    }
    if ( commandId == 1133 ) {
      ChangeRepresentation( SMESH_Actor::New(), 4 );
    }
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ChangeRepresentation( SMESH_Actor* ac, int type )
{
  if (ac->DataSource == NULL && type != 4 )
    return;

  if ( type != 4 ) {
    if ( ac->getMapper() == NULL ) {
      return;
    }
  }
  switch (type) {
  case 0 : {
    QApplication::setOverrideCursor( Qt::waitCursor );
    if (ac->getDisplayMode()==2) {
      bool isColored = ac->getMapper()->GetScalarVisibility(); //SAL3899
      vtkDataSetMapper* meshMapper = (vtkDataSetMapper*) (ac->getMapper());
      meshMapper->SetInput(ac->DataSource);
      meshMapper->SetScalarVisibility(isColored); //SAL3899
    }
    ac->setDisplayMode(0);
    ac->GetProperty()->SetRepresentationToWireframe();
    //    ac->SetActorProperty( ac->GetProperty() );
    QApplication::restoreOverrideCursor();
    break;
  }
  case 1 : {
    QApplication::setOverrideCursor( Qt::waitCursor );
    if (ac->getDisplayMode()==2) {
      bool isColored = ac->getMapper()->GetScalarVisibility(); //SAL3899
      vtkDataSetMapper* meshMapper = (vtkDataSetMapper*) (ac->getMapper());
      meshMapper->SetInput(ac->DataSource);
      meshMapper->SetScalarVisibility(isColored); //SAL3899
    }
    ac->setDisplayMode(1);
    ac->GetProperty()->SetRepresentationToSurface();
    QApplication::restoreOverrideCursor();
    //    ac->SetActorProperty( ac->GetProperty() );
    break;
  }
  case 2 : {
    //    if (!(ac->getDisplayMode()==2)) {
    //    ChangeRepresentation(ac, 1);
    QApplication::setOverrideCursor( Qt::waitCursor );
    ac->setDisplayMode(2);
    bool isColored = ac->getMapper()->GetScalarVisibility(); //SAL3899
    vtkDataSetMapper* meshMapper = (vtkDataSetMapper*) (ac->getMapper());
    vtkShrinkFilter *shrink = vtkShrinkFilter::New();
    shrink->SetInput(ac->DataSource);
    shrink->SetShrinkFactor(ac->GetShrinkFactor());
    
    meshMapper->SetInput( shrink->GetOutput() );
    meshMapper->SetScalarVisibility(isColored); //SAL3899
    ac->SetMapper( meshMapper );
    QApplication::restoreOverrideCursor();
    //    }
    break;
  }
  case 3:
    {
      float color[3];
      float edgecolor[3];
      float backfacecolor[3];
      float nodecolor[3];
      ac->GetColor(color[0],color[1],color[2]);
//       QColor c(color[0]*255,color[1]*255,color[2]*255);
      int c0 = int(color[0]*255);
      int c1 = int(color[1]*255);
      int c2 = int(color[2]*255);
      QColor c( c0, c1, c2 );
      ac->GetEdgeColor(edgecolor[0],edgecolor[1],edgecolor[2]);
//       QColor e(edgecolor[0]*255,edgecolor[1]*255,edgecolor[2]*255);
      c0 = int(edgecolor[0]*255);
      c1 = int(edgecolor[1]*255);
      c2 = int(edgecolor[2]*255);
      QColor e( c0, c1, c2 );
      ac->GetBackfaceProperty()->GetColor(backfacecolor);
//       QColor b(backfacecolor[0]*255,backfacecolor[1]*255,backfacecolor[2]*255);
      c0 = int(backfacecolor[0]*255);
      c1 = int(backfacecolor[1]*255);
      c2 = int(backfacecolor[2]*255);
      QColor b( c0, c1, c2 );
      ac->GetNodeColor(nodecolor[0], nodecolor[1], nodecolor[2] ) ;
//       QColor n(nodecolor[0]*255, nodecolor[1]*255, nodecolor[2]*255 ) ;
      c0 = int(nodecolor[0]*255);
      c1 = int(nodecolor[1]*255);
      c2 = int(nodecolor[2]*255);
      QColor n( c0, c1, c2 ) ;

      int Edgewidth = (int)ac->EdgeDevice->GetProperty()->GetLineWidth();
      if ( Edgewidth == 0 )
	Edgewidth = 1;
      int intValue = ac->GetNodeSize() ;
      float Shrink = ac->GetShrinkFactor();
 
      SMESHGUI_Preferences_ColorDlg *aDlg = new SMESHGUI_Preferences_ColorDlg( QAD_Application::getDesktop(), "" ) ;
      aDlg->SetColor(1,c);
      aDlg->SetColor(2,e);
      aDlg->SetColor(3,n);
      aDlg->SetColor(4,b);
      aDlg->SetIntValue(1,Edgewidth);
      aDlg->SetIntValue(2,intValue);
      aDlg->SetIntValue(3,int(Shrink*100.));

      if ( aDlg->exec() ) {
	QApplication::setOverrideCursor( Qt::waitCursor );
	QColor color = aDlg->GetColor(1);
	QColor edgecolor = aDlg->GetColor(2);
	QColor nodecolor = aDlg->GetColor(3);
	QColor backfacecolor = aDlg->GetColor(4);
	/* actor color and backface color */
	ac->GetProperty()->SetColor(float(color.red())/255.,float(color.green())/255.,float(color.blue())/255.);
	ac->SetColor(float(color.red())/255.,float(color.green())/255.,float(color.blue())/255.);
	ac->GetBackfaceProperty()->SetColor(float(backfacecolor.red())/255.,float(backfacecolor.green())/255.,float(backfacecolor.blue())/255.);

	/* edge color */
	ac->EdgeDevice->GetProperty()->SetColor(float(edgecolor.red())/255.,
						float(edgecolor.green())/255.,
						float(edgecolor.blue())/255.);
	ac->EdgeShrinkDevice->GetProperty()->SetColor(float(edgecolor.red())/255.,
						      float(edgecolor.green())/255.,
						      float(edgecolor.blue())/255.);
	ac->SetEdgeColor(float(edgecolor.red())/255.,
			 float(edgecolor.green())/255.,
			 float(edgecolor.blue())/255.);

	/* Shrink factor and size edges */
	ac->SetShrinkFactor( aDlg->GetIntValue(3)/100. );
	ac->EdgeDevice->GetProperty()->SetLineWidth( aDlg->GetIntValue(1) );	
	ac->EdgeShrinkDevice->GetProperty()->SetLineWidth( aDlg->GetIntValue(1) );

	/* Nodes color and size */
	ac->SetNodeColor(float(nodecolor.red())/255.,
			 float(nodecolor.green())/255.,
			 float(nodecolor.blue())/255.);
	ac->SetNodeSize(aDlg->GetIntValue(2)) ;
	
	if (ac->getDisplayMode()==2) {
	  bool isColored = ac->getMapper()->GetScalarVisibility(); //SAL3899
	  vtkDataSetMapper* meshMapper = (vtkDataSetMapper*) (ac->getMapper());
	  meshMapper->SetInput(ac->DataSource);
	  vtkShrinkFilter *shrink = vtkShrinkFilter::New();
	  shrink->SetInput(meshMapper->GetInput());
	  shrink->SetShrinkFactor(ac->GetShrinkFactor());
	  
	  meshMapper->SetInput( shrink->GetOutput() );
	  meshMapper->SetScalarVisibility(isColored); //SAL3899
	  ac->SetMapper( meshMapper );
	}
      }
      delete aDlg;
      QApplication::restoreOverrideCursor();
      break;
    }
  case 4:
    {
      EmitSignalDeactivateDialog() ;
      SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
      SMESHGUI_TransparencyDlg *aDlg = new SMESHGUI_TransparencyDlg( QAD_Application::getDesktop(), "", Sel) ;
      break;
    }
  case 5: {
    QApplication::setOverrideCursor( Qt::waitCursor );
    ac->GetProperty()->SetRepresentationToPoints();
    QApplication::restoreOverrideCursor();
    break;
  }
  }

  QApplication::setOverrideCursor( Qt::waitCursor );
  if ( myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
    vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
    theRenderer->Render();
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::UpdateView()
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  theRenderer->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayActor(SMESH_Actor* ac, bool visibility)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  MESSAGE("DisplayActor(): DataSource = " << ac->DataSource);

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkActorCollection* theActors = theRenderer->GetActors();
  theActors->InitTraversal();
  if ( theActors->IsItemPresent(ac) == 0 ) {
    vtkProperty* prop = vtkProperty::New();
    prop->SetColor( QAD_CONFIG->getSetting("SMESH:SettingsFillColorRed").toFloat()/255., 
		    QAD_CONFIG->getSetting("SMESH:SettingsFillColorGreen").toFloat()/255.,
		    QAD_CONFIG->getSetting("SMESH:SettingsFillColorBlue").toFloat()/255. ) ;

    prop->SetPointSize( QAD_CONFIG->getSetting("SMESH:SettingsNodesSize").toInt());
    prop->SetLineWidth( QAD_CONFIG->getSetting("SMESH:SettingsWidth").toInt() );
    ac->SetProperty( prop );
    ac->SetColor( QAD_CONFIG->getSetting("SMESH:SettingsFillColorRed").toFloat()/255., 
		  QAD_CONFIG->getSetting("SMESH:SettingsFillColorGreen").toFloat()/255.,
		  QAD_CONFIG->getSetting("SMESH:SettingsFillColorBlue").toFloat()/255. );

    //    prop->BackfaceCullingOn();
    vtkProperty* backprop = vtkProperty::New();
    backprop->SetColor( QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorRed").toFloat()/255., 
			QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorGreen").toFloat()/255.,
			QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorBlue").toFloat()/255. ) ;
    ac->SetBackfaceProperty(backprop);

    int intValue = QAD_CONFIG->getSetting("SMESH:SettingsShrinkCoeff").toInt();
    if (intValue == 0 )
      intValue = 80;       
    ac->SetShrinkFactor( intValue/100. );

    ac->GetMapper()->SetResolveCoincidentTopologyToShiftZBuffer();
    ac->GetMapper()->SetResolveCoincidentTopologyZShift(0.02);

    QString DisplayMode = QAD_CONFIG->getSetting("SMESH:DisplayMode");
    if ( DisplayMode.compare("Wireframe") == 0 ){
      ac->setDisplayMode(0);
      ChangeRepresentation( ac, 0 );
    } else if ( DisplayMode.compare("Shading") == 0 ){
      ac->setDisplayMode(1);
      ChangeRepresentation( ac, 1 );
    } else if ( DisplayMode.compare("Shrink") == 0 ) {
      ac->setDisplayMode(2);
      ChangeRepresentation( ac, 2 );
    }
    theRenderer->AddActor(ac);
  } else {
    if (ac->GetMapper())
      ac->GetMapper()->Update();
  }
 
//  if ( visibility )
  ac->SetVisibility(visibility);
//    ac->VisibilityOn();
//  else
//    ac->VisibilityOff();

  vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
  renWin->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EraseActor(SMESH_Actor* ac)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();

  //NRI- : 02/12/2002 : Fixed bugId 882
  //  ac->EdgeDevice->VisibilityOff();
  //  ac->EdgeShrinkDevice->VisibilityOff();
  //  ac->VisibilityOff();
  ac->SetVisibility( false );

  theRenderer->Render();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::AddActorInSelection(SMESH_Actor* ac)
{
  SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
  Sel->ClearIObjects();
  if ( ac->hasIO() )
    return Sel->AddIObject( ac->getIO() );
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
  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  /* copy the list */
  SALOME_ListIO List;
  SALOME_ListIteratorOfListIO Itinit( Sel->StoredIObjects() );
  for (; Itinit.More(); Itinit.Next()) {
    List.Append(Itinit.Value());
  }

  QString RefType = CheckTypeObject( Sel->firstIObject() );
  SALOME_ListIteratorOfListIO It( List );
  for (; It.More(); It.Next()) {
    Handle(SALOME_InteractiveObject) IObject = It.Value();
    QString Type = CheckTypeObject( IObject );
    if ( Type.compare( RefType ) != 0 )
      return "Heterogeneous Selection";
  }

  Sel->ClearIObjects();
  SALOME_ListIteratorOfListIO It1( List );
  for(;It1.More();It1.Next()) {
    int res = Sel->AddIObject( It1.Value() );
    if ( res == -1 )
      myActiveStudy->highlight( It1.Value(), false );
    if ( res == 0 )
      myActiveStudy->highlight( It1.Value(), true );
  }
  return RefType;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QString SMESHGUI::CheckTypeObject(const Handle(SALOME_InteractiveObject)& IO)
{
  SALOMEDS::SObject_var sobj = smeshGUI->myActiveStudy->getStudyDocument()->FindObjectID(IO->getEntry());
  if (!sobj->_is_nil()) {
    SALOMEDS::SComponent_var scomp = sobj->GetFatherComponent();
    if (strcmp(scomp->GetID(), IO->getEntry()) == 0) { // component is selected
      return "Component";
    }
  }

  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  Sel->ClearIObjects();

  Handle(SMESH_TypeFilter) aHypFilter = new SMESH_TypeFilter( HYPOTHESIS );
  Handle(SMESH_TypeFilter) anAlgoFilter = new SMESH_TypeFilter( ALGORITHM );
  Handle(SMESH_TypeFilter) aMeshFilter = new SMESH_TypeFilter( MESH );
  Handle(SMESH_TypeFilter) aSubMeshFilter = new SMESH_TypeFilter( SUBMESH );
  Handle(SMESH_TypeFilter) aMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );
  Handle(SMESH_TypeFilter) aSubMeshVextexFilter = new SMESH_TypeFilter( SUBMESH_VERTEX );
  Handle(SMESH_TypeFilter) aSubMeshEdgeFilter = new SMESH_TypeFilter( SUBMESH_EDGE );
  Handle(SMESH_TypeFilter) aSubMeshFaceFilter = new SMESH_TypeFilter( SUBMESH_FACE );
  Handle(SMESH_TypeFilter) aSubMeshSolidFilter = new SMESH_TypeFilter( SUBMESH_SOLID );
  Handle(SMESH_TypeFilter) aSubMeshCompoundFilter = new SMESH_TypeFilter( SUBMESH_COMPOUND );
  
  Sel->AddFilter(aHypFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "Hypothesis";
  } 

  Sel->ClearFilters();
  Sel->AddFilter(anAlgoFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "Algorithm";
  } 

  Sel->ClearFilters();
  Sel->AddFilter(aMeshFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "Mesh";
  } 

  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "SubMesh";
  } 
 
  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshVextexFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "SubMeshVertex";
  } 
 
  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshEdgeFilter);
  if ( Sel->AddIObject( IO ) != -1 ){
    Sel->ClearFilters();
    return "SubMeshEdge";
  } 
  
  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshFaceFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "SubMeshFace";
  } 
  
  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshSolidFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "SubMeshSolid";
  } 
  
  Sel->ClearFilters();
  Sel->AddFilter(aSubMeshCompoundFilter);
  if ( Sel->AddIObject( IO ) != -1 ) {
    Sel->ClearFilters();
    return "SubMeshCompound";
  } 
  
  Sel->ClearFilters();
  Sel->AddIObject( IO );
  return "NoType";
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnGUIEvent(int theCommandID,	QAD_Desktop* parent)
{
  /* Create or retrieve an object SMESHGUI */
  SMESHGUI::GetOrCreateSMESHGUI(parent);

  // NRI : Temporary added
  if ( smeshGUI->myStudy->GetProperties()->IsLocked() ) {
    return false;
  }
  //NRI

  //  QAD_Viewer3d* v3d;
  OCCViewer_Viewer3d* v3d;
  
  Handle(AIS_InteractiveContext) ic;
  vtkRenderer* Renderer;
  vtkRenderWindow* RenWin;

  if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_OCC ) {
    v3d =((OCCViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getViewer(); 
    ic = v3d->getAISContext();
  } else if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
    Renderer = ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
    RenWin = Renderer->GetRenderWindow();
  }

  switch (theCommandID)
    {
    case 33: // DELETE
      smeshGUI->OnEditDelete();
      break;

    case 113: // IMPORT
    case 112:
    case 111: 
      {
	Import_Document(parent,theCommandID);//NBU
	//Import_Mesh(parent,theCommandID);
	break;
      }

    case 122: // EXPORT MED
    case 121:
    case 123:
      {
	Export_Mesh(parent, theCommandID);
	break;
      }

    case 200: // SCALAR BAR
      {
	smeshGUI->DisplayScalarBar(false);
	break;
      }
    case 201:
      {
	SMESHGUI_EditScalarBarDlg *aDlg = new SMESHGUI_EditScalarBarDlg( parent, "", false ) ;
	aDlg->show();
	break;
      }
    case 202:
      {
	smeshGUI->DisplayScalarBar( true );
	break;
      }      

    case 1133: // DISPLAY MODE : WireFrame, Surface, Shrink
    case 1132:
    case 213:
    case 212:
    case 211:
      {
	smeshGUI->SetViewMode(theCommandID);
	break;
      }
      
    case 214 : // UPDATE
      {
	smeshGUI->Update();
	break;
      }
 
    case 300 : // ERASE
      {
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	  // VTK
	  SALOME_ListIteratorOfListIO It(Sel->StoredIObjects());
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IOS = It.Value();
	    if ( IOS->hasEntry() ) {
	      Standard_Boolean res;
	      SMESH_Actor *ac = smeshGUI->FindActorByEntry(IOS->getEntry(), res, true );
	      if ( res ) smeshGUI->EraseActor( ac );
	    }
	  }
	}
	Sel->ClearIObjects();
	smeshGUI->myActiveStudy->updateObjBrowser( true );
      }
     
    case 301 : // DISPLAY
      {
	if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
	  // VTK
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  SALOMEDS::SObject_var fatherSF = 
	    smeshGUI->myStudy->FindObjectID(smeshGUI->myActiveStudy->getActiveStudyFrame()->entry());

	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
	  
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IObject = It.Value();
	    if ( IObject->hasEntry() ) {
	      Standard_Boolean res;
	      SMESH_Actor *ac = smeshGUI->FindActorByEntry(IObject->getEntry(), res, true );
	      if ( res ) {
		smeshGUI->DisplayActor( ac, true );
		smeshGUI->DisplayEdges( ac ); 
		smeshGUI->ChangeRepresentation( ac, ac->getDisplayMode() );
	      }
	    }
	  }
	  QApplication::restoreOverrideCursor();
	}
	break;
      }

    case 302 : // DISPLAY ONLY
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  vtkActorCollection* theActors = Renderer->GetActors();
	  theActors->InitTraversal();
	  vtkActor *ac = theActors->GetNextActor();
	  while(!(ac==NULL)) {
	    if ( ac->IsA("SMESH_Actor") ) {
	      SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( ac );
	      if(!anActor->isHighlighted()) {
		//anActor->VisibilityOff();
		//NRI- : 02/12/2002 : Fixed bugId 882
		//		anActor->EdgeDevice->VisibilityOff();
		//		anActor->EdgeShrinkDevice->VisibilityOff();
		anActor->SetVisibility( false );
	      }
	    }
	    ac = theActors->GetNextActor();
	  }

	  // Display selection
	  SALOMEDS::SObject_var fatherSF = smeshGUI->myStudy->FindObjectID(smeshGUI->myActiveStudy->getActiveStudyFrame()->entry());
	  
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
	  
	  for(;It.More();It.Next()) {
	    Handle(SALOME_InteractiveObject) IObject = It.Value();
	    SALOMEDS::SObject_var obj = smeshGUI->myStudy->FindObjectID( IObject->getEntry() );
	    
	    VTKViewer_RenderWindowInteractor* myRenderInter= ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRWInteractor();
	    //	    vtkQGLRenderWindowInteractor* myRenderInter= smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getVTKView()->getRWInteractor();
	    
	    if(myRenderInter->isInViewer(IObject)) {
	      if ( IObject->hasEntry() ) {
		Standard_Boolean res;
		SMESH_Actor *ac = smeshGUI->FindActorByEntry(IObject->getEntry(), res, true );
		if ( res ) {
		  smeshGUI->DisplayActor( ac, true );
		  smeshGUI->DisplayEdges( ac ); 
		  smeshGUI->ChangeRepresentation( ac, ac->getDisplayMode() );
		}
	      }
	    }
	  }
	  smeshGUI->myActiveStudy->updateObjBrowser( true );
	  QApplication::restoreOverrideCursor();
	}
	break;
      }

    case 400: // NODES
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_NodesDlg *aDlg = new SMESHGUI_NodesDlg( parent, "", Sel ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }

    case 405: // MOVE NODE
      {
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  SMESHGUI_MoveNodesDlg *aDlg = new SMESHGUI_MoveNodesDlg( parent, "", Sel ) ;
	  break;
      }

    case 701: // COMPUTE MESH 
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  int nbSel = Sel->IObjectCount();
	  if ( nbSel != 1 ) {
	    QApplication::restoreOverrideCursor();
	    break;
	  }

	  SMESH::SMESH_Mesh_var aM;
	  SMESH::SMESH_subMesh_var aSubM;
	  Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
	  if ( IObject->hasEntry() ) {
	    SALOMEDS::SObject_var aMorSM = smeshGUI->myStudy->FindObjectID( IObject->getEntry() );
            SALOMEDS::GenericAttribute_var anAttr;
            SALOMEDS::AttributeIOR_var     anIOR;
	    if ( !aMorSM->_is_nil() ) {
	      if (aMorSM->FindAttribute(anAttr, "AttributeIOR") ) {
                anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
		MESSAGE( "SMESHGUI::OnGUIEvent - Compute mesh : IOR = " << anIOR->Value() )
		CORBA::Object_var cobj;
		try {
		  cobj = _orb->string_to_object(anIOR->Value());
		  if ( CORBA::is_nil(cobj) ) {
		    MESSAGE( "SMESHGUI::OnGUIEvent - Compute mesh : nil object" )
		  }
		}
		catch(CORBA::COMM_FAILURE& ex) {
		  MESSAGE( "SMESHGUI::OnGUIEvent - Compute mesh : exception (1)" )
		}
		aM = SMESH::SMESH_Mesh::_narrow(cobj);
		//aM = SMESH::SMESH_Mesh::_narrow( _orb->string_to_object(anIOR->Value()) );
		aSubM = SMESH::SMESH_subMesh::_narrow( _orb->string_to_object(anIOR->Value()) );
		if ( !aM->_is_nil() ) {
		  GEOM::GEOM_Shape_var refShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh(aMorSM);
		  if ( !refShape->_is_nil() ) {
		    bool compute = smeshGUI->myComponentMesh->IsReadyToCompute(aM, refShape);
		    if ( !compute ) {
		      QApplication::restoreOverrideCursor();
		      QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
					      tr ("SMESH_WRN_MISSING_PARAMETERS"), tr ("SMESH_BUT_YES") );
		      break;
		    }
		    try {
		      smeshGUI->myComponentMesh->Compute( aM, refShape);
		      smeshGUI->myStudyAPI.ModifiedMesh( aMorSM, true );
		      // TO Do : change icon of all submeshes
		    }
		    catch  (const SALOME::SALOME_Exception& S_ex) {
		      QtCatchCorbaException(S_ex);
		    }
		  }
		} else if ( !aSubM->_is_nil() ) {
		  aM = aSubM->GetFather();
		  GEOM::GEOM_Shape_var refShape = smeshGUI->myStudyAPI.GetShapeOnMeshOrSubMesh(aMorSM);
		  if ( !refShape->_is_nil() ) {
		    bool compute = smeshGUI->myComponentMesh->IsReadyToCompute(aM, refShape);
		    if ( !compute ) {
		      QApplication::restoreOverrideCursor();
		      QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
					      tr ("SMESH_WRN_MISSING_PARAMETERS"), tr ("SMESH_BUT_YES") );
		      break;
		    }
		    try {
		      smeshGUI->myComponentMesh->Compute( aM, refShape);
		      smeshGUI->myStudyAPI.ModifiedMesh( aMorSM, true );
		      // TO Do : change icon of all submeshes
		    }
		    catch  (const SALOME::SALOME_Exception& S_ex) {
		      QtCatchCorbaException(S_ex);
		    }
		  }
		}
	      }
	    }
	  }

	  // Check whether the actor for the mesh exists at least in one view
	  Standard_Boolean res;
	  SMESH_Actor* ac = smeshGUI->FindActorByEntry(IObject->getEntry(), res, false);
	  if (!res)
	    smeshGUI->InitActor( aM );
	  else {
	    // Check whether the actor belongs to the active view
	    VTKViewer_RenderWindowInteractor* rwInter = 
	      ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRWInteractor();

	    // The actor belongs to inactive view -> create a copy and add it in the active view
	    if (!rwInter->isInViewer(IObject)) {
	      SMESH_Actor* acCopy = SMESH_Actor::New();
	      acCopy->ShallowCopy(ac);

	      smeshGUI->DisplayActor( acCopy, false );
	    }
	  }

	  if ( smeshGUI->myAutomaticUpdate ) {
	    SMESH_Actor* Mesh = smeshGUI->ReadScript(aM);
	    if ( Mesh != NULL ) {
#ifdef TRACE
	      Dump( Mesh );
#endif
	      smeshGUI->DisplayActor( Mesh, true );
	      smeshGUI->DisplayEdges( Mesh, true );    
	      smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
	    }
	  }
	} else {
	  QApplication::restoreOverrideCursor();
	  QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				  tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	QApplication::restoreOverrideCursor();
	break;
      }

    case 702: // ADD SUB MESH 
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  SMESHGUI_AddSubMeshDlg *aDlg = new SMESHGUI_AddSubMeshDlg( parent, "", Sel ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	  QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }
    
    case 703: // INIT MESH 
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_InitMeshDlg *aDlg = new SMESHGUI_InitMeshDlg( parent, "", Sel ) ;
	break;
      }

    case 704: // EDIT Hypothesis 
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg( parent, "", Sel ) ;
	break;
      }

    case 705: //  EDIT Global Hypothesis
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg( parent, "", Sel ) ;
	break;
      }

    case 706: //  EDIT Local Hypothesis
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_EditHypothesesDlg *aDlg = new SMESHGUI_EditHypothesesDlg( parent, "", Sel ) ;
	break;
      }

    case 806: // ORIENTATION ELEMENTS
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	smeshGUI->myDesktop->SetSelectionMode(3, true);
	SMESHGUI_OrientationElementsDlg *aDlg = new SMESHGUI_OrientationElementsDlg( parent, "", Sel ) ;
	break;
      }

    case 807: // DIAGONAL INVERSION
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	Sel->ClearIObjects();
	smeshGUI->myDesktop->SetSelectionMode(2, true);
	SMESHGUI_DiagonalInversionDlg *aDlg = new SMESHGUI_DiagonalInversionDlg( parent, "", Sel ) ;
	break;
      }
     
    case 900: // MESH INFOS
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_MeshInfosDlg *aDlg = new SMESHGUI_MeshInfosDlg( parent, "", Sel ) ;
	break;
      }
      
    case 1001: // AUTOMATIC UPDATE PREFERENCES
      {
	parent->menuBar()->setItemChecked(1001, !parent->menuBar()->isItemChecked(1001) );
	if ( parent->menuBar()->isItemChecked(1001) ) {
	  QAD_CONFIG->addSetting("SMESH:AutomaticUpdate","true");
	  smeshGUI->myAutomaticUpdate = true;	  
	} else {
	  QAD_CONFIG->addSetting("SMESH:AutomaticUpdate","false");
	  smeshGUI->myAutomaticUpdate = false;
	}
	break;
      }
       
    case 1003: // MESH PREFERENCES
      {
	smeshGUI->SetDisplaySettings();
	break;
      }
      
    case 1005:
      {
	QString Bold = QAD_CONFIG->getSetting("ScalarBar:Bold");
	QString Italic = QAD_CONFIG->getSetting("ScalarBar:Italic");
	QString Shadow = QAD_CONFIG->getSetting("ScalarBar:Shadow");    
	QString FontFamily = QAD_CONFIG->getSetting("ScalarBar:FontFamily");
	QString Orientation = QAD_CONFIG->getSetting("ScalarBar:Orientation");
	float Width = QAD_CONFIG->getSetting("ScalarBar:Width").toFloat();  
	float Height = QAD_CONFIG->getSetting("ScalarBar:Height").toFloat();
	int NumberOfLabels = QAD_CONFIG->getSetting("ScalarBar:NumberOfLabels").toInt();
	int NumberOfColors = QAD_CONFIG->getSetting("ScalarBar:NumberOfColors").toInt();

	if ( Width == 0 ) Width = 0.17;
	if ( Height == 0 ) Height = 0.8;
	if ( NumberOfLabels == 0 ) NumberOfLabels = 5;
	if ( NumberOfColors == 0) NumberOfColors = 64;

	SMESHGUI_Preferences_ScalarBarDlg *aDlg = new SMESHGUI_Preferences_ScalarBarDlg( parent, "", true) ;

	if ( Bold.compare("true") == 0 ) 
	  aDlg->Bold->setChecked(true); else aDlg->Bold->setChecked(false);
	if ( Italic.compare("true") == 0 ) 
	  aDlg->Italic->setChecked(true); else aDlg->Italic->setChecked(false);
	if ( Shadow.compare("true") == 0 ) 
	  aDlg->Shadow->setChecked(true); else aDlg->Shadow->setChecked(false);

	if ( Orientation.compare("Horizontal") == 0 ) 
	  aDlg->RadioHoriz->setChecked(true); else aDlg->RadioVert->setChecked(true);

	int NbItems = aDlg->ComboBox1->count();
	int i = 0;
	aDlg->ComboBox1->setCurrentItem( i );
	while ( i < NbItems ) {
	  if ( FontFamily.compare( aDlg->ComboBox1->text( i ) ) == 0 )
	    aDlg->ComboBox1->setCurrentItem( i );
	  i++;
	}
	
	aDlg->LineEditWidth->setText( QString("%1").arg(Width) );
	aDlg->LineEditHeight->setText( QString("%1").arg(Height) );

	aDlg->SpinBoxLabels->setValue( NumberOfLabels );
	aDlg->SpinBoxColors->setValue( NumberOfColors );
	
	aDlg->show();
	if ( aDlg->result() ) {
	  if ( aDlg->RadioHoriz->isChecked() )
	    Orientation = "Horizontal"; else Orientation = "Vertical";
	  if ( aDlg->Bold->isChecked() )
	    Bold = "true"; else Bold = "false";
	  if ( aDlg->Italic->isChecked() )
	    Italic = "true"; else Italic = "false";
	  if ( aDlg->Shadow->isChecked() )
	    Shadow = "true"; else Shadow = "false";
			
	  FontFamily = aDlg->ComboBox1->currentText();
	  Width = aDlg->LineEditWidth->text().toFloat();
	  Height = aDlg->LineEditHeight->text().toFloat();
	  NumberOfColors = aDlg->SpinBoxColors->text().toInt();
	  NumberOfLabels = aDlg->SpinBoxLabels->text().toInt();

	  
	  vtkScalarBarActor* aScalarBar = smeshGUI->GetScalarBar();
	  if ( aScalarBar != NULL ) {
	    smeshGUI->SetSettingsScalarBar(aScalarBar, Bold, Italic, Shadow, 
					   FontFamily, Orientation,
					   Width, Height, NumberOfColors, NumberOfLabels);
	  }
	  
	  QAD_CONFIG->addSetting("ScalarBar:Bold", Bold);
	  QAD_CONFIG->addSetting("ScalarBar:Italic", Italic);
	  QAD_CONFIG->addSetting("ScalarBar:Shadow", Shadow);    
	  QAD_CONFIG->addSetting("ScalarBar:FontFamily", FontFamily);
	  QAD_CONFIG->addSetting("ScalarBar:Orientation", Orientation);
	  QAD_CONFIG->addSetting("ScalarBar:Width", Width);
	  QAD_CONFIG->addSetting("ScalarBar:Height", Height);
	  QAD_CONFIG->addSetting("ScalarBar:NumberOfLabels", NumberOfLabels);
	  QAD_CONFIG->addSetting("ScalarBar:NumberOfColors", NumberOfColors);
	}
	break;
      }
      
    case 1100: // EDIT HYPOTHESIS
      {
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	int nbSel = Sel->IObjectCount() ;

	if ( nbSel == 1 ) {
	  Standard_Boolean res;
	  SMESH::SMESH_Hypothesis_var Hyp = smeshGUI->ConvertIOinSMESHHypothesis( Sel->firstIObject(), res );

	  /* Look for all mesh objects that have this hupothesis affected in order to flag as ModifiedMesh */
	  /* At end below '...->updateObjBrowser(true)' will change icon of mesh objects                   */
	  /* Warning : however by internal mechanism all subMeshes icons are changed !                     */
	  SALOMEDS::Study::ListOfSObject_var listSOmesh = smeshGUI->GetMeshesUsingAlgoOrHypothesis( Hyp) ;

	  if ( res ) {
	    QString Name = Hyp->GetName();
	    
	    if ( Name.compare("LocalLength") == 0 ) {
	      SMESH::SMESH_LocalLength_var LL = SMESH::SMESH_LocalLength::_narrow( Hyp );
	      double beforeLength = LL->GetLength() ;
	      double Length = smeshGUI->Parameter( res, 
						   beforeLength,
						   tr("SMESH_LOCAL_LENGTH_HYPOTHESIS"), 
						   tr("SMESH_VALUE"), 
						   1.0E-5, 1E6, 6 ) ;
	      if ( res && Length != beforeLength ) {
		LL->SetLength( Length );
		for( int i=0; i<listSOmesh->length(); i++ ) {
		  smeshGUI->GetStudyAPI().ModifiedMesh( listSOmesh[i], false ) ;
		}
		break;
	      }
	      
	    } else if ( Name.compare("NumberOfSegments") == 0 ) {
	      SMESH::SMESH_NumberOfSegments_var NOS = SMESH::SMESH_NumberOfSegments::_narrow( Hyp );
	      int beforeNbSeg = NOS->GetNumberOfSegments() ;
	      int NbSeg = smeshGUI->Parameter( res,
					       beforeNbSeg, 
					       tr("SMESH_NB_SEGMENTS_HYPOTHESIS"), 
					       tr("SMESH_VALUE"), 
					       1, 1000000 ) ;

	      if ( res && NbSeg != beforeNbSeg ) {
		NOS->SetNumberOfSegments( NbSeg );
		for( int i=0; i<listSOmesh->length(); i++ ) {		  
		  SALOMEDS::SObject_var SO = listSOmesh[i] ;
		  smeshGUI->GetStudyAPI().ModifiedMesh( listSOmesh[i], false ) ;
		}
		break;
	      }
	      
	    } else if ( Name.compare("MaxElementArea") == 0 ) {
	      SMESH::SMESH_MaxElementArea_var MEA = SMESH::SMESH_MaxElementArea::_narrow( Hyp );
	      double beforeMaxArea = MEA->GetMaxElementArea() ;
	      double MaxArea = smeshGUI->Parameter( res,
						    beforeMaxArea,
						    tr("SMESH_MAX_ELEMENT_AREA_HYPOTHESIS"), 
						    tr("SMESH_VALUE"), 
						    1.0E-5, 1E6, 6 ) ;
	      if ( res && MaxArea != beforeMaxArea ) {
		MEA->SetMaxElementArea( MaxArea );
		for( int i=0; i<listSOmesh->length(); i++ ) {
		  smeshGUI->GetStudyAPI().ModifiedMesh( listSOmesh[i], false ) ;
		}
		break;
	      }
	      
	    }  else if ( Name.compare("MaxElementVolume") == 0 ) {
	      SMESH::SMESH_MaxElementVolume_var MEV = SMESH::SMESH_MaxElementVolume::_narrow( Hyp );
	      double beforeMaxVolume = MEV->GetMaxElementVolume() ;
	      double MaxVolume = smeshGUI->Parameter( res, 
			 			      beforeMaxVolume,
						      tr("SMESH_MAX_ELEMENT_VOLUME_HYPOTHESIS"), 
						      tr("SMESH_VALUE"), 
						      1.0E-5, 1E6, 6 ) ;
	      if ( res && MaxVolume != beforeMaxVolume ) {
		MEV->SetMaxElementVolume( MaxVolume );
		for( int i=0; i<listSOmesh->length(); i++ ) {
		  smeshGUI->GetStudyAPI().ModifiedMesh( listSOmesh[i], false ) ;
		}
		break;
	      }
	      
	    } else if ( Name.compare("Regular_1D") == 0 ) {
	    } else if ( Name.compare("MEFISTO_2D") == 0 ) {
	    } else {
	    }
	    
	  }
	}
	break;
      }
      
    case 1101: // RENAME
      {
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
	for ( ; It.More(); It.Next() ) {
	  Handle(SALOME_InteractiveObject) IObject = It.Value();

	  SALOMEDS::SObject_var obj = smeshGUI->myStudy->FindObjectID( IObject->getEntry() );
          SALOMEDS::GenericAttribute_var anAttr;
          SALOMEDS::AttributeName_var    aName;
	  if ( !obj->_is_nil() ) {
	    if (obj->FindAttribute(anAttr, "AttributeName")) {
              aName = SALOMEDS::AttributeName::_narrow(anAttr);
	      QString newName = QString( aName->Value() );
	      newName = SALOMEGUI_NameDlg::getName( QAD_Application::getDesktop(), newName);
	      if ( !newName.isEmpty() ) {
		QApplication::setOverrideCursor( Qt::waitCursor );
		smeshGUI->myActiveStudy->renameIObject( IObject, newName );
	      }
	      QApplication::restoreOverrideCursor();
	    }
	  }
	}
	break;
      }  

    case 1102: // REMOVE HYPOTHESIS / ALGORITHMS
      {
	QApplication::setOverrideCursor( Qt::waitCursor );
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
	for ( int i=0; It.More(); It.Next(), i++ ) {
	  Handle(SALOME_InteractiveObject) IObject = It.Value();
	  smeshGUI->RemoveHypothesisOrAlgorithmOnMesh( IObject );
	}
	Sel->ClearIObjects();
	smeshGUI->myActiveStudy->updateObjBrowser( true );
	QApplication::restoreOverrideCursor();
	break;
      }
 
    case 401: // GEOM::EDGE
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_AddEdgeDlg *aDlg = new SMESHGUI_AddEdgeDlg( parent, "", Sel ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }
    case 4021: // TRIANGLE
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_AddFaceDlg *aDlg = new SMESHGUI_AddFaceDlg( parent, "", Sel, 3 ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }
    case 4022: // QUAD
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_AddFaceDlg *aDlg = new SMESHGUI_AddFaceDlg( parent, "", Sel, 4 ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }
    case 4031: // TETRA
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_AddVolumeDlg *aDlg = new SMESHGUI_AddVolumeDlg( parent, "", Sel, 4 ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }
    case 4032: // HEXA
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_AddVolumeDlg *aDlg = new SMESHGUI_AddVolumeDlg( parent, "", Sel, 8 ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }

    case 4041: // REMOVES NODES
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(1, true);
	  parent->menuBar()->setItemChecked(9010, false );
	  parent->menuBar()->setItemChecked(9011, false );
	  smeshGUI->ViewNodes();
	  SMESHGUI_RemoveNodesDlg *aDlg = new SMESHGUI_RemoveNodesDlg( parent, "", Sel ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }
    case 4042: // REMOVES ELEMENTS
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  smeshGUI->EmitSignalDeactivateDialog() ;
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  Sel->ClearIObjects();
	  smeshGUI->myDesktop->SetSelectionMode(3, true);
	  SMESHGUI_RemoveElementsDlg *aDlg = new SMESHGUI_RemoveElementsDlg( parent, "", Sel ) ;	
	} else {
	  QApplication::restoreOverrideCursor();
	    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				   tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
	break;
      }

    case 5000: // HYPOTHESIS - ALGO
      {
	smeshGUI->CreateAlgorithm("Regular_1D","Wire Discretisation");	
	break;
      }
    case 5010: 
      {
	smeshGUI->CreateAlgorithm("MEFISTO_2D","Triangle (Mefisto)");
	break;
      }
    case 5011: 
      {
	smeshGUI->CreateAlgorithm("Quadrangle_2D","Quadrangle (Mapping)");	
	break;
      }
    case 5020: 
      {
	smeshGUI->CreateAlgorithm("Hexa_3D","Hexahedron (i,j,k)");	
	break;
      }
    case 5021: 
      {
	smeshGUI->CreateAlgorithm("NETGEN_3D","Tetrahedron (Netgen)");	
	break;
      }

    case 5030: // HYPOTHESIS - LOCAL LENGTH
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_LocalLengthDlg *aDlg = new SMESHGUI_LocalLengthDlg( parent, "" ) ;
	break;
      }
    case 5031: // HYPOTHESIS - NB SEGMENTS
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_NbSegmentsDlg *aDlg = new SMESHGUI_NbSegmentsDlg( parent, "" ) ;
	break;
      }

    case 5032: // HYPOTHESIS - MAX ELEMENT AREA
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_MaxElementAreaDlg *aDlg = new SMESHGUI_MaxElementAreaDlg( parent, "" ) ;
	break;
      }

    case 5033: // HYPOTHESIS - MAX ELEMENT VOLUME
      {
	smeshGUI->EmitSignalDeactivateDialog() ;
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	SMESHGUI_MaxElementVolumeDlg *aDlg = new SMESHGUI_MaxElementVolumeDlg( parent, "" ) ;
	break;
      }

    case 6016: // CONTROLS 
    case 6015:
    case 6014:
    case 6013:
    case 6012:
    case 6011:
    case 6001:
      {
	SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  	int nbSel = Sel->IObjectCount();
  	if ( nbSel != 1 )
  	  break;
	smeshGUI->Control(theCommandID);
	break;
      }
      
    case 6002:
      {
	if ( smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  SMESHGUI_EdgesConnectivityDlg *Dlg = new SMESHGUI_EdgesConnectivityDlg( parent, "", Sel ) ;
	} else {
	  QApplication::restoreOverrideCursor();
	  QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
				 tr ("SMESH_WRN_VIEWER_VTK"), tr ("SMESH_BUT_YES") );
	}
       	break;
      }

    case 9010:
      {
	if ( !parent->menuBar()->isItemChecked(9010) ) {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  parent->menuBar()->setItemChecked(9011, false );
	  
	  smeshGUI->EraseSimulationActors();
	  smeshGUI->mySimulationActors2D = vtkActor2DCollection::New();

	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  int nbSel = Sel->IObjectCount() ;
	  if ( nbSel == 1 ) {
	    Standard_Boolean res;
	    SMESH_Actor* MeshActor = smeshGUI->FindActorByEntry(Sel->firstIObject()->getEntry(), res, true);
	    if ( res ) {
	      parent->menuBar()->setItemChecked(9010, !parent->menuBar()->isItemChecked(9010) );

	      // It's necessary to display SMDS IDs instead of VTK ones, so 
	      // vtkIdFilter is unacceptable here. We have to do it manually :(
	      vtkUnstructuredGrid* ptGrid = vtkUnstructuredGrid::New();
	      ptGrid->CopyStructure(MeshActor->DataSource);

	      int numPts = MeshActor->DataSource->GetNumberOfPoints();

	      // Loop over points and generate ids
	      vtkIntArray *ptIds = vtkIntArray::New();
	      ptIds->SetNumberOfValues(numPts);

	      for (int id=0; id < numPts; id++) {
		int idSMDS = MeshActor->GetIdSMESHDSNode(id);
		ptIds->SetValue(id, idSMDS);
	      }

// mpv porting vtk4.2.2
	      // 	      vtkScalars* newScalars = vtkScalars::New();
	      // 	      newScalars->SetData(ptIds);
	      // 	      ptGrid->GetPointData()->SetScalars(newScalars);
	      ptGrid->GetPointData()->SetScalars(ptIds);
	      // 	      newScalars->Delete();
// mpv
	      ptIds->Delete();
	      
	      vtkMaskPoints* mask = vtkMaskPoints::New();
	      mask->SetInput(ptGrid);
	      mask->SetOnRatio( 1 );
	      //  	    mask->SetMaximumNumberOfPoints( 50 );
	      //  	    mask->RandomModeOn();
	      
	      vtkSelectVisiblePoints* visPts = vtkSelectVisiblePoints::New();
	      visPts->SetInput(mask->GetOutput());
	      visPts->SetRenderer(((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer());
	      //visPts->SetSelectInvisible(1);
	      visPts->SelectInvisibleOff();
	      visPts->SetTolerance(0.1);
	    
	      vtkLabeledDataMapper* ldm = vtkLabeledDataMapper::New();
	      ldm->SetInput(visPts->GetOutput());
	      ldm->SetLabelFormat("%g");
	      ldm->SetLabelModeToLabelScalars();
	      //ldm->SetLabelModeToLabelFieldData();
	      
	      ldm->SetFontFamilyToTimes();
	      ldm->SetFontSize(6 * parent->font().pointSize() / 5);
	      ldm->SetBold(1);
	      ldm->SetItalic(0);
	      ldm->SetShadow(0);
	      
	      vtkActor2D* pointLabels = vtkActor2D::New();
	      pointLabels->SetMapper(ldm);
	      pointLabels->GetProperty()->SetColor(0,1,0);
	      
	      visPts->Delete();
	      ldm->Delete();
	      smeshGUI->mySimulationActors2D->AddItem( pointLabels );
	      ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer()->AddActor2D( pointLabels );
	    }
	  } 
	} else {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  parent->menuBar()->setItemChecked(9010, !parent->menuBar()->isItemChecked(9010) );
	  smeshGUI->EraseSimulationActors();
	  smeshGUI->ScalarVisibilityOff();
	}
	QApplication::restoreOverrideCursor();
	break;
      }
    case 9011:
      { 
	if ( !parent->menuBar()->isItemChecked(9011) ) {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  parent->menuBar()->setItemChecked(9010, false );

	  smeshGUI->EraseSimulationActors();
	  smeshGUI->mySimulationActors2D = vtkActor2DCollection::New();

	  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
	  int nbSel = Sel->IObjectCount() ;
	  if ( nbSel == 1 ) {
	    Standard_Boolean res;
	    SMESH_Actor* MeshActor = smeshGUI->FindActorByEntry(Sel->firstIObject()->getEntry(), res, true);
	    if ( res ) {
	      parent->menuBar()->setItemChecked(9011, !parent->menuBar()->isItemChecked(9011) );

	      // It's necessary to display SMDS IDs instead of VTK ones, so 
	      // vtkIdFilter is unacceptable here. We have to do it manually :(
	      vtkUnstructuredGrid* elGrid = vtkUnstructuredGrid::New();
	      elGrid->CopyStructure(MeshActor->DataSource);

	      int numCells = MeshActor->DataSource->GetNumberOfCells();

	      // Loop over points and generate ids
	      vtkIntArray *cellIds = vtkIntArray::New();
	      cellIds->SetNumberOfValues(numCells);

	      for (int id=0; id < numCells; id++) {
		int idSMDS = MeshActor->GetIdSMESHDSElement(id);
		cellIds->SetValue(id, idSMDS);
	      }

// mpv porting vk4.2.2
	      // 	      vtkScalars* newScalars = vtkScalars::New();
	      // 	      newScalars->SetData(cellIds);
	      elGrid->GetCellData()->SetScalars(cellIds);
	      // 	      elGrid->GetCellData()->SetScalars(newScalars);
	      // 	      newScalars->Delete();
//mpv

	      cellIds->Delete();
	    
	      vtkCellCenters* cc = vtkCellCenters::New();
	      cc->SetInput(elGrid);

	      vtkSelectVisiblePoints* visCells = vtkSelectVisiblePoints::New();
	      visCells->SetInput(cc->GetOutput());
	      visCells->SetRenderer(((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer());
	      visCells->SelectInvisibleOff();
	      visCells->SetTolerance(0.1);
	      //	    visCells->SetSelectInvisible(1);
	    
	      vtkLabeledDataMapper* ldm = vtkLabeledDataMapper::New();
	      ldm->SetInput(visCells->GetOutput());
	      ldm->SetLabelFormat("%g");
	      ldm->SetLabelModeToLabelScalars();
	    
	      ldm->SetFontFamilyToTimes();
	      ldm->SetFontSize(6 * parent->font().pointSize() / 5);
	      ldm->SetBold(1);
	      ldm->SetItalic(0);
	      ldm->SetShadow(0);
	    
	      vtkActor2D* cellLabels = vtkActor2D::New();
	      cellLabels->SetMapper(ldm);
	      cellLabels->GetProperty()->SetColor(1,0,0);
	    
	      cc->Delete();
	      visCells->Delete();
	      ldm->Delete();
	      smeshGUI->mySimulationActors2D->AddItem( cellLabels );
	      ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer()->AddActor2D( cellLabels );
	    }
	  } 
	} else {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  parent->menuBar()->setItemChecked(9011, !parent->menuBar()->isItemChecked(9011) );
	  smeshGUI->EraseSimulationActors();
	  smeshGUI->ScalarVisibilityOff();
	}
	QApplication::restoreOverrideCursor();
	break;
      }

    case 10001: // DISPLAY MODE PREFERENCE
      {
	parent->menuBar()->setItemChecked(10001, !parent->menuBar()->isItemChecked(10001) );
	parent->menuBar()->setItemChecked(10002, false );
	parent->menuBar()->setItemChecked(10003, false );
	QAD_CONFIG->addSetting("SMESH:DisplayMode", "Wireframe");
	break;
      }
    case 10002:
      {
	parent->menuBar()->setItemChecked(10002, !parent->menuBar()->isItemChecked(10002) );
	parent->menuBar()->setItemChecked(10001, false );
	parent->menuBar()->setItemChecked(10003, false );
	QAD_CONFIG->addSetting("SMESH:DisplayMode", "Shading");
	break;
      }
    case 10003:
      {
	parent->menuBar()->setItemChecked(10003, !parent->menuBar()->isItemChecked(10003) );
	parent->menuBar()->setItemChecked(10002, false );
	parent->menuBar()->setItemChecked(10001, false );
	QAD_CONFIG->addSetting("SMESH:DisplayMode", "Shrink");
	break;
      }

    }
  
  smeshGUI->myActiveStudy->updateObjBrowser(true);
  return true ;
}


//=============================================================================
/*! function : GetMeshesUsingAlgoOrHypothesis()
 *  purpose  : return a list of Study objects (mesh kind) that have 'AlgoOrHyp' affected.
 *           : However is supposed here that father of father of an hypothesis is a Mesh Object.
 */
//=============================================================================      
SALOMEDS::Study::ListOfSObject* SMESHGUI::GetMeshesUsingAlgoOrHypothesis( SMESH::SMESH_Hypothesis_ptr AlgoOrHyp ) 
{
  SALOMEDS::Study::ListOfSObject_var listSOmesh = new SALOMEDS::Study::ListOfSObject ;
  listSOmesh->length(0) ;
  unsigned int index = 0 ;
  if( !AlgoOrHyp->_is_nil() ) {
    SALOMEDS::SObject_var SO_Hypothesis = smeshGUI->GetStudyAPI().FindHypothesisOrAlgorithms( AlgoOrHyp );
    if( !SO_Hypothesis->_is_nil() ) {
      SALOMEDS::Study::ListOfSObject_var listSO = smeshGUI->myStudy->FindDependances(SO_Hypothesis) ;
      for( unsigned int i=0; i<listSO->length(); i++ ) {
	SALOMEDS::SObject_var SO = listSO[i] ;
	if( !SO->_is_nil() ) {
	  SALOMEDS::SObject_var SOfatherFather = SO->GetFather()->GetFather() ;
	  if( !SOfatherFather->_is_nil() ) {
	    index++ ;
	    listSOmesh->length(index) ;
	    listSOmesh[index-1] = SOfatherFather ;
	  }
	}
      }
    }
  }
  return listSOmesh._retn() ;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Import_Mesh(QAD_Desktop* parent, int theCommandID)
{
  QString filter;
  string myExtension;
  Mesh_Reader* myReader;
  
  if (theCommandID==113) {
    filter = tr("MED files (*.med)");
    myExtension = string("MED");
    myReader = new DriverMED_R_SMESHDS_Mesh;
  }
  else if (theCommandID==112) {
    filter = tr("IDEAS files (*.unv)");
    myExtension = string("UNV");
  }
  else if (theCommandID==111) {
    filter = tr("DAT files (*.dat)");
    myExtension = string("MED");
    myReader = new DriverDAT_R_SMESHDS_Mesh;
  }

  QString filename = QAD_FileDlg::getFileName(parent,
					      "",
					      filter,
					      tr("Import mesh"),
					      true);
  if ( !filename.isEmpty() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    string myClass = string("SMESHDS_Mesh");
//    Mesh_Reader* myReader = SMESHDriver::GetMeshReader(myExtension, myClass);
    
    int myMeshId = (smeshGUI->myDocument)->NewMesh();
    SMDS_Mesh* myMesh = (smeshGUI->myDocument)->GetMesh(myMeshId);

    myReader->SetFile(string(filename.latin1()));
    myReader->SetMesh(myMesh);
    myReader->SetMeshId(myMeshId);
    myReader->Read();

    QApplication::restoreOverrideCursor();
  }
}
  
//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Export_Mesh(QAD_Desktop* parent, int theCommandID)
{
  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount() ;
  if ( nbSel == 1 ) {
    Standard_Boolean res;
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    SMESH::SMESH_Mesh_var aMesh = smeshGUI->ConvertIOinMesh( IObject, res );
    if ( res ) {
      if (theCommandID==122) { // EXPORT MED
	QString filename = QAD_FileDlg::getFileName(parent,
						    "",
						    tr("MED files (*.med)"),
						    tr("Export mesh"),
						    false);
	if ( !filename.isEmpty() ) {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  aMesh->ExportMED( filename.latin1() );
	  QApplication::restoreOverrideCursor();
	}
      }
      else if (theCommandID==121) { // EXPORT DAT
	QString filename = QAD_FileDlg::getFileName(parent,
						    "",
						    tr("DAT files (*.dat)"),
						    tr("Export mesh"),
						    false);
	if ( !filename.isEmpty() ) {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  aMesh->ExportDAT( filename.latin1() );
	  QApplication::restoreOverrideCursor();
	}
      }
      else if (theCommandID==123) { // EXPORT UNV
	QString filename = QAD_FileDlg::getFileName(parent,
						    "",
						    tr("IDEAS files (*.unv)"),
						    tr("Export mesh"),
						    false);
	if ( !filename.isEmpty() ) {
	  QApplication::setOverrideCursor( Qt::waitCursor );
	  aMesh->ExportUNV( filename.latin1() );
	  QApplication::restoreOverrideCursor();
	}
        else 
	  aMesh->ExportDAT( filename.latin1() );

	QApplication::restoreOverrideCursor();

	if ( IObject->hasEntry() )
	  {
	MESSAGE("---");
	    SALOMEDS::SObject_var SO = smeshGUI->myStudy->FindObjectID( IObject->getEntry() );
	    SALOMEDS::GenericAttribute_var anAttr;
	    SALOMEDS::AttributeComment_var    aFileName;
	    SALOMEDS::StudyBuilder_var aStudyBuilder = smeshGUI->myStudy->NewBuilder();
	    anAttr = aStudyBuilder->FindOrCreateAttribute(SO, "AttributeComment");
	    aFileName = SALOMEDS::AttributeComment::_narrow(anAttr);
	    aFileName->SetValue(filename.latin1());
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
void SMESHGUI::Import_Document(QAD_Desktop* parent, int theCommandID)
{
  QString filter;
  string myExtension;
  Document_Reader* myReader;

  if (theCommandID==113) {
    filter = tr("MED files (*.med)");
    myExtension = string("MED");
    myReader = new DriverMED_R_SMESHDS_Document;
  }
  else if (theCommandID==112) {
    filter = tr("IDEAS files (*.unv)");
    myExtension = string("UNV");
    myReader = new DriverUNV_R_SMESHDS_Document;
  }
  else if (theCommandID==111) {
    filter = tr("DAT files (*.dat)");
    myExtension = string("DAT");
    myReader = new DriverDAT_R_SMESHDS_Document;
  }

  QString filename = QAD_FileDlg::getFileName(parent,
					      "",
					      filter,
					      tr("Import document"),
					      true);
  if ( !filename.isEmpty() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    string myClass = string("SMESHDS_Document");
//    Document_Reader* myReader = SMESHDriver::GetDocumentReader(myExtension, myClass);
	SMESHDS_Document* newDocument = new SMESHDS_Document(1);

    myReader->SetFile(string(filename.latin1()));
    myReader->SetDocument(smeshGUI->myDocument);
    myReader->Read();
    QApplication::restoreOverrideCursor();
  }
}
  
void SMESHGUI::Export_Document(QAD_Desktop* parent, int theCommandID)
{
  QString filter;
  Document_Writer* myWriter;
  string myExtension;

  if (theCommandID==122) {
    filter = tr("MED files (*.med)");
    myExtension = string("MED");
    myWriter = new DriverMED_W_SMESHDS_Document;
  }
  else if (theCommandID==121) {
    filter = tr("DAT files (*.dat)");
    myExtension = string("DAT");
    myWriter = new DriverDAT_W_SMESHDS_Document;
  } else if (theCommandID==123) {
    filter = tr("IDEAS files (*.unv)");
    myExtension = string("UNV");
    myWriter = new DriverUNV_W_SMESHDS_Document;
  }

  QString filename = QAD_FileDlg::getFileName(parent,
					      "",
					      filter,
					      tr("Export document"),
					      false);
  if ( !filename.isEmpty() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    string myClass = string("SMESHDS_Document");
    //Document_Writer* myWriter = SMESHDriver::GetDocumentWriter(myExtension, myClass);

    myWriter->SetFile(string(filename.latin1()));
    myWriter->SetDocument(smeshGUI->myDocument);

    //StudyContextStruct* myStudyContext = _impl.GetStudyContext(myStudyId);
    //Handle(SMESHDS_Document) myDocument = myStudyContext->myDocument;
    //myWriter->SetDocument(myDocument);

    myWriter->Write();
    QApplication::restoreOverrideCursor();
  }
}
  
//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnMousePress(QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame)
{
  SMESHGUI::GetOrCreateSMESHGUI(parent);
  return false ;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnMouseMove (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame)
{
  SMESHGUI::GetOrCreateSMESHGUI(parent);

  return true;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnKeyPress (QKeyEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame)
{
  SMESHGUI::GetOrCreateSMESHGUI(parent);

  return true ;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::SetSettings( QAD_Desktop* parent )
{
  SMESHGUI::GetOrCreateSMESHGUI(parent);

  /* Display mode */
  QString DisplayMode = QAD_CONFIG->getSetting("SMESH:DisplayMode");
  if ( DisplayMode.compare("") == 0 ) {
    DisplayMode = "Shading";
    QAD_CONFIG->addSetting("SMESH:DisplayMode","Shading");
  }

  if ( DisplayMode.compare("Wireframe") == 0 ){
    parent->menuBar()->setItemChecked(10003, false );
    parent->menuBar()->setItemChecked(10002, false );
    parent->menuBar()->setItemChecked(10001, true );
  } else if ( DisplayMode.compare("Shading") == 0 ){
    parent->menuBar()->setItemChecked(10003, false );
    parent->menuBar()->setItemChecked(10002, true );
    parent->menuBar()->setItemChecked(10001, false );
  } else if ( DisplayMode.compare("Shrink") == 0 ) {
    parent->menuBar()->setItemChecked(10003, true );
    parent->menuBar()->setItemChecked(10002, false );
    parent->menuBar()->setItemChecked(10001, false );
  }

  /* Automatic Update */
  QString AutoUpdate = QAD_CONFIG->getSetting("SMESH:AutomaticUpdate");
  if ( AutoUpdate.compare("true") == 0 ) {
    parent->menuBar()->setItemChecked(1001, true );
    smeshGUI->myAutomaticUpdate = true;
  } else {
    parent->menuBar()->setItemChecked(1001, false );
    smeshGUI->myAutomaticUpdate = false;
  }

  /* menus disable */
  parent->menuBar()->setItemEnabled( 11, false); // IMPORT
   
  return true;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DefinePopup( QString & theContext, QString & theParent, QString & theObject )
{
  /* Create or retrieve an object SMESHGUI */
  SMESHGUI::GetOrCreateSMESHGUI(QAD_Application::getDesktop());

  // NRI : Temporary added
  //  if ( smeshGUI->myStudy->GetProperties()->IsLocked() ) {
  //    theObject = "NothingSelected";
  //    theContext = "NothingSelected";
  //  }
  // NRI

  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount();
  if (nbSel == 0) {
    if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK) {
      vtkScalarBarActor* aScalarBar = smeshGUI->GetScalarBar();
      if ((aScalarBar != NULL) && (aScalarBar->GetVisibility() == 1) ) {
	theObject = "ScalarBar";
	theContext = "";
      } else {
	theObject = "NothingSelected";
	theContext = "NothingSelected";
      }
    } else {
      theObject = "NothingSelected";
      theContext = "NothingSelected";
    }
  } else if (nbSel == 1 ) {
    theObject = smeshGUI->CheckTypeObject( Sel->firstIObject() ); 
    theContext = "";
  } else {
    theObject = smeshGUI->CheckHomogeneousSelection();
    theContext = "";
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::CustomPopup( QAD_Desktop* parent,
			   QPopupMenu* popup,
			   const QString& theContext,
			   const QString& theParent,
			   const QString& theObject )
{
  // Popup should be customized for any viewer since some meaningless commands may be present in the popup
  //if (smeshGUI->myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) //Test du type de viewer true=OCC false=VTK
  //  return false;
  /* Create or retrieve an object SMESHGUI */
  SMESHGUI::GetOrCreateSMESHGUI(parent);

  // NRI : Temporary added
  //  if ( smeshGUI->myStudy->GetProperties()->IsLocked() ) {
  //    return false;
  //  }
  // NRI

  SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
  QAD_StudyFrame* studyFrame = smeshGUI->myActiveStudy->getActiveStudyFrame();
  int nbSel = Sel->IObjectCount();
  
  if (nbSel == 0) { 
    if (studyFrame->getTypeView() != VIEW_VTK)
      popup->clear();
    return false;
  } else if (nbSel == 1 ) {
    QString parentComp = ((SALOMEGUI_Desktop*)parent)->getComponentFromSelection();
    // First check type of active viewer (VTK required)
    if (/*studyFrame->getTypeView() != VIEW_VTK ||*/ parentComp != parent->getActiveComponent())
      {
      //MESSAGE("CustomPopup(): VTK viewer required, removing all SMESH-specific popup menu items")
      while (1) {
	int id = popup->idAt(0);
	if (id <= QAD_TopLabel_Popup_ID)
	  popup->removeItemAt(0);
	else
	  break;
      }
      if (theObject.compare("Component") == 0) {
	popup->removeItem(QAD_DisplayOnly_Popup_ID);
      }
      return false;
    }

    // Remove common popup items for Submesh, Hypothesis and Algorithm
    if (theObject.compare("SubMesh") == 0 ||
	theObject.compare("Hypothesis") == 0 ||
	theObject.compare("Algorithm") == 0 ) {
      popup->removeItem(QAD_Display_Popup_ID);
      popup->removeItem(QAD_DisplayOnly_Popup_ID);
      popup->removeItem(QAD_Erase_Popup_ID);
      int id = popup->idAt(popup->count()-1); // last item
      if (id < 0 && id != -1) popup->removeItem(id); // separator
    }

    if (theObject.compare("Component") == 0) {
      popup->removeItem(QAD_DisplayOnly_Popup_ID);
      return true;
    }

    int id = QAD_TopLabel_Popup_ID;//popup->idAt(0);
    QFont f = QApplication::font();
    f.setBold( TRUE );
	
    Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
    
    if ( theParent.compare("Viewer")==0 ) {
      if (popup->idAt(0) == id) {
	popup->removeItem(id);
	popup->insertItem( new CustomItem ( QString(IObject->getName()), f ), id, 0 );
      }
      
      Standard_Boolean res;
      SMESH_Actor* ac = smeshGUI->FindActorByEntry( IObject->getEntry(), res, false );
      if ( res && studyFrame->getTypeView() == VIEW_VTK) {
	VTKViewer_RenderWindowInteractor* myRenderInter= 
	  ((VTKViewer_ViewFrame*)studyFrame->getRightFrame()->getViewFrame())->getRWInteractor();
	if ( myRenderInter->isVisible( IObject ) ) {
	  popup->removeItem(QAD_Display_Popup_ID);
	} else {
	  popup->removeItem(QAD_Erase_Popup_ID);
	}
      } else {
	popup->removeItem(QAD_Erase_Popup_ID);
	if (!res) {// mesh not computed -> can't display it
	  popup->removeItem(QAD_Display_Popup_ID);
	  popup->removeItem(QAD_DisplayOnly_Popup_ID);
	  popup->removeItemAt(popup->count() - 1);//separator
	}
      }
    } else if ( theParent.compare("ObjectBrowser")==0 ) {
      if (theObject.compare("Mesh") == 0 ||
	  theObject.compare("SubMesh") == 0 ||
	  theObject.compare("Hypothesis") == 0 ||
	  theObject.compare("Algorithm") == 0 ) {
	popup->removeItemAt(0);	
      } else {
	if (popup->idAt(0) == id) {
	  popup->removeItem(id);
	  popup->removeItemAt(0);//separator
	}
      }

      Standard_Boolean res;
      SMESH_Actor* ac = smeshGUI->FindActorByEntry( IObject->getEntry(), res, false );
      if ( res && studyFrame->getTypeView() == VIEW_VTK ) {
	VTKViewer_RenderWindowInteractor* myRenderInter= 
	  ((VTKViewer_ViewFrame*)studyFrame->getRightFrame()->getViewFrame())->getRWInteractor();
	if ( myRenderInter->isVisible( IObject ) ) {
	  popup->removeItem(QAD_Display_Popup_ID);
	} else {
	  popup->removeItem(QAD_Erase_Popup_ID);
	}
      } else {
	if (theObject.compare("Mesh") == 0 ||
	    theObject.compare("SubMesh") == 0 ||
	    theObject.compare("Hypothesis") == 0 ||
	    theObject.compare("Algorithm") == 0 ) {
	} else {
	  popup->removeItem(QAD_Erase_Popup_ID);
	  if (!res) {// mesh not computed -> can't display it
	    popup->removeItem(QAD_Display_Popup_ID);
	    popup->removeItem(QAD_DisplayOnly_Popup_ID);
	    popup->removeItemAt(popup->count() - 1);//separator
	  }
	}
      }
    }
  } else {
    QString parentComp = ((SALOMEGUI_Desktop*)parent)->getComponentFromSelection();
    QAD_StudyFrame* studyFrame = smeshGUI->myActiveStudy->getActiveStudyFrame();
    if (/*studyFrame->getTypeView() != VIEW_VTK ||*/ parentComp != parent->getActiveComponent())
    {
      //MESSAGE("CustomPopup(): VTK viewer required, removing all SMESH-specific popup menu items")
      while (1) {
	int id = popup->idAt(0);
	if (id <= QAD_TopLabel_Popup_ID && id != -1)
	  popup->removeItemAt(0);
	else
	  break;
      }
      if (parentComp.isNull()) { // objects from several components are selected
	popup->removeItem(QAD_DisplayOnly_Popup_ID);	
	popup->removeItem(QAD_Display_Popup_ID);
	popup->removeItem(QAD_Erase_Popup_ID);
	int id = popup->idAt(popup->count()-1); // last item
	if (id < 0 && id != -1) popup->removeItem(id); // separator
      }
      return false;
    }

    QString type =  smeshGUI->CheckHomogeneousSelection();
    if ( type.compare("Heterogeneous Selection") != 0 ) {
      int id = QAD_TopLabel_Popup_ID;//popup->idAt(0);
      QFont f = QApplication::font();
      f.setBold( TRUE );
      popup->removeItem(id);
      popup->insertItem( new CustomItem ( QString("%1 ").arg(nbSel) + type + " (s) ", f), id, 0  );
    }
  }
  return false;
}

//=============================================================================
/*! Method:  BuildPresentation(const Handle(SALOME_InteractiveObject)& theIO)
 *  Purpose: ensures that the actor for the given <theIO> exists in the active VTK view
 */
//=============================================================================
void SMESHGUI::BuildPresentation(const Handle(SALOME_InteractiveObject)& theIO)
{
  /* Create or retrieve an object SMESHGUI */
  SMESHGUI::GetOrCreateSMESHGUI(QAD_Application::getDesktop());
  
  QAD_StudyFrame* activeFrame = smeshGUI->myActiveStudy->getActiveStudyFrame();
  if (activeFrame->getTypeView() == VIEW_VTK) {
    // VTK
    SALOMEDS::SObject_var fatherSF = 
      smeshGUI->myStudy->FindObjectID(activeFrame->entry());
    
    SALOME_Selection* Sel = SALOME_Selection::Selection( smeshGUI->myActiveStudy->getSelection() );
    SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
    
//    for(;It.More();It.Next()) {
//      Handle(SALOME_InteractiveObject) IObject = It.Value();
      Handle(SALOME_InteractiveObject) IObject = theIO;
      if ( IObject->hasEntry() ) {
	// Look for the actor in all views
	Standard_Boolean res;
	SMESH_Actor *ac = smeshGUI->FindActorByEntry(IObject->getEntry(), res, false);

	// Actor not found at all -> mesh is not computed -> do nothing!!!
	if ( !res ) {
	  /*SMESH::SMESH_Mesh_var aM;
	  SALOMEDS::SObject_var aMorSM = smeshGUI->myStudy->FindObjectID( IObject->getEntry() );
	  SALOMEDS::SObject_var father = aMorSM->GetFather();
	  SALOMEDS::SObject_var fatherComp = aMorSM->GetFatherComponent();
	  
	  // Non-displayable objects (Hypo, Algo) have tags < 3 or have a father different from component
	  if (aMorSM->Tag() < 3 || strcmp(father->GetID(), fatherComp->GetID()) != 0)
	    continue;

	  SALOMEDS::GenericAttribute_var anAttr;
	  SALOMEDS::AttributeIOR_var     anIOR;
	  if ( !aMorSM->_is_nil() ) {
	    if (aMorSM->FindAttribute(anAttr, "AttributeIOR") ) {
	      anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
	      aM = SMESH::SMESH_Mesh::_narrow( _orb->string_to_object(anIOR->Value()) );
	    }
	  }

	  if (!aM->_is_nil()) {
	    smeshGUI->InitActor(aM);
	    ac = smeshGUI->ReadScript(aM);
	  }

	  if (ac) {
	    smeshGUI->DisplayActor( ac, true );
	    smeshGUI->DisplayEdges( ac ); 
	    smeshGUI->ChangeRepresentation( ac, ac->getDisplayMode() );
	  }*/
//	  continue;
	} else { // The actor exists in some view
	  // Check whether the actor belongs to the active view
	  VTKViewer_RenderWindowInteractor* rwInter = 
	    ((VTKViewer_ViewFrame*)activeFrame->getRightFrame()->getViewFrame())->getRWInteractor();

	  // The actor belongs to inactive view -> create a copy and display it in the active view
	  if (!rwInter->isInViewer(IObject)) {
	    SMESH_Actor* acCopy = SMESH_Actor::New();
	    acCopy->ShallowCopy(ac);
	    ac = acCopy;
	  }
	  smeshGUI->DisplayActor( ac, false );
	  smeshGUI->DisplayEdges( ac ); 
	  smeshGUI->ChangeRepresentation( ac, ac->getDisplayMode() );
	}
      }
//    }
  } else {
    MESSAGE("BuildPresentation() must not be called while non-VTK view is active")
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::setOrb()
{
  try {
    ORB_INIT &init = *SINGLETON_<ORB_INIT>::Instance();
    ASSERT(SINGLETON_<ORB_INIT>::IsAlreadyExisting());
    _orb = init( 0 , 0 );
  } catch (...) {
    INFOS("internal error : orb not found");
    _orb = 0;
  }
  ASSERT(! CORBA::is_nil(_orb));
}


//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH_Actor* SMESHGUI::ReadScript(SMESH::SMESH_Mesh_ptr aMesh)
{
  SMESH_Actor* MeshActor;
  if ( !aMesh->_is_nil() ) {
    Standard_Boolean result;
    MeshActor = FindActor(aMesh, result, true);
    if ( result ) {
      SMESH::log_array_var aSeq = aMesh->GetLog(true);
      
      if (aSeq->length() == 0) {
	MESSAGE("ReadScript(): log is empty")
	return MeshActor;
      }

      for (unsigned int ind = 0; ind < aSeq->length();ind++) {
	switch ( aSeq[ind].commandType )
	  {
	  case SMESH::ADD_NODE :
	    {
	      AddNodes( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::ADD_EDGE :
	    {
	      //	      AddEdges( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::ADD_TRIANGLE :
	    {
	      AddTriangles( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::ADD_QUADRANGLE :
	    {
	      AddQuadrangles( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::ADD_TETRAHEDRON :
	    {
	      AddTetras( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::ADD_PYRAMID :
	    {
	      break;
	    }
	  case SMESH::ADD_PRISM :
	    {
	      break;
	    }
	  case SMESH::ADD_HEXAHEDRON :
	    {
	      AddHexaedres( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::REMOVE_NODE :
	    {
	      RemoveNodes( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  case SMESH::REMOVE_ELEMENT :
	    {
	      RemoveElements( MeshActor, aSeq[ind].number, aSeq[ind].coords, aSeq[ind].indexes );
	      break;
	    }
	  }
      }
      return MeshActor;
    }
  }
  return NULL;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Dump(SMESH_Actor* Mactor)
{
  vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  vtkPoints *Pts = ugrid->GetPoints();
  int nbPts = Pts->GetNumberOfPoints();
  int nbCells = ugrid->GetNumberOfCells();

  FILE *In;
  int i,j;
  In = fopen("/tmp/dumpMesh","w+");
  fprintf(In,"%d %d\n",nbPts,nbCells);
  for ( int i = 0; i < nbPts; i++ ) {
    float *p = ugrid->GetPoint(i);
    fprintf(In,"%d %e %e %e\n",i, p[0], p[1], p[2] );
  }

  for ( int i = 0; i < nbCells; i++ ) {
    fprintf(In,"%d %d",i,ugrid->GetCell(i)->GetCellType());
    vtkIdList *Id = ugrid->GetCell(i)->GetPointIds();
    for (j=0; j<Id->GetNumberOfIds(); j++) {
      fprintf(In," %d",Id->GetId(j));
    }
    fprintf(In,"\n");
  }
  fclose(In);
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddNodes( SMESH_Actor* Mactor, int number, 
			 const SMESH::double_array& coords,
			 const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  if ( Mactor->GetMapper() == NULL ) {
    vtkPoints    *Pts = vtkPoints::New();
    SMESH_Grid *ugrid = SMESH_Grid::New();
    ugrid->Allocate();

    int i = 1;
    int j = 1;
    while ( i <= number ) {
      int idVTK = Pts->InsertNextPoint( coords[j-1], coords[j], coords[j+1] );
      //Mactor->AddNode( indexes[i-1], idVTK );
      ugrid->AddNode( indexes[i-1], idVTK );
      i++;
      j = j + 3;
    }
    //vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(Pts);
    vtkDataSetMapper * PtsMapper = vtkDataSetMapper::New();
    PtsMapper->SetInput( ugrid );
    Mactor->DataSource = PtsMapper->GetInput();
    Mactor->SetMapper(PtsMapper);
  } else {
    //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
    SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
    int i = 1;
    int j = 1;
    while ( i <= number ) {
      int idVTK = ugrid->GetPoints()->InsertNextPoint( coords[j-1], coords[j], coords[j+1] );
      ugrid->AddNode( indexes[i-1], idVTK );
      i++;
      j = j + 3;
    }
    vtkDataSetMapper *PtsMapper = vtkDataSetMapper::New();
    PtsMapper->SetInput( ugrid );
    Mactor->DataSource = PtsMapper->GetInput();
    Mactor->SetMapper(PtsMapper);
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddNode(SMESH_Actor* Mactor, int idnode, float x, float y, float z)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  MESSAGE ( "SMESHGUI::AddNode " << idnode << " : " << x << ";" << y << ";" << z )
  if ( Mactor->GetMapper() == NULL ) {
    vtkPoints *Pts = vtkPoints::New();
    int idVTK = Pts->InsertNextPoint( x, y, z );
    //Mactor->AddNode( idnode, idVTK );
    //vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    SMESH_Grid *ugrid = SMESH_Grid::New();
    ugrid->Allocate();
    ugrid->AddNode( idnode, idVTK );
    ugrid->SetPoints(Pts);
    vtkDataSetMapper * PtsMapper = vtkDataSetMapper::New();
    PtsMapper->SetInput( ugrid );
    Mactor->DataSource = PtsMapper->GetInput();
    Mactor->SetMapper(PtsMapper);
  } else {
    //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
    SMESH_Grid *ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
    int idVTK = ugrid->GetPoints()->InsertNextPoint( x, y, z );
    //Mactor->AddNode( idnode, idVTK );
    ugrid->AddNode( idnode, idVTK );
    vtkDataSetMapper *PtsMapper = vtkDataSetMapper::New();
    PtsMapper->SetInput( ugrid );
    Mactor->DataSource = PtsMapper->GetInput();
    Mactor->SetMapper(PtsMapper);
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveNode(SMESH_Actor* Mactor, int idnode)
{
  MESSAGE ( " OLD RemoveNode method " )
//   int id = Mactor->GetIdVTKNode( idnode );
//    MESSAGE ( " RemoveNode id VTK " << id )
//    if ( Mactor->GetMapper() != NULL ) {
//      vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
//      vtkUnstructuredGrid* newUgrid = vtkUnstructuredGrid::New();
//      vtkPoints *Pts = ugrid->GetPoints();
//      vtkPoints *newPts = vtkPoints::New();
//      int nbPts = Pts->GetNumberOfPoints();
//      bool findPt = false;
//      for ( int i = 0; i < nbPts; i++ ) {
//        if ( id != i ) {
//  	if ( !findPt)
//  	  newPts->InsertPoint(i, Pts->GetPoint(i) );
//  	else
//  	  newPts->InsertPoint(i-1, Pts->GetPoint(i) );
//        } else {
//  	findPt = true;
//  	Mactor->RemoveNode( idnode );
//        }
//      }

//      newUgrid->SetPoints(newPts);
//      int nbCells = ugrid->GetNumberOfCells();
//      for ( int i = 0; i < nbCells; i++ ) {
//        vtkIdList *Ids = ugrid->GetCell(i)->GetPointIds();
//        vtkIdList *newIds = vtkIdList::New();
//        int nbIds = Ids->GetNumberOfIds();
//        newIds->SetNumberOfIds(nbIds);
//        for ( int j = 0; j < nbIds; j++ ) {
//  	int theid = Ids->GetId(j);
//  	if ( theid > id ) {
//  	  newIds->SetId( j, theid-1 );
//  	} else
//  	  newIds->SetId( j, theid );
//        }
//        int idSMDSel = Mactor->GetIdSMESHDSElement( i );
//        Mactor->RemoveElement( idSMDSel, false );
//        int idVTKel = newUgrid->InsertNextCell( ugrid->GetCell(i)->GetCellType(), newIds );
//        Mactor->AddElement( idSMDSel, idVTKel );
//      }


//      vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
//      Mapper->SetInput( newUgrid );
//      Mactor->DataSource = Mapper->GetInput();
//      Mactor->SetMapper(Mapper);

//      UpdateView();
//    }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveNodes(SMESH_Actor* Mactor, int number, 
			   const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  int i = 1;
  while ( i <= number ) {
    Mactor->RemoveNode( indexes[i-1] );
    i++;
  }

  TColStd_DataMapOfIntegerInteger newMapVTKNodes;
  TColStd_DataMapOfIntegerInteger newMapSMESHDSNodes;
  TColStd_DataMapOfIntegerInteger MapOldNodesToNewNodes;

  if ( Mactor->GetMapper() != NULL ) {
    SMESH_Grid *ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
    SMESH_Grid *newUgrid = SMESH_Grid::New();
    newUgrid->CopyMaps(ugrid);

    vtkPoints *Pts = ugrid->GetPoints();
    vtkPoints *newPts = vtkPoints::New();
    int nbPts = Pts->GetNumberOfPoints();
    int j = 0;
    for ( int i = 0; i < nbPts; i++ ) {
      int idSMESHDSNode = Mactor->GetIdSMESHDSNode(i);
      if ( idSMESHDSNode != -1 ) {
	newPts->InsertPoint(j, Pts->GetPoint(i) );

	newMapVTKNodes.Bind(j, idSMESHDSNode);
	newMapSMESHDSNodes.Bind(idSMESHDSNode, j);

	MapOldNodesToNewNodes.Bind(i,j);
	j++;
      }
    }

    newUgrid->SetIdsVTKNode(newMapVTKNodes); 
    newUgrid->SetIdsSMESHDSNode(newMapSMESHDSNodes); 
    newUgrid->SetPoints(newPts);

    TColStd_DataMapOfIntegerInteger newMapElementSMDStoVTK;
    TColStd_DataMapOfIntegerInteger newMapElementVTKtoSMDS;

    int nbCells = ugrid->GetNumberOfCells();
    for ( int i = 0; i < nbCells; i++ ) {
      vtkIdList *Ids = ugrid->GetCell(i)->GetPointIds();
      vtkIdList *newIds = vtkIdList::New();
      int nbIds = Ids->GetNumberOfIds();
      newIds->SetNumberOfIds(nbIds);
      bool isGood = true;
      for ( int j = 0; j < nbIds; j++ ) {
	int theid = Ids->GetId(j);
	if ( MapOldNodesToNewNodes.IsBound( theid ) ) {
	  newIds->SetId( j, MapOldNodesToNewNodes.Find( theid ) );
	} else {
	  isGood = false;
	  break;
	}
      }

      // Filtering out cells based on non-existing nodes
      if (isGood) {
	int idSMDSel = Mactor->GetIdSMESHDSElement( i );
	int idVTKel = newUgrid->InsertNextCell( ugrid->GetCell(i)->GetCellType(), newIds );

	newMapElementSMDStoVTK.Bind(idSMDSel, idVTKel);
	newMapElementVTKtoSMDS.Bind(idVTKel, idSMDSel);
      }
    }

    newUgrid->SetIdsVTKElement(newMapElementVTKtoSMDS);
    newUgrid->SetIdsSMESHDSElement(newMapElementSMDStoVTK);
    
    // Copy new data to the old DatSource: keep the single DataSource for all actors
    ugrid->DeepCopy(newUgrid);

    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( ugrid );
    Mactor->SetMapper(Mapper);
    
    // Commented to avoid multiple viewer updates when called by ReadScript()
    //UpdateView();

  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveNodes(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    SMESH::long_array_var anArrayOfIdeces = new  SMESH::long_array;
    anArrayOfIdeces->length(MapIndex.Extent());
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int i = 0;
    for ( ; ite.More(); ite.Next() ) {
      //      MESSAGE ( " RemoveNode : id " << ac->GetIdSMESHDSNode(ite.Key()) )
      anArrayOfIdeces[i] = ac->GetIdSMESHDSNode(ite.Key());
      i++;
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    aMeshEditor->RemoveNodes(anArrayOfIdeces);

  }
  if ( myAutomaticUpdate ) {
    SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
    if ( Mesh != NULL ) {
      smeshGUI->DisplayActor( Mesh );
      smeshGUI->DisplayEdges( Mesh );
      smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
      AddActorInSelection( Mesh );
    }
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveElement(SMESH_Actor* Mactor, int idelement)
{
  MESSAGE ( " OLD RemoveElement method " )
//    int id = Mactor->GetIdVTKElement( idelement );
//    MESSAGE ( " RemoveElement id VTK : " << id )
//    if ( Mactor->GetMapper() != NULL ) {
//      vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
//      vtkUnstructuredGrid* newUgrid = vtkUnstructuredGrid::New();
    
//      int nbCells = ugrid->GetNumberOfCells();
//      for ( int i = 0; i < nbCells; i++ ) {
//        if ( id != i ) {
//  	newUgrid->InsertNextCell( ugrid->GetCell(i)->GetCellType(), 
//  				  ugrid->GetCell(i)->GetPointIds() );
//        } else
//    	Mactor->RemoveElement( idelement );
//      }
//      newUgrid->SetPoints(ugrid->GetPoints());
    
//      vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
//      Mapper->SetInput( newUgrid );
//      Mactor->DataSource = Mapper->GetInput();
//      Mactor->SetMapper(Mapper);
    
//      UpdateView();
//    }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveElements(SMESH_Actor* Mactor, int number, 
			      const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  int i = 1;
  while ( i <= number ) {
    Mactor->RemoveElement( indexes[i-1] );
    i++;
  }
  TColStd_DataMapOfIntegerInteger newMapElementSMDStoVTK;
  TColStd_DataMapOfIntegerInteger newMapElementVTKtoSMDS;

  if ( Mactor->GetMapper() != NULL ) {
    SMESH_Grid*    ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
    SMESH_Grid* newUgrid = SMESH_Grid::New();
    newUgrid->CopyMaps(ugrid);

    int nbCells = ugrid->GetNumberOfCells();
    for ( int i = 0; i < nbCells; i++ ) {
      int idSMESHDSElement = Mactor->GetIdSMESHDSElement(i);
      if ( idSMESHDSElement != -1 ) {
	int newId = newUgrid->InsertNextCell( ugrid->GetCell(i)->GetCellType(), 
					      ugrid->GetCell(i)->GetPointIds() );
	newMapElementSMDStoVTK.Bind(idSMESHDSElement, newId);
	newMapElementVTKtoSMDS.Bind(newId, idSMESHDSElement);
      }
    }

    newUgrid->SetIdsVTKElement(newMapElementVTKtoSMDS);
    newUgrid->SetIdsSMESHDSElement(newMapElementSMDStoVTK);

    newUgrid->SetPoints(ugrid->GetPoints());

    // Copy new data to the old DatSource: keep the single DataSource for all actors
    ugrid->DeepCopy(newUgrid);
    
    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( ugrid );
    Mactor->SetMapper(Mapper);

    // Commented to avoid multiple viewer updates when called by ReadScript()
    //UpdateView();
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::RemoveElements(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    SMESH::long_array_var anArrayOfIdeces = new  SMESH::long_array;
    anArrayOfIdeces->length(MapIndex.Extent());
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int i = 0;
    for ( ; ite.More(); ite.Next() ) {
      //      MESSAGE ( " RemoveElement : id " << ite.Key() << "," << ac->GetIdSMESHDSElement(ite.Key()) )
      anArrayOfIdeces[i] = ac->GetIdSMESHDSElement(ite.Key());
      i++;
    }

    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    aMeshEditor->RemoveElements(anArrayOfIdeces);
  }
    
  if ( myAutomaticUpdate ) {
    SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
    if ( Mesh != NULL ) {
      smeshGUI->DisplayActor( Mesh );
      smeshGUI->DisplayEdges( Mesh );
      smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
      AddActorInSelection( Mesh );
#ifdef TRACE
      Dump( Mesh );
#endif
    }
  }
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::OrientationElements(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    //vtkUnstructuredGrid* UGrid = vtkUnstructuredGrid::New();
    SMESH_Grid*   UGrid = SMESH_Grid::New();
    SMESH_Grid* oldGrid = SMESH_Grid::SafeDownCast(ac->DataSource);
    UGrid->CopyMaps(oldGrid);

    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    gf->SetInput( ac->DataSource );

    vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
    Mapper->SetInput( gf->GetOutput() );
    Mapper->Update();

    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    for ( ; ite.More(); ite.Next() ) {
       Mapper->GetInput()->ReverseCell( ite.Key() );
    }

    UGrid->SetPoints( Mapper->GetInput()->GetPoints() );
    int nbCells = Mapper->GetInput()->GetNumberOfCells();
    for ( int i = 0; i < nbCells; i++ ) {
      UGrid->InsertNextCell( Mapper->GetInput()->GetCellType(i), Mapper->GetInput()->GetCell(i)->GetPointIds() );
    }

    // Copy new data to the old DatSource: keep the single DataSource for all actors
    oldGrid->DeepCopy(UGrid);

    vtkDataSetMapper *NewMapper = vtkDataSetMapper::New();
    //NewMapper->SetInput( UGrid );
    NewMapper->SetInput( oldGrid );
    NewMapper->Update();

    //ac->DataSource = NewMapper->GetInput();
    ac->SetMapper( NewMapper );
  }
  vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
  renWin->Render();
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DiagonalInversion(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  QApplication::setOverrideCursor( Qt::waitCursor );
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();  
  Standard_Boolean result;

  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    //vtkUnstructuredGrid* UGrid = vtkUnstructuredGrid::New();
    SMESH_Grid* UGrid = SMESH_Grid::New();
    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    gf->SetInput( ac->DataSource );
    
    vtkExtractEdges *edges = vtkExtractEdges::New();
    edges->SetInput( ac->DataSource );

    vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();
    Mapper->SetInput( edges->GetOutput() );
    Mapper->Update();

    int nb = Mapper->GetInput()->GetNumberOfCells();
    //MESSAGE ( "nb : " << nb )

    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    for ( ; ite.More(); ite.Next() ) {
      vtkCell* StartEdge = Mapper->GetInput()->GetCell(ite.Key());
      //MESSAGE( "DCQ : Edge Id = " << ite.Key())
      int CellType = StartEdge->GetCellType();
      //MESSAGE( "DCQ : Cell Type = " << CellType)
      int nbPoints = StartEdge->GetNumberOfPoints();

      //MESSAGE( "DCQ : Nb Point = " << nbPoints)
      if ( nbPoints == 2 ) {
	vtkUnstructuredGrid* StartUGrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
	
	vtkIdList *IdCells =  vtkIdList::New();
	vtkIdList *IdPoints = StartEdge->GetPointIds();
	float p1[3];
	float p2[3];
	
	Mapper->GetInput()->GetPoints()->GetPoint(IdPoints->GetId(0),p1);
	Mapper->GetInput()->GetPoints()->GetPoint(IdPoints->GetId(1),p2);

	int idp1 = StartUGrid->FindPoint(p1);
	int idp2 = StartUGrid->FindPoint(p2);

	StartUGrid->GetPointCells( idp1, IdCells );
	
	//MESSAGE ( " pt 0 : " << IdPoints->GetId(0) )
	//MESSAGE ( " pt 1 : " << IdPoints->GetId(1) )

	//MESSAGE ( " pt 0 : " << idp1 )
	//MESSAGE ( " pt 1 : " << idp2 )

	vtkIdList *IdPts = vtkIdList::New();
	if (IdCells->GetNumberOfIds() >= 2) {
	  int nbCells = IdCells->GetNumberOfIds();
	  //MESSAGE ( " nbCells : " << nbCells )
	  for (int j=0; j<nbCells; j++) {
	    StartUGrid->GetCellPoints( IdCells->GetId(j), IdPts );
	    if ( IdPts->IsId( idp2 ) == -1 ) {
	      IdCells->DeleteId( IdCells->GetId(j) );
	    }
	  }
	

	  //MESSAGE ( " IdCells " << IdCells->GetNumberOfIds() )

	  vtkIdList *IdPts0 = vtkIdList::New();
	  vtkIdList *IdPts1 = vtkIdList::New();

	  if (IdCells->GetNumberOfIds() == 2) {
	    StartUGrid->GetCellPoints( IdCells->GetId(0), IdPts0 );
	    StartUGrid->GetCellPoints( IdCells->GetId(1), IdPts1 );
	    
	    //Create new faces
	    TColStd_MapOfInteger EndMapIndex;
	    for (int j = 0; j < 3; j++ ) {
	      if ( IdPts0->GetId(j) != idp1 && IdPts0->GetId(j) != idp2 ) {
		EndMapIndex.Add( IdPts0->GetId(j) );
	      }
	      if ( IdPts1->GetId(j) != idp1 && IdPts1->GetId(j) != idp2 ) {
		EndMapIndex.Add( IdPts1->GetId(j) );
	      }
	    }

	    bool MyAU = myAutomaticUpdate ;
	    myAutomaticUpdate = false ;

	    EndMapIndex.Add( idp1 );
	    TColStd_MapIteratorOfMapOfInteger ite1( EndMapIndex );
	    int i = 1;
	    while ( ite1.More() ) {
	      if ( ite1.Key() == idp1 )
		break;
	      i++;
	      ite1.Next();
	    }
	    bool reverse1 = ( i == 2 );
	    this->AddFace( aMesh, EndMapIndex, reverse1 );
	    
	    EndMapIndex.Remove( idp1 );
	    EndMapIndex.Add( idp2 );
	    TColStd_MapIteratorOfMapOfInteger ite2( EndMapIndex );
	    i = 1;
	    while ( ite2.More() ) {
	      if ( ite2.Key() == idp2 )
		break;
	      i++;
	      ite2.Next();
	    }
	    bool reverse2 = ( i == 2 );
	    this->AddFace( aMesh, EndMapIndex, !(reverse1 == reverse2) );
	    
	    myAutomaticUpdate = MyAU ;
	    //MESSAGE ( " myAutomaticUpdate = " << MyAU )
	    Mapper->Update();
	    //Remove old faces
	    TColStd_MapOfInteger StartMapIndex;
	    StartMapIndex.Add( IdCells->GetId(0) );
	    StartMapIndex.Add( IdCells->GetId(1) );
	    this->RemoveElements( aMesh, StartMapIndex );
	    
	    Mapper->Update();
	  } 
	}
      }
    }
    
    //      UGrid->SetPoints( Mapper->GetInput()->GetPoints() );
    //      int nbCells = Mapper->GetInput()->GetNumberOfCells();
    //      for ( int i = 0; i < nbCells; i++ ) {
    //        UGrid->InsertNextCell( Mapper->GetInput()->GetCellType(i), Mapper->GetInput()->GetCell(i)->GetPointIds() );
    //      }
    
    //      vtkDataSetMapper *NewMapper = vtkDataSetMapper::New();
    //      NewMapper->SetInput( UGrid );
    //      NewMapper->Update();
    
    //      ac->DataSource = NewMapper->GetInput();
    //      ac->SetMapper( NewMapper );
  }
  vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
  renWin->Render();
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddEdges( SMESH_Actor* Mactor, int number, 
			 const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int i = 1;
  int j = 1;
  while ( i <= number ) {
    vtkIdList *Ids = vtkIdList::New();
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+1] ) );
    int id = ugrid->InsertNextCell(VTK_LINE, Ids);
    Mactor->AddElement( indexes[j-1], id );
    i++;
    j = j + 3;
  }
  vtkDataSetMapper *EdgeMapper = vtkDataSetMapper::New();
  EdgeMapper->SetInput( ugrid );
  Mactor->DataSource = EdgeMapper->GetInput();
  Mactor->SetMapper(EdgeMapper);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddEdge(SMESH_Actor* Mactor, int idedge, int idnode1, int idnode2) 
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  MESSAGE ( "SMESHGUI::AddEdge " << idedge << " : " << idnode1 << ";" << idnode2 )
  vtkIdList *Ids = vtkIdList::New();
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode1 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode2 ) );

  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int id = ugrid->InsertNextCell(VTK_LINE, Ids);
  Mactor->AddElement( idedge, id );
  
  MESSAGE ( " Edge VTK id " << id )

  vtkDataSetMapper *EdgeMapper = vtkDataSetMapper::New();
  EdgeMapper->SetInput( ugrid );
  Mactor->DataSource = EdgeMapper->GetInput();
  Mactor->SetMapper(EdgeMapper);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddTriangles( SMESH_Actor* Mactor, int number, 
			     const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int i = 1;
  int j = 1;
  while ( i <= number ) {
    vtkIdList *Ids = vtkIdList::New();
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+1] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+2] ) );
    int id = ugrid->InsertNextCell(VTK_TRIANGLE, Ids);
    Mactor->AddElement( indexes[j-1], id );
    i++;
    j = j + 4;
  }
  vtkDataSetMapper *TriMapper = vtkDataSetMapper::New();
  TriMapper->SetInput( ugrid );
  Mactor->DataSource = TriMapper->GetInput();
  Mactor->SetMapper(TriMapper);
  QApplication::restoreOverrideCursor();
}
void SMESHGUI::AddTriangle(SMESH_Actor* Mactor, int idtri, int idnode1, int idnode2, int idnode3) 
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  vtkIdList *Ids = vtkIdList::New();
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode1 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode2 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode3 ) );

  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int id = ugrid->InsertNextCell(VTK_TRIANGLE, Ids);
  Mactor->AddElement( idtri, id );

  vtkDataSetMapper *TriMapper = vtkDataSetMapper::New();
  TriMapper->SetInput( ugrid );
  Mactor->DataSource = TriMapper->GetInput();
  Mactor->SetMapper(TriMapper);
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddQuadrangles( SMESH_Actor* Mactor, int number, 
			       const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int i = 1;
  int j = 1;
  while ( i <= number ) {
    vtkIdList *Ids = vtkIdList::New();
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+1] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+2] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+3] ) );
    int id = ugrid->InsertNextCell(VTK_QUAD, Ids);
    Mactor->AddElement( indexes[j-1], id );
    i++;
    j = j + 5;
  }
  vtkDataSetMapper *QuadMapper = vtkDataSetMapper::New();
  QuadMapper->SetInput( ugrid );
  Mactor->DataSource = QuadMapper->GetInput();
  Mactor->SetMapper(QuadMapper);
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddQuadrangle(SMESH_Actor* Mactor, int idquad, int idnode1, int idnode2, 
			     int idnode3, int idnode4) 
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  vtkIdList *Ids = vtkIdList::New();
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode1 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode2 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode3 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode4 ) );

  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int id = ugrid->InsertNextCell(VTK_QUAD, Ids);
  Mactor->AddElement( idquad, id );

  vtkDataSetMapper *QuadMapper = vtkDataSetMapper::New();
  QuadMapper->SetInput( ugrid );
  Mactor->DataSource = QuadMapper->GetInput();
  Mactor->SetMapper(QuadMapper);
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddTetras( SMESH_Actor* Mactor, int number, 
			       const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
    SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int i = 1;
  int j = 1;
  while ( i <= number ) {
    vtkIdList *Ids = vtkIdList::New();
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+1] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+2] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+3] ) );
    int id = ugrid->InsertNextCell(VTK_TETRA, Ids);
    Mactor->AddElement( indexes[j-1], id );
    i++;
    j = j + 5;
  }
  vtkDataSetMapper *TetraMapper = vtkDataSetMapper::New();
  TetraMapper->SetInput( ugrid );
  Mactor->DataSource = TetraMapper->GetInput();
  Mactor->SetMapper(TetraMapper);
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddTetra(SMESH_Actor* Mactor, int idtetra, int idnode1, int idnode2, 
			int idnode3, int idnode4) 
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  MESSAGE ( "SMESHGUI::AddTetra " << idtetra << " : " << idnode1 << ";" << idnode2  
	    << ";" << idnode3 << ";" << idnode4 )
  vtkIdList *Ids = vtkIdList::New();
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode1 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode2 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode3 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode4 ) );

  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int id = ugrid->InsertNextCell(VTK_TETRA, Ids);
  Mactor->AddElement( idtetra, id );
  
  MESSAGE ( " Tetra VTK id " << id )

  vtkDataSetMapper *TetraMapper = vtkDataSetMapper::New();
  TetraMapper->SetInput( ugrid );
  Mactor->DataSource = TetraMapper->GetInput();
  Mactor->SetMapper(TetraMapper);
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddHexaedres( SMESH_Actor* Mactor, int number, 
			       const SMESH::double_array& coords, const SMESH::long_array& indexes)
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  //vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int i = 1;
  int j = 1;
  while ( i <= number ) {
    vtkIdList *Ids = vtkIdList::New();
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+1] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+2] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+3] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+4] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+5] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+6] ) );
    Ids->InsertNextId( Mactor->GetIdVTKNode( indexes[j+7] ) );
    int id = ugrid->InsertNextCell(VTK_HEXAHEDRON, Ids);
    Mactor->AddElement( indexes[j-1], id );
    i++;
    j = j + 9;
  }
  vtkDataSetMapper *HexaMapper = vtkDataSetMapper::New();
  HexaMapper->SetInput( ugrid );
  Mactor->DataSource = HexaMapper->GetInput();
  Mactor->SetMapper(HexaMapper);
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddHexaedre(SMESH_Actor* Mactor, int idhexa, int idnode1, int idnode2, 
			   int idnode3, int idnode4, int idnode5, int idnode6, int idnode7, int idnode8) 
{
  QApplication::setOverrideCursor( Qt::waitCursor );
  MESSAGE ( "SMESHGUI::AddHexaedre " << idhexa << " : " << idnode1 << ";" << idnode2  
	    << ";" << idnode3 << ";" << idnode4 << ";" << idnode5 << ";" << idnode6
	    << ";" << idnode7 << ";" << idnode8)

  vtkIdList *Ids = vtkIdList::New();
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode1 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode2 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode3 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode4 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode5 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode6 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode7 ) );
  Ids->InsertNextId( Mactor->GetIdVTKNode( idnode8 ) );

  //vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  SMESH_Grid* ugrid = SMESH_Grid::SafeDownCast( Mactor->DataSource );
  int id = ugrid->InsertNextCell(VTK_HEXAHEDRON, Ids);
  Mactor->AddElement( idhexa, id );
  
  MESSAGE ( " Hexa VTK id " << id )

  vtkDataSetMapper *HexaMapper = vtkDataSetMapper::New();
  HexaMapper->SetInput( ugrid );
  Mactor->DataSource = HexaMapper->GetInput();
  Mactor->SetMapper(HexaMapper);
  QApplication::restoreOverrideCursor();
}

//=====================================================================================
// EXPORTED METHODS
//=====================================================================================
extern "C"
{
  bool OnGUIEvent(int theCommandID, QAD_Desktop* parent)
  {
    return SMESHGUI::OnGUIEvent(theCommandID, parent);
  }

  bool OnKeyPress (QKeyEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame)
  {
    return SMESHGUI::OnKeyPress (pe, parent, studyFrame);
  }

  bool OnMousePress (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame)
  {
    return SMESHGUI::OnMousePress (pe, parent, studyFrame);
  }

  bool OnMouseMove (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame)
  {
    return SMESHGUI::OnMouseMove (pe, parent, studyFrame);
  }

  bool SetSettings ( QAD_Desktop* parent )
  {
    return SMESHGUI::SetSettings( parent );
  }

  bool customPopup ( QAD_Desktop* parent, QPopupMenu* popup, const QString & theContext,
		     const QString & theParent, const QString & theObject )
  {
     return SMESHGUI::CustomPopup( parent, popup, theContext, theParent, theObject );
  }

  void definePopup ( QString & theContext, QString & theParent, QString & theObject )
  {
    SMESHGUI::DefinePopup( theContext, theParent, theObject );
  }
  
  bool activeStudyChanged ( QAD_Desktop* parent )
  {
    SMESHGUI::activeStudyChanged( parent );
  }

  void buildPresentation ( const Handle(SALOME_InteractiveObject)& theIO )
  {
    SMESHGUI::BuildPresentation(theIO);
  }

  void supportedViewType(int* buffer, int bufferSize)
  {
    if (!buffer || !bufferSize) return;
    buffer[0] = (int)VIEW_VTK;
  }

}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ViewNodes()
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkActorCollection* theActors = theRenderer->GetActors();
  theActors->InitTraversal();
  vtkActor *ac = theActors->GetNextActor();
  while(!(ac==NULL)) {
    if ( ac->IsA("SMESH_Actor") ) {
      SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( ac );
      if ( anActor->GetVisibility() == 1 ) {
	vtkGeometryFilter *gf = vtkGeometryFilter::New();
	gf->SetInput( anActor->DataSource );
	vtkMaskPoints *verts = vtkMaskPoints::New();
	verts->SetInput(gf->GetOutput());
	verts->GenerateVerticesOn();
	verts->SetOnRatio(1);
      
	vtkPolyDataMapper *vertMapper = vtkPolyDataMapper::New();
	vertMapper->SetInput(verts->GetOutput());
	vertMapper->ScalarVisibilityOff();
      
	vtkActor *vertActor = vtkActor::New();
	vertActor->SetMapper(vertMapper);

	float r, g , b ;
	anActor->GetNodeColor(r, g, b ) ;
	vertActor->GetProperty()->SetColor( r, g, b ) ;

	vertActor->GetProperty()->SetPointSize( anActor->GetNodeSize() );

	vertActor->PickableOff();

	mySimulationActors->AddItem( vertActor );
	theRenderer->AddActor( vertActor );
      }
    }
    ac = theActors->GetNextActor();
  }
  
  vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
  renWin->Render();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Control(int theCommandID)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  QApplication::setOverrideCursor( Qt::waitCursor );
  DisplayScalarBar( false );

  vtkDoubleArray *scalars = vtkDoubleArray::New();
  scalars->SetNumberOfComponents(1);

  vtkDataSetMapper* meshMapper = 0;
  SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
  int nbSel = Sel->IObjectCount();
  Standard_Boolean result;
  Handle(SALOME_InteractiveObject) IObject = Sel->firstIObject();
  SMESH_Actor* MeshActor = FindActorByEntry(IObject->getEntry(), result, true);

  // Mesh may be not updated after Compute
  if (!MeshActor->DataSource || !MeshActor->GetMapper()) {
    QApplication::restoreOverrideCursor();
    return;
  }

  vtkDataSet* aDataSet = MeshActor->DataSource;
  typedef double (*TScalarFun)(vtkCell* theCell);
  TScalarFun aScalarFun;
  if(result){
    QString type;
    switch (theCommandID){
    case 6001: {
      type = tr( "SMESH_CONTROL_LENGTH_EDGES");
      aDataSet = MeshActor->EdgeDevice->GetMapper()->GetInput();
      aScalarFun = &(SMESHGUI_ComputeScalarValue::LengthEdges);
      MESSAGE ( " init minimum length " << aDataSet->GetNumberOfCells() );
      if(MeshActor->getDisplayMode() != 0)
	ChangeRepresentation( MeshActor, 1);// limitation; in Wireframe, colored edges are not visible
      break;
    } 
    case 6011: {
      type = tr( "SMESH_CONTROL_AREA_ELEMENTS");
      aScalarFun = &(SMESHGUI_ComputeScalarValue::AreaElements);
      if(MeshActor->getDisplayMode() != 1)
	ChangeRepresentation( MeshActor, 1 );
      break;
    }
    case 6012: {
      type = tr( "SMESH_CONTROL_TAPER_ELEMENTS");
      aScalarFun = &(SMESHGUI_ComputeScalarValue::Taper);
      break;
    }
    case 6013: {
      type = tr( "SMESH_CONTROL_ASPECTRATIO_ELEMENTS");
      aScalarFun = &(SMESHGUI_ComputeScalarValue::AspectRatio);
      if(MeshActor->getDisplayMode() != 1)
	ChangeRepresentation( MeshActor, 1 );
      break;
    }
    case 6014: {
      type = tr( "SMESH_CONTROL_MINIMUMANGLE_ELEMENTS");
      aScalarFun = &(SMESHGUI_ComputeScalarValue::MinimumAngle);
      if(MeshActor->getDisplayMode() != 1)
	ChangeRepresentation( MeshActor, 1 );
      break;
    }
    case 6015: {
      type = tr( "SMESH_CONTROL_WARP_ELEMENTS");
      aScalarFun = &(SMESHGUI_ComputeScalarValue::Warp);
      break;
    }
    case 6016: {
      type = tr( "SMESH_CONTROL_SKEW_ELEMENTS");
      aScalarFun = &(SMESHGUI_ComputeScalarValue::Skew);
      break;
    }}

    for(int i = 0, iEnd = aDataSet->GetNumberOfCells(); i < iEnd; i++)
      scalars->InsertTuple1(i,aScalarFun(aDataSet->GetCell(i)));

    float range[2];
    scalars->GetRange(range);

    vtkLookupTable* wat = vtkLookupTable::New();
    wat->SetRange( range );
    wat->Build();

    scalars->SetLookupTable(wat);

    if (!meshMapper) meshMapper = (vtkDataSetMapper*) (MeshActor->getMapper());
    meshMapper->SetScalarModeToUseCellData();
    MeshActor->DataSource->GetCellData()->SetScalars(scalars);
    meshMapper->SetScalarRange( range );
    meshMapper->ScalarVisibilityOn();

    vtkScalarBarActor* aScalarBar = GetScalarBar();
    if ( aScalarBar == NULL ) {
      aScalarBar = vtkScalarBarActor::New();
      QString Bold = QAD_CONFIG->getSetting("ScalarBar:Bold");
      QString Italic = QAD_CONFIG->getSetting("ScalarBar:Italic");
      QString Shadow = QAD_CONFIG->getSetting("ScalarBar:Shadow");    
      QString FontFamily = QAD_CONFIG->getSetting("ScalarBar:FontFamily");
      QString Orientation = QAD_CONFIG->getSetting("ScalarBar:Orientation");
      float Width = QAD_CONFIG->getSetting("ScalarBar:Width").toFloat();  
      float Height = QAD_CONFIG->getSetting("ScalarBar:Height").toFloat();
      int NumberOfLabels = QAD_CONFIG->getSetting("ScalarBar:NumberOfLabels").toInt();
      int NumberOfColors = QAD_CONFIG->getSetting("ScalarBar:NumberOfColors").toInt();
      
      SetSettingsScalarBar(aScalarBar, Bold, Italic, Shadow, FontFamily, Orientation,
			   Width, Height, NumberOfColors, NumberOfLabels);
      vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
      theRenderer->AddActor2D(aScalarBar);
    }  
    aScalarBar->SetLookupTable( wat );
    aScalarBar->SetTitle( type.latin1() );

    scalars->Delete();
    //    wat->Delete();
    DisplayScalarBar( true );
  }
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetSettingsScalarBar(vtkScalarBarActor* theScalarBar,
				    QString Bold, QString Italic,
				    QString Shadow, QString FontFamily, 
				    QString Orientation, float Width, float Height, 
				    int NbColors, int NbLabels  )
{
  if ( Bold.isNull() || Bold.isEmpty() || (Bold.compare( "true" ) == 0) )
   theScalarBar->BoldOn();
  else
    theScalarBar->BoldOff();

  if ( Italic.isNull() || Italic.isEmpty() || (Italic.compare( "true" ) == 0) )
    theScalarBar->ItalicOn();
  else
    theScalarBar->ItalicOff();
  
  if ( Shadow.isNull() || Shadow.isEmpty() || (Shadow.compare( "true" ) == 0) )
    theScalarBar->ShadowOn();
  else
    theScalarBar->ShadowOff();
    
  if ( FontFamily.compare( "Arial" ) == 0 )
    theScalarBar->SetFontFamilyToArial();
  else if ( FontFamily.compare( "Courier" ) == 0 )
    theScalarBar->SetFontFamilyToCourier();
  else if ( FontFamily.compare( "Times" ) == 0 )
    theScalarBar->SetFontFamilyToTimes();
  else
    theScalarBar->SetFontFamilyToArial();

  if ( Orientation.isNull() || Orientation.isEmpty() || (Orientation.compare( "Vertical" ) == 0) )
    theScalarBar->SetOrientationToVertical();
  else
    theScalarBar->SetOrientationToHorizontal();
   
  
  theScalarBar->SetWidth((Width == 0)? 0.17 : Width);
  theScalarBar->SetHeight((Height == 0)? 0.8 : Height);

  theScalarBar->SetNumberOfLabels((NbLabels == 0)? 5 : NbLabels); 
  theScalarBar->SetMaximumNumberOfColors((NbColors == 0)? 64 : NbColors);
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayScalarBar(bool visibility)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkScalarBarActor* aScalarBar = GetScalarBar();

  if ( aScalarBar == NULL ) {
    MESSAGE("myScalarBar is NULL");
    return;
  }

  if ( visibility )
    aScalarBar->VisibilityOn();
  else {
    aScalarBar->VisibilityOff();
    //Turn off mesh coloring (influences on all views)
    vtkActorCollection* actorList=theRenderer->GetActors();
    actorList->InitTraversal();
    vtkActor *ac = actorList->GetNextActor();
    while(ac!=NULL) {
      if (ac->GetMapper() != NULL) {
	ac->GetMapper()->ScalarVisibilityOff();
      }
      ac = actorList->GetNextActor();
    }
    // Turn off ScalarBar in all views
    /*QList<QAD_StudyFrame> aFrames = myActiveStudy->getStudyFrames();
    for ( QAD_StudyFrame* aFrame = aFrames.first(); aFrame; aFrame = aFrames.next() ) {
      if (aFrame->getTypeView() == VIEW_VTK) {
	vtkRenderer *aRenderer = ((VTKViewer_ViewFrame*) aFrame->getRightFrame()->getViewFrame())->getRenderer();
	vtkActor2DCollection* actor2DList = aRenderer->GetActors2D();
	actor2DList->InitTraversal();
	vtkActor2D* aActor2d =  actor2DList->GetNextActor2D();
	while (aActor2d != NULL) {
	  if (aActor2d->IsA("vtkScalarBarActor")) {
	    aActor2d->VisibilityOff();
	    break;
	  }
	  actor2DList->GetNextActor2D();
	}
      }
    }*/
  }
  myActiveStudy->update3dViewers();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::UpdateScalarBar(float MinRange, float MaxRange)
{
  vtkScalarBarActor* aScalarBar = GetScalarBar();
  if ( aScalarBar == NULL ) {
    MESSAGE("myScalarBar is NULL");
    return;
  }  
  DisplayScalarBar(false); 
  
  aScalarBar->GetLookupTable()->SetRange(MinRange, MaxRange);
  vtkRenderer *aRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkActorCollection* aActorList = aRenderer->GetActors();
  aActorList->InitTraversal();
  vtkActor *aActor = aActorList->GetNextActor();
  while(aActor != NULL) {
    if (aActor->IsA("SMESH_Actor")) {
      SMESH_Actor* aMeshActor = SMESH_Actor::SafeDownCast(aActor); 
      vtkDataSetMapper* aMeshMapper = (vtkDataSetMapper*) (aMeshActor->getMapper());
      if ((aMeshMapper != NULL)) {
	aMeshMapper->SetScalarRange( MinRange, MaxRange );
	aMeshMapper->ScalarVisibilityOn();
      }
    }
    aActor = aActorList->GetNextActor();
  }
  DisplayScalarBar(true); 
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetDisplaySettings() 
{
  EmitSignalDeactivateDialog() ;
  SMESHGUI_Preferences_ColorDlg *aDlg = new SMESHGUI_Preferences_ColorDlg( QAD_Application::getDesktop(), "") ;
  
  QString SCr = QAD_CONFIG->getSetting("SMESH:SettingsFillColorRed");
  QString SCg = QAD_CONFIG->getSetting("SMESH:SettingsFillColorGreen");
  QString SCb = QAD_CONFIG->getSetting("SMESH:SettingsFillColorBlue");
  QColor color = QColor (SCr.toInt(), SCg.toInt(), SCb.toInt());
  aDlg->SetColor(1,color);

  SCr = QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorRed");
  SCg = QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorGreen");
  SCb = QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorBlue");
  color = QColor (SCr.toInt(), SCg.toInt(), SCb.toInt());
  aDlg->SetColor(2,color);

  SCr = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorRed");
  SCg = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorGreen");
  SCb = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorBlue");
  color = QColor (SCr.toInt(), SCg.toInt(), SCb.toInt());
  aDlg->SetColor(3,color);

  QString SBr = QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorRed");
  QString SBg = QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorGreen");
  QString SBb = QAD_CONFIG->getSetting("SMESH:SettingsBackFaceColorBlue");
  color = QColor (SBr.toInt(), SBg.toInt(), SBb.toInt());
  aDlg->SetColor(4,color);

  QString intValue = QAD_CONFIG->getSetting("SMESH:SettingsWidth");
  aDlg->SetIntValue(1,intValue.toInt());
  intValue = QAD_CONFIG->getSetting("SMESH:SettingsNodesSize");
  aDlg->SetIntValue(2,intValue.toInt());
  intValue = QAD_CONFIG->getSetting("SMESH:SettingsShrinkCoeff");
  aDlg->SetIntValue(3,intValue.toInt());
  
  if ( aDlg->exec() ) {
    QColor colorFill = aDlg->GetColor( 1 );
    QAD_CONFIG->addSetting("SMESH:SettingsFillColorRed",   colorFill.red());
    QAD_CONFIG->addSetting("SMESH:SettingsFillColorGreen", colorFill.green());
    QAD_CONFIG->addSetting("SMESH:SettingsFillColorBlue",  colorFill.blue());
    
    QColor colorOutline = aDlg->GetColor( 2 );
    QAD_CONFIG->addSetting("SMESH:SettingsOutlineColorRed",   colorOutline.red());
    QAD_CONFIG->addSetting("SMESH:SettingsOutlineColorGreen", colorOutline.green());
    QAD_CONFIG->addSetting("SMESH:SettingsOutlineColorBlue",  colorOutline.blue());
    
    QColor colorNode = aDlg->GetColor( 3 );
    QAD_CONFIG->addSetting("SMESH:SettingsNodeColorRed",   colorNode.red());
    QAD_CONFIG->addSetting("SMESH:SettingsNodeColorGreen", colorNode.green());
    QAD_CONFIG->addSetting("SMESH:SettingsNodeColorBlue",  colorNode.blue());
    
    QColor colorBackFace = aDlg->GetColor( 4 );
    QAD_CONFIG->addSetting("SMESH:SettingsBackFaceColorRed",   colorBackFace.red());
    QAD_CONFIG->addSetting("SMESH:SettingsBackFaceColorGreen", colorBackFace.green());
    QAD_CONFIG->addSetting("SMESH:SettingsBackFaceColorBlue",  colorBackFace.blue());

    int width = aDlg->GetIntValue( 1 );
    QAD_CONFIG->addSetting("SMESH:SettingsWidth", width);

    int nodes_size = aDlg->GetIntValue( 2 );
    QAD_CONFIG->addSetting("SMESH:SettingsNodesSize", nodes_size);

    int shrink_coeff = aDlg->GetIntValue( 3 );
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
//  top	       : minimum value to be entered
//  decimals   : number of decimals
//=======================================================================
double SMESHGUI::Parameter( Standard_Boolean& res,
	  		    const double      aValue,
			    const char*       aLabel,
			    const char*       aTitle,
			    const double      bottom,
			    const double      top,
			    const int         decimals )
{
  SMESHGUI_aParameterDlg* Dialog =
    new SMESHGUI_aParameterDlg( QAD_Application::getDesktop(),
			        aTitle,
			        aLabel,
			        bottom, top, decimals,
			        TRUE );
  Dialog->setValue( aValue );
  double X = 0.0;
  res = ( Dialog->exec() == QDialog::Accepted );
  if ( res )
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
//  top	       : minimum value to be entered
//=======================================================================
int SMESHGUI::Parameter( Standard_Boolean& res,
			 const int         aValue,
			 const char*       aLabel,
			 const char*       aTitle,
			 const int         bottom,
			 const int         top )
{
  SMESHGUI_aParameterDlg* Dialog =
    new SMESHGUI_aParameterDlg( QAD_Application::getDesktop(),
			        aTitle,
			        aLabel,
			        bottom, top,
			        TRUE );
  Dialog->setValue( aValue );
  int X = 0;
  res = ( Dialog->exec() == QDialog::Accepted );
  if ( res )
    X = Dialog->getIntValue();
  return X;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayEdges(SMESH_Actor* ac, bool visibility)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;
  if (ac->DataSource == NULL || ac->GetMapper() == NULL)
    return;

  vtkGeometryFilter *gf = vtkGeometryFilter::New();
  gf->SetInput(ac->DataSource);
  vtkFeatureEdges *edges = vtkFeatureEdges::New();
  edges->SetInput(gf->GetOutput());
  edges->BoundaryEdgesOn();
  edges->ManifoldEdgesOn();

  vtkPolyDataMapper *edgeMapper = vtkPolyDataMapper::New();
  edgeMapper->SetInput(edges->GetOutput());
  edgeMapper->ScalarVisibilityOff();
  
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkActorCollection* actorList=theRenderer->GetActors();

  int test=actorList->IsItemPresent(ac->EdgeDevice);
  if (test==0) { 
    vtkProperty* prop = vtkProperty::New();
    prop->SetColor( QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorRed").toFloat()/255., 
		    QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorGreen").toFloat()/255.,
		    QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorBlue").toFloat()/255. ) ;
    prop->SetPointSize(QAD_CONFIG->getSetting("SMESH:SettingsNodesSize").toInt());
    prop->SetLineWidth( QAD_CONFIG->getSetting("SMESH:SettingsWidth").toInt() );
    ac->EdgeDevice->SetProperty(prop);
    ac->SetEdgeColor( QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorRed").toFloat()/255., 
		      QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorGreen").toFloat()/255.,
		      QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorBlue").toFloat()/255. );
    ac->EdgeDevice->SetMapper(edgeMapper);
    
    theRenderer->AddActor(ac->EdgeDevice);
  } else {
    ac->EdgeDevice->SetMapper(edgeMapper);
    edgeMapper->Update();
  }
  
  vtkShrinkFilter *shrink = vtkShrinkFilter::New();
  shrink->SetInput(ac->DataSource);
  shrink->SetShrinkFactor(ac->GetShrinkFactor());
  vtkGeometryFilter *Shrinkgf = vtkGeometryFilter::New();
  Shrinkgf->SetInput( shrink->GetOutput() );
  vtkFeatureEdges *ShrinkEdges = vtkFeatureEdges::New();
  ShrinkEdges->SetInput(Shrinkgf->GetOutput());
  ShrinkEdges->BoundaryEdgesOn();
  ShrinkEdges->ManifoldEdgesOn();

  vtkPolyDataMapper *ShrinkEdgeMapper = vtkPolyDataMapper::New();
  ShrinkEdgeMapper->SetInput(ShrinkEdges->GetOutput());
  ShrinkEdgeMapper->ScalarVisibilityOff();

  test=actorList->IsItemPresent(ac->EdgeShrinkDevice);
  if (test==0) { 
    vtkProperty* prop = vtkProperty::New();
    prop->SetColor( QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorRed").toFloat()/255., 
		    QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorGreen").toFloat()/255.,
		    QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorBlue").toFloat()/255. ) ;
    prop->SetPointSize(QAD_CONFIG->getSetting("SMESH:SettingsNodesSize").toInt());
    prop->SetLineWidth( QAD_CONFIG->getSetting("SMESH:SettingsWidth").toInt() );
    ac->EdgeShrinkDevice->SetProperty(prop);
    ac->SetEdgeColor( QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorRed").toFloat()/255., 
		      QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorGreen").toFloat()/255.,
		      QAD_CONFIG->getSetting("SMESH:SettingsOutlineColorBlue").toFloat()/255. );
    ac->EdgeShrinkDevice->SetMapper(ShrinkEdgeMapper);
    
    theRenderer->AddActor(ac->EdgeShrinkDevice);
  } else {
    ac->EdgeShrinkDevice->SetMapper(ShrinkEdgeMapper);
    ShrinkEdgeMapper->Update();
  }

  vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
  renWin->Render();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::InitActor(SMESH::SMESH_Mesh_ptr aMesh)
{
  SALOMEDS::SObject_var aSO_M = myStudyAPI.FindMesh( aMesh );
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  if ( !aSO_M->_is_nil() ) {
    if (aSO_M->FindAttribute(anAttr, "AttributeName") ) {
      aName = SALOMEDS::AttributeName::_narrow(anAttr);
      Standard_Boolean result;
      SMESH_Actor* MeshActor = smeshGUI->FindActorByEntry(aSO_M->GetID(), result, true);
      if ( !result ) {
	SMESH_Actor* amesh = SMESH_Actor::New();
	Handle(SALOME_InteractiveObject) IO = new SALOME_InteractiveObject(aSO_M->GetID(),
									   "MESH",
									   aName->Value());
	amesh->setIO( IO );
	amesh->setName( aName->Value() );
	DisplayActor(amesh, false);
      }
    }
  }
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Update()
{
  if ( myActiveStudy->getActiveStudyFrame()->getTypeView() == VIEW_VTK ) { //VTK
    vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
    
    SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
    int nbSel = Sel->IObjectCount();
    if ( nbSel == 0 ) {
      vtkActorCollection* actorList=theRenderer->GetActors();
      actorList->InitTraversal();
      vtkActor *ac = actorList->GetNextActor();
      while(!(ac==NULL)) {
	if ( ac->IsA("SMESH_Actor") ) {
	  SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( ac );
	  if ( anActor->hasIO() ) {
	    Handle(SALOME_InteractiveObject) IO = anActor->getIO();
	    Update( IO );
	  }
	}
	ac = actorList->GetNextActor();
      }
    } else {
      SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
      for ( ; It.More(); It.Next() ) {
	Handle(SALOME_InteractiveObject) IO = It.Value();
	Update( IO );
      }
    }
    vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
    renWin->Render();
  }
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::Update(const Handle(SALOME_InteractiveObject)& IO)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  if ( IO->hasEntry() ) {
    Standard_Boolean res;
    SMESH::SMESH_Mesh_var aMesh = ConvertIOinMesh( IO, res );
    if ( res ) {
      SMESH_Actor* ac = FindActorByEntry( IO->getEntry(), res, false );
      if ( res ) {
	// Check whether the actor belongs to the active view
	VTKViewer_RenderWindowInteractor* rwInter = 
	  ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRWInteractor();

	// The actor belongs to inactive view -> create a copy and display it in the active view
	if (!rwInter->isInViewer(IO)) {
	  SMESH_Actor* acCopy = SMESH_Actor::New();
	  acCopy->ShallowCopy(ac);

	  smeshGUI->DisplayActor( acCopy, false );
	}
	ac = ReadScript( aMesh );
	if ( ac != NULL ) {
#ifdef TRACE
	  Dump( ac );
#endif
	  DisplayActor( ac );
	  DisplayEdges( ac );
	  smeshGUI->ChangeRepresentation( ac, ac->getDisplayMode() );
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
vtkActor* SMESHGUI::SimulationMoveNode(SMESH_Actor* Mactor, int idnode)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return NULL;

  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();

  vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( Mactor->DataSource );
  vtkIdList *IdCells =  vtkIdList::New();
  ugrid->GetPointCells( idnode, IdCells );

  vtkPoints *Pts = vtkPoints::New();
  Pts = ugrid->GetPoints();

  vtkUnstructuredGrid *ugridSimulation = vtkUnstructuredGrid::New();
  ugridSimulation->SetPoints( Pts );
  vtkIdList *IdPts =  vtkIdList::New();
  for (int j=0; j<IdCells->GetNumberOfIds(); j++) {
    ugrid->GetCellPoints( IdCells->GetId(j), IdPts );
    ugridSimulation->InsertNextCell( ugrid->GetCellType( IdCells->GetId(j) ), IdPts );
  }

  vtkProperty* prop = vtkProperty::New();
  prop->SetColor( 1., 0., 0. );
  prop->SetRepresentationToWireframe();
  
  int Edgewidth = (int)Mactor->EdgeDevice->GetProperty()->GetLineWidth();
  if ( Edgewidth == 0 )
    Edgewidth = 1;
  prop->SetLineWidth( Edgewidth+1 );

  vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
  Mapper->SetInput( ugridSimulation );
  vtkActor* ac = vtkActor::New();
  ac->SetMapper( Mapper );
  ac->SetProperty( prop );

  ac->GetMapper()->SetResolveCoincidentTopologyToShiftZBuffer();
  ac->GetMapper()->SetResolveCoincidentTopologyZShift(0.02);

  mySimulationActors->AddItem( ac );
  theRenderer->AddActor( ac );
  
  vtkRenderWindow *renWin = theRenderer->GetRenderWindow();
  renWin->Render();

  return ac;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationNode( SMESH::SMESH_Mesh_ptr aMesh, float x, float y, float z)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer* theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );

  if ( result ) {
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
    vtkUnstructuredGrid* newugrid = vtkUnstructuredGrid::New();

    vtkPoints *Pts = ugrid->GetPoints();
    vtkPoints *newPts = vtkPoints::New();
    int nbPts = Pts->GetNumberOfPoints();
    for ( int i = 0; i < nbPts; i++ ) {
      newPts->InsertPoint(i, Pts->GetPoint(i) );
    }

    newugrid->SetPoints(newPts);
    newugrid->GetPoints()->InsertNextPoint( x, y, z );
    
    vtkMaskPoints* verts = vtkMaskPoints::New();
    verts->SetInput(newugrid);
    verts->SetGenerateVertices(1);
    verts->SetOnRatio(1);
   
    vtkPolyDataMapper* vertMapper = vtkPolyDataMapper::New();
    vertMapper->SetInput( verts->GetOutput() );
    vertMapper->ScalarVisibilityOff();

    vtkActor* node = vtkActor::New();
    node->SetMapper( vertMapper );

    QString SCr = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorRed");
    QString SCg = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorGreen");
    QString SCb = QAD_CONFIG->getSetting("SMESH:SettingsNodeColorBlue");
    QColor nodecolor(SCr.toInt(), SCg.toInt(), SCb.toInt());
    if ( !nodecolor.isValid() )
//       nodecolor = QColor(0.,1.,0.);
      nodecolor = QColor(0,1,0);

    node->GetProperty()->SetColor( float(nodecolor.red())/255.,
				   float(nodecolor.green())/255.,
				   float(nodecolor.blue())/255.);

    int intValue = QAD_CONFIG->getSetting("SMESH:SettingsNodesSize").toInt();
    if ( intValue < 1 )
      intValue == 1;
    
    node->GetProperty()->SetPointSize( intValue );

    node->VisibilityOn();

    mySimulationActors->AddItem( node );
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
void SMESHGUI::DisplaySimulationMoveNode( vtkActor* ac, int idnode, float x, float y, float z)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  float *pt  = ac->GetMapper()->GetInput()->GetPoint(idnode);
  pt[0] = x; pt[1] = y; pt[2] = z;

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
void SMESHGUI::MoveNode( SMESH::SMESH_Mesh_ptr aMesh, int idnode, float x, float y, float z)
{
  Standard_Boolean result;

  SMESH_Actor* MeshActor = FindActor(aMesh, result, true);
  if ( result ) {
  }
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ScalarVisibilityOff()
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  vtkActorCollection* actorList=theRenderer->GetActors();

  actorList->InitTraversal();
  vtkActor *ac = actorList->GetNextActor();
  while(!(ac==NULL)) {
    if ( ac->IsA("SMESH_Actor") ) {
      SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( ac );
      if ( anActor->GetVisibility() == 1 ) {
	vtkDataSetMapper* meshMapper = (vtkDataSetMapper*) (anActor->getMapper());
	meshMapper->ScalarVisibilityOff();
	meshMapper->SetInput( anActor->DataSource );
	ChangeRepresentation( anActor, anActor->getDisplayMode() );
      }
    }
    ac = actorList->GetNextActor();
  }

  theRenderer->Render();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplaySimulationEdge( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex )
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int idNodes[2];
    int pts[2];
    int i = 0;
    vtkIdList *Ids = vtkIdList::New();
    for ( ; ite.More(); ite.Next() ) {
      idNodes[i] = ite.Key();
      i++;
    }
    
    Ids->InsertId( 0, idNodes[0] );
    pts[0] = idNodes[0];
    Ids->InsertId( 1, idNodes[1] );
    pts[1] = idNodes[1];

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
    vtkUnstructuredGrid* newugrid = vtkUnstructuredGrid::New();
    newugrid->SetPoints( ugrid->GetPoints() ) ;
    newugrid->InsertNextCell(VTK_LINE, 2, pts);

    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( newugrid );
    Mapper->Update();

    vtkActor* edge = vtkActor::New();
    edge->SetMapper( Mapper );
    edge->SetProperty( ac->GetProperty() );
    edge->SetBackfaceProperty( ac->GetBackfaceProperty() );
    edge->VisibilityOn();
    mySimulationActors->AddItem( edge );
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
void SMESHGUI::DisplaySimulationTriangle( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse )
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int idNodes[3];
    int pts[3];
    int i = 0;
    vtkIdList *Ids = vtkIdList::New();
    for ( ; ite.More(); ite.Next() ) {
      idNodes[i] = ite.Key();
      i++;
    }
    
    if ( reverse ) {
      Ids->InsertId( 0, idNodes[2] );
      pts[0] = idNodes[2];
      Ids->InsertId( 1, idNodes[1] );   
      pts[1] = idNodes[1];
      Ids->InsertId( 2, idNodes[0] );
      pts[2] = idNodes[0];
    } else {
      Ids->InsertId( 0, idNodes[0] );
      pts[0] = idNodes[0];
      Ids->InsertId( 1, idNodes[1] );
      pts[1] = idNodes[1];
      Ids->InsertId( 2, idNodes[2] );  
      pts[2] = idNodes[2];     
    }
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
    vtkUnstructuredGrid* newugrid = vtkUnstructuredGrid::New();
    newugrid->SetPoints( ugrid->GetPoints() ) ;
    newugrid->InsertNextCell(VTK_TRIANGLE, 3, pts);

    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( newugrid );
    Mapper->Update();

    vtkActor* tri = vtkActor::New();
    tri->SetMapper( Mapper );
    tri->SetProperty( ac->GetProperty() );
    tri->SetBackfaceProperty( ac->GetBackfaceProperty() );
    tri->VisibilityOn();
    mySimulationActors->AddItem( tri );
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
void SMESHGUI::DisplaySimulationQuadrangle( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse )
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );

  vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
  
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int i = 0;
    int idNodes[4];
    vtkIdList *Ids = vtkIdList::New();
    for ( ; ite.More(); ite.Next() ) {
      idNodes[i] = ite.Key();
      i++;
    }

    float *p0 = ugrid->GetPoint(idNodes[0]);
    float *p1 = ugrid->GetPoint(idNodes[1]);
    float *p2 = ugrid->GetPoint(idNodes[2]);
    float *p3 = ugrid->GetPoint(idNodes[3]);

    gp_Pnt P0(p0[0],p0[1],p0[2]);

    gp_Vec V1( P0, gp_Pnt(p1[0],p1[1],p1[2]) );
    gp_Vec V2( P0, gp_Pnt(p2[0],p2[1],p2[2]) );
    gp_Vec V3( P0, gp_Pnt(p3[0],p3[1],p3[2]) );

    gp_Vec Cross1 = V1 ^ V2;
    gp_Vec Cross2 = V2 ^ V3;

    int tmp;
    if ( Cross1.Dot(Cross2) < 0 ) {
      V1 = gp_Vec(P0, gp_Pnt(p2[0],p2[1],p2[2]) );
      V2 = gp_Vec(P0, gp_Pnt(p1[0],p1[1],p1[2]) );
      Cross1 = V1 ^ V2;
      Cross2 = V2 ^ V3;
      
      if ( Cross1.Dot(Cross2) < 0 ) {
	tmp = idNodes[2];
	idNodes[2] = idNodes[3];
	idNodes[3] = tmp;	  
      } else {
	tmp = idNodes[1];
	idNodes[1] = idNodes[2];
	idNodes[2] = tmp;
      }
    }

    if ( reverse ) {
      Ids->InsertId( 0, idNodes[3] );
      Ids->InsertId( 1, idNodes[2] );   
      Ids->InsertId( 2, idNodes[1] );
      Ids->InsertId( 3, idNodes[0] );
    } else {
      Ids->InsertId( 0, idNodes[0] );
      Ids->InsertId( 1, idNodes[1] );
      Ids->InsertId( 2, idNodes[2] );  
      Ids->InsertId( 3, idNodes[3] );  
    }

    //    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
    vtkUnstructuredGrid* newugrid = vtkUnstructuredGrid::New();
    newugrid->SetPoints( ugrid->GetPoints() ) ;
    newugrid->InsertNextCell(VTK_QUAD, Ids);

    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( newugrid );
    Mapper->Update();

    vtkActor* quad = vtkActor::New();
    quad->SetMapper( Mapper );
    quad->SetProperty( ac->GetProperty() );
    quad->SetBackfaceProperty( ac->GetBackfaceProperty() );
    quad->VisibilityOn();
    mySimulationActors->AddItem( quad );
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
void SMESHGUI::DisplaySimulationTetra( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex )
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int i = 0;
    int idNodes[4];
    vtkIdList *Ids = vtkIdList::New();
    for ( ; ite.More(); ite.Next() ) {
      idNodes[i] = ite.Key();
      i++;
    }
    
    Ids->InsertId( 0, idNodes[0] );
    Ids->InsertId( 1, idNodes[1] );
    Ids->InsertId( 2, idNodes[2] );  
    Ids->InsertId( 3, idNodes[3] );  

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
    vtkUnstructuredGrid* newugrid = vtkUnstructuredGrid::New();
    newugrid->SetPoints( ugrid->GetPoints() ) ;
    newugrid->InsertNextCell(VTK_TETRA, Ids);

    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( newugrid );
    Mapper->Update();

    vtkActor* tetra = vtkActor::New();
    tetra->SetMapper( Mapper );
    tetra->SetProperty( ac->GetProperty() );
    tetra->SetBackfaceProperty( ac->GetBackfaceProperty() );
    tetra->VisibilityOn();
    mySimulationActors->AddItem( tetra );
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
void SMESHGUI::DisplaySimulationHexa( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex )
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors = vtkActorCollection::New();
  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    int i = 0;
    int idNodes[8];
    vtkIdList *Ids = vtkIdList::New();
    for ( ; ite.More(); ite.Next() ) {
      idNodes[i] = ite.Key();
      i++;
    }
    
    Ids->InsertId( 0, idNodes[0] );
    Ids->InsertId( 1, idNodes[1] );
    Ids->InsertId( 2, idNodes[2] );  
    Ids->InsertId( 3, idNodes[3] );  
    Ids->InsertId( 4, idNodes[4] );
    Ids->InsertId( 5, idNodes[5] );
    Ids->InsertId( 6, idNodes[6] );  
    Ids->InsertId( 7, idNodes[7] );  

    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
    vtkUnstructuredGrid* newugrid = vtkUnstructuredGrid::New();
    newugrid->SetPoints( ugrid->GetPoints() ) ;
    newugrid->InsertNextCell(VTK_HEXAHEDRON, Ids);

    vtkDataSetMapper *Mapper = vtkDataSetMapper::New();
    Mapper->SetInput( newugrid );
    Mapper->Update();

    vtkActor* hexa = vtkActor::New();
    hexa->SetMapper( Mapper );
    hexa->SetProperty( ac->GetProperty() );
    hexa->SetBackfaceProperty( ac->GetBackfaceProperty() );
    hexa->VisibilityOn();
    mySimulationActors->AddItem( hexa );
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
void SMESHGUI::AddFace( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse )
{  
  QApplication::setOverrideCursor( Qt::waitCursor );
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    SMESH::long_array_var anArrayOfIdeces = new  SMESH::long_array;
    anArrayOfIdeces->length(MapIndex.Extent());
    
    int i = 0;
    for ( ; ite.More(); ite.Next() ) {
      anArrayOfIdeces[i] = ac->GetIdSMESHDSNode(ite.Key());
      i++;
    }

    int tmp;
    if ( MapIndex.Extent() == 4 ) {
      int idNodes[4];
      int i = 0;
      TColStd_MapIteratorOfMapOfInteger ite1( MapIndex );
      for ( ; ite1.More(); ite1.Next() ) {
	idNodes[i] = ite1.Key();
	i++;
      }
      
      vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->DataSource );
      float *p0 = ugrid->GetPoint(idNodes[0]);
      float *p1 = ugrid->GetPoint(idNodes[1]);
      float *p2 = ugrid->GetPoint(idNodes[2]);
      float *p3 = ugrid->GetPoint(idNodes[3]);

      gp_Pnt P0(p0[0],p0[1],p0[2]);

      gp_Vec V1( P0, gp_Pnt(p1[0],p1[1],p1[2]) );
      gp_Vec V2( P0, gp_Pnt(p2[0],p2[1],p2[2]) );
      gp_Vec V3( P0, gp_Pnt(p3[0],p3[1],p3[2]) );

      gp_Vec Cross1 = V1 ^ V2;
      gp_Vec Cross2 = V2 ^ V3;

      if ( Cross1.Dot(Cross2) < 0 ) {

	V1 = gp_Vec(P0, gp_Pnt(p2[0],p2[1],p2[2]) );
	V2 = gp_Vec(P0, gp_Pnt(p1[0],p1[1],p1[2]) );
	Cross1 = V1 ^ V2;
	Cross2 = V2 ^ V3;
	
	if ( Cross1.Dot(Cross2) < 0 ) {
	  tmp = anArrayOfIdeces[2];
	  anArrayOfIdeces[2] = anArrayOfIdeces[3];
	  anArrayOfIdeces[3] = tmp;	  
	} else {
	  tmp = anArrayOfIdeces[1];
	  anArrayOfIdeces[1] = anArrayOfIdeces[2];
	  anArrayOfIdeces[2] = tmp;
	}
      }
    }

    //    int tmp;
    if ( reverse ) {
      for (i=0; i < (MapIndex.Extent()/2); i++) {
	tmp = anArrayOfIdeces[i];
	anArrayOfIdeces[i] = anArrayOfIdeces[MapIndex.Extent()-i-1];
	anArrayOfIdeces[MapIndex.Extent()-i-1] = tmp;
      }
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    aMeshEditor->AddFace(anArrayOfIdeces);
  }
  if ( myAutomaticUpdate ) {
    SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
    if ( Mesh != NULL ) {
      smeshGUI->DisplayActor( Mesh );
      smeshGUI->DisplayEdges( Mesh );
      smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
      AddActorInSelection( Mesh );
    }
  }
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddVolume( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex )
{  
  QApplication::setOverrideCursor( Qt::waitCursor );
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    SMESH::long_array_var anArrayOfIdeces = new  SMESH::long_array;
    anArrayOfIdeces->length(MapIndex.Extent());
    
    int i = 0;
    for ( ; ite.More(); ite.Next() ) {
      anArrayOfIdeces[i] = ac->GetIdSMESHDSNode(ite.Key());
      i++;
    }

    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    aMeshEditor->AddVolume(anArrayOfIdeces);
  }
  if ( myAutomaticUpdate ) {
    SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
    if ( Mesh != NULL ) {
      smeshGUI->DisplayActor( Mesh );
      smeshGUI->DisplayEdges( Mesh );
      smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
      AddActorInSelection( Mesh );
    }
  }
  QApplication::restoreOverrideCursor();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddEdge( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex )
{  
  QApplication::setOverrideCursor( Qt::waitCursor );
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    TColStd_MapIteratorOfMapOfInteger ite( MapIndex );
    SMESH::long_array_var anArrayOfIdeces = new  SMESH::long_array;
    anArrayOfIdeces->length(MapIndex.Extent());
    
    int i = 0;
    for ( ; ite.More(); ite.Next() ) {
      anArrayOfIdeces[i] = ac->GetIdSMESHDSNode(ite.Key());
      i++;
    }

    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    aMeshEditor->AddEdge(anArrayOfIdeces);
  }
  if ( myAutomaticUpdate ) {
    SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
    if ( Mesh != NULL ) {
      smeshGUI->DisplayActor( Mesh );
      smeshGUI->DisplayEdges( Mesh );
      smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
      AddActorInSelection( Mesh );
    }
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::AddNode( SMESH::SMESH_Mesh_ptr aMesh, float x, float y, float z)
{  
  QApplication::setOverrideCursor( Qt::waitCursor );
  Standard_Boolean result;
  SMESH_Actor* ac = FindActor( aMesh, result, true );
  if ( result ) {
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    aMeshEditor->AddNode(x,y,z);
  
    if ( myAutomaticUpdate ) {
      SMESH_Actor* Mesh = smeshGUI->ReadScript(aMesh);
      if ( Mesh != NULL ) {
	smeshGUI->DisplayActor( Mesh );
	smeshGUI->DisplayEdges( Mesh );
	smeshGUI->ChangeRepresentation( Mesh, Mesh->getDisplayMode() );
	AddActorInSelection( Mesh );
      }
    }
  }
  QApplication::restoreOverrideCursor();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::DisplayEdgesConnectivityLegendBox(vtkActor *ac)
{
  if (myActiveStudy->getActiveStudyFrame()->getTypeView() != VIEW_VTK) 
    return;

  EraseSimulationActors();
  mySimulationActors2D = vtkActor2DCollection::New();

  vtkRenderer *theRenderer = ((VTKViewer_ViewFrame*)myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();

  vtkGlyphSource2D *gs2 = vtkGlyphSource2D::New();
  gs2->SetGlyphTypeToTriangle();
  gs2->FilledOff();

  vtkLegendBoxActor *legend = vtkLegendBoxActor::New();
  legend->GetPositionCoordinate()->SetValue(0.025, 0.025, 0.);
  legend->GetPosition2Coordinate()->SetValue(0.3, 0.3, 0.); //relative to Position

  legend->SetNumberOfEntries(4);
  legend->SetEntryString( 0, tr( "SMESH_BOUNDARYEDGES" ) );
  legend->SetEntrySymbol( 0, gs2->GetOutput() );
  legend->SetEntryColor(0, ac->GetMapper()->GetLookupTable()->GetColor(0.) );
  legend->SetEntryString( 1,  tr( "SMESH_MANIFOLDEDGES" ) );
  legend->SetEntrySymbol( 1, gs2->GetOutput() );
  legend->SetEntryColor(1, ac->GetMapper()->GetLookupTable()->GetColor(0.666667) );
  legend->SetEntryString( 2, tr( "SMESH_NONMANIFOLDEDGES" ) );
  legend->SetEntrySymbol( 2, gs2->GetOutput() );
  legend->SetEntryColor(2, ac->GetMapper()->GetLookupTable()->GetColor(0.222222) );
  legend->SetEntryString( 3, tr( "SMESH_FEATUREEDGES" ) );
  legend->SetEntrySymbol( 3, gs2->GetOutput() );
  legend->SetEntryColor(3, ac->GetMapper()->GetLookupTable()->GetColor(0.444444) );
  legend->SetPadding( 5 );
  //  legend->GetProperty()->SetColor();

  mySimulationActors2D->AddItem( legend );
  theRenderer->AddActor2D( legend );

  // Update the view
  myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame()->Repaint();
}

//===============================================================================
// function : OnEditDelete()
// purpose  :
//===============================================================================
void SMESHGUI::OnEditDelete()
{
  if ( QAD_MessageBox::warn2
       ( QAD_Application::getDesktop(),
	 tr ("SMESH_WRN_WARNING"),
	 tr ("SMESH_REALLY_DELETE"),
	 tr ("SMESH_BUT_YES"), tr ("SMESH_BUT_NO"), 1, 0, 0 ) != 1 )
    return;
  
  int nbSf = myActiveStudy->getStudyFramesCount();
    
  Standard_Boolean found;
  SALOMEDS::Study_var aStudy = myActiveStudy->getStudyDocument();
  SALOMEDS::StudyBuilder_var aStudyBuilder = myStudy->NewBuilder();
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeIOR_var     anIOR;
  
  SALOME_Selection* Sel = SALOME_Selection::Selection( myActiveStudy->getSelection() );
  SALOME_ListIteratorOfListIO It( Sel->StoredIObjects() );
  for(;It.More();It.Next()) {
    Handle(SALOME_InteractiveObject) IObject = It.Value();
    if ( IObject->hasEntry() ) {
      SALOMEDS::SObject_var SO = myStudy->FindObjectID( IObject->getEntry() );
      
      /* Erase child graphical objects */
      SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(SO);
      for (; it->More();it->Next()) {
	SALOMEDS::SObject_var CSO= it->Value();
	if (CSO->FindAttribute(anAttr, "AttributeIOR") ) {
          anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);

	  for ( int i = 0; i < nbSf; i++ ) {
	    QAD_StudyFrame* sf = myActiveStudy->getStudyFrame(i);
	    if ( sf->getTypeView() == VIEW_VTK ) {
	      vtkRenderer* Renderer = ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
	      SMESH_Actor* ac = smeshGUI->FindActorByEntry( CSO->GetID(), found, false );
	      if ( found ) {
		Renderer->RemoveActor(ac->EdgeDevice);
		Renderer->RemoveActor(ac->EdgeShrinkDevice);
		Renderer->RemoveActor(ac);
	      }
	    }
	  }
	}
      }
      
      /* Erase main graphical object */
      for ( int i = 0; i < nbSf; i++ ) {
	QAD_StudyFrame* sf = myActiveStudy->getStudyFrame(i);
	if ( sf->getTypeView() == VIEW_VTK ) {
	  vtkRenderer* Renderer = ((VTKViewer_ViewFrame*)smeshGUI->myActiveStudy->getActiveStudyFrame()->getRightFrame()->getViewFrame())->getRenderer();
	  VTKViewer_RenderWindowInteractor* myRenderInter= ((VTKViewer_ViewFrame*)sf->getRightFrame()->getViewFrame())->getRWInteractor();
	  SMESH_Actor* ac = smeshGUI->FindActorByEntry( IObject->getEntry(), found, false );
	  if (found) {
	    Renderer->RemoveActor(ac->EdgeDevice);
	    Renderer->RemoveActor(ac->EdgeShrinkDevice);
	  }
	  myRenderInter->Remove( IObject );
	}
      }
      
      /* Erase objects in Study */
      SALOMEDS::SObject_var obj = myStudy->FindObjectID( IObject->getEntry() );
      if ( !obj->_is_nil() ) {
	QAD_Operation* op = new SALOMEGUI_ImportOperation( myActiveStudy );
	op->start();
	aStudyBuilder->RemoveObject(obj);
	op->finish();
      }

    } /* IObject->hasEntry() */
  }   /* more/next           */

  /* Clear any previous selection */
  Sel->ClearIObjects() ; 
  myActiveStudy->updateObjBrowser();
}
