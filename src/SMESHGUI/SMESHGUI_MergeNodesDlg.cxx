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
//  File   : SMESHGUI_MergeNodesDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#include "SMESHGUI_MergeNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_SpinBox.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMDS_Mesh.hxx"

#include "GEOMBase.h"

#include "SUIT_ResourceMgr.h"

#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SVTK_Selection.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qheader.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_MergeNodesDlg()
// purpose  :
//=================================================================================
SMESHGUI_MergeNodesDlg::SMESHGUI_MergeNodesDlg( SMESHGUI* theModule, const char* name,
                                                bool modal, WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
                WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SMESH_MERGE_NODES")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  if (!name)
    setName("SMESHGUI_MergeNodesDlg");
  resize(303, 185);
  setCaption(tr("SMESH_MERGE_NODES"));
  setSizeGripEnabled(TRUE);
  SMESHGUI_MergeNodesDlgLayout = new QGridLayout(this);
  SMESHGUI_MergeNodesDlgLayout->setSpacing(6);
  SMESHGUI_MergeNodesDlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup(this, "GroupConstructors");
  GroupConstructors->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5,
                                               (QSizePolicy::SizeType)0, 0, 0,
                                               GroupConstructors->sizePolicy().hasHeightForWidth()));
  GroupConstructors->setTitle(tr("SMESH_MERGE_NODES" ));
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  QHBoxLayout* RBLayout = new QHBoxLayout(0, 0, 6, "Layout2");
  RadioButton1= new QRadioButton(GroupConstructors, "RadioButton1");
  RadioButton1->setText(tr("" ));
  RadioButton1->setPixmap(image0);
  RBLayout->addWidget(RadioButton1);
  GroupConstructorsLayout->addLayout(RBLayout, 0, 0);
  SMESHGUI_MergeNodesDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7,
                                          (QSizePolicy::SizeType)0, 0, 0,
                                          GroupButtons->sizePolicy().hasHeightForWidth()));
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
  buttonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
  buttonApply = new QPushButton(GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_APPLY" ));
  buttonApply->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);
  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  SMESHGUI_MergeNodesDlgLayout->addWidget(GroupButtons, 4, 0);

  /***************************************************************/

  // Controls for mesh defining
  GroupMesh = new QGroupBox(this, "GroupMesh");
  GroupMesh->setTitle(tr("SMESH_MESH"));
  GroupMesh->setColumnLayout(0, Qt::Vertical);
  GroupMesh->layout()->setSpacing(0);
  GroupMesh->layout()->setMargin(0);
  GroupMeshLayout = new QGridLayout(GroupMesh->layout());
  GroupMeshLayout->setAlignment(Qt::AlignTop);
  GroupMeshLayout->setSpacing(6);
  GroupMeshLayout->setMargin(11);

  TextLabelName = new QLabel(GroupMesh, "TextLabelName");
  TextLabelName->setText(tr("SMESH_NAME"));
  GroupMeshLayout->addWidget(TextLabelName, 0, 0);

  SelectMeshButton = new QPushButton(GroupMesh, "SelectMeshButton");
  SelectMeshButton->setPixmap(image1);
  GroupMeshLayout->addWidget(SelectMeshButton, 0, 1);

  LineEditMesh = new QLineEdit(GroupMesh, "LineEditMesh");
  LineEditMesh->setReadOnly(true);
  GroupMeshLayout->addWidget(LineEditMesh, 0, 2);

  SMESHGUI_MergeNodesDlgLayout->addWidget(GroupMesh, 1, 0);

  // Controls for coincident nodes detecting
  GroupCoincident = new QGroupBox(this, "GroupCoincident");
  GroupCoincident->setTitle(tr("COINCIDENT_NODES"));
  GroupCoincident->setColumnLayout(0, Qt::Vertical);
  GroupCoincident->layout()->setSpacing(0);
  GroupCoincident->layout()->setMargin(0);
  QGridLayout* GroupCoincidentLayout = new QGridLayout(GroupCoincident->layout());
  GroupCoincidentLayout->setAlignment(Qt::AlignTop);
  GroupCoincidentLayout->setSpacing(6);
  GroupCoincidentLayout->setMargin(11);

  TextLabelTolerance = new QLabel(GroupCoincident, "TextLabelTolerance");
  TextLabelTolerance->setText(tr("SMESH_TOLERANCE"));
  GroupCoincidentLayout->addWidget(TextLabelTolerance, 0, 0);

  SpinBoxTolerance = new SMESHGUI_SpinBox(GroupCoincident, "SpinBoxTolerance");
  GroupCoincidentLayout->addWidget(SpinBoxTolerance, 0, 1);

  QPushButton* DetectButton = new QPushButton(GroupCoincident, "DetectButton");
  DetectButton->setText(tr("DETECT"));
  GroupCoincidentLayout->addWidget(DetectButton, 0, 2);

  ListCoincident = new QListView(GroupCoincident);
  ListCoincident->setSorting(-1);
  ListCoincident->addColumn("Nodes");
  ListCoincident->header()->hide();

  GroupCoincidentLayout->addMultiCellWidget(ListCoincident, 1, 1, 0, 1);

  SelectAllCB = new QCheckBox(GroupCoincident, "SelectAllCB");
  SelectAllCB->setText(tr("SELECT_ALL"));
  GroupCoincidentLayout->addWidget(SelectAllCB, 2, 0);

  SMESHGUI_MergeNodesDlgLayout->addWidget(GroupCoincident, 2, 0);

  // Controls for editing group of nodes
  GroupEdit = new QGroupBox(this, "GroupEdit");
  GroupEdit->setTitle(tr("EDIT_GROUP_OF_NODES"));
  GroupEdit->setColumnLayout(0, Qt::Vertical);
  GroupEdit->layout()->setSpacing(0);
  GroupEdit->layout()->setMargin(0);
  QGridLayout* GroupEditLayout = new QGridLayout(GroupEdit->layout());
  GroupEditLayout->setAlignment(Qt::AlignTop);
  GroupEditLayout->setSpacing(6);
  GroupEditLayout->setMargin(11);

  ListEdit = new QListBox(GroupEdit, "ListEdit");
  ListEdit->setColumnMode(QListBox::FitToHeight);
  ListEdit->setSelectionMode(QListBox::Extended);
  GroupEditLayout->addMultiCellWidget(ListEdit, 0, 2, 0, 0);

  QPushButton* AddButton = new QPushButton(GroupEdit, "AddButton");
  AddButton->setText(tr("SMESH_BUT_ADD"));
  GroupEditLayout->addWidget(AddButton, 0, 1);

  QPushButton* RemoveButton = new QPushButton(GroupEdit, "RemoveButton");
  RemoveButton->setText(tr("SMESH_BUT_REMOVE"));
  GroupEditLayout->addWidget(RemoveButton, 1, 1);

  QSpacerItem* spacer = new QSpacerItem(20, 200, QSizePolicy::Minimum, QSizePolicy::Expanding);
  GroupEditLayout->addItem(spacer, 2, 1);

  SMESHGUI_MergeNodesDlgLayout->addWidget(GroupEdit, 3, 0);

  /* Initialisations */
  SpinBoxTolerance->RangeStepAndValidator(0.0, 999999.999, 0.1, 3);
  SpinBoxTolerance->SetValue(1e-05);

  RadioButton1->setChecked(TRUE);

  myEditCurrentArgument = (QWidget*)LineEditMesh; 

  myActor = 0;

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));

  connect(SelectMeshButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(DetectButton, SIGNAL (clicked()), this, SLOT(onDetect()));
  connect(ListCoincident, SIGNAL (selectionChanged()), this, SLOT(onSelectNodesGroup()));
  connect(ListCoincident, SIGNAL (pressed(QListViewItem*)), this, SLOT(updateControls()));
  connect(ListCoincident, SIGNAL (currentChanged(QListViewItem*)), this, SLOT(updateControls()));
  connect(SelectAllCB, SIGNAL(toggled(bool)), this, SLOT(onSelectAll(bool)));
  connect(ListEdit, SIGNAL (selectionChanged()), this, SLOT(onSelectNodesFromGroup()));
  connect(AddButton, SIGNAL (clicked()), this, SLOT(onAdd()));
  connect(RemoveButton, SIGNAL (clicked()), this, SLOT(onRemove()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));

  this->show(); /* displays Dialog */

  resize(0,0);
  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // Init Mesh field from selection
  SelectionIntoArgument();
}

