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
//  File   : SMESHGUI_BuildCompoundDlg.cxx
//  Author : Alexander KOVALEV
//  Module : SMESH

#include "SMESHGUI_BuildCompoundDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMESH_TypeFilter.hxx"

#include "SUIT_Desktop.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"
#include "SalomeApp_Study.h"

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
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qsizepolicy.h>
#include <qstring.h>

#include <vector>
#include <set>

//=================================================================================
// name    : SMESHGUI_BuildCompoundDlg
// Purpose :
//=================================================================================
SMESHGUI_BuildCompoundDlg::SMESHGUI_BuildCompoundDlg( SMESHGUI* theModule)
  : QDialog(SMESH::GetDesktop(theModule), "SMESHGUI_BuildCompoundDlg", false, WStyle_Customize |
            WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
    mySMESHGUI(theModule),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule))
{
  setCaption(tr("SMESH_BUILD_COMPOUND_TITLE"));

  SUIT_ResourceMgr* aResMgr = SUIT_Session::session()->resourceMgr();
  QPixmap image0 (aResMgr->loadPixmap("SMESH", tr("ICON_DLG_BUILD_COMPOUND_MESH")));
  QPixmap image1 (aResMgr->loadPixmap("SMESH", tr("ICON_SELECT")));

  setSizeGripEnabled(TRUE);
  SMESHGUI_BuildCompoundDlgLayout = new QGridLayout (this);
  SMESHGUI_BuildCompoundDlgLayout->setSpacing(6);
  SMESHGUI_BuildCompoundDlgLayout->setMargin(11);

  /***************************************************************/
  GroupConstructors = new QButtonGroup (this, "GroupConstructors");
  GroupConstructors->setTitle(tr("COMPOUND" ));
  GroupConstructors->setExclusive(TRUE);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  GroupConstructorsLayout = new QGridLayout (GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6);
  GroupConstructorsLayout->setMargin(11);
  Constructor1 = new QRadioButton (GroupConstructors, "Constructor1");
  Constructor1->setText(tr(""));
  Constructor1->setPixmap(image0);
  Constructor1->setChecked(TRUE);
  GroupConstructorsLayout->addWidget(Constructor1, 0, 0);
  SMESHGUI_BuildCompoundDlgLayout->addWidget(GroupConstructors, 0, 0);

  /***************************************************************/
  GroupName = new QGroupBox (this, "GroupName");
  GroupName->setTitle(tr("RESULT_NAME" ));
  GroupName->setColumnLayout(0, Qt::Vertical);
  GroupName->layout()->setSpacing(0);
  GroupName->layout()->setMargin(0);
  GroupNameLayout = new QGridLayout (GroupName->layout());
  GroupNameLayout->setAlignment(Qt::AlignTop);
  GroupNameLayout->setSpacing(6);
  GroupNameLayout->setMargin(11);
  TextLabelName = new QLabel (GroupName, "TextLabelName");
  TextLabelName->setText(tr("SMESH_NAME"));
  GroupNameLayout->addWidget(TextLabelName, 0, 0);
  LineEditName = new QLineEdit (GroupName, "LineEditName");
  GroupNameLayout->addWidget(LineEditName, 0, 1);
  SMESHGUI_BuildCompoundDlgLayout->addWidget(GroupName, 1, 0);

  /***************************************************************/
  GroupArgs = new QGroupBox (this, "GroupArgs");
  GroupArgs->setTitle(tr("SMESH_ARGUMENTS" ));
  GroupArgs->setColumnLayout(0, Qt::Vertical);
  GroupArgs->layout()->setSpacing(0);
  GroupArgs->layout()->setMargin(0);
  GroupArgsLayout = new QGridLayout (GroupArgs->layout());
  GroupArgsLayout->setAlignment(Qt::AlignTop);
  GroupArgsLayout->setSpacing(6);
  GroupArgsLayout->setMargin(11);

  TextLabelMeshes = new QLabel (GroupArgs, "TextLabelMeshes");
  TextLabelMeshes->setText(tr("MESHES"));
  TextLabelMeshes->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  GroupArgsLayout->addWidget(TextLabelMeshes, 0, 0);
  SelectButton = new QPushButton (GroupArgs, "SelectButton");
  SelectButton->setText(tr(""));
  SelectButton->setPixmap(image1);
  SelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  GroupArgsLayout->addWidget(SelectButton, 0, 1);
  LineEditMeshes = new QLineEdit (GroupArgs, "LineEditMeshes");
  LineEditMeshes->setReadOnly(true);
  GroupArgsLayout->addMultiCellWidget(LineEditMeshes, 0, 0, 2, 3);

  TextLabelUnion = new QLabel (GroupArgs, "TextLabelUnion");
  TextLabelUnion->setText(tr("PROCESSING_IDENTICAL_GROUPS"));
  TextLabelUnion->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  GroupArgsLayout->addMultiCellWidget(TextLabelUnion, 1, 1, 0, 2);
  ComboBoxUnion = new QComboBox(GroupArgs, "ComboBoxUnion");
  GroupArgsLayout->addMultiCellWidget(ComboBoxUnion, 1, 1, 3, 3);

  CheckBoxCommon = new QCheckBox(GroupArgs, "CheckBoxCommon");
  CheckBoxCommon->setText(tr("CREATE_COMMON_GROUPS" ));
  GroupArgsLayout->addMultiCellWidget(CheckBoxCommon, 2, 2, 0, 3);

  CheckBoxMerge = new QCheckBox(GroupArgs, "CheckBoxMerge");
  CheckBoxMerge->setText(tr("MERGE_NODES_AND_ELEMENTS" ));
  GroupArgsLayout->addMultiCellWidget(CheckBoxMerge, 3, 3, 0, 3);

  TextLabelTol = new QLabel (GroupArgs, "TextLabelTol");
  TextLabelTol->setText(tr("SMESH_TOLERANCE"));
  TextLabelTol->setAlignment(Qt::AlignCenter);
  GroupArgsLayout->addMultiCellWidget(TextLabelTol, 4, 4, 0, 1);
  SpinBoxTol = new SMESHGUI_SpinBox (GroupArgs, "SpinBoxTol");
  SpinBoxTol->RangeStepAndValidator(0.0, COORD_MAX, 0.1, 6);
  GroupArgsLayout->addMultiCellWidget(SpinBoxTol, 4, 4, 2, 3);

  SMESHGUI_BuildCompoundDlgLayout->addWidget(GroupArgs, 2, 0);

  /***************************************************************/
  GroupButtons = new QGroupBox (this, "GroupButtons");
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  GroupButtonsLayout = new QGridLayout (GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
  buttonHelp->setText(tr("SMESH_BUT_HELP" ));
  buttonHelp->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonHelp, 0, 4);
  buttonCancel = new QPushButton (GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
  buttonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
  buttonApply = new QPushButton (GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_APPLY" ));
  buttonApply->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);
  QSpacerItem* spacer_9 = new QSpacerItem (20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);
  buttonOk = new QPushButton (GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  SMESHGUI_BuildCompoundDlgLayout->addWidget(GroupButtons, 3, 0);

  myHelpFileName = "building_compounds_page.html";

  Init(); // Initialisations
}

//=================================================================================
// function : ~SMESHGUI_BuildCompoundDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_BuildCompoundDlg::~SMESHGUI_BuildCompoundDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::Init()
{
  GroupName->show();
  GroupArgs->show();
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myMesh = SMESH::SMESH_Mesh::_nil();

  myMeshFilter = new SMESH_TypeFilter (MESH);

  myMeshArray = new SMESH::mesh_array();

  // signals and slots connections
  connect(buttonOk    , SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply , SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectButton, SIGNAL(clicked()), this, SLOT(SelectionIntoArgument()));

  connect(CheckBoxMerge, SIGNAL(toggled(bool)), this, SLOT(onSelectMerge(bool)));

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));

  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs())       , this, SLOT(ClickOnCancel()));

  this->show(); // displays Dialog

  LineEditName->setText(GetDefaultName(tr("COMPOUND_MESH")));
  LineEditMeshes->setFocus();

  ComboBoxUnion->insertItem(tr("UNITE"));
  ComboBoxUnion->insertItem(tr("RENAME"));
  ComboBoxUnion->setCurrentItem(0);

  CheckBoxMerge->setChecked(false);

  TextLabelTol->setEnabled(CheckBoxMerge->isChecked());
  SpinBoxTol->SetValue(1e-05);

  SpinBoxTol->setEnabled(CheckBoxMerge->isChecked());

  mySelectionMgr->clearFilters();
  mySelectionMgr->installFilter(myMeshFilter);

  SelectionIntoArgument();
}

