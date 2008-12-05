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
// File   : SMESHGUI_GroupOpDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_GroupOpDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SVTK_Selection.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// Qt includes
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

#define SPACING 6
#define MARGIN  11

/*!
 *  Class       : SMESHGUI_GroupOpDlg
 *  Description : Perform boolean operations on groups
 */

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg( SMESHGUI* theModule, const int theMode )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  setModal(false);

  myMode = theMode;

  if (myMode == UNION) {
    setWindowTitle(tr("UNION_OF_TWO_GROUPS"));
    myHelpFileName = "using_operations_on_groups_page.html#union_anchor";
  }
  else if (myMode == INTERSECT) {
    setWindowTitle(tr("INTERSECTION_OF_TWO_GROUPS"));
    myHelpFileName = "using_operations_on_groups_page.html#intersection_anchor";
  }
  else {
    setWindowTitle(tr("CUT_OF_TWO_GROUPS"));
    myHelpFileName = "using_operations_on_groups_page.html#cut_anchor";
  }

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  QVBoxLayout* aDlgLay = new QVBoxLayout (this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  QWidget* aMainFrame = createMainFrame  (this);
  QWidget* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  Init();
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_GroupOpDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aMainGrp = new QWidget(theParent);
  QVBoxLayout* aLay = new QVBoxLayout(aMainGrp);
  aLay->setMargin(0);
  aLay->setSpacing(SPACING);
  
  // ------------------------------------------------------
  QGroupBox* aNameGrp = new QGroupBox(tr("NAME"), aMainGrp);
  QHBoxLayout* aNameGrpLayout = new QHBoxLayout(aNameGrp);
  aNameGrpLayout->setMargin(MARGIN);
  aNameGrpLayout->setSpacing(SPACING);

  QLabel* aNameLab = new QLabel(tr("RESULT_NAME"), aNameGrp);
  myNameEdit = new QLineEdit(aNameGrp);

  aNameGrpLayout->addWidget(aNameLab);
  aNameGrpLayout->addWidget(myNameEdit);

  // ------------------------------------------------------
  QGroupBox* anArgGrp = new QGroupBox(tr("ARGUMENTS"), aMainGrp);
  QGridLayout* anArgGrpLayout = new QGridLayout(anArgGrp);
  anArgGrpLayout->setMargin(MARGIN);
  anArgGrpLayout->setSpacing(SPACING);

  QLabel* aObj1Lab = new QLabel(myMode == CUT ? tr("MAIN_OBJECT") :tr("OBJECT_1"), anArgGrp);
  myBtn1 = new QPushButton(anArgGrp);
  myEdit1 = new QLineEdit(anArgGrp);
  myEdit1->setAlignment( Qt::AlignLeft );

  QLabel* aObj2Lab = new QLabel(myMode == CUT ? tr("TOOL_OBJECT") :tr("OBJECT_2"), anArgGrp);
  myBtn2 = new QPushButton(anArgGrp);
  myEdit2 = new QLineEdit(anArgGrp);
  myEdit2->setAlignment( Qt::AlignLeft );

  myEdit1->setReadOnly(true);
  myEdit2->setReadOnly(true);

  QPixmap aPix (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));
  myBtn1->setIcon(aPix);
  myBtn2->setIcon(aPix);

  anArgGrpLayout->addWidget(aObj1Lab, 0, 0);
  anArgGrpLayout->addWidget(myBtn1,   0, 1);
  anArgGrpLayout->addWidget(myEdit1,  0, 2);
  anArgGrpLayout->addWidget(aObj2Lab, 1, 0);
  anArgGrpLayout->addWidget(myBtn2,   1, 1);
  anArgGrpLayout->addWidget(myEdit2,  1, 2);

  // ------------------------------------------------------
  aLay->addWidget(aNameGrp);
  aLay->addWidget(anArgGrp);

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QWidget* SMESHGUI_GroupOpDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aFrame = new QGroupBox(theParent);

  myOkBtn    = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aFrame);
  myApplyBtn = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn  = new QPushButton(tr("SMESH_BUT_HELP"),  aFrame);

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
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::~SMESHGUI_GroupOpDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_GroupOpDlg::~SMESHGUI_GroupOpDlg()
{
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_GroupOpDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myFocusWg = myEdit1;

  myGroup1 = SMESH::SMESH_GroupBase::_nil();
  myGroup2 = SMESH::SMESH_GroupBase::_nil();

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(ClickOnClose()));

  connect(myBtn1, SIGNAL(clicked()), this, SLOT(onFocusChanged()));
  connect(myBtn2, SIGNAL(clicked()), this, SLOT(onFocusChanged()));

  // set selection mode
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->installFilter(new SMESH_TypeFilter (GROUP));
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_GroupOpDlg::isValid()
{
  // Verify validity of group name
  if (myNameEdit->text() == "") {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
				 tr("EMPTY_NAME"));
    return false;
  }

  // Verufy wheter arguments speciffiyed
  if (myGroup1->_is_nil() || myGroup2->_is_nil()) {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
				 tr("INCORRECT_ARGUMENTS"));
    return false;
  }

  // Verify whether arguments belongs to same mesh
  SMESH::SMESH_Mesh_ptr aMesh1 = myGroup1->GetMesh();
  SMESH::SMESH_Mesh_ptr aMesh2 = myGroup2->GetMesh();

  int aMeshId1 = !aMesh1->_is_nil() ? aMesh1->GetId() : -1;
  int aMeshId2 = !aMesh2->_is_nil() ? aMesh2->GetId() : -1;

  if (aMeshId1 != aMeshId2 || aMeshId1 == -1) {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
				 tr("DIFF_MESHES"));
    return false;
  }

  // Verify whether groups have same types of entities
  if (myGroup1->GetType() != myGroup2->GetType()) {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
				 tr("DIFF_TYPES"));
    return false;
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::onApply
// Purpose : SLOT called when "Apply" button pressed.
//=======================================================================
bool SMESHGUI_GroupOpDlg::onApply()
{
  if (!isValid() || mySMESHGUI->isActiveStudyLocked())
    return false;

  SMESH::SMESH_Mesh_ptr aMesh = myGroup1->GetMesh();
  QString aName = myNameEdit->text();
  SMESH::SMESH_Group_ptr aNewGrp = SMESH::SMESH_Group::_nil();

  if (myMode == UNION) aNewGrp = aMesh->UnionGroups(myGroup1, myGroup2, aName.toLatin1().data());
  else if (myMode == INTERSECT) aNewGrp = aMesh->IntersectGroups(myGroup1, myGroup2, aName.toLatin1().data());
  else aNewGrp = aMesh->CutGroups(myGroup1, myGroup2, aName.toLatin1().data());

  if (!aNewGrp->_is_nil()) {
    mySMESHGUI->updateObjBrowser(true);
    reset();
    return true;
  } else {
    SUIT_MessageBox::critical(this, tr("SMESH_ERROR"),
			      tr("SMESH_OPERATION_FAILED"));
    return false;
  }
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_GroupOpDlg::onOk()
{
  if (onApply())
    onClose();
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_GroupOpDlg::onClose()
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
void SMESHGUI_GroupOpDlg::onHelp()
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

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_GroupOpDlg::onSelectionDone()
{
  if (myFocusWg == myEdit1)
    myGroup1 = SMESH::SMESH_GroupBase::_nil();
  else
    myGroup2 = SMESH::SMESH_GroupBase::_nil();

  myFocusWg->setText("");

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  if (aList.Extent() == 1) {
    SMESH::SMESH_GroupBase_var aGroup =
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(aList.First());

    if (!aGroup->_is_nil())
    {
      myFocusWg->setText(aGroup->GetName());
      myFocusWg->setCursorPosition( 0 );

      if (myFocusWg == myEdit1)
        myGroup1 = aGroup;
      else
        myGroup2 = aGroup;
    }
  }
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_GroupOpDlg::onDeactivate()
{
  setEnabled(false);
  mySelectionMgr->clearFilters();
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_GroupOpDlg::enterEvent (QEvent*)
{
  mySMESHGUI->EmitSignalDeactivateDialog();
  setEnabled(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->installFilter(new SMESH_TypeFilter (GROUP));
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::closeEvent
// purpose :
//=======================================================================
void SMESHGUI_GroupOpDlg::closeEvent (QCloseEvent*)
{
  onClose();
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::onFocusChanged
// Purpose : SLOT. Called when "Select" button pressed.
//=======================================================================
void SMESHGUI_GroupOpDlg::onFocusChanged()
{
  const QObject* aSender = sender();
  myFocusWg = aSender == myBtn1 ? myEdit1 : myEdit2;
  onSelectionDone();
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::reset
// Purpose : Rest state of dialog
//=======================================================================
void SMESHGUI_GroupOpDlg::reset()
{
  myNameEdit->setText("");
  myEdit1->setText("");
  myEdit2->setText("");
  myFocusWg = myEdit1;
  myNameEdit->setFocus();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_GroupOpDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}
