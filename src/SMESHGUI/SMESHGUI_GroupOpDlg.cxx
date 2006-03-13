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
//  File   : SMESHGUI_GroupOpDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_GroupOpDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMESH_TypeFilter.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"

#include "LightApp_SelectionMgr.h"
#include "SVTK_Selection.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SALOME_ListIO.hxx"

// QT Includes
#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#define SPACING 5
#define MARGIN  10

/*!
 *  Class       : SMESHGUI_GroupOpDlg
 *  Description : Perform boolean operations on groups
 */

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg( SMESHGUI* theModule, const int theMode )
     : QDialog( SMESH::GetDesktop( theModule ), "SMESHGUI_GroupOpDlg", false,
                WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu ),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  myMode = theMode;

  if (myMode == UNION) setCaption(tr("UNION_OF_TWO_GROUPS"));
  else if (myMode == INTERSECT) setCaption(tr("INTERSECTION_OF_TWO_GROUPS"));
  else setCaption(tr("CUT_OF_TWO_GROUPS"));

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  QVBoxLayout* aDlgLay = new QVBoxLayout (this, MARGIN, SPACING);

  QFrame* aMainFrame = createMainFrame  (this);
  QFrame* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);

  Init();
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_GroupOpDlg::createMainFrame (QWidget* theParent)
{
  QGroupBox* aMainGrp = new QGroupBox(1, Qt::Horizontal, theParent);
  aMainGrp->setFrameStyle(QFrame::NoFrame);
  aMainGrp->setInsideMargin(0);

  QGroupBox* aNameGrp = new QGroupBox(1, Qt::Vertical, tr("NAME"), aMainGrp);
  new QLabel(tr("RESULT_NAME"), aNameGrp);
  myNameEdit = new QLineEdit(aNameGrp);

  QGroupBox* anArgGrp = new QGroupBox(3, Qt::Horizontal, tr("ARGUMENTS"), aMainGrp);

  new QLabel(myMode == CUT ? tr("MAIN_OBJECT") :tr("OBJECT_1"), anArgGrp);
  myBtn1 = new QPushButton(anArgGrp);
  myEdit1 = new QLineEdit(anArgGrp);
  myEdit1->setAlignment( Qt::AlignLeft );

  new QLabel(myMode == CUT ? tr("TOOL_OBJECT") :tr("OBJECT_2"), anArgGrp);
  myBtn2 = new QPushButton(anArgGrp);
  myEdit2 = new QLineEdit(anArgGrp);
  myEdit2->setAlignment( Qt::AlignLeft );

  myEdit1->setReadOnly(true);
  myEdit2->setReadOnly(true);

  QPixmap aPix (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));
  myBtn1->setPixmap(aPix);
  myBtn2->setPixmap(aPix);

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_GroupOpDlg::createButtonFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);
  aFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);

  myOkBtn     = new QPushButton(tr("SMESH_BUT_OK"   ), aFrame);
  myApplyBtn  = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn  = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);

  QSpacerItem* aSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame, MARGIN, SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addWidget(myApplyBtn);
  aLay->addItem(aSpacer);
  aLay->addWidget(myCloseBtn);

  // connect signals and slots
  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));

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

  this->show();

  // set selection mode
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->installFilter(new SMESH_TypeFilter (GROUP));

  return;
}

//=======================================================================
// name    : SMESHGUI_GroupOpDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_GroupOpDlg::isValid()
{
  // Verify validity of group name
  if (myNameEdit->text() == "") {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                             tr("EMPTY_NAME"), QMessageBox::Ok);
    return false;
  }

  // Verufy wheter arguments speciffiyed
  if (myGroup1->_is_nil() || myGroup2->_is_nil()) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                             tr("INCORRECT_ARGUMENTS"), QMessageBox::Ok);
    return false;
  }

  // Verify whether arguments belongs to same mesh
  SMESH::SMESH_Mesh_ptr aMesh1 = myGroup1->GetMesh();
  SMESH::SMESH_Mesh_ptr aMesh2 = myGroup2->GetMesh();

  int aMeshId1 = !aMesh1->_is_nil() ? aMesh1->GetId() : -1;
  int aMeshId2 = !aMesh2->_is_nil() ? aMesh2->GetId() : -1;

  if (aMeshId1 != aMeshId2 || aMeshId1 == -1) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                             tr("DIFF_MESHES"), QMessageBox::Ok);
    return false;
  }

  // Verify whether groups have same types of entities
  if (myGroup1->GetType() != myGroup2->GetType()) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                             tr("DIFF_TYPES"), QMessageBox::Ok);
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

  if (myMode == UNION) aNewGrp = aMesh->UnionGroups(myGroup1, myGroup2, aName.latin1());
  else if (myMode == INTERSECT) aNewGrp = aMesh->IntersectGroups(myGroup1, myGroup2, aName.latin1());
  else aNewGrp = aMesh->CutGroups(myGroup1, myGroup2, aName.latin1());

  if (!aNewGrp->_is_nil()) {
    mySMESHGUI->updateObjBrowser(true);
    reset();
    return true;
  } else {
    QMessageBox::critical(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                          tr("SMESH_OPERATION_FAILED"), "OK");
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
