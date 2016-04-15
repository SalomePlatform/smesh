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
//  File   : SMESHGUI_BuildCompoundDlg.cxx
//  Author : Alexander KOVALEV, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_BuildCompoundDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SalomeApp_Study.h>
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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QButtonGroup>

// STL includes
#include <set>

#define SPACING 6
#define MARGIN  11

//=================================================================================
// name    : SMESHGUI_BuildCompoundDlg
// Purpose :
//=================================================================================
SMESHGUI_BuildCompoundDlg::SMESHGUI_BuildCompoundDlg( SMESHGUI* theModule )
  : QDialog(SMESH::GetDesktop(theModule)),
    mySMESHGUI(theModule),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
    myIsApplyAndClose( false )
{
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_BUILD_COMPOUND_TITLE"));

  SUIT_ResourceMgr* aResMgr = SUIT_Session::session()->resourceMgr();
  QPixmap image0 (aResMgr->loadPixmap("SMESH", tr("ICON_DLG_BUILD_COMPOUND_MESH")));
  QPixmap image1 (aResMgr->loadPixmap("SMESH", tr("ICON_SELECT")));

  setSizeGripEnabled(true);

  QVBoxLayout* aTopLayout = new QVBoxLayout(this);
  aTopLayout->setSpacing(SPACING);
  aTopLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(tr("COMPOUND"), this);
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
  GroupName = new QGroupBox(tr("RESULT_NAME"), this);
  QHBoxLayout* GroupNameLayout = new QHBoxLayout(GroupName);
  GroupNameLayout->setSpacing(SPACING);
  GroupNameLayout->setMargin(MARGIN);

  TextLabelName = new QLabel(tr("SMESH_NAME"), GroupName);
  LineEditName = new QLineEdit(GroupName);

  GroupNameLayout->addWidget(TextLabelName);
  GroupNameLayout->addWidget(LineEditName);

  /***************************************************************/
  GroupArgs = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgsLayout = new QGridLayout(GroupArgs);
  GroupArgsLayout->setSpacing(SPACING);
  GroupArgsLayout->setMargin(MARGIN);

  TextLabelMeshes = new QLabel(tr("MESHES"), GroupArgs);
  SelectButton = new QPushButton(GroupArgs);
  SelectButton->setIcon(image1);
  SelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  LineEditMeshes = new QLineEdit(GroupArgs);
  LineEditMeshes->setReadOnly(true);

  TextLabelUnion = new QLabel(tr("PROCESSING_IDENTICAL_GROUPS"), GroupArgs);
  ComboBoxUnion = new QComboBox(GroupArgs);

  CheckBoxCommon = new QCheckBox(tr("CREATE_COMMON_GROUPS"), GroupArgs);

  CheckBoxMerge = new QCheckBox(tr("MERGE_NODES_AND_ELEMENTS"), GroupArgs);

  TextLabelTol = new QLabel(tr("SMESH_TOLERANCE"), GroupArgs);
  TextLabelTol->setAlignment(Qt::AlignCenter);
  SpinBoxTol = new SMESHGUI_SpinBox(GroupArgs);
  SpinBoxTol->RangeStepAndValidator(0.0, COORD_MAX, 0.00001, "len_tol_precision" );

  GroupArgsLayout->addWidget(TextLabelMeshes, 0, 0);
  GroupArgsLayout->addWidget(SelectButton,    0, 1);
  GroupArgsLayout->addWidget(LineEditMeshes,  0, 2, 1, 2);
  GroupArgsLayout->addWidget(TextLabelUnion,  1, 0, 1, 3);
  GroupArgsLayout->addWidget(ComboBoxUnion,   1, 3);
  GroupArgsLayout->addWidget(CheckBoxCommon,  2, 0, 1, 4);
  GroupArgsLayout->addWidget(CheckBoxMerge,   3, 0, 1, 4);
  GroupArgsLayout->addWidget(TextLabelTol,    4, 0, 1, 2);
  GroupArgsLayout->addWidget(SpinBoxTol,      4, 2, 1, 2);

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
  aTopLayout->addWidget(GroupConstructors);
  aTopLayout->addWidget(GroupName);
  aTopLayout->addWidget(GroupArgs);
  aTopLayout->addWidget(GroupButtons);

  myHelpFileName = "building_compounds_page.html";

  Init(); // Initialisations
}

