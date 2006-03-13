//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_CreateHypothesesDlg.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#include "SMESHGUI_CreateHypothesesDlg.h"

#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI.h"

#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"

#include "SALOME_ListIteratorOfListIO.hxx"

#include "utilities.h"

// QT Includes
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qheader.h>

using namespace std;

//=================================================================================
// function : SMESHGUI_CreateHypothesesDlg()
// purpose  : Constructs a SMESHGUI_CreateHypothesesDlg which is a child of 'parent', with the
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
SMESHGUI_CreateHypothesesDlg::SMESHGUI_CreateHypothesesDlg (SMESHGUI* theModule,
                                                            const char* name,
                                                            bool modal,
                                                            bool isAlgo)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
               WStyle_Title | WStyle_SysMenu | WDestructiveClose),
       myIsAlgo(isAlgo),
       mySMESHGUI( theModule )
{
  if (!name)
    setName("SMESHGUI_CreateHypothesesDlg");
  setCaption(isAlgo ? tr("SMESH_CREATE_ALGORITHMS" ) : tr("SMESH_CREATE_HYPOTHESES" ));
  setSizeGripEnabled(TRUE);

  QGridLayout* SMESHGUI_CreateHypothesesDlgLayout = new QGridLayout(this);
  SMESHGUI_CreateHypothesesDlgLayout->setSpacing(6);
  SMESHGUI_CreateHypothesesDlgLayout->setMargin(11);

  /***************************************************************/
  GroupAlgorithms = new QGroupBox(this, "GroupAlgorithms");
  GroupAlgorithms->setTitle(isAlgo ? tr("SMESH_AVAILABLE_ALGORITHMS") : tr("SMESH_AVAILABLE_HYPOTHESES"));
  GroupAlgorithms->setColumnLayout(0, Qt::Vertical);
  GroupAlgorithms->layout()->setSpacing(0);
  GroupAlgorithms->layout()->setMargin(0);

  QGridLayout* hypLayout = new QGridLayout(GroupAlgorithms->layout());
  hypLayout->setGeometry(QRect(12, 18, 139, 250));
  hypLayout->setAlignment(Qt::AlignTop);
  hypLayout->setSpacing(6);
  hypLayout->setMargin(11);

  ListAlgoDefinition = new QListView(GroupAlgorithms, "ListAlgoDefinition");
  ListAlgoDefinition->setMinimumSize(400, 200);
  ListAlgoDefinition->addColumn("");
  ListAlgoDefinition->header()->hide();
  ListAlgoDefinition->setSelectionMode(QListView::Single);
  ListAlgoDefinition->setResizeMode(QListView::AllColumns);
  ListAlgoDefinition->setRootIsDecorated(true);

  hypLayout->addWidget(ListAlgoDefinition, 0, 0);
  SMESHGUI_CreateHypothesesDlgLayout->addWidget(GroupAlgorithms, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);

  buttonApply = new QPushButton(GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_CREATE" ));
  buttonApply->setAutoDefault(TRUE);
  buttonApply->setDefault(FALSE);
  buttonApply->setEnabled(FALSE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);

  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);

  buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
  buttonCancel->setAutoDefault(TRUE);
  buttonCancel->setDefault(TRUE);
  buttonCancel->setEnabled(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);

  SMESHGUI_CreateHypothesesDlgLayout->addWidget(GroupButtons, 1, 0);
  /***************************************************************/

  Init();
}

//=================================================================================
// function : ~SMESHGUI_CreateHypothesesDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_CreateHypothesesDlg::~SMESHGUI_CreateHypothesesDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  InitAlgoDefinition();

  /* signals and slots connections */
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply , SIGNAL(clicked()), this, SLOT(ClickOnApply()));

//  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));

  connect(ListAlgoDefinition, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
  connect(ListAlgoDefinition, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(onDoubleClicked(QListViewItem*)));

  this->show();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::ClickOnCancel()
{
  close();
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;
  QListViewItem* item = ListAlgoDefinition->selectedItem();
  if (!item)
    return;
  QString aHypType = item->text(1);
  MESSAGE("Apply " << aHypType);
  char* sHypType = (char*)aHypType.latin1();

  HypothesisData* aHypData = SMESH::GetHypothesisData(sHypType);
  if (!aHypData)
    return;
  QString aClientLibName = aHypData->ClientLibName;
  MESSAGE("Client lib name = " << aClientLibName);

  if (aClientLibName == "") {
    // Call hypothesis creation server method (without GUI)
    QString aHypName = aHypData->Label;
    SMESH::CreateHypothesis(sHypType, aHypName, myIsAlgo);
  } else {
    // Get hypotheses creator client (GUI)
    SMESHGUI_GenericHypothesisCreator* aCreator =
      SMESH::GetHypothesisCreator(sHypType);

    if( aCreator )
      // Create hypothesis/algorithm
      aCreator->create( myIsAlgo, this );
    else
    {
      // report about error
    }
  }

//  buttonApply->setEnabled(FALSE);
  return;
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::ActivateThisDialog()
{
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupButtons->setEnabled(true);
  return;
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::enterEvent (QEvent*)
{
  ActivateThisDialog();
  return;
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::closeEvent (QCloseEvent* e)
{
  mySMESHGUI->ResetState();
  QDialog::closeEvent(e);
}

//=================================================================================
// function : onSelectionChanged()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::onSelectionChanged()
{
  QListViewItem* item = ListAlgoDefinition->selectedItem();
  buttonApply->setEnabled(item && item->depth() > 0);
}

//=================================================================================
// function : onDoubleClicked()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::onDoubleClicked (QListViewItem* i)
{
  if (i && i->depth() > 0)
    this->ClickOnApply();
}

//=================================================================================
// function : InitAlgoDefinition()
// purpose  :
//=================================================================================
void SMESHGUI_CreateHypothesesDlg::InitAlgoDefinition()
{
  ListAlgoDefinition->clear();
  QStringList HypList = SMESH::GetAvailableHypotheses(myIsAlgo);
  for (int i = 0; i < HypList.count(); ++i) {
    HypothesisData* aHypData = SMESH::GetHypothesisData(HypList[i]);
    QListViewItem* parentItem = 0;
    QListViewItem* childItem = ListAlgoDefinition->firstChild();
    while (childItem) {
      if (childItem->text(0) == aHypData->PluginName) {
	parentItem = childItem;
	break;
      }
      childItem = childItem->nextSibling();
    }
    if (!parentItem)
      parentItem = new QListViewItem(ListAlgoDefinition, aHypData->PluginName);
    parentItem->setOpen(true);
    QListViewItem* aItem = new QListViewItem(parentItem, aHypData->Label, HypList[i]);
    QPixmap aPixMap (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr(aHypData->IconId)));
    if (!aPixMap.isNull())
      aItem->setPixmap(0, aPixMap);
  }
}
