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
//  File   : SMESHGUI_RenumberingDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_RenumberingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMDS_Mesh.hxx"

#include "SUIT_Desktop.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "LightApp_Application.h"

#include "SALOME_ListIO.hxx"

#include "utilities.h"

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpixmap.h>

using namespace std;

//=================================================================================
// class    : SMESHGUI_RenumberingDlg()
// purpose  :
//=================================================================================
SMESHGUI_RenumberingDlg::SMESHGUI_RenumberingDlg( SMESHGUI* theModule, const char* name,
						  const int unit, bool modal, WFlags fl)
     : QDialog( SMESH::GetDesktop( theModule ), name, modal, WStyle_Customize | WStyle_NormalBorder |
               WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  myUnit = unit;

  if (!name)
    setName("SMESHGUI_RenumberingDlg");
  resize(303, 185);

  QPixmap image0;
  QPixmap image1(SUIT_Session::session()->resourceMgr()->loadPixmap("SMESH",tr("ICON_SELECT")));

  if (unit == 0) {
    image0 = QPixmap(SUIT_Session::session()->resourceMgr()->loadPixmap("SMESH",tr("ICON_DLG_RENUMBERING_NODES")));
    setCaption(tr("SMESH_RENUMBERING_NODES_TITLE" ));
  }
  else if (unit == 1) {
    image0 = QPixmap(SUIT_Session::session()->resourceMgr()->loadPixmap("SMESH",tr("ICON_DLG_RENUMBERING_ELEMENTS")));
    setCaption(tr("SMESH_RENUMBERING_ELEMENTS_TITLE" ));
  }

  setSizeGripEnabled(TRUE);
  SMESHGUI_RenumberingDlgLayout = new QGridLayout(this);
  SMESHGUI_RenumberingDlgLayout->setSpacing(6);
  SMESHGUI_RenumberingDlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup(this, "GroupConstructors");
  if (unit == 0) {
    GroupConstructors->setTitle(tr("SMESH_NODES" ));
    myHelpFileName = "/files/renumbering_nodes_and_elements.htm#renumber_nodes";
  }
  else if (unit == 1) {
    GroupConstructors->setTitle(tr("SMESH_ELEMENTS" ));
    myHelpFileName = "/files/renumbering_nodes_and_elements.htm#renumber_elements";
  }
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  Constructor1 = new QRadioButton(GroupConstructors, "Constructor1");
  Constructor1->setText(tr("" ));
  Constructor1->setPixmap(image0);
  Constructor1->setChecked(TRUE);
  Constructor1->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth()));
  Constructor1->setMinimumSize(QSize(50, 0));
  GroupConstructorsLayout->addWidget(Constructor1, 0, 0);
  QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupConstructorsLayout->addItem(spacer, 0, 1);
  SMESHGUI_RenumberingDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
  buttonHelp->setText(tr("SMESH_BUT_HELP" ));
  buttonHelp->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonHelp, 0, 4);
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
  SMESHGUI_RenumberingDlgLayout->addWidget(GroupButtons, 2, 0);

  /***************************************************************/
  GroupMesh = new QGroupBox(this, "GroupMesh");
  GroupMesh->setTitle(tr("SMESH_RENUMBERING" ));
  GroupMesh->setMinimumSize(QSize(0, 0));
  GroupMesh->setFrameShape(QGroupBox::Box);
  GroupMesh->setFrameShadow(QGroupBox::Sunken);
  GroupMesh->setColumnLayout(0, Qt::Vertical);
  GroupMesh->layout()->setSpacing(0);
  GroupMesh->layout()->setMargin(0);
  GroupMeshLayout = new QGridLayout(GroupMesh->layout());
  GroupMeshLayout->setAlignment(Qt::AlignTop);
  GroupMeshLayout->setSpacing(6);
  GroupMeshLayout->setMargin(11);
  TextLabelMesh = new QLabel(GroupMesh, "TextLabelMesh");
  TextLabelMesh->setText(tr("SMESH_MESH"));
  TextLabelMesh->setMinimumSize(QSize(50, 0));
  TextLabelMesh->setFrameShape(QLabel::NoFrame);
  TextLabelMesh->setFrameShadow(QLabel::Plain);
  GroupMeshLayout->addWidget(TextLabelMesh, 0, 0);
  SelectButton = new QPushButton(GroupMesh, "SelectButton");
  SelectButton->setText(tr("" ));
  SelectButton->setPixmap(image1);
  SelectButton->setToggleButton(FALSE);
  GroupMeshLayout->addWidget(SelectButton, 0, 1);
  LineEditMesh = new QLineEdit(GroupMesh, "LineEditMesh");
  LineEditMesh->setReadOnly(true);
  GroupMeshLayout->addWidget(LineEditMesh, 0, 2);
  SMESHGUI_RenumberingDlgLayout->addWidget(GroupMesh, 1, 0);

  Init(); /* Initialisations */
}

//=================================================================================
// function : ~SMESHGUI_RenumberingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RenumberingDlg::~SMESHGUI_RenumberingDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::Init()
{
  GroupMesh->show();
  myConstructorId = 0;
  Constructor1->setChecked(TRUE);
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myMesh = SMESH::SMESH_Mesh::_nil();

  myMeshFilter = new SMESH_TypeFilter (MESH);

  /* signals and slots connections */
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectButton, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));

  this->show(); /* displays Dialog */

  myEditCurrentArgument = LineEditMesh;
  LineEditMesh->setFocus();
  mySelectionMgr->clearFilters();
  mySelectionMgr->installFilter(myMeshFilter);

  SelectionIntoArgument();
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RenumberingDlg::ConstructorsClicked (int)
{
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return;

  if (!myMesh->_is_nil())
    {
      try
	{
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

	  QApplication::setOverrideCursor(Qt::waitCursor);
	  if (myUnit == 0) {
	    aMeshEditor->RenumberNodes();
	    if (isUnitsLabeled && anActor) anActor->SetPointsLabeled(true);
	  }
	  else if (myUnit == 1) {
	    aMeshEditor->RenumberElements();
	    if (isUnitsLabeled && anActor) anActor->SetCellsLabeled(true);
	  }
	  QApplication::restoreOverrideCursor();
	}
      catch(...)
	{
	}

      //mySelectionMgr->clearSelected();
      SMESH::UpdateView();
    }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::ClickOnCancel()
{
  //mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
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
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
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
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RenumberingDlg::closeEvent(QCloseEvent* e)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_RenumberingDlg::hideEvent (QHideEvent * e)
{
  if (!isMinimized())
    ClickOnCancel();
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

  if ( e->key() == Key_F1 )
    {
      e->accept();
      ClickOnHelp();
    }
}
