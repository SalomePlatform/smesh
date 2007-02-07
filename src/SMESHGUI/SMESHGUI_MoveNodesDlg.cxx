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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_MoveNodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_MoveNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"

#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"

#include "LightApp_SelectionMgr.h"
#include "LightApp_Application.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "SVTK_Selector.h"
#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SALOME_ListIO.hxx"

#include "SVTK_ViewWindow.h"
#include "VTKViewer_CellLocationsArray.h"

#include "utilities.h"

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// VTK includes
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

// QT includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#define MARGIN  10
#define SPACING 5


//=================================================================================
// name    : SMESHGUI_MoveNodesDlg::SMESHGUI_MoveNodesDlg
// Purpose :
//=================================================================================
SMESHGUI_MoveNodesDlg::SMESHGUI_MoveNodesDlg (SMESHGUI* theModule, 
					      const char* theName):
  QDialog(SMESH::GetDesktop(theModule), 
	  theName, 
	  false,
	  WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  myPreviewActor = 0;
  myBusy = false;

  setCaption(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (this, MARGIN, SPACING);

  QFrame* aMainFrame = createMainFrame  (this);
  QFrame* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  myHelpFileName = "/files/displacing_nodes.htm";

  Init();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_MoveNodesDlg::createButtonFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);
  aFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);

  myOkBtn     = new QPushButton(tr("SMESH_BUT_OK"   ), aFrame);
  myApplyBtn  = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn  = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn   = new QPushButton(tr("SMESH_BUT_HELP"), aFrame);

  QSpacerItem* aSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame, MARGIN, SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addWidget(myApplyBtn);
  aLay->addItem(aSpacer);
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_MoveNodesDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);

  QPixmap iconMoveNode (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_MOVE_NODE")));
  QPixmap iconSelect   (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  QButtonGroup* aPixGrp = new QButtonGroup(1, Qt::Vertical, tr("MESH_NODE"), aFrame);
  aPixGrp->setExclusive(TRUE);
  QRadioButton* aRBut = new QRadioButton(aPixGrp);
  aRBut->setPixmap(iconMoveNode);
  aRBut->setChecked(TRUE);

  QGroupBox* anIdGrp = new QGroupBox(1, Qt::Vertical, tr("SMESH_MOVE"), aFrame);
  new QLabel(tr("NODE_ID"), anIdGrp);
  (new QPushButton(anIdGrp))->setPixmap(iconSelect);
  myId = new QLineEdit(anIdGrp);
  myId->setValidator(new SMESHGUI_IdValidator(this, "validator", 1));

  QGroupBox* aCoordGrp = new QGroupBox(1, Qt::Vertical, tr("SMESH_COORDINATES"), aFrame);
  new QLabel(tr("SMESH_X"), aCoordGrp);
  myX = new SMESHGUI_SpinBox(aCoordGrp);
  new QLabel(tr("SMESH_Y"), aCoordGrp);
  myY = new SMESHGUI_SpinBox(aCoordGrp);
  new QLabel(tr("SMESH_Z"), aCoordGrp);
  myZ = new SMESHGUI_SpinBox(aCoordGrp);

  myX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 25.0, 3);
  myY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 25.0, 3);
  myZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 25.0, 3);

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(aPixGrp);
  aLay->addWidget(anIdGrp);
  aLay->addWidget(aCoordGrp);

  // connect signale and slots
  connect(myX, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myY, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myZ, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myId, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg
// Purpose :
//=======================================================================
SMESHGUI_MoveNodesDlg::~SMESHGUI_MoveNodesDlg()
{
  erasePreview();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::Init
// Purpose : Init dialog fields
//=======================================================================
void SMESHGUI_MoveNodesDlg::Init()
{
  myPreviewActor = 0;
  myMeshActor = 0;
  myBusy = false;

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(onClose()));

  reset();
  setEnabled(true);

  this->show();

  // set selection mode
  SMESH::SetPointRepresentation(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  onSelectionDone();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::isValid
// Purpose : Verify validity of entry information
//=======================================================================
bool SMESHGUI_MoveNodesDlg::isValid (const bool theMess) const
{
  if (myId->text().isEmpty()) {
    if (theMess)
      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_WARNING"),
                               tr("NODE_ID_IS_NOT_DEFINED"), QMessageBox::Ok);
    return false;
  }
  return true;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::reset
// Purpose : Reset the dialog state
//=======================================================================
void SMESHGUI_MoveNodesDlg::reset()
{
  myId->clear();
  myX->SetValue(0);
  myY->SetValue(0);
  myZ->SetValue(0);
  redisplayPreview();
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onApply
// Purpose : SLOT called when "Apply" button pressed.
//=======================================================================
bool SMESHGUI_MoveNodesDlg::onApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (!isValid(true))
    return false;

  SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
  if (aMesh->_is_nil()) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("SMESHG_NO_MESH"), QMessageBox::Ok);
    return false;
  }

  SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
  if (aMeshEditor->_is_nil())
    return false;

  int anId = myId->text().toInt();
  bool aResult = false;
  try {
    aResult = aMeshEditor->MoveNode(anId, myX->GetValue(), myY->GetValue(), myZ->GetValue());
  } catch (...) {
  }

  if (aResult) {
    SALOME_ListIO aList;
    aList.Append(myMeshActor->getIO());
    mySelectionMgr->setSelectedObjects(aList,false);
    SMESH::UpdateView();
    reset();
  }

  return aResult;
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_MoveNodesDlg::onOk()
{
  if (onApply())
    onClose();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_MoveNodesDlg::onClose()
{
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  erasePreview();
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
		QString platform;
#ifdef WIN32
		platform = "winapplication";
#else
		platform = "application";
#endif
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onTextChange
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;

  myOkBtn->setEnabled(false);
  myApplyBtn->setEnabled(false);
  erasePreview();

  // select entered node
  if(myMeshActor){
    if(SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh()){
      myBusy = true;
      Handle(SALOME_InteractiveObject) anIO = myMeshActor->getIO();
      SALOME_ListIO aList;
      aList.Append(anIO);
      mySelectionMgr->setSelectedObjects(aList,false);
      myBusy = false;

      if(const SMDS_MeshElement *anElem = aMesh->FindElement(theNewText.toInt())) {
	TColStd_MapOfInteger aListInd;
	aListInd.Add(anElem->GetID());
	mySelector->AddOrRemoveIndex(anIO,aListInd, false);
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->highlight(anIO,true,true);
	
	onSelectionDone();
      }
    }
  }
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_MoveNodesDlg::onSelectionDone()
{
  if (myBusy) return;
  myMeshActor = 0;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  if (aList.Extent() == 1) {
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    myMeshActor = SMESH::FindActorByEntry(anIO->getEntry());
    if(myMeshActor){
      QString aText;
      if (SMESH::GetNameOfSelectedNodes(mySelector,anIO,aText) == 1) {
        if(SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh()) {
          if(const SMDS_MeshNode* aNode = aMesh->FindNode(aText.toInt())) {
            myBusy = true;
            myId->setText(aText);
            myX->SetValue(aNode->X());
            myY->SetValue(aNode->Y());
            myZ->SetValue(aNode->Z());
            myBusy = false;
            erasePreview(); // avoid overlapping of a selection and a preview
            updateButtons();
            return;
          }
        }
      }
    }
  }

  reset();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_MoveNodesDlg::onDeactivate()
{
  setEnabled(false);
  erasePreview();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_MoveNodesDlg::enterEvent (QEvent*)
{
  if (!isEnabled()) {
    mySMESHGUI->EmitSignalDeactivateDialog();

    // set selection mode
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);

    redisplayPreview();

    setEnabled(true);
  }
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::closeEvent
// Purpose :
//=======================================================================
void SMESHGUI_MoveNodesDlg::closeEvent (QCloseEvent*)
{
  onClose();
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->Repaint();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::hideEvent
// Purpose : may be caused by ESC key
//=======================================================================
void SMESHGUI_MoveNodesDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    onClose();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::updateButtons
// Purpose : Update buttons state
//=======================================================================
void SMESHGUI_MoveNodesDlg::updateButtons()
{
  bool isEnabled = isValid(false);
  myOkBtn->setEnabled(isEnabled);
  myApplyBtn->setEnabled(isEnabled);
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::erasePreview
// Purpose : Erase preview
//=======================================================================
void  SMESHGUI_MoveNodesDlg::erasePreview()
{
  if (myPreviewActor == 0)
    return;

  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
  if (aViewWindow)
    aViewWindow->RemoveActor(myPreviewActor);
  myPreviewActor->Delete();
  myPreviewActor = 0;
  if (aViewWindow)
    aViewWindow->Repaint();
}

//=======================================================================
// name    : SMESHGUI_MoveNodesDlg::redisplayPreview
// Purpose : Redisplay preview
//=======================================================================
void SMESHGUI_MoveNodesDlg::redisplayPreview()
{
  if (myBusy)
    return;

  if (myPreviewActor != 0)
    erasePreview();

  if (!isValid(false))
    return;

  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints(1);
  aPoints->SetPoint(0, myX->GetValue(), myY->GetValue(), myZ->GetValue());

  // Create cells

  vtkIdList *anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds(1);

  vtkCellArray *aCells = vtkCellArray::New();
  aCells->Allocate(2, 0);

  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents(1);
  aCellTypesArray->Allocate(1);

  anIdList->SetId(0, 0);
  aCells->InsertNextCell(anIdList);
  aCellTypesArray->InsertNextValue(VTK_VERTEX);
  anIdList->Delete();

  VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
  aCellLocationsArray->SetNumberOfComponents(1);
  aCellLocationsArray->SetNumberOfTuples(1);

  aCells->InitTraversal();
  vtkIdType npts;
  aCellLocationsArray->SetValue(0, aCells->GetTraversalLocation(npts));

  aGrid->SetPoints(aPoints);
  aPoints->Delete();

  aGrid->SetCells(aCellTypesArray,aCellLocationsArray,aCells);
  aCellLocationsArray->Delete();
  aCellTypesArray->Delete();
  aCells->Delete();

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInput(aGrid);
  aGrid->Delete();

  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->PickableOff();
  myPreviewActor->SetMapper(aMapper);
  aMapper->Delete();

  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor(250, 0, 250);
  aProp->SetPointSize(5);
  myPreviewActor->SetProperty(aProp);
  aProp->Delete();

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    {
      aViewWindow->AddActor(myPreviewActor);
      aViewWindow->Repaint();
    }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MoveNodesDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Key_F1 )
    {
      e->accept();
      onHelp();
    }
}
