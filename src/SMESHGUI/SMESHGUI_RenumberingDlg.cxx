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
// File   : SMESHGUI_RenumberingDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_RenumberingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QButtonGroup>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

//=================================================================================
// class    : SMESHGUI_RenumberingDlg()
// purpose  :
//=================================================================================
SMESHGUI_RenumberingDlg::SMESHGUI_RenumberingDlg( SMESHGUI* theModule, const int unit)
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  myUnit = unit;

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(unit == 0 ? 
                 tr("SMESH_RENUMBERING_NODES_TITLE") : 
                 tr("SMESH_RENUMBERING_ELEMENTS_TITLE"));
  setSizeGripEnabled(true);

  SUIT_ResourceMgr* resMgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap image0(resMgr->loadPixmap("SMESH", unit == 0 ? 
                                    tr("ICON_DLG_RENUMBERING_NODES") : 
                                    tr("ICON_DLG_RENUMBERING_ELEMENTS")));
  QPixmap image1(resMgr->loadPixmap("SMESH",tr("ICON_SELECT")));

  QVBoxLayout* SMESHGUI_RenumberingDlgLayout = new QVBoxLayout(this);
  SMESHGUI_RenumberingDlgLayout->setSpacing(SPACING);
  SMESHGUI_RenumberingDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(unit == 0 ? 
                                    tr("SMESH_NODES") :
                                    tr("SMESH_ELEMENTS"), 
                                    this);
  myHelpFileName = unit == 0 ? 
    "renumbering_nodes_and_elements_page.html#renumbering_nodes_anchor" :
    "renumbering_nodes_and_elements_page.html#renumbering_elements_anchor";

  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  GroupConstructorsLayout->setSpacing(SPACING);
  GroupConstructorsLayout->setMargin(MARGIN);

  Constructor1 = new QRadioButton(GroupConstructors);
  Constructor1->setIcon(image0);
  Constructor1->setChecked(true);

  GroupConstructorsLayout->addWidget(Constructor1);
  ButtonGroup->addButton(Constructor1, 0);

  /***************************************************************/
  GroupMesh = new QGroupBox(tr("SMESH_RENUMBERING"), this);
  QHBoxLayout* GroupMeshLayout = new QHBoxLayout(GroupMesh);
  GroupMeshLayout->setMargin(MARGIN);
  GroupMeshLayout->setSpacing(SPACING);

  TextLabelMesh = new QLabel(tr("SMESH_MESH"), GroupMesh);
  SelectButton = new QPushButton(GroupMesh);
  SelectButton->setIcon(image1);
  LineEditMesh = new QLineEdit(GroupMesh);
  LineEditMesh->setReadOnly(true);

  GroupMeshLayout->addWidget(TextLabelMesh);
  GroupMeshLayout->addWidget(SelectButton);
  GroupMeshLayout->addWidget(LineEditMesh);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  SMESHGUI_RenumberingDlgLayout->addWidget(GroupConstructors);
  SMESHGUI_RenumberingDlgLayout->addWidget(GroupMesh);
  SMESHGUI_RenumberingDlgLayout->addWidget(GroupButtons);

  Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_RenumberingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RenumberingDlg::~SMESHGUI_RenumberingDlg()
{
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::Init()
{
  myConstructorId = 0;
  Constructor1->setChecked(true);
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myMesh = SMESH::SMESH_Mesh::_nil();

  myMeshFilter = new SMESH_TypeFilter (SMESH::MESH);

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectButton, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(reject()));

  myEditCurrentArgument = LineEditMesh;
  LineEditMesh->setFocus();
  mySelectionMgr->clearFilters();
  mySelectionMgr->installFilter(myMeshFilter);

  SelectionIntoArgument();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (!myMesh->_is_nil()) {
    try {
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
      SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
      bool isUnitsLabeled = false;
      
      if (myUnit == 0 && anActor) {
        isUnitsLabeled = anActor->GetPointsLabeled();
        if (isUnitsLabeled)  anActor->SetPointsLabeled(false);
      }
      else if (myUnit == 1 && anActor) {
        isUnitsLabeled = anActor->GetCellsLabeled();
        if (isUnitsLabeled)  anActor->SetCellsLabeled(false);
      }
      
      SUIT_OverrideCursor aWaitCursor;
      if (myUnit == 0) {
        aMeshEditor->RenumberNodes();
        if (isUnitsLabeled && anActor) anActor->SetPointsLabeled(true);
      }
      else if (myUnit == 1) {
        aMeshEditor->RenumberElements();
        if (isUnitsLabeled && anActor) anActor->SetCellsLabeled(true);
      }
    }
    catch(...) {
    }
    
    //mySelectionMgr->clearSelected();
    SMESH::UpdateView();
    SMESHGUI::Modified();
  }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnOk()
{
  ClickOnApply();
  reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::reject()
{
  //mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnHelp()
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
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser", 
                                                                 platform)).
                             arg(myHelpFileName));
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RenumberingDlg::SelectionIntoArgument()
{
  if (!GroupButtons->isEnabled()) // inactive
    return;

  QString aString = "";

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);
  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

  if (myEditCurrentArgument == LineEditMesh) {
    // mesh
    if (nbSel != 1) {
      myMesh = SMESH::SMESH_Mesh::_nil();
      aString = "";
    } else {
      Handle(SALOME_InteractiveObject) IO = aList.First();
      myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
      if (myMesh->_is_nil())
        aString = "";
    }
  }

  myEditCurrentArgument->setText(aString);

  bool isEnabled = (!myMesh->_is_nil());
  buttonOk->setEnabled(isEnabled);
  buttonApply->setEnabled(isEnabled);
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {
        if(send == SelectButton) {
          LineEditMesh->setFocus();
          myEditCurrentArgument = LineEditMesh;
        }
        SelectionIntoArgument();
        break;
      }
    }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupMesh->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupMesh->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::enterEvent(QEvent* e)
{
  if (GroupConstructors->isEnabled())
    return;
  ActivateThisDialog();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