//=================================================================================
// function : ~SMESHGUI_BuildCompoundDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_BuildCompoundDlg::~SMESHGUI_BuildCompoundDlg()
{
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myMesh = SMESH::SMESH_IDSource::_nil();

  myMeshFilter = new SMESH_TypeFilter (SMESH::IDSOURCE);

  myMeshArray = new SMESH::ListOfIDSources();

  // signals and slots connections
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectButton, SIGNAL(clicked()), this, SLOT(SelectionIntoArgument()));

  connect(CheckBoxMerge, SIGNAL(toggled(bool)), this, SLOT(onSelectMerge(bool)));

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));

  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),        this, SLOT(reject()));

  LineEditName->setText(GetDefaultName(tr("COMPOUND_MESH")));
  LineEditMeshes->setFocus();

  ComboBoxUnion->addItem(tr("UNITE"));
  ComboBoxUnion->addItem(tr("RENAME"));
  ComboBoxUnion->setCurrentIndex(0);

  CheckBoxMerge->setChecked(true);

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
    isUnique = (aSet.count(aName.toLatin1().data()) == 0);
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

  if (!isValid())
    return false;

  SMESH::SMESH_Mesh_var aMesh;

  if (!myMesh->_is_nil())
  {
    QStringList aParameters;
    aParameters << (CheckBoxMerge->isChecked() ? SpinBoxTol->text() : QString(" "));

    QStringList anEntryList;
    try {
      SUIT_OverrideCursor aWaitCursor;

      aMesh = myMeshArray[0]->GetMesh();
      aMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
      // concatenate meshes
      if(CheckBoxCommon->isChecked())
        aMesh = aSMESHGen->ConcatenateWithGroups(myMeshArray,
                                                 !(ComboBoxUnion->currentIndex()),
                                                 CheckBoxMerge->isChecked(),
                                                 SpinBoxTol->GetValue());
      else
        aMesh = aSMESHGen->Concatenate(myMeshArray,
                                       !(ComboBoxUnion->currentIndex()),
                                       CheckBoxMerge->isChecked(),
                                       SpinBoxTol->GetValue());

      _PTR(SObject) aSO = SMESH::FindSObject( aMesh );
      if( aSO ) {
        SMESH::SetName( aSO, LineEditName->text() );
        anEntryList.append( aSO->GetID().c_str() );
      }
      mySMESHGUI->updateObjBrowser();
    } catch(...) {
      return false;
    }

    LineEditName->setText(GetDefaultName(tr("COMPOUND_MESH")));

    // IPAL21468 Compound is hidden after creation.
    if ( SMESHGUI::automaticUpdate() ) {
      mySelectionMgr->clearSelected();
      SMESH::UpdateView();

      _PTR(SObject) aSO = SMESH::FindSObject(aMesh.in());
      if ( SMESH_Actor* anActor = SMESH::CreateActor(aSO->GetStudy(), aSO->GetID().c_str()) ) {
        SMESH::DisplayActor(SMESH::GetActiveWindow(), anActor);
        SMESH::UpdateView();
      }
    }// end IPAL21468

    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );

    SMESHGUI::Modified();

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
  setIsApplyAndClose( true );
  if (ClickOnApply())
    reject();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_BuildCompoundDlg::reject()
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
void SMESHGUI_BuildCompoundDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                                                 "application")).
                             arg(myHelpFileName));
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
      myMesh = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      myMeshArray[i] = myMesh;
    }
  }
  else {
    myMesh = SMESH::SMESH_IDSource::_nil();
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
void SMESHGUI_BuildCompoundDlg::enterEvent( QEvent* )
{
  if (GroupConstructors->isEnabled())
    return;
  ActivateThisDialog();
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

  if ( e->key() == Qt::Key_F1 ) {
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
  if(!toMerge)
    SpinBoxTol->SetValue(1e-05);
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_BuildCompoundDlg::isValid()
{
  QString msg;
  bool ok=true;
  if(CheckBoxMerge->isChecked())
    ok = SpinBoxTol->isValid( msg, true );

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}

//================================================================
// function : setIsApplyAndClose
// Purpose  : Set value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
void SMESHGUI_BuildCompoundDlg::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

//================================================================
// function : isApplyAndClose
// Purpose  : Get value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
bool SMESHGUI_BuildCompoundDlg::isApplyAndClose() const
{
  return myIsApplyAndClose;
}