//=================================================================================
// function : GetDefaultName()
// purpose  :
//=================================================================================
QString SMESHGUI_BuildCompoundDlg::GetDefaultName(const QString& theOperation)
{
  QString aName = "";

  // collect all object names of SMESH component
  SalomeApp_Study* appStudy =
    dynamic_cast<SalomeApp_Study*>( SUIT_Session::session()->activeApplication()->activeStudy() );
  if ( !appStudy ) return aName;
  _PTR(Study) aStudy = appStudy->studyDS();

  std::set<std::string> aSet;
  _PTR(SComponent) aMeshCompo (aStudy->FindComponent("SMESH"));
  if (aMeshCompo) {
    _PTR(ChildIterator) it (aStudy->NewChildIterator(aMeshCompo));
    _PTR(SObject) obj;
    for (it->InitEx(true); it->More(); it->Next()) {
      obj = it->Value();
      aSet.insert(obj->GetName());
    }
  }

  // build a unique name
  int aNumber = 0;
  bool isUnique = false;
  while (!isUnique) {
    aName = theOperation + "_" + QString::number(++aNumber);
    isUnique = (aSet.count(aName.latin1()) == 0);
  }

  return aName;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_BuildCompoundDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;
  if (!myMesh->_is_nil()) {
    try	{
      QApplication::setOverrideCursor(Qt::waitCursor);
      
      SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
      // concatenate meshes
      SMESH::SMESH_Mesh_var aCompoundMesh;
      if(CheckBoxCommon->isChecked())
	aCompoundMesh = aSMESHGen->ConcatenateWithGroups(myMeshArray, 
							 !(ComboBoxUnion->currentItem()), 
							 CheckBoxMerge->isChecked(), 
							 SpinBoxTol->GetValue());
      else
	aCompoundMesh = aSMESHGen->Concatenate(myMeshArray, 
					       !(ComboBoxUnion->currentItem()), 
					       CheckBoxMerge->isChecked(), 
					       SpinBoxTol->GetValue());
     
      SMESH::SetName( SMESH::FindSObject( aCompoundMesh ), LineEditName->text().latin1() );
      QApplication::restoreOverrideCursor();
      mySMESHGUI->updateObjBrowser();
    } catch(...) {
      return false;
    }

    LineEditName->setText(GetDefaultName(tr("COMPOUND_MESH")));

    //mySelectionMgr->clearSelected();
    SMESH::UpdateView();
    return true;
  }
  return false;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::ClickOnOk()
{
  if (ClickOnApply())
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::ClickOnCancel()
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
void SMESHGUI_BuildCompoundDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", "application")).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_BuildCompoundDlg::SelectionIntoArgument()
{
  if (!GroupButtons->isEnabled()) // inactive
    return;

  QString aString = "";

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);
  int nbSel = SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

  if (nbSel != 0) {
    myMeshArray->length(nbSel);
    for (int i = 0; nbSel != 0; i++, nbSel--) {
      Handle(SALOME_InteractiveObject) IO = aList.First();
      aList.RemoveFirst();
      myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
      myMeshArray[i] = myMesh;
    }
  }
  else {
    myMesh = SMESH::SMESH_Mesh::_nil();
    aString = "";
  }

  LineEditMeshes->setText(aString);

  bool isEnabled = (!myMesh->_is_nil());
  buttonOk->setEnabled(isEnabled);
  buttonApply->setEnabled(isEnabled);
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupName->setEnabled(false);
    GroupArgs->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupName->setEnabled(true);
  GroupArgs->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::enterEvent(QEvent* e)
{
  if (GroupConstructors->isEnabled())
    return;
  ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::closeEvent(QCloseEvent* e)
{
  /* same than click on cancel button */
  this->ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_BuildCompoundDlg::hideEvent (QHideEvent * e)
{
  if (!isMinimized())
    ClickOnCancel();
}


//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::keyPressEvent( QKeyEvent* e )
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


//=================================================================================
// function : onSelectMerge()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::onSelectMerge(bool toMerge)
{
  TextLabelTol->setEnabled(toMerge);
  SpinBoxTol->setEnabled(toMerge);
  
}
