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

// File   : SMESHGUI_DeleteGroupDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_DeleteGroupDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>

#include <SalomeApp_Study.h>
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>

#include <SVTK_Selection.h>
#include <SVTK_ViewWindow.h>

// Qt includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QListWidget>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#define SPACING 6
#define MARGIN  11

/*!
 *  Class       : SMESHGUI_DeleteGroupDlg
 *  Description : Delete groups and their contents
 */

//=================================================================================
// function : SMESHGUI_DeleteGroupDlg()
// purpose  : Constructor
//=================================================================================
SMESHGUI_DeleteGroupDlg::SMESHGUI_DeleteGroupDlg (SMESHGUI* theModule):
  QDialog(SMESH::GetDesktop(theModule)),
  mySMESHGUI(theModule),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule))
{
  setModal(false);
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout(this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  QWidget* aMainFrame = createMainFrame  (this);
  QWidget* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  myHelpFileName = "deleting_groups_page.html";

  Init();
}

//=================================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=================================================================================
QWidget* SMESHGUI_DeleteGroupDlg::createMainFrame (QWidget* theParent)
{
  QGroupBox* aMainGrp =
    new QGroupBox(tr("SELECTED_GROUPS"), theParent);
  QVBoxLayout* aLay = new QVBoxLayout(aMainGrp);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  myListBox = new QListWidget(aMainGrp);
  myListBox->setMinimumSize(150, 100);
  myListBox->setSelectionMode(QListWidget::NoSelection);
  //myListBox->setRowMode(QListBox::FitToWidth);
  myListBox->setFlow(QListWidget::LeftToRight);
  myListBox->setWrapping(true);

  aLay->addWidget(myListBox);
  
  return aMainGrp;
}

//=================================================================================
// function : createButtonFrame()
// purpose  : Create frame containing buttons
//=================================================================================
QWidget* SMESHGUI_DeleteGroupDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aFrame = new QGroupBox(theParent);

  myOkBtn     = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aFrame);
  myApplyBtn  = new QPushButton(tr("SMESH_BUT_APPLY"),           aFrame);
  myCloseBtn  = new QPushButton(tr("SMESH_BUT_CLOSE"),           aFrame);
  myHelpBtn   = new QPushButton(tr("SMESH_BUT_HELP"),            aFrame);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addSpacing(10);
  aLay->addWidget(myApplyBtn);
  aLay->addSpacing(10);
  aLay->addStretch();
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  // connect signals and slots
  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(reject()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  return aFrame;
}

//=================================================================================
// name    : ~SMESHGUI_DeleteGroupDlg()
// Purpose : Destructor
//=================================================================================
SMESHGUI_DeleteGroupDlg::~SMESHGUI_DeleteGroupDlg()
{
}

//=================================================================================
// function : Init()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=================================================================================
void SMESHGUI_DeleteGroupDlg::Init ()
{
  myBlockSelection = false;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(reject()));

  // set selection mode
  mySelectionMgr->installFilter(new SMESH_TypeFilter(SMESH::GROUP));
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  onSelectionDone();
}

//=================================================================================
// function : isValid()
// purpose  : Verify validity of input data
//=================================================================================
bool SMESHGUI_DeleteGroupDlg::isValid()
{
  if (myListBox->count() == 0) {
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                 tr("NO_SELECTED_GROUPS"));
    return false;
  }

  return !mySMESHGUI->isActiveStudyLocked();
}

//=================================================================================
// function : onApply()
// purpose  : SLOT called when "Apply" button pressed.
//=================================================================================
bool SMESHGUI_DeleteGroupDlg::onApply()
{
  if (!isValid())
    return false;

  SUIT_OverrideCursor wc;

  myBlockSelection = true;

  QList<SMESH::SMESH_GroupBase_var>::iterator anIter;
  for (anIter = myListGrp.begin(); anIter != myListGrp.end(); ++anIter) {
    SMESH::SMESH_Mesh_var aMesh = (*anIter)->GetMesh();
    if (!aMesh->_is_nil())
      aMesh->RemoveGroupWithContents(*anIter);
  }

  myListBox->clear();
  myListGrp.clear();
  mySelectionMgr->clearSelected();

  /** Erase graphical objects **/
  SALOME_ListIteratorOfListIO anIterIO (myListGrpIO);
  for ( ; anIterIO.More(); anIterIO.Next())
    SMESH::RemoveVisualObjectWithActors( anIterIO.Value()->getEntry(), /*fromAllViews=*/true );

  SMESH::UpdateView();
  SMESHGUI::Modified();
  mySMESHGUI->updateObjBrowser(true);

  myBlockSelection = false;
  return true;
}

//=================================================================================
// function : onOk()
// purpose  : SLOT called when "Ok" button pressed.
//=================================================================================
void SMESHGUI_DeleteGroupDlg::onOk()
{
  if (onApply())
    reject();
}

//=================================================================================
// function : reject()
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=================================================================================
void SMESHGUI_DeleteGroupDlg::reject()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  mySelectionMgr->clearFilters();
  QDialog::reject();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_DeleteGroupDlg::onHelp()
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
// function : onSelectionDone()
// purpose  : SLOT called when selection changed
//=================================================================================
void SMESHGUI_DeleteGroupDlg::onSelectionDone()
{
  if (myBlockSelection)
    return;

  myListGrp.clear();
  myListGrpIO.Clear();
  QStringList aNames;

  SALOME_ListIO aListIO;
  mySelectionMgr->selectedObjects(aListIO);
  SALOME_ListIteratorOfListIO anIter (aListIO);
  for ( ; anIter.More(); anIter.Next()) {
    SMESH::SMESH_GroupBase_var aGroup =
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIter.Value());
    if (!aGroup->_is_nil()) {
      aNames.append(aGroup->GetName());
      myListGrp.append(aGroup);
      myListGrpIO.Append( anIter.Value() );
    }
  }

  myListBox->clear();
  myListBox->addItems(aNames);
}

//=================================================================================
// function : onDeactivate()
// purpose  : SLOT called when dialog must be deativated
//=================================================================================
void SMESHGUI_DeleteGroupDlg::onDeactivate()
{
  mySelectionMgr->clearFilters();
  setEnabled(false);
}

//=================================================================================
// function : enterEvent()
// purpose  : Event filter
//=================================================================================
void SMESHGUI_DeleteGroupDlg::enterEvent (QEvent*)
{
  mySMESHGUI->EmitSignalDeactivateDialog();
  setEnabled(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->installFilter(new SMESH_TypeFilter (SMESH::GROUP));
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_DeleteGroupDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}
