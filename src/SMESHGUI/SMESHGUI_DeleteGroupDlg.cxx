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
//  File   : SMESHGUI_DeleteGroupDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_DeleteGroupDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMESH_TypeFilter.hxx"

#include "SUIT_Desktop.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "SalomeApp_Study.h"
#include "LightApp_Application.h"
#include "LightApp_SelectionMgr.h"

#include "SALOME_ListIO.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "SVTK_Selection.h"
#include "SVTK_ViewWindow.h"

// QT Includes
#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlist.h>
#include <qmessagebox.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#define SPACING 5
#define MARGIN  10

/*!
 *  Class       : SMESHGUI_DeleteGroupDlg
 *  Description : Delete groups and their contents
 */

//=================================================================================
// function : SMESHGUI_DeleteGroupDlg()
// purpose  : Constructor
//=================================================================================
SMESHGUI_DeleteGroupDlg::SMESHGUI_DeleteGroupDlg (SMESHGUI* theModule):
  QDialog(SMESH::GetDesktop(theModule), 
	  "SMESHGUI_DeleteGroupDlg", 
	  false,
	  WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  setCaption(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout(this, MARGIN, SPACING);

  QFrame* aMainFrame = createMainFrame  (this);
  QFrame* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);

  myHelpFileName = "deleting_groups.htm";

  Init();
}

//=================================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=================================================================================
QFrame* SMESHGUI_DeleteGroupDlg::createMainFrame (QWidget* theParent)
{
  QGroupBox* aMainGrp =
    new QGroupBox(1, Qt::Horizontal, tr("SELECTED_GROUPS"), theParent);

  myListBox = new QListBox(aMainGrp);
  myListBox->setMinimumHeight(100);
  myListBox->setSelectionMode(QListBox::NoSelection);
  myListBox->setRowMode(QListBox::FitToWidth);

  return aMainGrp;
}

//=================================================================================
// function : createButtonFrame()
// purpose  : Create frame containing buttons
//=================================================================================
QFrame* SMESHGUI_DeleteGroupDlg::createButtonFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);
  aFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);

  myOkBtn     = new QPushButton(tr("SMESH_BUT_OK"   ), aFrame);
  myApplyBtn  = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn  = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn  = new QPushButton(tr("SMESH_BUT_HELP"), aFrame);

  QSpacerItem* aSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame, MARGIN, SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addWidget(myApplyBtn);
  aLay->addItem(aSpacer);
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  // connect signals and slots
  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));
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
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(onClose()));

  this->show();

  // set selection mode
  mySelectionMgr->installFilter(new SMESH_TypeFilter(GROUP));
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  onSelectionDone();

  return;
}

//=================================================================================
// function : isValid()
// purpose  : Verify validity of input data
//=================================================================================
bool SMESHGUI_DeleteGroupDlg::isValid()
{
  if (myListBox->count() == 0) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                             tr("NO_SELECTED_GROUPS"), QMessageBox::Ok);
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

  myBlockSelection = true;

  QValueList<SMESH::SMESH_GroupBase_var>::iterator anIter;
  for (anIter = myListGrp.begin(); anIter != myListGrp.end(); ++anIter) {
    SMESH::SMESH_Mesh_ptr aMesh = (*anIter)->GetMesh();
    if (!aMesh->_is_nil())
      aMesh->RemoveGroupWithContents(*anIter);
  }

  myListBox->clear();
  myListGrp.clear();
  mySelectionMgr->clearSelected();
  SMESH::UpdateView();
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
    onClose();
}

//=================================================================================
// function : onClose()
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=================================================================================
void SMESHGUI_DeleteGroupDlg::onClose()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  mySelectionMgr->clearFilters();
  reject();
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
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
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
  QStringList aNames;

  SALOME_ListIO aListIO;
  mySelectionMgr->selectedObjects(aListIO);
  SALOME_ListIteratorOfListIO anIter (aListIO);
  for (; anIter.More(); anIter.Next()) {
    SMESH::SMESH_GroupBase_var aGroup =
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIter.Value());
    if (!aGroup->_is_nil()) {
      aNames.append(aGroup->GetName());
      myListGrp.append(aGroup);
    }
  }

  myListBox->clear();
  myListBox->insertStringList(aNames);
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
  mySelectionMgr->installFilter(new SMESH_TypeFilter (GROUP));
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_DeleteGroupDlg::closeEvent (QCloseEvent*)
{
  onClose();
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

  if ( e->key() == Key_F1 )
    {
      e->accept();
      onHelp();
    }
}