//=================================================================================
// function : ~SMESHGUI_MergeNodesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MergeNodesDlg::~SMESHGUI_MergeNodesDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_MergeNodesDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked() || myMesh->_is_nil())
    return false;

  try {
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    QApplication::setOverrideCursor(Qt::waitCursor);
    SMESH::array_of_long_array_var aGroupsOfNodes = new SMESH::array_of_long_array;
    aGroupsOfNodes->length(ListCoincident->childCount());
    QListViewItem* item = ListCoincident->firstChild();

    int anArrayNum = 0;
    while (item) {
      QStringList aListIds = QStringList("");
      if (((QCheckListItem*) item)->isOn())
        aListIds = QStringList::split(" ", item->text(0), false);

      SMESH::long_array_var anIds = new SMESH::long_array;
      anIds->length(aListIds.count());

      for (int i = 0; i < aListIds.count(); i++)
        anIds[i] = aListIds[i].toInt();

      aGroupsOfNodes[anArrayNum++] = anIds.inout();

      item = item->itemBelow();
    }

    aMeshEditor->MergeNodes (aGroupsOfNodes.inout());
    QApplication::restoreOverrideCursor();
  } catch(...) {
  }

  //mySelectionMgr->clearSelected();
  SMESH::UpdateView();

  onDetect();
  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::ClickOnOk()
{
  if (ClickOnApply())
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::ClickOnCancel()
{
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : onEditNodesGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onEditNodesGroup()
{
  if (ListCoincident->childCount() < 1)
    return;

  QString aNewIds = "";

  QListBoxItem* anItem;
  for (anItem = ListEdit->firstItem(); anItem != 0; anItem = anItem->next())
    aNewIds+=QString(" %1").arg(anItem->text());

  ListCoincident->currentItem()->setText(0, aNewIds);
}

//=================================================================================
// function : updateControls()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::updateControls()
{
  if (ListCoincident->childCount() < 1) {
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    return;
  }

  bool hasChecked = false, hasUnchecked = false;

  QListViewItem* item = ListCoincident->firstChild();

  while (item && (!hasChecked || !hasUnchecked)) {
    if (((QCheckListItem*) item)->isOn())
      hasChecked = true;
    else
      hasUnchecked = true;

    item = item->itemBelow();
  }

  if (hasUnchecked)
    SelectAllCB->setChecked(false);

  bool enable = !(myMesh->_is_nil()) && hasChecked;

  buttonOk->setEnabled(enable);
  buttonApply->setEnabled(enable);
}

//=================================================================================
// function : onDetect()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onDetect()
{
  if (myMesh->_is_nil())
    return;

  try {
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

    QApplication::setOverrideCursor(Qt::waitCursor);
    ListCoincident->clear();
    ListEdit->clear();

    SMESH::array_of_long_array_var aNodeGroups;
    aMeshEditor->FindCoincidentNodes(SpinBoxTolerance->GetValue(), aNodeGroups);

    for (int i = 0; i < aNodeGroups->length(); i++) {
      SMESH::long_array& aGroup = aNodeGroups[i];

      QString aNodeIds;
      for (int j = 0; j < aGroup.length(); j++)
        aNodeIds+=QString(" %1").arg(aGroup[j]);

      new QCheckListItem (ListCoincident, aNodeIds, QCheckListItem::CheckBox);
    }
    QApplication::restoreOverrideCursor();
  } catch(...) {
  }

  updateControls();
}

//=================================================================================
// function : onSelectNodesGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onSelectNodesGroup()
{
  if (!myActor)
    return;

  myEditCurrentArgument = (QWidget*)ListCoincident;

  QListViewItem* aSelectedItem = ListCoincident->selectedItem();
  if (!aSelectedItem)
    return;

  QStringList aListId = QStringList::split(" ", aSelectedItem->text(0), false);

  ListEdit->clear();
  ListEdit->insertStringList(aListId);

  //mySelectionMgr->clearSelected();
  //mySelectionMgr->AddIObject(myActor->getIO());
  SALOME_ListIO aList;
  aList.Append(myActor->getIO());
  mySelectionMgr->setSelectedObjects(aList, false);

  SMESH::SetPointRepresentation(true);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(NodeSelection);

  ListEdit->selectAll(true);
}

//=================================================================================
// function : onSelectAll()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onSelectAll (bool isToggled)
{
  if (isToggled) {
    int aNbItems = 0;
    QListViewItem* item = ListCoincident->firstChild();
    while (item) {
      aNbItems++;
      if (!((QCheckListItem*) item)->isOn())
        ((QCheckListItem*) item)->setOn(true);
      item = item->itemBelow();
    }

    if (aNbItems) {
      buttonOk->setEnabled(true);
      buttonApply->setEnabled(true);
    }
  }
}

//=================================================================================
// function : onSelectNodesFromGroup()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onSelectNodesFromGroup()
{
  if (!myActor)
    return;

  TColStd_MapOfInteger aIndexes;
  QListBoxItem* anItem;
  for (anItem = ListEdit->firstItem(); anItem != 0; anItem = anItem->next()) {
    if (anItem->isSelected()) {
      int anId = anItem->text().toInt();
      aIndexes.Add(anId);
    }
  }

  mySelector->AddOrRemoveIndex(myActor->getIO(), aIndexes, false);
  SALOME_ListIO aList;
  aList.Append(myActor->getIO());
  mySelectionMgr->setSelectedObjects(aList);
}

//=================================================================================
// function : onAdd()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onAdd()
{
  if (!myActor)
    return;

  QString aListStr = "";
  int aNbNnodes = 0;

  aNbNnodes = SMESH::GetNameOfSelectedNodes(mySelector, myActor->getIO(), aListStr);
  if (aNbNnodes < 1)
    return;

  QStringList aNodes = QStringList::split(" ", aListStr);
  QListBoxItem* anItem = 0;

  for (QStringList::iterator it = aNodes.begin(); it != aNodes.end(); ++it) {
    anItem = ListEdit->findItem(*it, Qt::ExactMatch);
    if (!anItem) {
      anItem = new QListBoxText(*it);
      ListEdit->insertItem(anItem);
    }
    ListEdit->setSelected(anItem, true);
  }

  onEditNodesGroup();
}

//=================================================================================
// function : onRemove()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::onRemove()
{
  if (myEditCurrentArgument != (QWidget*)ListCoincident)
    return;

  for (int i = ListEdit->count(); i > 0; i--) {
    if (ListEdit->isSelected(i-1))
      ListEdit->removeItem(i-1);
  }
  onEditNodesGroup();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (send == SelectMeshButton) {
    myEditCurrentArgument = (QWidget*)LineEditMesh;
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(myMeshOrSubMeshFilter);
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_MergeNodesDlg::SelectionIntoArgument()
{
  if (myEditCurrentArgument == (QWidget*)LineEditMesh) {
    QString aString = "";
    LineEditMesh->setText(aString);

    ListCoincident->clear();
    ListEdit->clear();

    int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
    if (nbSel != 1)
      return;

    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

    Handle(SALOME_InteractiveObject) IO = aList.First();
    myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
    myActor = SMESH::FindActorByEntry(aList.First()->getEntry());
    if (myMesh->_is_nil() || !myActor)
      return;

    LineEditMesh->setText(aString);
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupMesh->setEnabled(false);
    GroupCoincident->setEnabled(false);
    GroupEdit->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupMesh->setEnabled(true);
  GroupCoincident->setEnabled(true);
  GroupEdit->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MergeNodesDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
//function : hideEvent()
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_MergeNodesDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
}
